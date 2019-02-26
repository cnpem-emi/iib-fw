#ifndef __APPLICATION_H__
#define __APPLICATION_H__

#define  FAP_MODULE                 0
#define  OUTPUT_Q4_MODULE           1
#define  RECTIFIER_MODULE           2
#define  INPUT_MODULE               3
#define  FAC_CMD_MODULE             4
#define  OUTPUT_FAP_300A_MODULE     5

void LedIndicationStatus(void);
void AppConfiguration(void);

void InterlockClear(void);
void InterlockSet(void);
void InterlockClearCheck(void);
unsigned char InterlockRead(void);
void AppInterlock(void);

void AlarmSet(void);
void AlarmClear(void);
unsigned char AlarmRead(void);
void AppAlarm(void);

void InterlockAppCheck(void);
void AlarmAppCheck(void);

void Application(void);
unsigned char AppType(void);

extern void power_on_check();

void ClearDiagnosticCount(void);
void PrintDiagnosticVar(void);

extern void send_data_schedule();


#endif
