
#include "adc_internal.h"
#include "application.h"

#include "BoardTempHum.h"
#include "pt100.h"
#include "output.h"
//#include "rs485.h"
#include "leds.h"
#include "can_bus.h"
#include "input.h"

#include "stdbool.h"
#include "stdint.h"


typedef struct
{
    float Vin;
    bool VinAlarmSts;
    bool VinItlkSts;
    float Vout;
    bool VoutAlarmSts;
    bool VoutItlkSts;
    float IoutA1;
    bool IoutA1AlarmSts;
    bool IoutA1ItlkSts;
    float IoutA2;
    bool IoutA2AlarmSts;
    bool IoutA2ItlkSts;
    uint8_t TempIGBT1;
    bool TempIGBT1AlarmSts;
    bool TempIGBT1ItlkSts;
    bool TempIGBT1HwrItlk;
    bool TempIGBT1HwrItlkSts;
    uint8_t TempIGBT2;
    bool TempIGBT2AlarmSts;
    bool TempIGBT2ItlkSts;
    bool TempIGBT2HwrItlk;
    bool TempIGBT2HwrItlkSts;
    float DriveVoltage;
    float Drive1Current;
    float Drive2Current;
    bool Driver1Error;
    bool Driver1ErrorItlk;
    bool Driver2Error;
    bool Driver2ErrorItlk;
    uint8_t TempL;
    bool TempLAlarmSts;
    bool TempLItlkSts;
    uint8_t TempHeatSink;
    bool TempHeatSinkAlarmSts;
    bool TempHeatSinkItlkSts;
    bool Relay;
    bool ExternalItlk;
    bool ExternalItlkSts;
    bool LeakageCurrent;
    bool LeakageCurrentSts;
    bool Rack;
    bool RackSts;
} Q1Module_t;


/*
*PT100
- PT100CH1: Temperatura do dissipador, interlocar em 50�C;
- PT100CH2: Temperatura do indutor, interlocar em 60�C;

*Leitura de Tens�o via LV20-P
- LCH1: Tens�o do DC-link, rela��o 330V/25mA, interlocar em 300V;

*Leitura de corrente por sensor hall
- ICH1: Corrente de entrada, rela��o 300A/150mA, interlocar em 250A;
- ICH2: Corrente de sa�da, rela��o 500A/100mA, interlocar em 510A;
*/


typedef struct
{
    float Iin;
    bool IinAlarmSts;
    bool IinItlkSts;
    float Iout;
    bool IoutAlarmSts;
    bool IoutItlkSts;
    float VdcLink;
    bool VdcLinkAlarmSts;
    bool VdcLinkItlkSts;
    uint8_t TempIGBT1;
    bool TempIGBT1AlarmSts;
    bool TempIGBT1ItlkSts;
    bool TempIGBT1HwrItlk;
    bool TempIGBT1HwrItlkSts;
    uint8_t TempIGBT2;
    bool TempIGBT2AlarmSts;
    bool TempIGBT2ItlkSts;
    bool TempIGBT2HwrItlk;
    bool TempIGBT2HwrItlkSts;
    uint8_t TempL;
    bool TempLAlarmSts;
    bool TempLItlkSts;
    uint8_t TempHeatSink;
    bool TempHeatSinkAlarmSts;
    bool TempHeatSinkItlkSts;
    bool Driver1Error;
    bool Driver1ErrorItlk;
    bool Driver2Error;
    bool Driver2ErrorItlk;

} Q4Module_t;


//- PT100 CH1: interlockar ao passar de 100�C                                                                Indutor 1
//- PT100 CH2: interlockar ao passar de 100�C                                                                Indutor 2
//- Hall Sensor 1: Sensor hall de 300A/150mA, interlockar em 170A                                            Corrente de Entrada 1
//- Hall Sensor 2: Sensor hall de 300A/150mA, interlockar em 170A                                            Corrente de Entrada 2
//- Hall Sensor 3: Sensor hall de 300A/150mA, interlockar em 200A                                            Corrente de Sa�da 1
//- Hall Sensor 4: Sensor hall de 300A/150mA, interlockar em 200A                                            Corrente de Sa�da 2
//- Voltage Input CH1: Interlockar acima de 5V                                                               Tens�o de Sa�da 1
//- Voltage Input CH2: Interlockar acima de 5V                                                               Tens�o de Sa�da 2
//- Digital Input CH1: Acionar interlock quando em n�vel l�gico baixo                                        Interlock Externo

typedef struct
{

    float Iin;
    bool IinAlarmSts;
    bool IinItlkSts;
    float VdcLink;
    bool VdcLinkAlarmSts;
    bool VdcLinkItlkSts;
    uint8_t TempHeatsink;
    bool TempHeatsinkAlarmSts;
    bool TempHeatsinkItlkSts;
    uint8_t TempL;
    bool TempLAlarmSts;
    bool TempLItlkSts;
    bool Driver1Error;
    bool Driver1ErrorItlk;
    bool Driver2Error;
    bool Driver2ErrorItlk;

} InputModule_t;

typedef struct
{
    float IoutRectf1;
    bool IoutRectf1AlarmSts;
    bool IoutRectf1ItlkSts;
    float IoutRectf2;
    bool IoutRectf2AlarmSts;
    bool IoutRectf2ItlkSts;
    float VoutRectf1;
    bool VoutRectf1AlarmSts;
    bool VoutRectf1ItlkSts;
    float VoutRectf2;
    bool VoutRectf2AlarmSts;
    bool VoutRectf2ItlkSts;
    float LeakageCurrent;
    bool LeakageCurrentAlarmSts;
    bool LeakageCurrentItlkSts;
    uint8_t TempHeatSink;
    bool TempHeatSinkAlarmSts;
    bool TempHeatSinkItlkSts;
    uint8_t TempWater;
    bool TempWaterAlarmSts;
    bool TempWaterItlkSts;
    uint8_t TempModule1;
    bool TempModule1AlarmSts;
    bool TempModule1ItlkSts;
    uint8_t TempModule2;
    bool TempModule2AlarmSts;
    bool TempModule2ItlkSts;
    uint8_t TempL1;
    bool TempL1AlarmSts;
    bool TempL1ItlkSts;
    uint8_t TempL2;
    bool TempL2AlarmSts;
    bool TempL2ItlkSts;
    bool AcPhaseFault;
    bool AcPhaseFaultSts;
    bool AcOverCurrent;
    bool AcOverCurrentSts;
    bool AcTransformerOverTemp;
    bool AcTransformerOverTempSts;
    bool WaterFluxInterlock;
    bool WaterFluxInterlockSts;

} RectifierModule_t;

typedef struct
{
    /* PT100CH1 */
    uint8_t TempHeatSink;
    bool TempHeatSinkAlarmSts;
    bool TempHeatSinkItlkSts;
    /* PT100CH2 */
    uint8_t TempL;
    bool TempLAlarmSts;
    bool TempLItlkSts;
    /* LCH1 */
    float VcapBank;
    bool VcapBankAlarmSts;
    bool VcapBankItlkSts;
    /* LCH2 */
    float Vout;
    bool VoutAlarmSts;
    bool VoutItlkSts;
    /* GPDIO1 */
    bool ExtItlkSts;
    /* GPDI05*/
    bool ExtItlk2Sts;

} CommandDrawerModule_t;

static Q1Module_t               Q1Module;
static Q4Module_t               Q4Module;
static InputModule_t            InputModule;
static RectifierModule_t        Rectifier;
static CommandDrawerModule_t    CommandDrawer;

static unsigned char PowerModuleModel = 0;
static unsigned char Interlock = 0;
static unsigned char InterlockOld = 0;
static unsigned char ItlkClrCmd = 0;
static unsigned char InitApp = 0;
static unsigned char InitAppOld = 0;
static unsigned char Alarm = 0;

void AppConfiguration(void)
{

    // Set Power Module Model
    // This parameter guide the firmware behavior
    // Each Model has a different variable list that need to be check

    //PowerModuleModel = OUTPUT_Q1_MODULE;
    PowerModuleModel = OUTPUT_Q4_MODULE;
    //PowerModuleModel = RECTIFIER_MODULE;
    //PowerModuleModel = INPUT_MODULE;
    //PowerModuleModel = COMMAND_DRAWER_MODULE;
    
    switch(PowerModuleModel)
    {
        case OUTPUT_Q1_MODULE:

            init_q1_module();

            break;

        case OUTPUT_Q4_MODULE:

            init_q4_module();

            break;
         
        case RECTIFIER_MODULE:
            //Set current range
            CurrentCh1Init(300.0, 0.150, 50.0, 3);  // Rectifier1 Output Current Sensor Configuration: Hall Sensor
            CurrentCh2Init(300.0, 0.150, 50.0, 3);  // Rectifier2 Output Current Sensor Configuration: LEM LF 310-S
            CurrentCh3Init(1.0, 0.0125, 300.0, 0);  // Leakage Current
            CurrentCh4Init(125.0, 0.125, 50.0, 0);
            //Set protection limits
            CurrentCh1AlarmLevelSet(230.0);        // Rectifier1 Output Current Alarm
            CurrentCh1TripLevelSet(235.0);         // Rectifier1 Output Current Trip
            CurrentCh2AlarmLevelSet(230.0);        // Rectifier2 Output Current Alarm
            CurrentCh2TripLevelSet(235.0);         // Rectifier2 Output Current Trip
            CurrentCh3AlarmLevelSet(0.09);         // Leakage Current Alarm Level
            CurrentCh3TripLevelSet(0.1);           // Leakage Current Trip Level
            CurrentCh4AlarmLevelSet(100.0);
            CurrentCh4TripLevelSet(100.0);

            //Setar ranges de entrada
            VoltageCh1Init(61.21, 3);              // Rectifier1 Output Voltage Configuration.
            VoltageCh2Init(61.21, 3);              // Rectifier2 Output Voltage Configuration.
            VoltageCh3Init(10.0, 3);               // NTC SW1
            VoltageCh4Init(10.0, 3);               // NTC SW2

            ConfigPolVoltCh2(1);                   // Change the voltage polarity of the channel 2 (rectifier 2 voltage)

            ConfigVoltCh1AsNtc(0);                 // Config Voltage Ch2 as a voltage input
            ConfigVoltCh2AsNtc(0);                 // Config Voltage Ch2 as a voltage input
            ConfigVoltCh3AsNtc(1);                 // Config Voltage Ch3 as a NTC input
            ConfigVoltCh4AsNtc(1);                 // Config Voltage Ch4 as a NTC input

            //Setar limites
            VoltageCh1AlarmLevelSet(55.0);         // Rectifier1 Voltage Alarm
            VoltageCh1TripLevelSet(58.0);          // Rectifier1 Voltage Trip
            VoltageCh2AlarmLevelSet(55.0);         // Rectifier2 Voltage Alarm
            VoltageCh2TripLevelSet(58.0);          // Rectifier2 Voltage Trip
            VoltageCh3AlarmLevelSet(115.0);        // Rectifier1 NTC Temperature Alarm
            VoltageCh3TripLevelSet(125.0);         // Rectifier1 NTC Temperature Trip
            VoltageCh4AlarmLevelSet(115.0);        // Rectifier2 NTC Temperature Alarm
            VoltageCh4TripLevelSet(125.0);         // Rectifier2 NTC Temperature Tip

            // PT100 configuration limits
            Pt100SetCh1AlarmLevel(70);             // Heat Sink Temperature Alarm
            Pt100SetCh1TripLevel(80);              // Heat Sink Temperature Trip
            Pt100SetCh2AlarmLevel(55);             // L1 Temperature Alarm
            Pt100SetCh2TripLevel(60);              // L1 Temperature Trip
            Pt100SetCh3AlarmLevel(55);             // L2 Temperature Alarm
            Pt100SetCh3TripLevel(60);              // L2 Temperature Trip
            Pt100SetCh4AlarmLevel(55);             // Water Temperature Alarm
            Pt100SetCh4TripLevel(60);              // Water Temperature Trip

            // Delay 4 seconds
            Pt100SetCh1Delay(4);
            // Delay 4 seconds
            Pt100SetCh2Delay(4);
            // Delay 4 seconds
            Pt100SetCh3Delay(4);
            // Delay 4 seconds
            Pt100SetCh4Delay(4);

            // PT100 channel enable
            Pt100Ch1Enable();            // Enable PT100 channel 1
            Pt100Ch2Enable();            // Enable PT100 channel 2
            Pt100Ch3Enable();            // Enable PT100 channel 3
            Pt100Ch4Disable();           // Enable PT100 channel 4

            // Rh configuration limits
            RhAlarmLimitSet(80);
            RhTripLimitSet(90);

            // Temp board configuration limits
            TempBoardAlarmLimitSet(80);
            TempBoardTripLimitSet(90);

            Driver1ErrDisable();         // Driver1 Error Signal Disable
            Driver2ErrDisable();         // Driver1 Error Signal Disable

            // Init variables
            Rectifier.IoutRectf1               = 0;
            Rectifier.IoutRectf1AlarmSts       = 0;
            Rectifier.IoutRectf1ItlkSts        = 0;

            Rectifier.IoutRectf2               = 0;
            Rectifier.IoutRectf2AlarmSts       = 0;
            Rectifier.IoutRectf2ItlkSts        = 0;

            Rectifier.VoutRectf1               = 0;
            Rectifier.VoutRectf1AlarmSts       = 0;
            Rectifier.VoutRectf1ItlkSts        = 0;

            Rectifier.VoutRectf2               = 0;
            Rectifier.VoutRectf2AlarmSts       = 0;
            Rectifier.VoutRectf2ItlkSts        = 0;

            Rectifier.LeakageCurrent           = 0;
            Rectifier.LeakageCurrentAlarmSts   = 0;
            Rectifier.LeakageCurrentItlkSts    = 0;

            Rectifier.TempHeatSink             = 0;
            Rectifier.TempHeatSinkAlarmSts     = 0;
            Rectifier.TempHeatSinkItlkSts      = 0;

            Rectifier.TempWater                = 0;
            Rectifier.TempWaterAlarmSts        = 0;
            Rectifier.TempWaterItlkSts         = 0;

            Rectifier.TempModule1              = 0;
            Rectifier.TempModule1AlarmSts      = 0;
            Rectifier.TempModule1ItlkSts       = 0;

            Rectifier.TempModule2              = 0;
            Rectifier.TempModule2AlarmSts      = 0;
            Rectifier.TempModule2ItlkSts       = 0;

            Rectifier.TempL1                   = 0;
            Rectifier.TempL1AlarmSts           = 0;
            Rectifier.TempL1ItlkSts            = 0;

            Rectifier.TempL2                   = 0;
            Rectifier.TempL2AlarmSts           = 0;
            Rectifier.TempL2ItlkSts            = 0;

            Rectifier.AcPhaseFault             = 0;
            Rectifier.AcPhaseFaultSts          = 0;

            Rectifier.AcOverCurrent            = 0;
            Rectifier.AcOverCurrentSts         = 0;

            Rectifier.AcTransformerOverTemp    = 0;
            Rectifier.AcTransformerOverTempSts = 0;

            Rectifier.WaterFluxInterlock       = 0;
            Rectifier.WaterFluxInterlockSts    = 0;
            break;

        case INPUT_MODULE:
            //Set Current Range
            CurrentCh1Init(300.0, 0.150, 50.0, 10);    // INPUT CURRENT

            //Set Protection Limits
            CurrentCh1AlarmLevelSet(160);          // INPUT CURRENT ALARM LEVEL
            CurrentCh1TripLevelSet(170);           // INPUT CURRENT TRIP LEVEL

            //LV20-P INPUTS
            LvCurrentCh1Init(550.0, 0.025, 120.0, 10); // CONFIG CHANNEL FOR DC_LINK MEASURE

            //LV20-P LIMITS
            CurrentCh1AlarmLevelSet(535);         // INPUT DC_LINK VOLTAGE ALARM LEVEL
            CurrentCh1TripLevelSet(540);          // INPUT DC_LINK VOLTAGE TRIP LEVEL

            // PT100 configuration limits
            Pt100SetCh1AlarmLevel(45);            // HEATSINK TEMPERATURE ALARM LEVEL
            Pt100SetCh1TripLevel(50);             // HEATSINK TEMPERATURE TRIP LEVEL
            Pt100SetCh2AlarmLevel(55);            // INDUCTOR TEMPERATURE ALARM LEVEL
            Pt100SetCh2TripLevel(60);             // INDUCTOR TEMPERATURE TRIP LEVEL
            // PT100 channel enable
            Pt100Ch1Enable();                     // HEATSINK TEMPERATURE CHANNEL ENABLE
            Pt100Ch2Enable();                     // INDUCTOR TEMPERATURE CHANNEL ENABLE
            Pt100Ch3Disable();
            Pt100Ch4Disable();

            // Delay 4 seconds
            Pt100SetCh1Delay(4);
            // Delay 4 seconds
            Pt100SetCh2Delay(4);
            // Delay 4 seconds
            Pt100SetCh3Delay(4);
            // Delay 4 seconds
            Pt100SetCh4Delay(4);

            // Rh configuration limits
            RhAlarmLimitSet(80);
            RhTripLimitSet(90);

            // Temp board configuration limits
            TempBoardAlarmLimitSet(80);
            TempBoardTripLimitSet(90);

            // Disable all Driver Error Monitoring
            Driver1ErrDisable();
            Driver2ErrDisable();

            // Init Variables
            InputModule.Iin                        = 0.0;
            InputModule.IinAlarmSts                = 0;
            InputModule.IinItlkSts                 = 0;

            InputModule.VdcLink                    = 0.0;
            InputModule.VdcLinkAlarmSts            = 0;
            InputModule.VdcLinkItlkSts             = 0;

            InputModule.TempHeatsink               = 0.0;
            InputModule.TempHeatsinkAlarmSts       = 0;
            InputModule.TempHeatsinkItlkSts        = 0;

            InputModule.TempL                      = 0.0;
            InputModule.TempLAlarmSts              = 0;
            InputModule.TempLItlkSts               = 0;

            InputModule.Driver1Error               = 0;
            InputModule.Driver1ErrorItlk           = 0;

            InputModule.Driver2Error               = 0;
            InputModule.Driver2ErrorItlk           = 0;
            break;

        case COMMAND_DRAWER_MODULE:
            //Setar ranges de entrada
            VoltageCh1Init(330.0, 3);                 // Capacitors Voltage Configuration.
            VoltageCh2Init(250.0, 3);                 // Output Voltage Configuration.

            ConfigVoltCh1AsNtc(0);                 // Config Voltage Ch1 as a voltage input
            ConfigVoltCh2AsNtc(0);                 // Config Voltage Ch2 as a voltage input

            //Setar limites
            VoltageCh1AlarmLevelSet(250.0);           // Rectifier1 Voltage Alarm
            VoltageCh1TripLevelSet(300.0);            // Rectifier1 Voltage Trip
            VoltageCh2AlarmLevelSet(180.0);           // Rectifier2 Voltage Alarm
            VoltageCh2TripLevelSet(210.0);            // Rectifier2 Voltage Trip

            // PT100 configuration limits
            Pt100SetCh1AlarmLevel(55);            // HEATSINK TEMPERATURE ALARM LEVEL
            Pt100SetCh1TripLevel(60);             // HEATSINK TEMPERATURE TRIP LEVEL
            Pt100SetCh2AlarmLevel(55);            // INDUCTOR TEMPERATURE ALARM LEVEL
            Pt100SetCh2TripLevel(60);             // INDUCTOR TEMPERATURE TRIP LEVEL
            // PT100 channel enable
            Pt100Ch1Enable();                     // HEATSINK TEMPERATURE CHANNEL ENABLE
            Pt100Ch2Enable();                     // INDUCTOR TEMPERATURE CHANNEL ENABLE
            Pt100Ch3Disable();
            Pt100Ch4Disable();

            // Delay 4 seconds
            Pt100SetCh1Delay(4);
            // Delay 4 seconds
            Pt100SetCh2Delay(4);
            // Delay 4 seconds
            Pt100SetCh3Delay(4);
            // Delay 4 seconds
            Pt100SetCh4Delay(4);

            CommandDrawer.VcapBank                 = 0.0;
            CommandDrawer.VcapBankAlarmSts         = 0;
            CommandDrawer.VcapBankItlkSts          = 0;

            CommandDrawer.Vout                     = 0.0;
            CommandDrawer.VoutAlarmSts             = 0;
            CommandDrawer.VoutItlkSts              = 0;

            CommandDrawer.TempHeatSink             = 0;
            CommandDrawer.TempHeatSinkAlarmSts     = 0;
            CommandDrawer.TempHeatSinkItlkSts      = 0;

            CommandDrawer.TempL                    = 0;
            CommandDrawer.TempLAlarmSts            = 0;
            CommandDrawer.TempLItlkSts             = 0;

            CommandDrawer.ExtItlkSts               = 0;

            CommandDrawer.ExtItlk2Sts              = 0;
            break;

    }
    // End of configuration
    // Turn on Led1 (board started)
    Led1TurnOn();

}

// Set Interlock clear command
void InterlockClear(void)
{
    ItlkClrCmd = 1;
}

void InterlockSet(void)
{
    Interlock = 1;
}

void InterlockClearCheck(void)
{
      //if(ItlkClrCmd && Interlock)
      if(ItlkClrCmd)
      {
          Interlock = 0;
          InterlockOld = 0;

          InitApp = 0;
          InitAppOld = 0;

          AdcClearAlarmTrip();
          Pt100ClearAlarmTrip();
          RhTempClearAlarmTrip();
          
          ItlkClrCmd = 0;
          
          switch(PowerModuleModel)
          {
              case OUTPUT_Q1_MODULE:

                  clear_q1_interlocks();

                  break;

              case OUTPUT_Q4_MODULE:

                  clear_q4_interlocks();

                  break;

              case RECTIFIER_MODULE:

                  Rectifier.IoutRectf1AlarmSts          = 0;
                  Rectifier.IoutRectf1ItlkSts           = 0;
                  Rectifier.IoutRectf2AlarmSts          = 0;
                  Rectifier.IoutRectf2ItlkSts           = 0;
                  Rectifier.VoutRectf1AlarmSts          = 0;
                  Rectifier.VoutRectf1ItlkSts           = 0;
                  Rectifier.VoutRectf2AlarmSts          = 0;
                  Rectifier.VoutRectf2ItlkSts           = 0;
                  Rectifier.LeakageCurrentAlarmSts      = 0;
                  Rectifier.LeakageCurrentItlkSts       = 0;
                  Rectifier.TempHeatSinkAlarmSts        = 0;
                  Rectifier.TempHeatSinkItlkSts         = 0;
                  Rectifier.TempWaterAlarmSts           = 0;
                  Rectifier.TempWaterItlkSts            = 0;
                  Rectifier.TempModule1AlarmSts         = 0;
                  Rectifier.TempModule1ItlkSts          = 0;
                  Rectifier.TempModule2AlarmSts         = 0;
                  Rectifier.TempModule2ItlkSts          = 0;
                  Rectifier.TempL1AlarmSts              = 0;
                  Rectifier.TempL1ItlkSts               = 0;
                  Rectifier.TempL2AlarmSts              = 0;
                  Rectifier.TempL2ItlkSts               = 0;
                  Rectifier.AcPhaseFaultSts             = 0;
                  Rectifier.AcOverCurrentSts            = 0;
                  Rectifier.AcTransformerOverTempSts    = 0;
                  Rectifier.WaterFluxInterlockSts       = 0;

                  break;
               
              case INPUT_MODULE:

                  InputModule.IinAlarmSts               = 0;
                  InputModule.IinItlkSts                = 0;
                  InputModule.VdcLinkAlarmSts           = 0;
                  InputModule.VdcLinkItlkSts            = 0;
                  InputModule.TempHeatsinkAlarmSts      = 0;
                  InputModule.TempHeatsinkItlkSts       = 0;
                  InputModule.TempLAlarmSts             = 0;
                  InputModule.TempLItlkSts              = 0;
                  InputModule.Driver1ErrorItlk          = 0;
                  InputModule.Driver2ErrorItlk          = 0;

                  break;

              case COMMAND_DRAWER_MODULE:

                  CommandDrawer.VcapBankAlarmSts        = 0;
                  CommandDrawer.VcapBankItlkSts         = 0;
                  CommandDrawer.VoutAlarmSts            = 0;
                  CommandDrawer.VoutItlkSts             = 0;
                  CommandDrawer.TempHeatSinkAlarmSts    = 0;
                  CommandDrawer.TempHeatSinkItlkSts     = 0;
                  CommandDrawer.TempLAlarmSts           = 0;
                  CommandDrawer.TempLItlkSts            = 0;
                  CommandDrawer.ExtItlkSts              = 0;
                  CommandDrawer.ExtItlk2Sts             = 0;

                  break;
          }
          
      }
}

unsigned char InterlockRead(void)
{
      return Interlock;
}

void AppInterlock(void)
{
      
      // caso haja algum Interlock, o rele auxiliar deve ser desligado e as opera��es cabiveis de Interlock devem ser executadas
      
      // Analisar se todos os interlocks foram apagados para poder liberar o rele auxiliar
      // caso n�o haja mais Interlock, fechar o rele auxiliar
      
      switch(PowerModuleModel)
      {
       case OUTPUT_Q1_MODULE:

            ReleAuxTurnOff();
            ReleItlkTurnOff();

            break;

       case OUTPUT_Q4_MODULE:

            ReleAuxTurnOff();
            ReleItlkTurnOff();
            Gpdo1TurnOff();
            Gpdo2TurnOff();

            break;

       case RECTIFIER_MODULE:

            ReleAuxTurnOff();
            ReleItlkTurnOff();

            break;

       case INPUT_MODULE:

            ReleAuxTurnOff();
            ReleItlkTurnOff();

            break;

       case COMMAND_DRAWER_MODULE:

           ReleAuxTurnOff();
           ReleItlkTurnOff();

           break;

      }
      
}


void AlarmSet(void)
{
      Alarm = 1;
}

void AlarmClear(void)
{
      Alarm = 0;
}

unsigned char AlarmRead(void)
{
      return Alarm;
}

void AppAlarm(void)
{

}

void InterlockAppCheck(void)
{
   unsigned char Test = 0;

   switch(PowerModuleModel)
   {
       case OUTPUT_Q1_MODULE:

           Test = check_q1_interlocks();

           break;
       
       case OUTPUT_Q4_MODULE:

           Test = check_q4_interlocks();

           break;
       
       case RECTIFIER_MODULE:

            Test |= Rectifier.IoutRectf1ItlkSts;
            Test |= Rectifier.IoutRectf2ItlkSts;
            Test |= Rectifier.VoutRectf1ItlkSts;
            Test |= Rectifier.VoutRectf2ItlkSts;
            Test |= Rectifier.LeakageCurrentItlkSts;
            Test |= Rectifier.TempHeatSinkItlkSts;
            Test |= Rectifier.TempWaterItlkSts;
            Test |= Rectifier.TempModule1ItlkSts;
            Test |= Rectifier.TempModule2ItlkSts;
            Test |= Rectifier.TempL1ItlkSts;
            Test |= Rectifier.TempL2ItlkSts;
            Test |= Rectifier.AcPhaseFaultSts;
            Test |= Rectifier.AcOverCurrentSts;
            Test |= Rectifier.AcTransformerOverTempSts;
            Test |= Rectifier.WaterFluxInterlockSts;
            break;
       
       case INPUT_MODULE:

            Test |= InputModule.IinItlkSts;
            Test |= InputModule.VdcLinkItlkSts;
            Test |= InputModule.TempHeatsinkItlkSts;
            Test |= InputModule.TempLItlkSts;
            Test |= InputModule.Driver1ErrorItlk;
            Test |= InputModule.Driver2ErrorItlk;
            break;

       case COMMAND_DRAWER_MODULE:

           Test |= CommandDrawer.TempHeatSinkItlkSts;
           Test |= CommandDrawer.TempLItlkSts;
           Test |= CommandDrawer.VcapBankItlkSts;
           Test |= CommandDrawer.VoutItlkSts;
           Test |= CommandDrawer.ExtItlkSts;
           Test |= CommandDrawer.ExtItlk2Sts;
           break;
   }

   Test |= RhTripStatusRead();

   Test |= DriverVolatgeTripStatusRead();
   Test |= Driver1CurrentTripStatusRead();
   Test |= Driver2CurrentTripStatusRead();

   if(Test)InterlockSet();

}

void AlarmAppCheck(void)
{
   unsigned char Test = 0;
   
   switch(PowerModuleModel)
   {
       case OUTPUT_Q1_MODULE:

            Test = check_q1_alarms();

           break;

       case OUTPUT_Q4_MODULE:

           Test |= check_q4_alarms();

           break;

       case RECTIFIER_MODULE:
            Test |= Rectifier.IoutRectf1AlarmSts;
            Test |= Rectifier.IoutRectf2AlarmSts;
            Test |= Rectifier.VoutRectf1AlarmSts;
            Test |= Rectifier.VoutRectf2AlarmSts;
            Test |= Rectifier.LeakageCurrentAlarmSts;
            Test |= Rectifier.TempHeatSinkAlarmSts;
            Test |= Rectifier.TempWaterAlarmSts;
            Test |= Rectifier.TempModule1AlarmSts;
            Test |= Rectifier.TempModule2AlarmSts;
            Test |= Rectifier.TempL1AlarmSts;
            Test |= Rectifier.TempL2AlarmSts;
            break;

       case INPUT_MODULE:
            Test |= InputModule.IinAlarmSts;
            Test |= InputModule.TempHeatsinkAlarmSts;
            Test |= InputModule.TempLAlarmSts;
            Test |= InputModule.VdcLinkAlarmSts;
            break;

       case COMMAND_DRAWER_MODULE:
           Test |= CommandDrawer.TempHeatSinkAlarmSts;
           Test |= CommandDrawer.TempLAlarmSts;
           Test |= CommandDrawer.VcapBankAlarmSts;
           Test |= CommandDrawer.VoutAlarmSts;
   }

   Test |= RhAlarmStatusRead();


   if(Test)AlarmSet();
}



void LedIndicationStatus(void)
{
    switch(PowerModuleModel)
    {
        case OUTPUT_Q1_MODULE:

           check_q1_indication_leds();

           break;

        case OUTPUT_Q4_MODULE:

           check_q4_indications_leds();

           break;

        case RECTIFIER_MODULE:

            // Rectifier Output Over Voltage
            if(Rectifier.VoutRectf1ItlkSts || Rectifier.VoutRectf2ItlkSts) Led2TurnOn();
            else if(Rectifier.VoutRectf1AlarmSts || Rectifier.VoutRectf2AlarmSts) Led2Toggle();
            else Led2TurnOff();

            // Rectifier Output Over Current
            if(Rectifier.IoutRectf1ItlkSts || Rectifier.IoutRectf1ItlkSts) Led3TurnOn();
            else if(Rectifier.IoutRectf1AlarmSts || Rectifier.IoutRectf1AlarmSts) Led3Toggle();
            else Led3TurnOff();

            // Rectifier Over Temperature
            if(Rectifier.TempHeatSinkItlkSts || Rectifier.TempWaterItlkSts || Rectifier.TempModule1ItlkSts || Rectifier.TempModule2ItlkSts || Rectifier.TempL1ItlkSts || Rectifier.TempL2ItlkSts) Led4TurnOn();
            else if(Rectifier.TempHeatSinkAlarmSts || Rectifier.TempWaterAlarmSts || Rectifier.TempModule1AlarmSts || Rectifier.TempModule2AlarmSts || Rectifier.TempL1AlarmSts || Rectifier.TempL2AlarmSts) Led4Toggle();
            else Led4TurnOff();

            // External interlock or Driver error
            if(Rectifier.AcPhaseFaultSts || Rectifier.AcOverCurrentSts || Rectifier.AcTransformerOverTempSts || Rectifier.WaterFluxInterlockSts) Led5TurnOn();
            else if(!InterlockRead())Led5TurnOff();
            
            break;

        case INPUT_MODULE:

            // Input Over Current
            if(InputModule.IinItlkSts) Led2TurnOn();
            else if(InputModule.IinAlarmSts) Led2Toggle();
            else Led2TurnOff();

            // Dc-Link Overvoltage
            if(InputModule.VdcLinkItlkSts) Led3TurnOn();
            else if(InputModule.VdcLinkAlarmSts) Led3Toggle();
            else Led3TurnOff();

            // Heatsink Over Temperature
            if(InputModule.TempHeatsinkItlkSts) Led4TurnOn();
            else if(InputModule.TempHeatsinkAlarmSts) Led4Toggle();
            else Led4TurnOff();
            
            // Inductor Over Temperature
            if(InputModule.TempLItlkSts) Led5TurnOn();
            else if(InputModule.TempLAlarmSts) Led5Toggle();
            else Led5TurnOff();

            // Driver Error
            if(InputModule.Driver1ErrorItlk || InputModule.Driver2ErrorItlk) Led6TurnOn();
            else Led6TurnOff();

            break;

        case COMMAND_DRAWER_MODULE:
            if (CommandDrawer.VcapBankItlkSts) Led2TurnOn();
            else if (CommandDrawer.VcapBankAlarmSts) Led2Toggle();
            else Led2TurnOff();

            if (CommandDrawer.VoutItlkSts) Led3TurnOn();
            else if (CommandDrawer.VoutAlarmSts) Led3Toggle();
            else Led3TurnOff();

            if (CommandDrawer.TempHeatSinkItlkSts) Led4TurnOn();
            else if (CommandDrawer.TempHeatSinkAlarmSts) Led4Toggle();
            else Led4TurnOff();

            if (CommandDrawer.TempLItlkSts) Led5TurnOn();
            else if (CommandDrawer.TempLAlarmSts) Led5TurnOff();
            else Led5TurnOff();

            if (CommandDrawer.ExtItlkSts) Led6TurnOn();
            else Led6TurnOff();

            if (CommandDrawer.ExtItlk2Sts) Led7TurnOn();
            else Led7TurnOff();

            break;
      }
      
}

void Application(void)
{

    switch(PowerModuleModel)
    {
        case OUTPUT_Q1_MODULE:

            q1_application_readings();

            break;
       
        case OUTPUT_Q4_MODULE:

            q4_application_readings();

            break;
            
        case RECTIFIER_MODULE:
            Rectifier.IoutRectf1 = CurrentCh1Read();
            Rectifier.IoutRectf1AlarmSts = CurrentCh1AlarmStatusRead();
            if(!Rectifier.IoutRectf1ItlkSts) Rectifier.IoutRectf1ItlkSts        = CurrentCh1TripStatusRead();
            
            Rectifier.IoutRectf2 = CurrentCh2Read();
            Rectifier.IoutRectf2AlarmSts = CurrentCh2AlarmStatusRead();
            if(!Rectifier.IoutRectf2ItlkSts) Rectifier.IoutRectf2ItlkSts        = CurrentCh2TripStatusRead();
            
            Rectifier.VoutRectf1 = VoltageCh1Read();
            Rectifier.VoutRectf1AlarmSts = VoltageCh1AlarmStatusRead();
            if(!Rectifier.VoutRectf1ItlkSts) Rectifier.VoutRectf1ItlkSts        = VoltageCh1TripStatusRead();
            
            Rectifier.VoutRectf2 = VoltageCh2Read();
            Rectifier.VoutRectf2AlarmSts = VoltageCh2AlarmStatusRead();
            if(!Rectifier.VoutRectf2ItlkSts) Rectifier.VoutRectf2ItlkSts        = VoltageCh2TripStatusRead();
            
            Rectifier.LeakageCurrent = CurrentCh3Read();
            Rectifier.LeakageCurrentAlarmSts = CurrentCh3AlarmStatusRead();
            if(!Rectifier.LeakageCurrentItlkSts) Rectifier.LeakageCurrentItlkSts = CurrentCh3TripStatusRead();
            
            Rectifier.TempHeatSink = Pt100ReadCh1();
            Rectifier.TempHeatSinkAlarmSts = Pt100ReadCh1AlarmSts();
            if(!Rectifier.TempHeatSinkItlkSts) Rectifier.TempHeatSinkItlkSts    = Pt100ReadCh1TripSts();
            
            Rectifier.TempWater = Pt100ReadCh4();
            Rectifier.TempWaterAlarmSts = Pt100ReadCh4AlarmSts();
            if(!Rectifier.TempWaterItlkSts) Rectifier.TempWaterItlkSts          = Pt100ReadCh4TripSts();
            
            Rectifier.TempModule1 = VoltageCh3Read();
            Rectifier.TempModule1AlarmSts = VoltageCh3AlarmStatusRead();
            if(!Rectifier.TempModule1ItlkSts) Rectifier.TempModule1ItlkSts      = VoltageCh3TripStatusRead();
            
            Rectifier.TempModule2 = VoltageCh4Read();
            Rectifier.TempModule2AlarmSts = VoltageCh4AlarmStatusRead();
            if(!Rectifier.TempModule2ItlkSts) Rectifier.TempModule2ItlkSts      = VoltageCh4TripStatusRead();
            
            Rectifier.TempL1 = Pt100ReadCh2();
            Rectifier.TempL1AlarmSts = Pt100ReadCh2AlarmSts();
            if(!Rectifier.TempL1ItlkSts) Rectifier.TempL1ItlkSts                = Pt100ReadCh2TripSts();
            
            Rectifier.TempL2 = Pt100ReadCh3();
            Rectifier.TempL2AlarmSts = Pt100ReadCh3AlarmSts();
            if(!Rectifier.TempL2ItlkSts) Rectifier.TempL2ItlkSts                = Pt100ReadCh3TripSts();
            
            Rectifier.AcPhaseFault = !Gpdi1Read();
            if(!Rectifier.AcPhaseFaultSts) Rectifier.AcPhaseFaultSts            = !Gpdi1Read();
            
            Rectifier.AcOverCurrent = !Gpdi2Read();
            if(!Rectifier.AcOverCurrentSts) Rectifier.AcOverCurrentSts          = !Gpdi2Read();
            
            Rectifier.AcTransformerOverTemp = !Gpdi3Read();
            if(!Rectifier.AcTransformerOverTempSts) Rectifier.AcTransformerOverTempSts = !Gpdi3Read();
            
            Rectifier.WaterFluxInterlock = !Gpdi4Read();
            if(!Rectifier.WaterFluxInterlockSts) Rectifier.WaterFluxInterlockSts = !Gpdi4Read();
            
            if(Rectifier.AcPhaseFault || Rectifier.AcOverCurrent || Rectifier.AcTransformerOverTemp || Rectifier.WaterFluxInterlock) InterlockSet();
            
            break;
            
       case INPUT_MODULE:
            InputModule.Iin = CurrentCh1Read();
            InputModule.IinAlarmSts = CurrentCh1AlarmStatusRead();
            if(!InputModule.IinItlkSts) InputModule.IinItlkSts                  = CurrentCh1TripStatusRead();
            
            InputModule.VdcLink = LvCurrentCh1Read();
            InputModule.VdcLinkAlarmSts = CurrentCh1AlarmStatusRead();
            if(!InputModule.VdcLinkItlkSts) InputModule.VdcLinkItlkSts          = CurrentCh1TripStatusRead();
            
            InputModule.TempHeatsink = Pt100ReadCh1();
            InputModule.TempHeatsinkAlarmSts = Pt100ReadCh1AlarmSts();
            if(!InputModule.TempHeatsinkItlkSts) InputModule.TempHeatsinkItlkSts = Pt100ReadCh1TripSts();

            InputModule.TempL = Pt100ReadCh2();
            InputModule.TempLAlarmSts = Pt100ReadCh2AlarmSts();
            if(!InputModule.TempLItlkSts) InputModule.TempLItlkSts              = Pt100ReadCh2TripSts();

            /*
            InputModule.Driver1Error = Driver1TopErrRead();
            if(!InputModule.Driver1ErrorItlk) InputModule.Driver1ErrorItlk = Driver1TopErrRead();

            InputModule.Driver2Error = Driver2TopErrRead();
            if(!InputModule.Driver2ErrorItlk) InputModule.Driver2ErrorItlk = Driver2TopErrRead();
            */
            break;

       case COMMAND_DRAWER_MODULE:
           CommandDrawer.TempHeatSink = Pt100ReadCh1();
           CommandDrawer.TempHeatSinkAlarmSts = Pt100ReadCh1AlarmSts();
           if (!CommandDrawer.TempHeatSinkItlkSts) CommandDrawer.TempHeatSinkItlkSts = Pt100ReadCh1TripSts();

           CommandDrawer.TempL = Pt100ReadCh2();
           CommandDrawer.TempLAlarmSts = Pt100ReadCh2AlarmSts();
           if (!CommandDrawer.TempLItlkSts) CommandDrawer.TempLItlkSts          = Pt100ReadCh2TripSts();

           CommandDrawer.VcapBank = VoltageCh1Read();
           CommandDrawer.VcapBankAlarmSts = VoltageCh1AlarmStatusRead();
           if (!CommandDrawer.VcapBankItlkSts) CommandDrawer.VcapBankItlkSts    = VoltageCh1TripStatusRead();

           CommandDrawer.Vout = VoltageCh2Read();
           CommandDrawer.VoutAlarmSts = VoltageCh2AlarmStatusRead();
           if (!CommandDrawer.VoutItlkSts) CommandDrawer.VoutItlkSts            = VoltageCh2TripStatusRead();

           if(!CommandDrawer.ExtItlkSts) CommandDrawer.ExtItlkSts               = Gpdi1Read();

           if(!CommandDrawer.ExtItlk2Sts) CommandDrawer.ExtItlk2Sts             = Gpdi5Read();

           break;
      }

      // Interlock Test
      if(Interlock == 1 && InterlockOld == 0)
      {
            InterlockOld = 1;
            AppInterlock();
      }

      // Actions that needs to be taken during the Application initialization
      if(InitApp == 0 && Interlock == 0)
      {
            InitApp = 1;

            switch(PowerModuleModel)
            {
             case OUTPUT_Q1_MODULE:
                  ReleAuxTurnOn();
                  ReleItlkTurnOn();
                  break;

             case OUTPUT_Q4_MODULE:
                  ReleAuxTurnOn();
                  ReleItlkTurnOn();

                  Gpdo1TurnOn();
                  Gpdo2TurnOn();

                  break;

             case RECTIFIER_MODULE:
                  ReleAuxTurnOn();
                  ReleItlkTurnOn();
                  break;

             case INPUT_MODULE:
                  ReleAuxTurnOn();
                  ReleItlkTurnOn();
                  break;
             case COMMAND_DRAWER_MODULE:
                 ReleAuxTurnOn();
                 ReleItlkTurnOn();
                 break;
            }
      }

      InterlockClearCheck();
      CheckCan();

}

// Application type
//******************************************************************************
unsigned char AppType(void)
{
    return PowerModuleModel;
}




// Rectifier
//******************************************************************************
float RectifierIoutRectf1Read(void)
{
    return Rectifier.IoutRectf1;
}
            
unsigned char RectifierIoutRectf1AlarmStsRead(void)
{
    return Rectifier.IoutRectf1AlarmSts;
}

unsigned char RectifierIoutRectf1ItlkStsRead(void)
{
    return Rectifier.IoutRectf1ItlkSts;
}

//******************************************************************************
float RectifierIoutRectf2Read(void)
{
    return Rectifier.IoutRectf2;
}

unsigned char RectifierIoutRectf2AlarmStsRead(void)
{
    return Rectifier.IoutRectf2AlarmSts;
}

unsigned char RectifierIoutRectf2ItlkStsRead(void)
{
    return Rectifier.IoutRectf2ItlkSts;
}

//******************************************************************************
float RectifierVoutRectf1Read(void)
{
    return Rectifier.VoutRectf1;
}

unsigned char RectifierVoutRectf1AlarmStsRead(void)
{
    return Rectifier.VoutRectf1AlarmSts;
}

unsigned char RectifierVoutRectf1ItlkStsRead(void)
{
    return Rectifier.VoutRectf1ItlkSts;
}

//******************************************************************************
float RectifierVoutRectf2Read(void)
{
    return Rectifier.VoutRectf2;
}

unsigned char RectifierVoutRectf2AlarmStsRead(void)
{
    return Rectifier.VoutRectf2AlarmSts;
}

unsigned char RectifierVoutRectf2ItlkStsRead(void)
{
    return Rectifier.VoutRectf2ItlkSts;
}

//******************************************************************************
float RectifierLeakageCurrentRead(void)
{
    return Rectifier.LeakageCurrent;
}

unsigned char RectifierLeakageCurrentAlarmStsread(void)
{
    return Rectifier.LeakageCurrentAlarmSts;
}

unsigned char RectifierLeakageCurrentItlkStsRead(void)
{
    return Rectifier.LeakageCurrentItlkSts;
}

//******************************************************************************
unsigned char RectifierTempHeatSinkRead(void)
{
    return Rectifier.TempHeatSink;
}

unsigned char RectifierTempHeatSinkAlarmStsRead(void)
{
    return Rectifier.TempHeatSinkAlarmSts;
}

unsigned char RectifierTempHeatSinkItlkStsRead(void)
{
    return Rectifier.TempHeatSinkItlkSts;
}

//******************************************************************************
unsigned char RectifierTempWaterRead(void)
{
    return Rectifier.TempWater;
}

unsigned char RectifierTempWaterAlarmStsRead(void)
{
    return Rectifier.TempWaterAlarmSts;
}

unsigned char RectifierTempWaterItlkStsRead(void)
{
    return Rectifier.TempWaterItlkSts;
}

//******************************************************************************
unsigned char RectifierTempModule1Read(void)
{
    return Rectifier.TempModule1;
}

unsigned char RectifierTempModule1AlarmStsRead(void)
{
    return Rectifier.TempModule1AlarmSts;
}

unsigned char RectifierTempModule1ItlkStsRead(void)
{
    return Rectifier.TempModule1ItlkSts;
}

//******************************************************************************
unsigned char RectifierTempModule2Read(void)
{
    return Rectifier.TempModule2;
}

unsigned char RectifierTempModule2AlarmStsRead(void)
{
    return Rectifier.TempModule2AlarmSts;
}

unsigned char RectifierTempModule2ItlkStsRead(void)
{
    return Rectifier.TempModule2ItlkSts;
}

//******************************************************************************
unsigned char RectifierTempL1Read(void)
{
    return Rectifier.TempL1;
}

unsigned char RectifierTempL1AlarmStsRead(void)
{
    return Rectifier.TempL1AlarmSts;
}

unsigned char RectifierTempL1ItlkStsRead(void)
{
    return Rectifier.TempL1ItlkSts;
}

//******************************************************************************
unsigned char RectifierTempL2Read(void)
{
    return Rectifier.TempL2;
}

unsigned char RectifierTempL2AlarmStsRead(void)
{
    return Rectifier.TempL2AlarmSts;
}

unsigned char RectifierTempL2ItlkStsRead(void)
{
    return Rectifier.TempL2ItlkSts;
}

//******************************************************************************
unsigned char RectifierAcPhaseFaultRead(void)
{
    return Rectifier.AcPhaseFault;
}

unsigned char RectifierAcPhaseFaultStsRead(void)
{
    return Rectifier.AcPhaseFaultSts;
}

//******************************************************************************
unsigned char RectifierAcOverCurrentRead(void)
{
    return Rectifier.AcOverCurrent;
}

unsigned char RectifierAcOverCurrentStsRead(void)
{
    return Rectifier.AcOverCurrentSts;
}

//******************************************************************************
unsigned char RectifierAcTransformerOverTempRead(void)
{
    return Rectifier.AcTransformerOverTemp;
}

unsigned char RectifierAcTransformerOverTempStsRead(void)
{
    return Rectifier.AcTransformerOverTempSts;
}

//******************************************************************************
unsigned char RectifierWaterFluxInterlockRead(void)
{
    return Rectifier.WaterFluxInterlock;
}

unsigned char RectifierWaterFluxInterlockStsRead(void)
{
    return Rectifier.WaterFluxInterlockSts;
}

// Input Module
//******************************************************************************
float InputModuleIinRead(void)
{
    return InputModule.Iin;
}

unsigned char InputModuleIinAlarmStsRead(void)
{
    return InputModule.IinAlarmSts;
}

unsigned char InputModuleIinItlkStsRead(void)
{
    return InputModule.IinItlkSts;
}

//******************************************************************************
float InputModuleVdcLinkRead(void)
{
    return InputModule.VdcLink;
}

unsigned char InputModuleVdcLinkAlarmStsRead(void)
{
    return InputModule.VdcLinkAlarmSts;
}

unsigned char InputModuleVdcLinkItlkStsRead(void)
{
    return InputModule.VdcLinkItlkSts;
}

//******************************************************************************
unsigned char InputModuleTempHeatsinkRead(void)
{
    return InputModule.TempHeatsink;
}

unsigned char InputModuleTempHeatsinkAlarmStsRead(void)
{
    return InputModule.TempHeatsinkAlarmSts;
}

unsigned char InputModuleTempHeatsinkItlkStsRead(void)
{
    return InputModule.TempHeatsinkItlkSts;
}

//******************************************************************************
unsigned char InputModuleTempLRead(void)
{
    return InputModule.TempL;
}

unsigned char InputModuleTempLAlarmStsRead(void)
{
    return InputModule.TempLAlarmSts;
}

unsigned char InputModuleTempLItlkStsRead(void)
{
    return InputModule.TempLItlkSts;
}

//******************************************************************************
/* Command Drawer */
unsigned char CommandDrawerTempHeatSinkRead(void)
{
    return CommandDrawer.TempHeatSink;
}

unsigned char CommandDrawerTempHeatSinkAlarmStsRead(void)
{
    return CommandDrawer.TempHeatSinkAlarmSts;
}

unsigned char CommandDrawerTempHeatSinkItlkStsRead(void)
{
    return CommandDrawer.TempHeatSinkItlkSts;
}

//******************************************************************************
unsigned char CommandDrawerTempLRead(void)
{
    return CommandDrawer.TempL;
}

unsigned char CommandDrawerTempLAlarmStsRead(void)
{
    return CommandDrawer.TempLAlarmSts;
}

unsigned char CommandDrawerTempLItlkStsRead(void)
{
    return CommandDrawer.TempLItlkSts;
}

//******************************************************************************

float CommandDrawerVcapBankRead(void)
{
    return CommandDrawer.VcapBank;
}

unsigned char CommandDrawerVcapBankAlarmStsRead(void)
{
    return CommandDrawer.VcapBankAlarmSts;
}

unsigned char CommandDrawerVcapBankItlkStsRead(void)
{
    return CommandDrawer.VcapBankItlkSts;
}

//******************************************************************************
float CommandDrawerVoutRead(void)
{
    return CommandDrawer.Vout;
}

unsigned char CommandDrawerVoutAlarmStsRead(void)
{
    return CommandDrawer.VoutAlarmSts;
}

unsigned char CommandDrawerVoutItlkStsRead(void)
{
    return CommandDrawer.VoutItlkSts;
}

unsigned char CommandDrawerExtItlkStsRead()
{
    return CommandDrawer.ExtItlkSts;
}

unsigned char CommandDrawerExt2ItlkStsRead()
{
    return CommandDrawer.ExtItlk2Sts;
}
