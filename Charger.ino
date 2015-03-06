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

#define NORMAL_CHARGE 0
#define FAST_CHARGE 1

byte chargingMode = FAST_CHARGE;

bool bateryPresent = false;

float maxVoltage = 0;

unsigned long start_time_milis = 0;

float Thermistor(float RawADC) {
	long Resistance;
	float Temp;  // Dual-Purpose variable to save space.

	Resistance = pad * ((1023.0 / RawADC) - 1);
	Temp = log((float)Resistance); // Saving the Log(resistance) so not to calculate  it 4 times later
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
		samples[i] = analogRead(pin);
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
	float temperature = Thermistor(analogReadAvg(ThermistorPIN));

	Serial.print("Celsius: ");
	Serial.print(temperature, 2);                             // display Celsius
	Serial.println("C");

	float voltage = Voltage(analogReadAvg(VoltagePIN));

	Serial.print("Voltage: ");
	Serial.print(voltage, 3);                             // display Celsius
	Serial.println("V");

	if (voltage > maxVoltage )
	{
		maxVoltage = voltage;
	}

	if (bateryPresent == false && voltage > 0.5)
	{
		bateryPresent = true;
		// Update start clock time
		start_time_milis = millis();
	}
	else if (bateryPresent == true && voltage  < 0.5 || bateryPresent == true && voltage  > 3) // Battery is not present or someone pulled out batt during charge
	{
		bateryPresent = false;
	}

	if (bateryPresent)
	{
		if ((millis() - start_time_milis) > 10 * 60 * 60 * 1000 ||	// More than 10 hours elapsed
			voltage > 2.9 ||										// Voltage more than 1.45V per cell
			temperature > 45 ||										// Temperature of cells more than 45°C
			(maxVoltage - voltage) > 0.030							// Ve have voltage drop of more than 0.030V (30mV)
			)
		{
			Serial.println("End charging.                           ");
			// End charging
			digitalWrite(ChargePIN, LOW);
			digitalWrite(FastChargePIN, LOW);
		}
		else
		{
			// Begin charge
			digitalWrite(ChargePIN, HIGH);

			// If voltage is less than 1.4V per cell charge with C/5, if not charge with C/10
			if (voltage > 2.8)
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
	}

	delay(1000);                                      // Delay a bit... 
}


