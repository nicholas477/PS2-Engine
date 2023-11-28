#include "renderer/ps2gl_renderers/vertex_color_renderer.hpp"
#include "vu1_mem_linear.h"

#include "ps2gl/drawcontext.h"
#include "ps2gl/glcontext.h"
#include "ps2gl/immgmanager.h"
#include "ps2gl/lighting.h"
#include "ps2gl/material.h"
#include "ps2gl/matrix.h"
#include "ps2gl/metrics.h"
#include "ps2gl/texture.h"

#include "ps2gl/renderer.h"

#define VU_FUNCTIONS(name)        \
	void vsm##name##_CodeStart(); \
	void vsm##name##_CodeEnd()

#define mVsmAddr(name) ((void*)vsm##name##_CodeStart)
#define mVsmSize(name) ((u8*)vsm##name##_CodeEnd - (u8*)vsm##name##_CodeStart)

extern "C" {
VU_FUNCTIONS(VertexColorRenderer);
}

using namespace RendererProps;

static constexpr CRendererProps capabilities = {
	PrimType : kPtsLinesStripsFans,
	Lighting : 1,
	NumDirLights : k3DirLights | k8DirLights,
	NumPtLights : k1PtLight | k2PtLights | k8PtLights,
	Texture : 1,
	Specular : 1,
	PerVtxMaterial : kDiffuse,
	Clipping : kNonClipped | kClipped,
	CullFace : 1,
	TwoSidedLighting : 0,
	ArrayAccess : kLinear
};

static constexpr CRendererProps no_reqs = static_cast<CRendererProps>(0LL);
static_assert(sizeof(0LL) == sizeof(CRendererProps));

CVertexColorRenderer::CVertexColorRenderer()
    : CLinearRenderer(mVsmAddr(VertexColorRenderer), mVsmSize(VertexColorRenderer), 4, 3,
                      kInputStart,
                      kInputBufSize - kInputStart,
                      "Vertex color renderer")
{
	Requirements = 0;
	Capabilities = kVCRPrimTypeFlag;
}

CVertexColorRenderer* CVertexColorRenderer::Register()
{
	// create a renderer and register it

	CVertexColorRenderer* renderer = new CVertexColorRenderer;
	pglRegisterRenderer(renderer);

	// register the prim type

	pglRegisterCustomPrimType(kVCRPrimType,      // the prim type we will pass to ps2gl (glBegin...)
	                          kVCRPrimTypeFlag,  // the corresponding renderer requirement
	                          ~(tU64)0xffffffff, // we only care about the custom stuff (upper 32 bits)
	                          true);             // ok to merge multiple calls when possible

	return renderer;
}

void CVertexColorRenderer::InitContext(GLenum primType, tU32 rcChanges, bool userRcChanged)
{
	primType = GL_TRIANGLE_STRIP;

	CLinearRenderer::InitContext(primType, rcChanges, userRcChanged);
	return;

	CGLContext& glContext        = *pGLContext;
	CVifSCDmaPacket& packet      = glContext.GetVif1Packet();
	CImmDrawContext& drawContext = glContext.GetImmDrawContext();

	packet.Cnt();
	{
		AddVu1RendererContext(packet, primType, kContextStart);

		// overwrite the giftag built by CBaseRenderer::AddVu1RendererContext()...
		// ..it knows not what it does.
		primType &= 0x7; // convert from GL #define to gs prim number
		CImmDrawContext& drawContext = glContext.GetImmDrawContext();
		bool smoothShading           = drawContext.GetDoSmoothShading();
		bool useTexture              = glContext.GetTexManager().GetTexEnabled();
		bool alpha                   = drawContext.GetBlendEnabled();
		unsigned int nreg            = OutputQuadsPerVert;

		GS::tPrim prim = {prim_type : primType, iip : smoothShading, tme : useTexture, fge : 0, abe : alpha, aa1 : 0, fst : 0, ctxt : 0, fix : 0};
		tGifTag giftag = {NLOOP : 0, EOP : 1, pad0 : 0, id : 0, PRE : 1, PRIM : *(tU64*)&prim, FLG : 0, NREG : nreg, REGS0 : 2, REGS1 : 1, REGS2 : 4};

		packet.Pad96();
		packet.OpenUnpack(Vifs::UnpackModes::v4_32, kGifTag, Packet::kSingleBuff);
		packet += giftag;
		packet.CloseUnpack(1);

		packet.Mscal(0);
		packet.Flushe();

		packet.Base(kDoubleBufBase);
		packet.Offset(kDoubleBufOffset);
	}
	packet.CloseTag();

	CacheRendererState();
}

void CVertexColorRenderer::DrawLinearArrays(CGeometryBlock& block)
{
	block.SetNumVertsPerPrim(2);
	block.SetNumVertsToRestartStrip(2);

	int wordsPerVert   = block.GetWordsPerVertex();
	int wordsPerNormal = (block.GetNormalsAreValid()) ? block.GetWordsPerNormal() : 0;
	int wordsPerTex    = (block.GetTexCoordsAreValid()) ? block.GetWordsPerTexCoord() : 0;
	int wordsPerColor  = (block.GetColorsAreValid()) ? block.GetWordsPerColor() : 0;

	CVifSCDmaPacket& packet = pGLContext->GetVif1Packet();
	InitXferBlock(packet, wordsPerVert, wordsPerNormal, wordsPerTex, wordsPerColor);

	// get max number of vertices per vu1 buffer

	// let's assume separate unpacks for vertices, normals, tex uvs..
	int maxUnpackVerts = 256;
	// max number of vertex data in vu1 memory
	int vu1QuadsPerVert      = InputQuadsPerVert;
	int inputBufSize         = InputGeomBufSize;
	int maxVu1VertsPerBuffer = inputBufSize / vu1QuadsPerVert;
	// max vertices per buffer
	int maxVertsPerBuffer = std::min(maxUnpackVerts, maxVu1VertsPerBuffer);
	maxVertsPerBuffer -= 3;
	// make sure we don't end in the middle of a polygon
	maxVertsPerBuffer -= maxVertsPerBuffer % block.GetNumVertsPerPrim();

	// draw

	DrawBlock(packet, block, maxVertsPerBuffer);
}