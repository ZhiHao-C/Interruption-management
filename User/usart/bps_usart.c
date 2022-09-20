#include "bps_usart.h"
#include <stdio.h>

/* FreeRTOSͷ�ļ� */
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"



char Usart_Rx_Buf[USART_RBUFF_SIZE];

static void NVIC_Configuration(void)
{
  NVIC_InitTypeDef NVIC_InitStructure;
  
  /* Ƕ�������жϿ�������ѡ�� */
  NVIC_PriorityGroupConfig(NVIC_PriorityGroup_4);
  
  /* ����USARTΪ�ж�Դ */
  NVIC_InitStructure.NVIC_IRQChannel = DEBUG_USART_IRQ;
  /* �������ȼ�*/
  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 7;
  /* �����ȼ� */
  NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
  /* ʹ���ж� */
  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
  /* ��ʼ������NVIC */
  NVIC_Init(&NVIC_InitStructure);
}







void USART_Config(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	USART_InitTypeDef USART_InitStructure;

	// �򿪴���GPIO��ʱ��
	DEBUG_USART_GPIO_APBxClkCmd(DEBUG_USART_GPIO_CLK, ENABLE);
	
	// �򿪴��������ʱ��
	DEBUG_USART_APBxClkCmd(DEBUG_USART_CLK, ENABLE);

	// ��USART Tx��GPIO����Ϊ���츴��ģʽ
	GPIO_InitStructure.GPIO_Pin = DEBUG_USART_TX_GPIO_PIN;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(DEBUG_USART_TX_GPIO_PORT, &GPIO_InitStructure);

  // ��USART Rx��GPIO����Ϊ��������ģʽ
	GPIO_InitStructure.GPIO_Pin = DEBUG_USART_RX_GPIO_PIN;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
	GPIO_Init(DEBUG_USART_RX_GPIO_PORT, &GPIO_InitStructure);
	
	// ���ô��ڵĹ�������
	// ���ò�����
	USART_InitStructure.USART_BaudRate = DEBUG_USART_BAUDRATE;
	// ���� �������ֳ�
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;
	// ����ֹͣλ
	USART_InitStructure.USART_StopBits = USART_StopBits_1;
	// ����У��λ
	USART_InitStructure.USART_Parity = USART_Parity_No ;
	// ����Ӳ��������
	USART_InitStructure.USART_HardwareFlowControl = 
	USART_HardwareFlowControl_None;
	// ���ù���ģʽ���շ�һ��
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
	// ��ɴ��ڵĳ�ʼ������
	USART_Init(DEBUG_USARTx, &USART_InitStructure);

	// �����ж����ȼ�����
	NVIC_Configuration();
	
	// ���� ���ڿ���IDEL �ж�
	USART_ITConfig(DEBUG_USARTx, USART_IT_IDLE, ENABLE); 	
	
	//ʹ��DMA
	USART_DMACmd(DEBUG_USARTx,USART_DMAReq_Rx,ENABLE);
	
	// ʹ�ܴ���
	USART_Cmd(DEBUG_USARTx, ENABLE);	    
}





void USARTx_DMA_Config(void)
{
	
	DMA_InitTypeDef DMA_InitStruct;
	//����DMAʱ��
	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1,ENABLE);
	// ����DMAԴ��ַ���������ݼĴ�����ַ*/
	DMA_InitStruct.DMA_PeripheralBaseAddr=(uint32_t)USART_DR_ADDRESS;//�������ַ�ʽ(uint32_t)(0x40013800+0x04);//��M���͵�P
	// �ڴ��ַ(Ҫ����ı�����ָ��)
	DMA_InitStruct.DMA_MemoryBaseAddr = (uint32_t)Usart_Rx_Buf;
	// �������赽�洢��
	DMA_InitStruct.DMA_DIR= DMA_DIR_PeripheralSRC ;
	//�����С
	DMA_InitStruct.DMA_BufferSize=USART_RBUFF_SIZE;
	// �����ַ����	    
	DMA_InitStruct.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
	// �ڴ��ַ����
	DMA_InitStruct.DMA_MemoryInc = DMA_MemoryInc_Enable;
	// �������ݵ�λ	
	DMA_InitStruct.DMA_PeripheralDataSize=DMA_PeripheralDataSize_Byte;
	// �ڴ����ݵ�λ
	DMA_InitStruct.DMA_MemoryDataSize=DMA_MemoryDataSize_Byte;
	// DMAģʽ��һ�λ���ѭ��ģʽ
	DMA_InitStruct.DMA_Mode=DMA_Mode_Circular;
	// ���ȼ�����
	DMA_InitStruct.DMA_Priority=DMA_Priority_VeryHigh;
	// ��ֹ�ڴ浽�ڴ�Ĵ���
	DMA_InitStruct.DMA_M2M=DMA_M2M_Disable;
	// ����DMAͨ��	
	DMA_Init(USART_RX_DMA_CHANNEL,&DMA_InitStruct);
	// ���DMA���б�־//���Է��ֲ���Ҫ
//	DMA_ClearFlag(DMA1_FLAG_GL5);
//	DMA_ITConfig(USART_RX_DMA_CHANNEL, DMA_IT_TE, ENABLE);
	// ʹ��DMA
	DMA_Cmd (USART_RX_DMA_CHANNEL,ENABLE);
}

extern SemaphoreHandle_t BinarySem_Handle;

void Uart_DMA_Rx_Data(void)
{
	BaseType_t pxHigherPriorityTaskWoken;
	// �ر�DMA ����ֹ����
	DMA_Cmd(USART_RX_DMA_CHANNEL, DISABLE);      
	// ��DMA��־λ
//	DMA_ClearFlag( DMA1_FLAG_TC5 );          
	//  ���¸�ֵ����ֵ��������ڵ��������ܽ��յ�������֡��Ŀ
	USART_RX_DMA_CHANNEL->CNDTR = USART_RBUFF_SIZE;    
	DMA_Cmd(USART_RX_DMA_CHANNEL, ENABLE);       
	/* 
	xSemaphoreGiveFromISR(SemaphoreHandle_t xSemaphore,
												BaseType_t *pxHigherPriorityTaskWoken);
	*/
	
	//������ֵ�ź��� �����ͽ��յ������ݱ�־����ǰ̨�����ѯ
	xSemaphoreGiveFromISR(BinarySem_Handle,&pxHigherPriorityTaskWoken);	//�ͷŶ�ֵ�ź���
  //�����Ҫ�Ļ�����һ�������л���ϵͳ���ж��Ƿ���Ҫ�����л�
	portYIELD_FROM_ISR(pxHigherPriorityTaskWoken);

}






//����һ���ֽ�
void USART_Sendbyte(USART_TypeDef* USARTx,uint8_t Data)
{
	USART_SendData(USARTx,Data);
	while (!USART_GetFlagStatus(USARTx,USART_FLAG_TXE));//�жϷ������ݼĴ����Ƿ����ݷ��͵�������λ�Ĵ���
}
//���������ֽ�
void USART_Send_twobyte(USART_TypeDef* USARTx,uint16_t Data)
{
	uint8_t temp_h,temp_l;
	temp_h=Data>>8;
	temp_l=Data;
	USART_SendData(USARTx,temp_h);
	while (!USART_GetFlagStatus(USARTx,USART_FLAG_TXE));
	USART_SendData(USARTx,temp_l);
	while (!USART_GetFlagStatus(USARTx,USART_FLAG_TXE));
}
//����һ������
void USART_Sendarr(USART_TypeDef* USARTx,uint8_t*arr,uint8_t num)
{
	uint8_t i;
	for(i=0;i<num;i++)
	{
		USART_SendData(USARTx,arr[i]);
		while (!USART_GetFlagStatus(USARTx,USART_FLAG_TXE));
	}
}
//����һ���ַ��������������ģ�
void USART_Sendstr(USART_TypeDef* USARTx,uint8_t *str)
{
	while ((*str)!=0)
	{
		USART_SendData(USARTx,*str);
		str++;
		while (!USART_GetFlagStatus(USARTx,USART_FLAG_TXE));
	}
}

int fputc(int ch, FILE *f)
{
		/* ����һ���ֽ����ݵ����� */
		USART_SendData(DEBUG_USARTx, (uint8_t) ch);
		
		/* �ȴ�������� */
		while (USART_GetFlagStatus(DEBUG_USARTx, USART_FLAG_TXE) == RESET);		
	
		return (ch);
}


///�ض���c�⺯��scanf�����ڣ���д����ʹ��scanf��getchar�Ⱥ���
int fgetc(FILE *f)
{
		/* �ȴ������������� */
		while (USART_GetFlagStatus(DEBUG_USARTx, USART_FLAG_RXNE) == RESET);

		return (int)USART_ReceiveData(DEBUG_USARTx);
}

