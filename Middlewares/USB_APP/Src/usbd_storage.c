/**
 ******************************************************************************
 * @file    USB_Device/MSC_Standalone/Src/usbd_storage.c
 * @author  MCD Application Team
 * @brief   Memory management layer
 ******************************************************************************
 * @attention
 *
 * Copyright (c) 2017 STMicroelectronics.
 * All rights reserved.
 *
 * This software is licensed under terms that can be found in the LICENSE file
 * in the root directory of this software component.
 * If no LICENSE file comes with this software, it is provided AS-IS.
 *
 ******************************************************************************
 */
/* Includes ------------------------------------------------------------------*/
#include "usbd_storage.h"
#include "sdio/sd_app.h"
#include "periph.h"
#include "FreeRTOS.h"
#include "semphr.h"

/* define in User/Src/tasks.c */
extern SemaphoreHandle_t SemaphoreSdioWithUSBAndFatfs;

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
#define STORAGE_LUN_NBR 1
#define STORAGE_BLK_NBR 0x10000
#define STORAGE_BLK_SIZ 0x200

/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
__IO uint32_t writestatus, readstatus = 0;
/* USB Mass storage Standard Inquiry Data */
int8_t STORAGE_Inquirydata[] = {
	/* 36 */
	/* LUN 0 */
	0x00,
	0x80,
	0x02,
	0x02,
	(STANDARD_INQUIRY_DATA_LEN - 5),
	0x00,
	0x00,
	0x00,
	'S', 'T', 'M', ' ', ' ', ' ', ' ', ' ', /* Manufacturer: 8 bytes  */
	'P', 'r', 'o', 'd', 'u', 'c', 't', ' ', /* Product     : 16 Bytes */
	' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ',
	'0', '.', '0', '1', /* Version     : 4 Bytes  */
};

/* Private function prototypes -----------------------------------------------*/
int8_t STORAGE_Init(uint8_t lun);
int8_t STORAGE_GetCapacity(uint8_t lun, uint32_t *block_num, uint16_t *block_size);
int8_t STORAGE_IsReady(uint8_t lun);
int8_t STORAGE_IsWriteProtected(uint8_t lun);
int8_t STORAGE_Read(uint8_t lun, uint8_t *buf, uint32_t blk_addr, uint16_t blk_len);
int8_t STORAGE_Write(uint8_t lun, uint8_t *buf, uint32_t blk_addr, uint16_t blk_len);
int8_t STORAGE_GetMaxLun(void);

USBD_StorageTypeDef USBD_DISK_fops = {
	STORAGE_Init,
	STORAGE_GetCapacity,
	STORAGE_IsReady,
	STORAGE_IsWriteProtected,
	STORAGE_Read,
	STORAGE_Write,
	STORAGE_GetMaxLun,
	STORAGE_Inquirydata,
};
/* Private functions ---------------------------------------------------------*/

/**
 * @brief  Initializes the storage unit (medium)
 * @param  lun: Logical unit number
 * @retval Status (0 : Ok / -1 : Error)
 */
int8_t STORAGE_Init(uint8_t lun)
{
	static uint8_t init_sta = 0;
	if (init_sta == 1)
		return 0;
	if (sdio_init() == 0 && init_sta == 0)
	{
		init_sta = 1;
		return 0;
	}
	else
		return -1;
}

/**
 * @brief  Returns the medium capacity.
 * @param  lun: Logical unit number
 * @param  block_num: Number of total block number
 * @param  block_size: Block size
 * @retval Status (0: Ok / -1: Error)
 */
int8_t STORAGE_GetCapacity(uint8_t lun, uint32_t *block_num, uint16_t *block_size)
{
	int8_t ret = -1;

	*block_num = g_sdio1.SdCard.BlockNbr;
	*block_size = g_sdio1.SdCard.BlockSize;

	ret = 0;
	return ret;
}

/**
 * @brief  Checks whether the medium is ready.
 * @param  lun: Logical unit number
 * @retval Status (0: Ok / -1: Error)
 */
int8_t STORAGE_IsReady(uint8_t lun)
{
	return 0;
}

/**
 * @brief  Checks whether the medium is write protected.
 * @param  lun: Logical unit number
 * @retval Status (0: write enabled / -1: otherwise)
 */
int8_t STORAGE_IsWriteProtected(uint8_t lun)
{
	return 0;
}

/**
 * @brief  Reads data from the medium.
 * @param  lun: Logical unit number
 * @param  blk_addr: Logical block address
 * @param  blk_len: Blocks number
 * @retval Status (0: Ok / -1: Error)
 */
int8_t STORAGE_Read(uint8_t lun, uint8_t *buf, uint32_t blk_addr, uint16_t blk_len)
{
	int8_t ret = -1;
	if (xSemaphoreTakeFromISR(SemaphoreSdioWithUSBAndFatfs, NULL) == pdTRUE)
	{
		if (SD_Read_Disk(buf, blk_addr, blk_len) == 0)
		{
			ret = 0;
		}
		if( xSemaphoreGiveFromISR(SemaphoreSdioWithUSBAndFatfs, NULL) != pdTRUE )
		{
			printf("STORAGE_Read -> xSemaphoreGiveFromISR\r\n");
		}
	}

	return ret;
}
/**
 * @brief  Writes data into the medium.
 * @param  lun: Logical unit number
 * @param  blk_addr: Logical block address
 * @param  blk_len: Blocks number
 * @retval Status (0 : Ok / -1 : Error)
 */
int8_t STORAGE_Write(uint8_t lun, uint8_t *buf, uint32_t blk_addr, uint16_t blk_len)
{
	int8_t ret = -1;
	
	if (xSemaphoreTakeFromISR(SemaphoreSdioWithUSBAndFatfs, NULL) == pdTRUE)
	{
		if (SD_Write_Disk(buf, blk_addr, blk_len) == 0)
		{
			ret = 0;
		}
		if( xSemaphoreGiveFromISR(SemaphoreSdioWithUSBAndFatfs, NULL) != pdTRUE )
		{
			printf("STORAGE_Read -> xSemaphoreGiveFromISR\r\n");
		}
	}

	return ret;
}

/**
 * @brief  Returns the Max Supported LUNs.
 * @param  None
 * @retval Lun(s) number
 */
int8_t STORAGE_GetMaxLun(void)
{
	return (STORAGE_LUN_NBR - 1);
}
