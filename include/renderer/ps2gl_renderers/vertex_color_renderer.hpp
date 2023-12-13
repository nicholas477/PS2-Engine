#pragma once

#include "renderer/ps2gl_renderers/custom_renderer.hpp"

// The 31st bit is the custom primitive flag, 1 is the custom renderer type
static constexpr u32 VCRPrimType     = ((tU32)1 << 31) | 1;
static constexpr u64 VCRPrimTypeFlag = ((tU64)1 << 32);

//#define kVCRPrimType (((tU32)1 << 31) | 1)
//#define kVCRPrimTypeFlag ((tU64)1 << 32)

class CVertexColorRenderer: public CCustomRendererBase
{
public:
	CVertexColorRenderer();
	virtual void InitContext(GLenum primType, tU32 rcChanges, bool userRcChanged) override;
	static CVertexColorRenderer* Register();
};