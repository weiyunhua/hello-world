#ifndef __TIMER_H__
#define __TIMRE_H__

#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/time.h>

#include "list.h"
#include "thread.h"

typedef int (*timer_cb)(void* param);

#define		EVENT_PROCESS_NUM			1
#define		BASE_HANDEL					100
#define		TIMER_LIST_HEAD_ADDR 		(timer_set_t *)(&g_timer.timer_list.list)


// ���º궨��Ϊtimer_ioctrl()��cmd����
typedef enum
{
	TIMER_CMD_GETTMCOUNT = 0x00         // ��ȡ��ǰ�����ļ���������
	,   TIMER_CMD_GETTMPASST            // ��ȡ��ǰ��ʱ���Ѿ�������ʱ��(ms)
	,   TIMER_CMD_GETTMREACH            // ��ȡ��ǰ��ʱ���뵽���ʱ��(ms)
	,   TIMER_CMD_RESET_STARTTIME       // �������õ�ǰ��ʱ���Ŀ�ʼʱ��(ms)
	,   TIMER_CMD_BUIT
} TIMER_CMD;


typedef struct timer_set_s
{
	struct list_head list;
	int handle;
	int interval;
	int run_time;					// -1:ѭ������; N(N>0):����N�κ�ֹͣ
	timer_cb func;					// �ص�����
	void* cb_param;					// �ص���������
	unsigned int start_time;		// ms
	unsigned int end_time;			// ms
	unsigned int cb_start_time;		// ms
	unsigned int cb_end_time;		// ms
	char cb_func_name[64];
} timer_set_t;

typedef struct simple_timer_s
{
	timer_set_t timer_list;
	unsigned int timer_count;
	int base_handle;
	int timer_running;
	int curr_task_hdl;
	int timer_living;
	pthread_t loop_tid[EVENT_PROCESS_NUM];
	int monitor_running;
	pthread_mutex_t timer_lock;
} simple_timer_t;


simple_timer_t g_timer;


/*
 * �������ܣ���ʼ����ʱ��ģ��(����ģ���ʼ��һ�μ���)
 * �����������
 * �����������
 * ����ֵ  ��>0-�ɹ�; <0-ʧ��
 */
int timer_init();


/*
 * �������ܣ����һ����ʱ��
 * ���������once:     -1:ѭ����ʱ��N(N>0):����N�κ�ֹͣ
             interval: ��ʱ��ʱ��������λ������
             func:     ��ʱ���ص�����
             param:�ص���������
 * �����������
 * ����ֵ  ��>0-�ɹ�����ʾ��ʱ�������<=0-�������
 */
int timer_add(int run_time, int interval, char* cb_func_name, timer_cb func,
                void* param);

/*
 * �������ܣ���ʼһ����ʱ��
 * ���������handle: ��ʱ�����
 * �����������
 * ����ֵ  ��>=0-�ɹ���<0-ʧ��
 */
int timer_start(timer_set_t* p_timer);



/*
 * �������ܣ�ֹͣһ����ʱ��
 * ���������handle: ��ʱ�����
 * �����������
 * ����ֵ  ��>=0-�ɹ���<0-ʧ��
 */
int timer_stop(int handle);


/*
 * �������ܣ�ɾ��һ����ʱ��
 * ���������handle: ��ʱ�����
 * �����������
 * ����ֵ  ��>=0-�ɹ���<0-ʧ��
 */
int timer_del(timer_set_t* p_timer);


#endif // __TIMER_H__



