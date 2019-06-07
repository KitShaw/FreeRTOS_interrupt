
#include "stm32f10x.h"
#include "misc.h"
#include "led.h"
#include "usart.h"
#include "stdio.h"
//#include "key.h"
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "list.h"
#include "delay.h"
#include "stm32f10x_tim.h"

#define START_TASK_PRIO		1		     //�������ȼ�
#define START_STK_SIZE 		256				//�����ջ��С
TaskHandle_t StartTask_Handler;			//������
void start_task(void *pvParameters);//������

#define INTERRUPT_TASK_PRIO		2      //�������ȼ�
#define INTERRUPT_STK_SIZE			256			//�����ջ��С
TaskHandle_t INTERRUPTTask_Handler;     //������
void interrupt_task(void *p_arg);       //������

void TIM3_Int_Init(u16 arr, u16 pac);
void TIM5_Int_Init(u16 arr, u16 pac);


int main(void)
{
	
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_4);  //�ж����ȼ����� ����4
	
	USART1_init(115200);
    delay_init(); 
    led_init();
	TIM3_Int_Init(1000-1, 7200-1);              //��ʼ����ʱ��3�� ��ʱ������1S
	TIM5_Int_Init(1000-1, 7200-1);              //��ʼ����ʱ��5�� ��ʱ������1S
	xTaskCreate((TaskFunction_t )start_task,    //������
							 (const char*   )"start_task",  //��������
							 (uint16_t      )START_STK_SIZE, //�����ջ��С
							 (void*         )NULL, 						//������������Ĳ���
							 (UBaseType_t   )START_TASK_PRIO, //�������ȼ�
							 (TaskHandle_t* )&StartTask_Handler); //������
	
    vTaskStartScheduler();        
    return 0;
	
}

void start_task(void *pvParameters)
//��ʼ����������
{	
	taskENTER_CRITICAL();           //�����ٽ���
	xTaskCreate((TaskFunction_t   )interrupt_task,
							(const char*      )"interrupt_task",
							(uint16_t         )INTERRUPT_STK_SIZE,
							(void*						)NULL,
							(UBaseType_t      )INTERRUPT_TASK_PRIO,
							(TaskHandle_t*    )&INTERRUPTTask_Handler);
							
	vTaskDelete(StartTask_Handler);   //ɾ����ʼ����
	taskEXIT_CRITICAL();
}

void interrupt_task(void *pvparameters)
{
	static u32 total_num = 0;
	while(1)
	{
		total_num+=1;
		if(total_num==5)
		{
			printf("�ر��ж�.....\r\n");
			portDISABLE_INTERRUPTS();
			delay_xms(500);
			printf("���ж�.....\r\n");
			portENABLE_INTERRUPTS();
			
		}
		vTaskDelay(100);
		
	}
}

//ͨ�ö�ʱ��3�жϳ�ʼ��
//����ʱ��ѡ��ΪAPB1��2���� ��APB1λ36M
//arr; �Զ���װ
//psc: ʱ��Ԥ��Ƶ��
//����ʹ�õ��Ƕ�ʱ��3
void TIM3_Int_Init(u16 arr, u16 psc)
{
	TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;
	NVIC_InitTypeDef NVIC_InitStructure;
	
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3, ENABLE); //ʱ��ʹ��
	TIM_TimeBaseStructure.TIM_Period = arr; //�Զ���װ��ֵ
	TIM_TimeBaseStructure.TIM_Prescaler = psc;
	TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1;
	TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
	TIM_TimeBaseInit(TIM3, &TIM_TimeBaseStructure);
	TIM_ITConfig(TIM3, TIM_IT_Update, ENABLE);
	
	NVIC_InitStructure.NVIC_IRQChannel = TIM3_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 5;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);
	
	TIM_Cmd(TIM3, ENABLE);
	//TIM_Cmd(TIM3, DISABLE);
}


//ͨ�ö�ʱ��3�жϳ�ʼ��
//����ʱ��ѡ��ΪAPB1��2���� ��APB1λ36M
//arr; �Զ���װ
//psc: ʱ��Ԥ��Ƶ��
//����ʹ�õ��Ƕ�ʱ��3
void TIM5_Int_Init(u16 arr, u16 psc)
{
	TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;
	NVIC_InitTypeDef NVIC_InitStructure;
	
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM5, ENABLE); //ʱ��ʹ��
	TIM_TimeBaseStructure.TIM_Period = arr; //�Զ���װ��ֵ
	TIM_TimeBaseStructure.TIM_Prescaler = psc;
	TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1;
	TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
	TIM_TimeBaseInit(TIM5, &TIM_TimeBaseStructure);
	TIM_ITConfig(TIM5, TIM_IT_Update, ENABLE);
	
	NVIC_InitStructure.NVIC_IRQChannel = TIM5_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 4;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);
	
	TIM_Cmd(TIM5, ENABLE);
}


void TIM3_IRQHandler(void)
{
	if(TIM_GetITStatus(TIM3, TIM_IT_Update)==SET)
	{
		printf("TIM3���......\r\n");
		TIM_ClearITPendingBit(TIM3, TIM_IT_Update); 
	}
}

void TIM5_IRQHandler(void)
{
	if(TIM_GetITStatus(TIM5, TIM_IT_Update) == SET)
	{
		printf("TIM5 ���......\r\n");
	}
	TIM_ClearITPendingBit(TIM5,TIM_IT_Update);
		
}


