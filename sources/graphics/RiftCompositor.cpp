/*
 * RiftCompositor.cpp
 *
 *  Created on: 18/10/2013
 *      Author: prlpcf
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

namespace hpl {

	cRiftCompositor::cRiftCompositor(cGraphics* apGraphics, cResources* apResources)
	{
		mpLowLevelGraphics = apGraphics->GetLowLevel();
		mpRenderer3D = (cRendererStereo3D*)apGraphics->GetRenderer3D();
		mpGraphicsDrawer = apGraphics->GetDrawer();
		mpPostEffects = apGraphics->GetRendererPostEffects();
		mpResources = apResources;
		mpHMD = apGraphics->GetHMD();
		mpLeftFramebuffer = mpRenderer3D->GetLeftFramebuffer();
		mpRightFramebuffer = mpRenderer3D->GetRightFramebuffer();

		// TODO - problem with NVidia cards may be because of false for mipmaps. See http://www.opengl.org/wiki/Common_Mistakes#Render_To_Texture
		mpOverlayRenderTarget = mpLowLevelGraphics->CreateTexture(cVector2l(800,600), 32, cColor(0,0,0,0), false, eTextureType_RenderTarget, eTextureTarget_2D);

		cGpuProgramManager *pProgramManager = apResources->GetGpuProgramManager();
		mpStereoWarpFragProgram = pProgramManager->CreateProgram("oculus.cg","main",eGpuProgramType_Fragment);

		// TODO: Update the ui scale from riftSettings in config vars
		cVector2f uiSize = mpLowLevelGraphics->GetVirtualSize() * mpHMD->getUIScale();

		cVector2f uiPosition;
		uiPosition.x = (mpHMD->getFramebufferDimensions().x / 2) - (uiSize.x / 2);
		uiPosition.y = (mpHMD->getFramebufferDimensions().y / 2) - (uiSize.y / 2);;

		vOverlayQuadVec.push_back( cVertex(cVector3f(uiPosition.x, uiPosition.y,0),cVector2f(0,1),cColor(1,1.0f) ) );
		vOverlayQuadVec.push_back( cVertex(cVector3f(uiPosition.x+uiSize.x, uiPosition.y,0),cVector2f(1, 1),cColor(1,1.0f)) );
		vOverlayQuadVec.push_back( cVertex(cVector3f(uiPosition.x+uiSize.x, uiPosition.y+uiSize.y,0),cVector2f(1, 0),cColor(1,1.0f)) );
		vOverlayQuadVec.push_back( cVertex(cVector3f(uiPosition.x, uiPosition.y+uiSize.y,0),cVector2f(0, 0),cColor(1,1.0f)) );
	}

	cRiftCompositor::~cRiftCompositor()
	{
		if(mpOverlayRenderTarget)mpResources->GetTextureManager()->Destroy(mpOverlayRenderTarget);
		if(mpStereoWarpFragProgram)mpResources->GetGpuProgramManager()->Destroy(mpStereoWarpFragProgram);
	}

	void cRiftCompositor::RenderPostEffects()
	{
		mpLowLevelGraphics->SetRenderTarget(mpLeftFramebuffer);
		mpPostEffects->Render();

		mpLowLevelGraphics->SetRenderTarget(mpRightFramebuffer);
		mpPostEffects->Render();

		mpLowLevelGraphics->SetRenderTarget(NULL);
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

	void cRiftCompositor::CompositeAndWarpScreen()
	{
		// Draw 2D stuff to the overlay target
		mpLowLevelGraphics->SetRenderTarget(mpOverlayRenderTarget);
		mpLowLevelGraphics->SetClearColorActive(true);
		mpLowLevelGraphics->SetClearColor(cColor(0,0,0,0));
		mpLowLevelGraphics->ClearScreen();
		mpGraphicsDrawer->DrawAll();

		DrawOverlayQuad(mpOverlayRenderTarget, mpLeftFramebuffer);
		DrawOverlayQuad(mpOverlayRenderTarget, mpRightFramebuffer);

		// Bind textures and shader to quads and render

		// Render left to quad using oculus warp fragment program
		tVertexVec vLeftQuadVec;
		cVector2l res = mpHMD->getResolution();
		vLeftQuadVec.push_back( cVertex(cVector3f(0,0,0),cVector2f(0,1),cColor(1,1.0f) ) );
		vLeftQuadVec.push_back( cVertex(cVector3f(res.x/2,0,0),cVector2f(1, 1),cColor(1,1.0f)) );
		vLeftQuadVec.push_back( cVertex(cVector3f(res.x/2,res.y,0),cVector2f(1, 0),cColor(1,1.0f)) );
		vLeftQuadVec.push_back( cVertex(cVector3f(0,res.y,0),cVector2f(0, 0),cColor(1,1.0f)) );

		mpLowLevelGraphics->SetDepthTestActive(false);
		mpLowLevelGraphics->PushMatrix(eMatrix_ModelView);
		mpLowLevelGraphics->SetIdentityMatrix(eMatrix_ModelView);
		mpLowLevelGraphics->SetOrthoProjection(cVector2f(mpHMD->getResolution().x,mpHMD->getResolution().y),-1000,1000);
		mpLowLevelGraphics->SetBlendActive(false);
		mpLowLevelGraphics->SetTexture(0,mpLeftFramebuffer);
		mpStereoWarpFragProgram->Bind();
		mpStereoWarpFragProgram->SetVec2f("LensCenter",0.5+mpHMD->getLensSeparationDistance(), 0.5);
		mpStereoWarpFragProgram->SetVec2f("ScreenCenter",0, 0);
		mpStereoWarpFragProgram->SetVec2f("Scale",0.3, 0.35);
		mpStereoWarpFragProgram->SetVec2f("ScaleIn",2, 2);
		mpStereoWarpFragProgram->SetVec4f("HmdWarpParam", mpHMD->getDistortionK()[0], mpHMD->getDistortionK()[1], mpHMD->getDistortionK()[2], mpHMD->getDistortionK()[3] );
		mpLowLevelGraphics->DrawQuad(vLeftQuadVec);
		mpStereoWarpFragProgram->UnBind();
		mpLowLevelGraphics->PopMatrix(eMatrix_ModelView);


		// Render right to quad using oculus warp fragment program
		tVertexVec vRightQuadVec;
		vRightQuadVec.push_back( cVertex(cVector3f(res.x/2,0,0),cVector2f(0,1),cColor(1,1.0f) ) );
		vRightQuadVec.push_back( cVertex(cVector3f(res.x,0,0),cVector2f(1, 1),cColor(1,1.0f)) );
		vRightQuadVec.push_back( cVertex(cVector3f(res.x,res.y,0),cVector2f(1, 0),cColor(1,1.0f)) );
		vRightQuadVec.push_back( cVertex(cVector3f(res.x/2,res.y,0),cVector2f(0, 0),cColor(1,1.0f)) );

		mpLowLevelGraphics->SetDepthTestActive(false);
		mpLowLevelGraphics->PushMatrix(eMatrix_ModelView);
		mpLowLevelGraphics->SetIdentityMatrix(eMatrix_ModelView);
		mpLowLevelGraphics->SetOrthoProjection(cVector2f(mpHMD->getResolution().x,mpHMD->getResolution().y),-1000,1000);
		mpLowLevelGraphics->SetBlendActive(false);
		mpLowLevelGraphics->SetTexture(0,mpRightFramebuffer);
		mpStereoWarpFragProgram->Bind();
		mpStereoWarpFragProgram->SetVec2f("LensCenter",0.5-mpHMD->getLensSeparationDistance(), 0.5);
		mpStereoWarpFragProgram->SetVec2f("ScreenCenter",0, 0);
		mpStereoWarpFragProgram->SetVec2f("Scale",0.3, 0.35);
		mpStereoWarpFragProgram->SetVec2f("ScaleIn",2, 2);
		mpStereoWarpFragProgram->SetVec4f("HmdWarpParam", mpHMD->getDistortionK()[0], mpHMD->getDistortionK()[1], mpHMD->getDistortionK()[2], mpHMD->getDistortionK()[3] );
		mpLowLevelGraphics->DrawQuad(vRightQuadVec);
		mpStereoWarpFragProgram->UnBind();
		mpLowLevelGraphics->PopMatrix(eMatrix_ModelView);

		// Unbind textures
		mpLowLevelGraphics->SetTexture(0,NULL);

		///Composite for rift



		/*				osFb = lowLevelGraphics->getOffscreenBuffer
			setRenderTarget(leftFb)
			drawQuad(osFb)
			setRenderTarget(default)
			bindwarp
			drawQuad(leftFb)
	*/

	}

	void cRiftCompositor:: DrawOverlayQuad(iTexture* source, iTexture* target)
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
		mpLowLevelGraphics->DrawQuad(vOverlayQuadVec);
		mpLowLevelGraphics->SetTexture(0,NULL);

		// Restore matrix stack
		mpLowLevelGraphics->PopMatrix(eMatrix_ModelView);

		// Restore blend
		mpLowLevelGraphics->SetBlendActive(false);

		// Reset render target to screen
		mpLowLevelGraphics->SetRenderTarget(0);
	}

} /* namespace hpl */
