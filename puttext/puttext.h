
#ifndef PUT_TEXT_H
#define PUT_TEXT_H

#include <ft2build.h>
#include <freetype/freetype.h>

typedef struct {
	int font_size;  // 字体大小
	int word_space; // 字间距
	int line_space; // 行间距
	const char *type; // 字体类型
	int x;
	int y; // 打字的起始坐标
	char channel[4];
} text_set;

void set_text(const text_set *set);

void puttext(int line, const wchar_t *text, char *img, int width, int height);

#endif

