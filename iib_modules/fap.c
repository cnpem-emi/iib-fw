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

/////////////////////////////////////////////////////////////////////////////////////////////

#include <iib_modules/fap.h>
#include "iib_data.h"

#include "adc_internal.h"
#include "application.h"

#include "BoardTempHum.h"
#include "ntc_isolated_i2c.h"
#include "pt100.h"
#include "output.h"
#include "leds.h"
#include "can_bus.h"
#include "input.h"

#include <stdbool.h>
#include <stdint.h>

#include "peripheral_drivers/timer/timer.h"
#include "inc/hw_ssi.h"
#include "driverlib/sysctl.h"
#include "driverlib/ssi.h"

/////////////////////////////////////////////////////////////////////////////////////////////

fap_t fap;

/////////////////////////////////////////////////////////////////////////////////////////////

static uint32_t fap_interlocks_indication;
static uint32_t fap_alarms_indication;

static uint32_t ResetInterlocksRegister = 0;
static uint32_t ResetAlarmsRegister = 0;

/////////////////////////////////////////////////////////////////////////////////////////////

static uint32_t itlk_id;
static uint32_t alarm_id;

static uint8_t flag1 = 0;
static uint32_t FiltroUP1 = 1024;

static uint8_t flag2 = 0;
static uint32_t FiltroUP2 = 1024;

/////////////////////////////////////////////////////////////////////////////////////////////

void clear_fap_interlocks()
{
    fap.RelayOpenItlkSts = 0;
    fap.RelayContactStickingItlkSts = 0;
    fap.ReleAuxItlkSts = 0;
    fap.ReleExtItlkSts = 0;

    flag1 = 0;
    FiltroUP1 = 1024;

    flag2 = 0;
    FiltroUP2 = 1024;

////////////////////////////////////////

    fap.VinItlkSts               = 0;
    fap.VoutItlkSts              = 0;
    fap.IoutA1ItlkSts            = 0;
    fap.IoutA2ItlkSts            = 0;
    fap.TempIGBT1ItlkSts         = 0;
    fap.TempIGBT2ItlkSts         = 0;
    fap.Driver1ErrorItlkSts      = 0;
    fap.Driver2ErrorItlkSts      = 0;
    fap.TempLItlkSts             = 0;
    fap.TempHeatSinkItlkSts      = 0;
    fap.ExternalItlkSts          = 0;
    fap.RackItlkSts              = 0;
    fap.GroundLeakageItlkSts     = 0;
    fap.DriveVoltageItlkSts      = 0;
    fap.Drive1CurrentItlkSts     = 0;
    fap.Drive2CurrentItlkSts     = 0;
    fap.BoardTemperatureItlkSts  = 0;
    fap.RelativeHumidityItlkSts  = 0;

    itlk_id = 0;

}

/////////////////////////////////////////////////////////////////////////////////////////////

uint8_t check_fap_interlocks()
{
    uint8_t test = 0;

    test |= fap.VinItlkSts;
    test |= fap.VoutItlkSts;
    test |= fap.IoutA1ItlkSts;
    test |= fap.IoutA2ItlkSts;
    test |= fap.TempIGBT1ItlkSts;
    test |= fap.TempIGBT2ItlkSts;
    test |= fap.Driver1ErrorItlkSts;
    test |= fap.Driver2ErrorItlkSts;
    test |= fap.TempLItlkSts;
    test |= fap.TempHeatSinkItlkSts;
    test |= fap.ExternalItlkSts;
    test |= fap.RackItlkSts;
    test |= fap.GroundLeakageItlkSts;
    test |= fap.DriveVoltageItlkSts;
    test |= fap.Drive1CurrentItlkSts;
    test |= fap.Drive2CurrentItlkSts;
    test |= fap.BoardTemperatureItlkSts;
    test |= fap.RelativeHumidityItlkSts;

    return test;
}

/////////////////////////////////////////////////////////////////////////////////////////////

void clear_fap_alarms()
{
    fap.VinAlarmSts               = 0;
    fap.VoutAlarmSts              = 0;
    fap.IoutA1AlarmSts            = 0;
    fap.IoutA2AlarmSts            = 0;
    fap.TempIGBT1AlarmSts         = 0;
    fap.TempIGBT2AlarmSts         = 0;
    fap.TempLAlarmSts             = 0;
    fap.TempHeatSinkAlarmSts      = 0;
    fap.GroundLeakageAlarmSts     = 0;
    fap.DriveVoltageAlarmSts      = 0;
    fap.Drive1CurrentAlarmSts     = 0;
    fap.Drive2CurrentAlarmSts     = 0;
    fap.BoardTemperatureAlarmSts  = 0;
    fap.RelativeHumidityAlarmSts  = 0;

    alarm_id = 0;

}

/////////////////////////////////////////////////////////////////////////////////////////////

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
    test |= fap.GroundLeakageAlarmSts;
    test |= fap.DriveVoltageAlarmSts;
    test |= fap.Drive1CurrentAlarmSts;
    test |= fap.Drive2CurrentAlarmSts;
    test |= fap.BoardTemperatureAlarmSts;
    test |= fap.RelativeHumidityAlarmSts;

    return test;
}

/////////////////////////////////////////////////////////////////////////////////////////////

void check_fap_indication_leds()
{
    //Output over voltage
    if(fap.VoutItlkSts) Led2TurnOff();
    else if(fap.VoutAlarmSts) Led2Toggle();
    else Led2TurnOn();

/////////////////////////////////////////////////////////////////////////////////////////////

    //Input over voltage
    if(fap.VinItlkSts) Led3TurnOff();
    else if(fap.VinAlarmSts) Led3Toggle();
    else Led3TurnOn();

/////////////////////////////////////////////////////////////////////////////////////////////

    //Output over current
    if(fap.IoutA1ItlkSts || fap.IoutA2ItlkSts) Led4TurnOff();
    else if(fap.IoutA1AlarmSts || fap.IoutA2AlarmSts) Led4Toggle();
    else Led4TurnOn();

/////////////////////////////////////////////////////////////////////////////////////////////

    //Over temperature
    if(fap.TempIGBT1ItlkSts || fap.TempIGBT2ItlkSts ||  fap.TempLItlkSts || fap.TempHeatSinkItlkSts) Led5TurnOff();
    else if(fap.TempIGBT1AlarmSts || fap.TempIGBT2AlarmSts ||  fap.TempLAlarmSts || fap.TempHeatSinkAlarmSts) Led5Toggle();
    else Led5TurnOn();

/////////////////////////////////////////////////////////////////////////////////////////////

    //Interlock Externo
    if(fap.ExternalItlkSts) Led6TurnOff();
    else Led6TurnOn();

/////////////////////////////////////////////////////////////////////////////////////////////

    //Fuga para o Terra
    if(fap.GroundLeakageItlkSts) Led7TurnOff();
    else if(fap.GroundLeakageAlarmSts) Led7Toggle();
    else Led7TurnOn();

/////////////////////////////////////////////////////////////////////////////////////////////

    //Interlock do Rack
    if(fap.RackItlkSts) Led8TurnOff();
    else Led8TurnOn();

/////////////////////////////////////////////////////////////////////////////////////////////

    //Interlocks dos Drivers
    if(fap.Driver1ErrorItlkSts || fap.Driver2ErrorItlkSts || fap.DriveVoltageItlkSts || fap.Drive1CurrentItlkSts || fap.Drive2CurrentItlkSts) Led9TurnOff();
    else if(fap.DriveVoltageAlarmSts || fap.Drive1CurrentAlarmSts || fap.Drive2CurrentAlarmSts) Led9Toggle();
    else Led9TurnOn();

/////////////////////////////////////////////////////////////////////////////////////////////

    //Interlock Temperatura PCB e Umidade Relativa
    if(fap.BoardTemperatureItlkSts || fap.RelativeHumidityItlkSts) Led10TurnOff();
    else if(fap.BoardTemperatureAlarmSts || fap.RelativeHumidityAlarmSts) Led10Toggle();
    else Led10TurnOn();
}

/////////////////////////////////////////////////////////////////////////////////////////////

void fap_application_readings()
{
    //PT100 CH1 Dissipador
    fap.TempHeatSink.f = Pt100Ch1Read();
    fap.TempHeatSinkAlarmSts = Pt100Ch1AlarmStatusRead();
    if(!fap.TempHeatSinkItlkSts)fap.TempHeatSinkItlkSts = Pt100Ch1TripStatusRead();

/////////////////////////////////////////////////////////////////////////////////////////////

    //PT100 CH2 Indutor
    fap.TempL.f = Pt100Ch2Read();
    fap.TempLAlarmSts = Pt100Ch2AlarmStatusRead();
    if(!fap.TempLItlkSts)fap.TempLItlkSts = Pt100Ch2TripStatusRead();

/////////////////////////////////////////////////////////////////////////////////////////////

    //Temperatura IGBT1
    fap.TempIGBT1.f = TempIgbt1Read();
    fap.TempIGBT1AlarmSts = TempIgbt1AlarmStatusRead();
    if(!fap.TempIGBT1ItlkSts)fap.TempIGBT1ItlkSts = TempIgbt1TripStatusRead();

/////////////////////////////////////////////////////////////////////////////////////////////

    //Temperatura IGBT2
    fap.TempIGBT2.f = TempIgbt2Read();
    fap.TempIGBT2AlarmSts = TempIgbt2AlarmStatusRead();
    if(!fap.TempIGBT2ItlkSts)fap.TempIGBT2ItlkSts = TempIgbt2TripStatusRead();

/////////////////////////////////////////////////////////////////////////////////////////////

    //Temperatura PCB IIB
    fap.BoardTemperature.f = BoardTempRead();
    fap.BoardTemperatureAlarmSts = BoardTempAlarmStatusRead();
    if(!fap.BoardTemperatureItlkSts)fap.BoardTemperatureItlkSts = BoardTempTripStatusRead();

/////////////////////////////////////////////////////////////////////////////////////////////

    //Umidade Relativa
    fap.RelativeHumidity.f = RhRead();
    fap.RelativeHumidityAlarmSts = RhAlarmStatusRead();
    if(!fap.RelativeHumidityItlkSts)fap.RelativeHumidityItlkSts = RhTripStatusRead();

/////////////////////////////////////////////////////////////////////////////////////////////

    //DriverVotage
    fap.DriveVoltage.f = DriverVoltageRead();
    fap.DriveVoltageAlarmSts = DriverVoltageAlarmStatusRead();
    if(!fap.DriveVoltageItlkSts)fap.DriveVoltageItlkSts = DriverVolatgeTripStatusRead();

/////////////////////////////////////////////////////////////////////////////////////////////

    //Drive1Current
    fap.Drive1Current.f = Driver1CurrentRead();
    fap.Drive1CurrentAlarmSts = Driver1CurrentAlarmStatusRead();
    if(!fap.Drive1CurrentItlkSts)fap.Drive1CurrentItlkSts = Driver1CurrentTripStatusRead();

/////////////////////////////////////////////////////////////////////////////////////////////

    //Drive2Current
    fap.Drive2Current.f = Driver2CurrentRead();
    fap.Drive2CurrentAlarmSts = Driver2CurrentAlarmStatusRead();
    if(!fap.Drive2CurrentItlkSts)fap.Drive2CurrentItlkSts = Driver2CurrentTripStatusRead();

/////////////////////////////////////////////////////////////////////////////////////////////

    //Corrente de Saida IGBT1
    fap.IoutA1.f = CurrentCh1Read();//HALL CH1
    fap.IoutA1AlarmSts = CurrentCh1AlarmStatusRead();
    if(!fap.IoutA1ItlkSts)fap.IoutA1ItlkSts = CurrentCh1TripStatusRead();

/////////////////////////////////////////////////////////////////////////////////////////////

    //Corrente de Saida IGBT2
    fap.IoutA2.f = CurrentCh2Read();//HALL CH2
    fap.IoutA2AlarmSts = CurrentCh2AlarmStatusRead();
    if(!fap.IoutA2ItlkSts)fap.IoutA2ItlkSts = CurrentCh2TripStatusRead();

/////////////////////////////////////////////////////////////////////////////////////////////

    //Tensao de Entrada
    fap.Vin.f = LvCurrentCh1Read();
    fap.VinAlarmSts = LvCurrentCh1AlarmStatusRead();
    if(!fap.VinItlkSts)fap.VinItlkSts = LvCurrentCh1TripStatusRead();

/////////////////////////////////////////////////////////////////////////////////////////////

    //Tensao de Saida
    fap.Vout.f = LvCurrentCh2Read();
    fap.VoutAlarmSts = LvCurrentCh2AlarmStatusRead();
    if(!fap.VoutItlkSts)fap.VoutItlkSts = LvCurrentCh2TripStatusRead();

/////////////////////////////////////////////////////////////////////////////////////////////

    //Medida de Fuga para o Terra
    fap.GroundLeakage.f = LvCurrentCh3Read();
    fap.GroundLeakageAlarmSts = LvCurrentCh3AlarmStatusRead();
    if(!fap.GroundLeakageItlkSts)fap.GroundLeakageItlkSts = LvCurrentCh3TripStatusRead();

/////////////////////////////////////////////////////////////////////////////////////////////

#ifdef GIGA

    //Interlock externo
    fap.ExternalItlk = Gpdi5Read();//Variavel usada para debug
    if(!fap.ExternalItlkSts)fap.ExternalItlkSts = Gpdi5Read();

#endif

/*******************************************************************************************/

#ifdef SIRIUS_SALA_FONTES

    //Interlock externo
    fap.ExternalItlk = Gpdi5Read();//Variavel usada para debug
    if(!fap.ExternalItlkSts)fap.ExternalItlkSts = Gpdi5Read();

#endif

/*******************************************************************************************/

#ifdef SIRIUS_LT

    //Interlock externo
    fap.ExternalItlk = Gpdi1Read();//Variavel usada para debug
    if(!fap.ExternalItlkSts)fap.ExternalItlkSts = Gpdi1Read();

#endif

/////////////////////////////////////////////////////////////////////////////////////////////

#ifdef GIGA

    //Interlock do Rack
    fap.Rack = Gpdi6Read();//Variavel usada para debug
    if(!fap.RackItlkSts)fap.RackItlkSts = Gpdi6Read();

#endif

/*******************************************************************************************/

#ifdef SIRIUS_SALA_FONTES

    //Interlock do Rack
    fap.Rack = Gpdi7Read();//Variavel usada para debug
    if(!fap.RackItlkSts)fap.RackItlkSts = Gpdi7Read();

#endif

/*******************************************************************************************/

#ifdef SIRIUS_LT

    //Interlock do Rack
    fap.Rack = Gpdi3Read();//Variavel usada para debug
    if(!fap.RackItlkSts)fap.RackItlkSts = Gpdi3Read();

#endif

/////////////////////////////////////////////////////////////////////////////////////////////

#ifdef GIGA

    //Status do Contato do Rele
    fap.Relay = Gpdi7Read();

#endif

/*******************************************************************************************/

#ifdef SIRIUS_SALA_FONTES

    //Status do Contato do Rele
    fap.Relay = Gpdi8Read();

#endif

/*******************************************************************************************/

#ifdef SIRIUS_LT

    //Status do Contato do Rele
    fap.Relay = Gpdi4Read();

#endif

/////////////////////////////////////////////////////////////////////////////////////////////

    //Erro do Driver 1
    fap.Driver1Error = Driver1TopErrorRead();//Variavel usada para debug
    if(!fap.Driver1ErrorItlkSts)fap.Driver1ErrorItlkSts = Driver1TopErrorRead();

/////////////////////////////////////////////////////////////////////////////////////////////

    //Erro do Driver 2
    fap.Driver2Error = Driver2TopErrorRead();//Variavel usada para debug
    if(!fap.Driver2ErrorItlkSts)fap.Driver2ErrorItlkSts = Driver2TopErrorRead();

/////////////////////////////////////////////////////////////////////////////////////////////

    fap.ReleAuxItlkSts = ReleAuxSts();

    fap.ReleExtItlkSts = ReleExtItlkSts();

    if(fap.ReleAuxItlkSts == 0 && fap.ReleExtItlkSts == 0 && flag1 == 0)
    {
        if(!FiltroUP1)
        {
            fap.RelayOpenItlkSts = 1;
            fap.RelayContactStickingItlkSts = 0;

            FiltroUP1 = 1024;
            flag1 = 1;
        }
        else FiltroUP1--;

    }
    else if(flag1 == 1)
    {
        flag1 = 0;
        FiltroUP1 = 1024;
    }

/////////////////////////////////////////////////////////////////////////////////////////////

    if(fap.ReleAuxItlkSts == 0 && fap.ReleExtItlkSts == 1 && flag2 == 0)
    {
        if(!FiltroUP2)
        {
            fap.RelayContactStickingItlkSts = 1;
            fap.RelayOpenItlkSts = 0;

            FiltroUP2 = 1024;
            flag2 = 1;
        }
        else FiltroUP2--;

    }
    else if(flag2 == 1)
    {
        flag2 = 0;
        FiltroUP2 = 1024;
    }

/////////////////////////////////////////////////////////////////////////////////////////////

    //Se nao houver sinal na entrada digital dos 4 sinais, defina a acao como Interlock.
    if(fap.ExternalItlkSts || fap.RackItlkSts || fap.Driver1ErrorItlkSts || fap.Driver2ErrorItlkSts) InterlockSet();

/////////////////////////////////////////////////////////////////////////////////////////////

    if (fap.VinItlkSts)                     itlk_id |= FAP_INPUT_OVERVOLTAGE_ITLK;
    if (fap.VoutItlkSts)                    itlk_id |= FAP_OUTPUT_OVERVOLTAGE_ITLK;
    if (fap.IoutA1ItlkSts)                  itlk_id |= FAP_OUTPUT_OVERCURRENT_1_ITLK;
    if (fap.IoutA2ItlkSts)                  itlk_id |= FAP_OUTPUT_OVERCURRENT_2_ITLK;
    if (fap.TempIGBT1ItlkSts)               itlk_id |= FAP_IGBT1_OVERTEMP_ITLK;
    if (fap.TempIGBT2ItlkSts)               itlk_id |= FAP_IGBT2_OVERTEMP_ITLK;
    if (fap.DriveVoltageItlkSts)            itlk_id |= FAP_DRIVER_OVERVOLTAGE_ITLK;
    if (fap.Drive1CurrentItlkSts)           itlk_id |= FAP_DRIVER1_OVERCURRENT_ITLK;
    if (fap.Drive2CurrentItlkSts)           itlk_id |= FAP_DRIVER2_OVERCURRENT_ITLK;
    if (fap.Driver1ErrorItlkSts)            itlk_id |= FAP_DRIVER1_ERROR_ITLK;
    if (fap.Driver2ErrorItlkSts)            itlk_id |= FAP_DRIVER2_ERROR_ITLK;
    if (fap.TempLItlkSts)                   itlk_id |= FAP_INDUC_OVERTEMP_ITLK;
    if (fap.TempHeatSinkItlkSts)            itlk_id |= FAP_HS_OVERTEMP_ITLK;
    if (fap.RelayOpenItlkSts)               itlk_id |= FAP_RELAY_ITLK;
    if (fap.RelayContactStickingItlkSts)    itlk_id |= FAP_RELAY_CONTACT_STICKING_ITLK;
    if (fap.ExternalItlkSts)                itlk_id |= FAP_EXTERNAL_ITLK;
    if (fap.RackItlkSts)                    itlk_id |= FAP_RACK_ITLK;
    if (fap.GroundLeakageItlkSts)           itlk_id |= FAP_GROUND_LKG_ITLK;
    if (fap.BoardTemperatureItlkSts)        itlk_id |= FAP_BOARD_IIB_OVERTEMP_ITLK;
    if (fap.RelativeHumidityItlkSts)        itlk_id |= FAP_BOARD_IIB_OVERHUMIDITY_ITLK;

/////////////////////////////////////////////////////////////////////////////////////////////

    if (fap.VinAlarmSts)                    alarm_id |= FAP_INPUT_OVERVOLTAGE_ALM;
    if (fap.VoutAlarmSts)                   alarm_id |= FAP_OUTPUT_OVERVOLTAGE_ALM;
    if (fap.IoutA1AlarmSts)                 alarm_id |= FAP_OUTPUT_OVERCURRENT_1_ALM;
    if (fap.IoutA2AlarmSts)                 alarm_id |= FAP_OUTPUT_OVERCURRENT_2_ALM;
    if (fap.TempIGBT1AlarmSts)              alarm_id |= FAP_IGBT1_OVERTEMP_ALM;
    if (fap.TempIGBT2AlarmSts)              alarm_id |= FAP_IGBT2_OVERTEMP_ALM;
    if (fap.TempLAlarmSts)                  alarm_id |= FAP_INDUC_OVERTEMP_ALM;
    if (fap.TempHeatSinkAlarmSts)           alarm_id |= FAP_HS_OVERTEMP_ALM;
    if (fap.GroundLeakageAlarmSts)          alarm_id |= FAP_GROUND_LKG_ALM;
    if (fap.DriveVoltageAlarmSts)           alarm_id |= FAP_DRIVER_OVERVOLTAGE_ALM;
    if (fap.Drive1CurrentAlarmSts)          alarm_id |= FAP_DRIVER1_OVERCURRENT_ALM;
    if (fap.Drive2CurrentAlarmSts)          alarm_id |= FAP_DRIVER2_OVERCURRENT_ALM;
    if (fap.BoardTemperatureAlarmSts)       alarm_id |= FAP_BOARD_IIB_OVERTEMP_ALM;
    if (fap.RelativeHumidityAlarmSts)       alarm_id |= FAP_BOARD_IIB_OVERHUMIDITY_ALM;

/////////////////////////////////////////////////////////////////////////////////////////////

    fap_interlocks_indication = itlk_id;
    fap_alarms_indication = alarm_id;

    g_controller_iib.iib_itlk[0].u32        = fap_interlocks_indication;
    g_controller_iib.iib_itlk[1].u32        = ResetInterlocksRegister;

    g_controller_iib.iib_alarm[0].u32       = fap_alarms_indication;
    g_controller_iib.iib_alarm[1].u32       = ResetAlarmsRegister;

    g_controller_iib.iib_signals[0].f       = fap.Vin.f;
    g_controller_iib.iib_signals[1].f       = fap.Vout.f;
    g_controller_iib.iib_signals[2].f       = fap.IoutA1.f;
    g_controller_iib.iib_signals[3].f       = fap.IoutA2.f;
    g_controller_iib.iib_signals[4].f       = fap.TempIGBT1.f;
    g_controller_iib.iib_signals[5].f       = fap.TempIGBT2.f;
    g_controller_iib.iib_signals[6].f       = fap.DriveVoltage.f;
    g_controller_iib.iib_signals[7].f       = fap.Drive1Current.f;
    g_controller_iib.iib_signals[8].f       = fap.Drive2Current.f;
    g_controller_iib.iib_signals[9].f       = fap.TempL.f;
    g_controller_iib.iib_signals[10].f      = fap.TempHeatSink.f;
    g_controller_iib.iib_signals[11].f      = fap.GroundLeakage.f;
    g_controller_iib.iib_signals[12].f      = fap.BoardTemperature.f;
    g_controller_iib.iib_signals[13].f      = fap.RelativeHumidity.f;

}

/////////////////////////////////////////////////////////////////////////////////////////////

void config_module_fap(void)
{

#ifdef FAP

    //Set current range FAP 130 A
    CurrentCh1Init(LA_Primary_Current, LA_Secondary_Current, LA_Burden_Resistor, LA_Delay); //Corrente bra�o1: Sensor Hall
    CurrentCh2Init(LA_Primary_Current, LA_Secondary_Current, LA_Burden_Resistor, LA_Delay); //Corrente bra�o2: LEM LA 130-P

    //Set protection limits FAP 130 A
    CurrentCh1AlarmLevelSet(FAP_OUTPUT_OVERCURRENT_1_ALM_LIM);  //Corrente bra�o1
    CurrentCh1TripLevelSet(FAP_OUTPUT_OVERCURRENT_1_ITLK_LIM);  //Corrente bra�o1
    CurrentCh2AlarmLevelSet(FAP_OUTPUT_OVERCURRENT_2_ALM_LIM);  //Corrente bra�o2
    CurrentCh2TripLevelSet(FAP_OUTPUT_OVERCURRENT_2_ITLK_LIM);  //Corrente bra�o2

/////////////////////////////////////////////////////////////////////////////////////////////

    //Leitura de tens�o isolada
    LvCurrentCh1Init(LV_Primary_Voltage_Vin, LV_Secondary_Current_Vin, LV_Burden_Resistor, Delay_Vin); // Vin
    LvCurrentCh2Init(LV_Primary_Voltage_Vout, LV_Secondary_Current_Vin, LV_Burden_Resistor, Delay_Vout); // Vout
    LvCurrentCh3Init(LV_Primary_Voltage_GND_Leakage, LV_Secondary_Current_Vin, LV_Burden_Resistor, Delay_GND_Leakage); // Ground Leakage

    LvCurrentCh1AlarmLevelSet(FAP_INPUT_OVERVOLTAGE_ALM_LIM);  //Tens�o de entrada Alarme
    LvCurrentCh1TripLevelSet(FAP_INPUT_OVERVOLTAGE_ITLK_LIM);  //Tens�o de entrada Interlock
    LvCurrentCh2AlarmLevelSet(FAP_OUTPUT_OVERVOLTAGE_ALM_LIM); //Tens�o de sa�da Alarme
    LvCurrentCh2TripLevelSet(FAP_OUTPUT_OVERVOLTAGE_ITLK_LIM); //Tens�o de sa�da Interlock
    LvCurrentCh3AlarmLevelSet(FAP_GROUND_LEAKAGE_ALM_LIM);     //Fuga para o terra alarme
    LvCurrentCh3TripLevelSet(FAP_GROUND_LEAKAGE_ITLK_LIM);     //Fuga para o terra interlock

/////////////////////////////////////////////////////////////////////////////////////////////

    //PT100 configuration
    //Debouncing Delay seconds
    Pt100Ch1Delay(Delay_PT100CH1);
    Pt100Ch2Delay(Delay_PT100CH2);

    //PT100 configuration limits
    Pt100Ch1AlarmLevelSet(FAP_HS_OVERTEMP_ALM_LIM);     //Temperatura Dissipador
    Pt100Ch1TripLevelSet(FAP_HS_OVERTEMP_ITLK_LIM);     //Temperatura Dissipador
    Pt100Ch2AlarmLevelSet(FAP_INDUC_OVERTEMP_ALM_LIM);  //Temperatura L
    Pt100Ch2TripLevelSet(FAP_INDUC_OVERTEMP_ITLK_LIM);  //Temperatura L

/////////////////////////////////////////////////////////////////////////////////////////////

    //Temperature igbt1 configuration
    TempIgbt1Delay(Delay_IGBT1); //Inserir valor de delay

    //Temp Igbt1 configuration limits
    TempIgbt1AlarmLevelSet(FAP_IGBT1_OVERTEMP_ALM_LIM);
    TempIgbt1TripLevelSet(FAP_IGBT1_OVERTEMP_ITLK_LIM);

/////////////////////////////////////////////////////////////////////////////////////////////

    //Temperature igbt2 configuration
    TempIgbt2Delay(Delay_IGBT2); //Inserir valor de delay

    //Temp Igbt2 configuration limits
    TempIgbt2AlarmLevelSet(FAP_IGBT2_OVERTEMP_ALM_LIM);
    TempIgbt2TripLevelSet(FAP_IGBT2_OVERTEMP_ITLK_LIM);

/////////////////////////////////////////////////////////////////////////////////////////////

    //Temperature Board configuration
    BoardTempDelay(Delay_BoardTemp); //Inserir valor de delay

    //Temp board configuration limits
    BoardTempAlarmLevelSet(FAP_BOARD_OVERTEMP_ALM_LIM);
    BoardTempTripLevelSet(FAP_BOARD_OVERTEMP_ITLK_LIM);

/////////////////////////////////////////////////////////////////////////////////////////////

    //Humidity Board configuration
    RhDelay(Delay_BoardRh); //Inserir valor de delay

    //Rh configuration limits
    RhAlarmLevelSet(FAP_RH_OVERHUMIDITY_ALM_LIM);
    RhTripLevelSet(FAP_RH_OVERHUMIDITY_ITLK_LIM);

/////////////////////////////////////////////////////////////////////////////////////////////

    //Driver Voltage configuration
    DriverVoltageInit();

    DriverVoltageDelay(Delay_DriverVoltage); //Inserir valor de delay

    //Limite de alarme e interlock da tensao dos drivers
    DriverVoltageAlarmLevelSet(FAP_DRIVER_OVERVOLTAGE_ALM_LIM);
    DriverVoltageTripLevelSet(FAP_DRIVER_OVERVOLTAGE_ITLK_LIM);

/////////////////////////////////////////////////////////////////////////////////////////////

    //Driver Current configuration
    DriverCurrentInit();

    DriverCurrentDelay(Delay_DriverCurrent); //Inserir valor de delay

    //Limite de alarme e interlock da corrente do driver 1
    Driver1CurrentAlarmLevelSet(FAP_DRIVER1_OVERCURRENT_ALM_LIM);
    Driver1CurrentTripLevelSet(FAP_DRIVER1_OVERCURRENT_ITLK_LIM);

    //Limite de alarme e interlock da corrente do driver 2
    Driver2CurrentAlarmLevelSet(FAP_DRIVER2_OVERCURRENT_ALM_LIM);
    Driver2CurrentTripLevelSet(FAP_DRIVER2_OVERCURRENT_ITLK_LIM);

#endif

/////////////////////////////////////////////////////////////////////////////////////////////

    //Init Variables
    fap.Vin.f                        = 0.0;
    fap.VinAlarmSts                  = 0;
    fap.VinItlkSts                   = 0;
    fap.Vout.f                       = 0.0;
    fap.VoutAlarmSts                 = 0;
    fap.VoutItlkSts                  = 0;
    fap.IoutA1.f                     = 0.0;
    fap.IoutA1AlarmSts               = 0;
    fap.IoutA1ItlkSts                = 0;
    fap.IoutA2.f                     = 0.0;
    fap.IoutA2AlarmSts               = 0;
    fap.IoutA2ItlkSts                = 0;
    fap.TempIGBT1.f                  = 0.0;
    fap.TempIGBT1AlarmSts            = 0;
    fap.TempIGBT1ItlkSts             = 0;
    fap.TempIGBT2.f                  = 0.0;
    fap.TempIGBT2AlarmSts            = 0;
    fap.TempIGBT2ItlkSts             = 0;
    fap.DriveVoltage.f               = 0.0;
    fap.DriveVoltageAlarmSts         = 0;
    fap.DriveVoltageItlkSts          = 0;
    fap.Drive1Current.f              = 0.0;
    fap.Drive1CurrentAlarmSts        = 0;
    fap.Drive1CurrentItlkSts         = 0;
    fap.Drive2Current.f              = 0.0;
    fap.Drive2CurrentAlarmSts        = 0;
    fap.Drive2CurrentItlkSts         = 0;
    fap.Driver1Error                 = 0;
    fap.Driver1ErrorItlkSts          = 0;
    fap.Driver2Error                 = 0;
    fap.Driver2ErrorItlkSts          = 0;
    fap.TempL.f                      = 0.0;
    fap.TempLAlarmSts                = 0;
    fap.TempLItlkSts                 = 0;
    fap.TempHeatSink.f               = 0.0;
    fap.TempHeatSinkAlarmSts         = 0;
    fap.TempHeatSinkItlkSts          = 0;
    fap.Relay                        = 0;
    fap.ExternalItlk                 = 0;
    fap.ExternalItlkSts              = 0;
    fap.Rack                         = 0;
    fap.RackItlkSts                  = 0;
    fap.GroundLeakage.f              = 0.0;
    fap.GroundLeakageAlarmSts        = 0;
    fap.GroundLeakageItlkSts         = 0;
    fap.BoardTemperature.f           = 0.0;
    fap.BoardTemperatureAlarmSts     = 0;
    fap.BoardTemperatureItlkSts      = 0;
    fap.RelativeHumidity.f           = 0.0;
    fap.RelativeHumidityAlarmSts     = 0;
    fap.RelativeHumidityItlkSts      = 0;
    fap.ReleAuxItlkSts               = 0;
    fap.ReleExtItlkSts               = 0;
    fap.RelayOpenItlkSts             = 0;
    fap.RelayContactStickingItlkSts  = 0;
}

/////////////////////////////////////////////////////////////////////////////////////////////

