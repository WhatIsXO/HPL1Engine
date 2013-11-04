/*
 * cRendererStereo3D.h
 *
 *  Created on: 18/10/2013
 *      Author: prlpcf
 */

#ifndef CRENDERERSTEREO3D_H_
#define CRENDERERSTEREO3D_H_

#include "graphics/Renderer3D.h"
#include "graphics/GraphicsTypes.h"
#include "graphics/GraphicsDrawer.h"
#include "math/MathTypes.h"
#include "graphics/Material.h"
#include "math/Frustum.h"
#include "input/HMD.h"

namespace hpl {

class cRendererStereo3D: public hpl::cRenderer3D {
public:
	cRendererStereo3D(iLowLevelGraphics *apLowLevelGraphics,cResources* apResources,
			cMeshCreator* apMeshCreator, cRenderList *apRenderList, cHMD* apHMD);

	~cRendererStereo3D();

	void RenderWorld(cWorld3D* apWorld, cCamera3D* apCamera, float afFrameTime);
	iTexture* GetLeftFramebuffer() { return mpLeftFramebuffer; };
	iTexture* GetRightFramebuffer() { return mpRightFramebuffer; };

private:

	void RenderLeft(cWorld3D* apWorld, cCamera3D* apCamera, float afFrameTime);
	void RenderRight(cWorld3D* apWorld, cCamera3D* apCamera, float afFrameTime);

	iTexture *mpLeftFramebuffer;
	iTexture *mpRightFramebuffer;
	cHMD* mpHMD;
};

} /* namespace hpl */
#endif /* CRENDERERSTEREO3D_H_ */
