#include <Adafruit_NeoPixel.h>

// Connected pin
#define PIN            6

// Number of leds
#define LED_COUNT      30

Adafruit_NeoPixel pixels = Adafruit_NeoPixel(LED_COUNT, PIN, NEO_GRB + NEO_KHZ800);

//-----------------------------------------------------------
void setup()
{
	// Init NeoPixel & Serial
	pixels.begin();
	Serial.begin(57600);
}

//-----------------------------------------------------------
void loop()
{
	while(Serial.available() >= 4)
	{
		// Read command
		uint8_t dCmd		= Serial.read();
		uint8_t dR			= Serial.read();
		uint8_t dG			= Serial.read();
		uint8_t dB			= Serial.read();
		switch (dCmd)
		{
			case 'C':
			{
				for(int nCpt = 0;nCpt < LED_COUNT;++nCpt)
				{
					pixels.setPixelColor(nCpt, pixels.Color(dR, dG, dB));
				}
				pixels.show();
			}
			break;
			default:
			break;
		}
	}
}

