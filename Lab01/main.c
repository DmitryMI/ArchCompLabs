/* Monakhov D.I. IU7-54
Device controlls the oven. Oven consists of 3
heating elements. Functioning program:
1) Heating via each at once heating element
2) Pushing button turns oven off
*/

#include <LPC23xx.H>

#define OUTPUT_PINS 0x38000000 // Bit mask for all output pins
#define HEATER1_PIN 0x20000000 // Bit mask of first heater
#define HEATER2_PIN 0x10000000 // Second heater
#define HEATER3_PIN 0x8000000 // Third heater
#define BUTTON_PIN 0x4000000 // Button

// Describes binary pin state
enum pin_state
{
	ENABLED = 1, DISABLED = 0
};

// Sets state of selceted pin (or several pins)
void turn_heater(int heater_pin, enum pin_state state)
{
	switch(state)
	{
		case ENABLED:
			IOSET0 = heater_pin;
			break;
		
		case DISABLED:
			IOCLR0 = heater_pin;
			break;
	}
}

// Reads state of selected pin
enum pin_state read_pin(int pin)
{
	if((IOPIN0 & pin) != 0)
		return ENABLED;
	return DISABLED;
}

int main(void)
{
	// Variable to contain previous state of a button
	enum pin_state prev_button_state;
	
	// Current state of device (1 - turned on, 0 - turned off)
	int device_enabled = 1;
	
	// Timer
	int current_tick = 0;
	
	// Time to switch heater
	int cycle_tick_count = 1000000; // ~~ 1 second
	
	// Current active heater pin
	int current_heater = HEATER1_PIN;
	
	// Set controll to General Purpose Input/Outpu
	PINSEL0 = 0;
	
	/// Set output direction for heaters' pins
	IODIR0 = OUTPUT_PINS;

	while(1)
	{
		// Checking button
		enum pin_state button_state = read_pin(BUTTON_PIN);			
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
		}
		else // The device if turned off
		{
			turn_heater(HEATER1_PIN | HEATER2_PIN | HEATER3_PIN, DISABLED); // Turn off all heaters
			current_tick = 0; // Reset timer
		}		
	}
	
}

