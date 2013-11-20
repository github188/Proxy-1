
#include <turbojpeg.h>
#include <stdio.h>

/* img_decode : JPEG解码，使用turbojpeg库
 * [out] bmp: 用来存放解码后的RGB数据， w,h 用来存放解码后图像宽\高
 * [in] jpeg, length : jpeg图像的数据和长度
 */
inline int img_decode(unsigned char* bmp, int* wt, int* ht, unsigned char* jpeg, unsigned int length)
{
	int subSamp;
	int ret;
	
	tjhandle h = tjInitDecompress();

	ret = tjDecompressHeader2( h, (unsigned char*)jpeg, length, wt, ht, &subSamp);
	if (ret != 0) {
		printf("Decode jpeg header Error: %s\n", tjGetErrorStr());
		return 1;
	}

	ret = tjDecompress2( h, (unsigned char*)jpeg, length, (unsigned char*)bmp, *wt,
			*wt * tjPixelSize[TJPF_BGR], *ht, TJPF_RGB, TJFLAG_NOREALLOC);
	
	if (ret != 0) {
		printf("Decode Error: %s\n", tjGetErrorStr());
		return 1;
	}

	tjDestroy(h);

	return 0;
}

/* JPEG压缩
 *
 */
inline void img_encode(unsigned char *img_buf, int wt, int ht, unsigned char *jpeg_buffer,
		unsigned long jpeg_buffer_size, unsigned long *jpeg_comp_size, int quality)
{
	tjhandle h = tjInitCompress();

	tjCompress2(h, (unsigned char*)img_buf, wt, wt * tjPixelSize[TJPF_RGB], ht,
			TJPF_RGB, &jpeg_buffer, jpeg_comp_size, TJSAMP_444, quality, TJFLAG_NOREALLOC);

    tjDestroy(h);
}




