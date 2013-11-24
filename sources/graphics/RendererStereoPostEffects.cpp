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
#include "graphics/RendererStereoPostEffects.h"
#include "graphics/Texture.h"
#include "scene/Scene.h"
#include "system/LowLevelSystem.h"
#include "graphics/LowLevelGraphics.h"
#include "graphics/GPUProgram.h"
#include "resources/LowLevelResources.h"
#include "math/Math.h"
#include "resources/Resources.h"
#include "resources/GpuProgramManager.h"
#include "graphics/RenderList.h"
#include "graphics/Renderer3D.h"
#include "graphics/RendererStereo3D.h"

namespace hpl {

	//////////////////////////////////////////////////////////////////////////
	// CONSTRUCTORS
	//////////////////////////////////////////////////////////////////////////

	cRendererStereoPostEffects::cRendererStereoPostEffects(iLowLevelGraphics *apLowLevelGraphics,cResources* apResources,
														cRenderList *apRenderList, cRendererStereo3D* apRendererStereo3D)
								: iRendererPostEffects()
	{
		mpRightPostEffects = hplNew(cRendererFramebufferPostEffects,(apLowLevelGraphics, apResources, apRenderList, apRendererStereo3D->GetRightFramebuffer()));
		mpLeftPostEffects = hplNew(cRendererFramebufferPostEffects,(apLowLevelGraphics, apResources, apRenderList, apRendererStereo3D->GetLeftFramebuffer()));
		mMode = StereoMode_Left;
	}

	cRendererStereoPostEffects::~cRendererStereoPostEffects()
	{
		hplDelete(mpLeftPostEffects);
		hplDelete(mpRightPostEffects);
	}

	//////////////////////////////////////////////////////////////////////////
	// PUBLIC METHODS
	//////////////////////////////////////////////////////////////////////////

	void cRendererStereoPostEffects::Render()
	{
		mpLeftPostEffects->Render();
		mpRightPostEffects->Render();
	}

	void cRendererStereoPostEffects::SetActive(bool abX)
	{
		iRendererPostEffects::SetActive(abX);
		mpLeftPostEffects->SetActive(abX);
		mpRightPostEffects->SetActive(abX);
	}

	bool cRendererStereoPostEffects::GetActive()
	{
		return iRendererPostEffects::GetActive();
	}

	void cRendererStereoPostEffects::SetBloomActive(bool abX)
	{
		iRendererPostEffects::SetBloomActive(abX);
		mpLeftPostEffects->SetBloomActive(abX);
		mpRightPostEffects->SetBloomActive(abX);
	}

	bool cRendererStereoPostEffects::GetBloomActive()
	{
		return iRendererPostEffects::GetBloomActive();
	}

	void cRendererStereoPostEffects::SetBloomSpread(float afX)
	{
		iRendererPostEffects::SetBloomSpread(afX);
		mpLeftPostEffects->SetBloomSpread(afX);
		mpRightPostEffects->SetBloomSpread(afX);
	}

	float cRendererStereoPostEffects::GetBloomSpread()
	{
		return iRendererPostEffects::GetBloomSpread();
	}

	iTexture* cRendererStereoPostEffects::GetFreeScreenTexture()
	{
		if (mMode == StereoMode_Left)
			return mpLeftPostEffects->GetFreeScreenTexture();
		else
			return mpRightPostEffects->GetFreeScreenTexture();
	}

	//////////////////////////////////////////////////////////////////////////
	// CONSTRUCTORS
	//////////////////////////////////////////////////////////////////////////

	cRendererFramebufferPostEffects::cRendererFramebufferPostEffects(iLowLevelGraphics *apLowLevelGraphics,cResources* apResources,
												cRenderList *apRenderList, iTexture* apTargetFramebuffer)
	{
		mpLowLevelGraphics = apLowLevelGraphics;
		mpLowLevelResources = apResources->GetLowLevel();
		mpResources = apResources;

		mpGpuManager = mpResources->GetGpuProgramManager();
		mpTargetFramebuffer = apTargetFramebuffer;
		// Typically the actual resolution of the screen/window - want it to be per-framebuffer
		mvFrameSize = cVector2f((float)mpTargetFramebuffer->GetWidth(), (float)mpTargetFramebuffer->GetHeight());

		mpRenderList = apRenderList;

		///////////////////////////////////////////
		// Create screen buffers
		Log(" Creating screen buffers size %s\n",mvFrameSize.ToString().c_str());
		for(int i=0;i<2;i++)
		{
			if(mpLowLevelGraphics->GetCaps(eGraphicCaps_TextureTargetRectangle))
			{
				mpScreenBuffer[i] = mpLowLevelGraphics->CreateTexture(cVector2l(
					(int)mvFrameSize.x,(int)mvFrameSize.y),32,cColor(0,0,0,0),false,
					eTextureType_Normal, eTextureTarget_Rect);
				
				if(mpScreenBuffer[i]==NULL)
				{
					Error("Couldn't create screenbuffer!\n");
					mpScreenBuffer[0] = NULL;
					mpScreenBuffer[1] = NULL;
					break;
				}
				mpScreenBuffer[i]->SetWrapS(eTextureWrap_ClampToBorder);
				mpScreenBuffer[i]->SetWrapT(eTextureWrap_ClampToBorder);
			}
			else
			{	
				mpScreenBuffer[i] = NULL;
				Error("Texture rectangle not supported. Posteffects will be turned off.");
			}
		}

		///////////////////////////////////////////
		// Create programs

		Log(" Creating programs\n");
		
		/////////////////
		//Blur programs
		mbBlurFallback = false; //Set to true if the fallbacks are used.

		mpBlurVP = mpGpuManager->CreateProgram("PostEffect_Blur_vp.cg","main",eGpuProgramType_Vertex);
		if(!mpBlurVP) Error("Couldn't load 'PostEffect_Blur_vp.cg'!\n");
		
		mpBlurRectFP = mpGpuManager->CreateProgram("PostEffect_Blur_Rect_fp.cg","main",eGpuProgramType_Fragment);
		/*if(!mpBlurRectFP)
		{
			mbBlurFallback = true;
			Log(" Using Blur Rect FP fallback\n");
			mpBlurRectFP = mpGpuManager->CreateProgram("PostEffect_Fallback01_Blur_Rect_fp.cg","main",eGpuProgramType_Fragment);
			if(!mpBlurRectFP) Error("Couldn't load 'PostEffect_Blur_Rect_fp.cg'!\n");
		}*/
		
		
		mpBlur2dFP = mpGpuManager->CreateProgram("PostEffect_Blur_2D_fp.cg","main",eGpuProgramType_Fragment);
		/*if(!mpBlur2dFP)
		{
			mbBlurFallback = true;
			Log(" Using Blur 2D FP fallback\n");
			mpBlur2dFP = mpGpuManager->CreateProgram("PostEffect_Fallback01_Blur_2D_fp.cg","main",eGpuProgramType_Fragment);
			if(!mpBlur2dFP) Error("Couldn't load 'PostEffect_Blur_2D_fp.cg'!\n");
		}*/
		
		
		/////////////////
		// Bloom programs
		mpBloomVP = mpGpuManager->CreateProgram("PostEffect_Bloom_vp.cg","main",eGpuProgramType_Vertex);
		if(!mpBloomVP) Error("Couldn't load 'PostEffect_Bloom_vp.cg'!\n");
		
		mpBloomFP = mpGpuManager->CreateProgram("PostEffect_Bloom_fp.cg","main",eGpuProgramType_Fragment);
		if(!mpBloomFP) Error("Couldn't load 'PostEffect_Bloom_fp.cg'!\n");
		
		//Bloom blur textures
		mpBloomBlurTexture = mpLowLevelGraphics->CreateTexture(
												cVector2l(256,256),
												32,cColor(0,0,0,0),false,
												eTextureType_Normal, eTextureTarget_2D);
		
		if(mpBloomBlurTexture == NULL) {
			Error("Couldn't create bloom blur textures!\n");
		}
		else {
			mpBloomBlurTexture->SetWrapS(eTextureWrap_ClampToEdge);
			mpBloomBlurTexture->SetWrapT(eTextureWrap_ClampToEdge);
		}
		
		Log("  RendererPostEffects created\n");
		
		////////////////////////////////////
		// Variable setup
		
		//General
		mbActive = false;
		mvTexRectVtx.resize(4);
		
		//Bloom
		mbBloomActive = false;
		mfBloomSpread = 2.0f;
	}

	//-----------------------------------------------------------------------

	cRendererFramebufferPostEffects::~cRendererFramebufferPostEffects()
	{
		for(int i=0;i<2;i++)
			if(mpScreenBuffer[i])hplDelete(mpScreenBuffer[i]);

		if(mpBloomVP) mpGpuManager->Destroy(mpBloomVP);
		if(mpBloomFP) mpGpuManager->Destroy(mpBloomFP);
	}

	//-----------------------------------------------------------------------

	//////////////////////////////////////////////////////////////////////////
	// PUBLIC METHODS
	//////////////////////////////////////////////////////////////////////////
	
	void cRendererFramebufferPostEffects::Render()
	{
		if(mpScreenBuffer[0] == NULL || mpScreenBuffer[1] == NULL) return;
		if(!mpLowLevelGraphics->GetCaps(eGraphicCaps_TextureTargetRectangle)) return;
		
		if(mbActive==false || 
			(	mbBloomActive==false  ) ) 
		{
			return;
		}

		mpLowLevelGraphics->SetRenderTarget(mpTargetFramebuffer);

		mpLowLevelGraphics->SetDepthTestActive(false);
		mpLowLevelGraphics->PushMatrix(eMatrix_ModelView);
		mpLowLevelGraphics->SetIdentityMatrix(eMatrix_ModelView);
		mpLowLevelGraphics->SetOrthoProjection(mvFrameSize,-1000,1000);
				
		RenderBloom();
		
		mpLowLevelGraphics->PopMatrix(eMatrix_ModelView);
				
		mpLowLevelGraphics->SetRenderTarget(0);
	}

	//-----------------------------------------------------------------------

	void cRendererFramebufferPostEffects::RenderBlurTexture(iTexture *apDestination, iTexture *apSource,
													float afBlurAmount)
	{
		bool bProgramsLoaded = false;
		if(mpBlurRectFP && mpBlur2dFP && mpBlurVP) bProgramsLoaded = true;
		
		iLowLevelGraphics *pLowLevel = mpLowLevelGraphics;
		
		cVector2l vBlurSize = cVector2l(apDestination->GetWidth(), apDestination->GetHeight());
		cVector2f vBlurDrawSize = cVector2f(vBlurSize.x, vBlurSize.y);


		pLowLevel->SetBlendActive(false);

		///////////////////////////////////////////
		//Horizontal blur pass
		
		//Shader setup
		if(bProgramsLoaded)
		{
			//Setup vertex program
			mpBlurVP->Bind();
			mpBlurVP->SetFloat("xOffset",1);
			mpBlurVP->SetFloat("yOffset",0);
			mpBlurVP->SetFloat("amount",afBlurAmount);
			mpBlurVP->SetMatrixf("worldViewProj",eGpuProgramMatrix_ViewProjection,eGpuProgramMatrixOp_Identity);

			//Setup fragment program
			mpBlurRectFP->Bind();
		}

		//Draw the screen texture with blur
		pLowLevel->SetTexture(0,apSource);
		if(mbBlurFallback){
			mpLowLevelGraphics->SetTexture(1,apSource);
			mpLowLevelGraphics->SetTexture(2,apSource);
		}

		{
			mvTexRectVtx[0] = cVertex(cVector3f(0,0,0),cVector2f(0,1),cColor(1,1.0f) );
			mvTexRectVtx[1] = cVertex(cVector3f(vBlurDrawSize.x,0,0),cVector2f(1,1),cColor(1,1.0f));
			mvTexRectVtx[2] = cVertex(cVector3f(vBlurDrawSize.x,vBlurDrawSize.y,0),cVector2f(1,0),cColor(1,1.0f));
			mvTexRectVtx[3] = cVertex(cVector3f(0,vBlurDrawSize.y,0),cVector2f(0,0),cColor(1,1.0f));

			mpLowLevelGraphics->DrawQuad(mvTexRectVtx);
		}

		// TODO may need alternate method of context copy -> the lowlevelgraphics uses screensize internally, should use framesize
		pLowLevel->CopyContextToTexure(apDestination,0, vBlurSize);
			
		///////////////////////////////////////////
		//Vertical blur pass
		
		//Setup shaders
		//Shader setup
		if(bProgramsLoaded)
		{
			mpBlurVP->SetFloat("xOffset",0);
			mpBlurVP->SetFloat("yOffset",1);
			mpBlurVP->SetFloat("amount",(1 / pLowLevel->GetScreenSize().x) * afBlurAmount);

			mpBlurRectFP->UnBind();
			mpBlur2dFP->Bind();
		}

		//Set texture and draw
		pLowLevel->SetTexture(0,apDestination);
		if(mbBlurFallback){
			mpLowLevelGraphics->SetTexture(1,apDestination);
			mpLowLevelGraphics->SetTexture(2,apDestination);
		}
		
		{
			mvTexRectVtx[0] = cVertex(cVector3f(0,0,0),cVector2f(0,1),cColor(1,1.0f) );
			mvTexRectVtx[1] = cVertex(cVector3f(vBlurDrawSize.x,0,0),cVector2f(1,1),cColor(1,1.0f));
			mvTexRectVtx[2] = cVertex(cVector3f(vBlurDrawSize.x,vBlurDrawSize.y,0),cVector2f(1,0),cColor(1,1.0f));
			mvTexRectVtx[3] = cVertex(cVector3f(0,vBlurDrawSize.y,0),cVector2f(0,0),cColor(1,1.0f));

			mpLowLevelGraphics->DrawQuad(mvTexRectVtx);
		}

		//Shader setup
		if(bProgramsLoaded)
		{
			mpBlur2dFP->UnBind();
			mpBlurVP->UnBind();
		}

		pLowLevel->CopyContextToTexure(apDestination,0, vBlurSize);

		if(mbBlurFallback){
			mpLowLevelGraphics->SetTexture(1,NULL);
			mpLowLevelGraphics->SetTexture(2,NULL);
		}
	}

	//-----------------------------------------------------------------------


	//////////////////////////////////////////////////////////////////////////
	// PRIVATE METHODS
	//////////////////////////////////////////////////////////////////////////

	void cRendererFramebufferPostEffects::RenderBloom()
	{	
		if(mbBloomActive==false) return;

		if(mpBloomFP==NULL || mpBloomVP==NULL) return;

		//////////////////////////////
		// Setup
		
		iTexture *pScreenTexture = mpScreenBuffer[mImageTrailData.mlCurrentBuffer==0?1:0];

		//Copy frame to texture		
		mpLowLevelGraphics->CopyContextToTexure(pScreenTexture,0, 
												cVector2l((int)mvFrameSize.x,(int)mvFrameSize.y));
		
		//Get the blur texture
        RenderBlurTexture(mpBloomBlurTexture,pScreenTexture,mfBloomSpread);

		//Size of blur texture
		cVector2f vBlurSize = cVector2f((float)mpBloomBlurTexture->GetWidth(),(float)mpBloomBlurTexture->GetHeight());
		
		///////////////////////////////////////////
		//Draw Bloom

		//Setup vertex program
		mpBloomVP->Bind();
		mpBloomVP->SetMatrixf("worldViewProj",eGpuProgramMatrix_ViewProjection, eGpuProgramMatrixOp_Identity);

		//Setup fragment program
		mpBloomFP->Bind();

		mpLowLevelGraphics->SetTexture(0,mpBloomBlurTexture);
		mpLowLevelGraphics->SetTexture(1,pScreenTexture);

		//Draw
		{
			tVector3fVec vUvVec; vUvVec.resize(4);

			vUvVec[0] = cVector2f(0,1);
			vUvVec[1] = cVector2f(1,1);
			vUvVec[2] = cVector2f(1,0);
			vUvVec[3] = cVector2f(0,0);

			mvTexRectVtx[0] = cVertex(cVector3f(0,0,0),cVector2f(0,1),cColor(1,1.0f) );
			mvTexRectVtx[1] = cVertex(cVector3f(mvFrameSize.x,0,0),cVector2f(1,1),cColor(1,1.0f));
			mvTexRectVtx[2] = cVertex(cVector3f(mvFrameSize.x,mvFrameSize.y,0),cVector2f(1,0),cColor(1,1.0f));
			mvTexRectVtx[3] = cVertex(cVector3f(0,mvFrameSize.y,0),cVector2f(0,0),cColor(1,1.0f));

			mpLowLevelGraphics->DrawQuadMultiTex(mvTexRectVtx, vUvVec);
		}

		mpBloomVP->UnBind();
		mpBloomFP->UnBind();

		mpLowLevelGraphics->SetTexture(0,NULL);
		mpLowLevelGraphics->SetTexture(1,NULL);

		mpLowLevelGraphics->SetRenderTarget(0);
	}
}

