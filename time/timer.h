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


// 以下宏定义为timer_ioctrl()的cmd类型
typedef enum
{
	TIMER_CMD_GETTMCOUNT = 0x00         // 获取当前启动的计数器个数
	,   TIMER_CMD_GETTMPASST            // 获取当前定时器已经经过的时间(ms)
	,   TIMER_CMD_GETTMREACH            // 获取当前定时器离到达的时间(ms)
	,   TIMER_CMD_RESET_STARTTIME       // 重新设置当前定时器的开始时间(ms)
	,   TIMER_CMD_BUIT
} TIMER_CMD;


typedef struct timer_set_s
{
	struct list_head list;
	int handle;
	int interval;
	int run_time;					// -1:循环运行; N(N>0):运行N次后停止
	timer_cb func;					// 回调函数
	void* cb_param;					// 回调函数参数
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
 * 函数介绍：初始化定时器模块(主控模块初始化一次即可)
 * 输入参数：无
 * 输出参数：无
 * 返回值  ：>0-成功; <0-失败
 */
int timer_init();


/*
 * 函数介绍：添加一个定时器
 * 输入参数：once:     -1:循环定时，N(N>0):运行N次后停止
             interval: 定时器时间间隔，单位：毫秒
             func:     定时器回调函数
             param:回调函数参数
 * 输出参数：无
 * 返回值  ：>0-成功，表示定时器句柄；<=0-错误代码
 */
int timer_add(int run_time, int interval, char* cb_func_name, timer_cb func,
                void* param);

/*
 * 函数介绍：开始一个定时器
 * 输入参数：handle: 定时器句柄
 * 输出参数：无
 * 返回值  ：>=0-成功，<0-失败
 */
int timer_start(timer_set_t* p_timer);



/*
 * 函数介绍：停止一个定时器
 * 输入参数：handle: 定时器句柄
 * 输出参数：无
 * 返回值  ：>=0-成功，<0-失败
 */
int timer_stop(int handle);


/*
 * 函数介绍：删除一个定时器
 * 输入参数：handle: 定时器句柄
 * 输出参数：无
 * 返回值  ：>=0-成功，<0-失败
 */
int timer_del(timer_set_t* p_timer);


#endif // __TIMER_H__



