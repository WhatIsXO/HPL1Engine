/*
 * RiftCompositor.cpp
 *
 *  Created on: 18/10/2013
 *      Author: WhatIsXO
 */

#include "graphics/RiftCompositor.h"
#include "graphics/GraphicsDrawer.h"
#include "graphics/RendererStereo3D.h"
#include "graphics/Texture.h"
#include "graphics/GPUProgram.h"
#include "graphics/LowLevelGraphics.h"
#include "graphics/Graphics.h"
#include "graphics/RendererPostEffects.h"
#include "resources/Resources.h"
#include "resources/GpuProgramManager.h"
#include "resources/TextureManager.h"
#include <OVR.h>
#include <Util/Util_Render_Stereo.h>

using namespace OVR::Util::Render;

namespace hpl {

	cRiftCompositor::cRiftCompositor(cGraphics* apGraphics, cResources* apResources, bool pregenWarp)
	{
		mpLowLevelGraphics = apGraphics->GetLowLevel();
		mpRenderer3D = (cRendererStereo3D*)apGraphics->GetRenderer3D();
		mpGraphicsDrawer = apGraphics->GetDrawer();
		mpOverlayGraphicsDrawer = apGraphics->GetOverlayDrawer();
		mpPostEffects = apGraphics->GetRendererPostEffects();
		mpResources = apResources;
		mpHMD = apGraphics->GetHMD();
		mbPregenWarp = pregenWarp;
		mpLeftFramebuffer = mpRenderer3D->GetLeftFramebuffer();
		mpRightFramebuffer = mpRenderer3D->GetRightFramebuffer();

		cVector2l framebufferDimensions = mpHMD->getFramebufferDimensions();
		HMDInfo apHMDInfo = mpHMD->getHMDInfo();
		cVector2l avResolution = cVector2l(apHMDInfo.HResolution, apHMDInfo.VResolution);

		// Create framebuffer objects for 2D overlay rendering
		cGpuProgramManager *pProgramManager = apResources->GetGpuProgramManager();
		mpOverlayDrawerRenderTarget = mpLowLevelGraphics->CreateTexture(cVector2l(1280,1024), 32, cColor(0,0,0,0), false, eTextureType_RenderTarget_FBO, eTextureTarget_2D);
		mpGraphicsDrawerRenderTarget = mpLowLevelGraphics->CreateTexture(framebufferDimensions, 32, cColor(0,0,0,0), false, eTextureType_RenderTarget_FBO, eTextureTarget_2D);

		// CG program - recalculating
		mpRecalculatingStereoWarpFragProgram = pProgramManager->CreateProgram("oculus.cg","main",eGpuProgramType_Fragment);

		// Set the overlay graphics position, hacked for IPD offset
		// UI overlay is drawn in (-1,1) coordinate system
		cVector2f uiSize = cVector2f(1.8f, 1.35f);
		cVector2f uiPosition = cVector2f() - (uiSize / 2);
		vOverlayQuadVec.push_back( cVertex(cVector3f(uiPosition.x, uiPosition.y,0),cVector2f(0,1),cColor(1,1.0f) ) );
		vOverlayQuadVec.push_back( cVertex(cVector3f(uiPosition.x+uiSize.x, uiPosition.y,0),cVector2f(1, 1),cColor(1,1.0f)) );
		vOverlayQuadVec.push_back( cVertex(cVector3f(uiPosition.x+uiSize.x, uiPosition.y+uiSize.y,0),cVector2f(1, 0),cColor(1,1.0f)) );
		vOverlayQuadVec.push_back( cVertex(cVector3f(uiPosition.x, uiPosition.y+uiSize.y,0),cVector2f(0, 0),cColor(1,1.0f)) );

		// The direct draw overlay is drawn in orthographics coords - framebuffer pixel dimensions
		vDirectQuadVec.push_back( cVertex(cVector3f(0,0,0),cVector2f(0,1),cColor(1,1.0f) ) );
		vDirectQuadVec.push_back( cVertex(cVector3f(framebufferDimensions.x,0,0),cVector2f(1, 1),cColor(1,1.0f)) );
		vDirectQuadVec.push_back( cVertex(cVector3f(framebufferDimensions.x,framebufferDimensions.y,0),cVector2f(1, 0),cColor(1,1.0f)) );
		vDirectQuadVec.push_back( cVertex(cVector3f(0,framebufferDimensions.y,0),cVector2f(0, 0),cColor(1,1.0f)) );

		// The actual quads to draw to the screen are in orthographic coords - screen pixel dimensions
		vLeftQuadVec.push_back( cVertex(cVector3f(0,0,0),cVector2f(0,1),cColor(1,1.0f) ) );
		vLeftQuadVec.push_back( cVertex(cVector3f(avResolution.x/2,0,0),cVector2f(1, 1),cColor(1,1.0f)) );
		vLeftQuadVec.push_back( cVertex(cVector3f(avResolution.x/2,avResolution.y,0),cVector2f(1, 0),cColor(1,1.0f)) );
		vLeftQuadVec.push_back( cVertex(cVector3f(0,avResolution.y,0),cVector2f(0, 0),cColor(1,1.0f)) );
		vRightQuadVec.push_back( cVertex(cVector3f(avResolution.x/2,0,0),cVector2f(0,1),cColor(1,1.0f) ) );
		vRightQuadVec.push_back( cVertex(cVector3f(avResolution.x,0,0),cVector2f(1, 1),cColor(1,1.0f)) );
		vRightQuadVec.push_back( cVertex(cVector3f(avResolution.x,avResolution.y,0),cVector2f(1, 0),cColor(1,1.0f)) );
		vRightQuadVec.push_back( cVertex(cVector3f(avResolution.x/2,avResolution.y,0),cVector2f(0, 0),cColor(1,1.0f)) );

		// Lens distortion precompute textures and CG program
		if (mbPregenWarp)
		{
			mpLeftWarpComputeTexture = mpLowLevelGraphics->CreateTexture(cVector2l(avResolution.x/2, avResolution.y), 32, cColor(0,0,0,0), false, eTextureType_RenderTarget_RGBA16F, eTextureTarget_2D);
			mpRightWarpComputeTexture = mpLowLevelGraphics->CreateTexture(cVector2l(avResolution.x/2, avResolution.y), 32, cColor(0,0,0,0), false, eTextureType_RenderTarget_RGBA16F, eTextureTarget_2D);
			mpStereoWarpGenerateTextureFragProgram = pProgramManager->CreateProgram("oculus_gen_distortion_map.cg","main",eGpuProgramType_Fragment);
			mpStereoWarpTextureLookupFragProgram = pProgramManager->CreateProgram("oculus_distortion_lookup.cg","main",eGpuProgramType_Fragment);
			PregenerateWarpTexture();
		}
	}

	cRiftCompositor::~cRiftCompositor()
	{
		if(mpOverlayDrawerRenderTarget)mpResources->GetTextureManager()->Destroy(mpOverlayDrawerRenderTarget);
		if(mpGraphicsDrawerRenderTarget)mpResources->GetTextureManager()->Destroy(mpGraphicsDrawerRenderTarget);
		if(mpLeftWarpComputeTexture)mpResources->GetTextureManager()->Destroy(mpLeftWarpComputeTexture);
		if(mpRightWarpComputeTexture)mpResources->GetTextureManager()->Destroy(mpRightWarpComputeTexture);
		if(mpRecalculatingStereoWarpFragProgram)mpResources->GetGpuProgramManager()->Destroy(mpRecalculatingStereoWarpFragProgram);
		if(mpStereoWarpGenerateTextureFragProgram)mpResources->GetGpuProgramManager()->Destroy(mpStereoWarpGenerateTextureFragProgram);
		if(mpStereoWarpTextureLookupFragProgram)mpResources->GetGpuProgramManager()->Destroy(mpStereoWarpTextureLookupFragProgram);
	}

	void cRiftCompositor::PregenerateWarpTexture()
	{
		// Gather rift settings
		HMDInfo apHMDInfo = mpHMD->getHMDInfo();
		stereoConfig.SetHMDInfo(apHMDInfo);
		StereoEyeParams leftEye = stereoConfig.GetEyeRenderParams(StereoEye_Left);
		StereoEyeParams rightEye = stereoConfig.GetEyeRenderParams(StereoEye_Right);

		{
			mpLowLevelGraphics->SetRenderTarget(mpLeftWarpComputeTexture);
			mpLowLevelGraphics->SetDepthTestActive(false);
			mpLowLevelGraphics->PushMatrix(eMatrix_ModelView);
			mpLowLevelGraphics->SetIdentityMatrix(eMatrix_ModelView);
			mpLowLevelGraphics->SetOrthoProjection(cVector2f(apHMDInfo.HResolution/2, apHMDInfo.VResolution),-1000,1000);
			mpLowLevelGraphics->SetBlendActive(false);

			float w = float(apHMDInfo.HResolution) / float(apHMDInfo.HResolution),
				  h = float(apHMDInfo.VResolution) / float(apHMDInfo.VResolution),
				  x = float(0) / float(apHMDInfo.HResolution),
				  y = float(0) / float(apHMDInfo.VResolution);

			float as = float(apHMDInfo.HResolution) / float(apHMDInfo.VResolution);
			mpStereoWarpGenerateTextureFragProgram->Bind();
			mpRecalculatingStereoWarpFragProgram->SetVec2f("LensCenter",  x + (w + leftEye.pDistortion->XCenterOffset * 0.5f)*0.5f, y + h*0.5f);
			mpRecalculatingStereoWarpFragProgram->SetVec2f("ScreenCenter", x + w*0.5f, y + h*0.5f);

			float scaleFactor = 1.0f / leftEye.pDistortion->Scale;

			mpRecalculatingStereoWarpFragProgram->SetVec2f("Scale", (w/2) * scaleFactor, (h/2) * scaleFactor * as);
			mpRecalculatingStereoWarpFragProgram->SetVec2f("ScaleIn", (2/w), (2/h) / as);
			mpRecalculatingStereoWarpFragProgram->SetVec4f("HmdWarpParam",
												leftEye.pDistortion->K[0],
												leftEye.pDistortion->K[1],
												leftEye.pDistortion->K[2],
												leftEye.pDistortion->K[3] );

			mpLowLevelGraphics->DrawQuad(vLeftQuadVec);
			mpStereoWarpGenerateTextureFragProgram->UnBind();
			mpLowLevelGraphics->PopMatrix(eMatrix_ModelView);
		}

		{
			// Render right to quad using oculus warp fragment program
			// Set up ortho projection to draw quad
			mpLowLevelGraphics->SetRenderTarget(mpRightWarpComputeTexture);
			mpLowLevelGraphics->SetDepthTestActive(false);
			mpLowLevelGraphics->PushMatrix(eMatrix_ModelView);
			mpLowLevelGraphics->SetIdentityMatrix(eMatrix_ModelView);
			mpLowLevelGraphics->SetOrthoProjection(cVector2f(apHMDInfo.HResolution/2, apHMDInfo.VResolution/2),-1000,1000);
			mpLowLevelGraphics->SetBlendActive(false);

			float w = float(apHMDInfo.HResolution) / float(apHMDInfo.HResolution),
				  h = float(apHMDInfo.VResolution) / float(apHMDInfo.VResolution),
				  x = float(0) / float(apHMDInfo.HResolution),
				  y = float(0) / float(apHMDInfo.VResolution);

			float as = float(apHMDInfo.HResolution) / float(apHMDInfo.VResolution);

			mpRecalculatingStereoWarpFragProgram->Bind();
			mpRecalculatingStereoWarpFragProgram->SetVec2f("LensCenter",  x + (w - rightEye.pDistortion->XCenterOffset * 0.5f)*0.5f, y + h*0.5f);
			mpRecalculatingStereoWarpFragProgram->SetVec2f("ScreenCenter", x + w*0.5f, y + h*0.5f);

			float scaleFactor = 1.0f / rightEye.pDistortion->Scale;

			mpRecalculatingStereoWarpFragProgram->SetVec2f("Scale", (w/2) * scaleFactor, (h/2) * scaleFactor * as);
			mpRecalculatingStereoWarpFragProgram->SetVec2f("ScaleIn", (2/w), (2/h) / as);
			mpRecalculatingStereoWarpFragProgram->SetVec4f("HmdWarpParam",
												rightEye.pDistortion->K[0],
												rightEye.pDistortion->K[1],
												rightEye.pDistortion->K[2],
												rightEye.pDistortion->K[3] );
			mpLowLevelGraphics->DrawQuad(vRightQuadVec);
			mpRecalculatingStereoWarpFragProgram->UnBind();
			mpLowLevelGraphics->PopMatrix(eMatrix_ModelView);
		}
	}

	void cRiftCompositor::RenderPostEffects()
	{
	//	mpPostEffects->Render();
	}

	void cRiftCompositor::ClearBuffers()
	{
		mpLowLevelGraphics->SetClearColorActive(true);
		mpLowLevelGraphics->SetClearColor(cColor(0,0,0,0));

		mpLowLevelGraphics->SetRenderTarget(mpLeftFramebuffer);
		mpLowLevelGraphics->ClearScreen();

		mpLowLevelGraphics->SetRenderTarget(mpRightFramebuffer);
		mpLowLevelGraphics->ClearScreen();

		mpLowLevelGraphics->SetRenderTarget(NULL);
	}

	void cRiftCompositor::DrawGraphics()
	{
		// Render direct screen graphics (hidden, noise, etc)
		mpLowLevelGraphics->SetRenderTarget(mpGraphicsDrawerRenderTarget);
		mpLowLevelGraphics->SetClearColorActive(true);
		mpLowLevelGraphics->SetClearColor(cColor(0,0,0,0));
		mpLowLevelGraphics->ClearScreen();
		mpGraphicsDrawer->DrawAll();

		// Draw 2D stuff to the overlay target (UI, inventory)
		mpLowLevelGraphics->SetRenderTarget(mpOverlayDrawerRenderTarget);
		mpLowLevelGraphics->SetClearColorActive(true);
		mpLowLevelGraphics->SetClearColor(cColor(0,0,0,0));
		mpLowLevelGraphics->ClearScreen();
		mpOverlayGraphicsDrawer->DrawAll();
	}

	void cRiftCompositor::CompositeAndWarpScreen()
	{
		// Build up screen space stuff in framebuffers
		DrawGraphics();

		// Gather rift settings
		HMDInfo apHMDInfo = mpHMD->getHMDInfo();
		stereoConfig.SetHMDInfo(apHMDInfo);
		StereoEyeParams leftEye = stereoConfig.GetEyeRenderParams(StereoEye_Left);
		StereoEyeParams rightEye = stereoConfig.GetEyeRenderParams(StereoEye_Right);

		// Draw the direct screen framebuffer to the left and right framebuffers
		DrawScreenQuad(mpGraphicsDrawerRenderTarget, mpLeftFramebuffer, vDirectQuadVec);
		DrawScreenQuad(mpGraphicsDrawerRenderTarget, mpRightFramebuffer, vDirectQuadVec);

		// Draw the UI framebuffer to the left and right framebuffers
		DrawOverlayQuad(mpOverlayDrawerRenderTarget, mpLeftFramebuffer, vOverlayQuadVec, leftEye);
		DrawOverlayQuad(mpOverlayDrawerRenderTarget, mpRightFramebuffer, vOverlayQuadVec, rightEye);

		// Bind textures and shader to quads and render
		mpLowLevelGraphics->SetRenderTarget(0);
		cVector2f preservedViewport = mpLowLevelGraphics->GetViewportSize();
		mpLowLevelGraphics->SetViewportSize(cVector2f(apHMDInfo.HResolution, apHMDInfo.VResolution));
		{
			// Render left to quad using oculus warp fragment program
			// Set up ortho projection to draw quad
			mpLowLevelGraphics->SetDepthTestActive(false);
			mpLowLevelGraphics->PushMatrix(eMatrix_ModelView);
			mpLowLevelGraphics->SetIdentityMatrix(eMatrix_ModelView);
			mpLowLevelGraphics->SetOrthoProjection(cVector2f(apHMDInfo.HResolution, apHMDInfo.VResolution),-1000,1000);
			mpLowLevelGraphics->SetBlendActive(false);
			mpLowLevelGraphics->SetTexture(0,mpLeftFramebuffer);

			if (mbPregenWarp)
			{
				mpLowLevelGraphics->SetTexture(1,mpLeftWarpComputeTexture);
				mpStereoWarpTextureLookupFragProgram->Bind();
			}
			else
			{

				float w = float(apHMDInfo.HResolution) / float(apHMDInfo.HResolution),
					  h = float(apHMDInfo.VResolution) / float(apHMDInfo.VResolution),
					  x = float(0) / float(apHMDInfo.HResolution),
					  y = float(0) / float(apHMDInfo.VResolution);

				float as = float(apHMDInfo.HResolution) / float(apHMDInfo.VResolution);
				mpRecalculatingStereoWarpFragProgram->Bind();
				mpRecalculatingStereoWarpFragProgram->SetVec2f("LensCenter",  x + (w + leftEye.pDistortion->XCenterOffset * 0.5f)*0.5f, y + h*0.5f);
				mpRecalculatingStereoWarpFragProgram->SetVec2f("ScreenCenter", x + w*0.5f, y + h*0.5f);

				float scaleFactor = 1.0f / leftEye.pDistortion->Scale;

				mpRecalculatingStereoWarpFragProgram->SetVec2f("Scale", (w/2) * scaleFactor, (h/2) * scaleFactor * as);
				mpRecalculatingStereoWarpFragProgram->SetVec2f("ScaleIn", (2/w), (2/h) / as);
				mpRecalculatingStereoWarpFragProgram->SetVec4f("HmdWarpParam",
													leftEye.pDistortion->K[0],
													leftEye.pDistortion->K[1],
													leftEye.pDistortion->K[2],
													leftEye.pDistortion->K[3] );
			}

			mpLowLevelGraphics->DrawQuad(vLeftQuadVec);

			if (mbPregenWarp)
			{
				mpLowLevelGraphics->SetTexture(1, NULL);
				mpStereoWarpTextureLookupFragProgram->UnBind();
			}
			else
			{
				mpRecalculatingStereoWarpFragProgram->UnBind();
			}

			mpLowLevelGraphics->SetTexture(0, NULL);
			mpLowLevelGraphics->PopMatrix(eMatrix_ModelView);
		}

		/////////////////////////////////////////////////////// Right Eye //////////////////////////////////

		{
			// Render right to quad using oculus warp fragment program
			// Set up ortho projection to draw quad
			mpLowLevelGraphics->SetDepthTestActive(false);
			mpLowLevelGraphics->PushMatrix(eMatrix_ModelView);
			mpLowLevelGraphics->SetIdentityMatrix(eMatrix_ModelView);
			mpLowLevelGraphics->SetOrthoProjection(cVector2f(apHMDInfo.HResolution, apHMDInfo.VResolution),-1000,1000);
			mpLowLevelGraphics->SetBlendActive(false);
			mpLowLevelGraphics->SetTexture(0,mpRightFramebuffer);

			if (mbPregenWarp)
			{
				mpLowLevelGraphics->SetTexture(1,mpRightWarpComputeTexture);
				mpStereoWarpTextureLookupFragProgram->Bind();
			}
			else
			{

				float w = float(apHMDInfo.HResolution) / float(apHMDInfo.HResolution),
					  h = float(apHMDInfo.VResolution) / float(apHMDInfo.VResolution),
					  x = float(0) / float(apHMDInfo.HResolution),
					  y = float(0) / float(apHMDInfo.VResolution);

				float as = float(apHMDInfo.HResolution) / float(apHMDInfo.VResolution);

				mpRecalculatingStereoWarpFragProgram->Bind();
				mpRecalculatingStereoWarpFragProgram->SetVec2f("LensCenter",  x + (w - rightEye.pDistortion->XCenterOffset * 0.5f)*0.5f, y + h*0.5f);
				mpRecalculatingStereoWarpFragProgram->SetVec2f("ScreenCenter", x + w*0.5f, y + h*0.5f);

				float scaleFactor = 1.0f / rightEye.pDistortion->Scale;

				mpRecalculatingStereoWarpFragProgram->SetVec2f("Scale", (w/2) * scaleFactor, (h/2) * scaleFactor * as);
				mpRecalculatingStereoWarpFragProgram->SetVec2f("ScaleIn", (2/w), (2/h) / as);
				mpRecalculatingStereoWarpFragProgram->SetVec4f("HmdWarpParam",
													rightEye.pDistortion->K[0],
													rightEye.pDistortion->K[1],
													rightEye.pDistortion->K[2],
													rightEye.pDistortion->K[3] );
			}

			mpLowLevelGraphics->DrawQuad(vRightQuadVec);

			if (mbPregenWarp)
			{
				mpLowLevelGraphics->SetTexture(1, NULL);
				mpStereoWarpTextureLookupFragProgram->UnBind();
			}
			else
			{
				mpRecalculatingStereoWarpFragProgram->UnBind();
			}

			mpRecalculatingStereoWarpFragProgram->UnBind();
			mpLowLevelGraphics->PopMatrix(eMatrix_ModelView);
		}

		// Unbind textures
		mpLowLevelGraphics->SetTexture(0,NULL);
		mpLowLevelGraphics->SetDepthTestActive(true);
		mpLowLevelGraphics->SetViewportSize(preservedViewport);
	}

	void cRiftCompositor::DrawOverlayQuad(iTexture* source, iTexture* target, tVertexVec quad, OVR::Util::Render::StereoEyeParams eyeParams)
	{
		// Draw overlay to quad in both left and right buffers
		mpLowLevelGraphics->SetRenderTarget(target);
		// Ensure no depth test, respect alpha values
		mpLowLevelGraphics->SetDepthTestActive(false);
		mpLowLevelGraphics->SetBlendActive(true);
		mpLowLevelGraphics->SetBlendFunc(eBlendFunc_SrcAlpha, eBlendFunc_OneMinusSrcAlpha);

		// Set ortho projection to draw to screenspace
		mpLowLevelGraphics->PushMatrix(eMatrix_ModelView);
		mpLowLevelGraphics->SetIdentityMatrix(eMatrix_ModelView);
		mpLowLevelGraphics->SetMatrix(eMatrix_Projection, toHplMatrix(eyeParams.OrthoProjection));

		// Render quad to the texture
		mpLowLevelGraphics->SetTexture(0,source);
		mpLowLevelGraphics->DrawQuad(quad);
		mpLowLevelGraphics->SetTexture(0,NULL);

		// Restore matrix stack
		mpLowLevelGraphics->PopMatrix(eMatrix_ModelView);

		// Restore blend
		mpLowLevelGraphics->SetBlendActive(false);

		// Reset render target to screen
		mpLowLevelGraphics->SetRenderTarget(0);
	}

	void cRiftCompositor::DrawOverlayQuad(iTexture* source, iTexture* target, tVertexVec quad)
	{
		// Draw overlay to quad in both left and right buffers
		mpLowLevelGraphics->SetRenderTarget(target);
		// Ensure no depth test, respect alpha values
		mpLowLevelGraphics->SetDepthTestActive(false);
		mpLowLevelGraphics->SetBlendActive(true);
		mpLowLevelGraphics->SetBlendFunc(eBlendFunc_SrcAlpha, eBlendFunc_OneMinusSrcAlpha);

		// Set ortho projection to draw to screenspace
		mpLowLevelGraphics->PushMatrix(eMatrix_ModelView);
		mpLowLevelGraphics->SetIdentityMatrix(eMatrix_ModelView);
		mpLowLevelGraphics->SetOrthoProjection(cVector2f(mpHMD->getFramebufferDimensions().x, mpHMD->getFramebufferDimensions().y),-1000,1000);

		// Render quad to the texture
		mpLowLevelGraphics->SetTexture(0,source);
		mpLowLevelGraphics->DrawQuad(quad);
		mpLowLevelGraphics->SetTexture(0,NULL);

		// Restore matrix stack
		mpLowLevelGraphics->PopMatrix(eMatrix_ModelView);

		// Restore blend
		mpLowLevelGraphics->SetBlendActive(false);

		// Reset render target to screen
		mpLowLevelGraphics->SetRenderTarget(0);
	}

	void cRiftCompositor::DrawScreenQuad(iTexture* source, iTexture* target, tVertexVec quad)
	{
		// Draw overlay to quad in both left and right buffers
		mpLowLevelGraphics->SetRenderTarget(target);
		// Ensure no depth test, respect alpha values
		mpLowLevelGraphics->SetDepthTestActive(false);
		mpLowLevelGraphics->SetBlendActive(true);
		mpLowLevelGraphics->SetBlendFunc(eBlendFunc_SrcAlpha, eBlendFunc_OneMinusSrcAlpha);

		// Set ortho projection to draw to screenspace
		mpLowLevelGraphics->PushMatrix(eMatrix_ModelView);
		mpLowLevelGraphics->SetIdentityMatrix(eMatrix_ModelView);
		mpLowLevelGraphics->SetOrthoProjection(cVector2f(mpHMD->getFramebufferDimensions().x, mpHMD->getFramebufferDimensions().y),-1000,1000);

		// Render quad to the texture
		mpLowLevelGraphics->SetTexture(0,source);
		mpLowLevelGraphics->DrawQuad(quad);
		mpLowLevelGraphics->SetTexture(0,NULL);

		// Restore matrix stack
		mpLowLevelGraphics->PopMatrix(eMatrix_ModelView);

		// Restore blend
		mpLowLevelGraphics->SetBlendActive(false);

		// Reset render target to screen
		mpLowLevelGraphics->SetRenderTarget(0);
	}

	cMatrixf cRiftCompositor::toHplMatrix(Matrix4f ovrMatrix)
	{
		cMatrixf result;
		result.m[0][0] = ovrMatrix.M[0][0];
		result.m[0][1] = ovrMatrix.M[0][1];
		result.m[0][2] = ovrMatrix.M[0][2];
		result.m[0][3] = ovrMatrix.M[0][3];

		result.m[1][0] = ovrMatrix.M[1][0];
		result.m[1][1] = ovrMatrix.M[1][1];
		result.m[1][2] = ovrMatrix.M[1][2];
		result.m[1][3] = ovrMatrix.M[1][3];

		result.m[2][0] = ovrMatrix.M[2][0];
		result.m[2][1] = ovrMatrix.M[2][1];
		result.m[2][2] = ovrMatrix.M[2][2];
		result.m[2][3] = ovrMatrix.M[2][3];

		result.m[3][0] = ovrMatrix.M[3][0];
		result.m[3][1] = ovrMatrix.M[3][1];
		result.m[3][2] = ovrMatrix.M[3][2];
		result.m[3][3] = ovrMatrix.M[3][3];

		return result;
	}

} /* namespace hpl */
