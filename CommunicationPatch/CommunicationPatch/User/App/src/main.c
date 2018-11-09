
/*******************************************************************************
//Copyright(C)2018 , ������˾
// All rights reserved.
// Version: v1.0 
// Device : STM32F103C8T6
// Built  : IAR For ARM v7.70(Language: C)
// Date   : 2018-10-27
// Author : ����
// Functiom:��Ŀ���̵�mainԴ�ļ�
*******************************************************************************/


/******************************************************************************
//����ͷ�ļ�
*******************************************************************************/
#include "includes.h"




/******************************************************************************
//��������
*******************************************************************************/



/******************************************************************************
//��������
*******************************************************************************/





/******************************************************************************
//��������
*******************************************************************************/
/********************************************************
//��������: main
//��������: ϵͳ������
//��    ��: ��
//��    ��: ��
//��    ע: 
********************************************************/
int main(void)
{
    MCU_Init();                     //��Ƭ�����ӿڳ�ʼ��
    
    SysGlobalVariableInit();        //ϵͳ���е�ȫ�ֱ�����ʼ��
    
    SysPeripheralInit();            //ϵͳ���������ʼ��
  
    while(1)       
    {
        Server_Comm_Rec_Monitor();      //���������ͨ�ż��

        Device_Comm_Rec_Monitor();      //���豸�˵�ͨ�ż��

        System_Function_Control();      //���ܿ���

//        //��ʽ�����汾��ɾ��
//        Debug_Comm_Rec_Monitor();       //����Կ�ͨ�ŵļ��
    }
}












        
    





//********************************************************
//��������: assert_failed
//��������: �����������ʧ�ܴ�������
//��    ��: * file��uint8_t��ָ��Դ�ļ�����
//��    ��: * line��uint32_t��assert_param�����Դ�ļ��кţ�
//��    ע: 
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