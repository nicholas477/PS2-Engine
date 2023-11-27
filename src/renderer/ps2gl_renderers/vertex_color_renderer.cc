#include "renderer/ps2gl_renderers/vertex_color_renderer.hpp"
#include "vu1_mem_linear.h"

#define VU_FUNCTIONS(name)        \
	void vsm##name##_CodeStart(); \
	void vsm##name##_CodeEnd()

#define mVsmAddr(name) ((void*)vsm##name##_CodeStart)
#define mVsmSize(name) ((u8*)vsm##name##_CodeEnd - (u8*)vsm##name##_CodeStart)

extern "C" {
VU_FUNCTIONS(VertexColorRenderer);
}

VertexColorRenderer::VertexColorRenderer()
    : CLinearRenderer(mVsmAddr(VertexColorRenderer), mVsmSize(VertexColorRenderer), 4, 3,
                      kInputStart,
                      kInputBufSize - kInputStart,
                      "Vertex color renderer")
{
	Requirements = 0;                // no requirements
	Capabilities = kVCRPrimTypeFlag; // provides the "kVCRPrimType" capability
}

VertexColorRenderer* VertexColorRenderer::Register()
{
	// create a renderer and register it

	VertexColorRenderer* renderer = new VertexColorRenderer;
	pglRegisterRenderer(renderer);

	// register the prim type

	pglRegisterCustomPrimType(kVCRPrimType,      // the prim type we will pass to ps2gl (glBegin...)
	                          kVCRPrimTypeFlag,  // the corresponding renderer requirement
	                          ~(tU64)0xffffffff, // we only care about the custom stuff (upper 32 bits)
	                          true);             // ok to merge multiple calls when possible

	return renderer;
}