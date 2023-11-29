#pragma once

#include "GL/gl.h"
#include "ps2gl/immgmanager.h"
#include "ps2gl/linear_renderer.h"
#include "ps2gl/renderer.h"

// The 31st bit is the custom primitive flag, 1 is the custom renderer type
static constexpr u32 kVCRPrimType     = ((tU32)1 << 31) | 1;
static constexpr u64 kVCRPrimTypeFlag = ((tU64)1 << 32);

//#define kVCRPrimType (((tU32)1 << 31) | 1)
//#define kVCRPrimTypeFlag ((tU64)1 << 32)

class CVertexColorRenderer: public CLinearRenderer
{
public:
	CVertexColorRenderer();

	static CVertexColorRenderer* Register();
	virtual void InitContext(GLenum primType, tU32 rcChanges, bool userRcChanged) override;
	//virtual bool GetCachePackets(const CGeometryBlock& geometry) { return true; };
	virtual void DrawLinearArrays(CGeometryBlock& block) override;
};