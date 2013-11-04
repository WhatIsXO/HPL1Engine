/*
 * HMDFactory.h
 *
 *  Created on: 30/10/2013
 *      Author: prlpcf
 */

#ifndef HMDFACTORY_H_
#define HMDFACTORY_H_

#include "input/HMD.h"
#include "input/Input.h"

namespace hpl {

class HMDFactory {

	public:
		static HMDFactory* GetInstance();
		cHMD* CreateHMD(cInput* apInput);

	private:
		HMDFactory();
		virtual ~HMDFactory();

		static HMDFactory* instance;
};

} /* namespace hpl */
#endif /* HMDFACTORY_H_ */
