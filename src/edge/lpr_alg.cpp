#include "lpr_alg.h"
#include "TH_PlateID.h"
#include "jpg_codec.h"

#include <stdio.h>
#include <iostream>

using namespace std;

#define WIDTH           4096            // Max image width
#define HEIGHT          2160            // Max image height

TH_PlateIDCfg c_Config;
static unsigned char mem1[0x4000];                // 16K
static unsigned char mem2[100*1024*1024];            // 100M

//车牌识别初始化
bool vlpr_init()
{
    //c_Config = c_defConfig;
    c_Config.nMinPlateWidth = 80;
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
    TH_SetEnabledPlateFormat(PARAM_TWOROWYELLOW_OFF, &c_Config);
    //TH_SetEnabledPlateFormat(PARAM_INDIVIDUAL_ON, &c_Config);
    TH_SetEnabledPlateFormat(PARAM_ARMPOLICE_ON, &c_Config);
    //TH_SetEnabledPlateFormat(PARAM_TWOROWARMY_ON, &c_Config);
    //TH_SetEnabledPlateFormat(PARAM_ARMPOLICE2_ON, &c_Config);
    //TH_SetEnabledPlateFormat(PARAM_TRACTOR_ON, &c_Config);
    //TH_SetEnabledPlateFormat(PARAM_EMBASSY_ON, &c_Config);
    TH_SetImageFormat(ImageFormatRGB, 0, 1, &c_Config);
    //TH_SetEnableCarTypeClassify(1, &c_Config);
    //TH_SetEnableCarLogo( 1, &c_Config);
    //TH_SetEnableCarWidth( 1, &c_Config);
    TH_SetProvinceOrder((char*)default_province, &c_Config);
    
    //TH_SetRecogThreshold( 5, 1, &c_Config);
    //TH_SetDayNightMode( 1, &c_Config );
    return true;
}
//车牌识别，输入为jpg图像指针和长度，输出识别结果，识别结果结构体需要调用方分配
bool vlpr_analyze(const unsigned char *pImage, int len, PVPR pVPR)
{
    int w=0;int h=0; int c=3;
    unsigned char *rgb_buf = (unsigned char *)malloc(4096*2160*3);
    BoonJpegCodec bjc;
    clock_t t=clock();
    bool ret = bjc.JpegUnCompress((char *)pImage, len, (char *)rgb_buf,
                                  4096*2160*3, w, h, c);
    if(!ret)
        return false;
    clock_t tt = clock() - t;
    std::cout << "Jpeg w:\t" << w <<"\th:\t" << h << "\tTime:\t" << tt/1000 << "ms" << std::endl;
    TH_PlateIDResult result[6];
    int nResultNum=1;
    TH_RECT roi_rect;
    roi_rect.left= (int)w*0.1;
    roi_rect.right=(int)w*0.9;
    roi_rect.top = (int)h*0.3;
    roi_rect.bottom = (int)h*0.95;
    t=clock();
    int nRet=TH_RecogImage(rgb_buf, w, h,  result, &nResultNum, &roi_rect, &c_Config);
    tt = clock() - t;
    cout << "1 time " << tt/1000 << "ms" << endl;
    t=clock();
     nRet=TH_RecogImage(rgb_buf, w, h,  result, &nResultNum, &roi_rect, &c_Config);
    tt = clock() - t;
    cout << "2 time " << tt/1000 << "ms" << endl;
    t=clock();
     nRet=TH_RecogImage(rgb_buf, w, h,  result, &nResultNum, &roi_rect, &c_Config);
    tt = clock() - t;
    cout << "3 time " << tt/1000 << "ms" << endl;
    
    printf("ret=%d cnt=%d \n", nRet, nResultNum);
    
    if(nRet!=0)
    {
        printf("RecogImageRet = %d\n", nRet);
        return false;
    }
    
    if(nRet == 0)
    {
        for(int i=0; i<nResultNum; i++)
        {
            // memcpy(&nFrame,  result[i].pbyBits, 8);
            //printf("nFrame=%I64d\n", nFrame);
            if(result[i].nType>=0)
                printf("plate: %s color: %s confidence:%d time:%d\n", result[i].license, result[i].color, result[i].nConfidence, result[i].nTime);
            else
                printf("Type: %d\n", result[i].nType);
        }
    }
    return true;
}
