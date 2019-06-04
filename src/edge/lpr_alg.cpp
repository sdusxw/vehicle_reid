#include "lpr_alg.h"
#include "TH_PlateID.h"
#include "jpg_codec.h"
#include <string.h>
#include <stdio.h>
#include <iostream>

using namespace std;

#define WIDTH           4096            // Max image width
#define HEIGHT          2160            // Max image height

TH_PlateIDCfg c_Config;
BoonJpegCodec bjc;
static unsigned char mem1[0x4000];                // 16K
static unsigned char mem2[100*1024*1024];            // 100M

//车牌识别初始化
bool vlpr_init()
{
    //c_Config = c_defConfig;
    c_Config.nMinPlateWidth = 65;
    c_Config.nMaxPlateWidth = 220;
    c_Config.bMovingImage = 0;
    c_Config.bOutputSingleFrame = 1;
    c_Config.nMaxImageWidth = WIDTH;
    c_Config.nMaxImageHeight = HEIGHT;
    c_Config.pFastMemory = mem1;
    c_Config.nFastMemorySize = 0x4000;
    c_Config.pMemory = mem2;
    c_Config.nMemorySize= 100*1024*1024;
    c_Config.bIsFieldImage = 0;
    c_Config.bUTF8 = 1;
    c_Config.bLotDetect = 0;
    c_Config.bLeanCorrection = 1;
    c_Config.nOrderOpt = 2;
    c_Config.nImproveSpeed = 1;

    
    int nRet = TH_InitPlateIDSDK(&c_Config);
    printf("init SDK ret = %d\n", nRet);
    if(nRet!=0)
        return false;
    char default_province[] = "苏";
    
    TH_SetRecogThreshold(5,2,&c_Config);
    TH_SetEnabledPlateFormat(PARAM_TWOROWYELLOW_ON, &c_Config);
    TH_SetEnabledPlateFormat(PARAM_INDIVIDUAL_OFF, &c_Config);
    TH_SetEnabledPlateFormat(PARAM_ARMPOLICE_ON, &c_Config);
    TH_SetEnabledPlateFormat(PARAM_TWOROWARMY_OFF, &c_Config);
    TH_SetEnabledPlateFormat(PARAM_ARMPOLICE2_OFF, &c_Config);
    TH_SetEnabledPlateFormat(PARAM_TRACTOR_OFF, &c_Config);
    TH_SetEnabledPlateFormat(PARAM_EMBASSY_OFF, &c_Config);
    TH_SetImageFormat(ImageFormatRGB, 0, 1, &c_Config);
    TH_SetEnableCarTypeClassify(0, &c_Config);
    TH_SetEnableCarLogo( 0, &c_Config);
    TH_SetEnableCarWidth( 0, &c_Config);
    //TH_SetProvinceOrder((char*)default_province, &c_Config);
    
    //TH_SetRecogThreshold( 5, 1, &c_Config);
    //TH_SetDayNightMode( 1, &c_Config );
    return true;
}
//车牌颜色转化
//蓝 1   3
//黄 2   4
//白 3   2
//黑 4   3
//绿 5   5
//默认蓝色，没有黄绿色
//因为目前几乎没有黑色拍照，所有黑色拍照按照烂牌处理
int pcolor_transfer(int c)
{
    int r=3;
    switch (c) {
        case 1:
            r=3;
            break;
        case 2:
            r=4;
            break;
        case 3:
            r=2;
            break;
        case 4:
            r=3;
            break;
        case 5:
            r=5;
            break;
    }
    return r;
}
//车牌识别，输入为jpg图像指针和长度，输出识别结果，识别结果结构体需要调用方分配
bool vlpr_analyze(const unsigned char *pImage, int len, PVPR pVPR)
{
    int w=0;int h=0; int c=3;
    unsigned char *rgb_buf = (unsigned char *)malloc(4096*2160*3);
    if(rgb_buf==NULL)
    {
        return false;
    }
    bool ret = bjc.JpegUnCompress((char *)pImage, len, (char *)rgb_buf,
                                  4096*2160*3, w, h, c);
    if(!ret)
    {
        //JPG解码失败释放缓存
        if(rgb_buf)
        {
            free(rgb_buf);
            rgb_buf=NULL;
        }
        return false;
    }
    TH_PlateIDResult result[6];
    int nResultNum=1;
    TH_RECT roi_rect;
    roi_rect.left= (int)w*0.05;
    roi_rect.right=(int)w*0.95;
    roi_rect.top = (int)h*0.1;
    roi_rect.bottom = (int)h*0.98;
    int nRet=TH_RecogImage(rgb_buf, w, h,  result, &nResultNum, &roi_rect, &c_Config);
    //识别之后释放RGB缓存
    if(rgb_buf)
    {
        free(rgb_buf);
        rgb_buf=NULL;
    }
    if(nRet == 0)
    {
        if (nResultNum == 0) {
            return false;
        }
        //车牌结果输出
        strcpy(pVPR->license, result[0].license);
        //车牌颜色处理
        strcpy(pVPR->color, result[0].color);
        pVPR->nColor = pcolor_transfer(result[0].nColor);
        //车牌类型
        pVPR->nType = result[0].nType;
        //置信度
        pVPR->nConfidence = result[0].nConfidence;
        //车牌坐标
        pVPR->left = result[0].rcLocation.left;
        pVPR->right = result[0].rcLocation.right;
        pVPR->top = result[0].rcLocation.top;
        pVPR->bottom = result[0].rcLocation.bottom;
        return true;
    }
    else{
        return false;
    }
}
