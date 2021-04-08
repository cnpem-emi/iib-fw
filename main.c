
/////////////////////////////////////////////////////////////////////////////////////////////

#include <adc_internal.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include "stdlib.h"
#include <stdio.h>
#include "inc/hw_can.h"
#include "inc/hw_ints.h"
#include "inc/hw_memmap.h"
#include "inc/hw_sysctl.h"
#include "inc/hw_gpio.h"
#include "inc/hw_timer.h"
#include "inc/hw_types.h"
#include "driverlib/can.h"
#include "driverlib/gpio.h"
#include "driverlib/interrupt.h"
#include "driverlib/pin_map.h"
#include "driverlib/rom.h"
#include "driverlib/rom_map.h"
#include "driverlib/systick.h"
#include "driverlib/debug.h"
#include "driverlib/sysctl.h"
#include "driverlib/uart.h"
#include "driverlib/timer.h"
#include "can_bus.h"
#include "adc_internal.h"
#include "application.h"
#include "board_drivers/hardware_def.h"
#include "peripheral_drivers/gpio/gpio_driver.h"
#include "peripheral_drivers/timer/timer.h"
#include "leds.h"
#include "output.h"
#include "input.h"
#include "BoardTempHum.h"
#include "ntc_isolated_i2c.h"
#include "pt100.h"
#include "task.h"
#include "iib_data.h"
#include "PWMSoftware.h"

#include <iib_modules/fap.h>
#include <iib_modules/fac_os.h>
#include <iib_modules/fac_is.h>
#include <iib_modules/fac_cmd.h>

/////////////////////////////////////////////////////////////////////////////////////////////

#define SYSCLOCK    120000000

/////////////////////////////////////////////////////////////////////////////////////////////

volatile static uint32_t millis = 0;

volatile static uint8_t can_timestamp_100ms = 0;

static uint32_t ui32SysClock;

/////////////////////////////////////////////////////////////////////////////////////////////

uint32_t SysCtlClockGetTM4C129(void)
{
    return ui32SysClock;
}

/////////////////////////////////////////////////////////////////////////////////////////////

void delay_ms(uint32_t time)
{
    volatile uint32_t temp = millis;
    while ((millis - temp) < time);
}

/////////////////////////////////////////////////////////////////////////////////////////////

// Usado para testes com leituras rapidas.

/*void IntTimerTestHandler(void)
{
    // Clear the timer interrupt.
    TimerIntClear(TIMER2_BASE, TIMER_TIMA_TIMEOUT);

    send_data_schedule();
}*/

/////////////////////////////////////////////////////////////////////////////////////////////

void IntTimer1msHandler(void)
{
    // Clear the timer interrupt.
    TimerIntClear(TIMER1_BASE, TIMER_TIMA_TIMEOUT);

    // var count delay function
    millis++;

    // can time stamp
    if(can_timestamp_100ms >= 10)
    {
        RunToggle();

        can_timestamp_100ms = 0;

        RunToggle();
    }
    else can_timestamp_100ms++;

    RunToggle();
    sample_adc();
    RunToggle();
    task_1_ms();
}

/////////////////////////////////////////////////////////////////////////////////////////////

void IntTimer100usHandler(void)
{
    // Clear the timer interrupt.
    TimerIntClear(TIMER0_BASE, TIMER_TIMA_TIMEOUT);

    RunToggle();
    task_100_us();
    RunToggle();
}

/////////////////////////////////////////////////////////////////////////////////////////////

// Usado para testes com leituras rapidas.

/*void Timer_Test_Init(void)
{
    // Disable timer 2 peripheral
    SysCtlPeripheralDisable(SYSCTL_PERIPH_TIMER2);

    // Reset timer 2 peripheral
    SysCtlPeripheralReset(SYSCTL_PERIPH_TIMER2);

    // Enable timer 2 peripheral
    SysCtlPeripheralEnable(SYSCTL_PERIPH_TIMER2);

    // Wait for the timer 2 peripheral to be ready.
    while(!SysCtlPeripheralReady(SYSCTL_PERIPH_TIMER2));

    // Disable the timer 2 module.
    TimerDisable(TIMER2_BASE, TIMER_A);

    // Configure the two 32-bit periodic timers.
    TimerConfigure(TIMER2_BASE, TIMER_CFG_PERIODIC);
    TimerLoadSet(TIMER2_BASE, TIMER_A, (SYSCLOCK / 142) - 1);

    // Setup the interrupts for the timer timeouts.
    IntEnable(INT_TIMER2A);
    TimerIntEnable(TIMER2_BASE, TIMER_TIMA_TIMEOUT);
    TimerIntRegister(TIMER2_BASE, TIMER_A, IntTimerTestHandler);
    IntPrioritySet(INT_TIMER2A, 1);

    // Enable the timer 2.
    TimerEnable(TIMER2_BASE, TIMER_A);
}*/

/////////////////////////////////////////////////////////////////////////////////////////////

void Timer_1ms_Init(void)
{
    // Disable timer 1 peripheral
    SysCtlPeripheralDisable(SYSCTL_PERIPH_TIMER1);

    // Reset timer 1 peripheral
    SysCtlPeripheralReset(SYSCTL_PERIPH_TIMER1);

    // Enable timer 1 peripheral
    SysCtlPeripheralEnable(SYSCTL_PERIPH_TIMER1);

    // Wait for the timer 1 peripheral to be ready.
    while(!SysCtlPeripheralReady(SYSCTL_PERIPH_TIMER1));

    // Disable the timer 1 module.
    TimerDisable(TIMER1_BASE, TIMER_A);

    // Configure the two 32-bit periodic timers.
    TimerConfigure(TIMER1_BASE, TIMER_CFG_PERIODIC);
    TimerLoadSet(TIMER1_BASE, TIMER_A, (SYSCLOCK / 1000) - 1);

    // Setup the interrupts for the timer timeouts.
    IntEnable(INT_TIMER1A);
    TimerIntEnable(TIMER1_BASE, TIMER_TIMA_TIMEOUT);
    TimerIntRegister(TIMER1_BASE, TIMER_A, IntTimer1msHandler);
    IntPrioritySet(INT_TIMER1A, 1);

    // Enable the timer 1.
    TimerEnable(TIMER1_BASE, TIMER_A);
}

/////////////////////////////////////////////////////////////////////////////////////////////

void Timer_100us_Init(void)
{
    // Disable timer 0 peripheral
    SysCtlPeripheralDisable(SYSCTL_PERIPH_TIMER0);

    // Reset timer 0 peripheral
    SysCtlPeripheralReset(SYSCTL_PERIPH_TIMER0);

    // Enable timer 0 peripheral
    SysCtlPeripheralEnable(SYSCTL_PERIPH_TIMER0);

    // Wait for the timer 0 peripheral to be ready.
    while(!SysCtlPeripheralReady(SYSCTL_PERIPH_TIMER0));

    // Disable the timer 0 module.
    TimerDisable(TIMER0_BASE, TIMER_A);

    // Configure the two 32-bit periodic timers.
    TimerConfigure(TIMER0_BASE, TIMER_CFG_PERIODIC);
    TimerLoadSet(TIMER0_BASE, TIMER_A, (SYSCLOCK / 10000) - 1);

    // Setup the interrupts for the timer timeouts.
    IntEnable(INT_TIMER0A);
    TimerIntEnable(TIMER0_BASE, TIMER_TIMA_TIMEOUT);
    TimerIntRegister(TIMER0_BASE, TIMER_A, IntTimer100usHandler);
    IntPrioritySet(INT_TIMER0A, 0);

    // Enable the timer 0.
    TimerEnable(TIMER0_BASE, TIMER_A);
}

/////////////////////////////////////////////////////////////////////////////////////////////

/**
 * main.c
 *
 */
int main(void)
{

    ui32SysClock = SysCtlClockFreqSet((SYSCTL_OSC_MAIN | SYSCTL_USE_PLL |
                                       SYSCTL_XTAL_25MHZ | SYSCTL_CFG_VCO_480), 120000000);

    pinout_config();

    init_control_framwork(&g_controller_iib);

    AdcsInit();

    //LEDs initialization
    LedsInit();

    //Digital Input initialization
    InputInit();

    //Digital Output initialization
    OutputInit();

    InitCan(ui32SysClock);

    Timer_1ms_Init();

    Timer_100us_Init();

/////////////////////////////////////////////////////////////////////////////////////////////

    // Usado para testes com leituras rapidas.

    //Timer_Test_Init();

/////////////////////////////////////////////////////////////////////////////////////////////

    //PWM1SoftwareInit();

    //PT100 channels initialization
    Pt100Init();

    //Rh & Board Temp sensors initialization
#if ((RhEnable && BoardTempEnable) == 1)

    RhBoardTempSenseInit();

#endif

    //ADS1014 with NTC 5K Igbt1 and Igbt2 initialization
#if ((TempIgbt1Enable || TempIgbt2Enable) == 1)

    NtcInit();

#endif

    //Led test
    LedPong();

/////////////////////////////////////////////////////////////////////////////////////////////

    //Block application to sign that CAN Address is out of range
    while(get_can_address() == 0 || get_can_address() >= 496)
    {
        //Blink bar
        LedBarBlink();
        delay_ms(40);
    }

    AppConfiguration();

    while(1)
    {
        Application();
        BoardTask();
    }

    return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////////





