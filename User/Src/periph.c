#include "periph.h"
#include "bord_config.h"
#include <time.h>

TIM_HandleTypeDef g_tim11;
UART_HandleTypeDef g_uart1;
SD_HandleTypeDef g_sdio1;
I2C_HandleTypeDef g_i2c1;
I2S_HandleTypeDef g_i2s2;
DMA_HandleTypeDef g_dma_spi2;
SRAM_HandleTypeDef g_sram1;
SRAM_HandleTypeDef g_sram2;
DMA_HandleTypeDef g_dma_sdio_tx;
DMA_HandleTypeDef g_dma_sdio_rx;
DMA_HandleTypeDef g_dma_mem2mem;
RTC_HandleTypeDef g_rtc;

void gpio_init(void)
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};

    /* GPIO Ports Clock Enable */
    __HAL_RCC_GPIOF_CLK_ENABLE();

    /*Configure GPIO pin Output Level */
    HAL_GPIO_WritePin(LED0_GPIO_Port, LED0_Pin, GPIO_PIN_SET);
    HAL_GPIO_WritePin(LED1_GPIO_Port, LED1_Pin, GPIO_PIN_SET);

    /*Configure GPIO pin : PtPin */
    GPIO_InitStruct.Pin = LED0_Pin | LED1_Pin;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(LED0_GPIO_Port, &GPIO_InitStruct);
}

void tim_init(void)
{
    g_tim11.Instance = TIM11;
    g_tim11.Init.Prescaler = 168 - 1;
    g_tim11.Init.CounterMode = TIM_COUNTERMODE_UP;
    g_tim11.Init.Period = 1000 - 1;
    g_tim11.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
    g_tim11.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
    if (HAL_TIM_Base_Init(&g_tim11) != HAL_OK)
    {
        Error_Handler();
    }
    HAL_TIM_RegisterCallback(&g_tim11, HAL_TIM_PERIOD_ELAPSED_CB_ID, BSP_TIM11_ElapsCallback);
}

void uart_init(void)
{
    g_uart1.Instance = USART1;
    g_uart1.Init.BaudRate = 115200;
    g_uart1.Init.WordLength = UART_WORDLENGTH_8B;
    g_uart1.Init.StopBits = UART_STOPBITS_1;
    g_uart1.Init.Parity = UART_PARITY_NONE;
    g_uart1.Init.Mode = UART_MODE_TX_RX;
    g_uart1.Init.HwFlowCtl = UART_HWCONTROL_NONE;
    g_uart1.Init.OverSampling = UART_OVERSAMPLING_16;
    if (HAL_UART_Init(&g_uart1) != HAL_OK)
    {
        Error_Handler();
    }
}

int sdio_init(void)
{
    static uint8_t is_init = 0;
    if (is_init == 1)
    {
        return 0;
    }

    /* USER CODE END SDMMC1_Init 1 */
#if SDIO_USE_DMA
    dma_sdio_init();
#endif
    g_sdio1.Instance = SDIO;
    g_sdio1.Init.ClockEdge = SDIO_CLOCK_EDGE_RISING;
    g_sdio1.Init.ClockBypass = SDIO_CLOCK_BYPASS_DISABLE;
    g_sdio1.Init.ClockPowerSave = SDIO_CLOCK_POWER_SAVE_DISABLE;
    g_sdio1.Init.BusWide = SDIO_BUS_WIDE_1B;
    g_sdio1.Init.HardwareFlowControl = SDIO_HARDWARE_FLOW_CONTROL_DISABLE;
    g_sdio1.Init.ClockDiv = 1;
    if (HAL_SD_Init(&g_sdio1) != HAL_OK)
    {
        return -1;
    }
    if (HAL_SD_ConfigWideBusOperation(&g_sdio1, SDIO_BUS_WIDE_4B) != HAL_OK)
    {
        return -1;
    }
    is_init = 1;

#if SDIO_USE_REGISTCALLBAKS
    HAL_SD_RegisterCallback(&g_sdio1, HAL_SD_TX_CPLT_CB_ID, BSP_SD_TxCpltCallback);
    HAL_SD_RegisterCallback(&g_sdio1, HAL_SD_RX_CPLT_CB_ID, BSP_SD_RxCpltCallback);
    HAL_SD_RegisterCallback(&g_sdio1, HAL_SD_ERROR_CB_ID, BSP_SD_ErrorCallback);
#endif
    return 0;
}

void i2c_init(void)
{
    g_i2c1.Instance = I2C1;
    g_i2c1.Init.ClockSpeed = 400000;
    g_i2c1.Init.DutyCycle = I2C_DUTYCYCLE_16_9;
    g_i2c1.Init.OwnAddress1 = 0;
    g_i2c1.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
    g_i2c1.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
    g_i2c1.Init.OwnAddress2 = 0;
    g_i2c1.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
    g_i2c1.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;
    if (HAL_I2C_Init(&g_i2c1) != HAL_OK)
    {
        Error_Handler();
    }
}

int i2s_init(uint32_t bitswidth, uint32_t audioFreq)
{
    uint32_t dmaPDataAlign;
    uint32_t dmaMDataAlign;
    uint32_t DataFormat;

    if (bitswidth == 16)
    {
        dmaPDataAlign = DMA_PDATAALIGN_HALFWORD;
        dmaMDataAlign = DMA_MDATAALIGN_HALFWORD;
        DataFormat = I2S_DATAFORMAT_16B;
    }
    else if (bitswidth == 24)
    {
        dmaPDataAlign = DMA_PDATAALIGN_WORD;
        dmaMDataAlign = DMA_MDATAALIGN_WORD;
        DataFormat = I2S_DATAFORMAT_24B;
    }
    else
    {
        dmaPDataAlign = DMA_PDATAALIGN_WORD;
        dmaMDataAlign = DMA_MDATAALIGN_WORD;
        DataFormat = I2S_DATAFORMAT_32B;
    }

    g_i2s2.Instance = SPI2;
    g_i2s2.Init.Mode = I2S_MODE_MASTER_TX;
    g_i2s2.Init.Standard = I2S_STANDARD_PHILIPS;
    g_i2s2.Init.DataFormat = DataFormat;
    g_i2s2.Init.MCLKOutput = I2S_MCLKOUTPUT_ENABLE;
    g_i2s2.Init.AudioFreq = audioFreq;
    g_i2s2.Init.CPOL = I2S_CPOL_LOW;
    g_i2s2.Init.ClockSource = I2S_CLOCK_PLL;
    g_i2s2.Init.FullDuplexMode = I2S_FULLDUPLEXMODE_ENABLE;
    HAL_CHECK(HAL_I2S_DeInit(&g_i2s2));
    HAL_CHECK(HAL_I2S_Init(&g_i2s2));

    /* DMA controller clock enable */
    __HAL_RCC_DMA1_CLK_ENABLE();

    /* I2S2 DMA Init */
    /* SPI2_TX Init */
    g_dma_spi2.Instance = DMA1_Stream4;
    g_dma_spi2.Init.Channel = DMA_CHANNEL_0;
    g_dma_spi2.Init.Direction = DMA_MEMORY_TO_PERIPH;
    g_dma_spi2.Init.PeriphInc = DMA_PINC_DISABLE;
    g_dma_spi2.Init.MemInc = DMA_MINC_ENABLE;
    g_dma_spi2.Init.PeriphDataAlignment = dmaPDataAlign;
    g_dma_spi2.Init.MemDataAlignment = dmaMDataAlign;
    g_dma_spi2.Init.Mode = DMA_CIRCULAR;
    g_dma_spi2.Init.Priority = DMA_PRIORITY_HIGH;
    g_dma_spi2.Init.FIFOMode = DMA_FIFOMODE_DISABLE;
    // g_dma_spi2.Init.PeriphBurst = DMA_PBURST_SINGLE;
    // g_dma_spi2.Init.MemBurst = DMA_MBURST_SINGLE;
    HAL_CHECK(HAL_DMA_DeInit(&g_dma_spi2));
    HAL_CHECK(HAL_DMA_Init(&g_dma_spi2));

    /* DMA1_Stream4_IRQn interrupt configuration */
    HAL_NVIC_SetPriority(DMA1_Stream4_IRQn, 5, 0);
    HAL_NVIC_EnableIRQ(DMA1_Stream4_IRQn);

    __HAL_LINKDMA(&g_i2s2, hdmatx, g_dma_spi2);

    return 0;
}

void sram_init(void)
{
    FSMC_NORSRAM_TimingTypeDef Timing = {0};
    FSMC_NORSRAM_TimingTypeDef ExTiming = {0};
    /** Perform the SRAM1 memory initialization sequence
     */
    g_sram1.Instance = FSMC_NORSRAM_DEVICE;
    g_sram1.Extended = FSMC_NORSRAM_EXTENDED_DEVICE;
    /* g_sram1.Init */
    g_sram1.Init.NSBank = FSMC_NORSRAM_BANK3;
    g_sram1.Init.DataAddressMux = FSMC_DATA_ADDRESS_MUX_DISABLE;
    g_sram1.Init.MemoryType = FSMC_MEMORY_TYPE_SRAM;
    g_sram1.Init.MemoryDataWidth = FSMC_NORSRAM_MEM_BUS_WIDTH_16;
    g_sram1.Init.BurstAccessMode = FSMC_BURST_ACCESS_MODE_DISABLE;
    g_sram1.Init.WaitSignalPolarity = FSMC_WAIT_SIGNAL_POLARITY_LOW;
    g_sram1.Init.WrapMode = FSMC_WRAP_MODE_DISABLE;
    g_sram1.Init.WaitSignalActive = FSMC_WAIT_TIMING_BEFORE_WS;
    g_sram1.Init.WriteOperation = FSMC_WRITE_OPERATION_ENABLE;
    g_sram1.Init.WaitSignal = FSMC_WAIT_SIGNAL_DISABLE;
    g_sram1.Init.ExtendedMode = FSMC_EXTENDED_MODE_DISABLE;
    g_sram1.Init.AsynchronousWait = FSMC_ASYNCHRONOUS_WAIT_DISABLE;
    g_sram1.Init.WriteBurst = FSMC_WRITE_BURST_DISABLE;
    g_sram1.Init.PageSize = FSMC_PAGE_SIZE_NONE;
    /* Timing */
    Timing.AddressSetupTime = 0;
    Timing.AddressHoldTime = 0;
    Timing.DataSetupTime = 12;
    Timing.BusTurnAroundDuration = 0;
    Timing.CLKDivision = 0;
    Timing.DataLatency = 0;
    Timing.AccessMode = FSMC_ACCESS_MODE_A;
    /* ExtTiming */

    if (HAL_SRAM_Init(&g_sram1, &Timing, NULL) != HAL_OK)
    {
        Error_Handler();
    }

    /** Perform the SRAM2 memory initialization sequence
     */
    g_sram2.Instance = FSMC_NORSRAM_DEVICE;
    g_sram2.Extended = FSMC_NORSRAM_EXTENDED_DEVICE;
    /* g_sram2.Init */
    g_sram2.Init.NSBank = FSMC_NORSRAM_BANK4;
    g_sram2.Init.DataAddressMux = FSMC_DATA_ADDRESS_MUX_DISABLE;
    g_sram2.Init.MemoryType = FSMC_MEMORY_TYPE_SRAM;
    g_sram2.Init.MemoryDataWidth = FSMC_NORSRAM_MEM_BUS_WIDTH_16;
    g_sram2.Init.BurstAccessMode = FSMC_BURST_ACCESS_MODE_DISABLE;
    g_sram2.Init.WaitSignalPolarity = FSMC_WAIT_SIGNAL_POLARITY_LOW;
    g_sram2.Init.WrapMode = FSMC_WRAP_MODE_DISABLE;
    g_sram2.Init.WaitSignalActive = FSMC_WAIT_TIMING_BEFORE_WS;
    g_sram2.Init.WriteOperation = FSMC_WRITE_OPERATION_ENABLE;
    g_sram2.Init.WaitSignal = FSMC_WAIT_SIGNAL_DISABLE;
    g_sram2.Init.ExtendedMode = FSMC_EXTENDED_MODE_ENABLE;
    g_sram2.Init.AsynchronousWait = FSMC_ASYNCHRONOUS_WAIT_DISABLE;
    g_sram2.Init.WriteBurst = FSMC_WRITE_BURST_DISABLE;
    g_sram2.Init.PageSize = FSMC_PAGE_SIZE_NONE;
    /* Timing */
    Timing.AddressSetupTime = 15;
    Timing.AddressHoldTime = 0;
    Timing.DataSetupTime = 60;
    Timing.BusTurnAroundDuration = 15;
    Timing.CLKDivision = 16;
    Timing.DataLatency = 17;
    Timing.AccessMode = FSMC_ACCESS_MODE_A;
    /* ExtTiming */
    ExTiming.AddressSetupTime = 9;
    ExTiming.AddressHoldTime = 0;
    ExTiming.DataSetupTime = 9;
    ExTiming.BusTurnAroundDuration = 15;
    ExTiming.CLKDivision = 16;
    ExTiming.DataLatency = 17;
    ExTiming.AccessMode = FSMC_ACCESS_MODE_A;

    if (HAL_SRAM_Init(&g_sram2, &Timing, &ExTiming) != HAL_OK)
    {
        Error_Handler();
    }
}

void dma_sdio_init(void)
{
    /*  ！！！！！！！！！！！！！！！先初始化DMA时钟不然DMA无法配置！！！！！！！！！！！！！！！ */
    /* DMA controller clock enable */
    __HAL_RCC_DMA2_CLK_ENABLE();

    /* SDIO_TX Init */
    g_dma_sdio_tx.Instance = DMA2_Stream3;
    g_dma_sdio_tx.Init.Channel = DMA_CHANNEL_4;
    g_dma_sdio_tx.Init.Direction = DMA_MEMORY_TO_PERIPH;
    g_dma_sdio_tx.Init.PeriphInc = DMA_PINC_DISABLE;
    g_dma_sdio_tx.Init.MemInc = DMA_MINC_ENABLE;
    g_dma_sdio_tx.Init.PeriphDataAlignment = DMA_PDATAALIGN_WORD;
    g_dma_sdio_tx.Init.MemDataAlignment = DMA_MDATAALIGN_WORD;
    g_dma_sdio_tx.Init.Mode = DMA_PFCTRL;
    g_dma_sdio_tx.Init.Priority = DMA_PRIORITY_VERY_HIGH;
    g_dma_sdio_tx.Init.FIFOMode = DMA_FIFOMODE_ENABLE;
    g_dma_sdio_tx.Init.FIFOThreshold = DMA_FIFO_THRESHOLD_FULL;
    g_dma_sdio_tx.Init.MemBurst = DMA_MBURST_INC4;
    g_dma_sdio_tx.Init.PeriphBurst = DMA_PBURST_INC4;
    HAL_DMA_DeInit(&g_dma_sdio_tx);
    if (HAL_DMA_Init(&g_dma_sdio_tx) != HAL_OK)
    {
        Error_Handler();
    }
    __HAL_LINKDMA(&g_sdio1, hdmatx, g_dma_sdio_tx);

    /* SDIO_RX Init */
    g_dma_sdio_rx.Instance = DMA2_Stream6;
    g_dma_sdio_rx.Init.Channel = DMA_CHANNEL_4;
    g_dma_sdio_rx.Init.Direction = DMA_PERIPH_TO_MEMORY;
    g_dma_sdio_rx.Init.PeriphInc = DMA_PINC_DISABLE;
    g_dma_sdio_rx.Init.MemInc = DMA_MINC_ENABLE;
    g_dma_sdio_rx.Init.PeriphDataAlignment = DMA_PDATAALIGN_WORD;
    g_dma_sdio_rx.Init.MemDataAlignment = DMA_MDATAALIGN_WORD;
    g_dma_sdio_rx.Init.Mode = DMA_PFCTRL;
    g_dma_sdio_rx.Init.Priority = DMA_PRIORITY_VERY_HIGH;
    g_dma_sdio_rx.Init.FIFOMode = DMA_FIFOMODE_ENABLE;
    g_dma_sdio_rx.Init.FIFOThreshold = DMA_FIFO_THRESHOLD_FULL;
    g_dma_sdio_rx.Init.MemBurst = DMA_MBURST_INC4;
    g_dma_sdio_rx.Init.PeriphBurst = DMA_PBURST_INC4;
    HAL_DMA_DeInit(&g_dma_sdio_rx);
    if (HAL_DMA_Init(&g_dma_sdio_rx) != HAL_OK)
    {
        Error_Handler();
    }
    __HAL_LINKDMA(&g_sdio1, hdmarx, g_dma_sdio_rx);

    /* DMA interrupt init */
    /* DMA2_Stream3_IRQn interrupt configuration */
    HAL_NVIC_SetPriority(DMA2_Stream3_IRQn, 3, 0);
    HAL_NVIC_EnableIRQ(DMA2_Stream3_IRQn);

    /* DMA2_Stream6_IRQn interrupt configuration */
    HAL_NVIC_SetPriority(DMA2_Stream6_IRQn, 3, 0);
    HAL_NVIC_EnableIRQ(DMA2_Stream6_IRQn);
}

/*
 * @param: width can be 0:8bit, 1:16bit, 2:32bit
 * @param: cnt width bytes trans
 */
void mem_dma_init(void)
{
    __HAL_RCC_DMA2_CLK_ENABLE();
    /* Configure DMA request g_dma_mem2mem on DMA2_Stream0 */
    g_dma_mem2mem.Instance = DMA2_Stream0;
    g_dma_mem2mem.Init.Channel = DMA_CHANNEL_0;
    g_dma_mem2mem.Init.Direction = DMA_MEMORY_TO_MEMORY;
    g_dma_mem2mem.Init.PeriphInc = DMA_PINC_ENABLE;
    g_dma_mem2mem.Init.MemInc = DMA_MINC_DISABLE;
    g_dma_mem2mem.Init.PeriphDataAlignment = DMA_PDATAALIGN_HALFWORD;
    g_dma_mem2mem.Init.MemDataAlignment = DMA_MDATAALIGN_HALFWORD;
    g_dma_mem2mem.Init.Mode = DMA_NORMAL;
    g_dma_mem2mem.Init.Priority = DMA_PRIORITY_HIGH;
    g_dma_mem2mem.Init.FIFOMode = DMA_FIFOMODE_DISABLE;
    g_dma_mem2mem.Init.FIFOThreshold = DMA_FIFO_THRESHOLD_FULL;
    g_dma_mem2mem.Init.MemBurst = DMA_MBURST_INC8;
    g_dma_mem2mem.Init.PeriphBurst = DMA_PBURST_INC8;
    if (HAL_DMA_Init(&g_dma_mem2mem) != HAL_OK)
    {
        Error_Handler();
    }

    /* DMA interrupt init */
    /* DMA2_Stream0_IRQn interrupt configuration */
    HAL_NVIC_SetPriority(DMA2_Stream0_IRQn, 6, 0);
    HAL_NVIC_EnableIRQ(DMA2_Stream0_IRQn);
}

void rtc_init(void)
{
    struct tm tm;
    RTC_TimeTypeDef sTime = {0};
    RTC_DateTypeDef sDate = {0};

    g_rtc.Instance = RTC;
    g_rtc.Init.HourFormat = RTC_HOURFORMAT_24;
    g_rtc.Init.AsynchPrediv = 127;
    g_rtc.Init.SynchPrediv = 255;
    g_rtc.Init.OutPut = RTC_OUTPUT_DISABLE;
    g_rtc.Init.OutPutPolarity = RTC_OUTPUT_POLARITY_HIGH;
    g_rtc.Init.OutPutType = RTC_OUTPUT_TYPE_OPENDRAIN;
    HAL_CHECK(HAL_RTC_Init(&g_rtc));

    if (HAL_RTCEx_BKUPRead(&g_rtc, RTC_BKP_DR19) == 0xccff)
    {
        return;
    }

    // "2024-12-19 13:37:22"
    if (sscanf(BUILD_TIME, "%d-%d-%d-%d %d:%d:%d",
               &tm.tm_year, &tm.tm_mon, &tm.tm_mday, &tm.tm_wday,
               &tm.tm_hour, &tm.tm_min, &tm.tm_sec) != 7)
    {
        SYS_ERR("sscanf");
    }
    sTime.Hours = tm.tm_hour;
    sTime.Minutes = tm.tm_min;
    sTime.Seconds = tm.tm_sec;
    sTime.DayLightSaving = RTC_DAYLIGHTSAVING_NONE;
    sTime.StoreOperation = RTC_STOREOPERATION_RESET;
    HAL_CHECK(HAL_RTC_SetTime(&g_rtc, &sTime, RTC_FORMAT_BIN));

    sDate.Year = tm.tm_year - 2000;
    sDate.Month = tm.tm_mon;
    sDate.Date = tm.tm_mday;
    sDate.WeekDay = tm.tm_wday;
    HAL_CHECK(HAL_RTC_SetDate(&g_rtc, &sDate, RTC_FORMAT_BIN));

    HAL_RTCEx_BKUPWrite(&g_rtc, RTC_BKP_DR19, 0xccff);
}