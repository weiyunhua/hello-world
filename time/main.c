#include <stdio.h>
#include <unistd.h>

#include "user.h"


void cbfunc()
{
	printf("cb func!\n");
}


int main()
{
	int ret = 0;
	int interval = 10000;

	ret = user_init();
	if (ret < 0) {
		printf("user timer init failed!\n");
		return -1;
	}

	int nTimerID_1 = user_add(1, 5000, "timer_1", (timer_cb)cbfunc, NULL);

	ret = user_del(&nTimerID_1);
	if (ret < 0) {
		printf("user timer delete failed!\n");
		return -1;
	}
	
	int nTimerID_2 = user_add(2, 5000, "timer_2", (timer_cb)cbfunc, NULL);

	ret = user_ch_time(&nTimerID_2, &interval);
	if (ret < 0) {
		printf("user timer change interval failed!\n");
		return -1;
	}

	ret = user_start(&nTimerID_2);
	if (ret < 0) {
		printf("user timer start failed!\n");
		return -1;
	}


	

	ret = user_stop(&nTimerID_2);
	if (ret < 0) {
		printf("user timer stop failed!\n");
		return -1;
	}

	return 0;
}
