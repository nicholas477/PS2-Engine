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

#include "vu1_context.h"

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

#define kContextStart 0 // for the kLightBase stuff below

void CCustomRendererBase::AddVu1RendererContext(CVifSCDmaPacket& packet, GLenum primType, int vu1Offset)
{
	CGLContext& glContext = *pGLContext;

	packet.Stcycl(1, 1);
	packet.Flush();
	packet.Pad96();
	packet.OpenUnpack(Vifs::UnpackModes::v4_32, vu1Offset, Packet::kSingleBuff);
	{
		// find light pointers
		CImmLighting& lighting = glContext.GetImmLighting();
		tLightPtrs lightPtrs[8];

		bool doLighting = false;

		// transpose of object to world space xfrm (for light directions)
		cpu_mat_44 objToWorldXfrmTrans = glContext.GetModelViewStack().GetTop();
		// clear any translations.. should be doing a 3x3 transpose..
		objToWorldXfrmTrans.set_col3(cpu_vec_xyzw(0, 0, 0, 1));
		objToWorldXfrmTrans = objToWorldXfrmTrans.transpose();
		// do we need to rescale normals?
		cpu_mat_44 normalRescale;
		normalRescale.set_identity();
		float normalScale            = 1.0f;
		CImmDrawContext& drawContext = glContext.GetImmDrawContext();
		objToWorldXfrmTrans          = normalRescale * objToWorldXfrmTrans;

		// num lights
		packet += (tU64)0;
		packet += 0;

		// backface culling multiplier -- this is 1.0f or -1.0f, the 6th bit
		// also turns on/off culling
		float bfc_mult = (float)drawContext.GetCullFaceDir();
		unsigned int bfc_word;
		asm(" ## nop ## "
		    : "=r"(bfc_word)
		    : "0"(bfc_mult));
		bool do_culling = drawContext.GetDoCullFace() && (primType > GL_LINE_STRIP);
		packet += bfc_word | (unsigned int)do_culling << 5;

		// light pointers
		packet.Add(&lightPtrs[0], 8);

		float maxColorValue = GetMaxColorValue(glContext.GetTexManager().GetTexEnabled());

		// add light info
		for (int i = 0; i < 8; i++)
		{
			CImmLight& light = lighting.GetImmLight(i);
			packet += light.GetAmbient() * maxColorValue;
			packet += light.GetDiffuse() * maxColorValue;
			packet += light.GetSpecular() * maxColorValue;

			if (light.IsDirectional())
				packet += light.GetPosition();
			else
			{
				packet += light.GetPosition();
			}

			packet += light.GetSpotDir();

			// attenuation coeffs for positional light sources
			// because we're doing lighting calculations in object space,
			// we need to adjust the attenuation of positional light sources
			// and all lighting directions to take into account scaling
			packet += light.GetConstantAtten();
			packet += light.GetLinearAtten() * 1.0f / normalScale;
			packet += light.GetQuadAtten() * 1.0f / normalScale;
			packet += 0; // padding
		}

		// global ambient
		cpu_vec_4 globalAmb;
		if (doLighting)
			globalAmb = lighting.GetGlobalAmbient() * maxColorValue;
		else
			globalAmb = cpu_vec_4(0, 0, 0, 0);
		packet.Add((tU32*)&globalAmb, 3);

		// stick in the offset to convert clip space depth value to GS
		float depthClipToGs = (float)((1 << drawContext.GetDepthBits()) - 1) / 2.0f;
		packet += depthClipToGs;

		// cur material

		CImmMaterial& material = glContext.GetMaterialManager().GetImmMaterial();

		// add emissive component
		cpu_vec_4 emission;
		if (doLighting)
			emission = material.GetEmission() * maxColorValue;
		else
			emission = glContext.GetMaterialManager().GetCurColor() * maxColorValue;
		packet += emission;

		// ambient
		packet += material.GetAmbient();

		// diffuse
		cpu_vec_4 matDiffuse = material.GetDiffuse();
		// the alpha value is set to the alpha of the diffuse in the renderers;
		// this should be the current color alpha if lighting is disabled
		if (!doLighting)
			matDiffuse[3] = glContext.GetMaterialManager().GetCurColor()[3];
		packet += matDiffuse;

		// specular
		packet += material.GetSpecular();

		// vertex xform
		packet += drawContext.GetVertexXform();

		// fixed vertToEye vector for non-local specular
		cpu_vec_xyzw vertToEye(0.0f, 0.0f, 1.0f, 0.0f);
		packet += objToWorldXfrmTrans * vertToEye;

		// transpose of object to world space transform
		packet += objToWorldXfrmTrans;

		// world to object space xfrm (for light positions)
		cpu_mat_44 worldToObjXfrm = glContext.GetModelViewStack().GetInvTop();
		packet += worldToObjXfrm;

		// giftag - this is down at the bottom to make sure that when switching
		// primitives the last buffer will have a chance to copy the giftag before
		// it is overwritten with the new one
		GLenum newPrimType = drawContext.GetPolygonMode();
		if (newPrimType == GL_FILL)
			newPrimType = primType;
		newPrimType &= 0xff;
		tGifTag giftag = BuildGiftag(newPrimType);
		packet += giftag;

		// add info used by clipping code
		// first the dimensions of the framebuffer
		float xClip = (float)2048.0f / (drawContext.GetFBWidth() * 2.f);
		packet += Math::Max(xClip, 1.0f);
		float yClip = (float)2048.0f / (drawContext.GetFBHeight() * 2.f);
		packet += Math::Max(yClip, 1.0f);
		float depthClip = 2048.0f / depthClipToGs;
		// FIXME: maybe these 2048's should be 2047.5s...
		depthClip *= 1.003f; // round up a bit for fp error (????)
		//printf("depthClip: %f\n", depthClip);
		packet += depthClip;
		// enable/disable clipping
		packet += (drawContext.GetDoClipping()) ? 1 : 0;
	}
	packet.CloseUnpack();
}