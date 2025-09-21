#ifndef __PID_MANAGER_H
#define __PID_MANAGER_H

#include "main.h"
#include "pid.h"
#include "FM24CXX.h"
#include "tps02r.h"
#include <string.h>

/* ---------------------- �궨�� ---------------------- */
#define PID_MAGIC_WORD      0x5AA55AA5      /* EEPROMħ���� */
#define PID_VERSION         0x01            /* �����汾�� */
#define EEPROM_BASE_ADDR    0x0000          /* EEPROM����ַ */

/* �¶ȷ���߽綨�� */
#define TEMP_GROUP0_MAX     140.0f          /* ��140��ʹ����0 */
#define TEMP_GROUP1_MAX     180.0f          /* 141-180��ʹ����1 */

/* ���������� */
#define PARAM_GROUP_120     0               /* 120������� */
#define PARAM_GROUP_160     1               /* 160������� */  
#define PARAM_GROUP_220     2               /* 220������� */
#define PARAM_GROUP_MAX     3               /* ������� */

/* ϵͳ����ģʽ */
#define PID_MODE_STOP       0                   /* ֹͣģʽ */
#define PID_MODE_RUN        1                   /* ����ģʽ */
#define PID_MODE_AUTOTUNE   2               /* ������ģʽ */

/* ��������ɱ�־ */
#define AUTOTUNE_COMPLETE   5               /* ��Խ5����� */

/* ---------------------- ���ݽṹ���� ---------------------- */

/* PID������ṹ */
typedef struct 
{
    float front_Kp;                        /* ǰǹ�ܱ���ϵ�� */
    float front_Ti;                        /* ǰǹ�ܻ���ʱ�� */
    float front_Td;                        /* ǰǹ��΢��ʱ�� */
    float rear_Kp;                         /* ǻ�����ϵ�� */
    float rear_Ti;                         /* ǻ�����ʱ�� */
    float rear_Td;                         /* ǻ��΢��ʱ�� */
    uint8_t valid;                         /* ��Ч��־ */
    uint8_t reserved[3];                   /* ���뱣�� */
} PID_Param_Group_t;

/* EEPROM�洢�ṹ */
typedef struct 
{
    uint32_t magic;                        /* ħ���� */
    uint8_t version;                       /* �汾�� */
    uint8_t reserved[3];                   /* �����ֽ� */
    
    PID_Param_Group_t group[PARAM_GROUP_MAX]; /* ���������� */
    
    uint16_t checksum;                     /* У��� */
} PID_Storage_t;

/* ϵͳ״̬�ṹ�� */
typedef struct 
{
    /* �¶����� */
    float front_temp_pv;                   /* ǰǹ�ܵ�ǰ�¶� */
    float front_temp_sv;                   /* ǰǹ���趨�¶� */
    float rear_temp_pv;                    /* ǻ�嵱ǰ�¶� */  
    float rear_temp_sv;                    /* ǻ���趨�¶� */
    
    /* ����״̬ */
    uint8_t mode;                          /* ����ģʽ */
    uint8_t autotune_request;              /* �����������־ */
    uint8_t param_group;                   /* ��ǰ������ */
    uint8_t param_loaded;                  /* �������ر�־ */
    
    /* ���������� */
    uint8_t front_cross_cnt;              /* ǰǹ�ܴ�Խ���� */
    uint8_t rear_cross_cnt;               /* ǻ�崩Խ���� */
    uint8_t autotune_complete;            /* ��������ɱ�־ */
    
    /* ����״̬ */
    uint8_t error_code;                   /* ������� */
    uint8_t temp_error;                   /* �¶ȴ����־ */
} System_Status_t;

/* ---------------------- ȫ�ֱ������� ---------------------- */
extern System_Status_t g_system_status;   /* ȫ��ϵͳ״̬ */
extern PID_Storage_t g_pid_storage;       /* PID�洢�ṹ */

/* ---------------------- �������� ---------------------- */

/* ��������� */
uint8_t Get_Param_Group(float temp_sv);
void Check_And_Switch_Group(void);
bool Load_Parameters_From_Group(uint8_t group);
void Save_Parameters_To_Group(uint8_t group);

/* EEPROM���� */
bool Load_All_Parameters(void);
void Save_All_Parameters(void);
bool Verify_Parameters(PID_Storage_t *storage);
uint16_t Calculate_Checksum(PID_Storage_t *storage);

/* ������ʼ�� */
void Init_PID_Manager(void);
void Set_Default_Parameters(uint8_t group);
void Apply_Parameters_To_PID(uint8_t group);

/* ���������� */
void Start_Autotune(void);
void Stop_Autotune(void);
void Check_Autotune_Complete(void);
void Save_Autotune_Results(void);

/* ״̬���� */
void Update_System_Status(void);
void Clear_Error_Status(void);

#endif /* __PID_MANAGER_H */

