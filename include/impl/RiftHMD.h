#ifndef HPL_RIFTHMD_H
#define HPL_RIFTHMD_H

#include <vector>
#include <list>
#include <OVR.h>
#include "system/SystemTypes.h"
#include "input/HMD.h"

using namespace OVR;

namespace hpl {

	class cRiftHMD : public cHMD
	{
	public:
		cRiftHMD(cInput* apInput, HMDDevice* apHMDDevice);
		~cRiftHMD();

		cQuaternion getOrientation();

	private:
		HMDDevice* mpHMDDevice;
		SensorDevice* mpSensor;
		SensorFusion* mpFusion;
		HMDInfo mHMDInfo;

		// Size of the entire screen, in pixels.
		cVector2l Resolution;
		// Physical dimensions of the active screen in meters. Can be used to calculate
		// projection center while considering IPD.
		cVector2f ScreenSize;
		// Individual left or right framebuffer size, in pixels.
		cVector2l FramebufferDimensions;
		// Physical offset from the top of the screen to the eye center, in meters.
		// This will usually, but not necessarily be half of VScreenSize.
		float VScreenCenter;
		// Distance from the eye to screen surface, in meters.
		// Useful for calculating FOV and projection.
		float EyeToScreenDistance;
		// Distance between physical lens centers useful for calculating distortion center.
		float LensSeparationDistance;
		// Configured distance between the user's eye centers, in meters. Defaults to 0.064.
		float InterpupillaryDistance;
		// Radial distortion correction coefficients.
		// The distortion assumes that the input texture coordinates will be scaled
		// by the following equation:
		//   uvResult = uvInput * (K0 + K1 * uvLength^2 + K2 * uvLength^4)
		// Where uvInput is the UV vector from the center of distortion in direction
		// of the mapped pixel, uvLength is the magnitude of that vector, and uvResult
		// the corresponding location after distortion.
		float DistortionK[4];
		float ChromaAbCorrection[4];
};

};

#endif // HPL_RIFTHMD_H
