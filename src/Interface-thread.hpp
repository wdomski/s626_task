/**
 * \file Interface-thread.hpp
 *
 * \author Wojciech Domski
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

#ifndef INTERFACE_THREAD_HPP
#define INTERFACE_THREAD_HPP

#include <string>

#include <rtt/RTT.hpp>
#include <rtt/os/main.h>
#include <rtt/os/Mutex.hpp>
#include <rtt/os/Thread.hpp>

#include "S626API.h"

#define INTERFACE_ACTIVITY_MASK_ADC 0x01
#define INTERFACE_ACTIVITY_MASK_ENC 0x02
#define INTERFACE_ACTIVITY_MASK_DIO 0x04

class Interface_thread: public RTT::os::Thread {
public:

	Interface_thread(int scheduler, int priority, double period,
			unsigned int cpu_affinity, std::string name);

	void step(void);

	int getDIO(int channel);

	void setDIO( int channel, int mask, int value);

	int getADC(int channel);

	void setDAC( int channel, int value);

	int getENC(int channel);

	void setrangeADC( int mask, int value);

	int prepareENC(void);

	int resetDriver(std::string Device, int Bus, int Slot);

	int stopDriver(void);

  /**
   * \brief setActivePublishing
   *
   * Sets if ADC, ENC and DIO should be read.
   *
   * \param[in]	State			Encoded state for activity.
   * 											state on bit 0 controls ADC
   * 											state on bit 1 controls ENC
   * 											state on bit 2 controls DIO
   *
   * 											To enable ADC and ENC send activity to 3 (0b0011)
   */
  void setActivePublishing( int state);

  /**
   * \brief setInitialDIO
   *
   * Sets initial configuration of DIO.
   * Also specifies which ports are inputs and which
   * are outputs. This can not be changed afterwards.
   *
   * \param[in]	InitialDIO		Vector of 6 integers.
   *								Configuration goes with groups of two integers
   *								Each group corresponds to corresponding bank.
   *								First element in the group is mask
   *								the second is the value.
   *								Always put 6 integers
   */
  void setInitialDIO( std::vector<int> InitialDIO);

  /**
   * \brief	setInitialADC
   *
   * Sets initial configuration of ADC
   *
   * \param[in]					Integer which is a channel
   * 								selector.
   * 								Only channels which were
   * 								chosen will be published.
   */
  void setInitialADC( int InitialADC);

  /**
   * \brief	setInitialENC
   *
   * Sets initial configuration of ENC
   *
   * \param[in]					Integer which is a channel
   * 								selector.
   * 								Only channels which were
   * 								chosen will be published.
   */
  void setInitialENC( int InitialENC);

private:

	int runLoop;

	RTT::os::Mutex mutexCard;
	RTT::os::Mutex mutexData;
	RTT::os::Mutex mutexActivity;
	RTT::os::Mutex mutexConfig;

	ts626 * s626;

	int err;

	int DIO[3];

	int ADC[16];

	int ENC[6];

	int DIO_config[6];

	int ADC_config;

	int ENC_config;

	int state;

};
#endif
