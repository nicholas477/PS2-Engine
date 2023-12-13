#include "renderer/ps2gl_renderers/custom_renderer.hpp"

#include "ps2s/cpu_matrix.h"
#include "ps2s/math.h"
#include "ps2s/packet.h"

#include "ps2gl/drawcontext.h"
#include "ps2gl/glcontext.h"
#include "ps2gl/immgmanager.h"
#include "ps2gl/lighting.h"
#include "ps2gl/linear_renderer.h"
#include "ps2gl/material.h"
#include "ps2gl/matrix.h"
#include "ps2gl/metrics.h"
#include "ps2gl/texture.h"

CCustomRendererBase::CCustomRendererBase(void* packet, int packetSize,
                                         int inQuadsPerVert, int outQuadsPerVert,
                                         int inGeomOffset, int inGeomBufSize,
                                         const char* name)
    : CLinearRenderer(packet, packetSize, inQuadsPerVert, outQuadsPerVert,
                      inGeomOffset,
                      inGeomBufSize,
                      name)
{
	Requirements = 0;
	Capabilities = CustomPrimTypeFlag;
}

void CCustomRendererBase::InitContext(GLenum, tU32 rcChanges, bool userRcChanged)
{
	CGLContext& glContext        = *pGLContext;
	CVifSCDmaPacket& packet      = glContext.GetVif1Packet();
	CImmDrawContext& drawContext = glContext.GetImmDrawContext();

	packet.Cnt();
	{
		AddVu1RendererContext(packet, PrimType, ContextStart); // kContextStart

		// overwrite the giftag built by CBaseRenderer::AddVu1RendererContext()...
		// ..it knows not what it does.
		CImmDrawContext& drawContext = glContext.GetImmDrawContext();
		bool smoothShading           = drawContext.GetDoSmoothShading();
		bool useTexture              = glContext.GetTexManager().GetTexEnabled();
		bool alpha                   = drawContext.GetBlendEnabled();
		unsigned int nreg            = OutputQuadsPerVert;

		GS::tPrim prim = {prim_type : PrimType, iip : smoothShading, tme : useTexture, fge : 0, abe : alpha, aa1 : 0, fst : 0, ctxt : 0, fix : 0};
		tGifTag giftag = {NLOOP : 0, EOP : 1, pad0 : 0, id : 0, PRE : 1, PRIM : *(tU64*)&prim, FLG : 0, NREG : nreg, REGS0 : 2, REGS1 : 1, REGS2 : 4};

		// packet.Pad96();

		// packet.OpenUnpack(Vifs::UnpackModes::s_32, kTime, Packet::kSingleBuff);
		// packet += std::sin(Engine::get_game_time()) * 4.f;
		// packet.CloseUnpack(1);

		packet.Mscal(0);
		packet.Flushe();

		packet.Base(DoubleBufBase);
		packet.Offset(DoubleBufOffset);

		packet.Pad128();
	}
	packet.CloseTag();

	CacheRendererState();
}

void CCustomRendererBase::DrawLinearArrays(CGeometryBlock& block)
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