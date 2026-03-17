/******************************************************************************
 * Copyright (C) 2023 by LNLS - Brazilian Synchrotron Light Laboratory
 *
 * Redistribution, modification or use of this software in source or binary
 * forms is permitted as long as the files maintain this copyright. LNLS and
 * the Brazilian Center for Research in Energy and Materials (CNPEM) are not
 * liable for any misuse of this material.
 *
 *****************************************************************************/

/**
 * @file resonant_swls.h
 * @brief Brief description of module
 * 
 * Detailed description
 *
 * @author rogerio.marcondeli
 * @date 02/05/2023
 *
 */

/////////////////////////////////////////////////////////////////////////////////////////////

#ifndef RESONANT_SWLS_H_
#define RESONANT_SWLS_H_

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
    } Vin;

    volatile bool VinAlarmSts;
    volatile bool VinItlkSts;

    union {
    	volatile float       f;
    	volatile uint8_t     u8[4];
    } Vout;

    volatile bool VoutAlarmSts;
    volatile bool VoutItlkSts;

    union {
        volatile float       f;
        volatile uint8_t     u8[4];
    } Iin;

    volatile bool IinAlarmSts;
    volatile bool IinItlkSts;

    union {
        volatile float       f;
        volatile uint8_t     u8[4];
    } Iout;

    volatile bool IoutAlarmSts;
    volatile bool IoutItlkSts;

    union {
    	volatile float       f;
    	volatile uint8_t     u8[4];
    } GroundLeakage;

    volatile bool GroundLeakageAlarmSts;
    volatile bool GroundLeakageItlkSts;

    union {
    	volatile float       f;
    	volatile uint8_t     u8[4];
    } TempHeatSinkMainBoard;

    volatile bool TempHeatSinkMainBoardAlarmSts;
    volatile bool TempHeatSinkMainBoardItlkSts;

    union {
    	volatile float       f;
    	volatile uint8_t     u8[4];
    } TempHeatSinkTransformer;

    volatile bool TempHeatSinkTransformerAlarmSts;
    volatile bool TempHeatSinkTransformerItlkSts;

    union {
    	volatile float       f;
    	volatile uint8_t     u8[4];
    } TempHeatSinkDiodes;

    volatile bool TempHeatSinkDiodesAlarmSts;
    volatile bool TempHeatSinkDiodesItlkSts;

    union {
    	volatile float       f;
    	volatile uint8_t     u8[4];
    } TempOutputInductor;

    volatile bool TempOutputInductorAlarmSts;
    volatile bool TempOutputInductorItlkSts;

    union {
    	volatile float       f;
    	volatile uint8_t     u8[4];
    } DriverAuxVoltage;

    volatile bool DriverAuxVoltageAlarmSts;
    volatile bool DriverAuxVoltageItlkSts;

    union {
    	volatile float       f;
    	volatile uint8_t     u8[4];
    } Driver1Current;

    volatile bool Driver1CurrentAlarmSts;
    volatile bool Driver1CurrentItlkSts;

    union {
        volatile float       f;
        volatile uint8_t     u8[4];
    } AuxCurrent;

    volatile bool AuxCurrentAlarmSts;
    volatile bool AuxCurrentItlkSts;

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

    volatile bool ContactorK1;
    volatile bool ContactorK1OpenItlkSts;
    volatile bool ContactorK1ContactStickingItlkSts;

    volatile bool ContactorK2;
    volatile bool ContactorK2OpenItlkSts;
    volatile bool ContactorK2ContactStickingItlkSts;

    volatile bool ContactorK1OverCurrentItlk;
    volatile bool ContactorK1OverCurrentItlkSts;

    volatile bool EmergencyButtonItlk;
    volatile bool EmergencyButtonItlkSts;

    volatile bool ReleAuxItlkSts;

} resonant_swls_t;

/////////////////////////////////////////////////////////////////////////////////////////////

#define RESONANT_SWLS_INPUT_OVERVOLTAGE_ITLK          				0x00000001
#define RESONANT_SWLS_OUTPUT_OVERVOLTAGE_ITLK         				0x00000002
#define RESONANT_SWLS_INPUT_OVERCURRENT_ITLK          				0x00000004
#define RESONANT_SWLS_OUTPUT_OVERCURRENT_ITLK         				0x00000008
#define RESONANT_SWLS_HS_MAIN_BOARD_OVERTEMP_ITLK                   0x00000010
#define RESONANT_SWLS_HS_TRANSFORMER_OVERTEMP_ITLK                  0x00000020
#define RESONANT_SWLS_HS_DIODES_OVERTEMP_ITLK                       0x00000040
#define RESONANT_SWLS_INDUC_OUTPUT_OVERTEMP_ITLK      				0x00000080
#define RESONANT_SWLS_DRIVER_MOSFETS_AND_AUX_OVERVOLTAGE_ITLK   	0x00000100
#define RESONANT_SWLS_DRIVER1_MOSFETS_OVERCURRENT_ITLK				0x00000200
#define RESONANT_SWLS_AUX_SUPPLY_OVERCURRENT_ITLK          			0x00000400
#define RESONANT_SWLS_GROUND_LKG_ITLK                     		 	0x00000800
#define RESONANT_SWLS_BOARD_IIB_OVERTEMP_ITLK         				0x00001000
#define RESONANT_SWLS_BOARD_IIB_OVERHUMIDITY_ITLK     				0x00002000
#define RESONANT_SWLS_CONTACTOR_K1_OVER_CURRENT_ITLK                0x00004000
#define RESONANT_SWLS_CONTACTOR_K1_OPEN_ITLK                        0x00008000
#define RESONANT_SWLS_CONTACTOR_K1_CONTACT_STICKING_ITLK            0x00010000
#define RESONANT_SWLS_CONTACTOR_K2_OPEN_ITLK                        0x00020000
#define RESONANT_SWLS_CONTACTOR_K2_CONTACT_STICKING_ITLK            0x00040000
#define RESONANT_SWLS_EMERGENCY_BUTTON_ITLK                			0x00080000

/////////////////////////////////////////////////////////////////////////////////////////////

#define RESONANT_SWLS_INPUT_OVERVOLTAGE_ALM           				0x00000001
#define RESONANT_SWLS_OUTPUT_OVERVOLTAGE_ALM          				0x00000002
#define RESONANT_SWLS_INPUT_OVERCURRENT_ALM               			0x00000004
#define RESONANT_SWLS_OUTPUT_OVERCURRENT_ALM        				0x00000008
#define RESONANT_SWLS_HS_MAIN_BOARD_OVERTEMP_ALM                    0x00000010
#define RESONANT_SWLS_HS_TRANSFORMER_OVERTEMP_ALM           		0x00000020
#define RESONANT_SWLS_HS_DIODES_OVERTEMP_ALM                        0x00000040
#define RESONANT_SWLS_INDUC_OUTPUT_OVERTEMP_ALM                     0x00000080
#define RESONANT_SWLS_DRIVER_MOSFETS_AND_AUX_OVERVOLTAGE_ALM		0x00000100
#define RESONANT_SWLS_DRIVER1_MOSFETS_OVERCURRENT_ALM      			0x00000200
#define RESONANT_SWLS_AUX_SUPPLY_OVERCURRENT_ALM           			0x00000400
#define RESONANT_SWLS_GROUND_LKG_ALM                  				0x00000800
#define RESONANT_SWLS_BOARD_IIB_OVERTEMP_ALM          				0x00001000
#define RESONANT_SWLS_BOARD_IIB_OVERHUMIDITY_ALM      				0x00002000

/////////////////////////////////////////////////////////////////////////////////////////////

extern void clear_resonant_swls_interlocks(void);
extern uint8_t check_resonant_swls_interlocks(void);
extern void clear_resonant_swls_alarms(void);
extern uint8_t check_resonant_swls_alarms(void);
extern void check_resonant_swls_indication_leds(void);
extern void resonant_swls_application_readings(void);
extern void config_module_resonant_swls(void);

extern volatile resonant_swls_t resonant_swls;

/////////////////////////////////////////////////////////////////////////////////////////////

//Set Power Module Alarms And Interlocks

//RESONANT_SWLS

#ifdef RESONANT_SWLS

#include <iib_ps_parameters/development/resonant_swls/resonant_swls_parameters.h>

#endif /* RESONANT_SWLS */

/////////////////////////////////////////////////////////////////////////////////////////////

#endif /* RESONANT_SWLS_H_ */

/////////////////////////////////////////////////////////////////////////////////////////////




