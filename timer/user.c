#include <stdio.h>

#include "user.h"

static int check_head_addr(timer_set_t* p_timer)
{
	if (TIMER_LIST_HEAD_ADDR == p_timer) {
		printf("__Find error!\n");
		return -1;
	}

	return 0;
}

int user_init()
{
	int ret = 0;
	
	ret = timer_init();
	if (ret < 0) {
		printf("User init timer faild!\n");
	}

	printf("User init sucess!\n");

	return ret;
}

int user_add(int run_time	/* -1: 循环运行; N(N>0):运行N次后停止 */
                , int interval	/* ms */
                , char* cb_func_name
                , timer_cb func
                , void* param)
{
	int handle = timer_add(run_time, interval, cb_func_name, func, param);
	if (handle < 0) {
		printf("Add timer faild!\n");
	}

	printf("User add handle: %d\n", handle);
	
	return handle;
}


int user_del(int* handle)
{
	printf("User delete handle: %d\n", *handle);

	int ret = 0;
	timer_set_t* p_timer = NULL;
	timer_set_t* p_next  = NULL;

	if (-1 != *handle) {
		list_for_each_entry_safe(p_timer, p_next, &g_timer.timer_list.list, list) {
			if (*handle == p_timer->handle) {
				ret = timer_del(p_timer);
				break;
			}
		}

		ret = check_head_addr(p_timer);
	}

	return ret;
}

int user_start(int* handle)
{
	printf("User start handle: %d\n", *handle);

	int ret = 0;
	timer_set_t* p_timer = NULL;
	timer_set_t* p_next  = NULL;

	if (-1 != *handle) {
		list_for_each_entry_safe(p_timer, p_next, &g_timer.timer_list.list, list) {
			if (*handle == p_timer->handle) {
				ret = timer_start();
				break;
			}
		}

		ret = check_head_addr(p_timer);
	}

	return ret;
}

int user_stop(int* handle)
{
	printf("User stop handle: %d\n", *handle);

	int ret = 0;
	timer_set_t* p_timer = NULL;
	timer_set_t* p_next  = NULL;

	if (-1 != *handle) {
		list_for_each_entry_safe(p_timer, p_next, &g_timer.timer_list.list, list) {
			if (*handle == p_timer->handle) {
				timer_stop();
				break;
			}
		}

		ret = check_head_addr(p_timer);
	}

	return ret;
}


int user_ch_time(int* handle, int* interval)
{
	printf("User change handle: %d, interval: %d\n", *handle, *interval);

	int ret = 0;
	timer_set_t* p_timer = NULL;
	timer_set_t* p_next  = NULL;

	if (-1 != *handle) {
		list_for_each_entry_safe(p_timer, p_next, &g_timer.timer_list.list, list) {
			if (*handle == p_timer->handle) {
				p_timer->interval = *interval;
				break;
			}
		}

		ret = check_head_addr(p_timer);
	}

	return ret;
}

