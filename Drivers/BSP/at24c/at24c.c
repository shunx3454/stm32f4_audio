#include "at24c.h"
#include "main.h"
#include "delay/delay.h"
#include "sw_i2c/sw_i2c.h"

#define AT24C_ADDR 0xA0
#define AT24C_I2C_CLK_PIN GPIO_PIN_8
#define AT24C_I2C_SDA_PIN GPIO_PIN_9
#define AT24C_I2C_CLK_PORT GPIOB
#define AT24C_I2C_SDA_PORT GPIOB

int8_t at24c_i2c_bus_id = -1;

void at24c_i2c_io_init(void)
{
    GPIO_InitTypeDef gpio = {0};
    __HAL_RCC_GPIOB_CLK_ENABLE();

    gpio.Mode = GPIO_MODE_OUTPUT_OD;
    gpio.Pin = AT24C_I2C_CLK_PIN;
    gpio.Pull = GPIO_PULLUP;
    gpio.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(AT24C_I2C_CLK_PORT, &gpio);
    gpio.Pin = AT24C_I2C_SDA_PIN;
    HAL_GPIO_Init(AT24C_I2C_SDA_PORT, &gpio);
}

void at24c_i2c_delay(void)
{
    delay_us(2);
}

void at24c_i2c_sda_out(uint8_t var)
{
    var ? HAL_GPIO_WritePin(AT24C_I2C_SDA_PORT, AT24C_I2C_SDA_PIN, GPIO_PIN_SET) : HAL_GPIO_WritePin(AT24C_I2C_SDA_PORT, AT24C_I2C_SDA_PIN, GPIO_PIN_RESET);
}
uint8_t at24c_i2c_sda_in(void)
{
    return HAL_GPIO_ReadPin(AT24C_I2C_SDA_PORT, AT24C_I2C_SDA_PIN);
}

void at24c_i2c_clk(uint8_t var)
{
    var ? HAL_GPIO_WritePin(AT24C_I2C_CLK_PORT, AT24C_I2C_CLK_PIN, GPIO_PIN_SET) : HAL_GPIO_WritePin(AT24C_I2C_CLK_PORT, AT24C_I2C_CLK_PIN, GPIO_PIN_RESET);
}

void at24c_send(uint16_t RegAddr, uint8_t *pData, uint16_t DataSize)
{
    i2c_bus_send(at24c_i2c_bus_id, RegAddr, 0, pData, DataSize);
}

void at24c_receive(uint16_t RegAddr, uint8_t *pData, uint8_t DataSize)
{
    i2c_bus_receive(at24c_i2c_bus_id, RegAddr, 0, pData, DataSize);
}

void at24c_init(void)
{
    sw_i2c_config_t at24c_i2c_config = {
        .DevAddr = AT24C_ADDR,
        .io_init = at24c_i2c_io_init,
        .io_delay = at24c_i2c_delay,
        .io_sda_out = at24c_i2c_sda_out,
        .io_sda_in = at24c_i2c_sda_in,
        .io_clk = at24c_i2c_clk,
    };

    at24c_i2c_bus_id = sw_i2c_init(&at24c_i2c_config);
}

void at24c_test(void)
{
    uint8_t data[] = {'a', 't', '2', '4', 'c', '0', '2', '!'};
    uint8_t buf[8] = {0};

    at24c_init();

    at24c_send(0x10, data, 8);
    HAL_Delay(5);
    at24c_receive(0x10, buf, 8);
    for (uint8_t i = 0; i < 8; i++)
    {
        if (buf[i] != data[i])
        {
            printf("at24c02 test error\r\n");
            break;
        }
    }
    printf("at24c02 receive: %.8s\r\n", buf);
}