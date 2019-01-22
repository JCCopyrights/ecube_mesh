#include "Arduino.h"
#include "ecube_led.h"
#include "ecube_buzzer.h"
	void initialize_buzzer(){
		pinMode(BUZZER, OUTPUT);
	}
	
	void play_melody(int* melody, int* noteDurations, int num_notes){
		 for (int thisNote = 0; thisNote < num_notes; thisNote++) {
			// to calculate the note duration, take one second divided by the note type.
			//e.g. quarter note = 1000 / 4, eighth note = 1000/8, etc.
			int noteDuration = 1000 / noteDurations[thisNote];
			tone(BUZZER, melody[thisNote], noteDuration);
			// to distinguish the notes, set a minimum time between them.
			// the note's duration + 30% seems to work well:
			int pauseBetweenNotes = noteDuration * 1.30;
			delay(pauseBetweenNotes);
			// stop the tone playing:
			noTone(15);
		}
	}	