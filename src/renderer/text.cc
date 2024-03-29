/*	  Copyright (C) 2000,2001,2002  Sony Computer Entertainment America
       	  
       	  This file is subject to the terms and conditions of the GNU Lesser
	  General Public License Version 2.1. See the file "COPYING" in the
	  main directory of this archive for more details.                             */

#include <malloc.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "GL/gl.h"
#include "GL/glut.h" // for pglutAllocDmaMem
#include "GL/ps2gl.h"

#include "ps2s/gs.h"

#include "renderer/gs.hpp"
#include "renderer/text.hpp"

#include <utility>

float q_width = 127, q_height = 255.5f;
unsigned int font_texture = 0xffffffff;
unsigned int* font_clut;
float left_margin = 20, top_margin = 20;
float cursor_x = left_margin, cursor_y = top_margin;

void tsDrawString(float x, float y, const char* text)
{
	glPolygonMode(GL_FRONT, GL_FILL);
	glDisable(GL_ALPHA_TEST);

	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glLoadIdentity();

	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadIdentity();
	glOrtho(0, GS::get_screen_res().x, GS::get_screen_res().y, 0, 0, 10);

	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, font_texture);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
	glColorTable(GL_COLOR_TABLE, GL_RGBA, 256, GL_RGBA, GL_UNSIGNED_INT_8_8_8_8,
	             font_clut);

	glDisable(GL_LIGHTING);
	glEnable(GL_BLEND);

	float tex_u = 0, tex_v = 0,
	      cell_width  = 8.0f / 128.0f,
	      cell_height = 16.0f / 256.0f;

	glBegin(GL_QUADS);
	{
		IntVector2 cursor = {0, 0};
		for (int i = 0; i < (int)strlen(text); i++)
		{
			if (text[i] == '\n')
			{
				cursor.x = 0;
				cursor.y += 16;
				continue;
			}

			tex_u = ((unsigned int)text[i] % 16) * cell_width;
			tex_v = ((unsigned int)text[i] / 16) * cell_height;

			glTexCoord2f(tex_u, tex_v);
			glVertex3f(x + cursor.x, y + cursor.y, -1);

			glTexCoord2f(tex_u, tex_v + cell_height);
			glVertex3f(x + cursor.x, y + cursor.y + 16, -1);

			glTexCoord2f(tex_u + cell_width, tex_v + cell_height);
			glVertex3f(x + cursor.x + 8, y + cursor.y + 16, -1);

			glTexCoord2f(tex_u + cell_width, tex_v);
			glVertex3f(x + cursor.x + 8, y + cursor.y, -1);

			cursor.x += 8;
		}
	}
	glEnd();

	glPopMatrix();
	glMatrixMode(GL_MODELVIEW);
	glPopMatrix();

	glDisable(GL_BLEND);
	glEnable(GL_LIGHTING);
	glDisable(GL_TEXTURE_2D);
}

extern unsigned char glut_font_image[128 * 256];

void tsLoadFont()
{
	int font_image_size = 128 * 256;
	void* font_image    = memalign(16, font_image_size); //pglutAllocDmaMem(font_image_size);

	memcpy(font_image, glut_font_image, font_image_size);

	glGenTextures(1, &font_texture);
	glBindTexture(GL_TEXTURE_2D, font_texture);
	glTexImage2D(GL_TEXTURE_2D, 0, 3,
	             128, 256, 0,
	             GL_COLOR_INDEX, GL_UNSIGNED_BYTE, font_image);

	font_clut = (unsigned int*)memalign(16, sizeof(int) * 256); //(unsigned int*)pglutAllocDmaMem(sizeof(int) * 256);
	memset(font_clut, 0, sizeof(int) * 256);
	font_clut[1] = 0x808080ff;
}
