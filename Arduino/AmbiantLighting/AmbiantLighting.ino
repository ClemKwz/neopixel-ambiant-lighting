#include <Adafruit_NeoPixel.h>

// Connected pin
#define PIN            6

// Number of leds
#define LEDS_COUNT      30

Adafruit_NeoPixel pixels = Adafruit_NeoPixel(LEDS_COUNT, PIN, NEO_GRB + NEO_KHZ800);

//-----------------------------------------------------------
void setup()
{
	// Init NeoPixel & Serial
	pixels.begin();
	pixels.show();
	Serial.begin(57600);
}

//-----------------------------------------------------------
void loop()
{     
	// We have to read only 15 leds because serial can only have 64 bytes buffered
	// We use different command (A & B) to divide data to read
	// We only call pixels.show() at the end of the 'B' command reading to update all leds
	while(Serial.available() >= 46)
	{
		uint8_t dCmd = Serial.read();
		switch (dCmd)
		{
			case 'A':
				for(int nCpt = 0;nCpt < 15;++nCpt)
				{
					uint8_t dR = Serial.read();
					uint8_t dG = Serial.read();
					uint8_t dB = Serial.read();
					pixels.setPixelColor(nCpt, pixels.Color(dR, dG, dB));
				}
			break;
			case 'B':
				for(int nCpt = 15;nCpt < 30;++nCpt)
				{
					uint8_t dR = Serial.read();
					uint8_t dG = Serial.read();
					uint8_t dB = Serial.read();
					pixels.setPixelColor(nCpt, pixels.Color(dR, dG, dB));
				}
				pixels.show();
			break;
			default:
			break;
		}
	}
}
