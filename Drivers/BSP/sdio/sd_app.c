//
// Created by ShunX on 2023/12/31.
//
#include "sd_app.h"

/** sd_dma trans flag */
#define SD_RW_TIMEOUT 0xffffffff
#define SD_DMA_TRANS_MAX ((uint32_t)60 * 1024)
#define TESTDATA ((uint8_t)0xaa)
#define SDIO_BLOCK_SIZE 512UL

__IO uint8_t sd_tx_cplt = 1;
__IO uint8_t sd_rx_cplt = 1;
__IO uint8_t sd_rw_error = 0;

#if (SDIO_RW_TEST)
#define SD_TEST_BUFF_SIZE (uint32_t)1024 * 50
uint8_t __ALIGNED(4) sd_buff[SD_TEST_BUFF_SIZE] = {0};
#endif

#if SDIO_USE_DMA
#define SDIO_DMA_ALIGN4_BUF_SIZE (32 * 1024UL)
#define SDIO_DMA_ALIGN4_BUF_BLOCKS (SDIO_DMA_ALIGN4_BUF_SIZE / SDIO_BLOCK_SIZE)
#ifdef LINKSCRIP_EXSRAM
static uint8_t align4_buf[SDIO_DMA_ALIGN4_BUF_SIZE] __ALIGNED(4) __ATTR_EXSRAM;
#else
static uint8_t align4_buf[SDIO_DMA_ALIGN4_BUF_SIZE] __ALIGNED(4);
#endif
#endif

/**
 *   @brief 检测SD卡
 *   @retval 0:存在 -1：不存在
 *   @attention 检测引脚初始化时，需要下拉输入，读出高电平为检测到MSD卡
 */
int8_t SD_Detect(void)
{
	//   if(HAL_GPIO_ReadPin(SD_Detect_GPIO_Port, SD_Detect_Pin) != GPIO_PIN_SET)
	//   {
	//     return -1;
	//   } else
	return 0;
}
/**
 *  @brief 读sd卡
 *  @param pBuff: 数据地址
 *  @param Block_addr: 块地址（一般每块512B）
 *  @param Blocks_Nbr：块数量
 *  @retval 0:成功
 */
int8_t SD_Read_Disk(uint8_t *pBuff, uint32_t Block_addr, uint32_t Blocks_Nbr)
{
	uint32_t timeout = SD_RW_TIMEOUT;
	while (HAL_SD_GetCardState(&SD_HANDLE) != HAL_SD_CARD_TRANSFER)
	{
		timeout--;
		if (timeout == 0)
		{
			return 1;
		}
	}
	sd_rx_cplt = 0;
#if !SDIO_USE_DMA
	if (HAL_SD_ReadBlocks_IT(&SD_HANDLE, pBuff, Block_addr, Blocks_Nbr) != HAL_OK)
		return 2;
	while (sd_rx_cplt != 1 && sd_rw_error != 1)
	{
	}
	if (sd_rw_error)
	{
		sd_rw_error = 0;
		printf("sd error\r\n");
		return -1;
	}
	else
		return 0;
#else
	if ((uint32_t)pBuff % 4)
	{
		// SYS_LOG("SD Read", "NO Align pBuff");
		{ /* dma trans need align 4 byte */
			int8_t error = 0;
			uint16_t times = Blocks_Nbr / SDIO_DMA_ALIGN4_BUF_BLOCKS;
			uint16_t remain_blocks = Blocks_Nbr % SDIO_DMA_ALIGN4_BUF_BLOCKS;
			uint16_t trans_blocks;
			for (uint16_t i = 0; i < times + 1; i++)
			{
				if (i != times)
					trans_blocks = SDIO_DMA_ALIGN4_BUF_BLOCKS;
				else
					trans_blocks = remain_blocks;

				/* read to align buf : blocks*/
				if (HAL_SD_ReadBlocks_DMA(&SD_HANDLE, align4_buf, Block_addr + i * SDIO_DMA_ALIGN4_BUF_BLOCKS, trans_blocks) != HAL_OK)
				{
					SYS_ERR("HAL_SD_ReadBlocks_DMA");
					error = 2;
					break;
				}
				/* check trans complete */
				while (sd_rx_cplt != 1 && sd_rw_error != 1)
				{
				}
				if (sd_rw_error)
				{
					sd_rw_error = 0;
					SYS_ERR("sd_rw_error");
					error = 1;
					break;
				}

				/* copy aligned buf to pbuf: bytes*/
				memcpy(pBuff + i * (SDIO_DMA_ALIGN4_BUF_SIZE), align4_buf, trans_blocks * SDIO_BLOCK_SIZE);
			}
			return error;
		}
	}
	else
	{
		if (HAL_SD_ReadBlocks_DMA(&SD_HANDLE, pBuff, Block_addr, Blocks_Nbr) != HAL_OK)
			return -2;

		/* check trans complete */
		while (sd_rx_cplt != 1 && sd_rw_error != 1)
		{
		}
		if (sd_rw_error)
		{
			sd_rw_error = 0;
			SYS_ERR("sd_rw_error");
			return -2;
		}
		else
		{
			return 0;
		}
	}
#endif
}
/**
 *  @brief 写sd卡
 *  @param pBuff: 数据地址
 *  @param Block_addr: 块地址（一般每块512B）
 *  @param Blocks_Nbr：块数量
 *  @retval 0:成功
 */
int8_t SD_Write_Disk(uint8_t *pBuff, uint32_t Block_addr, uint32_t Blocks_Nbr)
{
	uint32_t timeout = SD_RW_TIMEOUT;
	while (HAL_SD_GetCardState(&SD_HANDLE) != HAL_SD_CARD_TRANSFER)
	{
		timeout--;
		if (timeout == 0)
		{
			return 1;
		}
	}
	sd_tx_cplt = 0;
#if !SDIO_USE_DMA
	if (HAL_SD_WriteBlocks_IT(&SD_HANDLE, pBuff, Block_addr, Blocks_Nbr) != HAL_OK)
		return 2;
	while (sd_tx_cplt != 1 && sd_rw_error != 1)
	{
	}
	if (sd_rw_error)
	{
		sd_rw_error = 0;
		printf("sd error\r\n");
		return -1;
	}
	else
		return 0;
#else
	if ((uint32_t)pBuff % 4)
	{
		// SYS_LOG("SD Write", "NO Align pBuff");
		{ /* dma trans need align 4 byte */
			int8_t error = 0;
			uint16_t times = Blocks_Nbr / SDIO_DMA_ALIGN4_BUF_BLOCKS;
			uint16_t remain_blocks = Blocks_Nbr % SDIO_DMA_ALIGN4_BUF_BLOCKS;
			uint16_t trans_blocks;
			for (uint16_t i = 0; i < times + 1; i++)
			{
				if (i != times)
					trans_blocks = SDIO_DMA_ALIGN4_BUF_BLOCKS;
				else
					trans_blocks = remain_blocks;

				/* copy pbuf mem to aligned buf : bytes*/
				memcpy(align4_buf, pBuff + i * (SDIO_DMA_ALIGN4_BUF_SIZE), trans_blocks * SDIO_BLOCK_SIZE);

				/* write align mem to addr: blocks*/
				if (HAL_SD_WriteBlocks_DMA(&SD_HANDLE, align4_buf, Block_addr + i * SDIO_DMA_ALIGN4_BUF_BLOCKS, trans_blocks) != HAL_OK)
				{
					SYS_ERR("HAL_SD_WriteBlocks_DMA");
					error = 2;
					break;
				}
				/* check trans complete */
				while (sd_tx_cplt != 1 && sd_rw_error != 1)
				{
				}
				if (sd_rw_error)
				{
					sd_rw_error = 0;
					SYS_ERR("sd_rw_error");
					error = 1;
					break;
				}
			}
			return error;
		}
	}
	else
	{
		if (HAL_SD_WriteBlocks_DMA(&SD_HANDLE, pBuff, Block_addr, Blocks_Nbr) != HAL_OK)
			return 2;
		/* check trans complete */
		while (sd_tx_cplt != 1 && sd_rw_error != 1)
		{
		}
		if (sd_rw_error)
		{
			sd_rw_error = 0;
			SYS_ERR("sd_rw_error");
			return -2;
		}
		else
		{
			return 0;
		}
	}
#endif
}
/** SD_Info_Print **/
void sdcard_info(SD_HandleTypeDef *sd)
{
	printf("---------- SD Card Info ----------\n");
	printf("CardType:%ld\n", sd->SdCard.CardType);
	printf("CardVersion:%ld\n", sd->SdCard.CardVersion);
	printf("Class:%ld\n", sd->SdCard.Class);
	printf("BlockNbr:%ld\n", sd->SdCard.BlockNbr);
	printf("BlockSize:%ld\n", sd->SdCard.BlockSize);
	printf("LogBlockNbr:%ld\n", sd->SdCard.LogBlockNbr);
	printf("LogBlockSize:%ld\n", sd->SdCard.LogBlockSize);
	// printf("CardSpeed:%ld\n", sd->SdCard.CardSpeed);
	printf("---------- Card Info End ----------\n");
}

/** 读写测试 */
#if SDIO_RW_TEST
void sd_rw_test(void)
{
	uint8_t ERROR = 0;
	uint32_t i;
	uint32_t ts = 0, te = 0;

	for (i = 0; i < SD_TEST_BUFF_SIZE; i++)
	{
		sd_buff[i] = TESTDATA;
	}

	/** 写 */
	ts = HAL_GetTick();
	SD_Write_Disk(sd_buff, 0, SD_TEST_BUFF_SIZE / 512);
	te = HAL_GetTick();
	printf("SDIO Write >>>: %ld ms / %ldkb\r\n", te - ts, SD_TEST_BUFF_SIZE / 1024);

	/** 读**/
	for (i = 0; i < SD_TEST_BUFF_SIZE; i++)
	{
		sd_buff[i] = 0x00;
	}

	ts = HAL_GetTick();
	SD_Read_Disk(sd_buff, 0, SD_TEST_BUFF_SIZE / 512);
	te = HAL_GetTick();
	printf("SDIO Read  >>>: %ld ms / %ldkb\r\n", te - ts, SD_TEST_BUFF_SIZE / 1024);

	for (i = 0; i < SD_TEST_BUFF_SIZE; i++)
	{
		if (sd_buff[i] != TESTDATA)
		{
			ERROR = 1;
			break;
		}
	}
	if (ERROR)
		printf("sd test error >>:%ld\r\n", i);
	else
		printf("sd test ok\r\n");
}
#endif

/**
 * @brief 以下为中断回调函数 标志位操作
 * @param hsd
 */

#if SDIO_USE_REGISTCALLBAKS
void BSP_SD_TxCpltCallback(SD_HandleTypeDef *hsd)
{
	sd_tx_cplt = 1;
	// printf("sd tx\r\n");
}
void BSP_SD_RxCpltCallback(SD_HandleTypeDef *hsd)
{
	sd_rx_cplt = 1;
	// printf("sd rx\r\n");
}
void BSP_SD_ErrorCallback(SD_HandleTypeDef *hsd)
{
	sd_rw_error = 1;
	// printf("sd error\r\n");
}

#else
void HAL_SD_TxCpltCallback(SD_HandleTypeDef *hsd)
{
	sd_tx_cplt = 1;
	// printf("sd tx\r\n");
}
void HAL_SD_RxCpltCallback(SD_HandleTypeDef *hsd)
{
	sd_rx_cplt = 1;
	// printf("sd rx\r\n");
}
void HAL_SD_ErrorCallback(SD_HandleTypeDef *hsd)
{
	sd_rw_error = 1;
	// printf("sd error\r\n");
}

#endif
