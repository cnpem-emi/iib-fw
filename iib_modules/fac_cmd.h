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
 * @file command_drawer.h
 * @brief Brief description of module
 * 
 * Detailed description
 *
 * @author allef.silva
 * @date 20 de out de 2018
 *
 */

/////////////////////////////////////////////////////////////////////////////////////////////

#ifndef FAC_CMD_H_
#define FAC_CMD_H_

/////////////////////////////////////////////////////////////////////////////////////////////

#include <stdbool.h>
#include <stdint.h>
#include "application.h"

/////////////////////////////////////////////////////////////////////////////////////////////

typedef volatile struct
{
    union {
        volatile float       f;
        volatile uint8_t     u8[4];
    } VcapBank;

    volatile bool VcapBankAlarmSts;
    volatile bool VcapBankItlkSts;

    union {
        volatile float       f;
        volatile uint8_t     u8[4];
    } Vout;

    volatile bool VoutAlarmSts;
    volatile bool VoutItlkSts;

    union {
        volatile float       f;
        volatile uint8_t     u8[4];
    } AuxIdbVoltage;

    volatile bool AuxIdbVoltageAlarmSts;
    volatile bool AuxIdbVoltageItlkSts;

    union {
        volatile float       f;
        volatile uint8_t     u8[4];
    } AuxCurrent;

    volatile bool AuxCurrentAlarmSts;
    volatile bool AuxCurrentItlkSts;

    union {
        volatile float       f;
        volatile uint8_t     u8[4];
    } IdbCurrent;

    volatile bool IdbCurrentAlarmSts;
    volatile bool IdbCurrentItlkSts;

    union {
        volatile float       f;
        volatile uint8_t     u8[4];
    } TempL;

    volatile bool TempLAlarmSts;
    volatile bool TempLItlkSts;

    union {
        volatile float       f;
        volatile uint8_t     u8[4];
    } TempHeatSink;

    volatile bool TempHeatSinkAlarmSts;
    volatile bool TempHeatSinkItlkSts;

    volatile bool MainOverCurrentItlk;
    volatile bool MainOverCurrentItlkSts;

    volatile bool EmergencyButtonItlk;
    volatile bool EmergencyButtonItlkSts;

    volatile bool MainUnderVoltageItlk;
    volatile bool MainUnderVoltageItlkSts;

    volatile bool MainOverVoltageItlk;
    volatile bool MainOverVoltageItlkSts;

    union {
        volatile float       f;
        volatile uint8_t     u8[4];
    } GroundLeakage;

    volatile bool GroundLeakageAlarmSts;
    volatile bool GroundLeakageItlkSts;

    union {
        volatile float       f;
        volatile uint8_t     u8[4];
    } BoardTemperature;

    volatile bool BoardTemperatureAlarmSts;
    volatile bool BoardTemperatureItlkSts;

    union {
        volatile float       f;
        volatile uint8_t     u8[4];
    } RelativeHumidity;

    volatile bool RelativeHumidityAlarmSts;
    volatile bool RelativeHumidityItlkSts;

    union {
    	volatile uint8_t     u8[4];
    	volatile uint32_t    u32;
    } InterlocksRegister;

    union {
    	volatile uint8_t     u8[4];
    	volatile uint32_t    u32;
    } AlarmsRegister;

    volatile bool PreChargeContactor;
    volatile bool AuxiliaryOptocoupler;

} fac_cmd_t;

/////////////////////////////////////////////////////////////////////////////////////////////

#define FAC_CMD_CAPBANK_OVERVOLTAGE_ITLK                0x00000001
#define FAC_CMD_OUTPUT_OVERVOLTAGE_ITLK                 0x00000002
#define FAC_CMD_AUX_AND_IDB_SUPPLY_OVERVOLTAGE_ITLK     0x00000004
#define FAC_CMD_AUX_SUPPLY_OVERCURRENT_ITLK             0x00000008
#define FAC_CMD_IDB_SUPPLY_OVERCURRENT_ITLK             0x00000010
#define FAC_CMD_INDUC_OVERTEMP_ITLK                     0x00000020
#define FAC_CMD_HS_OVERTEMP_ITLK                        0x00000040
#define FAC_CMD_MAIN_OVER_CURRENT_ITLK                  0x00000080
#define FAC_CMD_EMERGENCY_BUTTON_ITLK                   0x00000100
#define FAC_CMD_MAIN_OVER_VOLTAGE_ITLK                  0x00000200
#define FAC_CMD_MAIN_UNDER_VOLTAGE_ITLK                 0x00000400
#define FAC_CMD_GROUND_LKG_ITLK                         0x00000800
#define FAC_CMD_BOARD_IIB_OVERTEMP_ITLK                 0x00001000
#define FAC_CMD_BOARD_IIB_OVERHUMIDITY_ITLK             0x00002000

/////////////////////////////////////////////////////////////////////////////////////////////

#define FAC_CMD_CAPBANK_OVERVOLTAGE_ALM                 0x00000001
#define FAC_CMD_OUTPUT_OVERVOLTAGE_ALM                  0x00000002
#define FAC_CMD_AUX_AND_IDB_SUPPLY_OVERVOLTAGE_ALM      0x00000004
#define FAC_CMD_AUX_SUPPLY_OVERCURRENT_ALM              0x00000008
#define FAC_CMD_IDB_SUPPLY_OVERCURRENT_ALM              0x00000010
#define FAC_CMD_INDUC_OVERTEMP_ALM                      0x00000020
#define FAC_CMD_HS_OVERTEMP_ALM                         0x00000040
#define FAC_CMD_GROUND_LKG_ALM                          0x00000080
#define FAC_CMD_BOARD_IIB_OVERTEMP_ALM                  0x00000100
#define FAC_CMD_BOARD_IIB_OVERHUMIDITY_ALM              0x00000200

/////////////////////////////////////////////////////////////////////////////////////////////

extern void clear_fac_cmd_interlocks(void);
extern uint8_t check_fac_cmd_interlocks(void);
extern void clear_fac_cmd_alarms(void);
extern uint8_t check_fac_cmd_alarms(void);
extern void check_fac_cmd_indication_leds(void);
extern void fac_cmd_application_readings(void);
extern void config_module_fac_cmd(void);

extern volatile fac_cmd_t fac_cmd;

/////////////////////////////////////////////////////////////////////////////////////////////

//Set Power Module Alarms And Interlocks

//Rack PA-RaPSC01 / PA-RaPSC02

//BO-FAM:PS-QF

#ifdef BO_FAM_PS_QF__CMD

#include <iib_ps_parameters/PA/fac/bo_fam_ps_qf__cmd.h>

#endif /* BO_FAM_PS_QF__CMD */

/////////////////////////////////////////////////////////////////////////////////////////////

//Set Power Module Alarms And Interlocks

//Rack PA-RaPSE01 / PA-RaPSE02 / PA-RaPSE03 / PA-RaPSE04 / PA-RaPSE06 / PA-RaPSE07 / PA-RaPSE08 / PA-RaPSE09

//BO-FAM:PS-B-1

#ifdef BO_FAM_PS_B1__CMD

#include <iib_ps_parameters/PA/fac/bo_fam_ps_b1__cmd.h>

#endif /* BO_FAM_PS_B1__CMD */

/////////////////////////////////////////////////////////////////////////////////////////////

//Set Power Module Alarms And Interlocks

//Rack PA-RaPSF01 / PA-RaPSF02 / PA-RaPSF03 / PA-RaPSF04 / PA-RaPSF06 / PA-RaPSF07 / PA-RaPSF08 / PA-RaPSF09

//BO-FAM:PS-B-2

#ifdef BO_FAM_PS_B2__CMD

#include <iib_ps_parameters/PA/fac/bo_fam_ps_b2__cmd.h>

#endif /* BO_FAM_PS_B2__CMD */

/////////////////////////////////////////////////////////////////////////////////////////////

//Set Power Module Alarms And Interlocks

//FAC Giga de Testes CMD

#ifdef FAC_GIGA_TESTE__CMD

#include <iib_ps_parameters/development/fac/fac_giga_teste__cmd.h>

#endif /* FAC_GIGA_TESTE__CMD */

/////////////////////////////////////////////////////////////////////////////////////////////

#endif /* FAC_CMD_H_ */

/////////////////////////////////////////////////////////////////////////////////////////////




