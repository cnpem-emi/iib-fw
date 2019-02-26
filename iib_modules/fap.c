/******************************************************************************
 * Copyright (C) 2017 by LNLS - Brazilian Synchrotron Light Laboratory
 *
 * Redistribution, modification or use of this software in source or binary
 * forms is permitted as long as the files maintain this copyright. LNLS and
 * the Brazilian Center for Research in Energy and Materials (CNPEM) are not
 * liable for any misuse of this material.
 *
 *****************************************************************************/

/**
 * @file fap.c
 * @brief Brief description of module
 * 
 * Detailed description
 *
 * @author allef.silva
 * @date 20/10/2018
 *
 */

#include <iib_modules/fap.h>
#include "iib_data.h"

#include "adc_internal.h"
#include "application.h"

#include "BoardTempHum.h"
#include "pt100.h"
#include "output.h"
#include "leds.h"
#include "can_bus.h"
#include "input.h"

#include <stdbool.h>
#include <stdint.h>

/**
 * TODO: Put here your defines. Just what is local. If you don't
 * need to access it from other module, consider use a constant (const)
 */
#define FAP_INPUT_OVERVOLTAGE_ALM                    50.0
#define FAP_INPUT_OVERVOLTAGE_ITLK                   55.0
#define FAP_OUTPUT_OVERVOLTAGE_ALM                   35.0
#define FAP_OUTPUT_OVERVOLTAGE_ITLK                  40.0
#define FAP_OUTPUT_OVERCURRENT_1_ALM                 105.0
#define FAP_OUTPUT_OVERCURRENT_1_ITLK                115.0
#define FAP_OUTPUT_OVERCURRENT_2_ALM                 105.0
#define FAP_OUTPUT_OVERCURRENT_2_ITLK                115.0

//#define FAP_INPUT_OVERVOLTAGE_ALM                    435.0
//#define FAP_INPUT_OVERVOLTAGE_ITLK                   440.0
//#define FAP_OUTPUT_OVERVOLTAGE_ALM                   230.0
//#define FAP_OUTPUT_OVERVOLTAGE_ITLK                  235.0
//#define FAP_OUTPUT_OVERCURRENT_1_ALM                 115.0
//#define FAP_OUTPUT_OVERCURRENT_1_ITLK                120.0
//#define FAP_OUTPUT_OVERCURRENT_2_ALM                 115.0
//#define FAP_OUTPUT_OVERCURRENT_2_ITLK                120.0

#define FAP_IGBT1_OVERTEMP_ALM                       00.0    // Not in use
#define FAP_IGBT1_OVERTEMP_ITLK                      00.0    // Not in use
#define FAP_IGBT2_OVERTEMP_ALM                       00.0    // Not in use
#define FAP_IGBT2_OVERTEMP_ITLK                      00.0    // Not in use
#define FAP_DRIVER_OVERVOLTAGE_ALM                   00.0    // Not in use
#define FAP_DRIVER_OVERVOLTAGE_ITLK                  00.0    // Not in use
#define FAP_DRIVER1_OVERCURRENT_ALM                  00.0    // Not in use
#define FAP_DRIVER1_OVERCURRENT_ITLK                 00.0    // Not in use
#define FAP_DRIVER2_OVERCURRENT_ALM                  00.0    // Not in use
#define FAP_DRIVER2_OVERCURRENT_ITLK                 00.0    // Not in use
#define FAP_INDUC_OVERTEMP_ALM                       50.0
#define FAP_INDUC_OVERTEMP_ITLK                      60.0
#define FAP_HS_OVERTEMP_ALM                          60
#define FAP_HS_OVERTEMP_ITLK                         80
#define FAP_RH_ALM                                   80
#define FAP_RH_ITLK                                  90
#define FAP_BOARD_TEMP_ALM                           80
#define FAP_BOARD_TEMP_ITLK                          90

typedef struct
{
    union {
        float   f;
        uint8_t u8[4];
    } Vin;

    bool VinAlarmSts;
    bool VinItlkSts;

    union {
        float   f;
        uint8_t u8[4];
    } Vout;

    bool VoutAlarmSts;
    bool VoutItlkSts;

    union {
        float   f;
        uint8_t u8[4];
    } IoutA1;

    bool IoutA1AlarmSts;
    bool IoutA1ItlkSts;

    union {
        float   f;
        uint8_t u8[4];
    } IoutA2;

    bool IoutA2AlarmSts;
    bool IoutA2ItlkSts;

    union {
        float   f;
        uint8_t u8[4];
    } TempIGBT1;

    bool TempIGBT1AlarmSts;
    bool TempIGBT1ItlkSts;
    bool TempIGBT1HwrItlk;
    bool TempIGBT1HwrItlkSts;

    union {
        float   f;
        uint8_t u8[4];
    } TempIGBT2;

    bool TempIGBT2AlarmSts;
    bool TempIGBT2ItlkSts;
    bool TempIGBT2HwrItlk;
    bool TempIGBT2HwrItlkSts;

    union {
        float   f;
        uint8_t u[4];
    } DriveVoltage;

    union {
        float   f;
        uint8_t u[4];
    } Drive1Current;

    union {
        float   f;
        uint8_t u[4];
    } Drive2Current;

    bool Driver1Error;
    bool Driver1ErrorItlk;
    bool Driver2Error;
    bool Driver2ErrorItlk;

    union {
        float   f;
        uint8_t u[4];
    } TempL;

    bool TempLAlarmSts;
    bool TempLItlkSts;

    union {
        float   f;
        uint8_t u[4];
    } TempHeatSink;

    bool TempHeatSinkAlarmSts;
    bool TempHeatSinkItlkSts;
    bool Relay;
    bool ExternalItlk;
    bool ExternalItlkSts;
    bool LeakageCurrent;
    bool LeakageCurrentSts;
    bool Rack;
    bool RackSts;
} fap_t;

/**
 * TODO: Put here your constants and variables. Always use static for 
 * private members.
 */

/**
 * TODO: Put here your function prototypes for private functions. Use
 * static in declaration.
 */
fap_t fap;
uint32_t fap_interlocks_indication   = 0;
uint32_t fap_alarms_indication       = 0;

static uint32_t itlk_id;
static uint32_t alarm_id;

static void get_itlks_id();
static void get_alarms_id();

/**
 * TODO: Put here the implementation for your public functions.
 */

void init_fap()
{
    //Set current range FAP 150 A
    CurrentCh1Init(130.0, 0.130, 50.0, 3); // Corrente bra�o1: Sensor Hall
    CurrentCh2Init(130.0, 0.130, 50.0, 3); // Corrente bra�o2: LEM LA 130-P

    //Set protection limits FAP 150 A
    //     These interlocks are bypassed due to the fact that their ADC's
    //     will most probably saturate during operation at 300 A. These
    //     measures are also performed by UDC, which guarantees these
    //     protections
    CurrentCh1AlarmLevelSet(FAP_OUTPUT_OVERCURRENT_1_ALM);  // Corrente bra�o1
    CurrentCh1TripLevelSet(FAP_OUTPUT_OVERCURRENT_1_ITLK);  // Corrente bra�o1
    CurrentCh2AlarmLevelSet(FAP_OUTPUT_OVERCURRENT_2_ALM);  // Corrente bra�o2
    CurrentCh2TripLevelSet(FAP_OUTPUT_OVERCURRENT_2_ITLK);  // Corrente bra�o2

    // NTC contiguration type
    //ConfigNtcType(SEMIX);

    //Leitura de tens�o isolada
    LvCurrentCh1Init(450.0, 0.025, 120.0, 3); // Tens�o de entrada
    LvCurrentCh2Init(250.0, 0.025, 120.0, 3); // Tens�o de sa�da

    LvCurrentCh1AlarmLevelSet(FAP_INPUT_OVERVOLTAGE_ALM);   // Tens�o de entrada Alarme
    LvCurrentCh1TripLevelSet(FAP_INPUT_OVERVOLTAGE_ITLK);   // Tens�o de entrada Interlock
    LvCurrentCh2AlarmLevelSet(FAP_OUTPUT_OVERVOLTAGE_ALM);  // Tens�o de sa�da Alarme
    LvCurrentCh2TripLevelSet(FAP_OUTPUT_OVERVOLTAGE_ITLK);  // Tens�o de sa�da Interlock

    // PT100 configuration limits
    Pt100SetCh1AlarmLevel(FAP_HS_OVERTEMP_ALM);     // Temperatura Dissipador
    Pt100SetCh1TripLevel(FAP_HS_OVERTEMP_ITLK);     // Temperatura Dissipador
    Pt100SetCh2AlarmLevel(FAP_INDUC_OVERTEMP_ALM);  // Temperatura L
    Pt100SetCh2TripLevel(FAP_INDUC_OVERTEMP_ITLK);  // Temperatura L

    // Delay 4 seconds
    Pt100SetCh1Delay(4);
    // Delay 4 seconds
    Pt100SetCh2Delay(4);

    // PT100 channel enable
    Pt100Ch1Enable(); // Temperatura Dissipador
    Pt100Ch2Enable(); // Temperatura L
    Pt100Ch3Disable();
    Pt100Ch4Disable();

    // Rh configuration limits
    RhAlarmLimitSet(FAP_RH_ALM);
    RhTripLimitSet(FAP_RH_ITLK);

    // Temp board configuration limits
    TempBoardAlarmLimitSet(FAP_BOARD_TEMP_ALM);
    TempBoardTripLimitSet(FAP_BOARD_TEMP_ITLK);

    Driver1ErrEnable();
    Driver2ErrEnable();

    // Init Variables
    fap.Vin.f                 = 0.0;
    fap.VinAlarmSts           = 0;
    fap.VinItlkSts            = 0;
    fap.Vout.f                = 0.0;
    fap.VoutAlarmSts          = 0;
    fap.VoutItlkSts           = 0;
    fap.IoutA1.f              = 0.0;
    fap.IoutA1AlarmSts        = 0;
    fap.IoutA1ItlkSts         = 0;
    fap.IoutA2.f              = 0.0;
    fap.IoutA2AlarmSts        = 0;
    fap.IoutA2ItlkSts         = 0;
    fap.TempIGBT1.f           = 0.0;
    fap.TempIGBT1AlarmSts     = 0;
    fap.TempIGBT1ItlkSts      = 0;
    fap.TempIGBT1HwrItlk      = 0;
    fap.TempIGBT1HwrItlkSts   = 0;
    fap.TempIGBT2.f           = 0.0;
    fap.TempIGBT2AlarmSts     = 0;
    fap.TempIGBT2ItlkSts      = 0;
    fap.TempIGBT2HwrItlk      = 0;
    fap.TempIGBT2HwrItlkSts   = 0;
    fap.DriveVoltage.f        = 0.0;
    fap.Drive1Current.f       = 0.0;
    fap.Drive2Current.f       = 0.0;
    fap.Driver1Error          = 0;
    fap.Driver1ErrorItlk      = 0;
    fap.Driver2Error          = 0;
    fap.Driver2ErrorItlk      = 0;
    fap.TempL.f               = 0;
    fap.TempLAlarmSts         = 0;
    fap.TempLItlkSts          = 0;
    fap.TempHeatSink.f        = 0;
    fap.TempHeatSinkAlarmSts  = 0;
    fap.TempHeatSinkItlkSts   = 0;
    fap.Relay                 = 0;
    fap.ExternalItlk          = 0;
    fap.ExternalItlkSts       = 0;
    fap.LeakageCurrent        = 0;
    fap.LeakageCurrentSts     = 0;
    fap.Rack                  = 0;
    fap.RackSts               = 0;
}

void clear_fap_interlocks()
{
    fap.VinItlkSts            = 0;
    fap.VoutItlkSts           = 0;
    fap.IoutA1ItlkSts         = 0;
    fap.IoutA2ItlkSts         = 0;
    fap.TempIGBT1ItlkSts      = 0;
    fap.TempIGBT1HwrItlkSts   = 0;
    fap.TempIGBT2ItlkSts      = 0;
    fap.TempIGBT2HwrItlkSts   = 0;
    fap.Driver1ErrorItlk      = 0;
    fap.Driver2ErrorItlk      = 0;
    fap.TempLItlkSts          = 0;
    fap.TempHeatSinkItlkSts   = 0;
    fap.ExternalItlkSts       = 0;
    fap.LeakageCurrentSts     = 0;
    fap.RackSts               = 0;

    itlk_id = 0;
}

uint8_t check_fap_interlocks()
{
    uint8_t test = 0;

    test |= fap.VinItlkSts;
    test |= fap.VoutItlkSts;
    test |= fap.IoutA1ItlkSts;
    test |= fap.IoutA2ItlkSts;
    test |= fap.TempIGBT1ItlkSts;
    test |= fap.TempIGBT1HwrItlkSts;
    test |= fap.TempIGBT2ItlkSts;
    test |= fap.TempIGBT2HwrItlkSts;
    test |= fap.Driver1ErrorItlk;
    test |= fap.Driver2ErrorItlk;
    test |= fap.TempLItlkSts;
    test |= fap.TempHeatSinkItlkSts;
    test |= fap.ExternalItlkSts;
    test |= fap.LeakageCurrentSts;
    test |= fap.RackSts;

    return test;
}

void clear_fap_alarms()
{
    fap.VinItlkSts             = 0;
    fap.VoutItlkSts            = 0;
    fap.IoutA1ItlkSts          = 0;
    fap.IoutA2ItlkSts          = 0;
    fap.TempIGBT1ItlkSts       = 0;
    fap.TempIGBT1HwrItlkSts    = 0;
    fap.TempIGBT2ItlkSts       = 0;
    fap.TempIGBT2HwrItlkSts    = 0;
    fap.Driver1ErrorItlk       = 0;
    fap.Driver2ErrorItlk       = 0;
    fap.TempLItlkSts           = 0;
    fap.TempHeatSinkItlkSts    = 0;
    fap.ExternalItlkSts        = 0;
    fap.LeakageCurrentSts      = 0;
    fap.RackSts                = 0;

    alarm_id = 0;
}

uint8_t check_fap_alarms()
{
    uint8_t test = 0;

    test |= fap.VinAlarmSts;
    test |= fap.VoutAlarmSts;
    test |= fap.IoutA1AlarmSts;
    test |= fap.IoutA2AlarmSts;
    test |= fap.TempIGBT1AlarmSts;
    test |= fap.TempIGBT2AlarmSts;
    test |= fap.TempLAlarmSts;
    test |= fap.TempHeatSinkAlarmSts;

    return test;
}

void check_fap_indication_leds()
{
    // Output over voltage
    if(fap.VoutItlkSts) Led2TurnOff();
    else if(fap.VoutAlarmSts) Led2Toggle();
    else Led2TurnOn();

    // Input over voltage
    if(fap.VinItlkSts) Led3TurnOff();
    else if(fap.VinAlarmSts) Led3Toggle();
    else Led3TurnOn();

    // Output over current
    if (fap.IoutA1ItlkSts || fap.IoutA2ItlkSts) Led4TurnOff();
    else if(fap.IoutA1AlarmSts || fap.IoutA2AlarmSts) Led4Toggle();
    else Led4TurnOn();

    // Over temperature
    if(fap.TempIGBT1ItlkSts || fap.TempIGBT2ItlkSts ||  fap.TempLItlkSts || fap.TempHeatSinkItlkSts || fap.TempIGBT1HwrItlkSts || fap.TempIGBT2HwrItlkSts) Led5TurnOff();
    else if(fap.TempIGBT1AlarmSts || fap.TempIGBT2AlarmSts ||  fap.TempLAlarmSts || fap.TempHeatSinkAlarmSts) Led5Toggle();
    else Led5TurnOn();

    if(fap.ExternalItlkSts) Led6TurnOff();
    else Led6TurnOn();

    if(fap.LeakageCurrentSts) Led7TurnOff();
    else Led7TurnOn();

    if(fap.RackSts) Led8TurnOff();
    else Led8TurnOn();

    if(fap.Driver1ErrorItlk || fap.Driver2ErrorItlk) Led9TurnOff();
    else if(!InterlockRead()) Led9TurnOn();

    if(InterlockRead()) Led10TurnOff();
    else Led10TurnOn();
}

void fap_application_readings()
{
    fap.TempHeatSink.f = (float) Pt100ReadCh1();//PT100 CH1
    fap.TempHeatSinkAlarmSts = Pt100ReadCh1AlarmSts();
    if(!fap.TempHeatSinkItlkSts)fap.TempHeatSinkItlkSts        = Pt100ReadCh1TripSts();

    fap.TempL.f = (float) Pt100ReadCh2();//PT100 CH2
    fap.TempLAlarmSts = Pt100ReadCh2AlarmSts();
    if(!fap.TempLItlkSts)fap.TempLItlkSts                      = Pt100ReadCh2TripSts();

    fap.TempIGBT1.f = 0.0;
    fap.TempIGBT1AlarmSts = 0;
    fap.TempIGBT1ItlkSts = 0;

    if(!fap.TempIGBT1HwrItlkSts) fap.TempIGBT1HwrItlkSts       = Driver1OverTempRead();

    fap.TempIGBT2.f = 0.0;
    fap.TempIGBT2AlarmSts = 0;
    fap.TempIGBT2ItlkSts = 0;

    if(!fap.TempIGBT2HwrItlkSts) fap.TempIGBT2HwrItlkSts       = Driver2OverTempRead();

    fap.IoutA1.f = CurrentCh1Read();//HALL CH1
    fap.IoutA1AlarmSts = CurrentCh1AlarmStatusRead();
    if(!fap.IoutA1ItlkSts)fap.IoutA1ItlkSts                    = CurrentCh1TripStatusRead();

    fap.IoutA2.f = CurrentCh2Read();//HALL CH2
    fap.IoutA2AlarmSts = CurrentCh2AlarmStatusRead();
    if(!fap.IoutA2ItlkSts)fap.IoutA2ItlkSts                    = CurrentCh2TripStatusRead();

    fap.Vin.f = LvCurrentCh1Read();
    fap.VinAlarmSts = LvCurrentCh1AlarmStatusRead();
    if(!fap.VinItlkSts)fap.VinItlkSts                          = LvCurrentCh1TripStatusRead();

    fap.Vout.f = LvCurrentCh2Read();
    fap.VoutAlarmSts = LvCurrentCh2AlarmStatusRead();
    if(!fap.VoutItlkSts)fap.VoutItlkSts                        = LvCurrentCh2TripStatusRead();

    //fap.ExternalItlk = Gpdi5Read();
    //if(!fap.ExternalItlkSts) fap.ExternalItlkSts               = Gpdi5Read();
    fap.ExternalItlk = Gpdi1Read();
    if(!fap.ExternalItlkSts) fap.ExternalItlkSts               = Gpdi1Read();

    //fap.LeakageCurrent = Gpdi6Read();
    //if(!fap.LeakageCurrentSts) fap.LeakageCurrentSts           = Gpdi6Read();
    fap.LeakageCurrent = Gpdi2Read();
    if(!fap.LeakageCurrentSts) fap.LeakageCurrentSts           = Gpdi2Read();

    //fap.Rack = Gpdi7Read();
    //if(!fap.RackSts) fap.RackSts                               = Gpdi7Read();
    fap.Rack = Gpdi3Read();
    if(!fap.RackSts) fap.RackSts                               = Gpdi3Read();

    fap.Relay = Gpdi4Read();

    fap.Driver1Error = Driver1TopErrRead();
    if(!fap.Driver1ErrorItlk) fap.Driver1ErrorItlk             = Driver1TopErrRead();

    fap.Driver2Error = Driver2TopErrRead();
    if(!fap.Driver2ErrorItlk) fap.Driver2ErrorItlk             = Driver2TopErrRead();

    if(fap.ExternalItlkSts || fap.Driver2ErrorItlk || fap.Driver2ErrorItlk) InterlockSet(); // If no signal over the port, then set Interlock action

    fap_map_vars();
    get_itlks_id();
    get_alarms_id();
}


void fap_power_on_check()
{
    if (Gpdi4Read()) {
        Led1TurnOff();
        ReleItlkTurnOff();
    }
    else {
        Led1TurnOn();
        ReleItlkTurnOn();
    }
}


void fap_map_vars()
{
    g_controller_iib.iib_signals[0].u32     = fap_interlocks_indication;
    g_controller_iib.iib_signals[1].u32     = fap_alarms_indication;
    g_controller_iib.iib_signals[2].f       = fap.Vin.f;
    g_controller_iib.iib_signals[3].f       = fap.Vout.f;
    g_controller_iib.iib_signals[4].f       = fap.IoutA1.f;
    g_controller_iib.iib_signals[5].f       = fap.IoutA2.f;
    g_controller_iib.iib_signals[6].f       = fap.TempIGBT1.f;
    g_controller_iib.iib_signals[7].f       = fap.TempIGBT2.f;
    g_controller_iib.iib_signals[8].f       = fap.DriveVoltage.f;
    g_controller_iib.iib_signals[9].f       = fap.Drive1Current.f;
    g_controller_iib.iib_signals[10].f      = fap.Drive2Current.f;
    g_controller_iib.iib_signals[11].f      = fap.TempL.f;
    g_controller_iib.iib_signals[12].f      = fap.TempHeatSink.f;
}

void send_fap_data()
{
    uint8_t i;
    for (i = 2; i < 13; i++) send_data_message(i);
}

static void get_itlks_id()
{
    if (fap.VinItlkSts)          itlk_id |= INPUT_OVERVOLTAGE_ITLK;
    if (fap.VoutItlkSts)         itlk_id |= OUTPUT_OVERVOLTAGE_ITLK;
    if (fap.IoutA1ItlkSts)       itlk_id |= OUTPUT_OVERCURRENT_1_ITLK;
    if (fap.IoutA2ItlkSts)       itlk_id |= OUTPUT_OVERCURRENT_2_ITLK;
    if (fap.TempIGBT1ItlkSts)    itlk_id |= IGBT1_OVERTEMP_ITLK;
    if (fap.TempIGBT2ItlkSts)    itlk_id |= IGBT2_OVERTEMP_ITLK;
    if (fap.Driver1ErrorItlk)    itlk_id |= DRIVER1_ERROR_ITLK;
    if (fap.Driver2ErrorItlk)    itlk_id |= DRIVER2_ERROR_ITLK;
    if (fap.TempLItlkSts)        itlk_id |= INDUC_OVERTEMP_ITLK;
    if (fap.TempHeatSinkItlkSts) itlk_id |= HS_OVERTEMP_ITLK;
    if (fap.Relay)               itlk_id |= RELAY_ITLK;
    if (fap.ExternalItlkSts)     itlk_id |= EXTERNAL_ITLK;
    if (fap.LeakageCurrentSts)   itlk_id |= LEAKAGE_CURRENT_ITLK;
    if (fap.RackSts)             itlk_id |= RACK_ITLK;
}

static void get_alarms_id()
{
    if (fap.VinAlarmSts)          alarm_id |= INPUT_OVERVOLTAGE_ALM;
    if (fap.VoutAlarmSts)         alarm_id |= OUTPUT_OVERVOLTAGE_ALM;
    if (fap.IoutA1AlarmSts)       alarm_id |= OUTPUT_OVERCURRENT_1_ALM;
    if (fap.IoutA2AlarmSts)       alarm_id |= OUTPUT_OVERCURRENT_2_ALM;
    if (fap.TempIGBT1AlarmSts)    alarm_id |= IGBT1_OVERTEMP_ALM;
    if (fap.TempIGBT2AlarmSts)    alarm_id |= IGBT2_OVERTEMP_ALM;
    if (fap.TempLAlarmSts)        alarm_id |= INDUC_OVERTEMP_ALM;
    if (fap.TempHeatSinkAlarmSts) alarm_id |= HS_OVERTEMP_ALM;
}

void send_output_fap_itlk_msg()
{
    //send_interlock_message(itlk_id);
    send_data_message(0);
}

float fap_vout_read(void)
{
    return fap.Vout.f;
}

unsigned char fap_vout_alarm_sts_read(void)
{
    return fap.VoutAlarmSts;
}

unsigned char fap_vout_itlk_sts_read(void)
{
    return fap.VoutItlkSts;
}

//**********************************************
float fap_vin_read(void)
{
    return fap.Vin.f;
}

unsigned char fap_vin_alarm_sts_read(void)
{
    return fap.VinAlarmSts;
}

unsigned char fap_vin_itlk_sts_read(void)
{
    return fap.VinItlkSts;
}

//**********************************************
float fap_iout_a1_read(void)
{
    return fap.IoutA1.f;
}

unsigned char fap_iout_a1_alarm_sts_read(void)
{
    return fap.IoutA1AlarmSts;
}

unsigned char fap_iout_a1_itlk_sts_read(void)
{
    return fap.IoutA1ItlkSts;
}

//**********************************************
float fap_iout_a2_read(void)
{
    return fap.IoutA2.f;
}

unsigned char fap_iout_a2_alarm_sts_read(void)
{
    return fap.IoutA2AlarmSts;
}

unsigned char fap_iout_a2_itlk_sts_read(void)
{
    return fap.IoutA2ItlkSts;
}

//**********************************************
unsigned char fap_temp_IGBT1_read(void)
{
    return fap.TempIGBT1.f;
}

unsigned char fap_temp_IGBT1_alarm_sts_read(void)
{
    return fap.TempIGBT1AlarmSts;
}

unsigned char fap_temp_IGBT1_itlk_sts_read(void)
{
    return fap.TempIGBT1ItlkSts;
}

unsigned char fap_temp_IGBT1_hwr_itlk_read(void)
{
    return fap.TempIGBT1HwrItlk;
}

unsigned char fap_temp_IGBT1_hwr_itlk_sts_read(void)
{
    return fap.TempIGBT1HwrItlkSts;
}

//**********************************************
unsigned char fap_temp_IGBT2_read(void)
{
    return fap.TempIGBT2.f;
}

unsigned char fap_temp_IGBT2_alarm_sts_read(void)
{
    return fap.TempIGBT2AlarmSts;
}

unsigned char fap_temp_IGBT2_itlk_sts_read(void)
{
    return fap.TempIGBT2ItlkSts;
}

unsigned char fap_temp_IGBT2_hwr_itlk_read(void)
{
    return fap.TempIGBT2HwrItlk;
}

unsigned char fap_temp_IGBT2_hwr_itlk_sts_read(void)
{
    return fap.TempIGBT2HwrItlkSts;
}

//**********************************************
unsigned char fap_temp_heatsink_read(void)
{
    return fap.TempHeatSink.f;
}

unsigned char fap_temp_heatsink_alarm_sts_read(void)
{
    return fap.TempHeatSinkAlarmSts;
}

unsigned char fap_temp_heatsink_itlk_sts_read(void)
{
    return fap.TempHeatSinkItlkSts;
}

//**********************************************
unsigned char fap_tempL_read(void)
{
    return fap.TempL.f;
}

unsigned char fap_tempL_alarm_sts_read(void)
{
    return fap.TempLAlarmSts;
}

unsigned char fap_tempL_itlk_sts_read(void)
{
    return fap.TempLItlkSts;
}

//**********************************************
unsigned char fap_relay_read(void)
{
    return fap.Relay;
}

//**********************************************
unsigned char fap_driver1_error_read(void)
{
    return fap.Driver1Error;
}

unsigned char fap_driver1_error_itlk_read(void)
{
    return fap.Driver1ErrorItlk;
}

//**********************************************
unsigned char fap_driver2_error_read(void)
{
    return fap.Driver2Error;
}

unsigned char fap_driver2_error_itlk_read(void)
{
    return fap.Driver2ErrorItlk;
}

//**********************************************
unsigned char fap_external_itlk_read(void)
{
    return fap.ExternalItlk;
}

unsigned char fap_external_itlk_sts_read(void)
{
    return fap.ExternalItlkSts;
}

//**********************************************
unsigned char fap_leakage_current_read(void)
{
    return fap.LeakageCurrent;

}

unsigned char fap_leakage_current_sts_read(void)
{
    return fap.LeakageCurrentSts;

}

//**********************************************
unsigned char fapRackRead(void)
{
    return fap.Rack;

}

unsigned char fap_rack_sts_read(void)
{
    return fap.RackSts;
}
