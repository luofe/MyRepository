
/*******************************************************************************
//Copyright(C)2018 , ������˾
// All rights reserved.
// Version: v1.0 
// Device : STM32F103C8T6
// Built  : IAR For ARM v7.70(Language: C)
// Date   : 2018-10-27
// Author : ����
// Functiom:��Ŀ���̸�����Դ�ӡ��Ϣ��������������Ƿ�������ͨ�ŵ�Դ����
*******************************************************************************/

/******************************************************************************
//����ͷ�ļ�
*******************************************************************************/
#include "includes.h"


/******************************************************************************
//��������
*******************************************************************************/
//���ڵ��Ե�USART�����ݽṹ��
DebugCommStruct s_DebugComm;



/******************************************************************************
//��������
*******************************************************************************/
//����:	������Կڵĳ�ʼ������
void Debug_Init(void);

// �������USART�ĳ�ʼ������
void Debug_Usart_Init(void);

//��������: �����õ�GPIO�ڳ�ʼ������
void Debug_GPIO_Init(void);

//��������: ����Կ�ͨ�ŵ����ݰ���������
u8 Debug_Comm_Package_Analysis(u8 *rec_array, u16 rec_length);

//��������: ����Կ�ͨ�ŵļ�⺯��
void Debug_Comm_Rec_Monitor(void);


/******************************************************************************
//��������
*******************************************************************************/
/**********************************
//����:	Debug_Init
//����:	������Կڵĳ�ʼ������
//��ڲ���:	��
//���ڲ���:	��
**********************************/
void Debug_Init(void)
{
    Debug_Usart_Init();
    
//    Debug_GPIO_Init();
}

/**********************************
//����:	Debug_Usart_Init
//����:	�������USART�ĳ�ʼ������
//��ڲ���:	��
//���ڲ���:	��
**********************************/
void Debug_Usart_Init(void)
{
    GPIO_InitTypeDef  GPIO_InitStructure;
    USART_InitTypeDef USART_InitStructure;
	NVIC_InitTypeDef NVIC_InitStructure;	//��ʼ��NVIC�ṹ��

    //��λDEBUG_USARTģ��
    USART_DeInit(DEBUG_USART);

    //ʹ��DEBUG_USART_TX����ģ��ʱ��
    RCC_APB2PeriphClockCmd(DEBUG_USART_TX_GPIO_CLK, ENABLE);

    //ʹ��DEBUG_USART_RX����ģ��ʱ��
    RCC_APB2PeriphClockCmd(DEBUG_USART_RX_GPIO_CLK, ENABLE);

    //ʹ��USARTģ��ʱ��
    if(DEBUG_USART == USART1)
    {
        RCC_APB2PeriphClockCmd(DEBUG_USART_CLK, ENABLE);
    }
    else
    {
        RCC_APB1PeriphClockCmd(DEBUG_USART_CLK, ENABLE);
    }


    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;

    //DEBUG_USART��GPIO����
    //DEBUG_USART_TX: ���츴�����
    GPIO_InitStructure.GPIO_Pin   = DEBUG_USART_TX_GPIO_PIN;
    GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_AF_PP;      //���츴�����
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;	//ѡ��50MHz
    GPIO_Init(DEBUG_USART_TX_GPIO_PORT, &GPIO_InitStructure);
    
    //DEBUG_USART_RX: ��������(�����������)
    GPIO_InitStructure.GPIO_Pin   = DEBUG_USART_RX_GPIO_PIN;
    GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_IN_FLOATING;//��������
    GPIO_Init(DEBUG_USART_RX_GPIO_PORT, &GPIO_InitStructure);

    //DEBUG_USARTģ���������
    //������: USART1_BAUDRATE��8������λ��1��ֹͣλ����У��λ����Ӳ�������ƣ�ʹ�ܷ��ͺͽ��գ�
    USART_InitStructure.USART_BaudRate            = DEBUG_USART_BAUDRATE;
    USART_InitStructure.USART_WordLength          = USART_WordLength_8b;
    USART_InitStructure.USART_StopBits            = USART_StopBits_1;
    USART_InitStructure.USART_Parity              = USART_Parity_No ;
    USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
    USART_InitStructure.USART_Mode                = USART_Mode_Rx | USART_Mode_Tx;
    USART_Init(DEBUG_USART, &USART_InitStructure); 

    USART_Cmd(DEBUG_USART, ENABLE);                         //ʹ��DEBUG_USARTģ��
    
    NVIC_InitStructure.NVIC_IRQChannel                      = DEBUG_USART_IRQn; //ʹ��USART�ж�	
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority    = 0;                //�������ȼ�Ϊ1
	NVIC_InitStructure.NVIC_IRQChannelSubPriority           = 2;                //��Ӧ���ȼ�Ϊ1
    NVIC_InitStructure.NVIC_IRQChannelCmd                   = ENABLE;
    NVIC_Init(&NVIC_InitStructure);	//����USART��Ƕ�������ж�

	USART_ITConfig(DEBUG_USART, USART_IT_RXNE, DISABLE);	//�Ƚ�ֹUSART�����ж�
}

////********************************************************
////��������: Debug_GPIO_Init
////��������: �����õ�GPIO�ڳ�ʼ������
////��    ��: ��
////��    ��: ��
////��    ע: ��
////********************************************************
//void Debug_GPIO_Init(void)
//{
//    GPIO_InitTypeDef  GPIO_InitStructure;
//
//    // ʹ��485���䷽��������ŵ�ʱ��
//    RCC_APB2PeriphClockCmd(DEBUG_DIRECTION_CTR_GPIO_CLK, ENABLE);
//
//    // ����485�����������
//    GPIO_InitStructure.GPIO_Pin         = DEBUG_DIRECTION_CTR_GPIO_PIN;  //
//    GPIO_InitStructure.GPIO_Mode        = GPIO_Mode_Out_OD;	//��©���ģʽ
//    GPIO_InitStructure.GPIO_Speed       = GPIO_Speed_50MHz;	//ѡ��50MHz
//    GPIO_Init(DEBUG_DIRECTION_CTR_GPIO_PORT, &GPIO_InitStructure);  
//}

/**********************************
//����:	putchar
//����:	��дputchar��������ϵͳ�⺯������
//��ڲ���:	ch����int��Ҫ��ӡ����
//���ڲ���:	int,����ֵ
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
    USART_SendData(DEBUG_USART, ch); //ͨ��USART���һ���ַ�
    while(USART_GetFlagStatus(DEBUG_USART, USART_FLAG_TXE) == RESET);
                                                          //�ȴ��������
    return ch;
}

//********************************************************
//��������: Debug_Comm_Package_Analysis
//��������: ����Կ�ͨ�ŵ����ݰ���������
//��    ��: ��
//��    ��: ��
//��    ע: ��
//********************************************************
u8 Debug_Comm_Package_Analysis(u8 *rec_array, u16 rec_length)
{
//	u16     i = 0;
    u8      temp_data = 0;
//	u16     check = 0;	    //����У��
    u16     temp_index = 0;        //��������
//	u8      data_analysis_status = DEBUG_COMM_PACKAGE_ANALYSIS_HEAD;	    //���ݷ���״̬
    u8      packet_analysis_error_status = PACKAGE_ANALYSIS_SUCCEED;    //���ݰ���������״̬
    
#if (SERVER_PRINTF_EN)
printf("���յ����Կڵ�����: ");
#endif	

	for(temp_index = 0; temp_index < rec_length; temp_index++)        //
	{
		temp_data = rec_array[temp_index];  //ȡ�����ջ���������
        
#if (SERVER_PRINTF_EN)
printf("%02X ",temp_data);  // 
#endif

//		switch(data_analysis_status)    //���ݲ���״̬����
//		{
//			case DEBUG_COMM_PACKAGE_ANALYSIS_HEAD:	    //����ǰ�ͷ
//            {
//                
//            }
//            break;
//                
//			case DEBUG_COMM_PACKAGE_ANALYSIS_LENGTH:	        //��������ݰ����ݳ���
//            {
//                
//            }
//            break;
//                
//			case DEBUG_COMM_PACKAGE_ANALYSIS_FUNCTION:    //����ǹ�����
//            {
//                
//            }
//            break;
//                
//			case DEBUG_COMM_PACKAGE_ANALYSIS_CMD:          //�����������
//            {
//                
//            }
//            break;
//            
//			case DEBUG_COMM_PACKAGE_ANALYSIS_SERIAL:          //������к�
//            {
//                
//            }
//            break;
//                
//			case DEBUG_COMM_PACKAGE_ANALYSIS_DATA:          //�������������
//            {
//                
//            }
//            break;
//                
//			case DEBUG_COMM_PACKAGE_ANALYSIS_CHECK:          //�����У����
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
//��������: Debug_Comm_Rec_Monitor
//��������: ����Կ�ͨ�ŵļ�⺯��
//��    ��: ��
//��    ��: ��
//��    ע: ��
//********************************************************
void Debug_Comm_Rec_Monitor(void)      
{
    if(s_DebugComm.RxStatus == TRUE)   //�н���
    {
        if(s_DebugComm.RxTimeout_Count >= DEBUG_COMM_RX_DATA_TIMEOUT)    //�Ƿ�ʱ��
        {
            u16 temp_len = s_DebugComm.RxIndex;
            if(s_ServerCommRx.Status == FALSE)
            {
            
#if (SERVER_PRINTF_EN)
            printf("���յ�Debug�����ݰ�\r\n");
#endif	
            
                for(u16 i = 0; i < temp_len; i++)
                {
                    USART_SendData(SERVER_COMM_USART, s_DebugComm.RxBuffer[i]); 
                    while(USART_GetFlagStatus(SERVER_COMM_USART, USART_FLAG_TXE) == RESET);//�ȴ��������
                }
                s_DebugComm.RxStatus = FALSE;
                s_DebugComm.RxIndex = 0;
                s_DebugComm.RxTimeout_Count = 0;
            }
        }
    }
}




//////////////////////////////END OF C�ļ�////////////////////////////////////