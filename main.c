#include "stm32f4xx_conf.h"
#include "stm32f4xx.h"
#include "stdio.h"

volatile uint32_t countQ=0; //vari�vel de contagem de bordas de subida na interrup��o externa
volatile uint8_t a; //vari�vel utilizada para o debounce na interrup��o externa
volatile uint8_t Rel=0; //n�vel do reservat�rio elevado


extern void USART_Config(void); //fun��o de configura��o do m�dulo USART - comunica��o seria ass�ncrona

void TimingDelay_Decrement(void); //Fun��o Delay
void Delay(__IO uint32_t nTime);
extern __IO uint32_t TimingDelay;

void GPIO_Config(void); //fun��o de configura��o entradas e sa�das
void Timer_Config(void); //fun��o de configura��o PWM utilizando o m�dulo timer
void Time_Interrupt_Config(void); //fun��o de configura��o de dura��o de tempo da interrup��o externa
void Ext_Config(void); //fun��o de configura��o da interrup��o externa



int main(void)
{

	setvbuf( stdout, 0, _IONBF, 0 );

	// Chamando as fun��es de configura��es dos m�dulos
	USART_Config();

	GPIO_Config();

	Timer_Config();

	Time_Interrupt_Config();

	Ext_Config();


	 if (SysTick_Config(SystemCoreClock / 1000))
	{
		 /* Capture error */
		while (1);
	}


	 uint8_t value=0; //valor de intensidade PWM
	 uint8_t x; //leitura de PA0 - bot�o do usu�rio
	 uint8_t y; //leitura de PA10 - n�vel do reservat�rio
	 uint8_t w; //verifica flag de overflow do perio de amostragem

	 int count =0; //variavel para debounce do circuito de pot�ncia
	 int countime =0; //vari�vel de contagem de tempo de opera��o da bomba de recircula��o
	 uint8_t flag =0; //flag para indicar estado da moto-bomba
	 float vazao =0; //c�lculo da vaz�o
	 int tempo = 0; //valor PWM de sa�da vinculado ao timer


    	while(1)  //loop infinito
    	{

    	w=TIM_GetFlagStatus(TIM9, TIM_FLAG_Update); //verifica flag de overflow

    	if(w==Bit_SET) //ocorre a cada 1s

    	    	{
    	    	NVIC_DisableIRQ(EXTI3_IRQn); //desabilita fun��o externa


    			vazao = countQ/1.44; //calcula valor de vaz�o
    			printf("%3.2f\r\n", vazao); //apresenta o valor na tela via comunica��o USART
    			printf(" L/min\r\n");
    			countQ = 0;

    			NVIC_EnableIRQ(EXTI3_IRQn); //habilita a fun��o externa
	    		TIM_ClearFlag(TIM9, TIM_FLAG_Update); //limpa flag de overflow


    	    	}

    		x = GPIO_ReadInputDataBit(GPIOA, GPIO_Pin_0);
    		y = GPIO_ReadInputDataBit(GPIOA, GPIO_Pin_10);

    		    if(y == Bit_RESET) //verifica n�vel do reservat�rio
    		    {
    		    	count++;
    		    }
    		    else
    		    	{
    		    	if (flag==0) //verifica estado da moto-bomba
    		    	{
    		    		count=0; //debounce - moto-bomba desligada
    				}
    				if (flag==1)
    				{
    					count = 74000; //debounce - moto-bomba ligada
    				}
    	}


    		    if(x == Bit_SET) //verifica se o bot�o do usu�rio foi pressionado
    		    {
    		    	countime=0;
    		    value++; //incrementa a sequencia dos poss�veis valores de intensidade PWM

    		    if (value<=6)
    			{
    				switch(value){ //seta o valor de PWM de acordo com o estado sequencial - 5 possibilidades


    				case 1:
    				//-- valor a ser mostrado no display de 7 segmentos
    				GPIO_SetBits(GPIOB, GPIO_Pin_10);
    				GPIO_ResetBits(GPIOB, GPIO_Pin_11 | GPIO_Pin_12);

    				tempo=3999; //valor PWM de sa�da vinculado ao timer

    				flag=1;
    				count = 74000;

    				break;


    				case 2:
    				//-- valor a ser mostrado no display de 7 segmentos
    				GPIO_SetBits(GPIOB, GPIO_Pin_11);
    				GPIO_ResetBits(GPIOB, GPIO_Pin_10 | GPIO_Pin_12);

    				tempo=5199; //valor PWM de sa�da vinculado ao timer

    				flag=1;
    				count = 74000;

    				break;

    				case 3:
    				//-- valor a ser mostrado no display de 7 segmentos
    				GPIO_SetBits(GPIOB, GPIO_Pin_10 | GPIO_Pin_11);
    				GPIO_ResetBits(GPIOB, GPIO_Pin_12);

    				tempo=6399; //valor PWM de sa�da vinculado ao timer

    				flag=1;
    				count = 74000;

    				break;


    				case 4:
    				//-- valor a ser mostrado no display de 7 segmentos
    				GPIO_SetBits(GPIOB, GPIO_Pin_12);
    				GPIO_ResetBits(GPIOB, GPIO_Pin_10 | GPIO_Pin_11);

    				tempo=7599; //valor PWM de sa�da vinculado ao timer

    				flag=1;
    				count = 74000;

    				break;


    				case 5:
    				//-- valor a ser mostrado no display de 7 segmentos
    				GPIO_SetBits(GPIOB, GPIO_Pin_10 | GPIO_Pin_12);
    				GPIO_ResetBits(GPIOB, GPIO_Pin_11);

    				tempo=9990; //valor PWM de sa�da vinculado ao timer

    				flag=1;
    				count = 74000;

    				break;

    				case 6:
    				//-- valor a ser mostrado no display de 7 segmentos
    				GPIO_ResetBits(GPIOB, GPIO_Pin_10 | GPIO_Pin_11 | GPIO_Pin_12);

    				tempo=0; //valor PWM de sa�da vinculado ao timer

    				flag=1;
    				count = 74000;

    				break;

    				}
    			}
    			}
    		    while(x == Bit_SET)
    		       { //debounce bot�o do usu�rio
    		       	 x = GPIO_ReadInputDataBit(GPIOA, GPIO_Pin_0);
    		       	 Delay(50);
    		       }

    		    	if (value>5)
    		    	{
    		    	value=0;
    		    	}

    		    	TIM_SetCompare4(TIM4, tempo); // seta o valor do registrador de compara��o do canal 4, conforme valor de PWM

    		    	y = GPIO_ReadInputDataBit(GPIOA, GPIO_Pin_10);

    		    	if((y == Bit_RESET)&&(count>75000)) //verifica se reservat�rio est� cheio
    		    	{									//levando em considera��o o valor de count
    		    		NVIC_DisableIRQ(EXTI3_IRQn); //desabilita fun��o externa

    		    	Rel=1;
    		    	count=0;
    		    	value=0;
    		    	tempo=0; //desliga moto-bomba de recalque
    		    	GPIO_ResetBits(GPIOB, GPIO_Pin_10 | GPIO_Pin_11 | GPIO_Pin_12);
    		    	flag=0;
    		    	Delay(50);
    			GPIO_SetBits(GPIOB, GPIO_Pin_13); //liga moto-bomba de recircula��o
    		    	}
    		    	if (Rel==1)
    		    	{

    		w=TIM_GetFlagStatus(TIM9, TIM_FLAG_Update); //verifica flag de overflow
    	    if(w==Bit_SET) //se flag setada(ocorre a cada 1s)
    		{
    	countime++; //contagem de tempo de opera��o da bomba de recircula��o
    		}

    	}
    	if (countime==115)
    	{
    	//ap�s 115s com o reservat�rio de succ��o cheio e o reservat�rio elevado vazio [...]
    	GPIO_ResetBits(GPIOB, GPIO_Pin_13); // [...]desliga bomba de recircula��o[...]
    	}
    	if (countime==118)//[...]+3s reabilita a interrup��o e religa a moto-bomba de recalque para case 2
    	{
    		NVIC_EnableIRQ(EXTI3_IRQn);


    	Rel=0;
    	countime=0;
    	value=2;
    	GPIO_SetBits(GPIOB, GPIO_Pin_11);
    	GPIO_ResetBits(GPIOB, GPIO_Pin_10 | GPIO_Pin_12);

    	tempo=5199;

    	flag=0;
    	count = 74000;
    	}

    	}

    	}



void Timer_Config(void)
{
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOD, ENABLE);
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM4, ENABLE);
	//---configura fun��o alternativa do pino pd15 - tipo alternate function - TIM4

		GPIO_InitTypeDef pino;

		pino.GPIO_Mode = GPIO_Mode_AF; //alternate function
		pino.GPIO_OType = GPIO_OType_PP;
		pino.GPIO_PuPd = GPIO_PuPd_NOPULL;
		pino.GPIO_Speed = GPIO_Speed_2MHz;
		pino.GPIO_Pin = GPIO_Pin_15;
		GPIO_Init (GPIOD, &pino);
		//--- associa o pino pd15 ao timer 4
		GPIO_PinAFConfig(GPIOD, GPIO_PinSource15, GPIO_AF_TIM4);
		//--- configura a unidade de base de tempo do timer 4
		TIM_TimeBaseInitTypeDef Tim_TimeBaseStructure;
		TIM_TimeBaseStructInit(&Tim_TimeBaseStructure);
		//configura freq. contagem: 2MHZ
		Tim_TimeBaseStructure.TIM_Prescaler = 42-1;//clock barramento APB1
		// programa timer para contar de 0 a 9999
		Tim_TimeBaseStructure.TIM_Period = 10000-1;//flag setada a cada 5ms
		TIM_TimeBaseInit (TIM4, &Tim_TimeBaseStructure);
		//--- configura as unidades de captura e compara�ao do timer 4
		//--- canal 4 (pd15) como pwm

		TIM_OCInitTypeDef TIM_OCInitStructure;
		TIM_OCStructInit(&TIM_OCInitStructure);
		TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_PWM1;
		TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable;
		TIM_OCInitStructure.TIM_Pulse = 0;
		TIM_OC4Init(TIM4, &TIM_OCInitStructure);

		//--- habilita o "preload register" - necessario para gera��o do PWM

		TIM_OC4PreloadConfig(TIM4, TIM_OCPreload_Enable);
		//--- Habilita o contador do timer 4
		TIM_Cmd(TIM4, ENABLE); //habilita o contador do TIM4

}

void Time_Interrupt_Config(void)
{
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_TIM9, ENABLE);

	TIM_TimeBaseInitTypeDef Tim_BaseStructure;
	TIM_TimeBaseStructInit(&Tim_BaseStructure);
	Tim_BaseStructure.TIM_Prescaler = 21000-1;
	Tim_BaseStructure.TIM_Period = 10000-1; //tempo habilita��o da interrup��o externa 1s
	TIM_TimeBaseInit(TIM9, &Tim_BaseStructure);
	TIM_Cmd(TIM9, ENABLE);
}

void Ext_Config(void)
{
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOC, ENABLE); // m�dulo GPIOC
	//---Configura��o do m�dulo de interrup��o exeterna referente a PC3
	GPIO_InitTypeDef pino;
	pino.GPIO_Mode = GPIO_Mode_IN;
	pino.GPIO_OType = GPIO_OType_PP;
	pino.GPIO_PuPd = GPIO_PuPd_DOWN; //GPIO Pull down
	pino.GPIO_Speed = GPIO_Speed_100MHz;
	pino.GPIO_Pin = GPIO_Pin_3;
	GPIO_Init(GPIOC, &pino);

	EXTI_InitTypeDef EXTI_InitStruct;
	NVIC_InitTypeDef NVIC_InitStruct;
	RCC_APB2PeriphClockCmd (RCC_APB2Periph_SYSCFG, ENABLE);
	SYSCFG_EXTILineConfig (EXTI_PortSourceGPIOC, EXTI_PinSource3);
	EXTI_InitStruct.EXTI_Line = EXTI_Line3;
	EXTI_InitStruct.EXTI_LineCmd = ENABLE;
	EXTI_InitStruct.EXTI_Mode = EXTI_Mode_Interrupt;
	EXTI_InitStruct.EXTI_Trigger = EXTI_Trigger_Rising; //borda de subida
	EXTI_Init(&EXTI_InitStruct);
	NVIC_InitStruct.NVIC_IRQChannel = EXTI3_IRQn;
	NVIC_InitStruct.NVIC_IRQChannelPreemptionPriority = 0x01; //prioridade da interrup��o
	NVIC_InitStruct.NVIC_IRQChannelSubPriority = 0x01;
	NVIC_InitStruct.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStruct);

}

void Delay(__IO uint32_t nTime)
{
	TimingDelay = nTime;
	while(TimingDelay != 0);
}

void TimingDelay_Decrement(void)
{
	if (TimingDelay != 0x00)
	{
		TimingDelay--;
	}
}


void GPIO_Config(void)
{
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE); // m�dulo GPIOA
	GPIO_InitTypeDef pino; //define estrutura para configura��o dos pinos

	// m�dulo GPIOA
	pino.GPIO_Mode = GPIO_Mode_IN;
	pino.GPIO_OType = GPIO_OType_PP;
	pino.GPIO_PuPd = GPIO_PuPd_NOPULL;
	pino.GPIO_Speed = GPIO_Speed_2MHz;	//seta freq. clock do m�dulo GPIO
	pino.GPIO_Pin = GPIO_Pin_0 | GPIO_Pin_10;  //seleciona pino 0 e pino 10 (sensor n�vel)
	GPIO_Init(GPIOA, &pino); // configura GPIOA com par�metros da estrutura
	//---
	// m�dulo GPIOB - 10, 11 e 12 = display; 13 = bomba recircula��o

	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB, ENABLE); // m�dulo GPIOB

	pino.GPIO_Mode = GPIO_Mode_OUT;
	pino.GPIO_OType = GPIO_OType_PP;
	pino.GPIO_PuPd = GPIO_PuPd_NOPULL;
	pino.GPIO_Speed = GPIO_Speed_2MHz;
	pino.GPIO_Pin = GPIO_Pin_10 | GPIO_Pin_11 | GPIO_Pin_12 | GPIO_Pin_13;
	GPIO_Init(GPIOB, &pino);

}

//--- Fun��o relacionada a interrrup��o externa de PC03
//--- chamada em caso ocorra uma borda de subida
void EXTI3_IRQHandler(void)
{

if(EXTI_GetITStatus(EXTI_Line3) != RESET)
{

		a = GPIO_ReadInputDataBit(GPIOC, GPIO_Pin_3);

		if(a==Bit_SET)
		{
		countQ++;

		}

		while ((a==Bit_SET)&&(Rel==0))
		{
			a = GPIO_ReadInputDataBit(GPIOC, GPIO_Pin_3);

			unsigned long t;
			for(t=0; t<5000; t++)
			{}

		}

    	EXTI_ClearITPendingBit(EXTI_Line3);

}
}
