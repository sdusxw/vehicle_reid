#include <stdio.h>
#include <stdlib.h>
#include "jpg_codec.h"

#define BGR888

static unsigned char mem1[0x4000];				// 16K
static unsigned char mem2[40000000];			// 100M


TH_PlateIDCfg c_Config;

#define WIDTH           1600			// Max image width
#define HEIGHT          1200			// Max image height
unsigned char pImg[WIDTH*HEIGHT*3];		// memory for loading image

int main(int argc, char **argv)
{
	int i;
	int nResultNum;
/*
    char file_name[64];
    TH_PlateIDResult result[6];
    int nRet;
	int width, height;
	unsigned int ID1, ID2;
	TH_RECT rect;
	//__int64 nFrame;
	int nThreadNum;

    //c_Config = c_defConfig;
	c_Config.nMinPlateWidth = 60;
	c_Config.nMaxPlateWidth = 400;
	c_Config.bMovingImage = 0;
	c_Config.bOutputSingleFrame = 1;
	c_Config.nMaxImageWidth = WIDTH;
	c_Config.nMaxImageHeight = HEIGHT;
	c_Config.pFastMemory = mem1;
	c_Config.nFastMemorySize = 0x4000;
	c_Config.pMemory = mem2;
	c_Config.nMemorySize= 40000000;
	c_Config.bIsFieldImage = 0;
	c_Config.bUTF8 = 1;
	//c_Config.bLotDetect = 3;
	//c_Config.bLeanCorrection = 1;
	//nRet = TH_GetKeyMaxThread(&nThreadNum); 
	//printf("ret=%d MaxThread=%d\n", nRet, nThreadNum);

    nRet = TH_InitPlateIDSDK(&c_Config);
	printf("init SDK ret = %d\n", nRet);
	if(nRet!=0)
		return -1;
	
	//nRet = TH_GetKeyID(&ID1, &ID2);
	//printf("KeyID: %4X %4X\n", ID1,ID2);
	TH_SetRecogThreshold(5,2,&c_Config);
	//TH_SetEnabledPlateFormat(PARAM_TWOROWYELLOW_ON, &c_Config);
	//TH_SetEnabledPlateFormat(PARAM_INDIVIDUAL_ON, &c_Config);
	//TH_SetEnabledPlateFormat(PARAM_ARMPOLICE_ON, &c_Config);
	//TH_SetEnabledPlateFormat(PARAM_TWOROWARMY_ON, &c_Config);
	//TH_SetEnabledPlateFormat(PARAM_ARMPOLICE2_ON, &c_Config);
	//TH_SetEnabledPlateFormat(PARAM_TRACTOR_ON, &c_Config);
	//TH_SetEnabledPlateFormat(PARAM_EMBASSY_ON, &c_Config);
	TH_SetImageFormat(ImageFormatRGB, 0, 1, &c_Config);
	//TH_SetEnableCarTypeClassify(1, &c_Config);
	//TH_SetEnableCarLogo( 1, &c_Config);
	//TH_SetEnableCarWidth( 1, &c_Config);
	//TH_SetProvinceOrder("¾©",&c_Config);
	
	//TH_SetRecogThreshold( 5, 1, &c_Config);
	//TH_SetDayNightMode( 1, &c_Config );

	file_list = fopen("test_list.txt", "r");
	
	if(file_list == NULL)
	{
   		printf("fail to open test file list\n");
   		return -1;
	}
	while(readline(file_list, file_name) != 0)
	{
		//ReadBmp(file_name,pImg,&width,&height);
		read_JPEG_file(file_name, pImg, &width, &height);
		//printf("test %s %d %d...\n", file_name, width, height);
		
		nResultNum=6;
		nRet=TH_RecogImage(pImg, width, height,  result, &nResultNum, NULL, &c_Config);
		printf("ret=%d cnt=%d \n", nRet, nResultNum);
		
		if(nRet!=0)
		{
			printf("RecogImageRet = %d\n", nRet);
			return -1;
		}
 
		if(nRet == 0)
		{
			for(i=0; i<nResultNum; i++)
			{
				// memcpy(&nFrame,  result[i].pbyBits, 8);
				//printf("nFrame=%I64d\n", nFrame);
				printf("test %s %d %d...\n", file_name, width, height);
				if(result[i].nType>=0)
					printf("plate: %s confidence:%d time:%d\n", result[i].license, result[i].nConfidence, result[i].nTime);
				else
					printf("Type: %d\n", result[i].nType);
			}
		}
	}
	fclose(file_list);
 */
	return 0;
}

