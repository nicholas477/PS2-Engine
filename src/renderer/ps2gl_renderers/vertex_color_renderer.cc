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
	Capabilities = kVCRPrimTypeFlag | capabilities;
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
	                          false);            // ok to merge multiple calls when possible

	return renderer;
}

// void CVertexColorRenderer::InitContext(GLenum primType, tU32 rcChanges, bool userRcChanged)
// {
// 	//printf("CVertexColorRenderer::InitContext\n\n");
// 	CGLContext& glContext   = *pGLContext;
// 	CVifSCDmaPacket& packet = glContext.GetVif1Packet();

// 	packet.Cnt();
// 	{
// 		//AddVu1RendererContext(packet, primType, kContextStart);
// 		CGLContext& glContext = *pGLContext;

// 		packet.Stcycl(1, 1);
// 		packet.Flush();
// 		packet.Pad96();
// 		packet.OpenUnpack(Vifs::UnpackModes::v4_32, kContextStart, Packet::kSingleBuff);
// 		{
// 			// find light pointers
// 			CImmLighting& lighting = glContext.GetImmLighting();
// 			tLightPtrs lightPtrs[8];
// 			tLightPtrs *nextDir, *nextPt, *nextSpot;
// 			nextDir = nextPt = nextSpot = &lightPtrs[0];
// 			int numDirs, numPts, numSpots;
// 			numDirs = numPts = numSpots = 0;
// 			for (int i = 0; i < 8; i++)
// 			{
// 				CImmLight& light = lighting.GetImmLight(i);
// 				if (light.IsEnabled())
// 				{
// 					int lightBase = kLight0Base + kContextStart;
// 					if (light.IsDirectional())
// 					{
// 						nextDir->dir = lightBase + i * kLightStructSize;
// 						nextDir++;
// 						numDirs++;
// 					}
// 					else if (light.IsPoint())
// 					{
// 						nextPt->point = lightBase + i * kLightStructSize;
// 						nextPt++;
// 						numPts++;
// 					}
// 					else if (light.IsSpot())
// 					{
// 						nextSpot->spot = lightBase + i * kLightStructSize;
// 						nextSpot++;
// 						numSpots++;
// 					}
// 				}
// 			}

// 			bool doLighting = glContext.GetImmLighting().GetLightingEnabled();

// 			// transpose of object to world space xfrm (for light directions)
// 			cpu_mat_44 objToWorldXfrmTrans = glContext.GetModelViewStack().GetTop();
// 			// clear any translations.. should be doing a 3x3 transpose..
// 			objToWorldXfrmTrans.set_col3(cpu_vec_xyzw(0, 0, 0, 1));
// 			objToWorldXfrmTrans = objToWorldXfrmTrans.transpose();
// 			// do we need to rescale normals?
// 			cpu_mat_44 normalRescale;
// 			normalRescale.set_identity();
// 			float normalScale            = 1.0f;
// 			CImmDrawContext& drawContext = glContext.GetImmDrawContext();
// 			if (drawContext.GetRescaleNormals())
// 			{
// 				cpu_vec_xyzw fake_normal(1, 0, 0, 0);
// 				fake_normal = objToWorldXfrmTrans * fake_normal;
// 				normalScale = 1.0f / fake_normal.length();
// 				normalRescale.set_scale(cpu_vec_xyz(normalScale, normalScale, normalScale));
// 			}
// 			objToWorldXfrmTrans = normalRescale * objToWorldXfrmTrans;

// 			// num lights
// 			if (doLighting)
// 			{
// 				packet += numDirs;
// 				packet += numPts;
// 				packet += numSpots;
// 			}
// 			else
// 			{
// 				packet += (tU64)0;
// 				packet += 0;
// 			}

// 			// backface culling multiplier -- this is 1.0f or -1.0f, the 6th bit
// 			// also turns on/off culling
// 			float bfc_mult = (float)drawContext.GetCullFaceDir();
// 			unsigned int bfc_word;
// 			asm(" ## nop ## "
// 			    : "=r"(bfc_word)
// 			    : "0"(bfc_mult));
// 			bool do_culling = drawContext.GetDoCullFace() && (primType > GL_LINE_STRIP);
// 			packet += bfc_word | (unsigned int)do_culling << 5;

// 			// light pointers
// 			packet.Add(&lightPtrs[0], 8);

// 			float maxColorValue = GetMaxColorValue(glContext.GetTexManager().GetTexEnabled());

// 			// add light info
// 			for (int i = 0; i < 8; i++)
// 			{
// 				CImmLight& light = lighting.GetImmLight(i);
// 				packet += light.GetAmbient() * maxColorValue;
// 				packet += light.GetDiffuse() * maxColorValue;
// 				packet += light.GetSpecular() * maxColorValue;

// 				if (light.IsDirectional())
// 					packet += light.GetPosition();
// 				else
// 				{
// 					packet += light.GetPosition();
// 				}

// 				packet += light.GetSpotDir();

// 				// attenuation coeffs for positional light sources
// 				// because we're doing lighting calculations in object space,
// 				// we need to adjust the attenuation of positional light sources
// 				// and all lighting directions to take into account scaling
// 				packet += light.GetConstantAtten();
// 				packet += light.GetLinearAtten() * 1.0f / normalScale;
// 				packet += light.GetQuadAtten() * 1.0f / normalScale;
// 				packet += 0; // padding
// 			}

// 			// global ambient
// 			cpu_vec_4 globalAmb;
// 			if (doLighting)
// 				globalAmb = lighting.GetGlobalAmbient() * maxColorValue;
// 			else
// 				globalAmb = cpu_vec_4(0, 0, 0, 0);
// 			packet.Add((tU32*)&globalAmb, 3);

// 			// stick in the offset to convert clip space depth value to GS
// 			float depthClipToGs = (float)((1 << drawContext.GetDepthBits()) - 1) / 2.0f;
// 			packet += depthClipToGs;

// 			// cur material

// 			CImmMaterial& material = glContext.GetMaterialManager().GetImmMaterial();

// 			// add emissive component
// 			cpu_vec_4 emission;
// 			if (doLighting)
// 				emission = material.GetEmission() * maxColorValue;
// 			else
// 				emission = glContext.GetMaterialManager().GetCurColor() * maxColorValue;
// 			packet += emission;

// 			// ambient
// 			packet += material.GetAmbient();

// 			// diffuse
// 			cpu_vec_4 matDiffuse = material.GetDiffuse();
// 			// the alpha value is set to the alpha of the diffuse in the renderers;
// 			// this should be the current color alpha if lighting is disabled
// 			if (!doLighting)
// 				matDiffuse[3] = glContext.GetMaterialManager().GetCurColor()[3];
// 			packet += matDiffuse;

// 			// specular
// 			packet += material.GetSpecular();

// 			// vertex xform
// 			packet += drawContext.GetVertexXform();

// 			// fixed vertToEye vector for non-local specular
// 			cpu_vec_xyzw vertToEye(0.0f, 0.0f, 1.0f, 0.0f);
// 			packet += objToWorldXfrmTrans * vertToEye;

// 			// transpose of object to world space transform
// 			packet += objToWorldXfrmTrans;

// 			// world to object space xfrm (for light positions)
// 			cpu_mat_44 worldToObjXfrm = glContext.GetModelViewStack().GetInvTop();
// 			packet += worldToObjXfrm;

// 			// giftag - this is down at the bottom to make sure that when switching
// 			// primitives the last buffer will have a chance to copy the giftag before
// 			// it is overwritten with the new one
// 			GLenum newPrimType = drawContext.GetPolygonMode();
// 			if (newPrimType == GL_FILL)
// 				newPrimType = primType;
// 			newPrimType &= 0xff;
// 			tGifTag giftag = BuildGiftag(GL_TRIANGLE_STRIP);
// 			packet += giftag;

// 			// add info used by clipping code
// 			// first the dimensions of the framebuffer
// 			float xClip = (float)2048.0f / (drawContext.GetFBWidth() * 0.5f * 2.0f);
// 			packet += std::max(xClip, 1.0f);
// 			float yClip = (float)2048.0f / (drawContext.GetFBHeight() * 0.5f * 2.0f);
// 			packet += std::max(yClip, 1.0f);
// 			float depthClip = 2048.0f / depthClipToGs;
// 			// FIXME: maybe these 2048's should be 2047.5s...
// 			depthClip *= 1.003f; // round up a bit for fp error (????)
// 			packet += depthClip;
// 			// enable/disable clipping
// 			packet += (drawContext.GetDoClipping()) ? 1 : 0;
// 		}
// 		packet.CloseUnpack();


// 		packet.Mscal(0);
// 		packet.Flushe();

// 		packet.Base(kDoubleBufBase);
// 		packet.Offset(kDoubleBufOffset);
// 	}
// 	packet.CloseTag();

// 	CacheRendererState();
// }

void CVertexColorRenderer::DrawLinearArrays(CGeometryBlock& block)
{
	printf("Drawing linear arrays!\n");

	int wordsPerVert   = block.GetWordsPerVertex();
	int wordsPerNormal = (block.GetNormalsAreValid()) ? block.GetWordsPerNormal() : 0;
	int wordsPerTex    = (block.GetTexCoordsAreValid()) ? block.GetWordsPerTexCoord() : 0;
	int wordsPerColor  = (block.GetColorsAreValid()) ? block.GetWordsPerColor() : 0;

	printf("wordsPerVert: %d\n", wordsPerVert);
	printf("wordsPerNormal: %d\n", wordsPerNormal);
	printf("wordsPerTex: %d\n", wordsPerTex);
	printf("wordsPerColor: %d\n", wordsPerColor);

	int numStrips = block.GetNumStrips();

	printf("numStrips: %d\n", numStrips);

	printf("InputQuadsPerVert: %d\n", InputQuadsPerVert);
	printf("OutputQuadsPerVert: %d\n", OutputQuadsPerVert);

	CLinearRenderer::DrawLinearArrays(block);
}