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
 * @file fac_cmd.c
 * @brief Brief description of module
 * 
 * Detailed description
 *
 * @author allef.silva
 * @date 20 de out de 2018
 *
 */

/////////////////////////////////////////////////////////////////////////////////////////////

#include <iib_modules/fac_cmd.h>
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

volatile fac_cmd_t fac_cmd;

/////////////////////////////////////////////////////////////////////////////////////////////

void clear_fac_cmd_interlocks()
{
    fac_cmd.VcapBankItlkSts         = 0;
    fac_cmd.VoutItlkSts             = 0;
    fac_cmd.AuxIdbVoltageItlkSts    = 0;
    fac_cmd.AuxCurrentItlkSts       = 0;
    fac_cmd.IdbCurrentItlkSts       = 0;
    fac_cmd.MainOverCurrentItlkSts  = 0;
    fac_cmd.EmergencyButtonItlkSts  = 0;
    fac_cmd.MainUnderVoltageItlkSts = 0;
    fac_cmd.MainOverVoltageItlkSts  = 0;
    fac_cmd.GroundLeakageItlkSts    = 0;
    fac_cmd.TempLItlkSts            = 0;
    fac_cmd.TempHeatSinkItlkSts     = 0;
    fac_cmd.BoardTemperatureItlkSts = 0;
    fac_cmd.RelativeHumidityItlkSts = 0;

    fac_cmd.InterlocksRegister.u32  = 0;

}

/////////////////////////////////////////////////////////////////////////////////////////////

uint8_t check_fac_cmd_interlocks()
{
    uint8_t test = 0;

    test |= fac_cmd.VcapBankItlkSts;
    test |= fac_cmd.VoutItlkSts;
    test |= fac_cmd.AuxIdbVoltageItlkSts;
    test |= fac_cmd.AuxCurrentItlkSts;
    test |= fac_cmd.IdbCurrentItlkSts;
    test |= fac_cmd.MainOverCurrentItlkSts;
    test |= fac_cmd.EmergencyButtonItlkSts;
    test |= fac_cmd.MainUnderVoltageItlkSts;
    test |= fac_cmd.MainOverVoltageItlkSts;
    test |= fac_cmd.GroundLeakageItlkSts;
    test |= fac_cmd.TempLItlkSts;
    test |= fac_cmd.TempHeatSinkItlkSts;
    test |= fac_cmd.BoardTemperatureItlkSts;
    test |= fac_cmd.RelativeHumidityItlkSts;

    return test;
}

/////////////////////////////////////////////////////////////////////////////////////////////

void clear_fac_cmd_alarms()
{
    fac_cmd.VcapBankAlarmSts            = 0;
    fac_cmd.VoutAlarmSts                = 0;
    fac_cmd.AuxIdbVoltageAlarmSts       = 0;
    fac_cmd.AuxCurrentAlarmSts          = 0;
    fac_cmd.IdbCurrentAlarmSts          = 0;
    fac_cmd.GroundLeakageAlarmSts       = 0;
    fac_cmd.TempLAlarmSts               = 0;
    fac_cmd.TempHeatSinkAlarmSts        = 0;
    fac_cmd.BoardTemperatureAlarmSts    = 0;
    fac_cmd.RelativeHumidityAlarmSts    = 0;

    fac_cmd.AlarmsRegister.u32          = 0;

}

/////////////////////////////////////////////////////////////////////////////////////////////

uint8_t check_fac_cmd_alarms()
{
    uint8_t test = 0;

    test |= fac_cmd.VcapBankAlarmSts;
    test |= fac_cmd.VoutAlarmSts;
    test |= fac_cmd.AuxIdbVoltageAlarmSts;
    test |= fac_cmd.AuxCurrentAlarmSts;
    test |= fac_cmd.IdbCurrentAlarmSts;
    test |= fac_cmd.GroundLeakageAlarmSts;
    test |= fac_cmd.TempLAlarmSts;
    test |= fac_cmd.TempHeatSinkAlarmSts;
    test |= fac_cmd.BoardTemperatureAlarmSts;
    test |= fac_cmd.RelativeHumidityAlarmSts;

    return test;
}

/////////////////////////////////////////////////////////////////////////////////////////////

void check_fac_cmd_indication_leds()
{
	//AC-Mains Contactor Status
	if(!fac_cmd.PreChargeContactor) Led1Toggle();
	else if(!fac_cmd.AuxiliaryOptocoupler) Led1TurnOn();
	else Led1TurnOff();

/////////////////////////////////////////////////////////////////////////////////////////////

	//Interlocks main over current and emergency button
    if(fac_cmd.MainOverCurrentItlkSts || fac_cmd.EmergencyButtonItlkSts) Led2TurnOff();
    else Led2TurnOn();

/////////////////////////////////////////////////////////////////////////////////////////////

    //Interlocks main under voltage and main over voltage
    if(fac_cmd.MainUnderVoltageItlkSts || fac_cmd.MainOverVoltageItlkSts) Led3TurnOff();
    else Led3TurnOn();

/////////////////////////////////////////////////////////////////////////////////////////////

    //Input over voltage
    if(fac_cmd.VcapBankItlkSts) Led4TurnOff();
    else if(fac_cmd.VcapBankAlarmSts) Led4Toggle();
    else Led4TurnOn();

/////////////////////////////////////////////////////////////////////////////////////////////

    //Output over voltage
    if(fac_cmd.VoutItlkSts) Led5TurnOff();
    else if(fac_cmd.VoutAlarmSts) Led5Toggle();
    else Led5TurnOn();

////////////////////////////////////////////////////////////////////////////////////////////

    //Interlocks Aux and Idb voltage and current
    if(fac_cmd.AuxIdbVoltageItlkSts || fac_cmd.AuxCurrentItlkSts || fac_cmd.IdbCurrentItlkSts) Led6TurnOff();
    else if(fac_cmd.AuxIdbVoltageAlarmSts || fac_cmd.AuxCurrentAlarmSts || fac_cmd.IdbCurrentAlarmSts) Led6Toggle();
    else Led6TurnOn();

/////////////////////////////////////////////////////////////////////////////////////////////

    //Fuga para o Terra
    if(fac_cmd.GroundLeakageItlkSts) Led7TurnOff();
    else if(fac_cmd.GroundLeakageAlarmSts) Led7Toggle();
    else Led7TurnOn();

/////////////////////////////////////////////////////////////////////////////////////////////

    // Heatsink Over temperature
    if(fac_cmd.TempHeatSinkItlkSts) Led8TurnOff();
    else if(fac_cmd.TempHeatSinkAlarmSts) Led8Toggle();
    else Led8TurnOn();

/////////////////////////////////////////////////////////////////////////////////////////////

    // Inductor Over temperature
    if(fac_cmd.TempLItlkSts) Led9TurnOff();
    else if(fac_cmd.TempLAlarmSts) Led9Toggle();
    else Led9TurnOn();

/////////////////////////////////////////////////////////////////////////////////////////////

    //Interlock Temperatura PCB e Umidade Relativa
    if(fac_cmd.BoardTemperatureItlkSts || fac_cmd.RelativeHumidityItlkSts) Led10TurnOff();
    else if(fac_cmd.BoardTemperatureAlarmSts || fac_cmd.RelativeHumidityAlarmSts) Led10Toggle();
    else Led10TurnOn();
}

/////////////////////////////////////////////////////////////////////////////////////////////

void fac_cmd_application_readings()
{
    //PT100 CH1 Indutor
    fac_cmd.TempL.f = Pt100Ch1Read();
    fac_cmd.TempLAlarmSts = Pt100Ch1AlarmStatusRead();
    if(!fac_cmd.TempLItlkSts)fac_cmd.TempLItlkSts = Pt100Ch1TripStatusRead();

/////////////////////////////////////////////////////////////////////////////////////////////

    //PT100 CH2 Dissipador
    fac_cmd.TempHeatSink.f = Pt100Ch2Read();
    fac_cmd.TempHeatSinkAlarmSts = Pt100Ch2AlarmStatusRead();
    if(!fac_cmd.TempHeatSinkItlkSts)fac_cmd.TempHeatSinkItlkSts = Pt100Ch2TripStatusRead();

/////////////////////////////////////////////////////////////////////////////////////////////

    //Temperatura PCB IIB
    fac_cmd.BoardTemperature.f = BoardTempRead();
    fac_cmd.BoardTemperatureAlarmSts = BoardTempAlarmStatusRead();

#if (ItlkBoardTempEnable == 1)

    if(!fac_cmd.BoardTemperatureItlkSts)fac_cmd.BoardTemperatureItlkSts = BoardTempTripStatusRead();

#endif

/////////////////////////////////////////////////////////////////////////////////////////////

    //Umidade Relativa
    fac_cmd.RelativeHumidity.f = RhRead();
    fac_cmd.RelativeHumidityAlarmSts = RhAlarmStatusRead();

#if (ItlkRhEnable == 1)

    if(!fac_cmd.RelativeHumidityItlkSts)fac_cmd.RelativeHumidityItlkSts = RhTripStatusRead();

#endif

/////////////////////////////////////////////////////////////////////////////////////////////

    //Tensao de Saida
    fac_cmd.Vout.f = LvCurrentCh1Read();
    fac_cmd.VoutAlarmSts = LvCurrentCh1AlarmStatusRead();
    if(!fac_cmd.VoutItlkSts)fac_cmd.VoutItlkSts = LvCurrentCh1TripStatusRead();

/////////////////////////////////////////////////////////////////////////////////////////////

    //Tensao no Banco de Capacitores
    fac_cmd.VcapBank.f = LvCurrentCh2Read();
    fac_cmd.VcapBankAlarmSts = LvCurrentCh2AlarmStatusRead();
    if(!fac_cmd.VcapBankItlkSts)fac_cmd.VcapBankItlkSts = LvCurrentCh2TripStatusRead();

/////////////////////////////////////////////////////////////////////////////////////////////

    //Medida de Fuga para o Terra
    fac_cmd.GroundLeakage.f = VoltageCh1Read();
    fac_cmd.GroundLeakageAlarmSts = VoltageCh1AlarmStatusRead();
    if(!fac_cmd.GroundLeakageItlkSts)fac_cmd.GroundLeakageItlkSts = VoltageCh1TripStatusRead();

/////////////////////////////////////////////////////////////////////////////////////////////

    //Aux and Idb Voltage
    fac_cmd.AuxIdbVoltage.f = DriverVoltageRead();
    fac_cmd.AuxIdbVoltageAlarmSts = DriverVoltageAlarmStatusRead();
    if(!fac_cmd.AuxIdbVoltageItlkSts)fac_cmd.AuxIdbVoltageItlkSts = DriverVolatgeTripStatusRead();

/////////////////////////////////////////////////////////////////////////////////////////////

    //Aux Current
    fac_cmd.AuxCurrent.f = Driver1CurrentRead();
    fac_cmd.AuxCurrentAlarmSts = Driver1CurrentAlarmStatusRead();
    if(!fac_cmd.AuxCurrentItlkSts)fac_cmd.AuxCurrentItlkSts = Driver1CurrentTripStatusRead();

/////////////////////////////////////////////////////////////////////////////////////////////

    //Idb Current
    fac_cmd.IdbCurrent.f = Driver2CurrentRead();
    fac_cmd.IdbCurrentAlarmSts = Driver2CurrentAlarmStatusRead();
    if(!fac_cmd.IdbCurrentItlkSts)fac_cmd.IdbCurrentItlkSts = Driver2CurrentTripStatusRead();

/////////////////////////////////////////////////////////////////////////////////////////////

    //Interlock Main Over Current
    fac_cmd.MainOverCurrentItlk = Gpdi5Read();//Variavel usada para debug
    if(!fac_cmd.MainOverCurrentItlkSts)fac_cmd.MainOverCurrentItlkSts = Gpdi5Read();

/////////////////////////////////////////////////////////////////////////////////////////////

    //Interlock Emergency Button
    fac_cmd.EmergencyButtonItlk = Gpdi6Read();//Variavel usada para debug
    if(!fac_cmd.EmergencyButtonItlkSts)fac_cmd.EmergencyButtonItlkSts = Gpdi6Read();

/////////////////////////////////////////////////////////////////////////////////////////////

    //Interlock Main Under Voltage
    fac_cmd.MainUnderVoltageItlk = Gpdi7Read();//Variavel usada para debug
    if(!fac_cmd.MainUnderVoltageItlkSts)fac_cmd.MainUnderVoltageItlkSts = Gpdi7Read();

/////////////////////////////////////////////////////////////////////////////////////////////

    //Interlock Main Over Voltage
    fac_cmd.MainOverVoltageItlk = Gpdi8Read();//Variavel usada para debug
    if(!fac_cmd.MainOverVoltageItlkSts)fac_cmd.MainOverVoltageItlkSts = Gpdi8Read();

/////////////////////////////////////////////////////////////////////////////////////////////

    //Status do Contator de Pré-Carga
    fac_cmd.PreChargeContactor = Gpdi9Read();

/////////////////////////////////////////////////////////////////////////////////////////////

    //Status do Optoacoplador da Placa Auxiliar
    fac_cmd.AuxiliaryOptocoupler = Gpdi10Read();

/////////////////////////////////////////////////////////////////////////////////////////////

    //Se nao houver sinal na entrada digital dos 4 sinais, defina a acao como Interlock.
    if(fac_cmd.MainOverCurrentItlkSts || fac_cmd.EmergencyButtonItlkSts || fac_cmd.MainUnderVoltageItlkSts || fac_cmd.MainOverVoltageItlkSts) InterlockSet();

/////////////////////////////////////////////////////////////////////////////////////////////

    if(Interlock == 1)
    {
    	if (fac_cmd.VcapBankItlkSts)             fac_cmd.InterlocksRegister.u32 |= FAC_CMD_CAPBANK_OVERVOLTAGE_ITLK;
    	if (fac_cmd.VoutItlkSts)                 fac_cmd.InterlocksRegister.u32 |= FAC_CMD_OUTPUT_OVERVOLTAGE_ITLK;
    	if (fac_cmd.AuxIdbVoltageItlkSts)        fac_cmd.InterlocksRegister.u32 |= FAC_CMD_AUX_AND_IDB_SUPPLY_OVERVOLTAGE_ITLK;
    	if (fac_cmd.AuxCurrentItlkSts)           fac_cmd.InterlocksRegister.u32 |= FAC_CMD_AUX_SUPPLY_OVERCURRENT_ITLK;
    	if (fac_cmd.IdbCurrentItlkSts)           fac_cmd.InterlocksRegister.u32 |= FAC_CMD_IDB_SUPPLY_OVERCURRENT_ITLK;
    	if (fac_cmd.TempLItlkSts)                fac_cmd.InterlocksRegister.u32 |= FAC_CMD_INDUC_OVERTEMP_ITLK;
    	if (fac_cmd.TempHeatSinkItlkSts)         fac_cmd.InterlocksRegister.u32 |= FAC_CMD_HS_OVERTEMP_ITLK;
    	if (fac_cmd.MainOverCurrentItlkSts)      fac_cmd.InterlocksRegister.u32 |= FAC_CMD_MAIN_OVER_CURRENT_ITLK;
    	if (fac_cmd.EmergencyButtonItlkSts)      fac_cmd.InterlocksRegister.u32 |= FAC_CMD_EMERGENCY_BUTTON_ITLK;
    	if (fac_cmd.MainOverVoltageItlkSts)      fac_cmd.InterlocksRegister.u32 |= FAC_CMD_MAIN_OVER_VOLTAGE_ITLK;
    	if (fac_cmd.MainUnderVoltageItlkSts)     fac_cmd.InterlocksRegister.u32 |= FAC_CMD_MAIN_UNDER_VOLTAGE_ITLK;
    	if (fac_cmd.GroundLeakageItlkSts)        fac_cmd.InterlocksRegister.u32 |= FAC_CMD_GROUND_LKG_ITLK;
    	if (fac_cmd.BoardTemperatureItlkSts)     fac_cmd.InterlocksRegister.u32 |= FAC_CMD_BOARD_IIB_OVERTEMP_ITLK;
    	if (fac_cmd.RelativeHumidityItlkSts)     fac_cmd.InterlocksRegister.u32 |= FAC_CMD_BOARD_IIB_OVERHUMIDITY_ITLK;
    }

    else
    {
    	fac_cmd.InterlocksRegister.u32 = 0x00000000;
    }

/////////////////////////////////////////////////////////////////////////////////////////////

    if(Alarm == 1)
    {
    	if (fac_cmd.VcapBankAlarmSts)            fac_cmd.AlarmsRegister.u32 |= FAC_CMD_CAPBANK_OVERVOLTAGE_ALM;
    	if (fac_cmd.VoutItlkSts)                 fac_cmd.AlarmsRegister.u32 |= FAC_CMD_OUTPUT_OVERVOLTAGE_ALM;
    	if (fac_cmd.AuxIdbVoltageAlarmSts)       fac_cmd.AlarmsRegister.u32 |= FAC_CMD_AUX_AND_IDB_SUPPLY_OVERVOLTAGE_ALM;
    	if (fac_cmd.AuxCurrentAlarmSts)          fac_cmd.AlarmsRegister.u32 |= FAC_CMD_AUX_SUPPLY_OVERCURRENT_ALM;
    	if (fac_cmd.IdbCurrentAlarmSts)          fac_cmd.AlarmsRegister.u32 |= FAC_CMD_IDB_SUPPLY_OVERCURRENT_ALM;
    	if (fac_cmd.TempLAlarmSts)               fac_cmd.AlarmsRegister.u32 |= FAC_CMD_INDUC_OVERTEMP_ALM;
    	if (fac_cmd.TempHeatSinkAlarmSts)        fac_cmd.AlarmsRegister.u32 |= FAC_CMD_HS_OVERTEMP_ALM;
    	if (fac_cmd.GroundLeakageAlarmSts)       fac_cmd.AlarmsRegister.u32 |= FAC_CMD_GROUND_LKG_ALM;
    	if (fac_cmd.BoardTemperatureAlarmSts)    fac_cmd.AlarmsRegister.u32 |= FAC_CMD_BOARD_IIB_OVERTEMP_ALM;
    	if (fac_cmd.RelativeHumidityAlarmSts)    fac_cmd.AlarmsRegister.u32 |= FAC_CMD_BOARD_IIB_OVERHUMIDITY_ALM;
    }

    else
    {
    	fac_cmd.AlarmsRegister.u32 = 0x00000000;
    }

/////////////////////////////////////////////////////////////////////////////////////////////

    g_controller_iib.iib_signals[0].f       = fac_cmd.VcapBank.f;
    g_controller_iib.iib_signals[1].f       = fac_cmd.Vout.f;
    g_controller_iib.iib_signals[2].f       = fac_cmd.AuxIdbVoltage.f;
    g_controller_iib.iib_signals[3].f       = fac_cmd.AuxCurrent.f;
    g_controller_iib.iib_signals[4].f       = fac_cmd.IdbCurrent.f;
    g_controller_iib.iib_signals[5].f       = fac_cmd.GroundLeakage.f;
    g_controller_iib.iib_signals[6].f       = fac_cmd.TempL.f;
    g_controller_iib.iib_signals[7].f       = fac_cmd.TempHeatSink.f;
    g_controller_iib.iib_signals[8].f       = fac_cmd.BoardTemperature.f;
    g_controller_iib.iib_signals[9].f       = fac_cmd.RelativeHumidity.f;
    g_controller_iib.iib_signals[10].u32	= fac_cmd.InterlocksRegister.u32;
	g_controller_iib.iib_signals[11].u32    = fac_cmd.AlarmsRegister.u32;
}

/////////////////////////////////////////////////////////////////////////////////////////////

void config_module_fac_cmd(void)
{

#ifdef FAC_CMD

    /* Isolated Voltage */
    LvCurrentCh1Init(LV_Primary_Voltage_Vout, LV_Secondary_Current_Vin, LV_Burden_Resistor, Delay_Vout); /* Output Voltage */
    LvCurrentCh2Init(LV_Primary_Voltage_Cap_Bank, LV_Secondary_Current_Vin, LV_Burden_Resistor, Delay_Voltage_Cap_Bank); /* Voltage Capacitor Bank */

    /* Protection Limits */
    LvCurrentCh1AlarmLevelSet(FAC_CMD_OUTPUT_OVERVOLTAGE_ALM_LIM);
    LvCurrentCh1TripLevelSet(FAC_CMD_OUTPUT_OVERVOLTAGE_ITLK_LIM);
    LvCurrentCh2AlarmLevelSet(FAC_CMD_CAPBANK_OVERVOLTAGE_ALM_LIM);
    LvCurrentCh2TripLevelSet(FAC_CMD_CAPBANK_OVERVOLTAGE_ITLK_LIM);

/////////////////////////////////////////////////////////////////////////////////////////////

    //Leitura de tensão
    VoltageCh1Init(Current_GND_Leakage, Delay_GND_Leakage); //Ground Leakage

    VoltageCh1AlarmLevelSet(FAC_CMD_GROUND_LEAKAGE_ALM_LIM); //Fuga para o terra alarme
    VoltageCh1TripLevelSet(FAC_CMD_GROUND_LEAKAGE_ITLK_LIM); //Fuga para o terra interlock

/////////////////////////////////////////////////////////////////////////////////////////////

    //PT100 configuration
    //Delay 4 seconds
    Pt100Ch1Delay(Delay_PT100CH1);
    Pt100Ch2Delay(Delay_PT100CH2);

    /* Pt-100 Configuration Limits */
    Pt100Ch1AlarmLevelSet(FAC_CMD_INDUC_OVERTEMP_ALM_LIM);
    Pt100Ch1TripLevelSet(FAC_CMD_INDUC_OVERTEMP_ITLK_LIM);
    Pt100Ch2AlarmLevelSet(FAC_CMD_HS_OVERTEMP_ALM_LIM);
    Pt100Ch2TripLevelSet(FAC_CMD_HS_OVERTEMP_ITLK_LIM);

/////////////////////////////////////////////////////////////////////////////////////////////

    //Temperature Board configuration
    BoardTempDelay(Delay_BoardTemp); //Inserir valor de delay

    //Temp board configuration limits
    BoardTempAlarmLevelSet(FAC_CMD_BOARD_OVERTEMP_ALM_LIM);
    BoardTempTripLevelSet(FAC_CMD_BOARD_OVERTEMP_ITLK_LIM);

/////////////////////////////////////////////////////////////////////////////////////////////

    //Humidity Board configuration
    RhDelay(Delay_BoardRh); //Inserir valor de delay

    //Rh configuration limits
    RhAlarmLevelSet(FAC_CMD_RH_OVERHUMIDITY_ALM_LIM);
    RhTripLevelSet(FAC_CMD_RH_OVERHUMIDITY_ITLK_LIM);

/////////////////////////////////////////////////////////////////////////////////////////////

    //Configuration Aux and Idb voltage
    DriverVoltageInit();

    DriverVoltageDelay(Delay_DriverVoltage); //Inserir valor de delay

    //Limite de alarme e interlock da tensao
    DriverVoltageAlarmLevelSet(FAC_CMD_AUX_AND_IDB_SUPPLY_OVERVOLTAGE_ALM_LIM);
    DriverVoltageTripLevelSet(FAC_CMD_AUX_AND_IDB_SUPPLY_OVERVOLTAGE_ITLK_LIM);

/////////////////////////////////////////////////////////////////////////////////////////////

    //Configuration Aux and Idb current
    DriverCurrentInit();

    DriverCurrentDelay(Delay_DriverCurrent); //Inserir valor de delay

    //Limite de alarme e interlock Aux
    Driver1CurrentAlarmLevelSet(FAC_CMD_AUX_SUPPLY_OVERCURRENT_ALM_LIM);
    Driver1CurrentTripLevelSet(FAC_CMD_AUX_SUPPLY_OVERCURRENT_ITLK_LIM);

    //Limite de alarme e interlock Idb
    Driver2CurrentAlarmLevelSet(FAC_CMD_IDB_SUPPLY_OVERCURRENT_ALM_LIM);
    Driver2CurrentTripLevelSet(FAC_CMD_IDB_SUPPLY_OVERCURRENT_ITLK_LIM);

#endif

/////////////////////////////////////////////////////////////////////////////////////////////

    //Init Variables
    fac_cmd.VcapBank.f               = 0.0;
    fac_cmd.VcapBankAlarmSts         = 0;
    fac_cmd.VcapBankItlkSts          = 0;
    fac_cmd.Vout.f                   = 0.0;
    fac_cmd.VoutAlarmSts             = 0;
    fac_cmd.VoutItlkSts              = 0;
    fac_cmd.AuxIdbVoltage.f          = 0.0;
    fac_cmd.AuxIdbVoltageAlarmSts    = 0;
    fac_cmd.AuxIdbVoltageItlkSts     = 0;
    fac_cmd.AuxCurrent.f             = 0.0;
    fac_cmd.AuxCurrentAlarmSts       = 0;
    fac_cmd.AuxCurrentItlkSts        = 0;
    fac_cmd.IdbCurrent.f             = 0.0;
    fac_cmd.IdbCurrentAlarmSts       = 0;
    fac_cmd.IdbCurrentItlkSts        = 0;
    fac_cmd.MainOverCurrentItlk      = 0;
    fac_cmd.MainOverCurrentItlkSts   = 0;
    fac_cmd.EmergencyButtonItlk      = 0;
    fac_cmd.EmergencyButtonItlkSts   = 0;
    fac_cmd.MainUnderVoltageItlk     = 0;
    fac_cmd.MainUnderVoltageItlkSts  = 0;
    fac_cmd.MainOverVoltageItlk      = 0;
    fac_cmd.MainOverVoltageItlkSts   = 0;
    fac_cmd.GroundLeakage.f          = 0.0;
    fac_cmd.GroundLeakageAlarmSts    = 0;
    fac_cmd.GroundLeakageItlkSts     = 0;
    fac_cmd.TempL.f                  = 0.0;
    fac_cmd.TempLAlarmSts            = 0;
    fac_cmd.TempLItlkSts             = 0;
    fac_cmd.TempHeatSink.f           = 0.0;
    fac_cmd.TempHeatSinkAlarmSts     = 0;
    fac_cmd.TempHeatSinkItlkSts      = 0;
    fac_cmd.BoardTemperature.f       = 0.0;
    fac_cmd.BoardTemperatureAlarmSts = 0;
    fac_cmd.BoardTemperatureItlkSts  = 0;
    fac_cmd.RelativeHumidity.f       = 0.0;
    fac_cmd.RelativeHumidityAlarmSts = 0;
    fac_cmd.RelativeHumidityItlkSts  = 0;
    fac_cmd.PreChargeContactor       = 0;
    fac_cmd.AuxiliaryOptocoupler     = 0;
    fac_cmd.InterlocksRegister.u32   = 0;
    fac_cmd.AlarmsRegister.u32       = 0;

}

/////////////////////////////////////////////////////////////////////////////////////////////






