#ifndef LED_CPP_H
#define LED_CPP_H

#include "stm32l4xx_hal.h"

#ifdef __cplusplus
extern "C" {
#endif

/* --- C interface -------------------- */
typedef enum {
    COLOR_RED    = 0x04,
    COLOR_GREEN  = 0x02,
    COLOR_BLUE   = 0x01,
    COLOR_WHITE  = 0x07,
    COLOR_YELLOW = 0x06,
    COLOR_PURPLE = 0x05,
    COLOR_OFF    = 0x00,
} ColorC;

void led_init    (GPIO_TypeDef *gpio_r, uint16_t pin_r,
                  GPIO_TypeDef *gpio_g, uint16_t pin_g,
                  GPIO_TypeDef *gpio_b, uint16_t pin_b);
void led_setColor(ColorC color);
void led_toggle  (ColorC color);
void led_blink   (ColorC color, uint16_t ms);
void led_off     (void);

#ifdef __cplusplus
}  /* ? dóng extern "C" ? dây */
#endif

/* --- C++ class (ch? dùng trong .cpp) - */
#ifdef __cplusplus
namespace Color {
    enum Value : uint8_t {
        RED    = 0x04,
        GREEN  = 0x02,
        BLUE   = 0x01,
        WHITE  = 0x07,
        YELLOW = 0x06,
        PURPLE = 0x05,
        OFF    = 0x00,
    };
}

class RGB {
public:
    RGB(GPIO_TypeDef *gpio_r, uint16_t pin_r,
        GPIO_TypeDef *gpio_g, uint16_t pin_g,
        GPIO_TypeDef *gpio_b, uint16_t pin_b);

    void setColor(Color::Value color);
    void toggle  (Color::Value color);
    void blink   (Color::Value color, uint16_t blink_time_ms);
    void off     (void);

private:
    GPIO_TypeDef *gpio_red, *gpio_green, *gpio_blue;
    uint16_t      pin_red,   pin_green,   pin_blue;
    uint8_t       state;
    uint32_t      last_time_on;
    Color::Value  current_color;

    void write(uint8_t mask);
};
#endif /* __cplusplus */

#endif /* LED_CPP_H */



