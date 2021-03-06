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
#ifndef HPL_RENDERER_POST_EFFECTS_H
#define HPL_RENDERER_POST_EFFECTS_H

#include <set>
#include <list>
#include <math.h>

#include "math/MathTypes.h"
#include "graphics/GraphicsTypes.h"
#include "graphics/RendererPostEffectsItf.h"

namespace hpl {

#define kFilterProgramNum (1)
	
	class iLowLevelGraphics;
	class iLowLevelResources;
	class iGpuProgram;
	class cResources;
	class iTexture;
	class cGpuProgramManager;
	class cRenderList;
	class cRenderer3D;

	enum ePostEffectFilter
	{
		ePostEffectFilter_Offset,
		ePostEffectFilter_LastEnum
	};

	enum ePostEffectProgram
	{
		ePostEffectProgram_Offset,
		ePostEffectProgram_LastEnum
	};
	
	class cResources;
	
	class cRendererPostEffects : public iRendererPostEffects
	{
	public:
		cRendererPostEffects(iLowLevelGraphics *apLowLevelGraphics,cResources* apResources, 
							cRenderList *apRenderList, cRenderer3D *apRenderer3D);
		~cRendererPostEffects();
		
		/**
		 * Render post effects, called by cScene
		 */
		void Render();
		
		iTexture* GetScreenBuffer(int alNum){ return mpScreenBuffer[alNum];}

		iTexture* GetFreeScreenTexture(){ return mpScreenBuffer[mImageTrailData.mlCurrentBuffer==0?1:0];}
		

	private:
		void RenderBlurTexture(iTexture *apDestination, iTexture *apSource,float afBlurAmount);
		void RenderImageTrail();
		void RenderBloom();	
		void RenderMotionBlur();
		void RenderDepthOfField();

		iLowLevelGraphics *mpLowLevelGraphics;
		iLowLevelResources *mpLowLevelResources;
		cResources* mpResources;
		cGpuProgramManager* mpGpuManager;
		cRenderer3D *mpRenderer3D;

		cRenderList *mpRenderList;

		cVector2f mvScreenSize;
		
		iTexture* mpScreenBuffer[2];

		iGpuProgram *mpBlurVP;
		iGpuProgram *mpBlur2dFP;
		iGpuProgram *mpBlurRectFP;
		bool mbBlurFallback;

		iGpuProgram *mpBloomVP;
		iGpuProgram *mpBloomFP;

		iTexture *mpBloomBlurTexture;

		iGpuProgram *mpMotionBlurVP;
		iGpuProgram *mpMotionBlurFP;

		iGpuProgram *mpDepthOfFieldVP;
		iGpuProgram *mpDepthOfFieldFP;
		iTexture *mpDofBlurTexture;
		
		tVertexVec mvTexRectVtx;
	};

};
#endif // HPL_RENDERER_POST_EFFECTS_H
