//#include "stm32f10x.h"                  // Device header
#include "string.h"


#include "bps_led.h"
#include "bps_usart.h"
#include "key.h"
#include "bps_exti.h"

#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"

#define KEY1_IRQHandler            EXTI0_IRQHandler
#define KEY2_IRQHandler            EXTI15_10_IRQHandler

#define  QUEUE_LEN    4   /* ���еĳ��ȣ����ɰ������ٸ���Ϣ */
#define  QUEUE_SIZE   4   /* ������ÿ����Ϣ��С���ֽڣ� */
/**************************** ȫ�ֱ��� ********************************/

extern char Usart_Rx_Buf[USART_RBUFF_SIZE];
static uint32_t send_data1 = 1;
static uint32_t send_data2 = 2;

/**************************** ������ ********************************/
/* 
 * ��������һ��ָ�룬����ָ��һ�����񣬵����񴴽���֮�����;�����һ��������
 * �Ժ�����Ҫ��������������Ҫͨ�������������������������������Լ�����ô
 * ����������ΪNULL��
 */
 /* ���������� */
static TaskHandle_t AppTaskCreate_Handle = NULL;
static TaskHandle_t LED_Task_Handle = NULL;/*LED_Task ������ */
static TaskHandle_t KEY_Task_Handle = NULL;/* KEY_Task ������ */

//���о��
QueueHandle_t Test_Queue =NULL;
//��ֵ�ź������
SemaphoreHandle_t BinarySem_Handle;





//��������
static void LED_Task(void* parameter);
static void KEY_Task(void* parameter);
static void AppTaskCreate(void);

static void BSP_Init(void)
{
	/* 
	* STM32 �ж����ȼ�����Ϊ 4���� 4bit ��������ʾ��ռ���ȼ�����ΧΪ��0~15 
	* ���ȼ�����ֻ��Ҫ����һ�μ��ɣ��Ժ������������������Ҫ�õ��жϣ� 
	* ��ͳһ��������ȼ����飬ǧ��Ҫ�ٷ��飬�мɡ� 
	*/ 
	NVIC_PriorityGroupConfig( NVIC_PriorityGroup_4 ); 
	LED_GPIO_Config();
	USARTx_DMA_Config();
	USART_Config();
	KEY1_GPIO_Config();
	KEY2_GPIO_Config();
	
	KEY1_KEY2_EXITConfig();
	
	
	//����
//	led_G(on);
//	printf("���ڲ���");
}

int main()
{
	BaseType_t xReturn = NULL;/* ����һ��������Ϣ����ֵ��Ĭ��ΪpdPASS */
	
	BSP_Init();
	printf("����һ��[Ұ��]-STM32 ȫϵ�п�����-FreeRTOS ����֪ͨ������Ϣ����ʵ�飡\n");
	printf("����KEY1 | KEY2�����жϣ�\n");
  printf("���ڷ������ݴ����ж�,����������!\n");

	

	
	  /* ����AppTaskCreate���� */
  xReturn = xTaskCreate((TaskFunction_t )AppTaskCreate,  /* ������ں��� */
                        (const char*    )"AppTaskCreate",/* �������� */
                        (uint16_t       )512,  /* ����ջ��С */
                        (void*          )NULL,/* ������ں������� */
                        (UBaseType_t    )1, /* ��������ȼ� */
                        (TaskHandle_t*  )&AppTaskCreate_Handle);/* ������ƿ�ָ�� */ 
																							
	if(xReturn==pdPASS)
	{
		printf("��ʼ���񴴽��ɹ�\r\n");
		vTaskStartScheduler();
	}
	else 
	{
		return -1;
	}
	while(1)
	{
		
	}

}



static void LED_Task(void* parameter)
{
	BaseType_t xReturn=pdFALSE;
	uint32_t r_queue=0; 
	while(1)
	{
		xReturn = xQueueReceive( Test_Queue,    /* ��Ϣ���еľ�� */
												     &r_queue,      /* ���յ�����Ϣ���� */
												     portMAX_DELAY); /* �ȴ�ʱ�� һֱ�� */
		if(pdPASS == xReturn)
		{
			printf("�����жϵ��� KEY%d !\n",r_queue);
		}
//		else
//		{
//			printf("���ݽ��ճ���\n");
//		}
		
    LED_G_TOGGLE();
	}
}


static void KEY_Task(void* parameter)
{
	BaseType_t xReturn=pdFALSE;
	while(1)
	{
		//��ȡһ����ֵ�ź���
		xReturn=xSemaphoreTake(BinarySem_Handle,portMAX_DELAY);
		if(xReturn==pdTRUE)
		{
			LED_R_TOGGLE();
      printf("�յ�����:%s\n",Usart_Rx_Buf);
      memset(Usart_Rx_Buf,0,USART_RBUFF_SIZE);/* ���� */
		}
	}    
}



static void AppTaskCreate(void)
{
	BaseType_t xReturn = NULL;/* ����һ��������Ϣ����ֵ��Ĭ��ΪpdPASS */
	
	taskENTER_CRITICAL();           //�����ٽ���
	
	//����һ������
  Test_Queue = xQueueCreate((UBaseType_t ) QUEUE_LEN,/* ��Ϣ���еĳ��� */
                            (UBaseType_t ) QUEUE_SIZE);/* ��Ϣ�Ĵ�С */
	if(Test_Queue!=NULL)
	{
		printf("��Ϣ���д����ɹ�\n");
	}
	
	BinarySem_Handle=xSemaphoreCreateBinary();
	if(BinarySem_Handle!=NULL)
	{
			printf("��ֵ�ź��������ɹ�\n");
	}
	
	
	xReturn=xTaskCreate((TaskFunction_t	)LED_Task,		//������
															(const char* 	)"LED_Task",		//��������
															(uint16_t 		)512,	//�����ջ��С
															(void* 		  	)NULL,				//���ݸ��������Ĳ���
															(UBaseType_t 	)2, 	//�������ȼ�
															(TaskHandle_t*  )&LED_Task_Handle);/* ������ƿ�ָ�� */ 
															
															
	if(xReturn == pdPASS)/* �����ɹ� */
		printf("LED_Task���񴴽��ɹ�!\n");
	else
		printf("LED_Task���񴴽�ʧ��!\n");
	

	xReturn=xTaskCreate((TaskFunction_t	)KEY_Task,		//������
															(const char* 	)"KEY_Task",		//��������
															(uint16_t 		)512,	//�����ջ��С
															(void* 		  	)NULL,				//���ݸ��������Ĳ���
															(UBaseType_t 	)3, 	//�������ȼ�
															(TaskHandle_t*  )&KEY_Task_Handle);/* ������ƿ�ָ�� */ 
															
	if(xReturn == pdPASS)/* �����ɹ� */
		printf("KEY_Task���񴴽��ɹ�!\n");
	else
		printf("KEY_Task���񴴽�ʧ��!\n");
	
	
	
	vTaskDelete(AppTaskCreate_Handle); //ɾ��AppTaskCreate����
	
	taskEXIT_CRITICAL();            //�˳��ٽ���
}




/**************************** �жϺ��� ********************************/

void KEY1_IRQHandler(void)
{
	uint32_t ulReturn=0;
	BaseType_t pxHigherPriorityTaskWoken=pdFALSE;
	//�����ٽ��
	ulReturn = taskENTER_CRITICAL_FROM_ISR();
	if(EXTI_GetFlagStatus(EXTI_Line0)==SET)
	{
		/* ������д�루���ͣ��������У��ȴ�ʱ��Ϊ 0  */
		xQueueSendFromISR(Test_Queue, /* ��Ϣ���еľ�� */
											&send_data1,/* ���͵���Ϣ���� */
											&pxHigherPriorityTaskWoken);
		
		//�����Ҫ�Ļ�����һ�������л�
		portYIELD_FROM_ISR(pxHigherPriorityTaskWoken);
		//����жϱ�־λ
		EXTI_ClearITPendingBit(EXTI_Line0); 
	}
	 
	/* �˳��ٽ�� */
	taskEXIT_CRITICAL_FROM_ISR( ulReturn );
}

void KEY2_IRQHandler(void)
{
	uint32_t ulReturn=0;
	BaseType_t pxHigherPriorityTaskWoken=pdFALSE;
	//�����ٽ��
	 ulReturn = taskENTER_CRITICAL_FROM_ISR();
	
	if(EXTI_GetFlagStatus(EXTI_Line13)==SET)
	{
		/* ������д�루���ͣ��������У��ȴ�ʱ��Ϊ 0  */
		xQueueSendFromISR(Test_Queue, /* ��Ϣ���еľ�� */
											&send_data2,/* ���͵���Ϣ���� */
											&pxHigherPriorityTaskWoken);
		
		//�����Ҫ�Ļ�����һ�������л�
		portYIELD_FROM_ISR(pxHigherPriorityTaskWoken);
		//����жϱ�־λ
		EXTI_ClearITPendingBit(EXTI_Line13); 
	}
	 
	/* �˳��ٽ�� */
	taskEXIT_CRITICAL_FROM_ISR( ulReturn );
}


void USART1_IRQHandler(void)
{
	  uint32_t ulReturn;
  /* �����ٽ�Σ��ٽ�ο���Ƕ�� */
  ulReturn = taskENTER_CRITICAL_FROM_ISR();

	if(USART_GetITStatus(DEBUG_USARTx,USART_IT_IDLE)!=RESET)
	{		
		Uart_DMA_Rx_Data();       /* �ͷ�һ���ź�������ʾ�����ѽ��� */
		USART_ReceiveData(DEBUG_USARTx); /* �����־λ */
	}	 
  
  /* �˳��ٽ�� */
  taskEXIT_CRITICAL_FROM_ISR( ulReturn );
}