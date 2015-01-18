/**
  ******************************************************************************
  * @file    Audio_playback_and_record/src/stm32f4xx_it.c 
  * @author  MCD Application Team
  * @version V1.0.0
  * @date    28-October-2011
  * @brief   Main Interrupt Service Routines.
  *          This file provides template for all exceptions handler and 
  *          peripherals interrupt service routine.
  ******************************************************************************
  * @attention
  *
  * THE PRESENT FIRMWARE WHICH IS FOR GUIDANCE ONLY AIMS AT PROVIDING CUSTOMERS
  * WITH CODING INFORMATION REGARDING THEIR PRODUCTS IN ORDER FOR THEM TO SAVE
  * TIME. AS A RESULT, STMICROELECTRONICS SHALL NOT BE HELD LIABLE FOR ANY
  * DIRECT, INDIRECT OR CONSEQUENTIAL DAMAGES WITH RESPECT TO ANY CLAIMS ARISING
  * FROM THE CONTENT OF SUCH FIRMWARE AND/OR THE USE MADE BY CUSTOMERS OF THE
  * CODING INFORMATION CONTAINED HEREIN IN CONNECTION WITH THEIR PRODUCTS.
  *
  * <h2><center>&copy; COPYRIGHT 2011 STMicroelectronics</center></h2>
  ******************************************************************************
  */ 

/* Includes ------------------------------------------------------------------*/
#include <led.h>
#include <usb_core.h>
#include <usbh_core.h>
#include <ff.h>
#include "stm32f4_discovery_lis302dl.h"
#include "stm32f4_discovery_audio_codec.h"

/** @addtogroup STM32F4-Discovery_Audio_Player_Recorder
  * @{
  */

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
__IO uint8_t PauseResumeStatus = 2, Count = 0, LED_Toggle1 = 0;
uint16_t capture = 0;
extern __IO uint16_t CCR_Val;
extern __IO uint8_t RepeatState, AudioPlayStart;
extern uint8_t Buffer[];

#if defined MEDIA_USB_KEY
__IO uint16_t Time_Rec_Base = 0;
 extern USB_OTG_CORE_HANDLE          USB_OTG_Core;
 extern USBH_HOST                    USB_Host;
 extern FIL file;
 extern __IO uint8_t Data_Status;
 extern __IO uint32_t XferCplt ;
 extern __IO uint8_t Command_index;
#endif /* MEDIA_USB_KEY */

/**
  * @brief  This function handles External line 1 interrupt request.
  * @param  None
  * @retval None
  */
void EXTI1_IRQHandler(void)
{
  /* Check the clic on the accelerometer to Pause/Resume Playing */
  if(EXTI_GetITStatus(EXTI_Line1) != RESET)
  {
    if( Count==1)
    {
      PauseResumeStatus = 1;
      Count = 0;
    }
    else
    {
      PauseResumeStatus = 0;
      Count = 1;
    }
    /* Clear the EXTI line 1 pending bit */
    EXTI_ClearITPendingBit(EXTI_Line1);
  }
}

/**
  * @brief  This function handles TIM4 global interrupt request.
  * @param  None
  * @retval None
  */
void TIM4_IRQHandler(void)
{
   uint8_t clickreg = 0;

  if (AudioPlayStart != 0x00)
  {
    /* Read click status register */
    LIS302DL_Read(&clickreg, LIS302DL_CLICK_SRC_REG_ADDR, 1); 
    LIS302DL_Read(Buffer, LIS302DL_STATUS_REG_ADDR, 6);
  }
  
  /* Checks whether the TIM interrupt has occurred */
  if (TIM_GetITStatus(TIM4, TIM_IT_CC1) != RESET)
  {
    TIM_ClearITPendingBit(TIM4, TIM_IT_CC1);
    if( LED_Toggle1 == 3)
    {
      /* LED3 Orange toggling */
      LED_Toggle(LED1);
      LED_ChangeState(LED3, LED_OFF);
      LED_ChangeState(LED0, LED_OFF);
    }
    else if( LED_Toggle1 == 4)
    {
      /* LED4 Green toggling */
      LED_Toggle(LED0);
      LED_ChangeState(LED3, LED_OFF);
      LED_ChangeState(LED1, LED_OFF);
    }
    else if( LED_Toggle1 == 6)
    {
      /* LED6 Blue toggling */
      LED_ChangeState(LED1, LED_OFF);
      LED_ChangeState(LED0, LED_OFF);
      LED_Toggle(LED3);
    }
    else if (LED_Toggle1 ==0)
    {
      /* LED6 Blue On to signal Pause */
      LED_ChangeState(LED3, LED_ON);
    }
    else if (LED_Toggle1 == 7)
    {
      /* LED4 toggling with frequency = 439.4 Hz */
      LED_ChangeState(LED0, LED_OFF);
      LED_ChangeState(LED1, LED_OFF);
      LED_ChangeState(LED2, LED_OFF);
      LED_ChangeState(LED3, LED_OFF);
    }
    capture = TIM_GetCapture1(TIM4);
    TIM_SetCompare1(TIM4, capture + CCR_Val);
  }
}

#if defined MEDIA_USB_KEY
/**
  * @brief  EXTI0_IRQHandler
  *         This function handles External line 0 interrupt request.
  * @param  None
  * @retval None
  */
void EXTI0_IRQHandler(void)
{
  /* Checks whether the User Button EXTI line is asserted*/
  if (EXTI_GetITStatus(EXTI_Line0) != RESET) 
  { 
    if (Command_index == 1)
    {
      RepeatState = 0;
      /* Switch to play command */
      Command_index = 0;
    }
    else if (Command_index == 0)
    {
      /* Switch to record command */
      Command_index = 1;
      XferCplt = 1;
      EVAL_AUDIO_Stop(CODEC_PDWN_SW);
    }
    else
    {
      RepeatState = 0;
      /* Switch to play command */
      Command_index = 0; 
    }
  } 
  /* Clears the EXTI's line pending bit.*/ 
  EXTI_ClearITPendingBit(EXTI_Line0);
}


/**
  * @brief  This function handles TIM2 global interrupt request.
  * @param  None
  * @retval None
  */
void TIM2_IRQHandler(void)
{
  USB_OTG_BSP_TimerIRQ();
}


/**
  * @brief  This function handles USB-On-The-Go FS global interrupt request.
  * @param  None
  * @retval None
  */
void OTG_FS_IRQHandler(void)
{
  USBH_OTG_ISR_Handler(&USB_OTG_Core);
}
#endif /* MEDIA_USB_KEY */


/**
  * @}
  */ 
  
/******************* (C) COPYRIGHT 2011 STMicroelectronics *****END OF FILE****/
