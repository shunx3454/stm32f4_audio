#ifndef __BSP_AT24C_H_
#define __BSP_AT24C_H_

#include <stdint.h>

void at24c_init(void);
void at24c_test(void);
void at24c_send(uint16_t RegAddr, uint8_t *pData, uint16_t DataSize);
void at24c_receive(uint16_t RegAddr, uint8_t *pData, uint8_t DataSize);

#endif