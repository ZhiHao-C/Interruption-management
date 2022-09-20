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

#define  QUEUE_LEN    4   /* 队列的长度，最大可包含多少个消息 */
#define  QUEUE_SIZE   4   /* 队列中每个消息大小（字节） */
/**************************** 全局变量 ********************************/

extern char Usart_Rx_Buf[USART_RBUFF_SIZE];
static uint32_t send_data1 = 1;
static uint32_t send_data2 = 2;

/**************************** 任务句柄 ********************************/
/* 
 * 任务句柄是一个指针，用于指向一个任务，当任务创建好之后，它就具有了一个任务句柄
 * 以后我们要想操作这个任务都需要通过这个任务句柄，如果是自身的任务操作自己，那么
 * 这个句柄可以为NULL。
 */
 /* 创建任务句柄 */
static TaskHandle_t AppTaskCreate_Handle = NULL;
static TaskHandle_t LED_Task_Handle = NULL;/*LED_Task 任务句柄 */
static TaskHandle_t KEY_Task_Handle = NULL;/* KEY_Task 任务句柄 */

//队列句柄
QueueHandle_t Test_Queue =NULL;
//二值信号量句柄
SemaphoreHandle_t BinarySem_Handle;





//声明函数
static void LED_Task(void* parameter);
static void KEY_Task(void* parameter);
static void AppTaskCreate(void);

static void BSP_Init(void)
{
	/* 
	* STM32 中断优先级分组为 4，即 4bit 都用来表示抢占优先级，范围为：0~15 
	* 优先级分组只需要分组一次即可，以后如果有其他的任务需要用到中断， 
	* 都统一用这个优先级分组，千万不要再分组，切忌。 
	*/ 
	NVIC_PriorityGroupConfig( NVIC_PriorityGroup_4 ); 
	LED_GPIO_Config();
	USARTx_DMA_Config();
	USART_Config();
	KEY1_GPIO_Config();
	KEY2_GPIO_Config();
	
	KEY1_KEY2_EXITConfig();
	
	
	//测试
//	led_G(on);
//	printf("串口测试");
}

int main()
{
	BaseType_t xReturn = NULL;/* 定义一个创建信息返回值，默认为pdPASS */
	
	BSP_Init();
	printf("这是一个[野火]-STM32 全系列开发板-FreeRTOS 任务通知代替消息队列实验！\n");
	printf("按下KEY1 | KEY2触发中断！\n");
  printf("串口发送数据触发中断,任务处理数据!\n");

	

	
	  /* 创建AppTaskCreate任务 */
  xReturn = xTaskCreate((TaskFunction_t )AppTaskCreate,  /* 任务入口函数 */
                        (const char*    )"AppTaskCreate",/* 任务名字 */
                        (uint16_t       )512,  /* 任务栈大小 */
                        (void*          )NULL,/* 任务入口函数参数 */
                        (UBaseType_t    )1, /* 任务的优先级 */
                        (TaskHandle_t*  )&AppTaskCreate_Handle);/* 任务控制块指针 */ 
																							
	if(xReturn==pdPASS)
	{
		printf("初始任务创建成功\r\n");
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
		xReturn = xQueueReceive( Test_Queue,    /* 消息队列的句柄 */
												     &r_queue,      /* 接收到的消息内容 */
												     portMAX_DELAY); /* 等待时间 一直等 */
		if(pdPASS == xReturn)
		{
			printf("触发中断的是 KEY%d !\n",r_queue);
		}
//		else
//		{
//			printf("数据接收出错\n");
//		}
		
    LED_G_TOGGLE();
	}
}


static void KEY_Task(void* parameter)
{
	BaseType_t xReturn=pdFALSE;
	while(1)
	{
		//获取一个二值信号量
		xReturn=xSemaphoreTake(BinarySem_Handle,portMAX_DELAY);
		if(xReturn==pdTRUE)
		{
			LED_R_TOGGLE();
      printf("收到数据:%s\n",Usart_Rx_Buf);
      memset(Usart_Rx_Buf,0,USART_RBUFF_SIZE);/* 清零 */
		}
	}    
}



static void AppTaskCreate(void)
{
	BaseType_t xReturn = NULL;/* 定义一个创建信息返回值，默认为pdPASS */
	
	taskENTER_CRITICAL();           //进入临界区
	
	//创建一个队列
  Test_Queue = xQueueCreate((UBaseType_t ) QUEUE_LEN,/* 消息队列的长度 */
                            (UBaseType_t ) QUEUE_SIZE);/* 消息的大小 */
	if(Test_Queue!=NULL)
	{
		printf("消息队列创建成功\n");
	}
	
	BinarySem_Handle=xSemaphoreCreateBinary();
	if(BinarySem_Handle!=NULL)
	{
			printf("二值信号量创建成功\n");
	}
	
	
	xReturn=xTaskCreate((TaskFunction_t	)LED_Task,		//任务函数
															(const char* 	)"LED_Task",		//任务名称
															(uint16_t 		)512,	//任务堆栈大小
															(void* 		  	)NULL,				//传递给任务函数的参数
															(UBaseType_t 	)2, 	//任务优先级
															(TaskHandle_t*  )&LED_Task_Handle);/* 任务控制块指针 */ 
															
															
	if(xReturn == pdPASS)/* 创建成功 */
		printf("LED_Task任务创建成功!\n");
	else
		printf("LED_Task任务创建失败!\n");
	

	xReturn=xTaskCreate((TaskFunction_t	)KEY_Task,		//任务函数
															(const char* 	)"KEY_Task",		//任务名称
															(uint16_t 		)512,	//任务堆栈大小
															(void* 		  	)NULL,				//传递给任务函数的参数
															(UBaseType_t 	)3, 	//任务优先级
															(TaskHandle_t*  )&KEY_Task_Handle);/* 任务控制块指针 */ 
															
	if(xReturn == pdPASS)/* 创建成功 */
		printf("KEY_Task任务创建成功!\n");
	else
		printf("KEY_Task任务创建失败!\n");
	
	
	
	vTaskDelete(AppTaskCreate_Handle); //删除AppTaskCreate任务
	
	taskEXIT_CRITICAL();            //退出临界区
}




/**************************** 中断函数 ********************************/

void KEY1_IRQHandler(void)
{
	uint32_t ulReturn=0;
	BaseType_t pxHigherPriorityTaskWoken=pdFALSE;
	//进入临界段
	ulReturn = taskENTER_CRITICAL_FROM_ISR();
	if(EXTI_GetFlagStatus(EXTI_Line0)==SET)
	{
		/* 将数据写入（发送）到队列中，等待时间为 0  */
		xQueueSendFromISR(Test_Queue, /* 消息队列的句柄 */
											&send_data1,/* 发送的消息内容 */
											&pxHigherPriorityTaskWoken);
		
		//如果需要的话进行一次任务切换
		portYIELD_FROM_ISR(pxHigherPriorityTaskWoken);
		//清除中断标志位
		EXTI_ClearITPendingBit(EXTI_Line0); 
	}
	 
	/* 退出临界段 */
	taskEXIT_CRITICAL_FROM_ISR( ulReturn );
}

void KEY2_IRQHandler(void)
{
	uint32_t ulReturn=0;
	BaseType_t pxHigherPriorityTaskWoken=pdFALSE;
	//进入临界段
	 ulReturn = taskENTER_CRITICAL_FROM_ISR();
	
	if(EXTI_GetFlagStatus(EXTI_Line13)==SET)
	{
		/* 将数据写入（发送）到队列中，等待时间为 0  */
		xQueueSendFromISR(Test_Queue, /* 消息队列的句柄 */
											&send_data2,/* 发送的消息内容 */
											&pxHigherPriorityTaskWoken);
		
		//如果需要的话进行一次任务切换
		portYIELD_FROM_ISR(pxHigherPriorityTaskWoken);
		//清除中断标志位
		EXTI_ClearITPendingBit(EXTI_Line13); 
	}
	 
	/* 退出临界段 */
	taskEXIT_CRITICAL_FROM_ISR( ulReturn );
}


void USART1_IRQHandler(void)
{
	  uint32_t ulReturn;
  /* 进入临界段，临界段可以嵌套 */
  ulReturn = taskENTER_CRITICAL_FROM_ISR();

	if(USART_GetITStatus(DEBUG_USARTx,USART_IT_IDLE)!=RESET)
	{		
		Uart_DMA_Rx_Data();       /* 释放一个信号量，表示数据已接收 */
		USART_ReceiveData(DEBUG_USARTx); /* 清除标志位 */
	}	 
  
  /* 退出临界段 */
  taskEXIT_CRITICAL_FROM_ISR( ulReturn );
}