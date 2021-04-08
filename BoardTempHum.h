
/////////////////////////////////////////////////////////////////////////////////////////////

#ifndef __BOARDTEMPHUM_H__
#define __BOARDTEMPHUM_H__

/////////////////////////////////////////////////////////////////////////////////////////////

typedef struct
{
    float Value;
    float AlarmLimit;
    float TripLimit;
    unsigned char Alarm;
    unsigned char Trip;
    unsigned int  Alarm_Delay_ms; // milisecond
    unsigned int  Alarm_DelayCount;
    unsigned int  Itlk_Delay_ms; // milisecond
    unsigned int  Itlk_DelayCount;
}rh_tempboard_t;

/////////////////////////////////////////////////////////////////////////////////////////////

extern rh_tempboard_t TemperatureBoard;
extern rh_tempboard_t RelativeHumidity;

/////////////////////////////////////////////////////////////////////////////////////////////

extern void RhBoardTempSenseInit(void);
extern float RhRead(void);
extern float BoardTempRead(void);

/////////////////////////////////////////////////////////////////////////////////////////////

extern void BoardTemperatureStartConversion(void);
extern void BoardTemperatureRead(void);
extern void RelativeHumidityStartConversion(void);
extern void RelativeHumidityRead(void);

/////////////////////////////////////////////////////////////////////////////////////////////

extern void BoardTempAlarmLevelSet(float nValue);
extern void BoardTempTripLevelSet(float nValue);

/////////////////////////////////////////////////////////////////////////////////////////////

extern void BoardTempDelay(unsigned int delay_ms);

/////////////////////////////////////////////////////////////////////////////////////////////

extern unsigned char BoardTempAlarmStatusRead(void);
extern unsigned char BoardTempTripStatusRead(void);

/////////////////////////////////////////////////////////////////////////////////////////////

extern void RhAlarmLevelSet(float nValue);
extern void RhTripLevelSet(float nValue);

/////////////////////////////////////////////////////////////////////////////////////////////

extern void RhDelay(unsigned int delay_ms);

/////////////////////////////////////////////////////////////////////////////////////////////

extern unsigned char RhAlarmStatusRead(void);
extern unsigned char RhTripStatusRead(void);

/////////////////////////////////////////////////////////////////////////////////////////////

extern void RhBoardTempClearAlarmTrip(void);

/////////////////////////////////////////////////////////////////////////////////////////////

#endif

/////////////////////////////////////////////////////////////////////////////////////////////


