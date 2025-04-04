#include "speed.h"

/*测速相关变量*/
const uint16_t line_count = 11*90;    // 编码器线数*减速比（根据实际修改）
const uint16_t time_interval_ms = 10; // 采样间隔10ms
float rpm = 0; 

/*测电流相关变量*/
uint16_t raw_value=0;    //adc数值（0-4096）
float voltage=0;         //电压数值（0-3.3）
/*******************************************************************************************/

/*输出重定向*/
int fputc(int ch, FILE *f)	// 重写fputc函数，使printf的输出由UART1实现,  这里使用USART1
{	// 注意，不能使用HAL_UART_Transmit_IT(), 机制上会冲突; 因为调用中断发送函数后，如果上次发送还在进行，就会直接返回！
	// 它不会继续等待，也不会数据填入队列排队发送 
	// 使用HAL_UART_Transmit，相等于USART1->DR = ch, 函数内部加了简单的超时判断(ms)，防止卡死
	HAL_UART_Transmit(&huart1, (uint8_t *)&ch, 1, 0x02);  
    	return ch;
} 



/*adc读取电流*/
uint16_t Read_ADC_Value(ADC_HandleTypeDef *hadc)            //adc数值（0-4096）
{
  HAL_ADC_Start(hadc);                          // 启动转换
  if (HAL_ADC_PollForConversion(hadc, 100) == HAL_OK)
  {
    return HAL_ADC_GetValue(hadc);              // 返回原始值（0-4095）
  }
  return 0xFFFF; // 超时或错误
}

float Convert_To_Voltage(uint16_t adc_value)        //电压数值（0-3.3）
{
  return (adc_value * 3.3f) / 4095.0f;         // 转换为电压值（假设Vref=3.3V）
}

/*测速相关函数*/
float GetSpeed(void)
{
    static int16_t last_cnt = 0;                   //局部变量,程序启动时初始化，函数调用结束后值保留,直到程序结束。
    int16_t current_cnt = TIM3->CNT;               //读取计数寄存器
    int16_t delta = current_cnt - last_cnt;        //计算差值delta
    last_cnt = current_cnt;                        //保留上一次计数值
			
    // 计算转速（整数运算避免浮点）
    int32_t rpm_x10 = (delta * 1500*10) / line_count; // 扩大10倍保留1位小数公式：10ms的脉冲数（delta）/4（倍频）*100（一秒）*60（一分）=1500
    // 通过串口发送转速
    //printf("%3d.%d,%d,%d\r\n", rpm, decimal,120,10);
    return  rpm_x10/10 + rpm_x10%10*0.1;
}



