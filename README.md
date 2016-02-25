# OROCOS component for Sensoray 626 multi analog/digital I/O board

It is an OROCOS component which facilitates the features of s626 driver.
Each istance of OROCOS component can run different I/O board 
when multiple boards are available.

s626_task is an interface to Sensoray 626 I/O board allowing to control:

**ADC**, **DAC**, **ENC** and **DIO**

# Build

Place the component into your src directory inside you workspace.

Build it using command 

catkin_make --pkg s626_task

# Features

1.	Multiple I/O boards support.
2.	Separate thread for communication with the driver with 1kHz frequency.
3.	Enabling read from DIO, ENC and ADC peripherals selectable from interface.
4.	Channel selector for ADC.
5.	Setting default state on digital outputs.
6.	Setting default state on encoder channels.
7.	Setting range for ADC +/- 5V or +/- 10V.
8.	Queues for writing from multiple components to s626_task.

# Examples

## s626.ops

Example for using 1 I/O card.

## s626_multi2.ops

Example for using 2 I/O cards.

## s626_multi3.ops

Example for using 3 I/O cards.
