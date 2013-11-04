/*
 * RiftCompositor.h
 *
 *  Created on: 18/10/2013
 *      Author: prlpcf
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
	class cRendererPostEffects;
	class cHMD;

class cRiftCompositor
{
public:
	cRiftCompositor(cGraphics* apGraphics, cResources* apResources);
	virtual ~cRiftCompositor();

	void RenderPostEffects();
	void CompositeAndWarpScreen();

	void ClearBuffers();

private:

	void DrawOverlayQuad(iTexture* source, iTexture* target);

	cRendererStereo3D* mpRenderer3D;
	iLowLevelGraphics* mpLowLevelGraphics;
	cGraphicsDrawer* mpGraphicsDrawer;
	cRendererPostEffects* mpPostEffects;
	cResources* mpResources;
	cHMD* mpHMD;

	iTexture* mpOverlayRenderTarget;
	iTexture* mpLeftFramebuffer;
	iTexture* mpRightFramebuffer;
	iGpuProgram *mpStereoWarpFragProgram;
	tVertexVec vOverlayQuadVec;
};

} /* namespace hpl */
#endif /* RIFTCOMPOSITOR_H_ */
