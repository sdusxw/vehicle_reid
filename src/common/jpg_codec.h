/*********************************************************************************
  *Copyright(C):        i-eye.ai
  *FileName:            jpg_codec.h
  *Version:             2.71828
  *Date:                2017.11.04
  *Description:         图片JPG压缩、解压缩类
  *History:
     1. Date:           2017.10.15
     	Author：			Diao Guangqiang
        Modification:   首次生成文件
     2. Date:
        Author:			sxw@sdboon.com
        Modification:	修改类名，去掉命名空间
     3. Date:           2017.12.02
        Author:         sxw@sdboon.com
        Modification:   增加异常处理
    3. Date:           2019.05.01
        Author:         sxw@sdboon.com
        Modification:   修改解码参数，返回width和height
**********************************************************************************/
#ifndef _JPEG_CODEC_H
#define _JPEG_CODEC_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <iostream>

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#define Jpegargu_FILES	"../conf/jpeg.txt"

extern "C"
{
#include "jpeglib.h"
}

typedef struct boon_jpeg_info
{
    int jpeg_size;
    char *jpeg_data;
    boon_jpeg_info()
    {
        jpeg_size = 0;
        jpeg_data = NULL;
    }
    boon_jpeg_info(int size, char *ptr)
    {
        jpeg_size = size;
        jpeg_data = ptr;
    }
} BoonJpegInfo;

class BoonJpegConf
{
public:
    int quality;
};

class BoonJpegCodec
{
public:
    int Jpeg_conf_quality;
    bool JpegCompress(int w, int h, const char * rgb_data, int rgb_size,
                      char * jpeg_data, int *jpeg_size, int channel = 3);
    bool JpegUnCompress(const char * jpeg_data, int jpeg_size, char *rgb_data,
                        int rgb_size, int &w, int &h, int &c);
    bool init();
    bool Jpegarg_read(BoonJpegConf &argu);
};

/* jpeg图像处理异常类 */
class jpeg_mem_exception:public std::logic_error{
public:
    // 继承基类构造函数
    using std::logic_error::logic_error;
};

#endif
