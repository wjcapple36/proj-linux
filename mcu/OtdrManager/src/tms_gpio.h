#ifndef _TMS_GPIO_H_
#define _TMS_GPIO_H_
#include "autoconfig.h"
#include "libgpio.h"

#define		LED_ON  1
#define		LED_OFF 0
#define		IO_COUNT 40


// ioctl 命令类型，不同命令传递的参数不同

#define RET_SUCCESS (0)
#define RET_INVAILD (1)
#define RET_MEM_ERR (2)

/**
 * @brief	获取端口数
 * @param	arg : (int*)
 * @retval	
 */
#define CMD_GET_COUNT 0
/**
 * @brief	设置端口属性
 * @param	arg (struct gpio_resource*)
 */
#define CMD_SET_DIR    1

 /**
 * @brief	获取端口输入输出方向
 * @param	arg (struct gpio_resource*)
 */
#define CMD_GET_DIR    2


/**
 * @brief	获取端口输入输出方向
 * @param	arg (struct gpio_resource*)
 */
#define CMD_SET_VAL   3

/**
 * @brief	获取端口输入输出状态
 * @param	arg (struct gpio_resource*)
 */
#define CMD_GET_VAL   4

/**
 * @brief	获取端口描述
 * @param	arg (struct gpio_resource*)
 */
#define CMD_IO_NAME   5

 /**
 * @brief	获取端口属性
 * @param	arg (struct gpio_resource*)
 */
#define CMD_IO_INF   6


#define MAX_NAME (32)

struct gpio_resource
{
	int   gpio;
	char name[MAX_NAME];
#define IO_INPUT  0
#define IO_OUTPUT 1
#define IO_FLOATING   2
	int   dir;
	int   index;
};


struct gpio_name
{
	int index;
	char name[MAX_NAME];
};

struct gpio_data
{
	int index;
	int data;
};

struct gpio_dir
{
	int index;
	int dir;
};



// *****************************************************************************
// 根据驱动的不同下面的序号可能发生变动
#if (CONFIG_BOARD == 11)
#define      IO_IP_SET_GPIO     0
#define IO_UNIT16_220V_GPIO     1
#define IO_UNIT15_220V_GPIO     2
#define  IO_UNIT16_48V_GPIO     3
#define  IO_UNIT15_48V_GPIO     4
#define  IO_UNIT16_12V_GPIO     5
#define  IO_UNIT15_12V_GPIO     6
#define     IO_UNIT_13_GPIO     7
#define     IO_UNIT_12_GPIO     8
#define     IO_UNIT_11_GPIO     9
#define     IO_UNIT_10_GPIO     10
#define      IO_UNIT_9_GPIO     11
#define      IO_UNIT_8_GPIO     12
#define      IO_UNIT_7_GPIO     13
#define      IO_UNIT_6_GPIO     14
#define      IO_UNIT_5_GPIO     15
#define      IO_UNIT_4_GPIO     16
#define      IO_UNIT_3_GPIO     17
#define      IO_UNIT_2_GPIO     18
#define      IO_UNIT_1_GPIO     19
#define        IO_KEY_ALARM     20
#define  IO_LED_ALARM_TOTAL     21
#define          IO_LED_RUN     22
#define   IO_LED_ALARM_COMM     23
#define          IO_GSM_RST     24
#define           IO_GSM_EN     25
#define         IO_SW_RESET     26

#elif (CONFIG_BOARD == 12)
#define      IO_IP_SET_GPIO     00
#define IO_UNIT16_220V_GPIO     01
#define IO_UNIT15_220V_GPIO     02
#define  IO_UNIT16_48V_GPIO     03
#define  IO_UNIT15_48V_GPIO     04
#define  IO_UNIT16_12V_GPIO     05
#define  IO_UNIT15_12V_GPIO     06
#define     IO_UNIT_13_GPIO     07
#define     IO_UNIT_12_GPIO     08
#define     IO_UNIT_11_GPIO     09
#define     IO_UNIT_10_GPIO     10
#define      IO_UNIT_9_GPIO     11
#define      IO_UNIT_8_GPIO     12
#define      IO_UNIT_7_GPIO     13
#define      IO_UNIT_6_GPIO     14
#define      IO_UNIT_5_GPIO     15
#define      IO_UNIT_4_GPIO     16
#define      IO_UNIT_3_GPIO     17
#define      IO_UNIT_2_GPIO     18
#define      IO_UNIT_1_GPIO     19
#define        IO_KEY_ALARM     20
#define  IO_LED_ALARM_TOTAL     21
#define          IO_LED_RUN     22
#define          IO_RST_NET     23
#define   IO_LED_ALARM_COMM     24
#define          IO_GSM_RST     25
#define           IO_GSM_EN     26
#define         IO_SW_RESET     27
#define           IO_UNSUE0     28
#define       IO_LED_BUZZER     29
#endif



#endif
