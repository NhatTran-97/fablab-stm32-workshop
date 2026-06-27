
#ifndef INC_SPI_PROTOCOL_H_
#define INC_SPI_PROTOCOL_H_

#include "stm32f1xx_hal.h"

extern SPI_HandleTypeDef hspi1;

#define SPI_READ    0x00
#define SPI_WRITE   0x80


#define REGISTER_LED     0x01

uint8_t slave_regs[128] = {0};
uint8_t rx_buf[2] = {0};
uint8_t tx_buf[2] = {0};



#endif /* INC_SPI_PROTOCOL_H_ */