#include <stdlib.h>

#include "./drivers/inc/vga.h"
#include "./drivers/inc/ISRs.h"
#include "./drivers/inc/LEDs.h"
#include "./drivers/inc/audio.h"
#include "./drivers/inc/HPS_TIM.h"
#include "./drivers/inc/int_setup.h"
#include "./drivers/inc/wavetable.h"
#include "./drivers/inc/pushbuttons.h"
#include "./drivers/inc/ps2_keyboard.h"
#include "./drivers/inc/HEX_displays.h"
#include "./drivers/inc/slider_switches.h"

//array to store the keys currently pressed
char keysPressed[8] = {};
//array holding the frequencies, index matched to the keys pressed
float frequencies[] = {130.813, 146.832, 164.814, 174.614, 195.998, 220.000, 246.942, 261.626};

// Get the sample based on the frequency and the "index"
// Returns double: signal
double getSampleOld(float freq, int t) {

	int index = (((int)freq) * t)%48000;
	double signal = sine[index];

	return signal;
}

// Get the sample based on the frequency and the "index" using linear interpolation
// Returns double: signal
double getSample(float freq, int t) {
	int truncatedIndex = ((int) freq)*t;
	double fractional = (freq*t) - truncatedIndex;

	int index = truncatedIndex % 48000;
	//calculate linear interpolation
	//sine[casted + fractional] = (1-fractional)*sine[index] + fractional[index+1]
	double signal = (1.0 - fractional) * sine[index] + fractional * sine[index + 1]; //lol lets hope it doesnt overflow


	return signal;
}

// Generate the signal from each frequency pressed and add them together
// Returns double: summed signal
double generateSignal(char* keys, int t) {
	int i;
	double data = 0;
	// Loop through all keys
	for(i = 0; i < 8; i++){
		// Check if key is pressed
		if(keys[i] == 1){
			// Sum all frequency samples
			data += getSampleOld(frequencies[i], t);
			//data += getSample(frequencies[i], t);
		}
	}
	return data;
}

// Write names at the top of the screen
void drawWelcome(){
	//Screen is 79 x 59
	VGA_write_char_ASM(34, 2, 'T');
	VGA_write_char_ASM(35, 2, 'h');
	VGA_write_char_ASM(36, 2, 'o');
	VGA_write_char_ASM(37, 2, 'm');
	VGA_write_char_ASM(38, 2, 'a');
	VGA_write_char_ASM(39, 2, 's');

	VGA_write_char_ASM(41, 2, 'H');
	VGA_write_char_ASM(42, 2, 'i');
	VGA_write_char_ASM(43, 2, 'l');
	VGA_write_char_ASM(44, 2, 'l');
	VGA_write_char_ASM(45, 2, 'y');
	VGA_write_char_ASM(46, 2, 'e');
	VGA_write_char_ASM(47, 2, 'r');

	VGA_write_char_ASM(36, 4, 'O');
	VGA_write_char_ASM(37, 4, 'm');
	VGA_write_char_ASM(38, 4, 'a');
	VGA_write_char_ASM(39, 4, 'r');

	VGA_write_char_ASM(40, 4, 'Y');
	VGA_write_char_ASM(41, 4, 'a');
	VGA_write_char_ASM(42, 4, 'm');
	VGA_write_char_ASM(43, 4, 'a');
	VGA_write_char_ASM(44, 4, 'k');
}

void drawWords(){

	VGA_write_char_ASM(70, 59, 'V');
	VGA_write_char_ASM(71, 59, 'o');
	VGA_write_char_ASM(72, 59, 'l');
	VGA_write_char_ASM(73, 59, 'u');
	VGA_write_char_ASM(74, 59, 'm');
	VGA_write_char_ASM(75, 59, 'e');
	VGA_write_char_ASM(76, 59, ':');

}

int main() {
	// Setup timer
	int_setup(1, (int []){199});
	HPS_TIM_config_t hps_tim;
	hps_tim.tim = TIM0; //microsecond timer
	hps_tim.timeout = 20; //1/48000 = 20.8
	hps_tim.LD_en = 1; // initial count value
	hps_tim.INT_en = 1; //enabling the interrupt
	hps_tim.enable = 1; //enable bit to 1

	HPS_TIM_config_ASM(&hps_tim);
	
	// whether a key has been released
	char keyReleased = 0;
	// counter for signal
	int t = 0;
	// to store the previous set of drawn points for quicker clearing
	double history[320] = { 0 };
	//double valToDraw = 0;

	char value;

	char amplitude = 1;
	double signalSum = 0.0;

	drawWords();

	while(1) {
		if(read_slider_switches_ASM() > 0) {
			drawWelcome();
		}
		else{
				if (read_ps2_data_ASM(&value)) {
					switch (value){
				// Key = Note = Frequency
				// A = C = 130.813Hz
						case 0x1C:
							if(keyReleased == 1){
								// printf( "a release\n" );
								keysPressed[0] = 0;
								keyReleased = 0;
							} else{
								// printf( "a press\n" );
								keysPressed[0] = 1;
							}
							break;
				// S = D = 146.832Hz
						case 0x1B:
							if(keyReleased == 1){
								// printf( "s release\n" );
								keysPressed[1] = 0;
								keyReleased = 0;
							} else{
								// printf( "s press\n" );
								keysPressed[1] = 1;
							}
							break;
				// D = E = 164.814Hz
						case 0x23:
							if(keyReleased == 1){
								keysPressed[2] = 0;
								keyReleased = 0;
							} else{
								keysPressed[2] = 1;
							}
							break;
				// F = F = 174.614Hz
						case 0x2B:
							if(keyReleased == 1){
								keysPressed[3] = 0;
								keyReleased = 0;
							} else{
								keysPressed[3] = 1;
							}
							break;
				// J = G = 195.998Hz
						case 0x3B:
							if(keyReleased == 1){
								keysPressed[4] = 0;
								keyReleased = 0;
							} else{
								keysPressed[4] = 1;
							}
							break;
				// K = A = 220.000Hz
						case 0x42:
							if(keyReleased == 1){
								keysPressed[5] = 0;
								keyReleased = 0;
							} else{
								keysPressed[5] = 1;
							}
							break;
				// L = B = 246.942Hz
						case 0x4B:
							if(keyReleased == 1){
								keysPressed[6] = 0;
								keyReleased = 0;
							} else{
								keysPressed[6] = 1;
							}
							break;
				// ; = C = 261.626Hz
						case 0x4C:
							if(keyReleased == 1){
								keysPressed[7] = 0;
								keyReleased = 0;
							}else{
								keysPressed[7] = 1;
							}
							break;
						//volume up 
						case 0x49:
							if(keyReleased == 1){
								if(amplitude<10)
									amplitude++;
								keyReleased = 0;
							}
							break;
						//volume down
						case 0x41:
							if(keyReleased == 1){
								if(amplitude>0)
									amplitude--;
								keyReleased = 0;
							}
							break;
						case 0xF0: //the break code is the same for all keys
							keyReleased = 1;
							break;
						default:
							keyReleased = 0;
					}
				}
			}
			

			signalSum = generateSignal(keysPressed, t); //generate the signal at this t based on what keys were pressed

			signalSum = amplitude * signalSum; //this is volume control

			// Every 20 microseconds this flag goes high
			if(hps_tim0_int_flag == 1) {
				hps_tim0_int_flag = 0;
				audio_write_data_ASM(signalSum, signalSum);
				t++;
			}

			int drawIndex = 0;
			double valToDraw = 0;
			// To reduce the number of drawing operations
			if((t%10 == 0)){
				//draw volume number in bottom right
				if(amplitude == 10){
					VGA_write_byte_ASM(78, 59, 16);
				} else {
					//volume = 0-9
					VGA_write_byte_ASM(78, 59, amplitude);
				}

				drawIndex = (t/10)%320;
				//clear drawn points
				VGA_draw_point_ASM(drawIndex, history[drawIndex], 0);
				//120 centers the signal on the screen, 500000 is abitrary to make it fit
				valToDraw = 120 + signalSum/500000;
				//add new points to history array
				history[drawIndex] = valToDraw;
				//draw new points
				VGA_draw_point_ASM(drawIndex, valToDraw, 63);		
			}
			
			// Reset the signal
			signalSum = 0;
			// Reset the counter
			if(t==48000){
				t=0;
			}
		
	}


	return 0;
}
