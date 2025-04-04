# PID-DCmotor
有刷直流电机PID控制相关程序
​
# 测速部分
## 1.CubeMX配置
### 配置编码器接口（TIM3）

    模式：Encoder Mode（TI1和TI2）
    引脚：根据硬件连接选择（如PA6和PA7）
    分频（Prescaler）：0
    自动重装载值（AutoReload）：65535（0xFFFF）
    编码器模式：TI1和TI2边沿计数（四倍频）
### 配置定时器中断（TIM2）​

    时钟源：内部时钟
    分频（Prescaler）：7199（72MHz / 7200 = 10kHz）
    自动重装载值（AutoReload）：99（10kHz / 100 = 100Hz，即10ms中断）
    启用中断：开启TIM2更新中断
### 配置串口（USART1）​

    波特率：115200
    数据位：8
    停止位：1
    无校验
## 2.代码实现
### 2.1printf重定义

    int fputc(int ch, FILE *f)// 重写fputc函数，使printf的输出由UART1实现,  这里使用USART1
    {
          // 注意，不能使用HAL_UART_Transmit_IT(), 机制上会冲突; 因为调用中断发送函数后，如果上次发送 
          // 还在进行，就会直接返回！
    	  // 它不会继续等待，也不会数据填入队列排队发送
    	  // 使用HAL_UART_Transmit，相等于USART1->DR = ch, 函数内部加了简单的超时判断(ms)，防止卡死
        HAL_UART_Transmit(&huart1, (uint8_t *)&ch, 1, 0x02);   
        return ch;
    } 

### 2.2速度计算
    const uint16_t line_count = 11*90;    // 编码器线数*减速比（根据实际修改）
    const uint16_t time_interval_ms = 10; // 采样间隔10ms
    float rpm = 0;
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
# 测电流部分
## CubeMX配置
### ​​1.配置时钟​​：
    设置系统时钟（如使用外部晶振，配置为72MHz）。
    ADC时钟分频至≤14MHz（例如APB2为72MHz时，分频系数设为6，ADC时钟为12MHz）。
​​### 2.ADC引脚配置​​：

    选择ADC通道对应的引脚（如PA0对应ADC1_IN0），设置为模拟输入（Analog）。
### 3.ADC参数设置​​：
    ​​Mode​​：Independent mode
    ​​Scan Conversion Mode​​：Disabled（单通道不需要扫描）
    ​​Continuous Conversion Mode​​：Disabled（单次转换）
    ​​End of Conversion Selection​​：EOC flag at the end of conversion
    ​​Data Alignment​​：Right alignment（数据右对齐）
    ​​Sampling Time​​：根据需求选择（如55.5 cycles）
## 代码部分
### ADC校准放在main初始化部分
    HAL_ADCEx_Calibration_Start(&hadc1); // ADC校准
### adc读取函数
    uint16_t Read_ADC_Value(ADC_HandleTypeDef *hadc)
    {
      HAL_ADC_Start(hadc);                          // 启动转换
      if (HAL_ADC_PollForConversion(hadc, 100) == HAL_OK)
      {
        return HAL_ADC_GetValue(hadc);              // 返回原始值（0-4095）
      }
      return 0xFFFF; // 超时或错误
    }
    
    float Convert_To_Voltage(uint16_t adc_value)
    {
      return (adc_value * 3.3f) / 4095.0f;         // 转换为电压值（假设Vref=3.3V）
    }
# 调用部分
## 定时器调用
    // TIM2中断处理函数（转速计算和发送）
    void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim) 
    {
        if (htim->Instance == TIM2)
    	{ 
    		rpm = GetSpeed();
    		raw_value = Read_ADC_Value(&hadc1);
    		voltage = Convert_To_Voltage(raw_value);
    	}
    }

​

