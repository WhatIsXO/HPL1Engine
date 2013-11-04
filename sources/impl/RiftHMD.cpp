#include "impl/RiftHMD.h"
#include <OVR.h>

#ifdef WIN32
#include <conio.h>
#endif

using namespace OVR;

namespace hpl {
	
	cRiftHMD::cRiftHMD(cInput* apInput, HMDDevice* apHMDDevice) : cHMD(apInput)
	{
		mpHMDDevice->GetDeviceInfo(&mHMDInfo);

		mpSensor = mpHMDDevice->GetSensor();
		mpFusion = hplNew( SensorFusion, (mpSensor));
	}

	cRiftHMD::~cRiftHMD()
	{
		System::Destroy();
	}

	cQuaternion cRiftHMD::getOrientation()
	{
		cQuaternion quat;
		Quatf hmdQuat = mpFusion->GetOrientation();
		quat.v.x = hmdQuat.x;
		quat.v.y = hmdQuat.y;
		quat.v.z = hmdQuat.z;
		quat.w = hmdQuat.w;

		return quat;
	}
}
