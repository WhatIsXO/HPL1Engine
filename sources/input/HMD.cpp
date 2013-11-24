#include "input/HMD.h"
#include "input/Keyboard.h"
#include "math/Math.h"

#ifdef WIN32
#include <conio.h>
#endif


namespace hpl {
	
	cHMD::cHMD(cInput* apInput) : iUpdateable("HPL_HMD")
	{
		mpInput = apInput;

		// Fake values suggested by Oculus SDK samples
		mpHMDInfo.HScreenSize = 0.14976;
		mpHMDInfo.VScreenSize = 0.0935;
		mpHMDInfo.VScreenCenter = 0.0935 / 2;
		mpHMDInfo.EyeToScreenDistance = 0.041;
		mpHMDInfo.LensSeparationDistance = 0.067;
		mpHMDInfo.InterpupillaryDistance = 0.0675;
		mpHMDInfo.HResolution = 1280;
		mpHMDInfo.VResolution = 800;
		mpHMDInfo.DistortionK[0] = 1.0f;
		mpHMDInfo.DistortionK[1] = 0.22f;
		mpHMDInfo.DistortionK[2] = 0.24f;
		mpHMDInfo.DistortionK[3] = 0;
		mpHMDInfo.ChromaAbCorrection[0] = 0;
		mpHMDInfo.ChromaAbCorrection[1] = 0;
		mpHMDInfo.ChromaAbCorrection[2] = 0;
		mpHMDInfo.ChromaAbCorrection[3] = 0;

		mvFramebufferDimensions = cVector2l(800, 1000);
		mfUIScale = 0.75f;
		mfPitchDelta = 0;
		mfRollDelta = 0;
		mfYawDelta = 0;
	}

	cHMD::~cHMD()
	{
	}

	OVR::HMDInfo cHMD::getHMDInfo()
	{
		return mpHMDInfo;
	}

	cVector2l cHMD::getFramebufferDimensions()
	{
		return mvFramebufferDimensions;
	}

	float cHMD::getUIScale()
	{
		return mfUIScale;
	}

	void cHMD::Update(float afTimeStep)
	{
		if (mpInput->GetKeyboard()->KeyIsDown(eKey_LEFT))
		{
			mfYawDelta = -0.01;
		}
		else if (mpInput->GetKeyboard()->KeyIsDown(eKey_RIGHT))
		{
			mfYawDelta = 0.01;
		}
		else
		{
			mfYawDelta = 0;
		}

		if (mpInput->GetKeyboard()->KeyIsDown(eKey_UP))
		{
			mfPitchDelta = -0.01;
		}
		else if (mpInput->GetKeyboard()->KeyIsDown(eKey_DOWN))
		{
			mfPitchDelta = 0.01;
		}
		else
		{
			mfPitchDelta = 0;
		}
	}

	float cHMD::getYawDelta()
	{
		return mfYawDelta;
	}

	float cHMD::getPitchDelta()
	{
		return mfPitchDelta;
	}

	float cHMD::getRollDelta()
	{
		return mfRollDelta;
	}
}
