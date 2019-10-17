#include <stdio.h>

#include "user.h"


void cbfunc()
{
	printf("cb func!\n");
}

int test_del_timer()
{
	int ret = 0;

	int TimerID = user_add(1, 2 * 1000, "timer_1", (timer_cb)cbfunc, NULL);

	ret = user_del(&TimerID);
	if (ret < 0) {
		printf("user timer delete failed!\n");
		return -1;
	}

	return ret;
}

int test_stop_timer()
{
	int ret = 0;

	int TimerID = user_add(1, 2 * 1000, "timer_2", (timer_cb)cbfunc, NULL);

	ret = user_stop(&TimerID);
	if (ret < 0) {
		printf("user timer start failed!\n");
		return -1;
	}
	
	return ret;
}

int test_cht_loop_timer()
{
	int ret = 0;
	int interval = 5 * 1000;

	int TimerID = user_add(2, 2 * 1000, "timer_3", (timer_cb)cbfunc, NULL);

	ret = user_ch_time(&TimerID, &interval);
	if (ret < 0) {
		printf("user timer change interval failed!\n");
		return -1;
	}
	
	ret = user_start(&TimerID);
	if (ret < 0) {
		printf("user timer stop failed!\n");
		return -1;
	}

	return ret;
}


int test_single_timer()
{
	int ret = 0;

	int TimerID = user_add(1, 2 * 1000, "timer_4", (timer_cb)cbfunc, NULL);

	ret = user_start(&TimerID);
	if (ret < 0) {
		printf("user timer stop failed!\n");
		return -1;
	}

	return ret;
}


int func_test()
{
	int ret = 0;

	ret = test_del_timer();
	if (ret < 0) {
		printf("user timer delete failed!\n");
		return -1;
	}

	ret = test_stop_timer();
	if (ret < 0) {
		printf("user timer stop failed!\n");
		return -1;
	}

	

	ret = test_cht_loop_timer();
	if (ret < 0) {
		printf("user timer changes interval and loops 2 times failed!\n");
		return -1;
	}

	ret = test_single_timer();
	if (ret < 0) {
		printf("user timer 1 times failed!\n");
		return -1;
	}
	

	return ret;
}

int pressure_test()
{
	int ret = 0;
	int i = 0;
	int array[1000] = {0};

	for (i=0; i<1000; i++) {
		array[i] = user_add(1, 2 * 1000, "timer_n", (timer_cb)cbfunc, NULL);
	}

	for (i=0; i<1000; i++) {
		ret = user_del(&array[i]);
		if (ret < 0) {
			printf("user timer start failed!\n");
			return -1;
		}
	}
	
	return ret;
}

int main()
{
	int ret = 0;
	int interval = 10 * 1000;

	ret = user_init();
	if (ret < 0) {
		printf("user timer init failed!\n");
		return -1;
	}

	ret = func_test();
	if (ret < 0) {
		printf("func test failed!\n");
		return -1;
	}
	
	ret = pressure_test();
	if (ret < 0) {
		printf("pressure test failed!\n");
		return -1;
	}	

	return 0;
}
