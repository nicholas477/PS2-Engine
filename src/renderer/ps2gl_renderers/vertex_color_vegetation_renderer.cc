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
VU_FUNCTIONS(VertexColorRenderer);
}

CVertexColorVegetationRenderer::CVertexColorVegetationRenderer()
    : CCustomRendererBase(mVsmAddr(VertexColorRenderer), mVsmSize(VertexColorRenderer), 4, 3,
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

	drawContext.DrawEnv->SetFogColor(128, 128, 128);

	packet.Cnt();
	{
		AddVu1RendererContext(packet, primType, kContextStart);

		// overwrite the giftag built by CBaseRenderer::AddVu1RendererContext()...
		// ..it knows not what it does.
		primType &= 0x7; // convert from GL #define to gs prim number
		bool smoothShading = drawContext.GetDoSmoothShading();
		bool useTexture    = glContext.GetTexManager().GetTexEnabled();
		bool alpha         = drawContext.GetBlendEnabled();
		unsigned int nreg  = OutputQuadsPerVert;

		GS::tPrim prim = {prim_type : primType, iip : smoothShading, tme : useTexture, fge : 1, abe : alpha, aa1 : 0, fst : 0, ctxt : 0, fix : 0};
		tGifTag giftag = {NLOOP : 0, EOP : 1, pad0 : 0, id : 0, PRE : 1, PRIM : *(tU64*)&prim, FLG : 0, NREG : nreg, REGS0 : 2, REGS1 : 1, REGS2 : 4};

		packet.Pad96();

		packet.OpenUnpack(Vifs::UnpackModes::s_32, kIsVegetation, Packet::kSingleBuff);
		packet += (s32)1;
		packet.CloseUnpack(1);

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