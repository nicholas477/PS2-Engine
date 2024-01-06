#include "renderer/helpers.hpp"
#include "renderer/gs.hpp"

#include "GL/gl.h"
#include "GL/glut.h" // for pglutAllocDmaMem
#include "GL/ps2gl.h"

namespace GS::Helpers
{
void draw_2d_quad(const Vector& quad_rect, const Colors::Color& color)
{
	const Vector color_vector = color.to_vector();
	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glLoadIdentity();

	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadIdentity();
	glOrtho(0, GS::get_screen_res().x, GS::get_screen_res().y, 0, 0, 10);

	glDisable(GL_TEXTURE_2D);
	glEnable(GL_COLOR_MATERIAL);
	glColorMaterial(GL_FRONT_AND_BACK, GL_DIFFUSE);
	glDisable(GL_LIGHTING);
	glEnable(GL_BLEND);

	glBegin(GL_QUADS);
	{
		glColor4fv((const GLfloat*)&color_vector);
		glTexCoord2f(0.f, 0.f);
		glVertex3f(quad_rect.x, quad_rect.y, -1);

		glColor4fv((const GLfloat*)&color_vector);
		glTexCoord2f(0.f, 1.f);
		glVertex3f(quad_rect.x, quad_rect.y + quad_rect.w, -1);

		glColor4fv((const GLfloat*)&color_vector);
		glTexCoord2f(1.f, 1.f);
		glVertex3f(quad_rect.x + quad_rect.z, quad_rect.y + quad_rect.w, -1);

		glColor4fv((const GLfloat*)&color_vector);
		glTexCoord2f(1.f, 0.f);
		glVertex3f(quad_rect.x + quad_rect.z, quad_rect.y, -1);
	}
	glEnd();

	glPopMatrix();
	glMatrixMode(GL_MODELVIEW);
	glPopMatrix();

	glDisable(GL_BLEND);
	glEnable(GL_LIGHTING);
}
} // namespace GS::Helpers