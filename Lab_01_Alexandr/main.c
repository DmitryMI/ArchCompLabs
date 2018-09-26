#include <LPC23xx.H>

// Pin 29 - rotor
// Pin 28 - Heater
// Pin 27 - Pump
// Pin 26 - INPUT Button

#define PRESET_OUTPUT 0x38000000 // 29, 28, 27
#define INITIAL_SET 0x38000000 // 29, 28, 27
#define INPUT_PORT_MASK 0x4000000 // 26
#define PUMP_PIN_MASK 0x8000000 // 27

int pump_enabled = 1;
int prev_button_state = 0;

int check_input()
{
	return IOPIN0 & INPUT_PORT_MASK;
}

void set_pump_output(int enabled)
{
	if(enabled)
	{
		IOSET0 = PUMP_PIN_MASK;
	}
	else
	{
		IOCLR0 = PUMP_PIN_MASK;
	}
}

int main(void)
{
	PINSEL0 = 0; // Configure functions of I/O port 1 to GPIO
	
	IODIR0 = PRESET_OUTPUT; // Set 29 28 27 to output, other to input

	IOSET0 = INITIAL_SET; // Set all pins to 1
	
	while(1)
	{
		if(check_input())
		{
			// Button is presed
			
			prev_button_state = 1;			
		}		
		else
		{
			
			if(prev_button_state)
			{
				pump_enabled = !pump_enabled;
			}
			
			prev_button_state = 0;
		}
		
		set_pump_output(pump_enabled);
	}
}
