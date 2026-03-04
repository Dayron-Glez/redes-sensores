#include "Arduino.h"
#include "heltec.h"

#define BOTON 0
#define MI_LED 2
#define INTERVALO_TEMP 2000

#define TEMP_CONSIGNA 22.0
#define HISTERESIS 1.0
#define TEMP_BAJA (TEMP_CONSIGNA - HISTERESIS)
#define TEMP_ALTA (TEMP_CONSIGNA + HISTERESIS)

volatile bool botonPulsado = false;
bool termostatoEncendido = false;
bool calefaccionActiva = false;
unsigned long ultimaLectura = 0;
float temperatura = 20.0;

void IRAM_ATTR isrBoton()
{
	botonPulsado = true;
}

float leerTemperatura()
{

	float salto = 0.5 + random(0, 101) / 100.0;
	temperatura += calefaccionActiva ? salto : -salto;

	temperatura = constrain(temperatura, 10.0, 35.0);
	return temperatura;
}

void controlDosPuntos()
{
	if (temperatura < TEMP_BAJA)
	{
		calefaccionActiva = true;
		digitalWrite(MI_LED, HIGH);
	}
	else if (temperatura > TEMP_ALTA)
	{
		calefaccionActiva = false;
		digitalWrite(MI_LED, LOW);
	}
}

void actualizarDisplay()
{
	Heltec.display->clear();
	Heltec.display->setFont(ArialMT_Plain_24);
	Heltec.display->drawString(0, 12, String(temperatura, 1) + " °C");
	Heltec.display->setFont(ArialMT_Plain_10);

	if (!termostatoEncendido)
	{
		Heltec.display->drawString(0, 40, "Estado: APAGADO");
	}
	else
	{
		Heltec.display->drawString(0, 40, "Estado: ENCENDIDO");
		Heltec.display->drawString(0, 52, calefaccionActiva ? "Calefaccion: ON" : "Calefaccion: OFF");
	}

	Heltec.display->drawString(80, 52, "T:" + String(TEMP_CONSIGNA, 0) + "C");
	Heltec.display->display();

	// Reinicializar GPIO0 despues de escribir al display
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

	Serial.println("Ejercicio 5: Control por dos puntos");
	Serial.print("Consigna: ");
	Serial.print(TEMP_CONSIGNA, 1);
	Serial.print(" C | Histeresis: +/- ");
	Serial.println(HISTERESIS, 1);

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
		termostatoEncendido = !termostatoEncendido;

		if (!termostatoEncendido)
		{
			calefaccionActiva = false;
			digitalWrite(MI_LED, LOW);
		}

		Serial.print("Termostato: ");
		Serial.println(termostatoEncendido ? "ENCENDIDO" : "APAGADO");
		actualizarDisplay();
		attachInterrupt(digitalPinToInterrupt(BOTON), isrBoton, FALLING);
	}

	if (millis() - ultimaLectura >= INTERVALO_TEMP)
	{
		ultimaLectura = millis();
		temperatura = leerTemperatura();

		Serial.print("Temp: ");
		Serial.print(temperatura, 1);
		Serial.print(" C");

		if (termostatoEncendido)
		{
			controlDosPuntos();
			Serial.print(" | Calefaccion: ");
			Serial.print(calefaccionActiva ? "ON" : "OFF");
		}
		Serial.println();
		actualizarDisplay();
		attachInterrupt(digitalPinToInterrupt(BOTON), isrBoton, FALLING);
	}
}
