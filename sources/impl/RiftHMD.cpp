#include "impl/RiftHMD.h"
#include <OVR.h>

#ifdef WIN32
#include <conio.h>
#endif

namespace hpl {
	
	cRiftHMD::cRiftHMD(cInput* apInput, OVR::HMDDevice* apHMDDevice) : cHMD(apInput)
	{
		mpHMDDevice = apHMDDevice;

		// Framebuffer is 1600 / 1000 split to left and right;
		mvFramebufferDimensions = cVector2l(800, 1000);
		mpSensor = mpHMDDevice->GetSensor();
		mFusion.AttachToSensor(mpSensor);
		Update(0);
	}

	cRiftHMD::~cRiftHMD()
	{
		mpSensor->Release();
		OVR::System::Destroy();
	}

	void cRiftHMD::Update(float afTimeStep)
	{
		mpHMDDevice->GetDeviceInfo(&mpHMDInfo);
		OVR::Quatf hmdOrient = mFusion.GetPredictedOrientation();

        float    yaw = 0.0f;
        float    pitch = 0.0f;
        float    roll = 0.0f;
        hmdOrient.GetEulerAngles<OVR::Axis_Y, OVR::Axis_X, OVR::Axis_Z>(&yaw, &pitch, &roll);

        mfYawDelta = (yaw - mfPreviousYaw);
        mfPreviousYaw = yaw;

        mfPitchDelta = (pitch - mfPreviousPitch);
        mfPreviousPitch = pitch;

        mfRollDelta = (roll - mfPreviousRoll);
        mfPreviousRoll = roll;
	}

	OVR::HMDInfo cRiftHMD::getHMDInfo()
	{
		return mpHMDInfo;
	}
}
