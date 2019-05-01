/*********************************************************************************
 *Copyright(C):        i-eye.ai
 *FileName:            lpr_alg.h
 *Version:             1.0
 *Date:                2019.4.1
 *Description:         算法接口
 *History:
 1. Date:           2019.4.1
 Author：            sxw
 Modification:   首次生成文件
 **********************************************************************************/
#ifndef _LPR_ALG_H
#define _LPR_ALG_H

//车牌识别结果的结构体
typedef struct VehiclePlateResult
{
    char license[16];    // 车牌字符串
    char color[8];        // 车牌颜色
    
    int nColor;            // 车牌颜色
    int nType;            // 车牌类型
    int nConfidence;    // 整牌可信度
    
    //车牌坐标
    int left;
    int top;
    int right;
    int bottom;
    
    int jpeg_len;       //jpg 图片长度
} VPR, *PVPR;
//车牌识别初始化
bool vlpr_init();
//车牌识别，输入为jpg图像指针和长度，输出识别结果，识别结果结构体需要调用方分配
bool vlpr_analyze(const unsigned char *pImage, int len, PVPR pVPR);

#endif
