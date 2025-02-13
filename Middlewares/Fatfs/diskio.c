/*-----------------------------------------------------------------------*/
/* Low level disk I/O module SKELETON for FatFs     (C)ChaN, 2019        */
/*-----------------------------------------------------------------------*/
/* If a working storage control module is available, it should be        */
/* attached to the FatFs via a glue function rather than modifying it.   */
/* This is an example of glue functions to attach various exsisting      */
/* storage control modules to the FatFs module with a defined API.       */
/*-----------------------------------------------------------------------*/

#include "ff.h"		/* Obtains integer types */
#include "diskio.h" /* Declarations of disk functions */
#include "sdio/sd_app.h"
#include "periph.h"
#include "FreeRTOS.h"
#include "semphr.h"
#include <time.h>

/* define in User/Src/tasks.c */
extern SemaphoreHandle_t SemaphoreSdioWithUSBAndFatfs;

/* Definitions of physical drive number for each drive */
// #define DEV_RAM		0	/* Example: Map Ramdisk to physical drive 0 */
// #define DEV_MMC		1	/* Example: Map MMC/SD card to physical drive 1 */
// #define DEV_USB		2	/* Example: Map USB MSD to physical drive 2 */
#define DEV_SDCard 0

/*-----------------------------------------------------------------------*/
/* Get Drive Status                                                      */
/*-----------------------------------------------------------------------*/

DSTATUS disk_status(
	BYTE pdrv /* Physical drive nmuber to identify the drive */
)
{
	return RES_OK;
}

/*-----------------------------------------------------------------------*/
/* Inidialize a Drive                                                    */
/*-----------------------------------------------------------------------*/

DSTATUS disk_initialize(
	BYTE pdrv /* Physical drive nmuber to identify the drive */
)
{
	if (pdrv == DEV_SDCard)
	{
		if (sdio_init() == 0)
		{
			rtc_init();
			return RES_OK;
		}
		else
			return STA_NOINIT;
	}
	return STA_NODISK;
}

/*-----------------------------------------------------------------------*/
/* Read Sector(s)                                                        */
/*-----------------------------------------------------------------------*/

DRESULT disk_read(
	BYTE pdrv,	  /* Physical drive nmuber to identify the drive */
	BYTE *buff,	  /* Data buffer to store read data */
	LBA_t sector, /* Start sector in LBA */
	UINT count	  /* Number of sectors to read */
)
{
	DRESULT res = RES_PARERR;

	if (pdrv == DEV_SDCard)
	{
		if (xSemaphoreTake(SemaphoreSdioWithUSBAndFatfs, (TickType_t)1000) == pdTRUE)
		{
			if (SD_Read_Disk((uint8_t *)buff, sector, count) == 0)
				res = RES_OK;
			else
				res = RES_ERROR;

			if (xSemaphoreGive(SemaphoreSdioWithUSBAndFatfs) != pdTRUE)
			{
				printf("xSemaphoreGive(SemaphoreSdioWithUSBAndFatfs) diskio.c\r\n");
			}
		}
		else
		{
			res = RES_NOTRDY;
		}
	}

	return res;
}

/*-----------------------------------------------------------------------*/
/* Write Sector(s)                                                       */
/*-----------------------------------------------------------------------*/

#if FF_FS_READONLY == 0

DRESULT disk_write(
	BYTE pdrv,		  /* Physical drive nmuber to identify the drive */
	const BYTE *buff, /* Data to be written */
	LBA_t sector,	  /* Start sector in LBA */
	UINT count		  /* Number of sectors to write */
)
{
	DRESULT res = RES_PARERR;

	if (pdrv == DEV_SDCard)
	{
		if (xSemaphoreTake(SemaphoreSdioWithUSBAndFatfs, (TickType_t)1000) == pdTRUE)
		{
			if (SD_Write_Disk((uint8_t *)buff, sector, count) == 0)
				res = RES_OK;
			else
				res = RES_ERROR;

			if (xSemaphoreGive(SemaphoreSdioWithUSBAndFatfs) != pdTRUE)
			{
				printf("xSemaphoreGive(SemaphoreSdioWithUSBAndFatfs) diskio.c\r\n");
			}
		}
		else
		{
			res = RES_NOTRDY;
		}
	}

	return res;
}

#endif

/*-----------------------------------------------------------------------*/
/* Miscellaneous Functions                                               */
/*-----------------------------------------------------------------------*/

DRESULT disk_ioctl(
	BYTE pdrv, /* Physical drive nmuber (0..) */
	BYTE cmd,  /* Control code */
	void *buff /* Buffer to send/receive control data */
)
{
	DRESULT res;

	if (pdrv == DEV_SDCard)
	{
		switch (cmd)
		{
		case CTRL_SYNC:
			res = RES_OK;
			break;

		case GET_SECTOR_SIZE:
			*(DWORD *)buff = g_sdio1.SdCard.BlockSize;
			res = RES_OK;
			break;

		case GET_BLOCK_SIZE:
			*(WORD *)buff = g_sdio1.SdCard.LogBlockSize;
			res = RES_OK;
			break;

		case GET_SECTOR_COUNT:
			*(DWORD *)buff = g_sdio1.SdCard.LogBlockNbr;
			res = RES_OK;
			break;

		default:
			res = RES_PARERR;
			break;
		}
		return res;
	}

	return RES_PARERR;
}

DWORD get_fattime(void)
{
	RTC_TimeTypeDef time;
	RTC_DateTypeDef date;

	HAL_CHECK(HAL_RTC_GetTime(&g_rtc, &time, RTC_FORMAT_BIN));
	HAL_CHECK(HAL_RTC_GetDate(&g_rtc, &date, RTC_FORMAT_BIN));
	// 编码 FAT 时间格式
	return ((DWORD)(date.Year + 2000 - 60) << 25) // 年份：从 1980 年开始
		   | ((DWORD)(date.Month) << 21)   // 月份：tm_mon 是从 0 开始的，需要加 1
		   | ((DWORD)date.Date << 16)	   // 日期
		   | ((DWORD)time.Hours << 11)	   // 小时
		   | ((DWORD)time.Minutes << 5)	   // 分钟
		   | ((DWORD)(time.Seconds / 2));  // FAT 格式中，秒以2秒为单位
}