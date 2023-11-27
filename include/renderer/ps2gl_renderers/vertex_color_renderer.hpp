#pragma once

#include "GL/gl.h"
#include "ps2gl/immgmanager.h"
#include "ps2gl/linear_renderer.h"
#include "ps2gl/renderer.h"

#define kVCRPrimType (((tU32)1 << 31) | 1)
#define kVCRPrimTypeFlag ((tU64)1 << 32)

class VertexColorRenderer: public CLinearRenderer
{
public:
	VertexColorRenderer();

	static VertexColorRenderer* Register();
};