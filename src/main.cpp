
#include "Arduino.h"
#include "heltec.h"

#define BOTON 0 // Boton PRG de la placa (GPIO0)

volatile bool botonPulsado = false;
unsigned int contador = 0;

void IRAM_ATTR isrBoton()
{
	botonPulsado = true;
}

void setup()
{
	Heltec.begin(false, false, true, false);

	pinMode(BOTON, INPUT_PULLUP);
	attachInterrupt(digitalPinToInterrupt(BOTON), isrBoton, FALLING);
}

void loop()
{
	if (botonPulsado)
	{
		delay(50);
		botonPulsado = false;
		contador++;
		Serial.print("Boton pulsado! Pulsacion #");
		Serial.println(contador);
	}
}
