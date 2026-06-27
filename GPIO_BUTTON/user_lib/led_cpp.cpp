#include "led_cpp.h"
#include <new>  

RGB::RGB(GPIO_TypeDef *gpio_r, uint16_t pin_r,
         GPIO_TypeDef *gpio_g, uint16_t pin_g,
         GPIO_TypeDef *gpio_b, uint16_t pin_b)
    : gpio_red(gpio_r),   gpio_green(gpio_g),  gpio_blue(gpio_b),
      pin_red(pin_r),     pin_green(pin_g),    pin_blue(pin_b),
      state(0), last_time_on(0), current_color(Color::OFF)
{
}

void RGB::write(uint8_t mask) 
	{
    HAL_GPIO_WritePin(gpio_red,   pin_red,   (mask >> 2) & 1 ? GPIO_PIN_SET : GPIO_PIN_RESET);
    HAL_GPIO_WritePin(gpio_green, pin_green, (mask >> 1) & 1 ? GPIO_PIN_SET : GPIO_PIN_RESET);
    HAL_GPIO_WritePin(gpio_blue,  pin_blue,  (mask >> 0) & 1 ? GPIO_PIN_SET : GPIO_PIN_RESET);
}

void RGB::setColor(Color::Value color) 
	{
    current_color = color;
    write(static_cast<uint8_t>(color));
}

void RGB::off() {
    write(0x00);
}

void RGB::toggle(Color::Value color) 
{
  if(state) 
	{ 
		off(); 
		state = 0; 
	}
  else        
	{ 
		setColor(color); state = 1; 
	}
}

void RGB::blink(Color::Value color, uint16_t blink_time_ms) 
{
    uint32_t now = HAL_GetTick();
    if (now - last_time_on >= blink_time_ms) 
			{
        last_time_on = now;
        toggle(color);
    }
}


/* --- C wrapper — thêm vào cu?i, gi? nguyên ph?n trên -------- */

static uint8_t  _buf[sizeof(RGB)];
static RGB     *_led = NULL;

extern "C" {
    void led_init(GPIO_TypeDef *gpio_r, uint16_t pin_r,
                  GPIO_TypeDef *gpio_g, uint16_t pin_g,
                  GPIO_TypeDef *gpio_b, uint16_t pin_b) {
        // Construct RGB object vào buffer tinh — không dùng heap
        _led = new(_buf) RGB(gpio_r, pin_r,
                             gpio_g, pin_g,
                             gpio_b, pin_b);
    }
    void led_setColor(ColorC c) { if(_led) _led->setColor(static_cast<Color::Value>(c)); }
    void led_toggle  (ColorC c) { if(_led) _led->toggle  (static_cast<Color::Value>(c)); }
    void led_blink   (ColorC c, uint16_t ms) { if(_led) _led->blink(static_cast<Color::Value>(c), ms); }
    void led_off     (void)     { if(_led) _led->off(); }
}