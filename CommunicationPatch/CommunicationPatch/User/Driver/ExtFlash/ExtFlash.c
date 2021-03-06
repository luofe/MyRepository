
/*******************************************************************************
//Copyright(C)2018 , 蛙鸣公司
// All rights reserved.
// Version: v1.0
// Device : STM32F103C8T6
// Built  : IAR For ARM v7.70(Language: C)
// Date   : 2018-10-27
// Author : 刘锋
// Functiom:项目工程使用外部Flash存储传感器数据的源程序。
//备注:
The W25Q128FV array is organized into 65,536 programmable pages of 256-bytes each. Up to 256
bytes can be programmed at a time. Pages can be erased in groups of 16 (4KB sector erase), groups of
128 (32KB block erase), groups of 256 (64KB block erase) or the entire chip (chip erase). The
W25Q128FV has 4,096 erasable sectors and 256 erasable blocks respectively. The small 4KB sectors
allow for greater flexibility in applications that require data and parameter storage. (See Figure 2.)

存储空间规划：
1、如果按照每分钟一个数据包的存储算，可以支持最大45天（45 * 24 * 60 = 64800页）的数据存储；
2、如果按照每分钟两个数据包的存储算，则最大只能支持22天（64800 / （24 * 60 * 2） = 22.5）；
3、详见工程附件文档。

存储数据操作顺序：
//1、第一个扇区前2个字节用于存放本次存储数据的页码；
//2、第二个扇区开始，每页存放一组未发送的数据包；
//3、每个扇区有16页，总共有4096个字节，每次存储一页；
//4、先把整个扇区读取出来，然后把对应要写入的页的数据更新到扇区数组内，整个扇区再写入；
//5、如果存储到了45天，则重新从最早存储地址开始覆盖；
//6、扇区擦除的地址必须是4K的整数倍，即每个扇区的首地址。
*******************************************************************************/

/******************************************************************************
//包含头文件
*******************************************************************************/
#include "includes.h"




/******************************************************************************
//变量定义
*******************************************************************************/
static __IO uint32_t  SPITimeout = SPIT_LONG_TIMEOUT;

// 存储数据的页码
u16 g_DataPageNum = SENSOR_DATA_MIN_PAGE_NUM - 1;

// 外部存储是否有数据待发送
u8  g_ExtFlashHaveData = FALSE; //默认没有
















/******************************************************************************
//函数声明
*******************************************************************************/
//函数功能: SPI初始化函数
void SPI_FLASH_Init(void);

//函数功能: 擦除FLASH扇区函数
void SPI_FLASH_SectorErase(u32 SectorAddr);

//函数功能: 擦除FLASH扇区，整片擦除函数
void SPI_FLASH_BulkErase(void);

//函数功能: 对FLASH按页写入数据，调用本函数写入数据前需要先擦除扇区
void SPI_FLASH_PageWrite(u8* pBuffer, u32 WriteAddr, u16 NumByteToWrite);

//函数功能: 对FLASH写入数据，调用本函数写入数据前需要先擦除扇区
void SPI_FLASH_BufferWrite(u8* pBuffer, u32 WriteAddr, u16 NumByteToWrite);

//函数功能: 读取FLASH数据
void SPI_FLASH_BufferRead(u8* pBuffer, u32 ReadAddr, u16 NumByteToRead);

//函数功能: 读取FLASH ID
u32 SPI_FLASH_ReadID(void);

//函数功能: 读取FLASH Device ID
u32 SPI_FLASH_ReadDeviceID(void);

//函数功能: Flash开始读取的顺序
void SPI_FLASH_StartReadSequence(u32 ReadAddr);

//函数功能: 使用SPI读取一个字节的数据
u8 SPI_FLASH_ReadByte(void);

//函数功能: 使用SPI发送一个字节的数据
u8 SPI_FLASH_SendByte(u8 byte);

//函数功能: 发送半个字的函数
u16 SPI_FLASH_SendHalfWord(u16 HalfWord);

//函数功能: 向FLASH发送 写使能 命令
void SPI_FLASH_WriteEnable(void);

//函数功能: 等待到FLASH内部数据写入完毕
void SPI_FLASH_WaitForWriteEnd(void);

//函数功能: 进入掉电模式
void SPI_Flash_PowerDown(void);

//函数功能: 唤醒Flash
void SPI_Flash_WAKEUP(void);

//函数功能: 等待超时回调函数
static  uint16_t SPI_TIMEOUT_UserCallback(uint8_t errorCode);

//函数功能: 片外Flash检测函数
u8  Ext_Flash_Detect(void);

//函数功能: 数据存储处理函数
void Data_Storge_Process(u8* data, u16 len);










//函数功能: 片外Flash测试函数
void Ext_Flash_Test(void);

/******************************************************************************
//函数定义
*******************************************************************************/
//********************************************************
//函数名称: SPI_FLASH_Init
//函数功能: SPI初始化函数
//输    入: 无
//输    出: 无
//备    注: 无
//********************************************************
void SPI_FLASH_Init(void)
{
    SPI_InitTypeDef  SPI_InitStructure;
    GPIO_InitTypeDef GPIO_InitStructure;

    //复位SPI模块
    SPI_I2S_DeInit(FLASH_SPI);

    //使能USART模块时钟
    if(FLASH_SPI == SPI1)
    {
        RCC_APB2PeriphClockCmd(FLASH_SPI_CLK, ENABLE);
    }
    else
    {
        RCC_APB1PeriphClockCmd(FLASH_SPI_CLK, ENABLE);
    }
    /* 使能 FLASH_SPI 及GPIO 时钟 */
    RCC_APB2PeriphClockCmd(FLASH_SPI_SCK_GPIO_CLK | FLASH_SPI_MISO_GPIO_CLK|FLASH_SPI_MOSI_GPIO_CLK|FLASH_CS_GPIO_CLK, ENABLE);

    //配置时钟
    GPIO_InitStructure.GPIO_Pin     = FLASH_SPI_SCK_PIN;
    GPIO_InitStructure.GPIO_Mode    = GPIO_Mode_AF_PP;
    GPIO_InitStructure.GPIO_Speed   = GPIO_Speed_50MHz;
    GPIO_Init(FLASH_SPI_SCK_GPIO_PORT, &GPIO_InitStructure);

    //数据输入
    GPIO_InitStructure.GPIO_Pin     = FLASH_SPI_MISO_PIN;
    GPIO_InitStructure.GPIO_Mode    = GPIO_Mode_AF_PP;
    GPIO_InitStructure.GPIO_Speed   = GPIO_Speed_50MHz;
    GPIO_Init(FLASH_SPI_MISO_GPIO_PORT, &GPIO_InitStructure);

    //数据输出
    GPIO_InitStructure.GPIO_Pin     = FLASH_SPI_MOSI_PIN;
    GPIO_InitStructure.GPIO_Mode    = GPIO_Mode_AF_PP;
    GPIO_InitStructure.GPIO_Speed   = GPIO_Speed_50MHz;
    GPIO_Init(FLASH_SPI_MOSI_GPIO_PORT, &GPIO_InitStructure);

    //配置CS片选端口
    GPIO_InitStructure.GPIO_Pin     = FLASH_CS_PIN;
    GPIO_InitStructure.GPIO_Mode    = GPIO_Mode_Out_PP;
    GPIO_InitStructure.GPIO_Speed   = GPIO_Speed_50MHz;
    GPIO_Init(FLASH_CS_GPIO_PORT, &GPIO_InitStructure);
    //初始化电平
    SPI_FLASH_CS_HIGH();       //初始状态失能

    //SPI部分初始化配置
    //开始SD初始化阶段，SPI时钟频率必须小于400K
    SPI_InitStructure.SPI_Direction         = SPI_Direction_2Lines_FullDuplex;  //双线全双工
    SPI_InitStructure.SPI_Mode              = SPI_Mode_Master;                  //主模式
    SPI_InitStructure.SPI_DataSize          = SPI_DataSize_8b;                  //8位数据
    SPI_InitStructure.SPI_CPOL              = SPI_CPOL_High;                    //这里要注意，一定要配置为上升沿数据有效，因为SD卡为上升沿数据有效
    SPI_InitStructure.SPI_CPHA              = SPI_CPHA_2Edge;                   //在SCK的偶数边采集数据
    SPI_InitStructure.SPI_NSS               = SPI_NSS_Soft;                     //片选脚使用软件模式
    SPI_InitStructure.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_2;          //波特率分频值设置
    SPI_InitStructure.SPI_FirstBit          = SPI_FirstBit_MSB;                 //设置数据高位在前
    SPI_InitStructure.SPI_CRCPolynomial     = 7;                                //CRC校验中的多项式，
    SPI_Init(FLASH_SPI, &SPI_InitStructure);                                  //SPI1初始化，

    SPI_Cmd(FLASH_SPI,ENABLE);                               //使能SPI1
}

//********************************************************
//函数名称: SPI_FLASH_SectorErase
//函数功能: 擦除FLASH扇区函数
//输    入: SectorAddr：要擦除的扇区地址
//输    出: 无
//备    注: 无
//********************************************************
void SPI_FLASH_SectorErase(u32 SectorAddr)
{
    /* 发送FLASH写使能命令 */
    SPI_FLASH_WriteEnable();
    SPI_FLASH_WaitForWriteEnd();
    /* 擦除扇区 */
    /* 选择FLASH: CS低电平 */
    SPI_FLASH_CS_LOW();
    /* 发送扇区擦除指令*/
    SPI_FLASH_SendByte(W25X_SectorErase);
    /*发送擦除扇区地址的高位*/
    SPI_FLASH_SendByte((SectorAddr & 0xFF0000) >> 16);
    /* 发送擦除扇区地址的中位 */
    SPI_FLASH_SendByte((SectorAddr & 0xFF00) >> 8);
    /* 发送擦除扇区地址的低位 */
    SPI_FLASH_SendByte(SectorAddr & 0xFF);
    /* 停止信号 FLASH: CS 高电平 */
    SPI_FLASH_CS_HIGH();
    /* 等待擦除完毕*/
    SPI_FLASH_WaitForWriteEnd();
}

//********************************************************
//函数名称: SPI_FLASH_BulkErase
//函数功能: 擦除FLASH扇区，整片擦除函数
//输    入: 无
//输    出: 无
//备    注: 无
//********************************************************
void SPI_FLASH_BulkErase(void)
{
    /* 发送FLASH写使能命令 */
    SPI_FLASH_WriteEnable();

    /* 整块 Erase */
    /* 选择FLASH: CS低电平 */
    SPI_FLASH_CS_LOW();
    /* 发送整块擦除指令*/
    SPI_FLASH_SendByte(W25X_ChipErase);
    /* 停止信号 FLASH: CS 高电平 */
    SPI_FLASH_CS_HIGH();

    /* 等待擦除完毕*/
    SPI_FLASH_WaitForWriteEnd();
}

//********************************************************
//函数名称: SPI_FLASH_PageWrite
//函数功能: 对FLASH按页写入数据，调用本函数写入数据前需要先擦除扇区
//输    入: pBuffer，要写入数据的指针，WriteAddr，写入地址，NumByteToWrite，写入数据长度，必须小于等于SPI_FLASH_PerWritePageSize
//输    出: 无
//备    注: 无
//********************************************************
void SPI_FLASH_PageWrite(u8* pBuffer, u32 WriteAddr, u16 NumByteToWrite)
{
    /* 发送FLASH写使能命令 */
    SPI_FLASH_WriteEnable();

    /* 选择FLASH: CS低电平 */
    SPI_FLASH_CS_LOW();
    /* 写页写指令*/
    SPI_FLASH_SendByte(W25X_PageProgram);
    /*发送写地址的高位*/
    SPI_FLASH_SendByte((WriteAddr & 0xFF0000) >> 16);
    /*发送写地址的中位*/
    SPI_FLASH_SendByte((WriteAddr & 0xFF00) >> 8);
    /*发送写地址的低位*/
    SPI_FLASH_SendByte(WriteAddr & 0xFF);

    if(NumByteToWrite > SPI_FLASH_PerWritePageSize)
    {
        NumByteToWrite = SPI_FLASH_PerWritePageSize;

#if (FLASH_PRINTF_EN)
        printf("SPI_FLASH_PageWrite too large!\r\n");
#endif

    }

    /* 写入数据*/
    while (NumByteToWrite--)
    {
        /* 发送当前要写入的字节数据 */
        SPI_FLASH_SendByte(*pBuffer);
        /* 指向下一字节数据 */
        pBuffer++;
    }

    /* 停止信号 FLASH: CS 高电平 */
    SPI_FLASH_CS_HIGH();

    /* 等待写入完毕*/
    SPI_FLASH_WaitForWriteEnd();
}

//********************************************************
//函数名称: SPI_FLASH_BufferWrite
//函数功能: 对FLASH写入数据，调用本函数写入数据前需要先擦除扇区
//输    入: pBuffer，要写入数据的指针，WriteAddr，写入地址，NumByteToWrite，写入数据长度
//输    出: 无
//备    注: 无
//********************************************************
void SPI_FLASH_BufferWrite(u8* pBuffer, u32 WriteAddr, u16 NumByteToWrite)
{
    u8 NumOfPage = 0, NumOfSingle = 0, Addr = 0, count = 0, temp = 0;

	/*mod运算求余，若writeAddr是SPI_FLASH_PageSize整数倍，运算结果Addr值为0*/
    Addr = WriteAddr % SPI_FLASH_PageSize;

	/*差count个数据值，刚好可以对齐到页地址*/
    count = SPI_FLASH_PageSize - Addr;
	/*计算出要写多少整数页*/
    NumOfPage =  NumByteToWrite / SPI_FLASH_PageSize;
	/*mod运算求余，计算出剩余不满一页的字节数*/
    NumOfSingle = NumByteToWrite % SPI_FLASH_PageSize;

    /* Addr=0,则WriteAddr 刚好按页对齐 aligned  */
    if (Addr == 0)
    {
		/* NumByteToWrite < SPI_FLASH_PageSize */
        if (NumOfPage == 0)
        {
            SPI_FLASH_PageWrite(pBuffer, WriteAddr, NumByteToWrite);
        }
        else /* NumByteToWrite > SPI_FLASH_PageSize */
        {
			/*先把整数页都写了*/
            while (NumOfPage--)
            {
                SPI_FLASH_PageWrite(pBuffer, WriteAddr, SPI_FLASH_PageSize);
                WriteAddr +=  SPI_FLASH_PageSize;
                pBuffer += SPI_FLASH_PageSize;
            }

			/*若有多余的不满一页的数据，把它写完*/
            SPI_FLASH_PageWrite(pBuffer, WriteAddr, NumOfSingle);
        }
    }
	/* 若地址与 SPI_FLASH_PageSize 不对齐  */
    else
    {
		/* NumByteToWrite < SPI_FLASH_PageSize */
        if (NumOfPage == 0)
        {
			/*当前页剩余的count个位置比NumOfSingle小，写不完*/
            if (NumOfSingle > count)
            {
                temp = NumOfSingle - count;

				/*先写满当前页*/
                SPI_FLASH_PageWrite(pBuffer, WriteAddr, count);
                WriteAddr +=  count;
                pBuffer += count;

				/*再写剩余的数据*/
                SPI_FLASH_PageWrite(pBuffer, WriteAddr, temp);
            }
            else /*当前页剩余的count个位置能写完NumOfSingle个数据*/
            {
                SPI_FLASH_PageWrite(pBuffer, WriteAddr, NumByteToWrite);
            }
        }
        else /* NumByteToWrite > SPI_FLASH_PageSize */
        {
			/*地址不对齐多出的count分开处理，不加入这个运算*/
            NumByteToWrite -= count;
            NumOfPage =  NumByteToWrite / SPI_FLASH_PageSize;
            NumOfSingle = NumByteToWrite % SPI_FLASH_PageSize;

            SPI_FLASH_PageWrite(pBuffer, WriteAddr, count);
            WriteAddr +=  count;
            pBuffer += count;

			/*把整数页都写了*/
            while (NumOfPage--)
            {
                SPI_FLASH_PageWrite(pBuffer, WriteAddr, SPI_FLASH_PageSize);
                WriteAddr +=  SPI_FLASH_PageSize;
                pBuffer += SPI_FLASH_PageSize;
            }
			/*若有多余的不满一页的数据，把它写完*/
            if (NumOfSingle != 0)
            {
                SPI_FLASH_PageWrite(pBuffer, WriteAddr, NumOfSingle);
            }
        }
    }
}

//********************************************************
//函数名称: SPI_FLASH_BufferRead
//函数功能: 读取FLASH数据
//输    入: pBuffer，要写入数据的指针，ReadAddr，读取地址，NumByteToRead，读取数据长度
//输    出: 无
//备    注: 无
//********************************************************
void SPI_FLASH_BufferRead(u8* pBuffer, u32 ReadAddr, u16 NumByteToRead)
{
    /* 选择FLASH: CS低电平 */
    SPI_FLASH_CS_LOW();

    /* 发送 读 指令 */
    SPI_FLASH_SendByte(W25X_ReadData);

    /* 发送 读 地址高位 */
    SPI_FLASH_SendByte((ReadAddr & 0xFF0000) >> 16);
    /* 发送 读 地址中位 */
    SPI_FLASH_SendByte((ReadAddr& 0xFF00) >> 8);
    /* 发送 读 地址低位 */
    SPI_FLASH_SendByte(ReadAddr & 0xFF);

	/* 读取数据 */
    while (NumByteToRead--)
    {
        /* 读取一个字节*/
        *pBuffer = SPI_FLASH_SendByte(Dummy_Byte);
        /* 指向下一个字节缓冲区 */
        pBuffer++;
    }

    /* 停止信号 FLASH: CS 高电平 */
    SPI_FLASH_CS_HIGH();
}

//********************************************************
//函数名称: SPI_FLASH_ReadID
//函数功能: 读取FLASH ID
//输    入: 无
//输    出: FLASH ID
//备    注: 无
//********************************************************
u32 SPI_FLASH_ReadID(void)
{
    u32 Temp = 0, Temp0 = 0, Temp1 = 0, Temp2 = 0;

    /* 开始通讯：CS低电平 */
    SPI_FLASH_CS_LOW();

    /* 发送JEDEC指令，读取ID */
    SPI_FLASH_SendByte(W25X_JedecDeviceID);

    /* 读取一个字节数据 */
    Temp0 = SPI_FLASH_SendByte(Dummy_Byte);

    /* 读取一个字节数据 */
    Temp1 = SPI_FLASH_SendByte(Dummy_Byte);

    /* 读取一个字节数据 */
    Temp2 = SPI_FLASH_SendByte(Dummy_Byte);

    /* 停止通讯：CS高电平 */
    SPI_FLASH_CS_HIGH();

	/*把数据组合起来，作为函数的返回值*/
    Temp = (Temp0 << 16) | (Temp1 << 8) | Temp2;

    return Temp;
}

//********************************************************
//函数名称: SPI_FLASH_ReadDeviceID
//函数功能: 读取FLASH Device ID
//输    入: 无
//输    出: FLASH Device ID
//备    注: 无
//********************************************************
u32 SPI_FLASH_ReadDeviceID(void)
{
    u32 Temp = 0;

    /* Select the FLASH: Chip Select low */
    SPI_FLASH_CS_LOW();

    /* Send "RDID " instruction */
    SPI_FLASH_SendByte(W25X_DeviceID);
    SPI_FLASH_SendByte(Dummy_Byte);
    SPI_FLASH_SendByte(Dummy_Byte);
    SPI_FLASH_SendByte(Dummy_Byte);

    /* Read a byte from the FLASH */
    Temp = SPI_FLASH_SendByte(Dummy_Byte);

    /* Deselect the FLASH: Chip Select high */
    SPI_FLASH_CS_HIGH();

    return Temp;
}

//********************************************************
//函数名称: SPI_FLASH_StartReadSequence
//函数功能: Flash开始读取的顺序
//输    入: ReadAddr——读取地址
//输    出: 无
//备    注: 无
//********************************************************
void SPI_FLASH_StartReadSequence(u32 ReadAddr)
{
    /* Select the FLASH: Chip Select low */
    SPI_FLASH_CS_LOW();

    /* Send "Read from Memory " instruction */
    SPI_FLASH_SendByte(W25X_ReadData);

    /* Send the 24-bit address of the address to read from -----------------------*/
    /* Send ReadAddr high nibble address byte */
    SPI_FLASH_SendByte((ReadAddr & 0xFF0000) >> 16);
    /* Send ReadAddr medium nibble address byte */
    SPI_FLASH_SendByte((ReadAddr& 0xFF00) >> 8);
    /* Send ReadAddr low nibble address byte */
    SPI_FLASH_SendByte(ReadAddr & 0xFF);
}

//********************************************************
//函数名称: SPI_FLASH_ReadByte
//函数功能: 使用SPI读取一个字节的数据
//输    入: 无
//输    出: 返回接收到的数据
//备    注: 无
//********************************************************
u8 SPI_FLASH_ReadByte(void)
{
    return (SPI_FLASH_SendByte(Dummy_Byte));
}

//********************************************************
//函数名称: SPI_FLASH_SendByte
//函数功能: 使用SPI发送一个字节的数据
//输    入: byte：要发送的数据
//输    出: 返回接收到的数据
//备    注: 无
//********************************************************
u8 SPI_FLASH_SendByte(u8 byte)
{
    SPITimeout = SPIT_FLAG_TIMEOUT;

    /* 等待发送缓冲区为空，TXE事件 */
    while (SPI_I2S_GetFlagStatus(FLASH_SPI, SPI_I2S_FLAG_TXE) == RESET)
    {
        if((SPITimeout--) == 0) return SPI_TIMEOUT_UserCallback(0);
    }

    /* 写入数据寄存器，把要写入的数据写入发送缓冲区 */
    SPI_I2S_SendData(FLASH_SPI, byte);

    SPITimeout = SPIT_FLAG_TIMEOUT;

    /* 等待接收缓冲区非空，RXNE事件 */
    while (SPI_I2S_GetFlagStatus(FLASH_SPI, SPI_I2S_FLAG_RXNE) == RESET)
    {
        if((SPITimeout--) == 0) return SPI_TIMEOUT_UserCallback(1);
    }

    /* 读取数据寄存器，获取接收缓冲区数据 */
    return SPI_I2S_ReceiveData(FLASH_SPI);
}

//********************************************************
//函数名称: SPI_FLASH_SendHalfWord
//函数功能: 发送半个字的函数
//输    入: HalfWord——半字数据
//输    出: 返回接收到的数据
//备    注: 无
//********************************************************
u16 SPI_FLASH_SendHalfWord(u16 HalfWord)
{

    SPITimeout = SPIT_FLAG_TIMEOUT;

    /* Loop while DR register in not emplty */
    while (SPI_I2S_GetFlagStatus(FLASH_SPI, SPI_I2S_FLAG_TXE) == RESET)
    {
        if((SPITimeout--) == 0) return SPI_TIMEOUT_UserCallback(2);
    }

    /* Send Half Word through the FLASH_SPI peripheral */
    SPI_I2S_SendData(FLASH_SPI, HalfWord);

    SPITimeout = SPIT_FLAG_TIMEOUT;

    /* Wait to receive a Half Word */
    while (SPI_I2S_GetFlagStatus(FLASH_SPI, SPI_I2S_FLAG_RXNE) == RESET)
    {
        if((SPITimeout--) == 0) return SPI_TIMEOUT_UserCallback(3);
    }
    /* Return the Half Word read from the SPI bus */
    return SPI_I2S_ReceiveData(FLASH_SPI);
}

//********************************************************
//函数名称: SPI_FLASH_WriteEnable
//函数功能: 向FLASH发送 写使能 命令
//输    入: 无
//输    出: 无
//备    注: 无
//********************************************************
void SPI_FLASH_WriteEnable(void)
{
    /* 通讯开始：CS低 */
    SPI_FLASH_CS_LOW();

    /* 发送写使能命令*/
    SPI_FLASH_SendByte(W25X_WriteEnable);

    /*通讯结束：CS高 */
    SPI_FLASH_CS_HIGH();
}

//********************************************************
//函数名称: SPI_FLASH_WaitForWriteEnd
//函数功能: 等待到FLASH内部数据写入完毕
//输    入: 无
//输    出: 无
//备    注: 无
//********************************************************
void SPI_FLASH_WaitForWriteEnd(void)
{
    u8 FLASH_Status = 0;

    /* 选择 FLASH: CS 低 */
    SPI_FLASH_CS_LOW();

    /* 发送 读状态寄存器 命令 */
    SPI_FLASH_SendByte(W25X_ReadStatusReg);

    SPITimeout = SPIT_FLAG_TIMEOUT;
    /* 若FLASH忙碌，则等待 */
    do
    {
        /* 读取FLASH芯片的状态寄存器 */
        FLASH_Status = SPI_FLASH_SendByte(Dummy_Byte);

        {
            if((SPITimeout--) == 0)
            {
                SPI_TIMEOUT_UserCallback(4);
                return;
            }
        }
    }
    while ((FLASH_Status & WIP_Flag) == SET); /* 正在写入标志 */

    /* 停止信号  FLASH: CS 高 */
    SPI_FLASH_CS_HIGH();
}

//********************************************************
//函数名称: SPI_Flash_PowerDown
//函数功能: 进入掉电模式
//输    入: 无
//输    出: 无
//备    注: 无
//********************************************************
void SPI_Flash_PowerDown(void)
{
    /* 选择 FLASH: CS 低 */
    SPI_FLASH_CS_LOW();

    /* 发送 掉电 命令 */
    SPI_FLASH_SendByte(W25X_PowerDown);

    /* 停止信号  FLASH: CS 高 */
    SPI_FLASH_CS_HIGH();
}

//********************************************************
//函数名称: SPI_Flash_WAKEUP
//函数功能: 唤醒Flash
//输    入: 无
//输    出: 无
//备    注: 无
//********************************************************
void SPI_Flash_WAKEUP(void)
{
    /*选择 FLASH: CS 低 */
    SPI_FLASH_CS_LOW();

    /* 发上 上电 命令 */
    SPI_FLASH_SendByte(W25X_ReleasePowerDown);

    /* 停止信号 FLASH: CS 高 */
    SPI_FLASH_CS_HIGH();                   //等待TRES1
}

//********************************************************
//函数名称: SPI_TIMEOUT_UserCallback
//函数功能: 等待超时回调函数
//输    入: errorCode——错误码
//输    出: 无
//备    注: 无
//********************************************************
static  uint16_t SPI_TIMEOUT_UserCallback(uint8_t errorCode)
{
    /* 等待超时后的处理,输出错误信息 */
#if (FLASH_PRINTF_EN)
    printf("SPI 等待超时!errorCode = %d",errorCode);
#endif

    return 0;
}

//********************************************************
//函数名称: Ext_Flash_Detect
//函数功能: 片外Flash检测函数
//输    入: 无
//输    出: 返回检测结果，成功或者失败
//备    注: 无
//********************************************************
u8  Ext_Flash_Detect(void)
{
    u8  page_num[2];

    //读取的ID存储位置
    u32 DeviceID = 0;
    u32 FlashID = 0;

    /* 获取 Flash Device ID */
	DeviceID = SPI_FLASH_ReadDeviceID();
	Delay_us( 200 );
	/* 获取 SPI Flash ID */
	FlashID = SPI_FLASH_ReadID();

#if (FLASH_PRINTF_EN)
    printf("FlashID is 0x%X,  Device ID is 0x%X\r\n", FlashID, DeviceID);
#endif

	/* 检验 SPI Flash ID */
	if (FlashID == sFLASH_ID)
    {
        //读取出当前已经存储的数据包个数
        SPI_FLASH_BufferRead(page_num, FLASH_PACKAGE_NUM_ADDRESS, sizeof(page_num));
        g_DataPageNum = page_num[0] * 256 + page_num[1];

#if (FLASH_PRINTF_EN)
        if((g_DataPageNum >= SENSOR_DATA_MIN_PAGE_NUM) && (g_DataPageNum < 0xFFFF))
        {
            printf("有%d个存储包待发送\r\n", (g_DataPageNum - SENSOR_DATA_MIN_PAGE_NUM));

        }
#endif
        return SUCCEED;
    }
    else
    {
        return FAILURE;
    }
}

//********************************************************
//函数名称: Data_Storge_Process
//函数功能: 数据存储处理函数
//输    入: u8* data——数据内容, u16 len——数据长度
//输    出: 无
//备    注:
//1、第一个扇区前2个字节用于存放本次存储数据的页码；
//2、第二个扇区开始，每页存放一组未发送的数据包；
//3、每个扇区有16页，总共有4096个字节，每次存储一页；
//4、先把整个扇区读取出来，然后把对应要写入的页的数据更新到扇区数组内，整个扇区再写入；
//5、如果存储到了45天，则重新从最早存储地址开始覆盖。
//********************************************************
void Data_Storge_Process(u8* data, u16 len)
{
    for(u8 temp_index = 0; temp_index < 50; temp_index++)  //测试用！加快50倍测试写入
    {
        u8  page_num[2];

        SPI_FLASH_BufferRead(page_num, FLASH_PACKAGE_NUM_ADDRESS, sizeof(page_num));
        g_DataPageNum = page_num[0] * 256 + page_num[1];
        if((g_DataPageNum == 0xFFFF) || (g_DataPageNum == 0x0000)) //说明还没有存储过
        {
            g_DataPageNum = SENSOR_DATA_MIN_PAGE_NUM;
        }
        else                        //否则是之前已经存储过了
        {
            g_DataPageNum++;
            if(g_DataPageNum >= SENSOR_DATA_MAX_PAGE_NUM)       //注意：此处必须是">="
            {
                g_DataPageNum = SENSOR_DATA_MIN_PAGE_NUM;
            }
        }

#if (FLASH_PRINTF_EN)
        printf("\r\n本次写入的页码是%d\r\n", g_DataPageNum);
#endif

        //保存本次写入数据的页码
        page_num[0] = (u8)(g_DataPageNum >> 8);
        page_num[1] = (u8)(g_DataPageNum >> 0);
        SPI_FLASH_SectorErase(FLASH_PACKAGE_NUM_ADDRESS);
        SPI_FLASH_BufferWrite(page_num, FLASH_PACKAGE_NUM_ADDRESS, sizeof(page_num));

        //读出当前扇区的所有数据
        SPI_FLASH_BufferRead(g_PublicDataBuffer, ((g_DataPageNum / SPI_FLASH_PerSectorPage) * SPI_FLASH_PerSectorSize), SPI_FLASH_PerSectorSize);

        //将要存储的数据拷贝到扇区数组对应的位置
        memcpy(&g_PublicDataBuffer[(g_DataPageNum % SPI_FLASH_PerSectorPage) * SPI_FLASH_PageSize], data, len);

        //擦除当前要写入页对应的扇区
        SPI_FLASH_SectorErase((g_DataPageNum / SPI_FLASH_PerSectorPage) * SPI_FLASH_PerSectorSize); //地址必须是每个扇区的第一个地址

        //将当前扇区的数据写入
        SPI_FLASH_BufferWrite(g_PublicDataBuffer, ((g_DataPageNum / SPI_FLASH_PerSectorPage) * SPI_FLASH_PerSectorSize), SPI_FLASH_PerSectorSize);

//        Delay_ms(100);
    }
}

//********************************************************
//函数名称: Ext_Flash_Test
//函数功能: 片外Flash测试函数
//输    入: 无
//输    出: 无
//备    注: 无
//********************************************************
void Ext_Flash_Test(void)
{
    u8  Tx_Buffer[] = {
0x5B, 0xF8, 0x1F, 0x19,
0xFF, 0xFF, 0xFF, 0xFF,
0x05, 0xF1,
0x0E,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x03, 0x00, 0x00, 0x59, 0x0B, 0x00, 0x13, 0x53, 0x63, 0x00, 0x0B, 0x68, 0x8B,
0x04, 0xFF, 0xFF, 0xD4, 0x9E, 0xFF, 0xFF, 0x92, 0xED, 0x00, 0x00, 0x79, 0x8B,
0x05, 0xFF, 0xFF, 0xE7, 0x5D, 0xFF, 0xFF, 0x15, 0xFA, 0x00, 0x01, 0x10, 0x48,
0x06, 0x00, 0x02, 0x63, 0x99, 0x00, 0x03, 0x9B, 0x13, 0x00, 0x00, 0x0B, 0xF3,
0x07, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x60, 0x00, 0x00, 0x08, 0x16, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x61, 0x00, 0x00, 0x1A, 0x24, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x62, 0x00, 0x00, 0x4E, 0x84, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x63, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x64, 0x00, 0x00, 0x08, 0x8E, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x65, 0x00, 0x00, 0x18, 0x1A, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x66, 0x00, 0x01, 0x8D, 0xB2, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
    };

    u8  Rx_Buffer[256];

    s_GPSInfo.gmtTime = 0x5BF81F17;
    Ext_Flash_Detect();     //检测片外flash是否存在
    while(9)
    {
        s_GPSInfo.gmtTime += 2;
        Tx_Buffer[0] = (u8)(s_GPSInfo.gmtTime >> 24);
        Tx_Buffer[1] = (u8)(s_GPSInfo.gmtTime >> 16);
        Tx_Buffer[2] = (u8)(s_GPSInfo.gmtTime >> 8);
        Tx_Buffer[3] = (u8)(s_GPSInfo.gmtTime >> 0);

#if (FLASH_PRINTF_EN)
        printf("\r\n写入的数据为：\r\n");
        for(u16 i = 0; i < sizeof(Tx_Buffer); i++)
        {
            printf("%02X ", Tx_Buffer[i]);
        }
#endif

        Data_Storge_Process(Tx_Buffer, sizeof(Tx_Buffer));

        /* 将刚刚写入的数据读出来放到接收缓冲区中 */
        SPI_FLASH_BufferRead(Rx_Buffer, (g_DataPageNum * 256), sizeof(Tx_Buffer));

#if (FLASH_PRINTF_EN)
        printf("\r\n读出的数据为：\r\n");
        for(u16 i = 0; i < sizeof(Tx_Buffer); i++)
        {
            printf("%02X ", Rx_Buffer[i]);
        }
#endif

        Delay_ms(2000);
    }
}


/*********************************************END OF FILE**********************/
