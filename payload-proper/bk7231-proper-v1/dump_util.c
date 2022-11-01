/* Copyright (c) Kuba Szczodrzyński 2022-09-25. */

#include "dump.h"

uint8_t strlen(char *str) {
	uint8_t len = 0;
	while (*(str++)) {
		len++;
	}
	return len;
}
