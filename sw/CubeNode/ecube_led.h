#ifndef _ECUBE_LED_H_
#define _ECUBE_LED_H_
	#define LED1	12 
	#define LED2	13
	#define LED0	14
	#define LEDINT	2

	void initialize_led();

	void turn_led(uint8_t led_number, uint8_t led_state);	

	void toggle_led(uint8_t led_number);

#endif 
