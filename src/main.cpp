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

void actualizarDisplay()
{
	Heltec.display->clear();
	Heltec.display->setFont(ArialMT_Plain_16);
	if (ledEncendido)
	{
		Heltec.display->drawString(0, 25, "ENCENDIDO");
	}
	else
	{
		Heltec.display->drawString(0, 25, "APAGADO");
	}
	Heltec.display->display();
}

void setup()
{
	Heltec.begin(true, false, true, false);
	pinMode(BOTON, INPUT_PULLUP);
	pinMode(MI_LED, OUTPUT);
	digitalWrite(MI_LED, LOW);
	attachInterrupt(digitalPinToInterrupt(BOTON), isrBoton, FALLING);
	actualizarDisplay();
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
		actualizarDisplay();
	}
}
