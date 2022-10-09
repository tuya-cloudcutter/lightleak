/* Copyright (c) Kuba Szczodrzyński 2022-09-25. */

#include "dump.h"

uint8_t *find_data(uint8_t *start, uint8_t *end, uint8_t *data, uint8_t len) {
	uint8_t pos = 0;
	for (uint8_t *addr = start; addr < end; addr++) {
		if (*addr == data[pos]) {
			pos++;
			if (pos == len)
				return addr - len + 1;
		} else {
			pos = 0;
		}
	}
	return NULL;
}

uint8_t *find_word(uint8_t *start, uint8_t *end, uint32_t number) {
	for (uint8_t *addr = start; addr < end; addr += 4) {
		if (*((uint32_t *)addr) == number) {
			return addr;
		}
	}
	return NULL;
}

uint8_t *find_short(uint8_t *start, uint8_t *end, uint16_t number) {
	for (uint8_t *addr = start; addr < end; addr += 2) {
		uint16_t data = addr[0] | addr[1] << 8;
		if (data == number) {
			return addr;
		}
	}
	return NULL;
}

uint8_t *find_short_rev(uint8_t *start, uint8_t *end, uint16_t number) {
	for (uint8_t *addr = end; addr > start; addr -= 2) {
		uint16_t data = addr[0] | addr[1] << 8;
		if (data == number) {
			return addr;
		}
	}
	return NULL;
}

uint8_t *parse_branch(uint16_t *data) {
	uint32_t address = (uint32_t)data;

	int16_t poff = data[0];
	if (((poff >> 11) & 0b11111) != 0b11110)
		return NULL;
	uint16_t offs = data[1];
	if (((offs >> 11) & 0b11111) != 0b11111)
		return NULL;

	poff <<= 5;
	poff >>= 5;
	offs <<= 5;
	offs >>= 5;
	return (uint8_t *)(address + 4 + (poff << 12) + offs * 2);
}

uint8_t *find_function(FW_INTERFACE *intf, uint8_t *start, uint8_t *end, char *string, uint16_t push_opcode) {
	uint8_t len = strlen(string) + 1;

	intf->printf("Search %s\n", string);

	uint8_t *str_addr = find_data(start, end, (uint8_t *)string, len);
	if (str_addr == NULL) {
		intf->printf("Not found\n");
		return NULL;
	}
	intf->printf("Found %s %01x: %06x\n", string, 1, str_addr);

	uint8_t *str_offset_addr = find_word(start, end, (uint32_t)str_addr);
	if (str_offset_addr == NULL) {
		intf->printf("Not found\n");
		return NULL;
	}
	intf->printf("Found %s %01x: %06x\n", string, 2, str_offset_addr);

	uint8_t *func_addr = find_short_rev(start, str_offset_addr, push_opcode);
	if (func_addr == NULL) {
		intf->printf("Not found\n");
		return NULL;
	}
	intf->printf("Found %s %01x: %06x\n", string, 3, func_addr);
	return func_addr + 1;
}

void find_app_intf(FW_INTERFACE *intf, uint8_t *start, uint8_t *end) {
	uint8_t **store = NULL;

	for (uint16_t *data = (uint16_t *)start; data < (uint16_t *)end; data++) {
		if (data[0] == 0x2001) {
			// movs r0, #1
			intf->printf("Found create\n");
			store = (uint8_t **)&intf->socket;
			data++;
		}
		if (data[0] == 0x425B) {
			// neg r3, r3
			intf->printf("Found sendto\n");
			store = (uint8_t **)&intf->sendto;
			data++;
		}
		if (data[2] == 0x23A4) {
			// movs r3, #0xa4
			intf->printf("Found close\n");
			store = (uint8_t **)&intf->close;
		}

		if (store) {
			uint8_t *target_address = parse_branch(data);
			if (target_address)
				*store = target_address + 1;
		}

		store = NULL;
	}

	intf->printf("Found: create=%x, sendto=%x, close=%x\n", intf->socket, intf->sendto, intf->close);
	intf->search_performed = intf->socket && intf->sendto && intf->close;
}
