/*
* Inputs ADC Value from Thermistor and outputs Temperature in Celsius
*  requires: include <math.h>
* Utilizes the Steinhart-Hart Thermistor Equation:
*    Temperature in Kelvin = 1 / {A + B[ln(R)] + C[ln(R)]3}
*    where A = 0.001129148, B = 0.000234125 and C = 8.76741E-08
*
* These coefficients seem to work fairly universally, which is a bit of a
* surprise.
*
* Schematic:
*   [Ground] -- [10k-pad-resistor] -- | -- [thermistor] --[Vcc (5 or 3.3v)]
*                                     |
*                                Analog Pin 0
*
* In case it isn't obvious (as it wasn't to me until I thought about it), the analog ports
* measure the voltage between 0v -> Vcc which for an Arduino is a nominal 5v, but for (say)
* a JeeNode, is a nominal 3.3v.
*
* The resistance calculation uses the ratio of the two resistors, so the voltage
* specified above is really only required for the debugging that is commented out below
*
* Resistance = PadResistor * (1023/ADC -1)
*
*/

#include <math.h>

#define ThermistorPIN 0                 // Analog Pin 0
#define ChargePIN 1
#define FastChargePIN 2
#define VoltagePIN 3					// Analog Pin 3

float pad = 9850;                       // balance/pad resistor value, set this to  the measured resistance of your pad resistor
float thermr = 10000;                   // thermistor nominal resistance

#define NUMSAMPLES 5

int samples[NUMSAMPLES];

#define FAST_CHARGE 0
#define NORMAL_CHARGE 1

byte chargingMode = FAST_CHARGE;

bool bateryPresent = false;

unsigned long current_time_milis;

float Thermistor(float RawADC) {
	long Resistance;
	float Temp;  // Dual-Purpose variable to save space.

	Resistance = pad * ((1023.0 / RawADC) - 1);
	Temp = log(Resistance); // Saving the Log(resistance) so not to calculate  it 4 times later
	Temp = 1 / (0.001129148 + (0.000234125 * Temp) + (0.0000000876741 * Temp * Temp * Temp));
	Temp = Temp - 273.15;  // Convert Kelvin to Celsius                      

	// BEGIN- Remove these lines for the function not to display anything
	//Serial.print("ADC: "); 
	//Serial.print(RawADC); 
	//Serial.print(", pad: ");
	//Serial.print(pad/1000,3);
	//Serial.print(" Kohms, Volts: "); 
	//Serial.print(((RawADC*vcc)/1024.0),3);   
	//Serial.print(", Resistance: "); 
	//Serial.print(Resistance);
	//Serial.print(" ohms, ");
	// END- Remove these lines for the function not to display anything

	return Temp;                                      // Return the Temperature
}

void setup()
{
	Serial.begin(9600);
	pinMode(ChargePIN, OUTPUT);
	pinMode(FastChargePIN, OUTPUT);

	digitalWrite(ChargePIN, LOW);
	digitalWrite(FastChargePIN, LOW);
}

float analogReadAvg(uint8_t pin)
{
	uint8_t i;
	float average;

	// take N samples in a row, with a slight delay
	for (i = 0; i < NUMSAMPLES; i++) {
		samples[i] = analogRead(ThermistorPIN);
		delay(10);
	}

	// average all the samples out
	average = 0;
	for (i = 0; i < NUMSAMPLES; i++) {
		average += samples[i];
	}

	average /= NUMSAMPLES;

	return average;
}

float Voltage(float RawADC)
{
	return RawADC * (5.0 / 1023.0);
}

void loop()
{
	float temp = Thermistor(analogReadAvg(ThermistorPIN));      

	Serial.print("Celsius: ");
	Serial.print(temp, 2);                             // display Celsius
	Serial.println("");

	float voltage = Voltage(analogReadAvg(VoltagePIN));

	Serial.print("Voltage: ");
	Serial.print(voltage, 2);                             // display Celsius
	Serial.println("");

	if (bateryPresent == false && voltage > 0.1)
	{
		bateryPresent = true;
		// Update clock time
		current_time_milis = millis();
	}
	else
	{
		bateryPresent = false;
	}

	if (bateryPresent)
	{
		if (voltage > 2.8)
		{
			chargingMode = NORMAL_CHARGE;
		}
		else
		{
			chargingMode = FAST_CHARGE;
		}

		// begin charge
	}

	delay(1000);                                      // Delay a bit... 
}

