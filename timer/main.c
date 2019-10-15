#include <stdio.h>

#include "user.h"

void cbfunc()
{
	printf("cb func!\n");
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

	int TimerID_1 = user_add(1, 2 * 1000, "timer_1", (timer_cb)cbfunc, NULL);
	int TimerID_2 = user_add(2, 2 * 1000, "timer_2", (timer_cb)cbfunc, NULL);

	ret = user_ch_time(&TimerID_2, &interval);
	if (ret < 0) {
		printf("user timer change interval failed!\n");
		return -1;
	}

	ret = user_del(&TimerID_1);
	if (ret < 0) {
		printf("user timer delete failed!\n");
		return -1;
	}

	ret = user_start(&TimerID_2);
	if (ret < 0) {
		printf("user timer start failed!\n");
		return -1;
	}

	user_stop(&TimerID_2);

	return 0;
}
