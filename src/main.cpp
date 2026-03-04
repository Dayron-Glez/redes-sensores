#include "Arduino.h"
#include "heltec.h"

#define BOTON 0
#define MI_LED 2
#define INTERVALO_TEMP 5000

volatile bool botonPulsado = false;
bool ledEncendido = false;
unsigned long ultimaLectura = 0;
float temperatura = 20.0;

void IRAM_ATTR isrBoton()
{
	botonPulsado = true;
}

float leerTemperatura()
{
	return 15.0 + (random(0, 151) / 10.0);
}

void actualizarDisplay()
{
	Heltec.display->clear();
	if (ledEncendido)
	{
		Heltec.display->setFont(ArialMT_Plain_10);
		Heltec.display->drawString(0, 0, "Termostato IoT");
		Heltec.display->setFont(ArialMT_Plain_24);
		Heltec.display->drawString(0, 15, String(temperatura, 1) + " °C");
		Heltec.display->setFont(ArialMT_Plain_16);
		Heltec.display->drawString(0, 45, "ENCENDIDO");
	}
	Heltec.display->display();

	pinMode(BOTON, INPUT_PULLUP);
}

void setup()
{
	Heltec.begin(true, false, true, false);
	pinMode(BOTON, INPUT_PULLUP);
	pinMode(MI_LED, OUTPUT);
	digitalWrite(MI_LED, LOW);
	attachInterrupt(digitalPinToInterrupt(BOTON), isrBoton, FALLING);
	randomSeed(analogRead(0));
	temperatura = leerTemperatura();
	actualizarDisplay();
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
		Serial.print("Estado: ");
		Serial.println(ledEncendido ? "ENCENDIDO" : "APAGADO");
		actualizarDisplay();
		attachInterrupt(digitalPinToInterrupt(BOTON), isrBoton, FALLING);
	}

	if (millis() - ultimaLectura >= INTERVALO_TEMP)
	{
		ultimaLectura = millis();
		temperatura = leerTemperatura();
		Serial.print("Temperatura: ");
		Serial.print(temperatura, 1);
		Serial.println(" °C");
		actualizarDisplay();
		attachInterrupt(digitalPinToInterrupt(BOTON), isrBoton, FALLING);
	}
}
