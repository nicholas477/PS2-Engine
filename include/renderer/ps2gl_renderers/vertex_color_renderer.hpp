#pragma once

#include "GL/gl.h"
#include "ps2gl/immgmanager.h"
#include "ps2gl/linear_renderer.h"
#include "ps2gl/renderer.h"

static constexpr u64 kVCRPrimType = (((tU64)1 << 32) | GL_TRIANGLE_STRIP);
static constexpr u64 kVCRPrimTypeFlag((tU64)1 << 32);

class CVertexColorRenderer: public CLinearRenderer
{
public:
	CVertexColorRenderer();

	static CVertexColorRenderer* Register();
	//virtual void InitContext(GLenum primType, tU32 rcChanges, bool userRcChanged) override;
	virtual void DrawLinearArrays(CGeometryBlock& block) override;
};