#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#ifndef _ECUBE_SSD1306_H_
#define _ECUBE_SSD1306_H_

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels
#define OLED_RESET     -1 // Reset pin # (or -1 if sharing Arduino reset pin)
  
	void initialize_ssd1306(Adafruit_SSD1306 &display);
	void test_ssd1306(Adafruit_SSD1306 &display);
  
#endif 
