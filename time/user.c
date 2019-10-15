#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <sys/syscall.h>

#include "user.h"

int user_init()
{
	int ret = 0;
	
	ret = timer_init();
	if (ret < 0) {
		printf("Init user timer faild!\n");
	}

	return 0;
}

int user_add(int run_time	/* -1:循环运行; N(N>0):运行N次后停止 */
                , int interval	/* ms */
                , char* cb_func_name
                , timer_cb func
                , void* param)
{
	int handle = timer_add(run_time, interval, cb_func_name, func, param);
	if (handle < 0) {
		printf("Add timer faild!\n");
	}

	printf("Add user handle: %d\n", handle);
	
	return handle;
}


int user_del(int* handle)
{
	int ret = 0;
	timer_set_t* p_timer = NULL;
	timer_set_t* p_next  = NULL;

	printf("Delete user handle: %d\n", *handle);

	if (-1 == *handle) {
		// 如果handle等于-1，则不对handle进行校验
	} else {
		list_for_each_entry_safe(p_timer, p_next, &g_timer.timer_list.list, list) {
			if (*handle == p_timer->handle) {
				ret = _timer_stop(p_timer);
				break;
			}
		}

		if (TIMER_LIST_HEAD_ADDR == p_timer) {
			printf("_______Find error!\n");
			return -1;
		}
	}

	return ret;
}

int user_start(int* handle)
{
	int ret = 0;
	timer_set_t* p_timer = NULL;
	timer_set_t* p_next  = NULL;

	printf("Start user handle: %d\n", *handle);

	if (-1 == *handle) {
		// 如果handle等于-1，则不对handle进行校验
	} else {
		list_for_each_entry_safe(p_timer, p_next, &g_timer.timer_list.list, list) {
			if (*handle == p_timer->handle) {
				ret = _timer_start(p_timer);
				break;
			}
		}

		if (TIMER_LIST_HEAD_ADDR == p_timer) {
			printf("_______Find error!\n");
			return -1;
		}
	}

	return ret;
}

int user_stop(int* handle)
{
	int ret = 0;
	timer_set_t* p_timer = NULL;
	timer_set_t* p_next  = NULL;

	printf("Stop user handle: %d\n", *handle);

	if (-1 == *handle) {
		// 如果handle等于-1，则不对handle进行校验
	} else {
		list_for_each_entry_safe(p_timer, p_next, &g_timer.timer_list.list, list) {
			if (*handle == p_timer->handle) {
				ret = timer_stop(p_timer->handle);
				break;
			}
		}

		if (TIMER_LIST_HEAD_ADDR == p_timer) {
			printf("_______Find error!\n");
			return -1;
		}
	}

	return ret;
}


int user_ch_time(int* handle, int* interval)
{
	timer_set_t* p_timer = NULL;
	timer_set_t* p_next  = NULL;

	printf("Change user handle: %d, interval: %d\n", *handle, *interval);
	
	if (-1 == *handle) {
		// 如果handle等于-1，则不对handle进行校验
	} else {
		list_for_each_entry_safe(p_timer, p_next, &g_timer.timer_list.list, list) {
			if (*handle == p_timer->handle) {
				p_timer->interval = *interval;
				break;
			}
		}

		if (TIMER_LIST_HEAD_ADDR == p_timer) {
			printf("_______Find error!\n");
			return -1;
		}
	}

	return 0;
}

