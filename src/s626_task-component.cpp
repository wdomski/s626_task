/**
 * \file s626_task-component.cpp
 *
 * \author Wojciech Domski
 *
 * \brief Implementation of S626_task OROCOS component
 *
 */

/***************************************************************************
 *   Copyright (C) 2014 by Wojciech Domski                                 *
 *   Wojciech.Domski@gmail.com                                             *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/

#include "s626_task-component.hpp"
#include <rtt/Component.hpp>
#include <iostream>
#include <vector>
#include <native/timer.h>

#include "S626API.h"

S626_task::S626_task(std::string const& name) :
		TaskContext(name), err(0), Device("analogy0"), Bus(0), Slot(0), state(0) {

	this->addOperation("readDIO", &S626_task::readDIO, this, RTT::OwnThread).doc(
			"Read digital input").arg("Bank", "Bank number 0-2");

	this->addOperation("writeDIO", &S626_task::writeDIO, this, RTT::OwnThread).doc(
			"Write digital output").arg("Bank", "Bank number 0-2").arg("Mask",
			"Mask for bits to be written").arg("Value",
			"Value on specific bit position");

	this->addOperation("readADC", &S626_task::readADC, this, RTT::OwnThread).doc(
			"Read analog value").arg("Channel", "Channel to be read 0-15");

	this->addOperation("setrangeADC", &S626_task::setrangeADC, this,
			RTT::OwnThread).doc("Set range of ADC").arg("Mask",
			"Mask for bits to be written").arg("Value",
			"Value on specific bit position");

	this->addOperation("writeDAC", &S626_task::writeDAC, this, RTT::OwnThread).doc(
			"Write analog output").arg("Channel", "Channel to be written 0-3").arg(
			"Value", "Value to be written");

	this->addOperation("getLastError", &S626_task::getLastError, this,
			RTT::OwnThread).doc("Gets lats error and clears it");

	this->addOperation("prepareAllENC", &S626_task::prepareAllENC, this,
			RTT::OwnThread).doc("Prepare all encoders");

	this->addOperation("readENC", &S626_task::readENC, this, RTT::OwnThread).doc(
			"Read encoder").arg("Enc", "Specific encoder 0-5");

	this->addOperation("setActivePublishing", &S626_task::setActivePublishing,
			this, RTT::OwnThread).doc(
			"Activates publishing of read data on ADC, ENC and DIO").arg("state",
			"Encoded state 0-7");

	this->addOperation("setInitialDIO", &S626_task::setInitialDIO,
			this, RTT::OwnThread).doc(
			"Sets initial state on DIO and configures which ports are outputs")
			.arg("InitialDIO", "Vector of 6 integers");

	this->addOperation("setInitialADC", &S626_task::setInitialADC,
			this, RTT::OwnThread).doc(
			"Sets which ADC should be read")
			.arg("InitialADC", "Channel selector");

	this->addOperation("setInitialENC", &S626_task::setInitialENC,
			this, RTT::OwnThread).doc(
			"Sets which ENC channel should be read")
			.arg("InitialENC", "Channel selector");

	this->ports()->addPort("DIOInputPortWrite", DIOInputPortWrite).doc(
			"Input Port for DIO write.");

	this->ports()->addPort("DIOOutputPortRead", DIOOutputPortRead).doc(
			"Output Port for DIO read.");

	this->ports()->addPort("DACInputPort", DACInputPort).doc(
			"Input Port for DAC.");

	this->ports()->addPort("ADCOutputPort", ADCOutputPort).doc(
			"Output Port for ADC.");

	this->ports()->addPort("ENCOutputPort", ENCOutputPort).doc(
			"Output Port for ENC.");

	this->addOperation("prepareDriver", &S626_task::prepareDriver, this,
			RTT::OwnThread).doc("Prepare driver").arg("Device",
			"Analogy device, ex. analogy0").arg("Bus", "Bus number").arg("Slot",
			"Slot number");

	SelectedADCChannels = 0;
	SelectedENCChannels = 0;

	//create thread
	Interface = new Interface_thread(ORO_SCHED_RT, 10, 0.001, 1,
			"SensorayInterface");

	std::cout << "S626_task constructed !" << std::endl;

}

bool S626_task::configureHook() {
	std::cout << "S626_task configured !" << std::endl;
	return true;
}

bool S626_task::startHook() {

	Interface->start();

	std::cout << "Driver prepared, s626 ready\n" << "S626_task started !\n";

	return true;
}

int S626_task::prepareDriver(std::string nDevice, int nBus, int nSlot) {
	Device = nDevice;
	Bus = nBus;
	Slot = nSlot;

	Interface->resetDriver(nDevice, nBus, nSlot);

	return 0;
}

void S626_task::updateHook() {
	//std::cout << "S626_task executes updateHook !" <<std::endl;

	std::vector<int> DataDAC;
	std::vector<int> DataDIO;
	std::vector<int> v;
	int Channels, Banks;

	//check if there is new data on port for DIO write

	for (int k = 0; k < 15; ++k) {
		if (DIOInputPortWrite.read(DataDIO) == RTT::NewData) {
			Banks = DataDIO[0];

			for (int i = 0, j = 0; i < 3; ++i) {
				if (Banks & (1 << i)) {
					Interface->setDIO(i, DataDIO[1 + j], DataDIO[1 + j + 1]);

					j += 2;
				}
			}
		} else
			break;
	}

	for (int k = 0; k < 15; ++k) {
		if (DACInputPort.read(DataDAC) == RTT::NewData) {
			Channels = DataDAC[0];

			for (unsigned int i = 0, j = 1; i < 4; ++i) {
				if (Channels & (1 << i)) {
					Interface->setDAC(i, DataDAC[j]);
					++j;
				}
			}
		} else
			break;
	}

	//read data from interface and redirect it to output ports
	//dio
	v.clear();
	for(int i = 0; i < 3; ++i)
	{
		v.push_back(Interface->getDIO(i));
	}
	DIOOutputPortRead.write(v);

	//adc
	v.clear();
	for(int i = 0; i < 16; ++i)
	{
		if(SelectedADCChannels & (1 << i))
		{
			v.push_back(Interface->getADC(i));
		}
	}
	ADCOutputPort.write(v);

	//enc
	v.clear();
	for(int i = 0; i < 6; ++i)
	{
		if(SelectedENCChannels & (1 << i))
		{
			v.push_back(Interface->getENC(i));
		}
	}
	ENCOutputPort.write(v);

}

void S626_task::stopHook() {
	std::cout << "S626_task executes stopping !" << std::endl;

	Interface->stop();
}

void S626_task::cleanupHook() {
	std::cout << "S626_task cleaning up !" << std::endl;

	Interface->stopDriver();

	delete Interface;
}

void S626_task::setActivePublishing(int state) {
	Interface->setActivePublishing(state);
}

void S626_task::setInitialDIO( std::vector<double> InitialDIO)
{
	std::vector <int> tmp;

	if(InitialDIO.size() == 6)
	{
		for(int i = 0; i < 6; ++i)
		{
			tmp.push_back((int)InitialDIO[i]);
		}

		Interface->setInitialDIO(tmp);
	}
	else
		std::cout << "Please initialize DIO with 6 numbers\n";
}

void S626_task::setInitialADC( int InitialADC)
{
	SelectedADCChannels = InitialADC;
	Interface->setInitialADC(InitialADC);
}

void S626_task::setInitialENC( int InitialENC)
{
	SelectedENCChannels = InitialENC;
	Interface->setInitialENC(InitialENC);
}

int S626_task::readDIO(int bank) {
	if (bank >= 0 && bank <= 2)
		return Interface->getDIO(bank);
	else {
		std::cout << "Bad bank number, please enter value 0-2\n";
		return -1;
	}
}

void S626_task::writeDIO(int bank, int mask, int value) {
	if (bank >= 0 && bank <= 2)
		Interface->setDIO(bank, mask, value);
	else {
		std::cout << "Bad bank number, please enter value 0-2\n";
	}
}

int S626_task::readADC(int channel) {
	if (channel >= 0 && channel <= 15)
		return Interface->getADC(channel);
	else {
		std::cout << "Bad channel number, please enter value 0-15\n";
		return -1;
	}

}

void S626_task::setrangeADC(int mask, int value) {
	Interface->setrangeADC(mask, value);
}

void S626_task::writeDAC(int channel, int value) {
	if (channel >= 0 && channel <= 3) {
		if (value < 0) {
			std::cout << "Bad value, can't be negative. Setting to 0\n";
			value = 0;
		} else if (value > 0x3FFF) {
			std::cout
					<< "Bad value, can't be grater than 0x3FFF. Setting to 0x3FFF\n";
			value = 0x3FFF;
		}
		Interface->setDAC(channel, value);
	} else {
		std::cout << "Bad channel number, please enter value 0-3\n";
	}

}

int S626_task::prepareAllENC(void) {
	return Interface->prepareENC();
}

int S626_task::readENC(int enc) {
	if (enc >= 0 && enc <= 5)
		return Interface->getENC(enc);
	else {
		std::cout << "Bad encoder number, please enter value 0-5\n";
		return -1;
	}
}

int S626_task::getLastError(void) {
	int lastError;
	lastError = err;
	err = 0;

	return lastError;
}

/*
 * Using this macro, only one component may live
 * in one library *and* you may *not* link this library
 * with another component library. Use
 * ORO_CREATE_COMPONENT_TYPE()
 * ORO_LIST_COMPONENT_TYPE(S626_task)
 * In case you want to link with another library that
 * already contains components.
 *
 * If you have put your component class
 * in a namespace, don't forget to add it here too:
 */
ORO_CREATE_COMPONENT(S626_task)
