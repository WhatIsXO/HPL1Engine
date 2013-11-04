/*
 * cRendererStereo3D.cpp
 *
 *  Created on: 18/10/2013
 *      Author: prlpcf
 */

#include "graphics/RendererStereo3D.h"

#include "math/Math.h"
#include "graphics/Texture.h"
#include "system/LowLevelSystem.h"
#include "graphics/LowLevelGraphics.h"
#include "resources/Resources.h"
#include "resources/LowLevelResources.h"
#include "resources/TextureManager.h"
#include "graphics/VertexBuffer.h"
#include "graphics/MeshCreator.h"
#include "scene/Camera3D.h"
#include "scene/Entity3D.h"
#include "graphics/RenderList.h"
#include "graphics/Renderable.h"
#include "scene/World3D.h"
#include "scene/RenderableContainer.h"
#include "scene/Light3D.h"
#include "math/BoundingVolume.h"
#include "resources/GpuProgramManager.h"
#include "graphics/GPUProgram.h"
#include "graphics/RendererPostEffects.h"
#include "input/HMD.h"

namespace hpl
{
	cRendererStereo3D::cRendererStereo3D(iLowLevelGraphics *apLowLevelGraphics,cResources* apResources,
										 cMeshCreator* apMeshCreator, cRenderList *apRenderList, cHMD* apHMD)
						: cRenderer3D(apLowLevelGraphics, apResources, apMeshCreator, apRenderList)
	{
		mpHMD = apHMD;

		/////////////////////////////////////////////
		//Create stereoscopic framebuffer and fragment program
		mpLeftFramebuffer =  mpLowLevelGraphics->CreateTexture(mpHMD->getFramebufferDimensions(), 32, cColor(0,0,0,0), false, eTextureType_RenderTarget, eTextureTarget_2D);
		mpRightFramebuffer = mpLowLevelGraphics->CreateTexture(mpHMD->getFramebufferDimensions(), 32, cColor(0,0,0,0), false, eTextureType_RenderTarget, eTextureTarget_2D);
	}

	cRendererStereo3D::~cRendererStereo3D()
	{
		if(mpLeftFramebuffer)mpResources->GetTextureManager()->Destroy(mpLeftFramebuffer);
		if(mpRightFramebuffer)mpResources->GetTextureManager()->Destroy(mpRightFramebuffer);
	}

	void cRendererStereo3D::RenderWorld(cWorld3D* apWorld, cCamera3D* apCamera, float afFrameTime)
	{
		mfRenderTime += afFrameTime;

		//////////////////////////////
		//Setup render settings and logging
		if(mDebugFlags & eRendererDebugFlag_LogRendering){
			mbLog = true;
			mRenderSettings.mbLog = true;
		}
		else if(mbLog)
		{
			mbLog = false;
			mRenderSettings.mbLog = false;
		}
		mRenderSettings.mDebugFlags = mDebugFlags;

		// Render left and right to framebuffer textures
		mpLowLevelGraphics->SetRenderTarget(mpLeftFramebuffer);
		RenderLeft(apWorld, apCamera, afFrameTime);
		mpLowLevelGraphics->SetRenderTarget(mpRightFramebuffer);
		RenderRight(apWorld, apCamera, afFrameTime);

		// Restore default render context
		mpLowLevelGraphics->SetRenderTarget(NULL);
	}

	void cRendererStereo3D::RenderLeft(cWorld3D* apWorld, cCamera3D* pCamera3D, float afFrameTime)
	{
		/* Preserve then augment the camera projection and position to match rift values */
		cMatrixf originalProjection = pCamera3D->GetProjectionMatrix();
		cVector3f originalPosition = pCamera3D->GetPosition();

		/* Hack the projection */
		cMatrixf newProjection = cMatrixf(originalProjection);
		float ipdOffsetMeters = mpHMD->getScreenSize().x / 4 - mpHMD->getInterpupillaryDistance() / 2;
		newProjection.m[0][3] = ipdOffsetMeters;
		pCamera3D->SetProjection(newProjection);

		/* Hack the position for left IPD */
		cVector3f newPosition = cVector3f(originalPosition);
		newPosition.x = newPosition.x - (mpHMD->getInterpupillaryDistance() / 2);
		pCamera3D->SetPosition(newPosition);

//		leftCamera = new THREE.PerspectiveCamera(SCENE_FOV, DISPLAY_WIDTH / DISPLAY_HEIGHT);
//		leftCamera.projectionMatrix.elements[3] = ipdOffsetMeters;
//		leftCamera.position.x = -hmd.InterpupillaryDistance / 2;

		DoRenderInt(apWorld, pCamera3D, afFrameTime);

		/** Restore camera */
		pCamera3D->SetProjection(originalProjection);
		pCamera3D->SetPosition(originalPosition);
	}

	void cRendererStereo3D::RenderRight(cWorld3D* apWorld, cCamera3D* pCamera3D, float afFrameTime)
	{
		/* Preserve then augment the camera projection and position to match rift values */
		cMatrixf originalProjection = pCamera3D->GetProjectionMatrix();
		cVector3f originalPosition = pCamera3D->GetPosition();

		/* Hack the projection */
		cMatrixf newProjection = cMatrixf(originalProjection);
		float ipdOffsetMeters = mpHMD->getScreenSize().x / 4 - mpHMD->getInterpupillaryDistance() / 2;
		newProjection.m[0][3] = -ipdOffsetMeters;
		pCamera3D->SetProjection(newProjection);

		/* Hack the position for right IPD */
		cVector3f newPosition = cVector3f(originalPosition);
		newPosition.x = newPosition.x + (mpHMD->getInterpupillaryDistance() / 2);
		pCamera3D->SetPosition(newPosition);

//		leftCamera = new THREE.PerspectiveCamera(SCENE_FOV, DISPLAY_WIDTH / DISPLAY_HEIGHT);
//		leftCamera.projectionMatrix.elements[3] = ipdOffsetMeters;
//		leftCamera.position.x = -hmd.InterpupillaryDistance / 2;

		DoRenderInt(apWorld, pCamera3D, afFrameTime);

		/** Restore camera */
		pCamera3D->SetProjection(originalProjection);
		pCamera3D->SetPosition(originalPosition);
	}

} /* namespace hpl */
