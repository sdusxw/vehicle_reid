#include "lpr_alg.h"
#include "TH_PlateID.h"

#include <stdio.h>

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

    
    int nRet = TH_InitPlateIDSDK(&c_Config);
    printf("init SDK ret = %d\n", nRet);
    if(nRet!=0)
        return false;
    char default_province[] = "苏";
    
    TH_SetRecogThreshold(5,2,&c_Config);
    TH_SetEnabledPlateFormat(PARAM_TWOROWYELLOW_ON, &c_Config);
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
    return true;
}
