#include "renderer/ps2gl_renderers/vertex_color_renderer.hpp"
#include "vertex_color_renderer_mem_linear.h"

#include "ps2gl/drawcontext.h"
#include "ps2gl/glcontext.h"
#include "ps2gl/immgmanager.h"
#include "ps2gl/lighting.h"
#include "ps2gl/material.h"
#include "ps2gl/matrix.h"
#include "ps2gl/metrics.h"
#include "ps2gl/texture.h"

#include "ps2gl/renderer.h"

#include "engine.hpp"

extern "C" {
VU_FUNCTIONS(VertexColorRenderer);
}

CVertexColorRenderer::CVertexColorRenderer()
    : CCustomRendererBase(mVsmAddr(VertexColorRenderer), mVsmSize(VertexColorRenderer), 4, 3,
                          kInputStart,
                          kInputBufSize - kInputStart,
                          "Vertex color renderer")
{
	CustomPrimType     = VCRPrimType;
	CustomPrimTypeFlag = VCRPrimTypeFlag;

	Requirements = 0;
	Capabilities = CustomPrimTypeFlag;

	ContextStart    = kContextStart;
	DoubleBufBase   = kDoubleBufBase;
	DoubleBufOffset = kDoubleBufOffset;
	DoubleBufSize   = kDoubleBufSize;
}

CVertexColorRenderer* CVertexColorRenderer::Register()
{
	// create a renderer and register it

	CVertexColorRenderer* renderer = new CVertexColorRenderer;
	pglRegisterRenderer(renderer);

	// register the prim type

	pglRegisterCustomPrimType(VCRPrimType,       // the prim type we will pass to ps2gl (glBegin...)
	                          VCRPrimTypeFlag,   // the corresponding renderer requirement
	                          ~(tU64)0xffffffff, // we only care about the custom stuff (upper 32 bits)
	                          true);             // ok to merge multiple calls when possible

	return renderer;
}

void CVertexColorRenderer::InitContext(GLenum primType, tU32 rcChanges, bool userRcChanged)
{
	primType = GL_TRIANGLE_STRIP;

	CGLContext& glContext        = *pGLContext;
	CVifSCDmaPacket& packet      = glContext.GetVif1Packet();
	CImmDrawContext& drawContext = glContext.GetImmDrawContext();

	packet.Cnt();
	{
		AddVu1RendererContext(packet, primType, kContextStart);

		packet.Mscal(0);
		packet.Flushe();

		packet.Base(kDoubleBufBase);
		packet.Offset(kDoubleBufOffset);
	}
	packet.CloseTag();

	CacheRendererState();
}