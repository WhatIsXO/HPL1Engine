/*
 * iRendererPostEffects.h
 *
 *  Created on: 15/11/2013
 *      Author: prlpcf
 */

#ifndef HPL_RENDERER_POST_EFFECTS_ITF_H_
#define HPL_RENDERER_POST_EFFECTS_ITF_H_

namespace hpl {

	class iTexture;

class cImageTrailEffect
{
public:
	cImageTrailEffect() : mbActive(false), mbFirstPass(false), mlCurrentBuffer(0) {}

	bool mbActive;
	bool mbFirstPass;
	int mlCurrentBuffer;
	float mfAmount;
};

class iRendererPostEffects {
	public:
		iRendererPostEffects();
		virtual ~iRendererPostEffects();

		virtual void Render() = 0;

		virtual void SetImageTrailActive(bool abX)
		{
			if(!mImageTrailData.mbActive && abX) mImageTrailData.mbFirstPass = true;
			mImageTrailData.mbActive = abX;}

		virtual bool GetImageTrailActive(){return mImageTrailData.mbActive;}
		/**
		 * Set the amount of blur
		 * \param afAmount 0.0 - 1.0 are valid
		 */
		virtual void SetImageTrailAmount(float afAmount){mImageTrailData.mfAmount = afAmount;}

		virtual void SetActive(bool abX){ mbActive = abX;}
		virtual bool GetActive(){ return mbActive;}

		virtual void SetBloomActive(bool abX){ mbBloomActive = abX;}
		virtual bool GetBloomActive(){ return mbBloomActive;}

		virtual void SetBloomSpread(float afX){mfBloomSpread = afX;}
		virtual float GetBloomSpread(){ return mfBloomSpread;}

		virtual void SetMotionBlurActive(bool abX){ mbMotionBlurActive = abX; mbMotionBlurFirstTime = true;}
		virtual bool GetMotionBlurActive(){ return mbMotionBlurActive;}

		virtual void SetMotionBlurAmount(float afX){ mfMotionBlurAmount = afX;}
		virtual float GetMotionBlurAmount(){ return mfMotionBlurAmount;}

		virtual void SetDepthOfFieldActive(bool abX){ mbDofActive = abX;}
		virtual void SetDepthOfFieldMaxBlur(float afX){ mfDofMaxBlur = afX;}
		virtual void SetDepthOfFieldFocalPlane(float afX){ mfDofFocalPlane = afX;}
		virtual void SetDepthOfFieldNearPlane(float afX){ mfDofNearPlane = afX;}
		virtual void SetDepthOfFieldFarPlane(float afX){ mfDofFarPlane = afX;}

		virtual bool GetDepthOfFieldActive(){ return mbDofActive;}
		virtual float GetDepthOfFieldMaxBlur(){ return mfDofMaxBlur;}
		virtual float GetDepthOfFieldFocalPlane(){ return mfDofFocalPlane;}
		virtual float GetDepthOfFieldNearPlane(){ return mfDofNearPlane;}
		virtual float GetDepthOfFieldFarPlane(){ return mfDofFarPlane;}

		virtual iTexture* GetFreeScreenTexture() = 0;

	protected:
		cImageTrailEffect mImageTrailData;
		bool mbBloomActive;
		float mfBloomSpread;

		bool mbMotionBlurActive;
		float mfMotionBlurAmount;
		bool mbMotionBlurFirstTime;

		bool mbDofActive;
		float mfDofMaxBlur;
		float mfDofFocalPlane;
		float mfDofNearPlane;
		float mfDofFarPlane;

		bool mbActive;
};

} /* namespace hpl */
#endif /* HPL_RENDERER_POST_EFFECTS_ITF_H_ */
