
/*******************************************************************************
//Copyright(C)2018 , 蛙鸣公司
// All rights reserved.
// Version: v1.0 
// Device : STM32F103C8T6
// Built  : IAR For ARM v7.70(Language: C)
// Date   : 2018-10-27
// Author : 刘锋
// Functiom:项目工程的main源文件
*******************************************************************************/


/******************************************************************************
//包含头文件
*******************************************************************************/
#include "includes.h"




/******************************************************************************
//变量定义
*******************************************************************************/



/******************************************************************************
//函数声明
*******************************************************************************/





/******************************************************************************
//函数定义
*******************************************************************************/
/********************************************************
//函数名称: main
//函数功能: 系统主函数
//输    入: 无
//输    出: 无
//备    注: 
********************************************************/
int main(void)
{
    MCU_Init();                     //单片机各接口初始化
    
    SysGlobalVariableInit();        //系统所有的全局变量初始化
    
    SysPeripheralInit();            //系统所有外设初始化
  
    while(1)       
    {
        Server_Comm_Rec_Monitor();      //与服务器的通信监测

        Device_Comm_Rec_Monitor();      //与设备端的通信监测

        System_Function_Control();      //功能控制

//        //正式发布版本将删除
//        Debug_Comm_Rec_Monitor();       //与调试口通信的监测
    }
}












        
    





//********************************************************
//函数名称: assert_failed
//函数功能: 函数参数检查失败处理函数
//输    入: * file，uint8_t，指向源文件名；
//输    出: * line，uint32_t，assert_param错误的源文件行号；
//备    注: 
//********************************************************
#ifdef USE_FULL_ASSERT
void assert_failed(uint8_t* file, uint32_t line)
{
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */

  /* Infinite loop */
  while (1)
  {}
}
#endif

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
