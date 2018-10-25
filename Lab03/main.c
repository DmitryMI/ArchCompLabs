/* Monakhov D.I. IU7-54
Device controlls the oven. Oven consists of 3
heating elements. Functioning program:
1) Fast heating via all heaters for 10 seconds
2) Heating with one heater for 20 seconds each
3) Button turns oven off
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
#ifdef USE_TM
	tm1638_sendcmd(&tm, 0x46); // Sending READ Command
	
	received = tm1638_receivebyte(&tm);
#else
	int dir_backup = IODIR1;	
	return (IOPIN1 & BUTTON_PIN) != 0;
#endif
	return received; // We need only first button (SEG1)
}

// Sets state of selceted pin (or several pins)
void turn_heater(int heater_pin, enum pin_state state)
{
#ifdef USE_TM
	int dir_backup = IODIR1;
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
	
	IODIR1 = dir_backup;
	
#else
	switch(state)
	{
		case ENABLED:
			IOSET1 = heater_pin;
			break;
		
		case DISABLED:
			IOCLR1 = heater_pin;
			break;
	}
#endif
}

// Reads state of selected pin
enum pin_state read_pin(int pin)
{
	if((read_key_state() & 1) != 0)
		return ENABLED;
	
	return DISABLED;
}

void timer0_init(void)
{
	T0PR = 15000; // Timer PREDDELITEL

	T0TCR = 0x00000002; // Reset counter and divider

	T0MCR = 0x00000006; // Stop timer if match

	T0MR0 = 1000; // Match registry
}

int main(void)
{	
	// Variable to contain previous state of a button
	enum pin_state prev_button_state;
	
	// Current state of device (1 - turned on, 0 - turned off)
	int device_enabled = 0;
	
	int prev_enabled_state = 0;
		
	// Time to switch heater
#if USE_TM
	int cycle_tick_count = 1; // ~~ 1 second 1000000
#else
	int cycle_tick_count = 20000;
#endif
	
	// Current active heater pin
	int current_heater = HEATER1_PIN;
	
	// CURRENT STAGE OF WORK
	// 0 - Heating with all elements for 10 seconds
	// 1 - Heating with each element for 20 seconds
	int stage = 0;
	int prev_stage = -1;
	
#ifdef USE_TM
	
	tm.STB = 26;
	tm.CLK = 27;
	tm.DIO = 28;
	
	tm1638_init(&tm);
	
#else
	PINSEL1 = 0;
	IODIR1 = OUTPUT_PINS;
#endif

	timer0_init();

// Turning off all LEDS
	turn_heater(HEATER1_PIN, DISABLED);
	turn_heater(HEATER2_PIN, DISABLED);
	turn_heater(HEATER3_PIN, DISABLED);
	
	T0MR0 = cycle_tick_count;
	
	while(1)
	{
		enum pin_state button_state = read_pin(BUTTON_PIN);	
		
		if(button_state == DISABLED && prev_button_state == ENABLED)
		{
			device_enabled = !device_enabled;
		}
		
		prev_button_state = button_state;
		
		// Checking if device is enabled
		if(device_enabled)
		{
			if(stage == 0) // All heaters are working
			{
				if(prev_stage != stage)
				{
					T0TC = 0;
					T0TCR = 1;
					T0MR0 = cycle_tick_count / 2; // TODO 10 SECONDS
					
					turn_heater(HEATER1_PIN, ENABLED);
					turn_heater(HEATER2_PIN, ENABLED);
					turn_heater(HEATER3_PIN, ENABLED);					
				}
				
				if((T0TCR & 0x1) == 0) //  && prev_enabled_state == 1
				{
					stage = 1;
					turn_heater(HEATER2_PIN, DISABLED);
					turn_heater(HEATER3_PIN, DISABLED);
					current_heater = HEATER1_PIN;
					
					T0TC = 0;
					T0TCR = 1;
					T0MR0 = cycle_tick_count; // TODO 10 SECONDS
				}
				
				prev_stage = 0;
			}
			
			
			if(stage == 1)
			{
				// If timer was reset
				if((T0TCR & 0x1) != 0)
				{
					turn_heater(current_heater, ENABLED);						
				}			
				
				if((T0TCR & 0x1) == 0 && stage == prev_stage)
				{
					T0TC = 0;
					T0TCR = 1;				

					turn_heater(current_heater, DISABLED);
					
					if(current_heater == HEATER3_PIN)
					{
						current_heater = HEATER1_PIN;
						stage = 0; // Returning to intense heating stage
						prev_stage = 1;
						
						device_enabled = 0; // Self turning off
					}
					else
						current_heater >>= 1;	
				}
				
				prev_stage = 1;		
				
			}
				
			prev_enabled_state = 1;			
		}
		else
		{
			if(prev_enabled_state == 1)
			{
				turn_heater(HEATER1_PIN, DISABLED);
				turn_heater(HEATER2_PIN, DISABLED);
				turn_heater(HEATER3_PIN, DISABLED);
			}
			prev_enabled_state = 0;
			T0TC = 0;
		}		
	}
	
}

