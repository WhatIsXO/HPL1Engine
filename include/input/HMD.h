#ifndef HPL_SIMULATEDHDM_H
#define HPL_SIMULATEDHDM_H

#include <vector>
#include <list>
#include "system/SystemTypes.h"
#include "input/Input.h"
#include "game/Updateable.h"
#include "math/Math.h"

namespace hpl {

	class cHMD : public iUpdateable
	{
	public:
		cHMD(cInput* apInput);
		~cHMD();

		cVector2l getResolution();
		cVector2f getScreenSize();
		cVector2l getFramebufferDimensions();
		float getUIScale();
		float* getChromaAbCorrection();
		float* getDistortionK();
		float getEyeToScreenDistance();
		float getInterpupillaryDistance();
		float getLensSeparationDistance();
		float getScreenCenter();
		cQuaternion getOrientation();
		void Update(float afTimeStep);

	private:

		void AddYaw(float afAngle);

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
		float UIScale;

		float mfPitch;
		float mfYaw;
		float mfRoll;

		cMatrixf mtxOrientation;

		cInput* mpInput;
};

};

#endif // HPL_SIMULATEDHDM_H
