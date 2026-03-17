
/////////////////////////////////////////////////////////////////////////////////////////////

/*
 * can_bus.c
 *
 *  Created on: 30 de abr de 2021
 *      Author: rogerio.marcondeli
 */

/////////////////////////////////////////////////////////////////////////////////////////////

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include "inc/hw_can.h"
#include "inc/hw_ints.h"
#include "inc/hw_memmap.h"
#include "driverlib/can.h"
#include "driverlib/gpio.h"
#include "driverlib/interrupt.h"
#include "driverlib/pin_map.h"
#include "driverlib/sysctl.h"
#include "driverlib/uart.h"
#include "utils/uartstdio.h"
#include "can_bus.h"
#include "iib_data.h"
#include "BoardTempHum.h"
#include "input.h"
#include "application.h"
#include "adc_internal.h"
#include "leds.h"
#include "board_drivers/hardware_def.h"
#include "peripheral_drivers/gpio/gpio_driver.h"

#include <iib_modules/fap.h>
#include <iib_modules/fac_os.h>
#include <iib_modules/fac_is.h>
#include <iib_modules/fac_cmd.h>
#include <iib_modules/resonant_swls.h>

/////////////////////////////////////////////////////////////////////////////////////////////

//*****************************************************************************
//
// A global to keep track of the error flags that have been thrown so they may
// be processed. This is necessary because reading the error register clears
// the flags, so it is necessary to save them somewhere for processing.
//
//*****************************************************************************

volatile uint32_t g_ui32ErrFlag = 0;

/////////////////////////////////////////////////////////////////////////////////////////////

/******************************************************************************
 *                          Object Messages
 *****************************************************************************/

tCANMsgObject tx_message_data;

/////////////////////////////////////////////////////////////////////////////////////////////

tCANMsgObject rx_message_reset;

/////////////////////////////////////////////////////////////////////////////////////////////

volatile uint8_t message_data[MESSAGE_DATA_LEN] = {0};

/////////////////////////////////////////////////////////////////////////////////////////////

volatile uint8_t message_reset[MESSAGE_RESET_LEN] = {0};

/////////////////////////////////////////////////////////////////////////////////////////////

volatile uint8_t can_address = 0;
volatile uint8_t can_id      = 0;

/////////////////////////////////////////////////////////////////////////////////////////////

//*****************************************************************************
//
// This function is the interrupt handler for the CAN peripheral.  It checks
// for the cause of the interrupt, and maintains a count of all messages that
// have been transmitted.
//
//*****************************************************************************
void can_isr(void)
{
    uint32_t ui32Status;

    // Read the CAN interrupt status to find the cause of the interrupt
    ui32Status = CANIntStatus(CAN0_BASE, CAN_INT_STS_CAUSE);

    // If the cause is a controller status interrupt, then get the status
    if(ui32Status == CAN_INT_INTID_STATUS)
    {
        // Read the controller status.  This will return a field of status
        // error bits that can indicate various errors.  Error processing
        // is not done in this example for simplicity.  Refer to the
        // API documentation for details about the error status bits.
        // The act of reading this status will clear the interrupt.  If the
        // CAN peripheral is not connected to a CAN bus with other CAN devices
        // present, then errors will occur and will be indicated in the
        // controller status.
        ui32Status = CANStatusGet(CAN0_BASE, CAN_STS_CONTROL);

        //
        // Add ERROR flags to list of current errors. To be handled
        // later, because it would take too much time here in the
        // interrupt.
        //

        g_ui32ErrFlag |= ui32Status;
    }

/////////////////////////////////////////////////////////////////////////////////////////////

    // Check if the cause is message object 1, which what we are using for
    // sending messages.
    else if(ui32Status == MESSAGE_DATA_OBJ)
    {
    	// Getting to this point means that the TX interrupt occurred on
    	// message object 1, and the message TX is complete.
    	// Clear the message object interrupt.

    	CANIntClear(CAN0_BASE, MESSAGE_DATA_OBJ);

    	/* Tx object 1. Nothing to do for now. */

    	//
    	// Since a message was received, clear any error flags.
    	// This is done because before the message is received it triggers
    	// a Status Interrupt for RX complete. by clearing the flag here we
    	// prevent unnecessary error handling from happeneing
    	//

    	g_ui32ErrFlag = 0;
    }

/////////////////////////////////////////////////////////////////////////////////////////////

    // Check if the cause is message object 2, which what we are using for
    // receiving messages.
    else if(ui32Status == MESSAGE_RESET_OBJ)
    {
    	// Getting to this point means that the RX interrupt occurred on
    	// message object 2, and the message RX is complete.
    	// Clear the message object interrupt.

    	CANIntClear(CAN0_BASE, MESSAGE_RESET_OBJ);

    	handle_reset_message();

    	//
    	// Since a message was transmitted, clear any error flags.
    	// This is done because before the message is transmitted it triggers
    	// a Status Interrupt for TX complete. by clearing the flag here we
    	// prevent unnecessary error handling from happeneing
    	//

    	g_ui32ErrFlag = 0;
    }

/////////////////////////////////////////////////////////////////////////////////////////////

    // Otherwise, something unexpected caused the interrupt.
    // This should never happen.
    else
    {

        // Spurious interrupt handling can go here.

    }
}

/////////////////////////////////////////////////////////////////////////////////////////////

void InitCan(uint32_t ui32SysClock)
{
    // Disable CAN0 peripheral
    SysCtlPeripheralDisable(SYSCTL_PERIPH_CAN0);

    // Reset CAN0 peripheral
    SysCtlPeripheralReset(SYSCTL_PERIPH_CAN0);

    // Enable CAN0 peripheral
    SysCtlPeripheralEnable(SYSCTL_PERIPH_CAN0);

    // Wait for the CAN0 module to be ready.
    while(!SysCtlPeripheralReady(SYSCTL_PERIPH_CAN0));

	// Configure the GPIO pin muxing to select CAN0 functions for these pins.
    GPIOPinConfigure(GPIO_PA0_CAN0RX);
    GPIOPinConfigure(GPIO_PA1_CAN0TX);

    // Enable the alternate function on the GPIO pins.  The above step selects
    GPIOPinTypeCAN(GPIO_PORTA_BASE, GPIO_PIN_0 | GPIO_PIN_1);

    // Disable the CAN0 module.
    CANDisable(CAN0_BASE);

    // Initialize the CAN controller
    CANInit(CAN0_BASE);

    // Set up the bit rate for the CAN bus 1Mbps
    CANBitRateSet(CAN0_BASE, ui32SysClock, 1000000);

    // Enable interrupts on the CAN peripheral.
    CANIntEnable(CAN0_BASE, CAN_INT_MASTER | CAN_INT_ERROR | CAN_INT_STATUS);

    CANIntRegister(CAN0_BASE, &can_isr);

    IntPrioritySet(INT_CAN0, 1);

    // Disable auto-retry if no ACK-bit is received by the CAN controller.
    CANRetrySet(CAN0_BASE, 1);

    // Enable the CAN for operation.
    CANEnable(CAN0_BASE);

/////////////////////////////////////////////////////////////////////////////////////////////
    /*configuration sending messages*/
/////////////////////////////////////////////////////////////////////////////////////////////

    //message object 1
    tx_message_data.ui32MsgID           = 0;
    tx_message_data.ui32MsgIDMask       = 0;
    tx_message_data.ui32Flags           = (MSG_OBJ_TX_INT_ENABLE | MSG_OBJ_FIFO);
    tx_message_data.ui32MsgLen          = MESSAGE_DATA_LEN;

/////////////////////////////////////////////////////////////////////////////////////////////
    /*configuration receiving messages*/
/////////////////////////////////////////////////////////////////////////////////////////////

    //message object 2
    rx_message_reset.ui32MsgID          = MESSAGE_RESET_ID;
    rx_message_reset.ui32MsgIDMask      = 0xff;
    rx_message_reset.ui32Flags          = (MSG_OBJ_RX_INT_ENABLE | MSG_OBJ_USE_ID_FILTER | MSG_OBJ_FIFO);
    rx_message_reset.ui32MsgLen         = MESSAGE_RESET_LEN;

    CANMessageSet(CAN0_BASE, MESSAGE_RESET_OBJ, &rx_message_reset, MSG_OBJ_TYPE_RX);

/////////////////////////////////////////////////////////////////////////////////////////////

    // Module ID
    can_address = BoardAddressRead();

    if(can_address == 0) can_address = 1;

    switch(can_address)
    {
    	case 1:
    	{
    		can_id = ID_MODULE_1;
    		break;
    	}
    	case 2:
    	{
    		can_id = ID_MODULE_2;
    		break;
    	}
    	case 3:
    	{
    		can_id = ID_MODULE_3;
    		break;
    	}
    	case 4:
    	{
    		can_id = ID_MODULE_4;
    		break;
    	}
    	case 5:
    	{
    		can_id = ID_MODULE_5;
    		break;
    	}
    	case 6:
    	{
    		can_id = ID_MODULE_6;
    		break;
    	}
    	case 7:
    	{
    		can_id = ID_MODULE_7;
    		break;
    	}
    	case 8:
    	{
    		can_id = ID_MODULE_8;
    		break;
    	}
    	default:
    	{
    		break;
    	}

    }

}

/////////////////////////////////////////////////////////////////////////////////////////////

//*****************************************************************************
//
// Can ERROR handling. When a message is received if there is an erro it is
// saved to g_ui32ErrFlag, the Error Flag Set. Below the flags are checked
// and cleared. It is left up to the user to add handling fuctionality if so
// desiered.
//
// For more information on the error flags please see the CAN section of the
// microcontroller datasheet.
//
// NOTE: you may experience errors during setup when only one board is powered
// on. This is caused by one board sending signals and there not being another
// board there to acknoledge it. Dont worry about these errors, they can be
// disregarded.
//
//*****************************************************************************

void can_error_handler(void)
{
	// CAN controller has entered a Bus Off state.
	if(g_ui32ErrFlag & CAN_STATUS_BUS_OFF)
	{
		// Handle Error Condition here

		// Enable the CAN for operation.
		CANEnable(CAN0_BASE);

		// Clear CAN_STATUS_BUS_OFF Flag
		g_ui32ErrFlag &= ~(CAN_STATUS_BUS_OFF);
	}

//*****************************************************************************

	// CAN controller error level has reached warning level.
	if(g_ui32ErrFlag & CAN_STATUS_EWARN)
	{
		// Handle Error Condition here

		// Clear CAN_STATUS_EWARN Flag
		g_ui32ErrFlag &= ~(CAN_STATUS_EWARN);
	}

//*****************************************************************************

	// CAN controller error level has reached error passive level.
	if(g_ui32ErrFlag & CAN_STATUS_EPASS)
	{
		// Handle Error Condition here

		// Clear CAN_STATUS_EPASS Flag
		g_ui32ErrFlag &= ~(CAN_STATUS_EPASS);
	}

//*****************************************************************************

	// A message was received successfully since the last read of this status.
	if(g_ui32ErrFlag & CAN_STATUS_RXOK)
	{
		// Handle Error Condition here

		// Clear CAN_STATUS_RXOK Flag
		g_ui32ErrFlag &= ~(CAN_STATUS_RXOK);
	}

//*****************************************************************************

	// A message was transmitted successfully since the last read of this status.
	if(g_ui32ErrFlag & CAN_STATUS_TXOK)
	{
		// Handle Error Condition here

		// Clear CAN_STATUS_TXOK Flag
		g_ui32ErrFlag &= ~(CAN_STATUS_TXOK);
	}

//*****************************************************************************

	// This is the mask for the last error code field.
	if(g_ui32ErrFlag & CAN_STATUS_LEC_MSK)
	{
		// Handle Error Condition here

		// Clear CAN_STATUS_LEC_MSK Flag
		g_ui32ErrFlag &= ~(CAN_STATUS_LEC_MSK);
	}

//*****************************************************************************

	// There was no error.
	if(g_ui32ErrFlag & CAN_STATUS_LEC_NONE)
	{
		// Handle Error Condition here

		// Clear CAN_STATUS_LEC_NONE Flag
		g_ui32ErrFlag &= ~(CAN_STATUS_LEC_NONE);
	}

//*****************************************************************************

	// A bit stuffing error has occurred.
	if(g_ui32ErrFlag & CAN_STATUS_LEC_STUFF)
	{
		// Handle Error Condition here

		// Clear CAN_STATUS_LEC_STUFF Flag
		g_ui32ErrFlag &= ~(CAN_STATUS_LEC_STUFF);
	}

//*****************************************************************************

	// A formatting error has occurred.
	if(g_ui32ErrFlag & CAN_STATUS_LEC_FORM)
	{
		// Handle Error Condition here

		// Clear CAN_STATUS_LEC_FORM Flag
		g_ui32ErrFlag &= ~(CAN_STATUS_LEC_FORM);
	}

//*****************************************************************************

	// An acknowledge error has occurred.
	if(g_ui32ErrFlag & CAN_STATUS_LEC_ACK)
	{
		// Handle Error Condition here

		// Clear CAN_STATUS_LEC_ACK Flag
		g_ui32ErrFlag &= ~(CAN_STATUS_LEC_ACK);
	}

//*****************************************************************************

	// The bus remained a bit level of 1 for longer than is allowed.
	if(g_ui32ErrFlag & CAN_STATUS_LEC_BIT1)
	{
		// Handle Error Condition here

		// Clear CAN_STATUS_LEC_BIT1 Flag
		g_ui32ErrFlag &= ~(CAN_STATUS_LEC_BIT1);
	}

//*****************************************************************************

	// The bus remained a bit level of 0 for longer than is allowed.
	if(g_ui32ErrFlag & CAN_STATUS_LEC_BIT0)
	{
		// Handle Error Condition here

		// Clear CAN_STATUS_LEC_BIT0 Flag
		g_ui32ErrFlag &= ~(CAN_STATUS_LEC_BIT0);
	}

//*****************************************************************************

	// A CRC error has occurred.
	if(g_ui32ErrFlag & CAN_STATUS_LEC_CRC)
	{
		// Handle Error Condition here

		// Clear CAN_STATUS_LEC_CRC Flag
		g_ui32ErrFlag &= ~(CAN_STATUS_LEC_CRC);
	}

//*****************************************************************************

	// This is the mask for the CAN Last Error Code (LEC).
	if(g_ui32ErrFlag & CAN_STATUS_LEC_MASK)
	{
		// Handle Error Condition here

		// Clear CAN_STATUS_LEC_MASK Flag
		g_ui32ErrFlag &= ~(CAN_STATUS_LEC_MASK);
	}

//*****************************************************************************

	// If there are any bits still set in g_ui32ErrFlag then something unhandled
    // has happened. Print the value of g_ui32ErrFlag.
    if(g_ui32ErrFlag !=0)
	{

    }
}

/////////////////////////////////////////////////////////////////////////////////////////////

void handle_reset_message(void)
{
    uint8_t id;

    rx_message_reset.pui8MsgData = (uint8_t*)message_reset;

    CANMessageGet(CAN0_BASE, MESSAGE_RESET_OBJ, &rx_message_reset, 0);

    id = message_reset[0];

    if (id == 1)
    {
        InterlockClear();

        AlarmClear();

        message_reset[0] = 0;
    }
}

/////////////////////////////////////////////////////////////////////////////////////////////

void send_data_message(uint8_t var)
{

/////////////////////////////////////////////////////////////////////////////////////////////

	// Error Handling
	if(g_ui32ErrFlag != 0)
	{
		can_error_handler();
	}

/////////////////////////////////////////////////////////////////////////////////////////////

#ifdef FAP

	switch(var)
	{
	case 0:
	{
		message_data[0] = g_controller_iib.iib_signals[0].u8[0]; // fap.Vin.f
		message_data[1] = g_controller_iib.iib_signals[0].u8[1]; //
		message_data[2] = g_controller_iib.iib_signals[0].u8[2]; //
		message_data[3] = g_controller_iib.iib_signals[0].u8[3]; //

		message_data[4] = g_controller_iib.iib_signals[1].u8[0]; // fap.Vout.f
		message_data[5] = g_controller_iib.iib_signals[1].u8[1]; //
		message_data[6] = g_controller_iib.iib_signals[1].u8[2]; //
		message_data[7] = g_controller_iib.iib_signals[1].u8[3]; //

		break;
	}
	case 1:
	{
		message_data[0] = g_controller_iib.iib_signals[2].u8[0]; // fap.IoutA1.f
		message_data[1] = g_controller_iib.iib_signals[2].u8[1]; //
		message_data[2] = g_controller_iib.iib_signals[2].u8[2]; //
		message_data[3] = g_controller_iib.iib_signals[2].u8[3]; //

		message_data[4] = g_controller_iib.iib_signals[3].u8[0]; // fap.IoutA2.f
		message_data[5] = g_controller_iib.iib_signals[3].u8[1]; //
		message_data[6] = g_controller_iib.iib_signals[3].u8[2]; //
		message_data[7] = g_controller_iib.iib_signals[3].u8[3]; //

		break;
	}
	case 2:
	{
		message_data[0] = g_controller_iib.iib_signals[6].u8[0]; // fap.DriverVoltage.f
		message_data[1] = g_controller_iib.iib_signals[6].u8[1]; //
		message_data[2] = g_controller_iib.iib_signals[6].u8[2]; //
		message_data[3] = g_controller_iib.iib_signals[6].u8[3]; //

		message_data[4] = g_controller_iib.iib_signals[11].u8[0]; // fap.GroundLeakage.f
		message_data[5] = g_controller_iib.iib_signals[11].u8[1]; //
		message_data[6] = g_controller_iib.iib_signals[11].u8[2]; //
		message_data[7] = g_controller_iib.iib_signals[11].u8[3]; //

		break;
	}
	case 3:
	{
		message_data[0] = g_controller_iib.iib_signals[7].u8[0]; // fap.Driver1Current.f
		message_data[1] = g_controller_iib.iib_signals[7].u8[1]; //
		message_data[2] = g_controller_iib.iib_signals[7].u8[2]; //
		message_data[3] = g_controller_iib.iib_signals[7].u8[3]; //

		message_data[4] = g_controller_iib.iib_signals[8].u8[0]; // fap.Driver2Current.f
		message_data[5] = g_controller_iib.iib_signals[8].u8[1]; //
		message_data[6] = g_controller_iib.iib_signals[8].u8[2]; //
		message_data[7] = g_controller_iib.iib_signals[8].u8[3]; //

		break;
	}
	case 4:
	{
		message_data[0] = g_controller_iib.iib_signals[4].u8[0]; // fap.TempIGBT1.f
		message_data[1] = g_controller_iib.iib_signals[4].u8[1]; //
		message_data[2] = g_controller_iib.iib_signals[4].u8[2]; //
		message_data[3] = g_controller_iib.iib_signals[4].u8[3]; //

		message_data[4] = g_controller_iib.iib_signals[5].u8[0]; // fap.TempIGBT2.f
		message_data[5] = g_controller_iib.iib_signals[5].u8[1]; //
		message_data[6] = g_controller_iib.iib_signals[5].u8[2]; //
		message_data[7] = g_controller_iib.iib_signals[5].u8[3]; //

		break;
	}
	case 5:
	{
		message_data[0] = g_controller_iib.iib_signals[9].u8[0]; // fap.TempL.f
		message_data[1] = g_controller_iib.iib_signals[9].u8[1]; //
		message_data[2] = g_controller_iib.iib_signals[9].u8[2]; //
		message_data[3] = g_controller_iib.iib_signals[9].u8[3]; //

		message_data[4] = g_controller_iib.iib_signals[10].u8[0]; // fap.TempHeatSink.f
		message_data[5] = g_controller_iib.iib_signals[10].u8[1]; //
		message_data[6] = g_controller_iib.iib_signals[10].u8[2]; //
		message_data[7] = g_controller_iib.iib_signals[10].u8[3]; //

		break;
	}
	case 6:
	{
		message_data[0] = g_controller_iib.iib_signals[12].u8[0]; // fap.BoardTemperature.f
		message_data[1] = g_controller_iib.iib_signals[12].u8[1]; //
		message_data[2] = g_controller_iib.iib_signals[12].u8[2]; //
		message_data[3] = g_controller_iib.iib_signals[12].u8[3]; //

		message_data[4] = g_controller_iib.iib_signals[13].u8[0]; // fap.RelativeHumidity.f
		message_data[5] = g_controller_iib.iib_signals[13].u8[1]; //
		message_data[6] = g_controller_iib.iib_signals[13].u8[2]; //
		message_data[7] = g_controller_iib.iib_signals[13].u8[3]; //

		break;
	}
	case 7:
	{
		message_data[0] = g_controller_iib.iib_signals[14].u8[0]; // fap Interlock
		message_data[1] = g_controller_iib.iib_signals[14].u8[1]; //
		message_data[2] = g_controller_iib.iib_signals[14].u8[2]; //
		message_data[3] = g_controller_iib.iib_signals[14].u8[3]; //

		message_data[4] = g_controller_iib.iib_signals[15].u8[0]; // fap Alarm
		message_data[5] = g_controller_iib.iib_signals[15].u8[1]; //
		message_data[6] = g_controller_iib.iib_signals[15].u8[2]; //
		message_data[7] = g_controller_iib.iib_signals[15].u8[3]; //

		break;
	}
	default:
	{
		break;
	}
	}

#endif

/////////////////////////////////////////////////////////////////////////////////////////////

#ifdef FAC_OS

	switch(var)
	{
	case 0:
	{
		message_data[0] = g_controller_iib.iib_signals[0].u8[0]; // fac_os.VdcLink.f
		message_data[1] = g_controller_iib.iib_signals[0].u8[1]; //
		message_data[2] = g_controller_iib.iib_signals[0].u8[2]; //
		message_data[3] = g_controller_iib.iib_signals[0].u8[3]; //

		message_data[4] = g_controller_iib.iib_signals[5].u8[0]; // fac_os.DriverVoltage.f
		message_data[5] = g_controller_iib.iib_signals[5].u8[1]; //
		message_data[6] = g_controller_iib.iib_signals[5].u8[2]; //
		message_data[7] = g_controller_iib.iib_signals[5].u8[3]; //

		break;
	}
	case 1:
	{
		message_data[0] = g_controller_iib.iib_signals[1].u8[0]; // fac_os.Iin.f;
		message_data[1] = g_controller_iib.iib_signals[1].u8[1]; //
		message_data[2] = g_controller_iib.iib_signals[1].u8[2]; //
		message_data[3] = g_controller_iib.iib_signals[1].u8[3]; //

		message_data[4] = g_controller_iib.iib_signals[2].u8[0]; // fac_os.Iout.f
		message_data[5] = g_controller_iib.iib_signals[2].u8[1]; //
		message_data[6] = g_controller_iib.iib_signals[2].u8[2]; //
		message_data[7] = g_controller_iib.iib_signals[2].u8[3]; //

		break;
	}
	case 2:
	{
		message_data[0] = g_controller_iib.iib_signals[6].u8[0]; // fac_os.Driver1Current.f
		message_data[1] = g_controller_iib.iib_signals[6].u8[1]; //
		message_data[2] = g_controller_iib.iib_signals[6].u8[2]; //
		message_data[3] = g_controller_iib.iib_signals[6].u8[3]; //

		message_data[4] = g_controller_iib.iib_signals[7].u8[0]; // fac_os.Driver2Current.f
		message_data[5] = g_controller_iib.iib_signals[7].u8[1]; //
		message_data[6] = g_controller_iib.iib_signals[7].u8[2]; //
		message_data[7] = g_controller_iib.iib_signals[7].u8[3]; //

		break;
	}
	case 3:
	{
		message_data[0] = g_controller_iib.iib_signals[3].u8[0]; // fac_os.TempIGBT1.f
		message_data[1] = g_controller_iib.iib_signals[3].u8[1]; //
		message_data[2] = g_controller_iib.iib_signals[3].u8[2]; //
		message_data[3] = g_controller_iib.iib_signals[3].u8[3]; //

		message_data[4] = g_controller_iib.iib_signals[4].u8[0]; // fac_os.TempIGBT2.f
		message_data[5] = g_controller_iib.iib_signals[4].u8[1]; //
		message_data[6] = g_controller_iib.iib_signals[4].u8[2]; //
		message_data[7] = g_controller_iib.iib_signals[4].u8[3]; //

		break;
	}
	case 4:
	{
		message_data[0] = g_controller_iib.iib_signals[8].u8[0]; // fac_os.TempL.f
		message_data[1] = g_controller_iib.iib_signals[8].u8[1]; //
		message_data[2] = g_controller_iib.iib_signals[8].u8[2]; //
		message_data[3] = g_controller_iib.iib_signals[8].u8[3]; //

		message_data[4] = g_controller_iib.iib_signals[9].u8[0]; // fac_os.TempHeatSink.
		message_data[5] = g_controller_iib.iib_signals[9].u8[1]; //
		message_data[6] = g_controller_iib.iib_signals[9].u8[2]; //
		message_data[7] = g_controller_iib.iib_signals[9].u8[3]; //

		break;
	}
	case 5:
	{
		message_data[0] = g_controller_iib.iib_signals[10].u8[0]; // fac_os.BoardTemperature.f
		message_data[1] = g_controller_iib.iib_signals[10].u8[1]; //
		message_data[2] = g_controller_iib.iib_signals[10].u8[2]; //
		message_data[3] = g_controller_iib.iib_signals[10].u8[3]; //

		message_data[4] = g_controller_iib.iib_signals[11].u8[0]; // fac_os.RelativeHumidity.f
		message_data[5] = g_controller_iib.iib_signals[11].u8[1]; //
		message_data[6] = g_controller_iib.iib_signals[11].u8[2]; //
		message_data[7] = g_controller_iib.iib_signals[11].u8[3]; //

		break;
	}
	case 6:
	{
		message_data[0] = g_controller_iib.iib_signals[12].u8[0]; // fac_os Interlock
		message_data[1] = g_controller_iib.iib_signals[12].u8[1]; //
		message_data[2] = g_controller_iib.iib_signals[12].u8[2]; //
		message_data[3] = g_controller_iib.iib_signals[12].u8[3]; //

		message_data[4] = g_controller_iib.iib_signals[13].u8[0]; // fac_os Alarm
		message_data[5] = g_controller_iib.iib_signals[13].u8[1]; //
		message_data[6] = g_controller_iib.iib_signals[13].u8[2]; //
		message_data[7] = g_controller_iib.iib_signals[13].u8[3]; //

		break;
	}
	default:
	{
		break;
	}
	}

#endif

/////////////////////////////////////////////////////////////////////////////////////////////

#ifdef FAC_IS

	switch(var)
	{
	case 0:
	{
		message_data[0] = g_controller_iib.iib_signals[0].u8[0]; // fac_is.VdcLink.f
		message_data[1] = g_controller_iib.iib_signals[0].u8[1]; //
		message_data[2] = g_controller_iib.iib_signals[0].u8[2]; //
		message_data[3] = g_controller_iib.iib_signals[0].u8[3]; //

		message_data[4] = g_controller_iib.iib_signals[3].u8[0]; // fac_is.DriverVoltage.f
		message_data[5] = g_controller_iib.iib_signals[3].u8[1]; //
		message_data[6] = g_controller_iib.iib_signals[3].u8[2]; //
		message_data[7] = g_controller_iib.iib_signals[3].u8[3]; //

		break;
	}
	case 1:
	{
		message_data[0] = g_controller_iib.iib_signals[1].u8[0]; // fac_is.Iin.f
		message_data[1] = g_controller_iib.iib_signals[1].u8[1]; //
		message_data[2] = g_controller_iib.iib_signals[1].u8[2]; //
		message_data[3] = g_controller_iib.iib_signals[1].u8[3]; //

		message_data[4] = g_controller_iib.iib_signals[4].u8[0]; // fac_is.Driver1Current.f
		message_data[5] = g_controller_iib.iib_signals[4].u8[1]; //
		message_data[6] = g_controller_iib.iib_signals[4].u8[2]; //
		message_data[7] = g_controller_iib.iib_signals[4].u8[3]; //

		break;
	}
	case 2:
	{
		message_data[0] = g_controller_iib.iib_signals[2].u8[0]; // fac_is.TempIGBT1.f
		message_data[1] = g_controller_iib.iib_signals[2].u8[1]; //
		message_data[2] = g_controller_iib.iib_signals[2].u8[2]; //
		message_data[3] = g_controller_iib.iib_signals[2].u8[3]; //

		message_data[4] = 0; // zero
		message_data[5] = 0; //
		message_data[6] = 0; //
		message_data[7] = 0; //

		break;
	}
	case 3:
	{
		message_data[0] = g_controller_iib.iib_signals[5].u8[0]; // fac_is.TempL.f
		message_data[1] = g_controller_iib.iib_signals[5].u8[1]; //
		message_data[2] = g_controller_iib.iib_signals[5].u8[2]; //
		message_data[3] = g_controller_iib.iib_signals[5].u8[3]; //

		message_data[4] = g_controller_iib.iib_signals[6].u8[0]; // fac_is.TempHeatSink.f
		message_data[5] = g_controller_iib.iib_signals[6].u8[1]; //
		message_data[6] = g_controller_iib.iib_signals[6].u8[2]; //
		message_data[7] = g_controller_iib.iib_signals[6].u8[3]; //

		break;
	}
	case 4:
	{
		message_data[0] = g_controller_iib.iib_signals[7].u8[0]; // fac_is.BoardTemperature.f
		message_data[1] = g_controller_iib.iib_signals[7].u8[1]; //
		message_data[2] = g_controller_iib.iib_signals[7].u8[2]; //
		message_data[3] = g_controller_iib.iib_signals[7].u8[3]; //

		message_data[4] = g_controller_iib.iib_signals[8].u8[0]; // fac_is.RelativeHumidity.f
		message_data[5] = g_controller_iib.iib_signals[8].u8[1]; //
		message_data[6] = g_controller_iib.iib_signals[8].u8[2]; //
		message_data[7] = g_controller_iib.iib_signals[8].u8[3]; //

		break;
	}
	case 5:
	{
		message_data[0] = g_controller_iib.iib_signals[9].u8[0]; // fac_is Interlock
		message_data[1] = g_controller_iib.iib_signals[9].u8[1]; //
		message_data[2] = g_controller_iib.iib_signals[9].u8[2]; //
		message_data[3] = g_controller_iib.iib_signals[9].u8[3]; //

		message_data[4] = g_controller_iib.iib_signals[10].u8[0]; // fac_is Alarm
		message_data[5] = g_controller_iib.iib_signals[10].u8[1]; //
		message_data[6] = g_controller_iib.iib_signals[10].u8[2]; //
		message_data[7] = g_controller_iib.iib_signals[10].u8[3]; //

		break;
	}
	default:
	{
		break;
	}
	}

#endif

/////////////////////////////////////////////////////////////////////////////////////////////

#ifdef FAC_CMD

	switch(var)
	{
	case 0:
	{
		message_data[0] = g_controller_iib.iib_signals[0].u8[0]; // fac_cmd.VcapBank.f
		message_data[1] = g_controller_iib.iib_signals[0].u8[1]; //
		message_data[2] = g_controller_iib.iib_signals[0].u8[2]; //
		message_data[3] = g_controller_iib.iib_signals[0].u8[3]; //

		message_data[4] = g_controller_iib.iib_signals[1].u8[0]; // fac_cmd.Vout.f
		message_data[5] = g_controller_iib.iib_signals[1].u8[1]; //
		message_data[6] = g_controller_iib.iib_signals[1].u8[2]; //
		message_data[7] = g_controller_iib.iib_signals[1].u8[3]; //

		break;
	}
	case 1:
	{
		message_data[0] = g_controller_iib.iib_signals[3].u8[0]; // fac_cmd.AuxCurrent.f
		message_data[1] = g_controller_iib.iib_signals[3].u8[1]; //
		message_data[2] = g_controller_iib.iib_signals[3].u8[2]; //
		message_data[3] = g_controller_iib.iib_signals[3].u8[3]; //

		message_data[4] = g_controller_iib.iib_signals[4].u8[0]; // fac_cmd.IdbCurrent.f
		message_data[5] = g_controller_iib.iib_signals[4].u8[1]; //
		message_data[6] = g_controller_iib.iib_signals[4].u8[2]; //
		message_data[7] = g_controller_iib.iib_signals[4].u8[3]; //

		break;
	}
	case 2:
	{
		message_data[0] = g_controller_iib.iib_signals[2].u8[0]; // fac_cmd.AuxIdbVoltage.f
		message_data[1] = g_controller_iib.iib_signals[2].u8[1]; //
		message_data[2] = g_controller_iib.iib_signals[2].u8[2]; //
		message_data[3] = g_controller_iib.iib_signals[2].u8[3]; //

		message_data[4] = g_controller_iib.iib_signals[5].u8[0]; // fac_cmd.GroundLeakage.f
		message_data[5] = g_controller_iib.iib_signals[5].u8[1]; //
		message_data[6] = g_controller_iib.iib_signals[5].u8[2]; //
		message_data[7] = g_controller_iib.iib_signals[5].u8[3]; //

		break;
	}
	case 3:
	{
		message_data[0] = g_controller_iib.iib_signals[6].u8[0]; // fac_cmd.TempL.f
		message_data[1] = g_controller_iib.iib_signals[6].u8[1]; //
		message_data[2] = g_controller_iib.iib_signals[6].u8[2]; //
		message_data[3] = g_controller_iib.iib_signals[6].u8[3]; //

		message_data[4] = g_controller_iib.iib_signals[7].u8[0]; // fac_cmd.TempHeatSink.f
		message_data[5] = g_controller_iib.iib_signals[7].u8[1]; //
		message_data[6] = g_controller_iib.iib_signals[7].u8[2]; //
		message_data[7] = g_controller_iib.iib_signals[7].u8[3]; //

		break;
	}
	case 4:
	{
		message_data[0] = g_controller_iib.iib_signals[8].u8[0]; // fac_cmd.BoardTemperature.f
		message_data[1] = g_controller_iib.iib_signals[8].u8[1]; //
		message_data[2] = g_controller_iib.iib_signals[8].u8[2]; //
		message_data[3] = g_controller_iib.iib_signals[8].u8[3]; //

		message_data[4] = g_controller_iib.iib_signals[9].u8[0]; // fac_cmd.RelativeHumidity.f
		message_data[5] = g_controller_iib.iib_signals[9].u8[1]; //
		message_data[6] = g_controller_iib.iib_signals[9].u8[2]; //
		message_data[7] = g_controller_iib.iib_signals[9].u8[3]; //

		break;
	}
	case 5:
	{
		message_data[0] = g_controller_iib.iib_signals[10].u8[0]; // fac_cmd Interlock
		message_data[1] = g_controller_iib.iib_signals[10].u8[1]; //
		message_data[2] = g_controller_iib.iib_signals[10].u8[2]; //
		message_data[3] = g_controller_iib.iib_signals[10].u8[3]; //

		message_data[4] = g_controller_iib.iib_signals[11].u8[0]; // fac_cmd Alarm
		message_data[5] = g_controller_iib.iib_signals[11].u8[1]; //
		message_data[6] = g_controller_iib.iib_signals[11].u8[2]; //
		message_data[7] = g_controller_iib.iib_signals[11].u8[3]; //

		break;
	}
	default:
	{
		break;
	}
	}

#endif

/////////////////////////////////////////////////////////////////////////////////////////////

#ifdef RES_SWLS

	switch(var)
	{
	case 0:
	{
		message_data[0] = g_controller_iib.iib_signals[0].u8[0]; // resonant_swls.Vin.f
		message_data[1] = g_controller_iib.iib_signals[0].u8[1]; //
		message_data[2] = g_controller_iib.iib_signals[0].u8[2]; //
		message_data[3] = g_controller_iib.iib_signals[0].u8[3]; //

		message_data[4] = g_controller_iib.iib_signals[1].u8[0]; // resonant_swls.Vout.f
		message_data[5] = g_controller_iib.iib_signals[1].u8[1]; //
		message_data[6] = g_controller_iib.iib_signals[1].u8[2]; //
		message_data[7] = g_controller_iib.iib_signals[1].u8[3]; //

		break;
	}
	case 1:
	{
		message_data[0] = g_controller_iib.iib_signals[2].u8[0]; // resonant_swls.Iin.f
		message_data[1] = g_controller_iib.iib_signals[2].u8[1]; //
		message_data[2] = g_controller_iib.iib_signals[2].u8[2]; //
		message_data[3] = g_controller_iib.iib_signals[2].u8[3]; //

		message_data[4] = g_controller_iib.iib_signals[3].u8[0]; // resonant_swls.Iout.f
		message_data[5] = g_controller_iib.iib_signals[3].u8[1]; //
		message_data[6] = g_controller_iib.iib_signals[3].u8[2]; //
		message_data[7] = g_controller_iib.iib_signals[3].u8[3]; //

		break;
	}
	case 2:
	{
		message_data[0] = g_controller_iib.iib_signals[4].u8[0]; // resonant_swls.TempHeatSinkMainBoard.f
		message_data[1] = g_controller_iib.iib_signals[4].u8[1]; //
		message_data[2] = g_controller_iib.iib_signals[4].u8[2]; //
		message_data[3] = g_controller_iib.iib_signals[4].u8[3]; //

		message_data[4] = g_controller_iib.iib_signals[5].u8[0]; // resonant_swls.TempHeatSinkTransformer.f
		message_data[5] = g_controller_iib.iib_signals[5].u8[1]; //
		message_data[6] = g_controller_iib.iib_signals[5].u8[2]; //
		message_data[7] = g_controller_iib.iib_signals[5].u8[3]; //

		break;
	}
	case 3:
	{
		message_data[0] = g_controller_iib.iib_signals[6].u8[0]; // resonant_swls.TempHeatSinkDiodes.f
		message_data[1] = g_controller_iib.iib_signals[6].u8[1]; //
		message_data[2] = g_controller_iib.iib_signals[6].u8[2]; //
		message_data[3] = g_controller_iib.iib_signals[6].u8[3]; //

		message_data[4] = g_controller_iib.iib_signals[7].u8[0]; // resonant_swls.TempOutputInductor.f
		message_data[5] = g_controller_iib.iib_signals[7].u8[1]; //
		message_data[6] = g_controller_iib.iib_signals[7].u8[2]; //
		message_data[7] = g_controller_iib.iib_signals[7].u8[3]; //

		break;
	}
	case 4:
	{
		message_data[0] = g_controller_iib.iib_signals[8].u8[0]; // resonant_swls.DriverAuxVoltage.f
		message_data[1] = g_controller_iib.iib_signals[8].u8[1]; //
		message_data[2] = g_controller_iib.iib_signals[8].u8[2]; //
		message_data[3] = g_controller_iib.iib_signals[8].u8[3]; //

		message_data[4] = g_controller_iib.iib_signals[11].u8[0]; // resonant_swls.GroundLeakage.f
		message_data[5] = g_controller_iib.iib_signals[11].u8[1]; //
		message_data[6] = g_controller_iib.iib_signals[11].u8[2]; //
		message_data[7] = g_controller_iib.iib_signals[11].u8[3]; //

		break;
	}
	case 5:
	{
		message_data[0] = g_controller_iib.iib_signals[9].u8[0]; // resonant_swls.Driver1Current.f
		message_data[1] = g_controller_iib.iib_signals[9].u8[1]; //
		message_data[2] = g_controller_iib.iib_signals[9].u8[2]; //
		message_data[3] = g_controller_iib.iib_signals[9].u8[3]; //

		message_data[4] = g_controller_iib.iib_signals[10].u8[0]; // resonant_swls.AuxCurrent.f
		message_data[5] = g_controller_iib.iib_signals[10].u8[1]; //
		message_data[6] = g_controller_iib.iib_signals[10].u8[2]; //
		message_data[7] = g_controller_iib.iib_signals[10].u8[3]; //

		break;
	}
	case 6:
	{
		message_data[0] = g_controller_iib.iib_signals[12].u8[0]; // resonant_swls.BoardTemperature.f
		message_data[1] = g_controller_iib.iib_signals[12].u8[1]; //
		message_data[2] = g_controller_iib.iib_signals[12].u8[2]; //
		message_data[3] = g_controller_iib.iib_signals[12].u8[3]; //

		message_data[4] = g_controller_iib.iib_signals[13].u8[0]; // resonant_swls.RelativeHumidity.f
		message_data[5] = g_controller_iib.iib_signals[13].u8[1]; //
		message_data[6] = g_controller_iib.iib_signals[13].u8[2]; //
		message_data[7] = g_controller_iib.iib_signals[13].u8[3]; //

		break;
	}
	case 7:
	{
		message_data[0] = g_controller_iib.iib_signals[14].u8[0]; // resonant_swls Interlock
		message_data[1] = g_controller_iib.iib_signals[14].u8[1]; //
		message_data[2] = g_controller_iib.iib_signals[14].u8[2]; //
		message_data[3] = g_controller_iib.iib_signals[14].u8[3]; //

		message_data[4] = g_controller_iib.iib_signals[15].u8[0]; // resonant_swls Alarm
		message_data[5] = g_controller_iib.iib_signals[15].u8[1]; //
		message_data[6] = g_controller_iib.iib_signals[15].u8[2]; //
		message_data[7] = g_controller_iib.iib_signals[15].u8[3]; //

		break;
	}
	default:
	{
		break;
	}
	}

#endif

/////////////////////////////////////////////////////////////////////////////////////////////

	tx_message_data.ui32MsgID = (can_id + var);

	tx_message_data.pui8MsgData = (uint8_t*)message_data;

	CANMessageSet(CAN0_BASE, MESSAGE_DATA_OBJ, &tx_message_data, MSG_OBJ_TYPE_TX);
}

/////////////////////////////////////////////////////////////////////////////////////////////

uint16_t get_can_address(void)
{
    return can_address;
}

/////////////////////////////////////////////////////////////////////////////////////////////




