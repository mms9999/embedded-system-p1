//*****************************************************************************
//
// hello.c - Simple hello world example.
//
// Copyright (c) 2012-2014 Texas Instruments Incorporated.  All rights reserved.
// Software License Agreement
// 
// Texas Instruments (TI) is supplying this software for use solely and
// exclusively on TI's microcontroller products. The software is owned by
// TI and/or its suppliers, and is protected under applicable copyright
// laws. You may not combine this software with "viral" open-source
// software in order to form a larger program.
// 
// THIS SOFTWARE IS PROVIDED "AS IS" AND WITH ALL FAULTS.
// NO WARRANTIES, WHETHER EXPRESS, IMPLIED OR STATUTORY, INCLUDING, BUT
// NOT LIMITED TO, IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
// A PARTICULAR PURPOSE APPLY TO THIS SOFTWARE. TI SHALL NOT, UNDER ANY
// CIRCUMSTANCES, BE LIABLE FOR SPECIAL, INCIDENTAL, OR CONSEQUENTIAL
// DAMAGES, FOR ANY REASON WHATSOEVER.
// 
// This is part of revision 2.1.0.12573 of the EK-TM4C123GXL Firmware Package.
//
//*****************************************************************************
//
// By Thien Nguyen and Philip Chan

#include <stdint.h>
#include <stdbool.h>
#include "inc/hw_memmap.h"
#include "inc/hw_types.h"
#include "inc/hw_gpio.h"


#include "driverlib/debug.h"
#include "driverlib/fpu.h"
#include "driverlib/gpio.h"
#include "driverlib/pin_map.h"
#include "driverlib/rom.h"
#include "driverlib/sysctl.h"
#include "driverlib/uart.h"
#include "utils/uartstdio.h"

//*****************************************************************************
//
//! \addtogroup example_list
//! <h1>Hello World (hello)</h1>
//!
//! A very simple ``hello world'' example.  It simply displays ``Hello World!''
//! on the UART and is a starting point for more complicated applications.
//!
//! UART0, connected to the Virtual Serial Port and running at
//! 115,200, 8-N-1, is used to display messages from this application.
//
//*****************************************************************************

//*****************************************************************************
//
// The error routine that is called if the driver library encounters an error.
//
//*****************************************************************************
#ifdef DEBUG
void
__error__(char *pcFilename, uint32_t ui32Line)
{
}
#endif

//*****************************************************************************
//
// Configure the UART and its pins.  This must be called before UARTprintf().
//
//*****************************************************************************
void
ConfigureUART(void)
{
    //
    // Enable the GPIO Peripheral used by the UART.
    //
    ROM_SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOA);

    //
    // Enable UART0
    //
    ROM_SysCtlPeripheralEnable(SYSCTL_PERIPH_UART0);

    //
    // Configure GPIO Pins for UART mode.
    //
    ROM_GPIOPinConfigure(GPIO_PA0_U0RX);
		ROM_GPIOPinConfigure(GPIO_PA1_U0TX);
    ROM_GPIOPinTypeUART(GPIO_PORTA_BASE, GPIO_PIN_0 | GPIO_PIN_1);

    //
    // Use the internal 16MHz oscillator as the UART clock source.
    //
    UARTClockSourceSet(UART0_BASE, UART_CLOCK_PIOSC);

    //
    // Initialize the UART for console I/O.
    //
    UARTStdioConfig(0, 115200, 16000000);
}

int main(void)
{
    // initialize inputs and outputs
    int sw_input;
    int leds;
    int main_light = 3; // green
    int side_light = 1; // red
    int cross_walk = 1; // don't walk
    // initialize flags
    int pushed = 0;
    int walk_flag = 0;
    int extend_flag = 0;
    
    //
    // Enable lazy stacking for interrupt handlers.
    ROM_FPULazyStackingEnable();

    // Set the clocking to run directly from the crystal.
    ROM_SysCtlClockSet(SYSCTL_SYSDIV_4 | SYSCTL_USE_PLL | SYSCTL_XTAL_16MHZ | SYSCTL_OSC_MAIN);

    // Enable the GPIO port that is used for the on-board LED.
    ROM_SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOF);

    HWREG(GPIO_PORTF_BASE + GPIO_O_LOCK) = GPIO_LOCK_KEY;
    HWREG(GPIO_PORTF_BASE + GPIO_O_CR) |= 0x01;
    HWREG(GPIO_PORTF_BASE + GPIO_O_LOCK) = 0;
        
    // Configure the switches as inputs
    ROM_GPIOPinTypeGPIOInput(GPIO_PORTF_BASE, GPIO_PIN_0 | GPIO_PIN_4); // PF0, PF4 = switches

        // Need pull-up resistors on switch inputs
        // Call this after pins configured as inputs
    
   ROM_GPIOPadConfigSet(GPIO_PORTF_BASE, GPIO_PIN_0 | GPIO_PIN_4, GPIO_STRENGTH_8MA, GPIO_PIN_TYPE_STD_WPU);

    // Configure the LEDs as outputs

    ROM_GPIOPinTypeGPIOOutput(GPIO_PORTF_BASE, GPIO_PIN_1 | GPIO_PIN_2 | GPIO_PIN_3);

    // initialize to LED red
    ROM_GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_1 | GPIO_PIN_2 | GPIO_PIN_3, GPIO_PIN_1);

    // Initialize the UART.
    ConfigureUART();

    // Indicate start of while loop
    UARTprintf("Starting Program...\n\n");


    while(1)
    {   
        // Print default status and LED red
        UARTprintf("Main: Green     Side: Red      Pedestrian: Don't Walk\n");
        ROM_GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_1, GPIO_PIN_1);
        
        // Loop while waiting for input
        for (int i=0;i<300;i++) // loop for 15s minimum
        {
            sw_input = ROM_GPIOPinRead(GPIO_PORTF_BASE, GPIO_PIN_0 | GPIO_PIN_4); // check switches
            if ((sw_input & 0x11) != 0x11) // check if any switch is pressed
            {
                if (pushed !=1) // if flag not set:
                {
                    pushed = 1; // set flag
                    if ((ROM_GPIOPinRead(GPIO_PORTF_BASE, GPIO_PIN_4)) == 0x10) // check if sw2 pushed
                    {    
                        walk_flag = 1;
                        UARTprintf("switch 2 detected\n");
                    }
                    else // assume switch 1 pressed
                        UARTprintf("switch 1 detected\n");
                }
            }
            ROM_SysCtlDelay(SysCtlClockGet()/3/20); // delay 0.05s: checks for input every 0.05s
        }
        
        // if switch not pushed yet, wait pushed to change state
        while (pushed == 0)
        {
            sw_input = ROM_GPIOPinRead(GPIO_PORTF_BASE, GPIO_PIN_0 | GPIO_PIN_4); // check switches
            if ((sw_input & 0x11) != 0x11) // check if any switch is pressed
            {
                pushed = 1; // set flag
                if ((ROM_GPIOPinRead(GPIO_PORTF_BASE, GPIO_PIN_4)) == 0x10) // check if sw2 pushed
                {
                    walk_flag = 1;
                    UARTprintf("switch 2 detected\n");
                }
                else
                {
                    UARTprintf("switch 1 detected\n");
                }
            }
        }

        // change Main street from green to red
        main_light = 4; // change to yellow
        UARTprintf("Main: Yellow    Side: Red         Pedestrian: Don't Walk\n");
        ROM_SysCtlDelay(SysCtlClockGet()); // Stay yellow 3 seconds
                        
        main_light = 1; // change to red
        UARTprintf("Main: Red       Side: Red      Pedestrian: Don't Walk\n");
        ROM_SysCtlDelay(SysCtlClockGet() /3/2); // Stay red for half a second
                        
        // Pedestrian walking 
        if (walk_flag == 1)
        {
            ROM_GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_1, 0);
            ROM_GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_3, GPIO_PIN_3);  // led green
            UARTprintf("Main: Red       Side: Green      Pedestrian: Walk\n");
            ROM_SysCtlDelay(SysCtlClockGet() *10/3); // Delay 10s
                            
            UARTprintf("Main: Red       Side: Green      Pedestrian: Don't Walk\n");
            ROM_SysCtlDelay(SysCtlClockGet() *5/3); //Delay 5s
            ROM_GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_1, GPIO_PIN_1);  // led yellow
              
            UARTprintf("Main: Red       Side: Yellow      Pedestrian: Don't Walk\n");
            ROM_SysCtlDelay(SysCtlClockGet()); //Delay 3s
                            
            ROM_GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_3, 0);   // led red
            UARTprintf("Main: Red       Side: Red      Pedestrian: Don't Walk\n");
            ROM_SysCtlDelay(SysCtlClockGet() /3/2); //Delay 0.5s
        }
        else  // Only side cars crossing
        {
            ROM_GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_1, 0);
            ROM_GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_3, GPIO_PIN_3);  // led green
            UARTprintf("Main: Red       Side: Green      Pedestrian: Don't Walk\n");
            // Green for 5s unless extended
            for (int i=0;i<100;i++) // loop for 5s
            {
                sw_input = ROM_GPIOPinRead(GPIO_PORTF_BASE, GPIO_PIN_0 | GPIO_PIN_4); // check switches
                if ((sw_input & 0x11) != 0x11) // check if switches pushed
                {
                    if (extend_flag != 1)
                    {
                        if ((ROM_GPIOPinRead(GPIO_PORTF_BASE, GPIO_PIN_0)) == 0x01) // check if sw1 pushed
                        {    
                            extend_flag = 1; // set flag to extend 5s
                            UARTprintf("switch 1 detected\n");
                        }
                    }
                }
                ROM_SysCtlDelay(SysCtlClockGet()/3/20); // delay 0.05s
            }
            
            if (extend_flag == 1) // extend 5s once
            {
                extend_flag = 0; // reset extension flag
                for (int i=0;i<100;i++) // loop for 5s
                {
                    sw_input = ROM_GPIOPinRead(GPIO_PORTF_BASE, GPIO_PIN_0 | GPIO_PIN_4); // check switches
                    if ((sw_input & 0x11) != 0x11) // check switches
                    {
                        if (extend_flag != 1)
                        {
                            if ((ROM_GPIOPinRead(GPIO_PORTF_BASE, GPIO_PIN_0)) == 0x01) // check if sw1 pushed
                            {    
                                extend_flag = 1;
                                UARTprintf("switch 1 detected again\n");
                            }
                        }
                    }
                    ROM_SysCtlDelay(SysCtlClockGet()/3/20); // delay 0.05s
                }
            }
                                
            if (extend_flag == 1) // extend another 5s
            {
                extend_flag = 0; // reset extension flag
                for (int i=0;i<100;i++) // loop for 5s
                {
                    sw_input = ROM_GPIOPinRead(GPIO_PORTF_BASE, GPIO_PIN_0 | GPIO_PIN_4); // check switches
                    if ((sw_input & 0x11) != 0x11) // check switches
                    {
                        if (extend_flag != 1)
                        {
                            if ((ROM_GPIOPinRead(GPIO_PORTF_BASE, GPIO_PIN_0)) == 0x01) // check if sw1 pushed
                            {    
                                extend_flag = 1;
                                UARTprintf("switch 1 detected, no extension\n");
                            }
                        }
                    }
                    ROM_SysCtlDelay(SysCtlClockGet()/3/20); // delay 0.05s
                }
            }
            
            
            // Transition side from green to red
            ROM_GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_1, GPIO_PIN_1); // led yellow
            UARTprintf("Main: Red       Side: Yellow      Pedestrian: Don't Walk\n");
            ROM_SysCtlDelay(SysCtlClockGet()); //Delay 3s
            ROM_GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_3, 0);  // led red
            UARTprintf("Main: Red       Side: Red      Pedestrian: Don't Walk\n");
            ROM_SysCtlDelay(SysCtlClockGet() / 3/2); // delay 1/2 seconds
                                 
        }
        
        // reset all flags
        extend_flag = 0;
        walk_flag = 0;
        pushed = 0;
                        
    }
}
