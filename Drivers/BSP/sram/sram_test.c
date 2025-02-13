#include "sram_test.h"

uint8_t ram_test(uint32_t BaseAddr, uint32_t size)
{
	uint32_t i = 0; // 计数变量
	__IO uint32_t *pSDRAM_32b;
	__IO uint16_t *pSDRAM_16b;
	__IO uint8_t *pSDRAM_8b;
	uint32_t ReadData_32b = 0; // 读取到的数据
	uint16_t ReadData_16b = 0;
	uint8_t ReadData_8b = 0;

	uint32_t ExecutionTime_Begin; // 开始时间
	uint32_t ExecutionTime_End;	  // 结束时间
	uint32_t ExecutionTime;		  // 执行时间
	float ExecutionSpeed;		  // 执行速度

	printf("\r\n***********************************************************************************************\r\n");
	printf("\r\n进行速度测试>>>\r\n\r\n");

	// 写入 >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
	pSDRAM_32b = (uint32_t *)BaseAddr;
	printf("以32位数据宽度写入...\r\n");
	ExecutionTime_Begin = HAL_GetTick(); // 获取 systick 当前时间，单位ms
	for (i = 0; i < size / 4; i++)
	{
		*pSDRAM_32b++ = i; // 写入数据
	}
	ExecutionTime_End = HAL_GetTick();										 // 获取 systick 当前时间，单位ms
	ExecutionTime = ExecutionTime_End - ExecutionTime_Begin;				 // 计算擦除时间，单位ms
	ExecutionSpeed = (float)size / 1024 / 1024 / ExecutionTime * 1000; // 计算速度，单位 MB/S
	printf("写入数据完毕，大小：%ld MB，耗时: %ld ms, 写入速度：%.2f MB/s\r\n", size / 1024 / 1024, ExecutionTime, ExecutionSpeed);

	// 读取	>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
	pSDRAM_32b = (uint32_t *)BaseAddr;
	ExecutionTime_Begin = HAL_GetTick(); // 获取 systick 当前时间，单位ms
	for (i = 0; i < size / 4; i++)
	{
		ReadData_32b = *pSDRAM_32b++; // 从SDRAM读出数据
	}
	ExecutionTime_End = HAL_GetTick();										 // 获取 systick 当前时间，单位ms
	ExecutionTime = ExecutionTime_End - ExecutionTime_Begin;				 // 计算擦除时间，单位ms
	ExecutionSpeed = (float)size / 1024 / 1024 / ExecutionTime * 1000; // 计算速度，单位 MB/S
	printf("读取数据完毕，大小：%ld MB，耗时: %ld ms, 读取速度：%.2f MB/s\r\n", size / 1024 / 1024, ExecutionTime, ExecutionSpeed);

	//// 数据校验 >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
	pSDRAM_32b = (uint32_t *)BaseAddr;
	printf("进行数据校验...\r\n");
	for (i = 0; i < size / 4; i++)
	{
		ReadData_32b = *pSDRAM_32b++; // 从SDRAM读出数据
		if (ReadData_32b != i)		   // 检测数据，若不相等，跳出函数,返回检测失败结果。
		{
			printf("\r\nSDRAM测试失败！！出错位置：%ld,读出数据：%ld\r\n ", i, ReadData_32b);
			return ERROR; // 返回失败标志
		}
	}

	printf("32位数据宽度读写通过。\r\n\r\n以16位数据宽度写入...\r\n");
	pSDRAM_16b = (uint16_t *)BaseAddr;
	ExecutionTime_Begin = HAL_GetTick();
	for (i = 0; i < size / 2; i++)
	{
		*pSDRAM_16b++ = (uint16_t)i;
	}
	ExecutionTime_End = HAL_GetTick();
	ExecutionTime = ExecutionTime_End - ExecutionTime_Begin;
	ExecutionSpeed = (float)size / 1024 / 1024 / ExecutionTime * 1000; // 计算速度，单位 MB/S
	printf("写入数据完毕，大小：%ld MB，耗时: %ld ms, 写入速度：%.2f MB/s\r\n", size / 1024 / 1024, ExecutionTime, ExecutionSpeed);

	// 读取	>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
	pSDRAM_16b = (uint16_t *)BaseAddr;
	ExecutionTime_Begin = HAL_GetTick(); // 获取 systick 当前时间，单位ms
	for (i = 0; i < size / 2; i++)
	{
		ReadData_16b = *pSDRAM_16b++; // 从SDRAM读出数据
	}
	ExecutionTime_End = HAL_GetTick();										 // 获取 systick 当前时间，单位ms
	ExecutionTime = ExecutionTime_End - ExecutionTime_Begin;				 // 计算擦除时间，单位ms
	ExecutionSpeed = (float)size / 1024 / 1024 / ExecutionTime * 1000; // 计算速度，单位 MB/S
	printf("读取数据完毕，大小：%ld MB，耗时: %ld ms, 读取速度：%.2f MB/s\r\n", size / 1024 / 1024, ExecutionTime, ExecutionSpeed);

	//// 数据校验 >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
	printf("进行数据校验...\r\n");
	pSDRAM_16b = (uint16_t *)BaseAddr;
	for (i = 0; i < size / 2; i++)
	{
		ReadData_16b = *pSDRAM_16b++;
		if (ReadData_16b != (uint16_t)i) // 检测数据，若不相等，跳出函数,返回检测失败结果。
		{
			printf("\r\n16位数据宽度读写测试失败！！\r\n");
			printf("请检查NBL0和NBL1的连接\r\n");
			return ERROR; // 返回失败标志
		}
	}

	printf("16位数据宽度读写通过。\r\n\r\n以8位数据宽度写入...\r\n");
	pSDRAM_8b = (uint8_t *)BaseAddr;
	ExecutionTime_Begin = HAL_GetTick();
	for (i = 0; i < size; i++)
	{
		*pSDRAM_8b++ = (uint8_t)i;
	}
	ExecutionTime_End = HAL_GetTick();
	ExecutionTime = ExecutionTime_End - ExecutionTime_Begin;
	ExecutionSpeed = (float)size / 1024 / 1024 / ExecutionTime * 1000; // 计算速度，单位 MB/S
	printf("写入数据完毕，大小：%ld MB，耗时: %ld ms, 写入速度：%.2f MB/s\r\n", size / 1024 / 1024, ExecutionTime, ExecutionSpeed);

	// 读取	>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
	pSDRAM_8b = (uint8_t *)BaseAddr;
	ExecutionTime_Begin = HAL_GetTick(); // 获取 systick 当前时间，单位ms
	for (i = 0; i < size; i++)
	{
		ReadData_8b = *pSDRAM_8b++; // 从SDRAM读出数据
	}
	ExecutionTime_End = HAL_GetTick();										 // 获取 systick 当前时间，单位ms
	ExecutionTime = ExecutionTime_End - ExecutionTime_Begin;				 // 计算擦除时间，单位ms
	ExecutionSpeed = (float)size / 1024 / 1024 / ExecutionTime * 1000; // 计算速度，单位 MB/S
	printf("读取数据完毕，大小：%ld MB，耗时: %ld ms, 读取速度：%.2f MB/s\r\n", size / 1024 / 1024, ExecutionTime, ExecutionSpeed);

	//// 数据校验 >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
	printf("进行数据校验...\r\n");
	pSDRAM_8b = (uint8_t *)BaseAddr;
	for (i = 0; i < size; i++)
	{
		ReadData_8b = *pSDRAM_8b++;
		if (ReadData_8b != (uint8_t)i) // 检测数据，若不相等，跳出函数,返回检测失败结果。
		{
			printf("\r\n8位数据宽度读写测试失败！！\r\n");
			printf("请检查NBL0和NBL1的连接\r\n");
			return ERROR; // 返回失败标志
		}
	}

	printf("8位数据宽度读写通过。\r\n\r\nSDRAM读写测试通过，系统正常\r\n");
	printf("\r\n***********************************************************************************************\r\n");

	return SUCCESS; // 返回成功标志
}
