/********************************** (C) COPYRIGHT *******************************
* File Name          : main.c
* Author             : Dennis
* Version            : V1.0.0
* Date               : 2026/06/03
* Description        : LED Blink example for CH32V307 Evaluation Board.
*********************************************************************************
* Based on WCH GPIO_Toggle example
* LED: PA0, Active Low (点亮电平：低电平)
*******************************************************************************/

#include "debug.h"

/* Global define */
#define LED_PIN        GPIO_Pin_0
#define LED_GPIO_PORT  GPIOA
#define LED_GPIO_CLK   RCC_APB2Periph_GPIOA
#define BLINK_DELAY    500  /* ms */

/* Global Variable */

/*********************************************************************
 * @fn      LED_Init
 *
 * @brief   Initializes LED GPIO (PA0, Push-Pull Output)
 *
 * @return  none
 */
void LED_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStructure = {0};

    RCC_APB2PeriphClockCmd(LED_GPIO_CLK, ENABLE);

    GPIO_InitStructure.GPIO_Pin = LED_PIN;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(LED_GPIO_PORT, &GPIO_InitStructure);

    /* 默认关闭LED（高电平 = 关闭，因为低电平点亮） */
    GPIO_SetBits(LED_GPIO_PORT, LED_PIN);
}

/*********************************************************************
 * @fn      LED_On
 *
 * @brief   Turn on LED (Output Low)
 *
 * @return  none
 */
void LED_On(void)
{
    GPIO_ResetBits(LED_GPIO_PORT, LED_PIN);
}

/*********************************************************************
 * @fn      LED_Off
 *
 * @brief   Turn off LED (Output High)
 *
 * @return  none
 */
void LED_Off(void)
{
    GPIO_SetBits(LED_GPIO_PORT, LED_PIN);
}

/*********************************************************************
 * @fn      LED_Toggle
 *
 * @brief   Toggle LED state
 *
 * @return  none
 */
void LED_Toggle(void)
{
    if (GPIO_ReadOutputDataBit(LED_GPIO_PORT, LED_PIN) == Bit_SET)
    {
        LED_On();
    }
    else
    {
        LED_Off();
    }
}

/*********************************************************************
 * @fn      main
 *
 * @brief   Main program.
 *
 * @return  none
 */
int main(void)
{
    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);
    SystemCoreClockUpdate();
    Delay_Init();
    USART_Printf_Init(115200);
    printf("SystemClk:%ld\r\n", SystemCoreClock);
    printf("ChipID:%08lx\r\n", DBGMCU_GetCHIPID());

    printf("LED Blink TEST\r\n");
    printf("LED: PA0, Active Low\r\n");
    LED_Init();

    while(1)
    {
        LED_Toggle();
        Delay_Ms(BLINK_DELAY);
    }
}
