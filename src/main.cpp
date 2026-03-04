#include "Arduino.h"
#include "heltec.h"

#define BOTON 0
#define MI_LED 2

volatile bool botonPulsado = false;
bool ledEncendido = false;

void IRAM_ATTR isrBoton()
{
	botonPulsado = true;
}

void setup()
{
	Heltec.begin(false, false, true, false);
	pinMode(BOTON, INPUT_PULLUP);
	pinMode(MI_LED, OUTPUT);
	digitalWrite(MI_LED, LOW);
	attachInterrupt(digitalPinToInterrupt(BOTON), isrBoton, FALLING);
}

void loop()
{
	if (botonPulsado)
	{
		delay(50);
		botonPulsado = false;
		ledEncendido = !ledEncendido;
		digitalWrite(MI_LED, ledEncendido ? HIGH : LOW);
		Serial.print("LED: ");
		Serial.println(ledEncendido ? "ENCENDIDO" : "APAGADO");
	}
}
