/* Monakhov D.I. IU7-54
Device controlls the oven. Oven consists of 3
heating elements. Functioning program:
1) Heating via each at once heating element
2) Pushing button turns oven off
*/

// LPC2300.s EMC_SETPU EQU 1. Line 435 - changed to make project to 
// work in emulation

#include <LPC23xx.H>
#include "controller.h"

#define OUTPUT_PINS 0x38000000 // Bit mask for all output pins
#define HEATER1_PIN 0x20000000 // Bit mask of first heater
#define HEATER2_PIN 0x10000000 // Second heater
#define HEATER3_PIN 0x8000000 // Third heater
#define BUTTON_PIN 0x4000000 // Button

struct tm1638 tm;

// Describes binary pin state
enum pin_state
{
	ENABLED = 1, DISABLED = 0
};

void set_led_enabled(int led_num, int enabled)
{	
	int n = led_num * 2 + 1;
	tm1638_setadr(&tm, n);
	
	if(enabled)		
		tm1638_sendbyte(&tm, n);
	else
		tm1638_sendbyte(&tm, 0);
}

int read_key_state()
{
	int received = 0;
	tm1638_sendcmd(&tm, 0x46); // Sending READ Command
	
	received = tm1638_receivebyte(&tm);
	return received; // We need only first button (SEG1)
}

// Sets state of selceted pin (or several pins)
void turn_heater(int heater_pin, enum pin_state state)
{
	int led_num;
	switch(heater_pin)
	{
		case HEATER1_PIN:
			led_num = 0;
			break;
		case HEATER2_PIN:
			led_num = 1;
			break;
		case HEATER3_PIN:
			led_num = 2;
			break;
		
		default: led_num = 0;
	}
	
	switch(state)
	{
		case ENABLED:
			set_led_enabled(led_num, 1);
			break;
		
		case DISABLED:
			set_led_enabled(led_num, 0);
			break;
	}
}

// Reads state of selected pin
enum pin_state read_pin(int pin)
{
	if((read_key_state() & 1) != 0)
		return ENABLED;
	
	return DISABLED;
}

int main(void)
{	
	// Variable to contain previous state of a button
	enum pin_state prev_button_state;
	
	// Current state of device (1 - turned on, 0 - turned off)
	int device_enabled = 1;
	
	int prev_enabled_state = 0;
	
	// Timer
	int current_tick = 0;
	
	// Time to switch heater
	int cycle_tick_count = 1; // ~~ 1 second 1000000
	
	// Current active heater pin
	int current_heater = HEATER1_PIN;
	
	tm.STB = 26;
	tm.CLK = 27;
	tm.DIO = 28;
	
	tm1638_init(&tm);

// Turning off all LEDS
		turn_heater(HEATER1_PIN, DISABLED);
		turn_heater(HEATER2_PIN, DISABLED);
		turn_heater(HEATER3_PIN, DISABLED);

	while(1)
	{
		// Checking button
		enum pin_state button_state = read_pin(BUTTON_PIN);	
		
		
		//button_state = DISABLED; // FIXME
		
		if(button_state == DISABLED && prev_button_state == ENABLED)
		{
			// Switching device state
			device_enabled = !device_enabled;
		}
		
		// Updating previous button state
		prev_button_state = button_state;
		
		// Checking if device is enabled
		if(device_enabled)
		{
			// If timer was reset
			if(current_tick == 0)
			{
				// Enabling current heated
				turn_heater(current_heater, ENABLED);		
			}			
			
			// Checking how much time past since timer reset
			if(current_tick >= cycle_tick_count)
			{
				// Reset timer
				current_tick = 0;
				
				// Turn current heater off
				turn_heater(current_heater, DISABLED);
				
				// Switch heater to next
				if(current_heater == HEATER3_PIN)
					current_heater = HEATER1_PIN;
				else
					current_heater >>= 1;	
			}
			else			
				current_tick++; // Timer increment
			
			prev_enabled_state = 1;
		}
		else // The device if turned off
		{
			//turn_heater(HEATER1_PIN | HEATER2_PIN | HEATER3_PIN, DISABLED); // Turn off all heaters
			if(prev_enabled_state == 1)
			{
				turn_heater(HEATER1_PIN, DISABLED);
				turn_heater(HEATER2_PIN, DISABLED);
				turn_heater(HEATER3_PIN, DISABLED);
			}
			prev_enabled_state = 0;
			current_tick = 0; // Reset timer
		}		
	}
	
}

