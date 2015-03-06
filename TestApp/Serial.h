#ifndef _SERIAL_H_
#define _SERIAL_H_

#include <stdio.h>

class SerialClass
{
public:
	void begin(int i){}
	void print(char* s)
	{
		printf("%s", s);
	}
	void print(int i)
	{
		printf("%d", i);
	}
	void print(char c)
	{
		printf("%c", c);
	}
	void print(float f, int i)
	{
		char format[] = "%. f";
		format[2] = i + 48;
		printf(format, f);
	}
	void println(char* s)
	{
		printf("%s\n", s);
	}
	void println(int i)
	{
		printf("%d\n", i);
	}
};

extern SerialClass Serial;

#endif