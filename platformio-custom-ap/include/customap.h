/* Copyright (c) Kuba Szczodrzyński 2022-10-01. */

#pragma once

#include <Arduino.h>
#include <CRC32.h>

#ifdef ESP8266
#include <ESP8266WiFi.h>
#else
#include <WiFi.h>
#endif

#define AP_DEFAULT_SSID "LightleakIdle"
#define AP_DEFAULT_PASS "cl0udcutt3r!@#"

#define CLIENT_PACKET_MAGIC	  "cctr"
#define CLIENT_RESPONSE_OK	  0xDE
#define CLIENT_RESPONSE_ERROR 0xAD

typedef enum {
	STATE_MAGIC0 = 0,
	STATE_MAGIC1 = 1,
	STATE_MAGIC2 = 2,
	STATE_MAGIC3 = 3,
	STATE_LENGTH = 4,
	STATE_DATA	 = 5,
} ClientState;

typedef struct {
	char ssid[33];
	char pass[63]; // we only support max. 62 char passwords
	uint32_t stopTimeout;
	uint32_t crc;
} ClientPacket;

typedef struct {
	ClientState state;
	uint32_t length;
	ClientPacket packet;
} ClientData;

void startDefaultAP();
void startCustomAP();
void onStationConnected(WiFiEvent_t event);
void readClientPacket(WiFiClient *client);
void parseClientPacket(WiFiClient *client);
