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
	start = THUMB_ADDR(start);
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

uint8_t *find_function(FW_INTERFACE *intf, uint8_t *start, uint8_t *end, char *string, uint16_t push_opcode) {
#ifdef BK7231T
	uint8_t len = strlen(string) + 1;
#endif
#ifdef BK7231N
	// don't expect '\0' at the end of string
	// (we're searching parts of strings)
	uint8_t len = strlen(string);
#endif

	LOG("Search %s\n", string);

	uint8_t *str_addr = find_data(start, end, (uint8_t *)string, len);
	if (str_addr == NULL) {
		LOG("Not found\n");
		return NULL;
	}
	LOG("Found %s %01x: %06x\n", string, 1, str_addr);

	uint8_t *str_offset_addr = find_word(start, end, (uint32_t)str_addr);
	if (str_offset_addr == NULL) {
		LOG("Not found\n");
		return NULL;
	}
	LOG("Found %s %01x: %06x\n", string, 2, str_offset_addr);

	uint8_t *func_addr = find_short_rev(start, str_offset_addr, push_opcode);
	if (func_addr == NULL) {
		LOG("Not found\n");
		return NULL;
	}
	LOG("Found %s %01x: %06x\n", string, 3, func_addr);
	return func_addr + 1;
}

void find_app_intf(FW_INTERFACE *intf) {
	uint8_t **store = NULL;
	uint16_t *start, *end;

	// reset intf so that we don't get false success
	intf->socket		   = 0;
	intf->sendto		   = 0;
	intf->close			   = 0;
	intf->sys_timer_handle = 0;

	start = (uint16_t *)THUMB_ADDR(intf->ap_cfg_send_err_code);
	end	  = (uint16_t *)intf->ap_cfg_send_err_code_end;
	for (uint16_t *data = start; data < end; data++) {
		if (data[0] == 0x2001) {
			// movs r0, #1
			LOG("Found create\n");
			store = (uint8_t **)&intf->socket;
			data++;
		}
		if (data[0] == 0x425B) {
			// neg r3, r3
			LOG("Found sendto\n");
			store = (uint8_t **)&intf->sendto;
			data++;
		}
		if (data[2] == 0x23A4) {
			// movs r3, #0xa4
			LOG("Found close\n");
			store = (uint8_t **)&intf->close;
		}

		if (store) {
			uint8_t *target_address = parse_branch(data);
			if (target_address)
				*store = target_address + 1;
		}

		store = NULL;
	}

	LOG("Found: create=%x, sendto=%x, close=%x\n", intf->socket, intf->sendto, intf->close);

	start = (uint16_t *)THUMB_ADDR(intf->sys_stop_timer);
	end	  = (uint16_t *)intf->sys_stop_timer_end;
	for (uint16_t *data = start; data < end; data++) {
		if (data[1] == 0x1C29 && data[2] == 0x6818) {
			// movs r1, r5
			// ldr r0, [r3]
			uint32_t ***addr = (uint32_t ***)parse_ldr_pc(data, NULL);
			LOG("Found %s %01x: %06x\n", "imm", 1, addr);
			if (addr == NULL)
				continue;
			LOG("Found %s %01x: %06x\n", "hnd", 2, *addr);
			intf->sys_timer_handle = **addr;
		}
	}

	LOG("Found: tmr_hndl: %x\n", intf->sys_timer_handle);

	if (intf->socket && intf->sendto && intf->close && intf->sys_timer_handle)
		intf->search_performed = 0xDEADBEEF;
	else
		intf->search_performed = 0;
}
