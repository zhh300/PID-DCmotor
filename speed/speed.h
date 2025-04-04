#ifndef __SPEED_H
#define __SPEED_H
#include "main.h"

#include <stdio.h>
#include "usart.h"
#include "adc.h"
/*
	测速相关变量
*/
extern const uint16_t line_count ;      			// 编码器线数*减速比（根据实际修改）
extern const uint16_t time_interval_ms; 			// 采样间隔10ms
extern float rpm;                             // 每分钟转速


extern uint16_t raw_value;                    //adc数值（0-4096）
extern float voltage;                         //电压数值（0-3.3）


float GetSpeed(void);                                //测速函数，每次调用与上一次做差测速
	

uint16_t Read_ADC_Value(ADC_HandleTypeDef *hadc);    //获取adc数值0-4096
float Convert_To_Voltage(uint16_t adc_value);        //换算为电压0-3.3


#endif
