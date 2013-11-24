#ifndef HPL_SIMULATEDHDM_H
#define HPL_SIMULATEDHDM_H

#include <vector>
#include <list>
#include "system/SystemTypes.h"
#include "input/Input.h"
#include "game/Updateable.h"
#include "math/Math.h"
#include <OVR.h>

namespace hpl {

	enum StereoMode
	{
		StereoMode_Left,
		StereoMode_Right
	};

	class cHMD : public iUpdateable
	{
	public:
		cHMD(cInput* apInput);
		~cHMD();

		OVR::HMDInfo getHMDInfo();

		float getUIScale();
		cVector2l getFramebufferDimensions();
		float getYawDelta();
		float getPitchDelta();
		float getRollDelta();
		void Update(float afTimeStep);

	protected:

		void AddYaw(float afAngle);

		OVR::HMDInfo mpHMDInfo;

		// Individual left or right framebuffer size, in pixels.
		cVector2l mvFramebufferDimensions;

		float mfUIScale;
		float mfPitchDelta;
		float mfYawDelta;
		float mfRollDelta;

		cQuaternion mqPreviousOrientation;

		cInput* mpInput;
};

};

#endif // HPL_SIMULATEDHDM_H
