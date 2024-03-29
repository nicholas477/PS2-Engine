#include "renderer/ps2gl_renderers/vertex_color_vegetation_renderer.hpp"
#include "vertex_color_renderer_mem_linear.h"

#include "ps2gl/drawcontext.h"
#include "ps2gl/glcontext.h"
#include "ps2gl/immgmanager.h"
#include "ps2gl/lighting.h"
#include "ps2gl/material.h"
#include "ps2gl/matrix.h"
#include "ps2gl/metrics.h"
#include "ps2gl/texture.h"
#include "ps2s/drawenv.h"

#include "ps2gl/renderer.h"

#include "engine.hpp"

extern "C" {
VU_FUNCTIONS(VertexColorVegetationRenderer);
}

CVertexColorVegetationRenderer::CVertexColorVegetationRenderer()
    : CCustomRendererBase(mVsmAddr(VertexColorVegetationRenderer), mVsmSize(VertexColorVegetationRenderer), 4, 3,
                          kInputStart,
                          kInputBufSize - kInputStart,
                          "Vertex color vegetation renderer")
{
	CustomPrimType     = VCVRPrimType;
	CustomPrimTypeFlag = VCVRPrimTypeFlag;

	Requirements = 0;
	Capabilities = CustomPrimTypeFlag;

	ContextStart    = kContextStart;
	DoubleBufBase   = kDoubleBufBase;
	DoubleBufOffset = kDoubleBufOffset;
	DoubleBufSize   = kDoubleBufSize;
}

CVertexColorVegetationRenderer* CVertexColorVegetationRenderer::Register()
{
	// create a renderer and register it

	CVertexColorVegetationRenderer* renderer = new CVertexColorVegetationRenderer;
	pglRegisterRenderer(renderer);

	// register the prim type

	pglRegisterCustomPrimType(VCVRPrimType,      // the prim type we will pass to ps2gl (glBegin...)
	                          VCVRPrimTypeFlag,  // the corresponding renderer requirement
	                          ~(tU64)0xffffffff, // we only care about the custom stuff (upper 32 bits)
	                          true);             // ok to merge multiple calls when possible

	return renderer;
}

static float get_veg_p(float offset)
{
	return sinf(Engine::get_game_time() + offset);
}

void CVertexColorVegetationRenderer::InitContext(GLenum primType, tU32 rcChanges, bool userRcChanged)
{
	primType = GL_TRIANGLE_STRIP;

	CGLContext& glContext        = *pGLContext;
	CVifSCDmaPacket& packet      = glContext.GetVif1Packet();
	CImmDrawContext& drawContext = glContext.GetImmDrawContext();

	packet.Cnt();
	{
		AddVu1RendererContext(packet, primType, kContextStart);

		packet.Pad96();

		packet.OpenUnpack(Vifs::UnpackModes::s_32, kVegetationParams, Packet::kSingleBuff);
		packet += get_veg_p(0.f);
		packet += get_veg_p(M_PI / 6.f);
		packet += get_veg_p((2.f * M_PI) / 6.f);
		packet += get_veg_p((3.f * M_PI) / 6.f);
		packet += get_veg_p((4.f * M_PI) / 6.f);
		packet += get_veg_p((5.f * M_PI) / 6.f);
		packet.CloseUnpack(6);

		packet.Mscal(0);
		packet.Flushe();

		packet.Base(kDoubleBufBase);
		packet.Offset(kDoubleBufOffset);

		packet.Pad128();
	}
	packet.CloseTag();

	CacheRendererState();
}