/**
 * @file    main.c
 * @brief   LED test
 * @date    9 kwi 2014
 * @author  Michal Ksiezopolski
 *
 *
 * @verbatim
 * Copyright (c) 2014 Michal Ksiezopolski.
 * All rights reserved. This program and the
 * accompanying materials are made available
 * under the terms of the GNU Public License
 * v3.0 which accompanies this distribution,
 * and is available at
 * http://www.gnu.org/licenses/gpl.html
 * @endverbatim
 */

#include <stdio.h>
#include <string.h>
#include <timers.h>
#include <led.h>
#include <comm.h>
#include <keys.h>
#include <usb_core.h>
#include <usbh_core.h>
#include <usbh_msc_core.h>
#include <usbh_usr.h>
#include <stm32f4xx.h>

#define SYSTICK_FREQ 1000 ///< Frequency of the SysTick set at 1kHz.
#define COMM_BAUD_RATE 115200UL ///< Baud rate for communication with PC

void softTimerCallback(void);

#define DEBUG

#ifdef DEBUG
#define print(str, args...) printf(""str"%s",##args,"")
#define println(str, args...) printf("MAIN--> "str"%s",##args,"\r\n")
#else
#define print(str, args...) (void)0
#define println(str, args...) (void)0
#endif

#if defined MEDIA_USB_KEY
 USB_OTG_CORE_HANDLE          USB_OTG_Core;
 USBH_HOST                    USB_Host;
#endif

volatile uint8_t RepeatState = 0;
volatile uint16_t CCR_Val = 16826;
extern volatile uint8_t LED_Toggle1;

static void TIM_LED_Config(void);


/**
 * @brief Main
 * @return None
 */
int main(void) {

  COMM_Init(COMM_BAUD_RATE); // initialize communication with PC
  println("Starting program"); // Print a string to terminal

  TIMER_Init(SYSTICK_FREQ); // Initialize timer

  // Add a soft timer with callback running every 1000ms
  int8_t timerID = TIMER_AddSoftTimer(1000, softTimerCallback);
  TIMER_StartSoftTimer(timerID); // start the timer

  LED_Init(LED0); // Add an LED
  LED_Init(LED1); // Add an LED
  LED_Init(LED2); // Add an LED
  LED_Init(LED3); // Add an LED
  LED_Init(LED5); // Add nonexising LED for test
  LED_ChangeState(LED5, LED_ON);

  // Green Led On: start of application
  LED_ChangeState(LED0, LED_ON);

  KEYS_Init(); // Initialize matrix keyboard

  uint8_t buf[255]; // buffer for receiving commands from PC
  uint8_t len;      // length of command

  // test another way of measuring time delays
  uint32_t softTimer = TIMER_GetTime(); // get start time for delay


  /* Configure TIM4 Peripheral to manage LEDs lighting */
  TIM_LED_Config();

  /* Initialize the repeat status */
  RepeatState = 0;
  LED_Toggle1 = 7;

#if defined MEDIA_IntFLASH

  WavePlayBack(I2S_AudioFreq_48k);
  while (1);

#elif defined MEDIA_USB_KEY

  /* Initialize User Button */
  STM_EVAL_PBInit();

  /* Init Host Library */
  USBH_Init(&USB_OTG_Core, USB_OTG_FS_CORE_ID, &USB_Host, &USBH_MSC_cb, &USR_Callbacks);


  while (1) {

    USBH_Process(&USB_OTG_Core, &USB_Host);

    // test delay method
    if (TIMER_DelayTimer(1000, softTimer)) {
      LED_Toggle(LED3);
      softTimer = TIMER_GetTime(); // get start time for delay
    }

    // check for new frames from PC
    if (!COMM_GetFrame(buf, &len)) {
      println("Got frame of length %d: %s", (int)len, (char*)buf);

      // control LED0 from terminal
      if (!strcmp((char*)buf, ":LED 0 ON")) {
        LED_ChangeState(LED0, LED_ON);
      }
      if (!strcmp((char*)buf, ":LED 0 OFF")) {
        LED_ChangeState(LED0, LED_OFF);
      }
      if (!strcmp((char*)buf, ":LED 1 ON")) {
        LED_ChangeState(LED1, LED_ON);
      }
      if (!strcmp((char*)buf, ":LED 1 OFF")) {
        LED_ChangeState(LED1, LED_OFF);
      }
    }

    TIMER_SoftTimersUpdate(); // run timers
    KEYS_Update(); // run keyboard
  }
#else
  #error "Error - must define MEDIA_USB_KEY or MEDIA_IntFLASH"
#endif
}

/**
 * @brief Callback function called on every soft timer overflow
 */
void softTimerCallback(void) {

//  static uint8_t counter;
//
//  switch (counter % 3) {
//
//  case 0:
//    LED_ChangeState(LED1, LED_OFF);
//    LED_ChangeState(LED2, LED_OFF);
//    break;
//
//  case 1:
//    LED_ChangeState(LED1, LED_ON);
//    LED_ChangeState(LED2, LED_OFF);
//    break;
//
//  case 2:
//    LED_ChangeState(LED1, LED_OFF);
//    LED_ChangeState(LED2, LED_ON);
//    break;
//
//  }

//  println("Test string sent from STM32F4!!!"); // Print test string
//  counter++;
}

/**
  * @brief  Configures the TIM Peripheral for Led toggling.
  * @param  None
  * @retval None
  */
static void TIM_LED_Config(void)
{
  TIM_OCInitTypeDef  TIM_OCInitStructure;
  TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;
  NVIC_InitTypeDef NVIC_InitStructure;
  uint16_t prescalervalue = 0;

  /* TIM4 clock enable */
  RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM4, ENABLE);

  NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);

  /* Enable the TIM3 gloabal Interrupt */
  NVIC_InitStructure.NVIC_IRQChannel = TIM4_IRQn;
  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
  NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
  NVIC_Init(&NVIC_InitStructure);


  /* Compute the prescaler value */
  prescalervalue = (uint16_t) ((SystemCoreClock ) / 550000) - 1;

  /* Time base configuration */
  TIM_TimeBaseStructure.TIM_Period = 65535; // about 100 ms
  TIM_TimeBaseStructure.TIM_Prescaler = prescalervalue;
  TIM_TimeBaseStructure.TIM_ClockDivision = 0;
  TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
  TIM_TimeBaseInit(TIM4, &TIM_TimeBaseStructure);

  /* Enable TIM4 Preload register on ARR */
  TIM_ARRPreloadConfig(TIM4, ENABLE);

  /* TIM PWM1 Mode configuration: Channel */
  TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_Timing;
  TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable;
  TIM_OCInitStructure.TIM_Pulse = CCR_Val;
  TIM_OCInitStructure.TIM_OCPolarity = TIM_OCPolarity_High;

  /* Output Compare PWM1 Mode configuration: Channel2 */
  TIM_OC1Init(TIM4, &TIM_OCInitStructure);
  TIM_OC1PreloadConfig(TIM4, TIM_OCPreload_Disable);

  /* TIM Interrupts enable */
  TIM_ITConfig(TIM4, TIM_IT_CC1 , ENABLE);

  /* TIM4 enable counter */
  TIM_Cmd(TIM4, ENABLE);
}
