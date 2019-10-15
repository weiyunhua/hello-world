#ifndef __USER_H__
#define __USER_H__

#include "timer.h"

/*
 * 函数介绍：用户控制初始化定时器模块(主控模块初始化一次即可)
 * 输入参数：无
 * 输出参数：无
 * 返回值  ：>0-成功; <0-失败
 */
int user_init();

/*
 * 函数介绍：用户控制添加定时器
 * 输入参数：once:     -1:循环定时，N(N>0):运行N次后停止
             interval: 定时器时间间隔，单位：毫秒
             func:     定时器回调函数
             param:    回调函数参数
 * 输出参数：无
 * 返回值  ：>0-成功，表示定时器句柄；<=0-错误代码
 */
int user_add(int run_time, int interval, char* cb_func_name,
                timer_cb func, void* param);


/*
 * 函数介绍：用户删除定时器
 * 输入参数：handle: 定时器句柄
 * 输出参数：无
 * 返回值  ：>=0-成功，<0-失败
 */
int user_del(int* handle);

/*
 * 函数介绍：用户启动定时器
 * 输入参数：handle: 定时器句柄
 * 输出参数：无
 * 返回值  ：>=0-成功，<0-失败
 */
int user_start(int* handle);

/*
 * 函数介绍：用户停止定时器
 * 输入参数：handle: 定时器句柄
 * 输出参数：无
 * 返回值  ：>=0-成功，<0-失败
 */
int user_stop(int* handle);


/*
 * 函数介绍：用户改变定时器模块的间隔
 * 输入参数：handle: 定时器句柄, interval: 定时间隔
 * 输出参数：无
 * 返回值  ：>0-成功; <0-失败
 */
int user_ch_time(int* handle, int* interval);;


#endif // __USER_H__



