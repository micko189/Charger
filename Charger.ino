/*
* Inputs ADC Value from Thermistor and outputs Temperature in Celsius
* Utilizes the Steinhart-Hart Thermistor Equation:
*    Temperature in Kelvin = 1 / {A + B[ln(R)] + C[ln(R)]3}
*    where A = 0.001129148, B = 0.000234125 and C = 8.76741E-08
*
* Schematic:
*   [Ground] -- [10k-pad-resistor] -- | -- [thermistor] --[Vcc (5 or 3.3v)]
*                                     |
*                                Analog Pin 0
*
*/

#include <math.h>

#define NUMBER_OF_CELLS 2

#define ChargePIN 1						// Digital Pin 1
#define FastChargePIN 2					// Digital Pin 2
#define ThermistorPIN 0                 // Analog Pin 0
#define VoltagePIN 3					// Analog Pin 3

float pad = 9850;                       // balance/pad resistor value, set this to  the measured resistance of your pad resistor
float thermr = 10000;                   // thermistor nominal resistance

#define NUMSAMPLES 5

#define NORMAL_CHARGE 0
#define FAST_CHARGE 1
byte chargingMode = FAST_CHARGE;

bool bateryPresent = false;

float maxVoltage = 0;

unsigned long start_time_milis = 0;
unsigned long elapsed_time_milis = 0;

bool chargingEnded = false;

void setup()
{
	Serial.begin(9600);
	pinMode(ChargePIN, OUTPUT);
	pinMode(FastChargePIN, OUTPUT);
	 
	// Set both charging pins to LOW
	digitalWrite(ChargePIN, LOW);
	digitalWrite(FastChargePIN, LOW);
}

/// <summary>
/// Read analog value 5 times and retuns average.
/// </summary>
/// <param name="pin">The pin.</param>
/// <returns>Averafe analog raw value</returns>
float analogReadAvg(uint8_t pin)
{
	uint8_t i;
	float average = 0;

	// take N samples in a row, with a slight delay
	for (i = 0; i < NUMSAMPLES; i++) {
		average += analogRead(pin);
		if (i != NUMSAMPLES - 1)
		{
			delay(10); // Delay a bit between analog reed
		}
	}

	// average all the samples out
	average /= NUMSAMPLES;

	return average;
}

/// <summary>
/// Calculates temperature from raw analog value.
/// </summary>
/// <param name="RawADC">The raw analog value.</param>
/// <returns></returns>
float Thermistor(float RawADC) {
	float Resistance;
	float Temp;  // Dual-Purpose variable to save space.

	Resistance = pad * ((1023.0 / RawADC) - 1);
	Temp = log(Resistance); // Saving the Log(resistance) so not to calculate  it 4 times later
	Temp = 1 / (0.001129148 + (0.000234125 * Temp) + (0.0000000876741 * Temp * Temp * Temp));
	Temp = Temp - 273.15;  // Convert Kelvin to Celsius                      

	return Temp;	// Return the Temperature
}

/// <summary>
/// Calculates voltage from raw analog value.
/// </summary>
/// <param name="RawADC">The raw analog value.</param>
/// <returns></returns>
float Voltage(float RawADC)
{
	return RawADC * (5.0 / 1023.0);
}

void loop()
{
	float temperature = Thermistor(analogReadAvg(ThermistorPIN));

	Serial.print("Celsius: ");
	Serial.print(temperature, 2);                             // display Celsius
	Serial.println("C");

	float voltage = Voltage(analogReadAvg(VoltagePIN));

	Serial.print("Voltage: ");
	Serial.print(voltage, 3);                             // display Celsius
	Serial.println("V");

	if (voltage > maxVoltage)
	{
		// Set max voltage for later use
		maxVoltage = voltage;
	}

	if (bateryPresent == false && voltage > 0.5)
	{
		bateryPresent = true;
		// Update start clock time
		start_time_milis = millis();
	}
	else if ((bateryPresent == true && voltage  < 0.5) ||	// Battery is not present
		(bateryPresent == true && voltage  > 3))			// Someone pulled out batt during charge
	{
		bateryPresent = false;
	}

	if (bateryPresent)
	{
		if (!chargingEnded)
		{
			elapsed_time_milis = (millis() - start_time_milis);
		}

		if (elapsed_time_milis > 10 * 60 * 60 * 1000 ||	// More than 10 hours is elapsed
			voltage > NUMBER_OF_CELLS * 1.45 ||			// Voltage is more than 1.45V per cell
			temperature > 45.0 ||						// Temperature of cells is more than 45°C
			(maxVoltage - voltage) > 0.030				// We have voltage drop of more than 0.030V (30mV)
			)
		{
			int msec = elapsed_time_milis % 1000;
			int sec = (elapsed_time_milis / 1000) % 60;
			int min = ((elapsed_time_milis / 1000) / 60) % 60;
			int hour = (((elapsed_time_milis / 1000) / 60) / 60) % 60;
			Serial.print("End charging, charging time: ");
			Serial.print(hour, 1);
			Serial.print(':');
			Serial.print(min, 2);
			Serial.print(':');
			Serial.print(sec, 2);
			Serial.print(':');
			Serial.print(msec, 3);
			Serial.println("");
			// End charging
			digitalWrite(ChargePIN, LOW);
			digitalWrite(FastChargePIN, LOW);
			chargingEnded = true;
		}
		else
		{
			// Begin charge
			digitalWrite(ChargePIN, HIGH);
			chargingEnded = false;

			// If voltage is less than 1.4V per cell charge with C/5, if not charge with C/10
			if (voltage > NUMBER_OF_CELLS * 1.4)
			{
				chargingMode = NORMAL_CHARGE;
			}
			else
			{
				chargingMode = FAST_CHARGE;
			}

			// Set fast charge pin
			if (chargingMode == FAST_CHARGE)
			{
				digitalWrite(FastChargePIN, HIGH);
			}
			else
			{
				digitalWrite(FastChargePIN, LOW);
			}
		}
	}
	else if (start_time_milis > 0)
	{
		Serial.println("End charging, battery not present.");
		// No battery, end charging
		digitalWrite(ChargePIN, LOW);
		digitalWrite(FastChargePIN, LOW);
		chargingEnded = true;
	}

	delay(1000);	// Delay a bit... 
}
