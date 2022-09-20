#include "bps_exti.h"


static void NVIC_Config(void)//�ú���ֻ��������ļ����ݲ��ܱ����������ļ����ò���
{
	
	//����KEY1�ж�����
	NVIC_InitTypeDef NVIC_InitStruct;//���ñ���
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_4);//����Ϊ��1�������ȼ�Ϊ1bit �����ȼ�Ϊ3bit��
	NVIC_InitStruct.NVIC_IRQChannel=EXTI0_IRQn;//����������Դ
	NVIC_InitStruct.NVIC_IRQChannelPreemptionPriority=5;
	NVIC_InitStruct.NVIC_IRQChannelSubPriority=0;
	NVIC_InitStruct.NVIC_IRQChannelCmd=ENABLE;
	NVIC_Init(&NVIC_InitStruct);
	
	//����KEY2�ж�����
	NVIC_InitStruct.NVIC_IRQChannel=EXTI15_10_IRQn;//����������Դ
	NVIC_Init(&NVIC_InitStruct);
}

void EXTI_KEY1_Config(void)
{
	EXTI_InitTypeDef EXTI_InitStruct;
	GPIO_InitTypeDef GPIO_InitStruct;
	
	//�����ж����ȼ�
	NVIC_Config();
	
	//��ʼ��ʹ��GPIOA PB0Ϊ����״̬
	RCC_APB2PeriphClockCmd(KEY1_INT_CLK,ENABLE);
	GPIO_InitStruct.GPIO_Pin=KEY1_INT_PIN;
	GPIO_InitStruct.GPIO_Mode=GPIO_Mode_IN_FLOATING;
	GPIO_Init(KEY1_INT_PORT, &GPIO_InitStruct);
	//��ʼ��EXTI����ѡ��ģʽ ������ʽ ����
	RCC_APB2PeriphResetCmd(RCC_APB2Periph_AFIO,ENABLE);//��AFIOʱ��
	GPIO_EXTILineConfig(GPIO_PortSourceGPIOA,GPIO_PinSource0);
	
	EXTI_DeInit();//�ָ�Ĭ��״̬
	EXTI_InitStruct.EXTI_Line=EXTI_Line0;
	EXTI_InitStruct.EXTI_Mode=EXTI_Mode_Interrupt;
	EXTI_InitStruct.EXTI_Trigger=EXTI_Trigger_Rising;
	EXTI_InitStruct.EXTI_LineCmd=ENABLE;
	EXTI_Init(&EXTI_InitStruct);
}




void KEY1_KEY2_EXITConfig(void)
{
	EXTI_InitTypeDef EXTI_InitStruct;
	GPIO_InitTypeDef GPIO_InitStruct;
	
	//��ʼ��ʹ��GPIOA PB0Ϊ����״̬
	RCC_APB2PeriphClockCmd(KEY1_INT_CLK,ENABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO,ENABLE);//��AFIOʱ��
	//�����ж����ȼ�
	NVIC_Config();
	
	/************************KEY1����*******************/
	
	GPIO_InitStruct.GPIO_Pin=KEY1_INT_PIN;
	GPIO_InitStruct.GPIO_Mode=GPIO_Mode_IN_FLOATING;
	GPIO_Init(KEY1_INT_PORT, &GPIO_InitStruct);
	//��ʼ��EXTI����ѡ��ģʽ ������ʽ ����
	GPIO_EXTILineConfig(GPIO_PortSourceGPIOA,GPIO_PinSource0);
	
	EXTI_InitStruct.EXTI_Line=EXTI_Line0;
	EXTI_InitStruct.EXTI_Mode=EXTI_Mode_Interrupt;
	EXTI_InitStruct.EXTI_Trigger=EXTI_Trigger_Rising;
	EXTI_InitStruct.EXTI_LineCmd=ENABLE;
	EXTI_Init(&EXTI_InitStruct);
	
	/************************KEY2����*******************/
	GPIO_InitStruct.GPIO_Pin=KEY2_INT_PIN;
	GPIO_InitStruct.GPIO_Mode=GPIO_Mode_IN_FLOATING;
	GPIO_Init(KEY1_INT_PORT, &GPIO_InitStruct);
	
	//��ʼ��EXTI����ѡ��ģʽ ������ʽ ����
	GPIO_EXTILineConfig(GPIO_PortSourceGPIOC,GPIO_PinSource13);
	
	EXTI_InitStruct.EXTI_Line=EXTI_Line13;
	EXTI_InitStruct.EXTI_Mode=EXTI_Mode_Interrupt;
	EXTI_InitStruct.EXTI_Trigger=EXTI_Trigger_Rising;
	EXTI_InitStruct.EXTI_LineCmd=ENABLE;
	EXTI_Init(&EXTI_InitStruct);
	
}