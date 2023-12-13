#pragma once

#include "ps2gl/renderer.h"
#include "ps2gl/linear_renderer.h"

#define VU_FUNCTIONS(name)        \
	void vsm##name##_CodeStart(); \
	void vsm##name##_CodeEnd()

#define mVsmAddr(name) ((void*)vsm##name##_CodeStart)
#define mVsmSize(name) ((u8*)vsm##name##_CodeEnd - (u8*)vsm##name##_CodeStart)

static constexpr CRendererProps no_reqs = static_cast<CRendererProps>(0LL);
static_assert(sizeof(0LL) == sizeof(CRendererProps));

// The 31st bit is the custom primitive flag, 1 is the custom renderer type
// static constexpr u32 kVCRPrimType     = ((tU32)1 << 31) | 1;
// static constexpr u64 kVCRPrimTypeFlag = ((tU64)1 << 32);

class CCustomRendererBase: public CLinearRenderer
{
public:
	CCustomRendererBase(void* packet, int packetSize,
	                    int inQuadsPerVert, int outQuadsPerVert,
	                    int inGeomOffset, int inGeomBufSize,
	                    const char* name);

	virtual void InitContext(GLenum primType, tU32 rcChanges, bool userRcChanged) override;
	virtual void DrawLinearArrays(CGeometryBlock& block) override;

	// This is the prim type passed to the gs
	u32 PrimType           = 0x7; // 0x7 = triangle strip
	u32 CustomPrimType     = 0;
	u64 CustomPrimTypeFlag = 0;

	u32 ContextStart    = 0;
	u32 DoubleBufBase   = 0;
	u32 DoubleBufOffset = 0;
	u32 DoubleBufSize   = 0;
};
