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
 * @file fap_300A.c
 * @brief Brief description of module
 * 
 * Detailed description
 *
 * @author allef.silva
 * @date 20 de out de 2018
 *
 */

#include <iib_modules/fap_300A.h>
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
#define FAP_INPUT_OVERVOLTAGE_ALM                    40.0
#define FAP_INPUT_OVERVOLTAGE_ITLK                   45.0
#define FAP_OUTPUT_OVERVOLTAGE_ALM                   9.0
#define FAP_OUTPUT_OVERVOLTAGE_ITLK                  10.0
#define FAP_OUTPUT_OVERCURRENT_1_ALM                 151.0
#define FAP_OUTPUT_OVERCURRENT_1_ITLK                152.0
#define FAP_OUTPUT_OVERCURRENT_2_ALM                 151.5
#define FAP_OUTPUT_OVERCURRENT_2_ITLK                152.0
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
#define FAP_HS_OVERTEMP_ALM                          80
#define FAP_HS_OVERTEMP_ITLK                         60
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
} fap_300A_t;

/**
 * TODO: Put here your constants and variables. Always use static for 
 * private members.
 */

/**
 * TODO: Put here your function prototypes for private functions. Use
 * static in declaration.
 */
fap_300A_t fap_300A;
uint32_t fap_300A_interlocks_indication   = 0;
uint32_t fap_300A_alarms_indication       = 0;

static uint32_t itlk_id;
static uint32_t alarm_id;

static void get_itlks_id();
static void get_alarms_id();

/**
 * TODO: Put here the implementation for your public functions.
 */

void init_fap_300A()
{
    //Set current range FAP 300 A
    CurrentCh1Init(150.0, 0.150, 50.0, 3); // Corrente bra�o1: Sensor Hall
    CurrentCh2Init(150.0, 0.150, 50.0, 3); // Corrente bra�o2: LEM LA 130-


    //Set protection limits FAP 300 A
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
    fap_300A.Vin.f                 = 0.0;
    fap_300A.VinAlarmSts           = 0;
    fap_300A.VinItlkSts            = 0;
    fap_300A.Vout.f                = 0.0;
    fap_300A.VoutAlarmSts          = 0;
    fap_300A.VoutItlkSts           = 0;
    fap_300A.IoutA1.f              = 0.0;
    fap_300A.IoutA1AlarmSts        = 0;
    fap_300A.IoutA1ItlkSts         = 0;
    fap_300A.IoutA2.f              = 0.0;
    fap_300A.IoutA2AlarmSts        = 0;
    fap_300A.IoutA2ItlkSts         = 0;
    fap_300A.TempIGBT1.f           = 0.0;
    fap_300A.TempIGBT1AlarmSts     = 0;
    fap_300A.TempIGBT1ItlkSts      = 0;
    fap_300A.TempIGBT1HwrItlk      = 0;
    fap_300A.TempIGBT1HwrItlkSts   = 0;
    fap_300A.TempIGBT2.f           = 0.0;
    fap_300A.TempIGBT2AlarmSts     = 0;
    fap_300A.TempIGBT2ItlkSts      = 0;
    fap_300A.TempIGBT2HwrItlk      = 0;
    fap_300A.TempIGBT2HwrItlkSts   = 0;
    fap_300A.DriveVoltage.f        = 0.0;
    fap_300A.Drive1Current.f       = 0.0;
    fap_300A.Drive2Current.f       = 0.0;
    fap_300A.Driver1Error          = 0;
    fap_300A.Driver1ErrorItlk      = 0;
    fap_300A.Driver2Error          = 0;
    fap_300A.Driver2ErrorItlk      = 0;
    fap_300A.TempL.f               = 0;
    fap_300A.TempLAlarmSts         = 0;
    fap_300A.TempLItlkSts          = 0;
    fap_300A.TempHeatSink.f        = 0;
    fap_300A.TempHeatSinkAlarmSts  = 0;
    fap_300A.TempHeatSinkItlkSts   = 0;
    fap_300A.Relay                 = 0;
    fap_300A.ExternalItlk          = 0;
    fap_300A.ExternalItlkSts       = 0;
    fap_300A.LeakageCurrent        = 0;
    fap_300A.LeakageCurrentSts     = 0;
    fap_300A.Rack                  = 0;
    fap_300A.RackSts               = 0;
}

void clear_fap_300A_interlocks()
{
    fap_300A.VinItlkSts            = 0;
    fap_300A.VoutItlkSts           = 0;
    fap_300A.IoutA1ItlkSts         = 0;
    fap_300A.IoutA2ItlkSts         = 0;
    fap_300A.TempIGBT1ItlkSts      = 0;
    fap_300A.TempIGBT1HwrItlkSts   = 0;
    fap_300A.TempIGBT2ItlkSts      = 0;
    fap_300A.TempIGBT2HwrItlkSts   = 0;
    fap_300A.Driver1ErrorItlk      = 0;
    fap_300A.Driver2ErrorItlk      = 0;
    fap_300A.TempLItlkSts          = 0;
    fap_300A.TempHeatSinkItlkSts   = 0;
    fap_300A.ExternalItlkSts       = 0;
    fap_300A.LeakageCurrentSts     = 0;
    fap_300A.RackSts               = 0;

    itlk_id = 0;
}

uint8_t check_fap_300A_interlocks()
{
    uint8_t test = 0;

    test |= fap_300A.VinItlkSts;
    test |= fap_300A.VoutItlkSts;
    test |= fap_300A.IoutA1ItlkSts;
    test |= fap_300A.IoutA2ItlkSts;
    test |= fap_300A.TempIGBT1ItlkSts;
    test |= fap_300A.TempIGBT1HwrItlkSts;
    test |= fap_300A.TempIGBT2ItlkSts;
    test |= fap_300A.TempIGBT2HwrItlkSts;
    test |= fap_300A.Driver1ErrorItlk;
    test |= fap_300A.Driver2ErrorItlk;
    test |= fap_300A.TempLItlkSts;
    test |= fap_300A.TempHeatSinkItlkSts;
    test |= fap_300A.ExternalItlkSts;
    test |= fap_300A.LeakageCurrentSts;
    test |= fap_300A.RackSts;

    return test;
}

void clear_fap_300A_alarms()
{
    fap_300A.VinItlkSts             = 0;
    fap_300A.VoutItlkSts            = 0;
    fap_300A.IoutA1ItlkSts          = 0;
    fap_300A.IoutA2ItlkSts          = 0;
    fap_300A.TempIGBT1ItlkSts       = 0;
    fap_300A.TempIGBT1HwrItlkSts    = 0;
    fap_300A.TempIGBT2ItlkSts       = 0;
    fap_300A.TempIGBT2HwrItlkSts    = 0;
    fap_300A.Driver1ErrorItlk       = 0;
    fap_300A.Driver2ErrorItlk       = 0;
    fap_300A.TempLItlkSts           = 0;
    fap_300A.TempHeatSinkItlkSts    = 0;
    fap_300A.ExternalItlkSts        = 0;
    fap_300A.LeakageCurrentSts      = 0;
    fap_300A.RackSts                = 0;

    alarm_id = 0;
}

uint8_t check_fap_300A_alarms()
{
    uint8_t test = 0;

    test |= fap_300A.VinAlarmSts;
    test |= fap_300A.VoutAlarmSts;
    test |= fap_300A.IoutA1AlarmSts;
    test |= fap_300A.IoutA2AlarmSts;
    test |= fap_300A.TempIGBT1AlarmSts;
    test |= fap_300A.TempIGBT2AlarmSts;
    test |= fap_300A.TempLAlarmSts;
    test |= fap_300A.TempHeatSinkAlarmSts;

    return test;
}

void check_fap_300A_indication_leds()
{
    // Output over voltage
    if(fap_300A.VoutItlkSts) Led2TurnOn();
    else if(fap_300A.VoutAlarmSts) Led2Toggle();
    else Led2TurnOff();

    // Input over voltage
    if(fap_300A.VinItlkSts) Led3TurnOn();
    else if(fap_300A.VinAlarmSts) Led3Toggle();
    else Led3TurnOff();

    // Output over current
    if (fap_300A.IoutA1ItlkSts || fap_300A.IoutA2ItlkSts) Led4TurnOn();
    else if(fap_300A.IoutA1AlarmSts || fap_300A.IoutA2AlarmSts) Led4Toggle();
    else Led4TurnOff();

    // Over temperature
    if(fap_300A.TempIGBT1ItlkSts || fap_300A.TempIGBT2ItlkSts ||  fap_300A.TempLItlkSts || fap_300A.TempHeatSinkItlkSts || fap_300A.TempIGBT1HwrItlkSts || fap_300A.TempIGBT2HwrItlkSts) Led5TurnOn();
    else if(fap_300A.TempIGBT1AlarmSts || fap_300A.TempIGBT2AlarmSts ||  fap_300A.TempLAlarmSts || fap_300A.TempHeatSinkAlarmSts) Led5Toggle();
    else Led5TurnOff();

    if(fap_300A.ExternalItlkSts) Led6TurnOn();
    else Led6TurnOff();

    if(fap_300A.LeakageCurrentSts) Led7TurnOn();
    else Led7TurnOff();

    if(fap_300A.RackSts) Led8TurnOn();
    else Led8TurnOff();

    if(fap_300A.Driver1ErrorItlk || fap_300A.Driver2ErrorItlk) Led9TurnOn();
    else if(!InterlockRead()) Led9TurnOff();

    if(InterlockRead()) Led10TurnOn();
    else Led10TurnOff();
}

void fap_300A_application_readings()
{
    fap_300A.TempHeatSink.f = (float) Pt100ReadCh1();//PT100 CH1
    fap_300A.TempHeatSinkAlarmSts = Pt100ReadCh1AlarmSts();
    if(!fap_300A.TempHeatSinkItlkSts)fap_300A.TempHeatSinkItlkSts        = Pt100ReadCh1TripSts();

    fap_300A.TempL.f = (float) Pt100ReadCh2();//PT100 CH2
    fap_300A.TempLAlarmSts = Pt100ReadCh2AlarmSts();
    if(!fap_300A.TempLItlkSts)fap_300A.TempLItlkSts                      = Pt100ReadCh2TripSts();

    fap_300A.TempIGBT1.f = 0.0;
    fap_300A.TempIGBT1AlarmSts = 0;
    fap_300A.TempIGBT1ItlkSts = 0;

    if(!fap_300A.TempIGBT1HwrItlkSts) fap_300A.TempIGBT1HwrItlkSts       = Driver1OverTempRead();

    fap_300A.TempIGBT2.f = 0.0;
    fap_300A.TempIGBT2AlarmSts = 0;
    fap_300A.TempIGBT2ItlkSts = 0;

    if(!fap_300A.TempIGBT2HwrItlkSts) fap_300A.TempIGBT2HwrItlkSts       = Driver2OverTempRead();

    fap_300A.IoutA1.f = CurrentCh1Read();//HALL CH1
    fap_300A.IoutA1AlarmSts = CurrentCh1AlarmStatusRead();
    if(!fap_300A.IoutA1ItlkSts)fap_300A.IoutA1ItlkSts                    = CurrentCh1TripStatusRead();

    fap_300A.IoutA2.f = CurrentCh2Read();//HALL CH2
    fap_300A.IoutA2AlarmSts = CurrentCh2AlarmStatusRead();
    if(!fap_300A.IoutA2ItlkSts)fap_300A.IoutA2ItlkSts                    = CurrentCh2TripStatusRead();

    fap_300A.Vin.f = LvCurrentCh1Read();
    fap_300A.VinAlarmSts = LvCurrentCh1AlarmStatusRead();
    if(!fap_300A.VinItlkSts)fap_300A.VinItlkSts                          = LvCurrentCh1TripStatusRead();

    fap_300A.Vout.f = LvCurrentCh2Read();
    fap_300A.VoutAlarmSts = LvCurrentCh2AlarmStatusRead();
    if(!fap_300A.VoutItlkSts)fap_300A.VoutItlkSts                        = LvCurrentCh2TripStatusRead();

    fap_300A.ExternalItlk = Gpdi1Read();
    if(!fap_300A.ExternalItlkSts) fap_300A.ExternalItlkSts               = Gpdi1Read();

    fap_300A.LeakageCurrent = Gpdi2Read();
    if(!fap_300A.LeakageCurrentSts) fap_300A.LeakageCurrentSts           = Gpdi2Read();

    fap_300A.Rack = Gpdi3Read();
    if(!fap_300A.RackSts) fap_300A.RackSts                               = Gpdi3Read();

    fap_300A.Relay = !Gpdi4Read();

    fap_300A.Driver1Error = Driver1TopErrRead();
    if(!fap_300A.Driver1ErrorItlk) fap_300A.Driver1ErrorItlk             = Driver1TopErrRead();

    fap_300A.Driver2Error = Driver2TopErrRead();
    if(!fap_300A.Driver2ErrorItlk) fap_300A.Driver2ErrorItlk             = Driver2TopErrRead();

    if(fap_300A.ExternalItlkSts || fap_300A.Driver2ErrorItlk || fap_300A.Driver2ErrorItlk) InterlockSet(); // If no signal over the port, then set Interlock action

    fap_300A_map_vars();
    get_itlks_id();
    get_alarms_id();
}

void fap_300A_map_vars()
{
    g_controller_iib.iib_signals[0].u32     = fap_300A_interlocks_indication;
    g_controller_iib.iib_signals[1].u32     = fap_300A_alarms_indication;
    g_controller_iib.iib_signals[2].f       = fap_300A.Vin.f;
    g_controller_iib.iib_signals[3].f       = fap_300A.Vout.f;
    g_controller_iib.iib_signals[4].f       = fap_300A.IoutA1.f;
    g_controller_iib.iib_signals[5].f       = fap_300A.IoutA2.f;
    g_controller_iib.iib_signals[6].f       = fap_300A.TempIGBT1.f;
    g_controller_iib.iib_signals[7].f       = fap_300A.TempIGBT2.f;
    g_controller_iib.iib_signals[8].f       = fap_300A.DriveVoltage.f;
    g_controller_iib.iib_signals[9].f       = fap_300A.Drive1Current.f;
    g_controller_iib.iib_signals[10].f      = fap_300A.Drive2Current.f;
    g_controller_iib.iib_signals[11].f      = fap_300A.TempL.f;
    g_controller_iib.iib_signals[12].f      = fap_300A.TempHeatSink.f;
}

void send_fap_300A_data()
{
    uint8_t i;
    for (i = 2; i < 13; i++) send_data_message(i);
}

static void get_itlks_id()
{
    if (fap_300A.VinItlkSts)          itlk_id |= INPUT_OVERVOLTAGE_ITLK;
    if (fap_300A.VoutItlkSts)         itlk_id |= OUTPUT_OVERVOLTAGE_ITLK;
    if (fap_300A.IoutA1ItlkSts)       itlk_id |= OUTPUT_OVERCURRENT_1_ITLK;
    if (fap_300A.IoutA2ItlkSts)       itlk_id |= OUTPUT_OVERCURRENT_2_ITLK;
    if (fap_300A.TempIGBT1ItlkSts)    itlk_id |= IGBT1_OVERTEMP_ITLK;
    if (fap_300A.TempIGBT2ItlkSts)    itlk_id |= IGBT2_OVERTEMP_ITLK;
    if (fap_300A.Driver1ErrorItlk)    itlk_id |= DRIVER1_ERROR_ITLK;
    if (fap_300A.Driver2ErrorItlk)    itlk_id |= DRIVER2_ERROR_ITLK;
    if (fap_300A.TempLItlkSts)        itlk_id |= INDUC_OVERTEMP_ITLK;
    if (fap_300A.TempHeatSinkItlkSts) itlk_id |= HS_OVERTEMP_ITLK;
    if (fap_300A.Relay)               itlk_id |= RELAY_ITLK;
    if (fap_300A.ExternalItlkSts)     itlk_id |= EXTERNAL_ITLK;
    if (fap_300A.LeakageCurrentSts)   itlk_id |= LEAKAGE_CURRENT_ITLK;
    if (fap_300A.RackSts)             itlk_id |= RACK_ITLK;
}

static void get_alarms_id()
{
    if (fap_300A.VinAlarmSts)          alarm_id |= INPUT_OVERVOLTAGE_ALM;
    if (fap_300A.VoutAlarmSts)         alarm_id |= OUTPUT_OVERVOLTAGE_ALM;
    if (fap_300A.IoutA1AlarmSts)       alarm_id |= OUTPUT_OVERCURRENT_1_ALM;
    if (fap_300A.IoutA2AlarmSts)       alarm_id |= OUTPUT_OVERCURRENT_2_ALM;
    if (fap_300A.TempIGBT1AlarmSts)    alarm_id |= IGBT1_OVERTEMP_ALM;
    if (fap_300A.TempIGBT2AlarmSts)    alarm_id |= IGBT2_OVERTEMP_ALM;
    if (fap_300A.TempLAlarmSts)        alarm_id |= INDUC_OVERTEMP_ALM;
    if (fap_300A.TempHeatSinkAlarmSts) alarm_id |= HS_OVERTEMP_ALM;
}

void send_output_fap_300A_itlk_msg()
{
    send_data_message(0);
}

float fap_300A_vout_read(void)
{
    return fap_300A.Vout.f;
}

unsigned char fap_300A_vout_alarm_sts_read(void)
{
    return fap_300A.VoutAlarmSts;
}

unsigned char fap_30A_module_vout_itlk_sts_read(void)
{
    return fap_300A.VoutItlkSts;
}

//**********************************************
float fap_300A_vin_read(void)
{
    return fap_300A.Vin.f;
}

unsigned char fap_300A_vin_alarm_sts_read(void)
{
    return fap_300A.VinAlarmSts;
}

unsigned char fap_300A_vin_itlk_sts_read(void)
{
    return fap_300A.VinItlkSts;
}

//**********************************************
float fap_300A_iout_a1_read(void)
{
    return fap_300A.IoutA1.f;
}

unsigned char fap_300A_iout_a1_alarm_sts_read(void)
{
    return fap_300A.IoutA1AlarmSts;
}

unsigned char fap_300A_iout_a1_itlk_sts_read(void)
{
    return fap_300A.IoutA1ItlkSts;
}

//**********************************************
float fap_300A_iout_a2_read(void)
{
    return fap_300A.IoutA2.f;
}

unsigned char fap_300A_iout_a2_alarm_sts_read(void)
{
    return fap_300A.IoutA2AlarmSts;
}

unsigned char fap_300A_iout_a2_itlk_sts_read(void)
{
    return fap_300A.IoutA2ItlkSts;
}

//**********************************************
unsigned char fap_300A_temp_IGBT1_read(void)
{
    return fap_300A.TempIGBT1.f;
}

unsigned char fap_300A_temp_IGBT1_alarm_sts_read(void)
{
    return fap_300A.TempIGBT1AlarmSts;
}

unsigned char fap_300A_temp_IGBT1_itlk_sts_read(void)
{
    return fap_300A.TempIGBT1ItlkSts;
}

unsigned char fap_300A_temp_IGBT1_hwr_itlk_read(void)
{
    return fap_300A.TempIGBT1HwrItlk;
}

unsigned char fap_300A_temp_IGBT1_hwr_itlk_sts_read(void)
{
    return fap_300A.TempIGBT1HwrItlkSts;
}

//**********************************************
unsigned char fap_300A_temp_IGBT2_read(void)
{
    return fap_300A.TempIGBT2.f;
}

unsigned char fap_300A_temp_IGBT2_alarm_sts_read(void)
{
    return fap_300A.TempIGBT2AlarmSts;
}

unsigned char fap_300A_temp_IGBT2_itlk_sts_read(void)
{
    return fap_300A.TempIGBT2ItlkSts;
}

unsigned char fap_300A_temp_IGBT2_hwr_itlk_read(void)
{
    return fap_300A.TempIGBT2HwrItlk;
}

unsigned char fap_300A_temp_IGBT2_hwr_itlk_sts_read(void)
{
    return fap_300A.TempIGBT2HwrItlkSts;
}

//**********************************************
unsigned char fap_300A_temp_heatsink_read(void)
{
    return fap_300A.TempHeatSink.f;
}

unsigned char fap_300A_temp_heatsink_alarm_sts_read(void)
{
    return fap_300A.TempHeatSinkAlarmSts;
}

unsigned char fap_300A_temp_heatsink_itlk_sts_read(void)
{
    return fap_300A.TempHeatSinkItlkSts;
}

//**********************************************
unsigned char fap_300A_tempL_read(void)
{
    return fap_300A.TempL.f;
}

unsigned char fap_300A_tempL_alarm_sts_read(void)
{
    return fap_300A.TempLAlarmSts;
}

unsigned char fap_300A_tempL_itlk_sts_read(void)
{
    return fap_300A.TempLItlkSts;
}

//**********************************************
unsigned char fap_300A_relay_read(void)
{
    return fap_300A.Relay;
}

//**********************************************
unsigned char fap_300A_driver1_error_read(void)
{
    return fap_300A.Driver1Error;
}

unsigned char fap_300A_driver1_error_itlk_read(void)
{
    return fap_300A.Driver1ErrorItlk;
}

//**********************************************
unsigned char fap_300A_driver2_error_read(void)
{
    return fap_300A.Driver2Error;
}

unsigned char fap_300A_driver2_error_itlk_read(void)
{
    return fap_300A.Driver2ErrorItlk;
}

//**********************************************
unsigned char fap_300A_external_itlk_read(void)
{
    return fap_300A.ExternalItlk;
}

unsigned char fap_300A_external_itlk_sts_read(void)
{
    return fap_300A.ExternalItlkSts;
}

//**********************************************
unsigned char fap_300A_leakage_current_read(void)
{
    return fap_300A.LeakageCurrent;

}

unsigned char fap_300A_leakage_current_sts_read(void)
{
    return fap_300A.LeakageCurrentSts;

}

//**********************************************
unsigned char fap_300ARackRead(void)
{
    return fap_300A.Rack;

}

unsigned char fap_300A_rack_sts_read(void)
{
    return fap_300A.RackSts;
}
