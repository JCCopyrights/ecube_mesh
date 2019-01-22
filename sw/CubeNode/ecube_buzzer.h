#ifndef _ECUBE_BUZZER_H_
#define _ECUBE_BUZZER_H_
	#define BUZZER	15 

	void initialize_buzzer();

	void play_melody(int* melody, int* noteDurations, int num_notes);	

#endif 