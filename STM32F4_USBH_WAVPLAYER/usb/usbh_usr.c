/**
  ******************************************************************************
  * @file    Audio_playback_and_record/src/usbh_usr.c
  * @author  MCD Application Team
  * @version V1.0.0
  * @date    28-October-2011
  * @brief   This file includes the usb host user callbacks
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

#include "usbh_usr.h"
#include <led.h>

#define DEBUG

#ifdef DEBUG
#define print(str, args...) printf(""str"%s",##args,"")
#define println(str, args...) printf("USB_USR--> "str"%s",##args,"\r\n")
#else
#define print(str, args...) (void)0
#define println(str, args...) (void)0
#endif


__IO uint8_t Command_index = 0;
/*  Points to the DEVICE_PROP structure of current device */
/*  The purpose of this register is to speed up the execution */
FATFS fatfs;
FIL file;
FIL fileR;
DIR dir;
FILINFO fno;

USBH_Usr_cb_TypeDef USR_Callbacks = {
  USBH_USR_Init,
  USBH_USR_DeInit,
  USBH_USR_DeviceAttached,
  USBH_USR_ResetDevice,
  USBH_USR_DeviceDisconnected,
  USBH_USR_OverCurrentDetected,
  USBH_USR_DeviceSpeedDetected,
  USBH_USR_Device_DescAvailable,
  USBH_USR_DeviceAddressAssigned,
  USBH_USR_Configuration_DescAvailable,
  USBH_USR_Manufacturer_String,
  USBH_USR_Product_String,
  USBH_USR_SerialNum_String,
  USBH_USR_EnumerationDone,
  USBH_USR_UserInput,
  USBH_USR_MSC_Application,
  USBH_USR_DeviceNotSupported,
  USBH_USR_UnrecoveredError
};


extern __IO uint8_t RepeatState ;
extern __IO uint8_t LED_Toggle1;
extern __IO uint32_t WaveDataLength ;
extern __IO uint16_t Time_Rec_Base;

static uint8_t USBH_USR_ApplicationState = USH_USR_FS_INIT;

/**
  * @brief  USBH_USR_MSC_Application
  * @param  None
  * @retval Staus
  */
int USBH_USR_MSC_Application(void) {

  switch (USBH_USR_ApplicationState) {
    case USH_USR_FS_INIT:

      /* Initialises the File System*/
      if (f_mount( 0, &fatfs ) != FR_OK ) {
        /* efs initialisation fails*/
        println("Disk init failed");
        return(-1);
      }

      /* Flash Disk is write protected */
      if (USBH_MSC_Param.MSWriteProtect == DISK_WRITE_PROTECTED) {
        while(1) {
          /* Red LED On */
          LED_ChangeState(LED2, LED_ON);
          println("Disk write protected");
        }
      }
      /* Go to menu */
      USBH_USR_ApplicationState = USH_USR_AUDIO;
      break;

    case USH_USR_AUDIO:

      /* Go to Audio menu */
      COMMAND_AudioExecuteApplication();

      // FIXME Why reinit all the time???
      /* Set user initialization flag */
      USBH_USR_ApplicationState = USH_USR_FS_INIT;
      break;

    default:
      break;
  }
  return(0);
}

/**
  * @brief  COMMAND_AudioExecuteApplication
  * @param  None
  * @retval None
  */
void COMMAND_AudioExecuteApplication(void) {
  /* Execute the command switch the command index */
  switch (Command_index) {
  /* Start Playing from USB Flash memory */
  case CMD_PLAY:
    if (RepeatState == 0)
      WavePlayerStart();
    break;
    /* Start Recording in USB Flash memory */
  case CMD_RECORD:
    RepeatState = 0;
    WaveRecorderUpdate();
    break;
  default:
    break;
  }
}
/**
* @brief  USBH_USR_Init
*         Displays the message on LCD for host lib initialization
* @param  None
* @retval None
*/
void USBH_USR_Init(void) {

  println("USB Host library started.");
}

/**
* @brief  USBH_USR_DeviceAttached
*         Displays the message on LCD on device attached
* @param  None
* @retval None
*/
void USBH_USR_DeviceAttached(void) {
  println("Device attached");
  RepeatState = 0;

  LED_Toggle1 = 7;
  /* Red LED off when device attached */
  LED_ChangeState(LED2, LED_OFF);
  /* Green LED on */
  LED_ChangeState(LED0, LED_ON);
  /* TIM Interrupts enable */
  TIM_ITConfig(TIM4, TIM_IT_CC1, ENABLE);
}

/**
* @brief  USBH_USR_UnrecoveredError
* @param  None
* @retval None
*/
void USBH_USR_UnrecoveredError (void) {
  println("Unrecovered error");
}

/**
* @brief  USBH_DisconnectEvent
*         Device disconnect event
* @param  None
* @retval Staus
*/
void USBH_USR_DeviceDisconnected (void) {
  println("Device disconnected");
  /* Red Led on if the USB Key is removed */
  LED_ChangeState(LED2, LED_ON);
  /* Disable the Timer */
  TIM_ITConfig(TIM4, TIM_IT_CC1 , DISABLE);

  /* If USB key Removed when playing a wave */
  if( (WaveDataLength!=0)&& (Command_index != 1))
  {
    WavePlayer_CallBack();
    Command_index = 0;
  }

  /* If USB key Removed when recording a wave */
  if(Command_index == 1)
  {
    WaveRecorderStop();
    LED_ChangeState(LED1, LED_OFF);
    Command_index = 1;
    Time_Rec_Base = 0;
    LED_Toggle1 = 7;
  }
}
/**
* @brief  USBH_USR_ResetUSBDevice
* @param  None
* @retval None
*/
void USBH_USR_ResetDevice(void) {
  println("Device reset");
}

/**
* @brief  USBH_USR_DeviceSpeedDetected
*         Displays the message on LCD for device speed
* @param  Device speed
* @retval None
*/
void USBH_USR_DeviceSpeedDetected(uint8_t DeviceSpeed) {
  if(DeviceSpeed == HPRT0_PRTSPD_HIGH_SPEED) {
    println("High speed device");
  } else if(DeviceSpeed == HPRT0_PRTSPD_FULL_SPEED) {
    println("Full speed device");
  } else if(DeviceSpeed == HPRT0_PRTSPD_LOW_SPEED) {
    println("Low speed device");
  } else {
    println("Error in speed device");
  }
}

/**
* @brief  USBH_USR_Device_DescAvailable
*         Displays the message on LCD for device descriptor
* @param  device descriptor
* @retval None
*/
void USBH_USR_Device_DescAvailable(void *DeviceDesc) {

  USBH_DevDesc_TypeDef *hs;
  hs = DeviceDesc;

  println("VID : %04xh" , (unsigned int)((*hs).idVendor));
  println("PID : %04xh" , (unsigned int)((*hs).idProduct));
}

/**
* @brief  USBH_USR_DeviceAddressAssigned
*         USB device is successfully assigned the Address
* @param  None
* @retval None
*/
void USBH_USR_DeviceAddressAssigned(void) {
  println("Device addressed");
}


/**
* @brief  USBH_USR_Conf_Desc
*         Displays the message on LCD for configuration descriptor
* @param  Configuration descriptor
* @retval None
*/
void USBH_USR_Configuration_DescAvailable(USBH_CfgDesc_TypeDef * cfgDesc,
                                          USBH_InterfaceDesc_TypeDef *itfDesc,
                                          USBH_EpDesc_TypeDef *epDesc) {
  USBH_InterfaceDesc_TypeDef *id;

  id = itfDesc;

  if((*id).bInterfaceClass  == 0x08) {
    println("MSC device");
  } else if((*id).bInterfaceClass  == 0x03) {
    println("HID device");
  }
}

/**
* @brief  USBH_USR_Manufacturer_String
*         Displays the message on LCD for Manufacturer String
* @param  Manufacturer String
* @retval None
*/
void USBH_USR_Manufacturer_String(void *ManufacturerString) {
  println("Manufacturer : %s", (char *)ManufacturerString);
}

/**
* @brief  USBH_USR_Product_String
*         Displays the message on LCD for Product String
* @param  Product String
* @retval None
*/
void USBH_USR_Product_String(void *ProductString) {
  println("Product : %s", (char *)ProductString);
}

/**
* @brief  USBH_USR_SerialNum_String
*         Displays the message on LCD for SerialNum_String
* @param  SerialNum_String
* @retval None
*/
void USBH_USR_SerialNum_String(void *SerialNumString) {
  println( "Serial Number : %s", (char *)SerialNumString);
}

/**
* @brief  EnumerationDone
*         User response request is displayed to ask application jump to class
* @param  None
* @retval None
*/
void USBH_USR_EnumerationDone(void) {
  println("Enumeration done");
  /* 0.5 seconds delay */
  USB_OTG_BSP_mDelay(500);

  USBH_USR_MSC_Application();

}

/**
* @brief  USBH_USR_DeviceNotSupported
*         Device is not supported
* @param  None
* @retval None
*/
void USBH_USR_DeviceNotSupported(void) {
  println ("Device not supported");
}


/**
* @brief  USBH_USR_UserInput
*         User Action for application state entry
* @param  None
* @retval USBH_USR_Status : User response for key button
*/
USBH_USR_Status USBH_USR_UserInput(void) {
  USBH_USR_Status usbh_usr_status;

  usbh_usr_status = USBH_USR_NO_RESP;

  /*Key B3 is in polling mode to detect user action */
//  if(STM_EVAL_PBGetState(Button_KEY) == RESET)
//  {

    usbh_usr_status = USBH_USR_RESP_OK;

//  }
  return usbh_usr_status;
}

/**
* @brief  USBH_USR_OverCurrentDetected
*         Over Current Detected on VBUS
* @param  None
* @retval Staus
*/
void USBH_USR_OverCurrentDetected (void) {
  println("Overcurrent detected.");
}

/**
* @brief  USBH_USR_DeInit
*         Deint User state and associated variables
* @param  None
* @retval None
*/
void USBH_USR_DeInit(void) {
  println("Deinit");
  USBH_USR_ApplicationState = USH_USR_FS_INIT;
}
