/**
 ******************************************************************************
 * @file	TMSxxTC\drivers\test\src\gpio.h
 * @brief	应用层到调用GPIO的接口，需要tms_gpio.ko的支持

 *
 @section exynos-4412

 @section Library
	-# 
- 2015-9,Menglong Woo,MenglongWoo@aliyun.com
 	
*/

/**
 * @brief	应用层到调用GPIO的接口，需要tms_gpio.ko的支持
 
 * @see	
 */

#ifndef _LIB_GPIO_H_
#define _LIB_GPIO_H_
#include "autoconfig.h"
#ifdef x86

	#define get_count( val) 
	#define get_io( val) 
	#define io_name( val) 
	#define set_io( val) 
	#define set_dir( val) 
    #define open_io( file)  (0)
	#define close_io()

#else

	int get_count(int *val);
	int get_io(struct gpio_data *val);
	int io_name(struct gpio_name *val);
	int set_io(struct gpio_data *val);
	int set_dir(struct gpio_dir *val);
	int open_io(char *file);
	int close_io();
#endif



#endif
