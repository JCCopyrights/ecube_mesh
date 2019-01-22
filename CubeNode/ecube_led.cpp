#include "Arduino.h"
#include "ecube_led.h"

	void initialize_led(){
		pinMode(LEDINT, OUTPUT);
		pinMode(LED0, OUTPUT);
		pinMode(LED1, OUTPUT);
		pinMode(LED2, OUTPUT);
	}
	
	void turn_led(uint8_t led_number, uint8_t led_state){
		led_state=(led_number==LEDINT)?!led_state:led_state;
		digitalWrite(led_number, led_state);
	}	

	void toggle_led(uint8_t led_number){
        turn_led(led_number,!digitalRead(led_number));  
	}
