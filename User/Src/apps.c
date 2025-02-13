
#include "main.h"
#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"
#include "ff.h"
#include "periph.h"
#include "bsp.h"
#include "usbd_msc_app.h"
#include "lcd/lcd.h"
#include "lvgl.h"
#include "examples/porting/lv_port_disp.h"
#include "examples/porting/lv_port_indev.h"
#include "demos/lv_demos.h"

#define EXSRAM_ADDR 0x68000000
#define EXSRAM_SIZE 0x100000

// 定义静态内存区域
static StaticTask_t IdleTaskTCB;		 // TCB (Task Control Block)
static StackType_t IdleTaskStack[1024];	 // 堆栈空间
static StaticTask_t TimerTaskTCB;		 // Timer Task 的 TCB（任务控制块）
static StackType_t TimerTaskStack[1024]; // 堆栈空间

TaskHandle_t ThreadStart;
TaskHandle_t ThreadAppMain;
TaskHandle_t ThreadSdio;
SemaphoreHandle_t SemaphoreSdioWithUSBAndFatfs;

FATFS fs;

void fatfs_init(FATFS *fs, const char *path);
void fatfs_test(void);
void thread_start(void *param);
void thread_app_main(void *param);
void thread_sdio(void *param);

void os_run(void)
{
	xTaskCreate(thread_start, "start", 1024, NULL, 10, &ThreadStart);
	vTaskStartScheduler();
}

void thread_start(void *param)
{
	taskENTER_CRITICAL();

	/* User code begin */
	xTaskCreate(thread_app_main, "app_main", 2048, NULL, 10, &ThreadAppMain);
	xTaskCreate(thread_sdio, "sdio", 128, NULL, 5, &ThreadSdio);
	vSemaphoreCreateBinary(SemaphoreSdioWithUSBAndFatfs);

	sram_init();
	gpio_init();
	uart_init();
	tim_init();
	mem_dma_init();

	// lv_init();
	// lv_port_disp_init();
	// lv_port_indev_init();
	// lv_demo_benchmark();
	// lv_demo_music();

	// at24c_test();
	// usbd_msc_init();

	taskEXIT_CRITICAL();

	vTaskDelete(NULL);
}

void thread_app_main(void *param)
{
	HAL_TIM_Base_Start_IT(&g_tim11);
	/* After SemaphoreSdioWithUSBAndFatfs */
	fatfs_init(&fs, "/SD");
	// fatfs_test();

	Audio audio;
	AudioInit(&audio);
	audio_start("/SD/MUSIC/Spectre.flac");

	for (;;)
	{
		audio_loop();
		// touch_loop();
		// lv_timer_handler();
		vTaskDelay(50 / portTICK_PERIOD_MS);
	}
}
void thread_sdio(void *param)
{
	for (;;)
	{
		vTaskDelay(1000 / portTICK_PERIOD_MS);
		printf("sdio int num:%d & dma int num:%d\r\n", sdio_int_num, dma_int_num);
		// printf("RTC BKP DR19 %#lx\r\n", HAL_RTCEx_BKUPRead(&g_rtc, RTC_BKP_DR19));
	}
}

void vApplicationGetIdleTaskMemory(StaticTask_t **ppxIdleTaskTCBBuffer,
								   StackType_t **ppxIdleTaskStackBuffer,
								   configSTACK_DEPTH_TYPE *puxIdleTaskStackSize)
{
	*ppxIdleTaskTCBBuffer = &IdleTaskTCB;
	*ppxIdleTaskStackBuffer = IdleTaskStack;
	*puxIdleTaskStackSize = 1024;
}

void vApplicationGetTimerTaskMemory(StaticTask_t **ppxTimerTaskTCBBuffer,
									StackType_t **ppxTimerTaskStackBuffer,
									configSTACK_DEPTH_TYPE *puxTimerTaskStackSize)
{
	*ppxTimerTaskTCBBuffer = &TimerTaskTCB;
	*ppxTimerTaskStackBuffer = TimerTaskStack;
	*puxTimerTaskStackSize = 1024;
}

void BSP_TIM11_ElapsCallback(TIM_HandleTypeDef* htim)
{
	if(htim == &g_tim11)
	{
		lv_tick_inc(1);
		HAL_IncTick();
	}
}

void fatfs_init(FATFS *fs, const char *path)
{
	FRESULT res;
	uint8_t *fbuf;

	res = f_mount(fs, path, 1);
	if (res == FR_OK)
	{
		SYS_LOG("FS", "Fatfs mount ok!");
		return;
	}

	if (res == FR_NO_FILESYSTEM)
	{
		SYS_LOG("FS", "No file system");
		SYS_LOG("FS", "Format...");

		fbuf = ff_memalloc(1024);
		if (fbuf == NULL)
		{
			SYS_LOG("FS", "Format fail.");
			return;
		}
		res = f_mkfs("/SD", NULL, fbuf, 1024);
		ff_memfree(fbuf);
		if (res != FR_OK)
		{
			SYS_LOG("FS", "Format fail.");
			return;
		}
		SYS_LOG("FS", "File system create ok!");
	}
	else
	{
		printf("Fatfs mount error:%d\r\n", res);
	}
}

void fatfs_test()
{
	FRESULT res;
	FATFS *pfs;
	UINT nbw, nbr;
	DWORD fre_clust, fre_sect, tot_sect;
	FIL *fl0, *fl1, *fl2;
	uint8_t *wbuf;
	const int wbuf_size = 1024;
	const char *text = "This is a test file\r\n";
	const char *charzhcn = "中文字符测试，今天天气真好！";

	/* mem alloc */
	fl0 = pvPortMalloc(sizeof(FIL));
	fl1 = pvPortMalloc(sizeof(FIL));
	fl2 = pvPortMalloc(sizeof(FIL));
	wbuf = pvPortMalloc(wbuf_size);

	/* get file system free */
	/* Get volume information and free clusters of drive 1 */
	res = f_getfree("/SD", &fre_clust, &pfs);
	if (res == FR_OK)
	{
		/* Get total sectors and free sectors */
		tot_sect = (pfs->n_fatent - 2) * pfs->csize;
		fre_sect = fre_clust * pfs->csize;

		/* Print the free space (assuming 512 bytes/sector) */
		printf("\r\n%10lu KiB total drive space.\r\n%10lu KiB available.\r\n", tot_sect / 2, fre_sect / 2);
	}

	/* file function test */
	res = f_open(fl0, "/SD/text1.txt", FA_CREATE_ALWAYS | FA_WRITE | FA_READ);
	if (res)
		SYS_ERR("f_open");
	res = f_write(fl0, text, (UINT)strlen(text), (UINT *)&nbw);
	if (res)
		SYS_ERR("f_write");
	else
		printf("text1.txt written %d bytes\r\n", nbw);
	f_close(fl0);

	/* Multi files opt */
	res = f_open(fl0, "/SD/MUSIC/MUSIC.txt", FA_READ);
	if (res)
	{
		SYS_ERR("f_open");
	}
	res = f_open(fl1, "/SD/text1.txt", FA_OPEN_EXISTING | FA_WRITE);
	if (res)
	{
		SYS_ERR("f_open");
		f_close(fl0);
		goto FS_RETURN;
	}
	memset(wbuf, 0, wbuf_size);
	f_read(fl0, wbuf, wbuf_size, &nbr);
	printf("/SD/MUSIC/MUSIC.txt: read %d bytes\n", nbr);
	f_lseek(fl1, f_size(fl1));
	f_write(fl1, wbuf, strlen(wbuf), &nbw);
	printf("/SD/text1.txt: append %d bytes\n", nbw);

	/* close files */
	f_close(fl0);
	f_close(fl1);

	/* chinese char test */
	res = f_open(fl2, "/SD/这是一个中文件名测试文件.txt", FA_CREATE_ALWAYS | FA_WRITE | FA_READ);
	if (res)
	{
		printf("中文文件名测试失败\r\n");
		goto FS_RETURN;
	}
	res = f_write(fl2, charzhcn, strlen(charzhcn), &nbw);
	if (res)
	{
		printf("中文件名写入失败\r\n");
		f_close(fl2);
		goto FS_RETURN;
	}
	else
	{
		printf("成功写入 %u 字节\r\n", strlen(charzhcn));
	}
	f_close(fl2);
	HAL_Delay(1000);
	printf("开始读取中文文件...\r\n");
	res = f_open(fl2, "/SD/这是一个中文件名测试文件.txt", FA_READ);
	if (res)
	{
		printf("打开文件失败\r\n");
		goto FS_RETURN;
	}
	memset(wbuf, 0, wbuf_size);
	res = f_read(fl2, wbuf, wbuf_size, &nbr);
	if (res)
	{
		printf("读取文件失败\r\n");
		f_close(fl2);
		goto FS_RETURN;
	}
	printf("成功读取 %u 字节 \r\n%s\r\n", nbr, wbuf);
	f_close(fl2);

FS_RETURN:
	/* mem free */
	vPortFree(fl0);
	vPortFree(fl1);
	vPortFree(fl2);
	vPortFree(wbuf);
}
