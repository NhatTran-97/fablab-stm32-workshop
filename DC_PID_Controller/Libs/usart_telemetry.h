#ifndef USART_TELEMETRY_H
#define USART_TELEMETRY_H

#include "main.h"
#include "pid_control.h"
#include "motor_encoder.h"


/*
frame structure:
[0xAA][0x55][TYPE][LEN][PAYLOAD...][CRC8]
SOF1  SOF2  type  length           checksum

SOF = Start of Frame
*/

// Frame structure
#define TELEM_SOF1          0xAA
#define TELEM_SOF2          0x55
#define PKT_TYPE_TELEMETRY  0x01  // STM32 > PC
#define PKT_TYPE_CMD        0x02  // PC -> STM32

// TX packet - telemetry data
typedef struct __attribute__((packed)) {
	
	// Motor Left
    float setpoint;
    float velocity;
    float position;
    float pid_output;
    int32_t encoder_raw_left;
	
	// Motor Right
    float setpoint_right;
    float velocity_right;
    float position_right;
    float pid_output_right;
    int32_t encoder_raw_right;
	
	
	  uint8_t mode;               // 0=VELOCITY, 1=POSITION
    float pos_setpoint_left;
    float pos_setpoint_right;  
		// Tong: 40 + 1 + 4 + 4 = 49 bytes
} telem_tx_payload_t;

// RX packet - command from PC
typedef struct __attribute__((packed)) {
    // Motor Left
    float kp_left;
    float ki_left;
    float kd_left;
    float setpoint_left;
    // Motor Right
    float kp_right;
    float ki_right;
    float kd_right;
    float setpoint_right;
	
	  float pos_kp_left;
    float pos_ki_left;
    float pos_kd_left;
    float pos_kp_right;
    float pos_ki_right;
    float pos_kd_right;
		
} telem_rx_payload_t;



// Verify size at compile time
//typedef char tx_size_check[(sizeof(telem_tx_payload_t) == 49) ? 1 : -1];
//typedef char rx_size_check[(sizeof(telem_rx_payload_t) == 56) ? 1 : -1];



// Full frame
typedef struct __attribute__((packed)) {
    uint8_t sof1;
    uint8_t sof2;
    uint8_t type;
    uint8_t len;
    union __attribute__((packed)) {  // ? thêm __attribute__((packed)) vào dây
        telem_tx_payload_t tx;
        telem_rx_payload_t rx;
    } payload;
    uint8_t crc8;
} telem_frame_t;

// RX state machine
typedef enum {
    RX_WAIT_SOF1,
    RX_WAIT_SOF2,
    RX_WAIT_TYPE,
    RX_WAIT_LEN,
    RX_WAIT_PAYLOAD,
    RX_WAIT_CRC
} rx_state_t;

// Public API
void telem_init(UART_HandleTypeDef *huart);

void telem_send(telem_tx_payload_t *data);

void telem_rx_callback(UART_HandleTypeDef *huart);

uint8_t telem_get_cmd(telem_rx_payload_t *out);

#endif
