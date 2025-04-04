#include "speed.h"


const uint16_t line_count = 11*90;    // ����������*���ٱȣ�����ʵ���޸ģ�
const uint16_t time_interval_ms = 10; // �������10ms
float rpm = 0; 

int fputc(int ch, FILE *f)				 // ��дfputc������ʹprintf�������UART1ʵ��,  ����ʹ��USART1
{			// ע�⣬����ʹ��HAL_UART_Transmit_IT(), �����ϻ��ͻ; ��Ϊ�����жϷ��ͺ���������ϴη��ͻ��ڽ��У��ͻ�ֱ�ӷ��أ�
			// ����������ȴ���Ҳ����������������Ŷӷ��� 
			// ʹ��HAL_UART_Transmit�������USART1->DR = ch, �����ڲ����˼򵥵ĳ�ʱ�ж�(ms)����ֹ����
	  HAL_UART_Transmit(&huart1, (uint8_t *)&ch, 1, 0x02);  
    return ch;
} 


// TIM2�жϴ�������ת�ټ���ͷ��ͣ�
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim) 
{
    if (htim->Instance == TIM2) {
			
        static int16_t last_cnt = 0;                   //�ֲ�����,��������ʱ��ʼ�����������ý�����ֵ����,ֱ�����������
        int16_t current_cnt = TIM3->CNT;               //��ȡ�����Ĵ���
        int16_t delta = current_cnt - last_cnt;        //�����ֵdelta
        last_cnt = current_cnt;                        //������һ�μ���ֵ
			
        // ����ת�٣�����������⸡�㣩
        int32_t rpm_x10 = (delta * 1500*10) / line_count; // ����10������1λС����ʽ��10ms����������delta��/4����Ƶ��*100��һ�룩*60��һ�֣�=1500
        // ͨ�����ڷ���ת��
        //printf("%3d.%d,%d,%d\r\n", rpm, decimal,120,10);
			  rpm = rpm_x10/10 + rpm_x10%10*0.1;
    }
		
}
