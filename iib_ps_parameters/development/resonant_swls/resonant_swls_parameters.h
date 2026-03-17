/*
 * resonant_swls_parameters.h
 *
 *  Created on: 03 de may de 2023
 *      Author: rogerio.marcondeli
 */

/////////////////////////////////////////////////////////////////////////////////////////////

#ifndef RESONANT_SWLS_PARAMETERS_H_
#define RESONANT_SWLS_PARAMETERS_H_

/////////////////////////////////////////////////////////////////////////////////////////////

//Set Power Module Alarms And Interlocks

//Resonant SWLS

///////////////////////

#define RES_SWLS

#define ON                                      	      				1
#define OFF                                               				0

#define RESONANT_SWLS_INPUT_OVERVOLTAGE_ALM_LIM           				360.0
#define RESONANT_SWLS_INPUT_OVERVOLTAGE_ITLK_LIM          				370.0

#define RESONANT_SWLS_OUTPUT_OVERVOLTAGE_ALM_LIM          				10.5
#define RESONANT_SWLS_OUTPUT_OVERVOLTAGE_ITLK_LIM         				11.0

#define RESONANT_SWLS_INPUT_OVERCURRENT_ALM_LIM           				10.5
#define RESONANT_SWLS_INPUT_OVERCURRENT_ITLK_LIM          				11.0

#define RESONANT_SWLS_OUTPUT_OVERCURRENT_ALM_LIM          				310.0
#define RESONANT_SWLS_OUTPUT_OVERCURRENT_ITLK_LIM         				320.0

#define RESONANT_SWLS_GROUND_LEAKAGE_ALM_LIM              				0.450
#define RESONANT_SWLS_GROUND_LEAKAGE_ITLK_LIM             				0.475

#define RESONANT_SWLS_HS_MAIN_BOARD_OVERTEMP_ALM_LIM			        50.0
#define RESONANT_SWLS_HS_MAIN_BOARD_OVERTEMP_ITLK_LIM			        60.0

#define RESONANT_SWLS_HS_TRANSFORMER_OVERTEMP_ALM_LIM        		    50.0
#define RESONANT_SWLS_HS_TRANSFORMER_OVERTEMP_ITLK_LIM       		    60.0

#define RESONANT_SWLS_HS_DIODES_OVERTEMP_ALM_LIM		        		60.0
#define RESONANT_SWLS_HS_DIODES_OVERTEMP_ITLK_LIM       				70.0

#define RESONANT_SWLS_INDUC_OUTPUT_OVERTEMP_ALM_LIM        				50.0
#define RESONANT_SWLS_INDUC_OUTPUT_OVERTEMP_ITLK_LIM       				60.0

#define RESONANT_SWLS_DRIVER_MOSFETS_AND_AUX_OVERVOLTAGE_ALM_LIM 		16.0
#define RESONANT_SWLS_DRIVER_MOSFETS_AND_AUX_OVERVOLTAGE_ITLK_LIM  		17.0

#define RESONANT_SWLS_DRIVER1_MOSFETS_OVERCURRENT_ALM_LIM 				2.0
#define RESONANT_SWLS_DRIVER1_MOSFETS_OVERCURRENT_ITLK_LIM				2.4

#define RESONANT_SWLS_AUX_SUPPLY_OVERCURRENT_ALM_LIM 					2.0
#define RESONANT_SWLS_AUX_SUPPLY_OVERCURRENT_ITLK_LIM					2.4

#define RESONANT_SWLS_BOARD_IIB_OVERHUMIDITY_ALM_LIM 					80.0
#define RESONANT_SWLS_BOARD_IIB_OVERHUMIDITY_ITLK_LIM 					90.0

#define RESONANT_SWLS_BOARD_IIB_OVERTEMP_ALM_LIM 		        		50.0
#define RESONANT_SWLS_BOARD_IIB_OVERTEMP_ITLK_LIM 		      			60.0

/////////////////////////////////////////////////////////////////////////////////////////////

//Set current range FAP 130 A

//Sensor Hall LEM LA 130-P

//CurrentCh1Init

#define LA_Primary_Current                      				130.0

#define LA_Secondary_Current                    				0.130

#define LA_Burden_Resistor                      				50.0

//Debouncing delay_us
#define LA_Delay                                				10

/////////////////////////////////////////////////////////////////////////////////////////////

//Set current range

//Sensor Hall LEM LF 510-S

//CurrentCh2Init

#define Hall_Primary_Current                                	800.0 //Primary current, measuring range max value

#define Hall_Secondary_Current                              	0.160 //Secondary current, measuring range max value

#define Hall_Burden_Resistor                                	50.0

//Debouncing delay_us
#define Hall_Delay                                          	10

#define CurrentCh1Enable                        				ON
#define CurrentCh2Enable                        				ON
#define CurrentCh3Enable                        				OFF
#define CurrentCh4Enable                        				OFF

/////////////////////////////////////////////////////////////////////////////////////////////

//Set LV 20P range

//LvCurrentCh1Init and LvCurrentCh2Init

#define LV_Primary_Voltage_Vin                  				402.5 //resistores mais 250 ohms do LV 20P

#define LV_Primary_Voltage_Vout                 				16.5  //resistores mais 250 ohms do LV 20P

#define LV_Secondary_Current_Vin                				0.025

#define LV_Burden_Resistor                      				120.0

//Debouncing delay_us
#define Delay_Vin                               				100

//Debouncing delay_us
#define Delay_Vout                              				100

#define LvCurrentCh1Enable                      				ON
#define LvCurrentCh2Enable                      				ON
#define LvCurrentCh3Enable                      				OFF

/////////////////////////////////////////////////////////////////////////////////////////////

//PT100 CH1 and CH2 and CH3 and CH4 configuration
//Debouncing delay_ms

#define Delay_PT100CH1                          				4
#define Delay_PT100CH2                          				4
#define Delay_PT100CH3                          				4
#define Delay_PT100CH4                          				4

#define Pt100Ch1Enable                          				ON
#define Pt100Ch2Enable                          				ON
#define Pt100Ch3Enable                          				ON
#define Pt100Ch4Enable                          				ON

/////////////////////////////////////////////////////////////////////////////////////////////

//Temperature Board and Humidity Board configuration
//Debouncing delay_ms

#define Delay_BoardTemp                         				3
#define Delay_BoardRh                           				3

#define BoardTempEnable                         				ON
#define RhEnable                                				ON

#define ItlkBoardTempEnable                     				OFF
#define ItlkRhEnable                            				OFF

/////////////////////////////////////////////////////////////////////////////////////////////

//Driver Voltage and Driver Current configuration
//Debouncing delay_ms

#define Delay_DriverVoltage                     				3
#define Delay_DriverCurrent                     				3

#define DriverVoltageEnable                     				ON  //Voltage Driver 1 and AUX enable
#define Driver1CurrentEnable                    				ON  //Current Driver 1 enable
#define Driver2CurrentEnable                    				ON  //Current Aux enable

/////////////////////////////////////////////////////////////////////////////////////////////

//Voltage configuration

//VoltageCh1 convert to Current

#define Gain_LDC_Transducer 4.0 //Gain Transducer CTSR 0.3-P

//#define Gain_LDC_Transducer 1.2 //Gain Transducer CTSR 1-P

#define Gain_LDC_Amplifier 4.99

#define Current_GND_Leakage 10.0 / (Gain_LDC_Transducer * Gain_LDC_Amplifier)

//Debouncing delay_us
#define Delay_GND_Leakage                       				100

#define VoltageCh1Enable                        				ON
#define VoltageCh2Enable                        				OFF
#define VoltageCh3Enable                        				OFF
#define VoltageCh4Enable                        				OFF

/////////////////////////////////////////////////////////////////////////////////////////////

//Gpdi configuration

#define Gpdi1Enable                             				OFF
#define Gpdi2Enable                             				OFF
#define Gpdi3Enable                             				OFF
#define Gpdi4Enable                             				OFF
#define Gpdi5Enable                             				ON  // Emergency Button
#define Gpdi6Enable                             				ON  // Contactor K1 Over Current
#define Gpdi7Enable                             				ON  // Status Contactor K1
#define Gpdi8Enable                             				ON  // Status Contactor K2
#define Gpdi9Enable                             				OFF
#define Gpdi10Enable                            				OFF
#define Gpdi11Enable                            				OFF
#define Gpdi12Enable                            				OFF

/////////////////////////////////////////////////////////////////////////////////////////////

//Gpdo configuration

#define Gpdo1Enable                             				OFF
#define Gpdo2Enable                             				OFF
#define Gpdo3Enable                             				OFF
#define Gpdo4Enable                             				OFF

/////////////////////////////////////////////////////////////////////////////////////////////

//ReleAux and ReleExtItlk configuration

#define ReleAuxEnable                           				ON
#define ReleExtItlkEnable                       				OFF

/////////////////////////////////////////////////////////////////////////////////////////////

#endif /* RESONANT_SWLS_PARAMETERS_H_ */

/////////////////////////////////////////////////////////////////////////////////////////////




