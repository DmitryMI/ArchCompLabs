#include <LPC23xx.H>
#include "controller.h"

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

struct tm1638 tm;

int check_input()
{
	tm1638_setadr(&tm, 0);
	tm1638_sendcmd(&tm, 0x46); // Sending cmd to READ
	
	return tm1638_receivebyte(&tm) & (1); // We need only first button (SEG1)
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

void enable(int ledNum)
{
	// Send enable command to LED[N]
	
	tm1638_sendcmd(&tm, 0x40);
	tm1638_setadr(&tm, ledNum);	
	tm1638_sendbyte(&tm, 1);
}

void enable_all()
{
	// Send enable command to LED1, LED2, LED3
	
	
}

int main(void)
{	
	
	unsigned int n, i;
	int received;
	
	tm.STB = 26;
	tm.CLK = 27;
	tm.DIO = 28;
	
	tm1638_init(&tm);	
	
	
	while(1)
	{
		for(n = 1; n <= 5; n += 2)
		{
			i = 1;
			while(i != 0)
			{
				tm1638_sendcmd(&tm, 0x46);	
				received = tm1638_receivebyte(&tm);
				i = received & 0x4B0; // 4th button - 2nd byte, 4th bit // Probably step over to
			}
			
			tm1638_setadr(&tm, n);
			tm1638_sendbyte(&tm, n);
			delay(0xffff);
			tm1638_sendbyte(&tm, 0);
		}
	}
	
}
