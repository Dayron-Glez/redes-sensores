#include <Arduino.h>
#include <heltec.h>

#define BAND (868E6)
#define ORG (2)
#define DESTINO_DEFECTO (5)
#define LED (2)
#define BOTON (0)
#define TREBOTE (20)
#define TDELAY (100)
#define TIME_TEMP (10000)

// Variables
static bool teclaPulsada;
static String rcvMsgRadio;
static String rcvPayloadRadio;
static unsigned long nextEventTemp;
static bool termostatoEncendido = false;
static bool calefaccionActiva = false;
static float temperatura = 20.0;
static int lastRadioOrigin = -1;
static int lastRadioDestination = -1;

#define TEMP_CONSIGNA 22.0
#define HISTERESIS 1.0
#define TEMP_BAJA (TEMP_CONSIGNA - HISTERESIS)
#define TEMP_ALTA (TEMP_CONSIGNA + HISTERESIS)

// --- Funciones auxiliares (del loopback) ---

void actualizarDisplay();

void printScreen(const String &msg, const int16_t &pos = 0)
{
	Heltec.display->clear();
	Heltec.display->drawString(pos, 10, msg);
	Heltec.display->display();
	pinMode(BOTON, INPUT_PULLUP); // Fix GPIO0/OLED
}

float getTemp(void)
{
	float temperature = 0.0;
	int counter = (millis() / 1000) % 20;
	if (counter < 10)
	{
		temperature = 15.0 + counter;
	}
	else
	{
		temperature = 25.0 - (counter % 10);
	}
	return temperature;
}

void sendRadio(const String &message, const int destino = DESTINO_DEFECTO)
{
	String packet = String(destino) + String(ORG) + message;
	Serial.println("Radio TX: " + packet);
	LoRa.beginPacket();
	LoRa.setTxPower(14, RF_PACONFIG_PASELECT_PABOOST);
	LoRa.print(packet);
	LoRa.endPacket();
	LoRa.receive();
}

void replyRadio(const String &message)
{
	if (lastRadioOrigin >= 0)
	{
		sendRadio(message, lastRadioOrigin);
	}
}

void aplicarComandoRadio(const String &payload)
{
	bool comandoAplicado = false;

	if (lastRadioOrigin == 2 && payload == "ENCENDER")
	{
		termostatoEncendido = true;
		comandoAplicado = true;
	}
	else if (lastRadioOrigin == 2 && payload == "APAGADO")
	{
		termostatoEncendido = false;
		calefaccionActiva = false;
		digitalWrite(LED, LOW);
		comandoAplicado = true;
	}

	if (comandoAplicado)
	{
		actualizarDisplay();
	}
}

void onRadioMsg(int packetSize)
{
	String readMsg = "";
	for (int i = 0; i < packetSize; i++)
	{
		readMsg += (char)LoRa.read();
	}

	if (readMsg.length() >= 2 && isDigit(readMsg[0]) && isDigit(readMsg[1]))
	{
		lastRadioDestination = readMsg.substring(0, 1).toInt();
		lastRadioOrigin = readMsg.substring(1, 2).toInt();
		rcvPayloadRadio = readMsg.substring(2);

		if (lastRadioDestination == ORG && rcvMsgRadio.isEmpty())
		{
			rcvMsgRadio = readMsg;
		}
	}
	else
	{
		lastRadioDestination = -1;
		lastRadioOrigin = -1;
		rcvPayloadRadio = "";
	}
}

// --- Control del termostato (de la P1) ---

void controlDosPuntos()
{
	if (temperatura < TEMP_BAJA)
	{
		calefaccionActiva = true;
		digitalWrite(LED, HIGH);
	}
	else if (temperatura > TEMP_ALTA)
	{
		calefaccionActiva = false;
		digitalWrite(LED, LOW);
	}
}

void actualizarDisplay()
{
	Heltec.display->clear();
	Heltec.display->setFont(ArialMT_Plain_24);
	Heltec.display->drawString(0, 0, String(temperatura, 1) + " C");
	Heltec.display->setFont(ArialMT_Plain_10);

	if (!termostatoEncendido)
	{
		Heltec.display->drawString(0, 40, "Estado: APAGADO");
	}
	else
	{
		Heltec.display->drawString(0, 40, "Estado: ENCENDIDO");
		Heltec.display->drawString(0, 52, calefaccionActiva ? "Calef: ON" : "Calef: OFF");
	}
	Heltec.display->display();
	pinMode(BOTON, INPUT_PULLUP); // Fix GPIO0/OLED
}

// --- Setup y Loop ---

void setup()
{
	Heltec.begin(true, true, true, true, BAND); // Display, LoRa, Serial, PABOOST, frecuencia

	pinMode(BOTON, INPUT_PULLUP);
	pinMode(LED, OUTPUT);
	digitalWrite(LED, LOW);

	Serial.begin(115200);
	Serial.println("P2 Ej1: Temperatura por radio");

	Heltec.display->init();
	Heltec.display->setFont(ArialMT_Plain_10);

	LoRa.onReceive(onRadioMsg);
	LoRa.receive();
	rcvMsgRadio = "";
	rcvPayloadRadio = "";

	nextEventTemp = millis();
	teclaPulsada = false;
	temperatura = getTemp();
	actualizarDisplay();
}

void loop()
{
	// Evento del boton (polling, como en el loopback)
	if (!digitalRead(BOTON) && !teclaPulsada)
	{
		sendRadio("ENCENDER");
		Serial.print("Orden enviada a ");
		Serial.print(DESTINO_DEFECTO);
		Serial.print(": ");
		Serial.println("ENCENDER");
		teclaPulsada = true;
		delay(TREBOTE);
	}
	else if (digitalRead(BOTON) && teclaPulsada)
	{
		teclaPulsada = false;
		delay(TREBOTE);
	}

	// Evento de temperatura cada 10 segundos
	if (millis() >= nextEventTemp)
	{
		temperatura = getTemp();

		// Mostrar por serial
		Serial.print("Temp: ");
		Serial.print(temperatura, 1);
		Serial.print(" C");

		if (termostatoEncendido)
		{
			controlDosPuntos();
			Serial.print(" | Calef: ");
			Serial.print(calefaccionActiva ? "ON" : "OFF");
		}
		Serial.println();

		// Enviar por radio
		sendRadio("TEMP" + String(temperatura, 1));

		// Actualizar display
		actualizarDisplay();

		nextEventTemp += TIME_TEMP;
	}

	if (!rcvMsgRadio.isEmpty())
	{
		Serial.println("Radio RX: " + rcvMsgRadio);

		if (lastRadioOrigin >= 0)
		{
			Serial.println("Payload RX: " + rcvPayloadRadio);
			aplicarComandoRadio(rcvPayloadRadio);
			if (!rcvPayloadRadio.startsWith("RECIBIDO"))
			{
				replyRadio("RECIBIDO" + rcvPayloadRadio);
			}
		}

		rcvMsgRadio = "";
		rcvPayloadRadio = "";
	}

	delay(TDELAY);
}
