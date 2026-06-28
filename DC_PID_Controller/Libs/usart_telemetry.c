#include "usart_telemetry.h"
#include <string.h>

// --- Private ------------------------------------------------
static UART_HandleTypeDef *_huart;
static uint8_t _rx_byte;

// RX state machine
static rx_state_t   _rx_state   = RX_WAIT_SOF1;
static uint8_t _rx_buf[sizeof(telem_rx_payload_t) + 8];
static uint8_t      _rx_idx     = 0;
static uint8_t      _rx_len     = 0;
static uint8_t      _rx_type    = 0;

// Parsed command buffer
static telem_rx_payload_t _cmd_buf;
static volatile uint8_t   _cmd_ready = 0;

// --- CRC8 (polynomial 0x07) ---------------------------------
static uint8_t crc8_calc(uint8_t *data, uint8_t len)
{
    uint8_t crc = 0x00;
    for (uint8_t i = 0; i < len; i++) {
        crc ^= data[i];
        for (uint8_t b = 0; b < 8; b++) {
            if (crc & 0x80)
                crc = (crc << 1) ^ 0x07;
            else
                crc <<= 1;
        }
    }
    return crc;
}


void telem_init(UART_HandleTypeDef *huart)
{
    _huart = huart;
    HAL_UART_Receive_IT(_huart, &_rx_byte, 1);
}

// --- TX -----------------------------------------------------


//void telem_send(telem_tx_payload_t *data)
//{
//	
//	  //char check_tx[(sizeof(telem_tx_payload_t) == 49) ? 1 : -1];
//    //char check_rx[(sizeof(telem_rx_payload_t) == 56) ? 1 : -1];
//	
//    telem_frame_t frame;
//    frame.sof1 = TELEM_SOF1;
//    frame.sof2 = TELEM_SOF2;
//    frame.type = PKT_TYPE_TELEMETRY;
//    frame.len  = sizeof(telem_tx_payload_t);
//    
//    memcpy(&frame.payload.tx, data, sizeof(telem_tx_payload_t));
//    
//    // CRC
//    uint8_t crc_buf[2 + sizeof(telem_tx_payload_t)];
//    crc_buf[0] = frame.type;
//    crc_buf[1] = frame.len;
//    memcpy(&crc_buf[2], data, sizeof(telem_tx_payload_t));
//    frame.crc8 = crc8_calc(crc_buf, sizeof(crc_buf));
//    
//    HAL_UART_Transmit(_huart, (uint8_t*)&frame,
//                      4 + sizeof(telem_tx_payload_t) + 1, 100);
//}


void telem_send(telem_tx_payload_t *data)
{
    uint8_t tx_buf[4 + sizeof(telem_tx_payload_t) + 1];  // dúng size
    
    tx_buf[0] = TELEM_SOF1;
    tx_buf[1] = TELEM_SOF2;
    tx_buf[2] = PKT_TYPE_TELEMETRY;
    tx_buf[3] = sizeof(telem_tx_payload_t);  // 49
    
    memcpy(&tx_buf[4], data, sizeof(telem_tx_payload_t));
    
    // CRC
    uint8_t crc_buf[2 + sizeof(telem_tx_payload_t)];
    crc_buf[0] = PKT_TYPE_TELEMETRY;
    crc_buf[1] = sizeof(telem_tx_payload_t);
    memcpy(&crc_buf[2], data, sizeof(telem_tx_payload_t));
    tx_buf[4 + sizeof(telem_tx_payload_t)] = crc8_calc(crc_buf, sizeof(crc_buf));
    
    HAL_UART_Transmit(_huart, tx_buf, sizeof(tx_buf), 100);
}


//void telem_send(float setpoint, float velocity, float position,
//                float pid_output, int32_t encoder_raw)
//{
//    telem_frame_t frame;
//    frame.sof1 = TELEM_SOF1;
//    frame.sof2 = TELEM_SOF2;
//    frame.type = PKT_TYPE_TELEMETRY;
//    frame.len  = sizeof(telem_tx_payload_t);

//    frame.payload.tx.setpoint    = setpoint;
//    frame.payload.tx.velocity    = velocity;
//    frame.payload.tx.position    = position;
//    frame.payload.tx.pid_output  = pid_output;
//    frame.payload.tx.encoder_raw = encoder_raw;

//    // CRC tính trên type + len + payload
//    uint8_t crc_buf[2 + sizeof(telem_tx_payload_t)];
//    crc_buf[0] = frame.type;
//    crc_buf[1] = frame.len;
//    memcpy(&crc_buf[2], &frame.payload.tx, sizeof(telem_tx_payload_t));
//    frame.crc8 = crc8_calc(crc_buf, sizeof(crc_buf));

//    HAL_UART_Transmit(_huart, (uint8_t*)&frame,
//                      4 + sizeof(telem_tx_payload_t) + 1, 100);
//}

// --- RX State Machine ----------------------------------------
void telem_rx_callback(UART_HandleTypeDef *huart)
{
    if (huart->Instance != _huart->Instance) return;

    uint8_t byte = _rx_byte;

    switch (_rx_state)
    {
        case RX_WAIT_SOF1:
            if (byte == TELEM_SOF1)
                _rx_state = RX_WAIT_SOF2;
            break;

        case RX_WAIT_SOF2:
            if (byte == TELEM_SOF2)
                _rx_state = RX_WAIT_TYPE;
            else
                _rx_state = RX_WAIT_SOF1;  // reset
            break;

        case RX_WAIT_TYPE: // Determine STM32 -> PC or PC -> STM32
            _rx_type  = byte;
            _rx_state = RX_WAIT_LEN;
            break;

        case RX_WAIT_LEN:
            _rx_len   = byte;
            _rx_idx   = 0;
            _rx_state = RX_WAIT_PAYLOAD;
            break;

        case RX_WAIT_PAYLOAD:
            _rx_buf[_rx_idx++] = byte;

	
            if (_rx_idx >= _rx_len)
                _rx_state = RX_WAIT_CRC;
            break;

        case RX_WAIT_CRC:
        {
            // Verify CRC
            uint8_t crc_buf[2 + sizeof(telem_rx_payload_t)];
            crc_buf[0] = _rx_type;
            crc_buf[1] = _rx_len;
					
            memcpy(&crc_buf[2], _rx_buf, _rx_len);
					
            uint8_t expected = crc8_calc(crc_buf, 2 + _rx_len);

            if (byte == expected && _rx_type == PKT_TYPE_CMD)
            {
                memcpy(&_cmd_buf, _rx_buf, sizeof(telem_rx_payload_t));
                _cmd_ready = 1;
            }
            _rx_state = RX_WAIT_SOF1;  // reset cho frame ti?p
            break;
        }
    }

    // Re-arm RX
    HAL_UART_Receive_IT(_huart, &_rx_byte, 1);
}

// --- Get Command ---------------------------------------------
uint8_t telem_get_cmd(telem_rx_payload_t *out)
{
    if (!_cmd_ready) return 0;
    _cmd_ready = 0;
    memcpy(out, &_cmd_buf, sizeof(telem_rx_payload_t));
    return 1;
}
