/*
 * Copyright (C) 2006-2010 - Frictional Games
 *
 * This file is part of HPL1 Engine.
 *
 * HPL1 Engine is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * HPL1 Engine is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with HPL1 Engine.  If not, see <http://www.gnu.org/licenses/>.
 */
#ifndef HPL_RENDERER_STEREO_POST_EFFECTS_H
#define HPL_RENDERER_STEREO_POST_EFFECTS_H

#include <set>
#include <list>
#include <math.h>

#include "math/MathTypes.h"
#include "graphics/GraphicsTypes.h"
#include "graphics/RendererPostEffects.h"
#include "graphics/RendererStereo3D.h"

namespace hpl {
	
	class iLowLevelGraphics;
	class iLowLevelResources;
	class iGpuProgram;
	class cResources;
	class iTexture;
	class cGpuProgramManager;
	class cRenderList;
	class cResources;

	class cRendererFramebufferPostEffects : public iRendererPostEffects
	{
	public:
		cRendererFramebufferPostEffects(iLowLevelGraphics *apLowLevelGraphics,cResources* apResources,
							cRenderList *apRenderList, iTexture* apTargetFramebuffer);
		~cRendererFramebufferPostEffects();

		/**
		 * Render post effects, called by cScene
		 */
		void Render();

		void RenderBlurTexture(iTexture *apDestination, iTexture *apSource,float afBlurAmount);

		iTexture* GetFreeScreenTexture(){ return mpScreenBuffer[mImageTrailData.mlCurrentBuffer==0?1:0];}

	private:
		void RenderBloom();

		iLowLevelGraphics *mpLowLevelGraphics;
		iLowLevelResources *mpLowLevelResources;
		cResources* mpResources;
		cGpuProgramManager* mpGpuManager;
		cRenderer3D *mpRenderer3D;

		cRenderList *mpRenderList;

		cVector2f mvFrameSize;

		iGpuProgram *mpBlurVP;
		iGpuProgram *mpBlur2dFP;
		iGpuProgram *mpBlurRectFP;
		bool mbBlurFallback;

		iTexture* mpScreenBuffer[2];
		iGpuProgram *mpBloomVP;
		iGpuProgram *mpBloomFP;

		iTexture *mpBloomBlurTexture;

		iTexture* mpTargetFramebuffer;

		tVertexVec mvTexRectVtx;
	};

	class cRendererStereoPostEffects : public iRendererPostEffects
	{
	public:
		cRendererStereoPostEffects(iLowLevelGraphics *apLowLevelGraphics,cResources* apResources,
							cRenderList *apRenderList, cRendererStereo3D* apRendererStereo3D);
		~cRendererStereoPostEffects();

		/**
		 * Render post effects, called by cScene
		 */
		void Render();

		void SetStereoMode(StereoMode aMode) { mMode = aMode; }

		void SetActive(bool abX);
		bool GetActive();

		void SetBloomActive(bool abX);
		bool GetBloomActive();

		void SetBloomSpread(float afX);
		float GetBloomSpread();

		iTexture* GetFreeScreenTexture();

	private:

		cRendererFramebufferPostEffects* mpLeftPostEffects;
		cRendererFramebufferPostEffects* mpRightPostEffects;

		StereoMode mMode;
	};

};
#endif // HPL_RENDERER_POST_EFFECTS_H

