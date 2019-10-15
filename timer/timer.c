#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/syscall.h>

#include "timer.h"

#define TIMER_LIST_ADD(p_timer, g_timer) \
    do{ \
        list_add_tail(&p_timer->list, &g_timer.timer_list.list); \
    } while(0)

#define TIMER_LIST_DEL(p_timer, g_timer) \
	do{ \
		g_timer.timer_count--; \
		p_timer->handle = 0; \
		list_del(&p_timer->list); \
 		free(p_timer); \
	} while(0)

static int _loop_event_process();

int timer_init()
{
	int i = 0;
	memset(&g_timer, 0, sizeof(g_timer));
	pthread_mutex_init(&g_timer.timer_lock, NULL);
	INIT_LIST_HEAD(&(g_timer.timer_list.list));
	g_timer.base_handle = BASE_HANDEL;
	g_timer.timer_running = 1;

	for (i = 0; i < EVENT_PROCESS_NUM; i++) {
		if (pthread_create(&g_timer.loop_tid[i], "timer-loop", NULL,
		                   (void*)_loop_event_process, NULL) < 0) {
			printf("thread_create error!\n");
			return -1;
		}
	}

	return 0;
}

static int _loop_event_process()
{
	unsigned int curr_time_ms = 0;
	struct timespec ts = {0, 0};

	while (g_timer.timer_running) {
		memset(&ts, 0, sizeof(ts));
		clock_gettime(CLOCK_MONOTONIC, &ts);
		curr_time_ms = ts.tv_sec * 1000 + ts.tv_nsec / (1000 * 1000);
		pthread_mutex_lock(&g_timer.timer_lock);
		timer_set_t* p_timer = NULL;
		timer_set_t* p_next  = NULL;
		list_for_each_entry_safe(p_timer, p_next, &g_timer.timer_list.list, list) {
			if (p_timer->run_time > 0) {
				/* 运行N次(N>0)的定时器 */
				if (curr_time_ms >= p_timer->end_time) {
					p_timer->run_time--;
					g_timer.curr_task_hdl = p_timer->handle;
					/* 更新开始时间和结束时间 */
					p_timer->start_time = curr_time_ms;
					p_timer->end_time = p_timer->start_time + p_timer->interval;
					printf("p_timer->start_time:%d, p_timer->interval:%d, p_timer->end_time:%d\n", p_timer->start_time, p_timer->interval, p_timer->end_time);
					memset(&ts, 0, sizeof(ts));
					clock_gettime(CLOCK_MONOTONIC, &ts);
					p_timer->cb_start_time = ts.tv_sec * 1000 + ts.tv_nsec / (1000 * 1000);

					if (p_timer->func != NULL) {
						p_timer->func(p_timer->cb_param);
					}

					memset(&ts, 0, sizeof(ts));
					clock_gettime(CLOCK_MONOTONIC, &ts);
					p_timer->cb_end_time = ts.tv_sec * 1000 + ts.tv_nsec / (1000 * 1000);

					if ((p_timer->cb_end_time - p_timer->cb_start_time) > 1) {
						printf("\033[0;32m""======111====== Tid %ld Callback task %s use %u ms""\033[0m\n",
						       syscall(SYS_gettid), p_timer->cb_func_name,
						       (p_timer->cb_end_time - p_timer->cb_start_time));
					}

					if (0 == p_timer->run_time) {
						//_timer_stop(p_timer);						
					}
				}
			} else if(p_timer->run_time == -1) {
				/* 循环运行 */
				if ((curr_time_ms - p_timer->start_time) >= p_timer->interval) {
					/* 更新开始时间和结束时间 */
					p_timer->start_time = curr_time_ms;
					p_timer->end_time = p_timer->start_time + p_timer->interval;
					g_timer.curr_task_hdl = p_timer->handle;
					memset(&ts, 0, sizeof(ts));
					clock_gettime(CLOCK_MONOTONIC, &ts);
					p_timer->cb_start_time = ts.tv_sec * 1000 + ts.tv_nsec / (1000 * 1000);

					if (p_timer->func != NULL) {
						p_timer->func(p_timer->cb_param);
					}

					memset(&ts, 0, sizeof(ts));
					clock_gettime(CLOCK_MONOTONIC, &ts);
					p_timer->cb_end_time = ts.tv_sec * 1000 + ts.tv_nsec / (1000 * 1000);

					if ((p_timer->cb_end_time - p_timer->cb_start_time) > 1) {
						printf("\033[0;32m""======222====== Tid %ld Callback task %s use %u ms""\033[0m\n",
						       syscall(SYS_gettid), p_timer->cb_func_name,
						       (p_timer->cb_end_time - p_timer->cb_start_time));
					}
				}
			} else if (p_timer->run_time == 0) {
				/* 周期循环运行后，结束定时器 */
				g_timer.timer_running = 0;
			}
		}
		pthread_mutex_unlock(&g_timer.timer_lock);
		usleep(1 * 1000);	/* 1ms */
		g_timer.timer_living = 0;
	}

	return 0;
}

int timer_add(int run_time	/* -1:循环运行; N(N>0):运行N次后停止 */
                , int interval	/* ms */
                , char* cb_func_name
                , timer_cb func
                , void* param)
{
	unsigned int curr_time_ms;
	struct timespec ts = {0, 0};

	if (g_timer.timer_running) {
		g_timer.timer_running = 0;
	}

	timer_set_t* p_timer = (timer_set_t*)malloc(sizeof(timer_set_t));

	if (NULL == p_timer) {
		printf("malloc error!\n");
		return -1;
	}

	memset(&ts, 0, sizeof(ts));
	clock_gettime(CLOCK_MONOTONIC, &ts);
	curr_time_ms = ts.tv_sec * 1000 + ts.tv_nsec / (1000 * 1000);
	pthread_mutex_lock(&g_timer.timer_lock);
	p_timer->handle = g_timer.base_handle++;
	g_timer.timer_count++;
	p_timer->cb_param = param;
	p_timer->start_time = curr_time_ms;
	p_timer->interval = interval;
	p_timer->end_time = p_timer->start_time + p_timer->interval;
	p_timer->func = func;
	p_timer->run_time = run_time;
	memset(p_timer->cb_func_name, 0, sizeof(p_timer->cb_func_name));
	strncpy(p_timer->cb_func_name
	        , cb_func_name
	        , strlen(cb_func_name) > sizeof(p_timer->cb_func_name) ? 
	          sizeof(p_timer->cb_func_name) : strlen(cb_func_name));
	TIMER_LIST_ADD(p_timer, g_timer);
	pthread_mutex_unlock(&g_timer.timer_lock);
	return p_timer->handle;
}

int timer_start()
{
	int ret = 0;
	if (!g_timer.timer_running) {
		g_timer.timer_running = 1;
	}

	ret = _loop_event_process();

	return ret;
}

int timer_del(timer_set_t* p_timer)
{
	TIMER_LIST_DEL(p_timer, g_timer);
	return 0;
}

void timer_stop()
{
	if (g_timer.timer_running) {
		g_timer.timer_running = 0;
	}
}

int timer_stop_all(int handle)
{
	pthread_mutex_lock(&g_timer.timer_lock);
	timer_set_t* p_timer = NULL;
	timer_set_t* p_next  = NULL;
	list_for_each_entry_safe(p_timer, p_next, &g_timer.timer_list.list, list) {
		if (handle == p_timer->handle) {
			TIMER_LIST_DEL(p_timer, g_timer);
			pthread_mutex_unlock(&g_timer.timer_lock);
			return 0;
		}
	}
	pthread_mutex_unlock(&g_timer.timer_lock);

	if (TIMER_LIST_HEAD_ADDR == p_timer) {
		printf("Handle error!\n");
	}

	return -1;
}

