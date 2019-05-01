#include <stdio.h>
#include <stdlib.h>
#include "TH_PlateID.h"

#define BGR888


int readline(FILE* f, char *line)
{
    char c;
    int len = 0;
       
    while ((c=fgetc(f)) != EOF && c != '\n' && c != '\r')
    {
        line[len++] = c;
    }
    line[len] = '\0';
	if(len > 0)
		return 1;
    else
		return 0;
	
	//if(c==EOF)
		//fseek(f, 0, 0);
	return 1;
	
}


int ReadBmp(char * FileName, unsigned char * pImg, int *pwidth, int *pheight);
int read_JPEG_file (char * filename, unsigned char* pImg, int* pwidth, int* pheight);


static unsigned char mem1[0x4000];				// 16K
static unsigned char mem2[40000000];			// 100M


TH_PlateIDCfg c_Config;
int nMinSram, nMinSdram;				// check SRAM and SDRAM

#ifdef BGR888
#define WIDTH           1600			// Max image width
#define HEIGHT          1200			// Max image height
unsigned char pImg[WIDTH*HEIGHT*3];		// memory for loading image

int main(int argc, char **argv)
{
	int i;
    FILE *file_list;
	int nResultNum;
    
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
	//TH_SetProvinceOrder("京",&c_Config);
	
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
			
		TH_CheckMinFreeMemory(&nMinSram, &nMinSdram, &c_Config);
		printf("MinSram: %d MinSdram: %d\n", nMinSram, nMinSdram);
		//if(nResultNum)
		//{
		//	TH_EvaluateCarColor(pImg, width, height,  result, &nResultNum, NULL, &c_Config);
		//}
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
	return 0;
}

#endif

#if (defined(YUV422) || defined(YUV420) || defined(UYVY) || defined(NV21))
#define WIDTH           1920				
#define HEIGHT          1088
unsigned char pImg[WIDTH*HEIGHT*3];

// Test YUV422 
int main(void)
{
	int i = 0;
	int j;
    FILE *file_list, *fin;
	int nResultNum;
	char buf[32];
    int width = WIDTH; 
    int height = HEIGHT;
	TH_RECT rect;
    char file_name[64];
    TH_PlateIDResult result[6];
	int nResultCnt = 0;
    int nRet;
	int nFileCnt = 0;
#ifdef DSP
	Setup_Cache();
#endif
    c_Config=c_defConfig;
	c_Config.nMaxImageWidth = WIDTH;
	c_Config.nMaxImageHeight = HEIGHT;
	c_Config.pFastMemory=mem1;
	c_Config.nFastMemorySize=0x4000;
	c_Config.pMemory=mem2;
	c_Config.nMemorySize=10000000;
	c_Config.bIsFieldImage=0;
	c_Config.nMinPlateWidth=80;
	c_Config.nMaxPlateWidth=200;
	
	c_Config.bOutputSingleFrame=0;
	printf("init SDK\n");
    nRet = TH_InitPlateIDSDK(&c_Config);	
    printf("init ret = %d\n", nRet);
#ifdef YUV422
	TH_SetImageFormat(ImageFormatNV16, 0, 0, &c_Config);
	//TH_SetImageFormat(ImageFormatYUV422, 0, 1, &c_Config);
#endif
#ifdef YUV420
	TH_SetImageFormat(ImageFormatRGB, 0, 1, &c_Config);
#endif
#ifdef UYVY
	TH_SetImageFormat(ImageFormatUYVY, 0, 0, &c_Config);
#endif

#ifdef NV21
	TH_SetImageFormat(ImageFormatNV21, 0, 0, &c_Config);
#endif	
	TH_SetRecogThreshold(5,1,&c_Config);
	//TH_SetProvinceOrder("鲁", &c_Config);
	TH_SetEnabledPlateFormat(PARAM_INDIVIDUAL_ON, &c_Config);
	TH_SetEnabledPlateFormat(PARAM_ARMPOLICE_ON, &c_Config);
	TH_SetEnabledPlateFormat(PARAM_TWOROWARMY_ON, &c_Config);
	TH_SetEnabledPlateFormat(PARAM_ARMPOLICE2_ON, &c_Config);
	file_list = fopen("test_list_yuv.txt", "r");
	//file_list = fopen("test_list.txt", "r");
    if(file_list == NULL)
    {
        printf("fail to open test file list\n");
        //return -1;
    }
	/*	
	while(readline(file_list, file_name) != 0)
	{
		
		memset(filenames[i],0, 512);
		memcpy(filenames[i], file_name, strlen(file_name));
		//printf("%s\n",filenames[i]);
		i++;
	}
	nFileCnt = i;
	*/    
	printf("openfile\n");
	//while(readline(file_list, file_name) != 0)
    while(1)
	{
		//srand(clock());
		//j=rand()%nFileCnt;
		//memset(file_name, 0, 64);
		//memcpy(file_name, filenames[j], strlen(filenames[j]));
		//sscanf(file_name, "%[^.]", buf);
		printf("%s ", file_name);
        //fin = fopen(file_name, "rb");
		fin = fopen("E:\\teamwork\\l刘冬\\jpg+原始数据\\jpg+原始数据_1\\3349.txt", "rb");
        if(fin == NULL)
        {
            printf("fail to open %s\n", file_name);
            return -2;
        }
        //fread(pImg, width*height, 2, fin);

		fseek(fin, 555808, 0);
		fread(pImg, width*height, 3, fin);
        fclose(fin);
		nResultNum=6;
		nRet=TH_RecogImage(pImg, width, height,  result, &nResultNum, NULL, &c_Config);
		//TH_CheckMinFreeMemory(&nMinSram, &nMinSdram, &c_Config);
		// printf("SRAM %d SDRAM %d\n", nMinSram, nMinSdram);
		
		printf("ret=%d ", nRet);
        printf("cnt=%d ", nResultNum);
		nResultCnt+=nResultNum;
		/*
		if(strstr(file_name, "Plate") != 0 && nResultNum == 0)
		{
			return -3;
		}
		*/
		for(i=0; i<nResultNum; i++)
		{
			//printf("plate: %s, color: %s\nleft: %d, top: %d, right: %d, bottom: %d %d\n",
			//	result[i].license, result[i].color, result[i].rcLocation.left, result[i].rcLocation.top,
			//	result[i].rcLocation.right, result[i].rcLocation.bottom, result[i].nType);
			printf("plate: %s ", result[i].license);
		}
		printf("\n");
    }
	printf("Total result count: %d\n", nResultCnt);
    fclose(file_list);
    return 0;
}

#endif
