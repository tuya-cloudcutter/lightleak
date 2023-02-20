#include "customap.h"

WiFiServer server(8080);
ClientData data;
unsigned long long stopAt = 0;

const char packetMagic[] = CLIENT_PACKET_MAGIC;

void setup() {
	Serial.begin(115200);

	WiFi.softAPConfig(IPAddress(10, 0, 0, 1), IPAddress(10, 0, 0, 1), IPAddress(255, 255, 255, 0));

#if defined(ESP8266)
	WiFi.onEvent(onStationConnected, WIFI_EVENT_SOFTAPMODE_STACONNECTED);
#else
	WiFi.onEvent(onStationConnected, ARDUINO_EVENT_WIFI_AP_STACONNECTED);
#endif

	startDefaultAP();
	server.begin();
	data.state = STATE_MAGIC0;
}

void loop() {
	WiFiClient client = server.available();

	if (client) {
		Serial.println("Client connected");
		uint32_t timeout = millis() + 5000;
		while (client.connected()) {
			readClientPacket(&client);
			if (millis() > timeout) {
				Serial.println("Read timeout!");
				break;
			}
		}
		client.stop();
		Serial.println("Client disconnected");
	}

	if (stopAt > 1 && millis() > stopAt) {
		Serial.println("Timeout!");
		startDefaultAP();
	}
}

void startDefaultAP() {
	Serial.print("Starting default AP - SSID: ");
	Serial.print(AP_DEFAULT_SSID);
	Serial.print(" / Password: ");
	Serial.println(AP_DEFAULT_PASS);
	stopAt = 0;
	WiFi.softAPdisconnect(false);
	WiFi.softAP(AP_DEFAULT_SSID, AP_DEFAULT_PASS);
}

void startCustomAP() {
	Serial.print("Starting custom AP - SSID: ");
	Serial.print(data.packet.ssid);
	Serial.print(" / Password: ");
	Serial.println(data.packet.pass);
	stopAt = 1;
	WiFi.softAPdisconnect(false);
	WiFi.softAP(data.packet.ssid, data.packet.pass);
}

void onStationConnected(WiFiEvent_t event) {
	Serial.print("Station connected! ");

	if (stopAt == 0) {
		Serial.println("Default AP mode, timeout disabled");
		return;
	}

	if (data.packet.stopTimeout) {
		Serial.print("Waiting ");
		Serial.print(data.packet.stopTimeout);
		Serial.println("ms to stop custom AP");
		stopAt = millis() + data.packet.stopTimeout;
	} else {
		Serial.println("Stop timeout disabled");
		stopAt = 1;
	}
}

void readClientPacket(WiFiClient *client) {
	if (!client->available())
		return;
	uint8_t c = client->read();

	switch (data.state) {
		case STATE_MAGIC0:
		case STATE_MAGIC1:
		case STATE_MAGIC2:
		case STATE_MAGIC3:
			if (c != packetMagic[data.state])
				data.state = STATE_MAGIC0;
			else
				data.state = (ClientState)(data.state + 1);
			return;

		case STATE_LENGTH:
			Serial.println("Received packet magic");
			if (c != sizeof(ClientPacket)) {
				data.state = STATE_MAGIC0;
			} else {
				data.state	= STATE_DATA;
				data.length = c;
			}
			return;

		case STATE_DATA:
			if (data.length == 0) {
				data.state = STATE_MAGIC0;
				return;
			}
			break;

		default:
			data.state = STATE_MAGIC0;
			return;
	}

	uint8_t *buf = (uint8_t *)&data.packet;
	uint8_t pos	 = sizeof(ClientPacket) - (data.length--);
	buf[pos]	 = c;

	if (data.length == 0) {
		// complete packet received
		Serial.println("Received client packet");
		data.state = STATE_MAGIC0;
		parseClientPacket(client);
	}
}

void parseClientPacket(WiFiClient *client) {
	uint8_t *buf = (uint8_t *)&data.packet;
	uint8_t len	 = sizeof(ClientPacket) - 4;
	uint32_t crc = CRC32::calculate(buf, len);

	if (!client->connected())
		return;

	if (data.packet.crc != crc) {
		Serial.println("CRC error");
		client->write(CLIENT_RESPONSE_ERROR);
		client->flush();
		client->stop();
		return;
	}

	Serial.println("CRC OK");
	client->write(CLIENT_RESPONSE_OK);
	client->flush();
	client->stop();

	data.packet.ssid[32] = '\0';
	data.packet.pass[62] = '\0';

	startCustomAP();
}
