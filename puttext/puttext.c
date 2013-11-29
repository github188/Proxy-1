
/*
 * //TODO 并发??
 */
#include "puttext.h"
#include <ctype.h>
#include <locale.h>

static text_set local_set = {
	32, 2, 20, "/usr/local/bin/simsun.ttc", 50, 50,
	{(char)0xFF, (char)0x00, (char)0x00, (char)0x00} };

static FT_Library g_lib;
static FT_Face g_face;

static char g_libraryok = 0;

static void free_freetype_library() 
{
	if(g_libraryok) {
		FT_Done_Face(g_face);
		FT_Done_FreeType(g_lib);
		g_libraryok = 0;
	}
}

static void init_freetype_library(const text_set *set)
{
	FT_Init_FreeType(&g_lib);
	FT_New_Face(g_lib, set->type, 0, &g_face);
	setlocale(LC_ALL,"zh_CN.UTF-8");

	g_libraryok = 1;
}

void set_text(const text_set *set)
{
	/* 需要重新初始化库,重新加载字体文件 */
	if( strcmp(local_set.type, set->type) != 0 ) {
		free_freetype_library();
		init_freetype_library(set);
	}
	local_set = *set;
}

void puttext( int line, const wchar_t *text, char *img, int width, int height)
{

	if(!g_libraryok) init_freetype_library(&local_set);
	FT_Set_Pixel_Sizes(g_face, local_set.font_size, 0);

	int index;
	int c, r;   // 用来计算打字的位置

	int count_half = 0;
	for(index = 0; text[index] != '\0'; index++) {

		wchar_t wc = text[index];

		FT_UInt idx = FT_Get_Char_Index(g_face, wc);
		FT_Load_Glyph(g_face, idx, FT_LOAD_DEFAULT);
		FT_Render_Glyph(g_face->glyph, FT_RENDER_MODE_MONO);

		FT_GlyphSlot slot = g_face->glyph;
		int top = slot->bitmap_top;
		int rows = slot->bitmap.rows;
		int cols = slot->bitmap.width;

		int i, j;
		for(i = 0; i < rows; i++) {
			for(j = 0; j < cols; j++) {
				int off = i * slot->bitmap.pitch + j/8;
				if( slot->bitmap.buffer[off] & (0xC0 >> (j%8)) ) {
					
					c = local_set.x + j + index * (local_set.font_size + local_set.word_space);
					c -= count_half * (local_set.font_size ) / 2; // + local_set.word_space) / 2;
					r = local_set.y + i - top + 
						(line - 1) * (local_set.font_size + local_set.line_space);

					if(r >= 0 && r < height && c >= 0 && c < width) {
						// 将坐标r,c处像素值改为某颜色
						char *rgb = img + (width * 3) * r + c * 3;
						*rgb = local_set.channel[0];
						*(rgb + 1) = local_set.channel[1];
						*(rgb + 2) = local_set.channel[2];
					} else {
						printf("out of bound.\n");
					}
					//printf("*");
				} else {
					//printf("-");
				}
			}
			//printf("\n");
		}

		if( isascii(wc) ) {
			count_half ++;
		}
	}
}

