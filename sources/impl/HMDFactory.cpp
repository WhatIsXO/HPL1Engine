/*
 * HMDFactory.cpp
 *
 *  Created on: 30/10/2013
 *      Author: WhatIsXO
 */
#include "impl/HMDFactory.h"
#include "impl/RiftHMD.h"
#include "input/HMD.h"
#include <OVR.h>

using namespace OVR;

namespace hpl {

	HMDFactory* HMDFactory::instance;

	HMDFactory* HMDFactory::GetInstance()
	{
		if (!instance)
		{
			instance = hplNew(HMDFactory, ());
		}

		return instance;
	}

	HMDFactory::HMDFactory()
	{
	}

	HMDFactory::~HMDFactory()
	{
	}

	cHMD* HMDFactory::CreateHMD(cInput* apInput)
	{
		System::Init();

		DeviceManager* apDeviceManager = DeviceManager::Create();
		HMDDevice* apHMDDevice = apDeviceManager->EnumerateDevices<HMDDevice>().CreateDevice();

		if (apHMDDevice)
		{
			return hplNew(cRiftHMD, (apInput, apHMDDevice));
		}
		else
		{
			// No rift detected, create simulation
			return hplNew(cHMD, (apInput));
		}

	}

} /* namespace hpl */
