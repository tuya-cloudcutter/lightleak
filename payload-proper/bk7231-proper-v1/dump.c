/* Copyright (c) Kuba Szczodrzyński 2022-09-25. */

#include "dump.h"

int main(uint8_t *data, FW_INTERFACE *intf, uint32_t command) {
	// data = lan->buf+0x54 = lan+0x58
	LAN_AP_NW_CFG_S *lan = (LAN_AP_NW_CFG_S *)(data - 0x58);

	uint8_t *start = (uint8_t *)(0x10000);
	uint8_t *end   = (uint8_t *)(start + 0x107800);

	if (intf->search_performed != 0xDEADBEEF) {
		uint8_t *func_start, *func_end;
		LOG("Search\n");

		// push {r4-r7, lr}
		func_start = find_function(intf, start, end, "ap_cfg_send_err_code", 0xB5F0);
		// pop {r4-r7, pc}
		func_end = find_short(func_start, end, 0xBDF0) + 2;
		// store function bounds
		intf->ap_cfg_send_err_code	   = (ap_cfg_send_err_code_t)func_start;
		intf->ap_cfg_send_err_code_end = func_end;

		// push {r4, r5, lr}
		func_start = find_function(intf, start, end, "sys_stop_timer", 0xB530);
		// pop {r4, r5, pc}
		func_end = find_short(func_start, end, 0xBD30) + 2;
		// store function bounds
		intf->sys_stop_timer	 = (sys_stop_timer_t)func_start;
		intf->sys_stop_timer_end = func_end;

		find_app_intf(intf);

		if (intf->search_performed != 0xDEADBEEF)
			return 1;
	}

	uint32_t *data32	= (uint32_t *)data;
	uint32_t request_id = data32[0];
	uint32_t address	= data32[1];

	LOG("cmd=%02x\n", command);

	switch (command) {
		case 0x01:
			cmd_flash_read(intf, request_id, address, data32);
			break;

		case 0x0E:
			cmd_stop_timer(intf, request_id, address, data32);
			break;

		case 0x10:
			cmd_fill_intf(intf, request_id, address, data32);
			break;
	}

	return 0;
}
