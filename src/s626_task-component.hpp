/**
 * \file s626_task-component.hpp
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

#ifndef OROCOS_S626_TASK_COMPONENT_HPP
#define OROCOS_S626_TASK_COMPONENT_HPP

#include <rtt/RTT.hpp>

#include <vector>

#include "S626API.h"

#include "Interface-thread.hpp"

#include <rtt/os/Mutex.hpp>

/**
 * \brief S626_task
 *
 * Class for S626_task OROCOS component
 *
 * Implements the interface for handling Sensoray 626 I/O board
 */
class S626_task : public RTT::TaskContext{
  public:

    S626_task(std::string const& name);

    bool configureHook();
    bool startHook();
    void updateHook();
    void stopHook();
    void cleanupHook();

    /**
     * \brief readDIO
     *
     * Reads digital inputs from specific bank.
     * The read value stores all the 16 states on digital
     * input port
     *
     * \param[in] bank		Bank indicator
     *
     * \return		The state of input ports on the bank \link
     *						bank bank \endlink
     */
    int readDIO( int bank);

    /**
     * \brief writeDIO
     *
     * Writes specific state to digital output ports
     *
     * \param[in] bank 		Bank indicator
     * \param[in] mask  	Selection mask
     * \param[in]	value		Value which should be written to digital
     * 										output ports that are selected by mask,
     * 										on the bank \link bank bank \endlink
     */
    void writeDIO( int bank, int mask, int value);

    /**
     * \brief readADC
     *
     * Reads analog value on specific channel.
     *
     * \param[in]	channel		Channel indicator
     * 											Accepted value from 0 to 15
     *
     * \return		The value on the channel \link channel channel \endlink
     * 						The value in range from 0 to 2^14 - 1
     */
    int readADC( int channel);

    /**
     * \brief setrangeADC
     *
     * Sets range of ADC on +/-5V or +/-10V.
     *
     * \param[in] mask		Mask defining which channels should be affected
     * \param[in]	value		Range settings. '1's are +/-10V and '0's are +/-5V
     */
    void setrangeADC( int mask, int value);

    /**
     * \brief writeDAC
     *
     * Writes the set value to DAC's channel
     *
     * \param[in]		channel 	Channel indicator
     * \param[in]		value 		Set value
     * 												Accepted value from 0 to 2^14 -1
     */
    void writeDAC( int channel, int value);

    /**
     * \brief readENC
     *
     * Returns the value stored in the register of the encoders
     * \link enc enc \endlink
     *
     * \param[in]		enc		Encoder channel
     * 										Accepted value from 0 to 5
     *
     * \return			Value of encoder's counter
     */
    int readENC( int enc);

    /**
     * \brief	prepareAllENC
     *
     * Prepares all the encoders for work
     *
     * \return			0			When everything was performed without error
     * 						!=0			Otherwise
     */
    int prepareAllENC( void);

    /**
     * \brief getLastError
     *
     * Return standard error code. For more informations please look
     * into standard linux kernel errors.
     *
     * \return error code
     */
    int getLastError( void);

    /**
     * \brief prepareDriver
     *
     * Prepares driver for work.
     *
     * \param[in]	Device		Name of analogy device ex. analogy0, analogy1, ...
     * \param[in] Bus				Bus number on which s626 is physically available
     * \param[in] Slot			Slot number on which s626 is physically available
     *
     * \return		0					Component and card is ready to use
     * 					!=0					Last error code. For more information look at dmesg output and
     * 											getLastError to obtain last error code.
     */
    int prepareDriver( std::string Device, int Bus, int Slot);

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
     * 											Use logic alternative to choose which
     * 											data should be published.
     * 											To enable ADC and ENC send activity to 3 (0b0011).
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
     *
     */
    void setInitialDIO( std::vector<double> InitialDIO);

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

    /**
     * Holds lat error code
     */
    int err;

    std::string Device;
    int Bus;
    int Slot;
    int state;

    Interface_thread * Interface;

    int SelectedADCChannels;
    int SelectedENCChannels;

    /**
     * \brief DIOInputPortWrite
     *
     * Input port for writing to the DIO.
     *
     * Transmitted value has a form of a vector.
     * The element with the index 0 of this vector has
     * a sense of bank selector.
     * The elements (i, i+1) where i = 1, 3, 5, ...
     * forms logical pair, where the first element of
     * the pair is a mask for the selected bank while the
     * second is the encoded state for digital
     * outputs in the selected bank.
     *
     * Example
     * Bank 0. Set the bit 0 to 0
     * 				 and the bit 3 to 1
		 * Bank 2. Set the bits 0-7  to 1
		 * 				 and the bits 8-15 to 0
		 *
		 * The vector to be transmitted by \link DIOInputPortWrite
		 * DIOInputPortWrite \endlink is as follows
     * No of element in vector | Value
     * 1st                     | 0x0005
     * 2nd                     | 0x0005
     * 3rd                     | 0x0004
     * 4th                     | 0xFFFF
     * 5th                     | 0x00FF
     */
    RTT::InputPort <std::vector<int> > DIOInputPortWrite;

    /**
     * \brief DIOOutputPortRead
     *
     * Output port for reading selected DIO
     * banks' values.
     *
     * Each element in the vector corresponds to the
     * value of particular DIO bank. Similarly to
     * the \link ADCOutputPort ADCOutputPort \endlink
     */
    RTT::OutputPort <std::vector<int> > DIOOutputPortRead;

    /**
     * \brief DACInputPort
     *
     * Input port for setting desired voltage on 4
     * available Sensoray's DAC ports.
     *
     * Transmitted value has form of a vector.
     * The element with the index 0 has sense of
     * the channel selector. The next consecutive
     * elements of this vector hold values
     * for selected channels in
     * ascending order.
     *
     * DAC port operates on +/- 10V range and theese values
     * have to be mapped into integer values from
     * 0 to 2^14 - 1. For example
     * int value | DAC voltage (V)
     * 0         |  -10
     * 2^13      |   0
     * 2^14 - 1  |  +10
     *
     * Example Channel 0 set to -10V
     * 				 Channel 2 set to   0V
     * No of element in vector | Value
     * 1st                     | 0x05
     * 2nd                     | 0x0000
     * 3rd                     | 0x2000
     */
    RTT::InputPort <std::vector<int> > DACInputPort;

    /**
     * \brief ADCOutputPort
     *
     * Output port holding ADC values of desired channels.
     *
     * Assume that it was required to read channels 0, 1 and 5.
     * After Sensoray 626 finishes the process this
     * component will send a vector of integers. Where
     * first element of this vector is filled with the
     * value of the first desired channel.
     *
     * Example, it was requested to read channels 0, 1 and 5.
     * Following vector will be send:
     * No of element in vector | Corresponding channel
     * 1st                     | 0
     * 2nd                     | 1
     * 3rd                     | 5
     */
    RTT::OutputPort <std::vector<int> > ADCOutputPort;

    /**
     * \brief ENCOutputPort
     *
     * Output port with desired readings of selected encoder's
     * channels.
     *
     * Similarly, to the \link ADCOutputPort
     * ADCOutputPort \endlink the same
     * procedure holds.
     */
    RTT::OutputPort <std::vector<int> > ENCOutputPort;

};
#endif
