/**
 * \file Interface-thread.cpp
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

#include "Interface-thread.hpp"

Interface_thread::Interface_thread(int scheduler, int priority, double period,
		unsigned int cpu_affinity, std::string name) :
		Thread(scheduler, priority, period, cpu_affinity, name), s626(NULL), state(
				0) {
	for(int i = 0; i < 6; ++i)
	{
		DIO_config[i] = 0;
	}

	ADC_config = 0;
}

void Interface_thread::step(void) {
	int Datai;
	int Activity = 0;
	std::vector<int> Data;
	int tmp;

	/*
	 //time measure service

	 RTIME t1 = rt_timer_read();
	 */

	mutexActivity.lock();
	Activity = this->state;
	mutexActivity.unlock();

	if (Activity > 0) {

		mutexCard.lock();
		if (s626) {
			mutexCard.unlock();

			if (INTERFACE_ACTIVITY_MASK_DIO & Activity) {
				//read all DIO
				Data.clear();
				for (int i = 0; i < 3; ++i) {
					mutexCard.lock();
					err = s626_dio_read(s626, i + 2, 0, &Datai);
					mutexCard.unlock();

					if (err < 0) {
						std::cout << "error";
						return;
					}

					mutexData.lock();
					DIO[i] = Datai;
					mutexData.unlock();
				}
			}

			if (INTERFACE_ACTIVITY_MASK_ENC & Activity) {
				//read all ENC
				Data.clear();
				for (int i = 0; i < 6; ++i) {

					mutexConfig.lock();
					if( ENC_config & (1 << i))
					{
						mutexConfig.unlock();

						mutexCard.lock();
						err = s626_gpct_read_enc(s626, 5, i, &Datai);
						mutexCard.unlock();

						if (err < 0) {
							std::cout << "error";
							return;
						}

						mutexData.lock();
						ENC[i] = Datai;
						mutexData.unlock();

						//avoid next unlock of mutexConfig
						continue;
					}
					mutexConfig.unlock();

				}
			}

			if (INTERFACE_ACTIVITY_MASK_ADC & Activity) {
				//read ADC
				Data.clear();
				for (int i = 0; i < 16; ++i) {

					mutexConfig.lock();
					if( ADC_config & (1 << i))
					{
						mutexConfig.unlock();

						mutexCard.lock();
						err = s626_adc_read(s626, 0, i, (char *) (&Datai), 1);
						mutexCard.unlock();

						if (err < 0) {
							std::cout << "error";
							return;
						}

						mutexData.lock();
						ADC[i] = Datai & 0x3FFF;
						mutexData.unlock();

						//avoid next unlock of mutexConfig
						continue;
					}
					mutexConfig.unlock();

				}
			}

			/*
			 //time measure service

			 RTIME t2 = rt_timer_read();
			 std::cout << (float) (t2 - t1) / 1000.0 << " us\n";
			 */

		} else {
			mutexCard.unlock();
		}
	}

}

int Interface_thread::resetDriver(std::string Device, int Bus, int Slot) {

	err = stopDriver();

	mutexCard.lock();

	if (err < 0) {
		mutexCard.unlock();
		return -1;
	}

	char * DeviceChar;

	DeviceChar = new char[Device.size()];
	strcpy(DeviceChar, Device.c_str());

	std::cout << "On device " << DeviceChar << "\n" << "On bus " << Bus << "\n"
			<< "On slot " << Slot << "\n";

	s626 = s626_init("S626", DeviceChar);

	delete[] DeviceChar;

	err = s626_set_options(s626);
	if (err < 0) {
		mutexCard.unlock();
		return -2;
	}

	err = s626_set_bus(s626, Bus);
	if (err < 0) {
		mutexCard.unlock();
		return -3;
	}

	err = s626_set_slot(s626, Slot);
	if (err < 0) {
		mutexCard.unlock();
		return -4;
	}

	err = s626_open(s626);

	if (err < 0) {
		mutexCard.unlock();
		return -5;
	}

	mutexCard.unlock();

	return 0;
}

int Interface_thread::stopDriver(void) {

	mutexCard.lock();

	if (s626) {
		err = s626_close(s626);

		if (err < 0) {
			mutexCard.unlock();
			return -1;
		}

		s626_deinit(s626);

		s626 = NULL;
	}

	mutexCard.unlock();

	return 0;
}

int Interface_thread::getDIO(int channel) {
	mutexData.lock();
	int tmp = DIO[channel];
	mutexData.unlock();
	return tmp;
}

void Interface_thread::setDIO(int channel, int mask, int value) {
	static int c_mask;
	static int c_value;


	mutexConfig.lock();
	c_mask = DIO_config[channel * 2];
	c_value = DIO_config[channel * 2 + 1];
	mutexConfig.unlock();

	//mask is still the same

	//clear only the bits which are affected
	c_value = c_value & ~mask;
	//set new value of the bits
	c_value = (c_value | (value & mask)) & 0xFFFF;

	mutexConfig.lock();
	DIO_config[channel * 2] = c_mask;
	DIO_config[channel * 2 + 1] = c_value;
	mutexConfig.unlock();

	mutexCard.lock();
	err = s626_dio_write(s626, channel + 2, c_mask, c_value);
	mutexCard.unlock();
}

int Interface_thread::getADC(int channel) {
	mutexData.lock();
	int tmp = ADC[channel];
	mutexData.unlock();
	return tmp;
}

void Interface_thread::setDAC(int channel, int value) {
	char buffer[2];

	*(short int *) (&buffer[0]) = (short int) (value & 0xFFFF);

	mutexCard.lock();
	err = s626_dac_write(s626, 1, channel, &buffer[0]);
	mutexCard.unlock();
}

int Interface_thread::getENC(int channel) {
	mutexData.lock();
	int tmp = ENC[channel];
	mutexData.unlock();
	return tmp;
}

void Interface_thread::setrangeADC(int mask, int value) {
	mutexCard.lock();
	s626_adc_set_range(s626, mask, value);
	mutexCard.unlock();
}

int Interface_thread::prepareENC(void) {

	mutexCard.lock();
	if (s626) {
		mutexCard.unlock();
		for (int i = 0; i < 6; ++i) {
			mutexCard.lock();
			s626_gpct_conf_enc(s626, 5, i);
			mutexCard.unlock();
			mutexData.lock();
			ENC[i] = 0;
			mutexData.unlock();
		}
	} else {
		mutexCard.unlock();
		err = -100;
		return -1;
	}

	return 0;
}

void Interface_thread::setActivePublishing(int state) {
	mutexActivity.lock();
	this->state = state;
	mutexActivity.unlock();
}

void Interface_thread::setInitialDIO( std::vector<int> InitialDIO) {
	mutexData.lock();
	for(int i = 0; i < 6; ++i)
		DIO_config[i] = InitialDIO[i] & 0xFFFF;
	mutexData.unlock();

	mutexConfig.lock();
	mutexCard.lock();
	for( int i = 0; i < 3; ++i)
	{
		err = s626_dio_write(s626, i + 2,
				DIO_config[i*2] & 0xFFFF, DIO_config[i*2+1] & 0xFFFF);
	}
	mutexCard.unlock();
	mutexConfig.unlock();
}

void Interface_thread::setInitialADC( int InitialADC) {
	mutexConfig.lock();
	ADC_config = InitialADC;
	mutexConfig.unlock();
}

void Interface_thread::setInitialENC( int InitialENC) {
	mutexConfig.lock();
	ENC_config = InitialENC;
	mutexConfig.unlock();
}
