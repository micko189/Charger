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

byte pins[10] = { 1, 1, 1, 1, 1, 1, 1, 1, 1, 1 };
unsigned long lastPinChange[10] = { 0 };

void pinMode(uint8_t, uint8_t){}

void digitalWrite(uint8_t, uint8_t){}

int digitalRead(uint8_t pin)
{
	return pins[pin];
}

int apins[10] = { 0 };

int analogRead(uint8_t pin){ return apins[pin]; }

void analogWrite(uint8_t pin, int val){ apins[pin] = val; }

#include "Charger.ino"

void GotoXY(byte x, byte y)
{
	COORD coord;
	coord.X = x;
	coord.Y = y;
	SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), coord);
}

void main()
{
	setup();
	analogWrite(ThermistorPIN, 600);
	analogWrite(VoltagePIN, 600);
	for (;;)
	{
		loop();
		GotoXY(0, 0);
	}
}