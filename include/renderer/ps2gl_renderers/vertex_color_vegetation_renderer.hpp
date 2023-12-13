#pragma once

#include "GL/gl.h"
#include "ps2gl/immgmanager.h"
#include "ps2gl/linear_renderer.h"
#include "ps2gl/renderer.h"
#include "renderer/ps2gl_renderers/custom_renderer.hpp"

// The 31st bit is the custom primitive flag, 1 is the custom renderer type
static constexpr u32 VCVRPrimType     = ((tU32)1 << 31) | 2;
static constexpr u64 VCVRPrimTypeFlag = ((tU64)1 << 33);

//#define kVCRPrimType (((tU32)1 << 31) | 1)
//#define kVCRPrimTypeFlag ((tU64)1 << 32)

class CVertexColorVegetationRenderer: public CCustomRendererBase
{
public:
	CVertexColorVegetationRenderer();

	static CVertexColorVegetationRenderer* Register();
	virtual void InitContext(GLenum primType, tU32 rcChanges, bool userRcChanged) override;
};