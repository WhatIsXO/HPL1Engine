/*
 * RiftCompositor.h
 *
 *  Created on: 18/10/2013
 *      Author: WhatIsXO
 */

#ifndef RIFTCOMPOSITOR_H_
#define RIFTCOMPOSITOR_H_

#include "graphics/RendererStereo3D.h"

namespace hpl
{
	class iTexture;
	class cRendererStereo3D;
	class iLowLevelGraphics;
	class cResources;
	class iGpuProgram;
	class cGraphicsDrawer;
	class cGraphics;
	class iRendererPostEffects;
	class cHMD;

class cRiftCompositor
{
public:
	cRiftCompositor(cGraphics* apGraphics, cResources* apResources, bool pregenWarp);
	virtual ~cRiftCompositor();

	void RenderPostEffects();
	void CompositeAndWarpScreen();

	void ClearBuffers();

	void SetPregenerateWarp(bool pregenWarp) { mbPregenWarp = pregenWarp; }

private:
	void DrawGraphics();
	void DrawOverlayQuad(iTexture* source, iTexture* target, tVertexVec quad);
	void DrawOverlayQuad(iTexture* source, iTexture* target, tVertexVec quad, OVR::Util::Render::StereoEyeParams eyeParams);
	void DrawScreenQuad(iTexture* source, iTexture* target, tVertexVec quad);
	void PregenerateWarpTexture();
	cMatrixf toHplMatrix(Matrix4f matrix);

	bool mbPregenWarp;

	cRendererStereo3D* mpRenderer3D;
	iLowLevelGraphics* mpLowLevelGraphics;
	cGraphicsDrawer* mpGraphicsDrawer;
	cGraphicsDrawer* mpOverlayGraphicsDrawer;
	iRendererPostEffects* mpPostEffects;
	cResources* mpResources;
	cHMD* mpHMD;
	OVR::Util::Render::StereoConfig stereoConfig;

	iTexture* mpLeftWarpComputeTexture;
	iTexture* mpRightWarpComputeTexture;
	iTexture* mpOverlayDrawerRenderTarget;
	iTexture* mpGraphicsDrawerRenderTarget;
	iTexture* mpLeftFramebuffer;
	iTexture* mpRightFramebuffer;
	iGpuProgram *mpRecalculatingStereoWarpFragProgram;
	iGpuProgram *mpStereoWarpGenerateTextureFragProgram;
	iGpuProgram *mpStereoWarpTextureLookupFragProgram;

	tVertexVec vDirectQuadVec;
	tVertexVec vOverlayQuadVec;

	tVertexVec vLeftQuadVec;
	tVertexVec vRightQuadVec;
};

} /* namespace hpl */
#endif /* RIFTCOMPOSITOR_H_ */
