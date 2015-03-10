#include <windows.h>
#include "Serial.h"

SerialClass Serial;

#define INPUT 0x0
#define OUTPUT 0x1
#define INPUT_PULLUP 0x2

typedef unsigned char byte;
typedef unsigned char boolean;
typedef unsigned char uint8_t;
#define HIGH 1
#define LOW 0

unsigned long millis(void)
{
	return GetTickCount();
}

void delay(unsigned long t)
{
	Sleep(t);
}

byte pins[10] = { 0 };

void pinMode(uint8_t, uint8_t){}

void digitalWrite(uint8_t pin, uint8_t val)
{
	pins[pin] = val;
}

int digitalRead(uint8_t pin)
{
	return pins[pin];
}

int apins[10] = { 0 };

int analogRead(uint8_t pin)
{
	return apins[pin];
}

void analogWrite(uint8_t pin, int val)
{
	apins[pin] = val;
}

#include "Charger.ino"

void GotoXY(byte x, byte y)
{
	COORD coord;
	coord.X = x;
	coord.Y = y;
	SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), coord);
}

#define VoltageThreshold 591 // If 591 simulate overcharge voltage drop, if grater no overcharge simulation

void main()
{
	bool notInOvercharge = true;
	setup();
	analogWrite(ThermistorPIN, 0);
	analogWrite(VoltagePIN, 0);

	int cnt = 0;

	int startVol = 500;
	int startTmp = 620;


	for (;;)
	{
		printf("Seconds elapsed: %ds\n", cnt);

		if (cnt >= 4)
		{
			analogWrite(VoltagePIN, startVol);
		}

		analogWrite(ThermistorPIN, startTmp);

		loop();

		if (notInOvercharge && analogRead(VoltagePIN) <= VoltageThreshold) // Simulate voltage drop when overcharging
		{
			if (digitalRead(ChargePIN))
			{
				startVol++;
				startTmp++;
			}

			if (digitalRead(FastChargePIN))
			{
				startVol++;
				startTmp++;
			}
		}
		else
		{
			notInOvercharge = false;
			if (digitalRead(ChargePIN))
			{
				startVol--;
				startTmp++;
			}
		}

		printf("Bat pres: %d, charging: %d, fast: %d \n", bateryPresent, digitalRead(ChargePIN), digitalRead(FastChargePIN));

		GotoXY(0, 0);

		cnt++;
	}
}