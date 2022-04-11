#include "led.h"
extern void delay_ms(unsigned int ms);
void led_init(void)
{
	RCC_MP_AHB4ENSETR |= (0x1 << 4);
	RCC_MP_AHB4ENSETR |= (0x1 << 5);
	gpioInit_t init;
	init.pin = LED1;
	init.mode = OUTPUT;
	init.type = PP;
	init.speed = LOW;
	init.pupd = NOPUPD;
	hal_gpio_init(LED1_PORT, &init);
	init.pin = LED2;
	hal_gpio_init(LED2_PORT, &init);
	init.pin = LED3;
	hal_gpio_init(LED3_PORT, &init);
}
void led_flash(unsigned int ms)
{
	hal_gpio_write(LED1_PORT, LED1,GPIO_SET);
	hal_gpio_write(LED2_PORT, LED2, GPIO_SET);
	hal_gpio_write(LED3_PORT, LED3,GPIO_SET);
	delay_ms(ms);
	hal_gpio_write(LED1_PORT, LED1,GPIO_RESET);
	hal_gpio_write(LED2_PORT, LED2, GPIO_RESET);
	hal_gpio_write(LED3_PORT, LED3,GPIO_RESET);
	delay_ms(ms);
}
