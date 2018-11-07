
/*******************************************************************************
//Copyright(C)2018 , 蛙鸣公司
// All rights reserved.
// Version: v1.0 
// Device : STM32F103C8T6
// Built  : IAR For ARM v7.70(Language: C)
// Date   : 2018-10-27
// Author : 刘锋
// Functiom:项目工程负责调试打印信息和与服务器（不是服务器）通信的源程序。
*******************************************************************************/

/******************************************************************************
//包含头文件
*******************************************************************************/
#include "includes.h"


/******************************************************************************
//变量定义
*******************************************************************************/
//用于调试的USART的数据结构体
DebugCommStruct s_DebugComm;



/******************************************************************************
//函数声明
*******************************************************************************/
//功能:	程序调试口的初始化函数
void Debug_Init(void);

// 程序调试USART的初始化函数
void Debug_Usart_Init(void);

//函数功能: 调试用的GPIO口初始化函数
void Debug_GPIO_Init(void);

//函数功能: 与调试口通信的数据包解析函数
u8 Debug_Comm_Package_Analysis(u8 *rec_array, u16 rec_length);

//函数功能: 与调试口通信的监测函数
void Debug_Comm_Rec_Monitor(void);


/******************************************************************************
//函数定义
*******************************************************************************/
/**********************************
//名称:	Debug_Init
//功能:	程序调试口的初始化函数
//入口参数:	无
//出口参数:	无
**********************************/
void Debug_Init(void)
{
    Debug_Usart_Init();
    
//    Debug_GPIO_Init();
}

/**********************************
//名称:	Debug_Usart_Init
//功能:	程序调试USART的初始化函数
//入口参数:	无
//出口参数:	无
**********************************/
void Debug_Usart_Init(void)
{
    GPIO_InitTypeDef  GPIO_InitStructure;
    USART_InitTypeDef USART_InitStructure;
	NVIC_InitTypeDef NVIC_InitStructure;	//初始化NVIC结构体

    //复位DEBUG_USART模块
    USART_DeInit(DEBUG_USART);

    //使能DEBUG_USART_TX引脚模块时钟
    RCC_APB2PeriphClockCmd(DEBUG_USART_TX_GPIO_CLK, ENABLE);

    //使能DEBUG_USART_RX引脚模块时钟
    RCC_APB2PeriphClockCmd(DEBUG_USART_RX_GPIO_CLK, ENABLE);

    //使能USART模块时钟
    if(DEBUG_USART == USART1)
    {
        RCC_APB2PeriphClockCmd(DEBUG_USART_CLK, ENABLE);
    }
    else
    {
        RCC_APB1PeriphClockCmd(DEBUG_USART_CLK, ENABLE);
    }


    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;

    //DEBUG_USART的GPIO配置
    //DEBUG_USART_TX: 推挽复用输出
    GPIO_InitStructure.GPIO_Pin   = DEBUG_USART_TX_GPIO_PIN;
    GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_AF_PP;      //推挽复用输出
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;	//选择50MHz
    GPIO_Init(DEBUG_USART_TX_GPIO_PORT, &GPIO_InitStructure);
    
    //DEBUG_USART_RX: 浮空输入(或带上拉输入)
    GPIO_InitStructure.GPIO_Pin   = DEBUG_USART_RX_GPIO_PIN;
    GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_IN_FLOATING;//浮空输入
    GPIO_Init(DEBUG_USART_RX_GPIO_PORT, &GPIO_InitStructure);

    //DEBUG_USART模块参数配置
    //波特率: USART1_BAUDRATE；8个数据位；1个停止位；无校验位；无硬件流控制；使能发送和接收；
    USART_InitStructure.USART_BaudRate            = DEBUG_USART_BAUDRATE;
    USART_InitStructure.USART_WordLength          = USART_WordLength_8b;
    USART_InitStructure.USART_StopBits            = USART_StopBits_1;
    USART_InitStructure.USART_Parity              = USART_Parity_No ;
    USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
    USART_InitStructure.USART_Mode                = USART_Mode_Rx | USART_Mode_Tx;
    USART_Init(DEBUG_USART, &USART_InitStructure); 

    USART_Cmd(DEBUG_USART, ENABLE);                         //使能DEBUG_USART模块
    
    NVIC_InitStructure.NVIC_IRQChannel                      = DEBUG_USART_IRQn; //使能USART中断	
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority    = 0;                //抢断优先级为1
	NVIC_InitStructure.NVIC_IRQChannelSubPriority           = 2;                //响应优先级为1
    NVIC_InitStructure.NVIC_IRQChannelCmd                   = ENABLE;
    NVIC_Init(&NVIC_InitStructure);	//配置USART的嵌套向量中断

	USART_ITConfig(DEBUG_USART, USART_IT_RXNE, DISABLE);	//先禁止USART接收中断
}

////********************************************************
////函数名称: Debug_GPIO_Init
////函数功能: 调试用的GPIO口初始化函数
////输    入: 无
////输    出: 无
////备    注: 无
////********************************************************
//void Debug_GPIO_Init(void)
//{
//    GPIO_InitTypeDef  GPIO_InitStructure;
//
//    // 使能485传输方向控制引脚的时钟
//    RCC_APB2PeriphClockCmd(DEBUG_DIRECTION_CTR_GPIO_CLK, ENABLE);
//
//    // 配置485方向控制引脚
//    GPIO_InitStructure.GPIO_Pin         = DEBUG_DIRECTION_CTR_GPIO_PIN;  //
//    GPIO_InitStructure.GPIO_Mode        = GPIO_Mode_Out_OD;	//开漏输出模式
//    GPIO_InitStructure.GPIO_Speed       = GPIO_Speed_50MHz;	//选择50MHz
//    GPIO_Init(DEBUG_DIRECTION_CTR_GPIO_PORT, &GPIO_InitStructure);  
//}

/**********************************
//名称:	putchar
//功能:	重写putchar函数，供系统库函数调用
//入口参数:	ch——int，要打印数据
//出口参数:	int,返回值
**********************************/
//PUTCHAR_PROTOTYPE
//{
//    USART_SendData(DEBUG_USART, (u8)ch);
//    while(USART_GetFlagStatus(DEBUG_USART, USART_FLAG_TXE) == RESET);
//    
//    return ch;
//}
int putchar(int ch)
{
    USART_SendData(DEBUG_USART, ch); //通过USART输出一个字符
    while(USART_GetFlagStatus(DEBUG_USART, USART_FLAG_TXE) == RESET);
                                                          //等待发送完成
    return ch;
}

//********************************************************
//函数名称: Debug_Comm_Package_Analysis
//函数功能: 与调试口通信的数据包解析函数
//输    入: 无
//输    出: 无
//备    注: 无
//********************************************************
u8 Debug_Comm_Package_Analysis(u8 *rec_array, u16 rec_length)
{
//	u16     i = 0;
    u8      temp_data = 0;
//	u16     check = 0;	    //定义校验
    u16     temp_index = 0;        //数据索引
//	u8      data_analysis_status = DEBUG_COMM_PACKAGE_ANALYSIS_HEAD;	    //数据分析状态
    u8      packet_analysis_error_status = PACKAGE_ANALYSIS_SUCCEED;    //数据包解析错误状态
    
#if (SERVER_PRINTF_EN)
printf("接收到调试口的数据: ");
#endif	

	for(temp_index = 0; temp_index < rec_length; temp_index++)        //
	{
		temp_data = rec_array[temp_index];  //取出接收缓存区数据
        
#if (SERVER_PRINTF_EN)
printf("%02X ",temp_data);  // 
#endif

//		switch(data_analysis_status)    //根据查找状态处理
//		{
//			case DEBUG_COMM_PACKAGE_ANALYSIS_HEAD:	    //如果是包头
//            {
//                
//            }
//            break;
//                
//			case DEBUG_COMM_PACKAGE_ANALYSIS_LENGTH:	        //如果是数据包数据长度
//            {
//                
//            }
//            break;
//                
//			case DEBUG_COMM_PACKAGE_ANALYSIS_FUNCTION:    //如果是功能码
//            {
//                
//            }
//            break;
//                
//			case DEBUG_COMM_PACKAGE_ANALYSIS_CMD:          //如果是命令码
//            {
//                
//            }
//            break;
//            
//			case DEBUG_COMM_PACKAGE_ANALYSIS_SERIAL:          //如果序列号
//            {
//                
//            }
//            break;
//                
//			case DEBUG_COMM_PACKAGE_ANALYSIS_DATA:          //如果是数据内容
//            {
//                
//            }
//            break;
//                
//			case DEBUG_COMM_PACKAGE_ANALYSIS_CHECK:          //如果是校验码
//            {
//                
//            }
//            break;
//            
//			default:
//            {
//                
//            }
//            break;
//		}
	}
    
    return packet_analysis_error_status;
}

//********************************************************
//函数名称: Debug_Comm_Rec_Monitor
//函数功能: 与调试口通信的监测函数
//输    入: 无
//输    出: 无
//备    注: 无
//********************************************************
void Debug_Comm_Rec_Monitor(void)      
{
    if(s_DebugComm.RxStatus == TRUE)   //有接收
    {
        if(s_DebugComm.RxTimeout_Count >= DEBUG_COMM_RX_DATA_TIMEOUT)    //是否超时了
        {
            u16 temp_len = s_DebugComm.RxIndex;
            if(s_ServerCommRx.Status == FALSE)
            {
            
#if (SERVER_PRINTF_EN)
            printf("接收到Debug口数据包\r\n");
#endif	
            
                for(u16 i = 0; i < temp_len; i++)
                {
                    USART_SendData(SERVER_COMM_USART, s_DebugComm.RxBuffer[i]); 
                    while(USART_GetFlagStatus(SERVER_COMM_USART, USART_FLAG_TXE) == RESET);//等待发送完成
                }
                s_DebugComm.RxStatus = FALSE;
                s_DebugComm.RxIndex = 0;
                s_DebugComm.RxTimeout_Count = 0;
            }
        }
    }
}




//////////////////////////////END OF C文件////////////////////////////////////
