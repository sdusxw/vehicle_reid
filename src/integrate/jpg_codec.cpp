#include "jpg_codec.h"

#include <iostream>
#include <fstream>

using namespace std;

/* 自定义jpeg图像压缩/解压缩过程中错误退出函数 */
METHODDEF(void) jpeg_mem_error_exit(j_common_ptr cinfo)
{
	// 调用 format_message 生成错误信息
	char err_msg[JMSG_LENGTH_MAX];
	(*cinfo->err->format_message)(cinfo, err_msg);
	// 抛出c++异常
	throw jpeg_mem_exception(err_msg);
}

BoonJpegConf Jpeg_conf;
void BoonJpegInitDestination(j_compress_ptr cinfo)
{
}

static boolean BoonJpegEmptyOutputBuffer(j_compress_ptr cinfo)
{
	return TRUE;
}

static void BoonJpegTermDestination(j_compress_ptr cinfo)
{
	//    jpegDstDataLen = jpegDstBufferLen - jpegDstManager.free_in_buffer;
}

/**
 Raw Rgb Data converted to Jpeg data
 */
bool BoonJpegCodec::init()
{
	if (!Jpegarg_read(Jpeg_conf))
	{
		Jpeg_conf.quality = 90;
	}
	Jpeg_conf_quality = Jpeg_conf.quality;
	cout << " Jpeg_conf_quality " << Jpeg_conf_quality << endl;
	return true;
}
bool BoonJpegCodec::Jpegarg_read(BoonJpegConf &argu)
{
	fstream cfg_files;
	cfg_files.open(Jpegargu_FILES);
	if (!cfg_files.is_open())
	{
		cout << "Error: Open jpeg argu file " << Jpegargu_FILES << endl;
		return false;
	}
	char tmp[256];
	while (!cfg_files.eof())
	{

		char tmp[256] = "";
		cfg_files.getline(tmp, 256);
		string line(tmp);
		//cout << "READ\t" << line << endl;

		if (line.length() > 0)
		{
			size_t pos = line.find('=');
			string tmp_key = line.substr(0, pos);
			if (tmp_key == "quality")
			{
				argu.quality = atoi((line.substr(pos + 1)).c_str());
			}
		}
	}
	return true;
}
bool BoonJpegCodec::JpegCompress(int w, int h, const char * rgb_data,
		int rgb_size, char * jpeg_data, int *jpeg_size, int channel)
{
	//cout<<"JpegCompress in"<<endl;
	struct jpeg_compress_struct cinfo;
	struct jpeg_error_mgr jerr;
	struct jpeg_destination_mgr jpegDstManager;
	int ret;

	unsigned char *srcBuf = new unsigned char[w * 3];
	JSAMPROW rowPointer[1];
	rowPointer[0] = (JSAMPROW) srcBuf;
	int left_size;
	left_size = *jpeg_size;
	cinfo.err = jpeg_std_error(&jerr);
	jpeg_create_compress(&cinfo);

	cinfo.image_width = w;
	cinfo.image_height = h;
	if (channel == 3)
	{
		cinfo.input_components = 3;
		cinfo.in_color_space = JCS_EXT_BGR;
	}
	else if (channel == 1)
	{
		cinfo.input_components = 1;
		cinfo.in_color_space = JCS_GRAYSCALE;
	}
	else
	{
		// todo add by diaoguangqiang 2017/09/21
		free(srcBuf);
		srcBuf = NULL;

		//cout<<"JpegCompress out" << __LINE__ <<endl;
		return false;
	}
	//cout<<"jpegDstManager before "<< __LINE__ << endl;
	cinfo.raw_data_in = true;
	jpeg_set_defaults(&cinfo);

	cinfo.dest = &jpegDstManager;

	jpegDstManager.next_output_byte = (unsigned char*) jpeg_data;
	jpegDstManager.free_in_buffer = left_size;
	jpegDstManager.init_destination = BoonJpegInitDestination;
	jpegDstManager.empty_output_buffer = BoonJpegEmptyOutputBuffer;
	jpegDstManager.term_destination = BoonJpegTermDestination;

	jpeg_set_quality(&cinfo, Jpeg_conf_quality, TRUE);

	jpeg_start_compress(&cinfo, TRUE);
	//cout<<"jpeg_start_compress after"<<endl;
	for (int y = 0; y < h; y++)
	{
		rowPointer[0] = (unsigned char*) (rgb_data + y * w * channel);

		ret = jpeg_write_scanlines(&cinfo, rowPointer, 1);

	}
	//cout<<"jpeg_start_compress after"<< __LINE__ <<endl;
	jpeg_finish_compress(&cinfo);
	jpeg_destroy_compress(&cinfo);
	*jpeg_size = left_size - jpegDstManager.free_in_buffer;
	free(srcBuf);
	srcBuf = NULL;
	//cout<<"JpegCompress out"<<endl;
	return true;
}

void BoonJpegInitSource(j_decompress_ptr cinfo)
{

}

boolean BoonJpegFillInputBuffer(j_decompress_ptr cinfo)
{
	/*
	 jpegError = true;
	 jpegSrcManager.bytes_in_buffer = jpegBufferLen;
	 jpegSrcManager.next_input_byte = (JOCTET *)jpegBufferPtr;    */
	return TRUE;
}

void BoonJpegSkipInputData(j_decompress_ptr cinfo, long num_bytes)
{/*
 if (num_bytes < 0 || (size_t)num_bytes > jpegSrcManager.bytes_in_buffer) {
 jpegError = true;
 jpegSrcManager.bytes_in_buffer = jpegBufferLen;
 jpegSrcManager.next_input_byte = (JOCTET *)jpegBufferPtr;
 } else {
 jpegSrcManager.next_input_byte += (size_t) num_bytes;
 jpegSrcManager.bytes_in_buffer -= (size_t) num_bytes;
 }*/
}

void BoonJpegTermSource(j_decompress_ptr cinfo)
{
	/* No work necessary here. */
}

#define my_error_ptr (void*)
/*
 METHODDEF(void) my_emit_message(j_common_ptr cinfo, int msg_level)
 {
 my_error_ptr myerr = (my_error_ptr) cinfo->err;
 if(msg_level == -1) longjmp(myerr->setjmp_buffer, 1);
 }
 */
bool BoonJpegCodec::JpegUnCompress(const char * jpeg_data, int jpeg_size,
		char *rgb_data, int rgb_size, int &w, int &h, int &c)
{
	bool b_ret = false;
	struct jpeg_decompress_struct cinfo;
	struct jpeg_error_mgr jerr;
	struct jpeg_source_mgr jpegSrcManager;
	int ret;
	JSAMPROW rowPointer[1];
	cinfo.err = jpeg_std_error(&jerr);
	jpeg_create_decompress(&cinfo);

	// 设置自定义的错误处理函数
	jerr.error_exit = jpeg_mem_error_exit;

	jpegSrcManager.init_source = BoonJpegInitSource;
	jpegSrcManager.fill_input_buffer = BoonJpegFillInputBuffer;
	jpegSrcManager.skip_input_data = BoonJpegSkipInputData;
	jpegSrcManager.resync_to_restart = jpeg_resync_to_restart;
	jpegSrcManager.term_source = BoonJpegTermSource;
	jpegSrcManager.next_input_byte = (unsigned char*) jpeg_data;
	jpegSrcManager.bytes_in_buffer = jpeg_size;
	cinfo.src = &jpegSrcManager;

	try
	{
		jpeg_mem_src(&cinfo, (unsigned char *) jpeg_data, jpeg_size);
		jpeg_read_header(&cinfo, TRUE);
		if (c == 3)
			cinfo.out_color_space = /*JCS_EXT_BGR;//*/JCS_RGB;
		else if (c == 1)
			cinfo.out_color_space = JCS_GRAYSCALE;
		jpeg_start_decompress(&cinfo);
        w = cinfo.output_width;
        h = cinfo.output_height;
		/*if (cinfo.output_width != (unsigned int) w
				&& cinfo.output_height != (unsigned int) h)
		{
			jpeg_destroy_decompress(&cinfo);
			return false;
		}*/
		for (int dy = 0; cinfo.output_scanline < cinfo.output_height; dy++)
		{
			rowPointer[0] = (unsigned char *) (rgb_data + w * dy * c);
			ret = jpeg_read_scanlines(&cinfo, rowPointer, 1);
		}
		jpeg_finish_decompress(&cinfo);
		jpeg_destroy_decompress(&cinfo);
		b_ret = true;
	}
	catch (jpeg_mem_exception& e)
	{
		// 处理压缩过程中抛出的异常
		jpeg_destroy_decompress(&cinfo);
		std::cout << "fuck jpeg error" << std::endl;
		b_ret = false;
	}
	return b_ret;
}
