#ifndef HPL_RIFTHMD_H
#define HPL_RIFTHMD_H

#include <vector>
#include <list>
#include <OVR.h>
#include "system/SystemTypes.h"
#include "input/HMD.h"

namespace hpl {

	class cRiftHMD : public cHMD
	{
	public:
		cRiftHMD(cInput* apInput, OVR::HMDDevice* apHMDDevice);
		~cRiftHMD();

		OVR::HMDInfo getHMDInfo();
		void Update(float afTimeStep);

	private:
		OVR::HMDDevice* mpHMDDevice;
		OVR::SensorDevice* mpSensor;
		OVR::SensorFusion mFusion;

		float mfPreviousYaw;
		float mfPreviousPitch;
		float mfPreviousRoll;
	};

};

#endif // HPL_RIFTHMD_H
