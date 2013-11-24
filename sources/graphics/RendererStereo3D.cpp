/*
 * cRendererStereo3D.cpp
 *
 *  Created on: 18/10/2013
 *      Author: WhatIsXO
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
#include "graphics/RendererStereoPostEffects.h"
#include "input/HMD.h"

using namespace OVR::Util::Render;

namespace hpl
{
	cRendererStereo3D::cRendererStereo3D(iLowLevelGraphics *apLowLevelGraphics,cResources* apResources,
										 cMeshCreator* apMeshCreator, cRenderList *apRenderList, cHMD* apHMD)
						: cRenderer3D(apLowLevelGraphics, apResources, apMeshCreator, apRenderList)
	{
		mpHMD = apHMD;

		/////////////////////////////////////////////
		//Create stereoscopic framebuffer and fragment program
		mpLeftFramebuffer =  mpLowLevelGraphics->CreateTexture(mpHMD->getFramebufferDimensions(), 32, cColor(0,0,0,0), false, eTextureType_RenderTarget_FBO, eTextureTarget_2D);
		mpRightFramebuffer = mpLowLevelGraphics->CreateTexture(mpHMD->getFramebufferDimensions(), 32, cColor(0,0,0,0), false, eTextureType_RenderTarget_FBO, eTextureTarget_2D);
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

		DoRenderInt(apWorld, apCamera, afFrameTime);
	}

	void cRendererStereo3D::SetCameraToEye(cCamera3D* apCamera3D, StereoMode mode)
	{
		/** Get stereo config */
		StereoConfig stereoConfig;
		stereoConfig.SetHMDInfo(mpHMD->getHMDInfo());
		StereoEyeParams eyeParams;

		if (mode == StereoMode_Left)
		{
			eyeParams = stereoConfig.GetEyeRenderParams(StereoEye_Left);
			mpLowLevelGraphics->SetRenderTarget(mpLeftFramebuffer);
		}
		else
		{
			eyeParams = stereoConfig.GetEyeRenderParams(StereoEye_Right);
			mpLowLevelGraphics->SetRenderTarget(mpRightFramebuffer);
		}

		// Preserve then augment the camera projection and position to match rift values
		mOriginalProjection = apCamera3D->GetProjectionMatrix();

		// Obtain and set the eye projection matrix
		cMatrixf eyeProjection = toHplMatrix(eyeParams.Projection);
		apCamera3D->SetProjection(eyeProjection);

		// Obtain the view translate matrix, apply to camera view matrix
		cMatrixf view = apCamera3D->GetViewMatrix();
		cMatrixf transform = toHplMatrix(eyeParams.ViewAdjust);
		cMatrixf transformedView = cMath::MatrixMul(view, transform);

		// Ensure camera doesn't freak out and set the view and projection directly
		apCamera3D->SetRotateMode(eCameraRotateMode_Matrix);
		apCamera3D->SetViewMatrix(transformedView);
	}

	void cRendererStereo3D::RestoreCamera(cCamera3D* apCamera3D)
	{
		// Restore camera view and projection

		// Causes recalculation of view matrix
		apCamera3D->SetRotateMode(eCameraRotateMode_EulerAngles);
		apCamera3D->GetViewMatrix();

		// Restore preserved projection
		apCamera3D->SetProjection(mOriginalProjection);
	}

/*	void cRendererStereo3D::RenderRight(cWorld3D* apWorld, cCamera3D* pCamera3D, float afFrameTime)
	{
		StereoConfig stereoConfig;
		stereoConfig.SetHMDInfo(mpHMD->getHMDInfo());

		 Preserve then augment the camera projection and position to match rift values
		cMatrixf originalProjection = pCamera3D->GetProjectionMatrix();
		cVector3f originalPosition = pCamera3D->GetPosition();

		* Obtain the eye projection matrix
		StereoEyeParams eyeParams = stereoConfig.GetEyeRenderParams(StereoEye_Right);
		cMatrixf projRight = toHplMatrix(eyeParams.Projection);

		* Obtain the view translate matrix, apply to camera view matrix
		cMatrixf view = pCamera3D->GetViewMatrix();
		cMatrixf rightTrans = toHplMatrix(eyeParams.ViewAdjust);
		cMatrixf viewLeft = cMath::MatrixMul(view, rightTrans);

		* Ensure camera doesn't freak out and set the view and projection directly
		pCamera3D->SetRotateMode(eCameraRotateMode_Matrix);
		pCamera3D->SetViewMatrix(viewLeft);
		pCamera3D->SetProjection(projRight);

		* Render the world
		DoRenderInt(apWorld, pCamera3D, afFrameTime);

		* Restore camera position and projection
		pCamera3D->SetRotateMode(eCameraRotateMode_EulerAngles);
		pCamera3D->GetViewMatrix();
		pCamera3D->SetProjection(originalProjection);
		pCamera3D->SetPosition(originalPosition);*/

/*		 Preserve then augment the camera projection and position to match rift values
		cMatrixf originalProjection = pCamera3D->GetProjectionMatrix();
		cVector3f originalPosition = pCamera3D->GetPosition();

		 Hack the projection
		cMatrixf newProjection = cMatrixf(originalProjection);
		float ipdOffsetMeters = mpHMD->getScreenSize().x / 4 - mpHMD->getInterpupillaryDistance() / 2;
		newProjection.m[0][3] = -ipdOffsetMeters;
		pCamera3D->SetProjection(newProjection);

		 Hack the position for right IPD
		cVector3f newPosition = cVector3f(originalPosition);
		newPosition.x = newPosition.x + (mpHMD->getInterpupillaryDistance() / 2);
		pCamera3D->SetPosition(newPosition);

		DoRenderInt(apWorld, pCamera3D, afFrameTime);

		* Restore camera
		pCamera3D->SetProjection(originalProjection);
		pCamera3D->SetPosition(originalPosition);

		 Preserve then augment the camera projection and position to match rift values
		cMatrixf originalProjection = pCamera3D->GetProjectionMatrix();
		cVector3f originalPosition = pCamera3D->GetPosition();

		// Compute Aspect Ratio. Stereo mode cuts width in half.
		HMDInfo info = mpHMD->getHMDInfo();

		float aspectRatio = float(info.HResolution * 0.5f) / float(info.VResolution);

		// Compute Vertical FOV based on distance.
		float halfScreenDistance = (info.VScreenSize / 2);
		float yfov = 2.0f * atan(halfScreenDistance/info.EyeToScreenDistance);
		// Post-projection viewport coordinates range from (-1.0, 1.0), with the
		// center of the left viewport falling at (1/4) of horizontal screen size.
		// We need to shift this projection center to match with the lens center.
		// We compute this shift in physical units (meters) to correct
		// for different screen sizes and then rescale to viewport coordinates.
		float viewCenter = info.HScreenSize * 0.25f;
		float projectionCenterOffset = viewCenter - info.LensSeparationDistance*0.5f;


		// View is correct, projection needs work.

		// Projection matrix for the "center eye", which the left/right matrices are based on.
		//cMatrixf projCenter = PerspectiveLH(yfov, aspectRatio, 0.3f, 1000.0f);
		pCamera3D->SetFarClipPlane(1000.0f);
		pCamera3D->SetNearClipPlane(0.3f);
		pCamera3D->SetAspect(aspectRatio);
		cMatrixf projCenter = pCamera3D->GetProjectionMatrix();
		cMatrixf projLeft = cMath::MatrixMul(cMath::MatrixTranslate(cVector3f(-projectionCenterOffset, 0, 0)), projCenter);

		// View transformation translation in world units.
		float halfIPD =info.InterpupillaryDistance * 0.5f;

		cMatrixf view = pCamera3D->GetViewMatrix();
		cMatrixf viewLeft = cMath::MatrixMul(cMath::MatrixTranslate(cVector3f(-halfIPD, 0, 0)), view);

		pCamera3D->SetRotateMode(eCameraRotateMode_Matrix);
		pCamera3D->SetViewMatrix(viewLeft);
		//pCamera3D->SetProjection(projLeft);
		DoRenderInt(apWorld, pCamera3D, afFrameTime);

		* Restore camera
		pCamera3D->SetRotateMode(eCameraRotateMode_EulerAngles);
		pCamera3D->GetViewMatrix();
		pCamera3D->SetProjection(originalProjection);
		pCamera3D->SetPosition(originalPosition);
	}*/

	cMatrixf cRendererStereo3D::toHplMatrix(Matrix4f ovrMatrix)
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

}

/* namespace hpl */
