/* Copyright (c) Kuba Szczodrzyński 2022-09-25. */

#include "dump.h"

int main(uint8_t *data, FW_INTERFACE *intf, uint32_t command) {
	// data = lan->buf+0x54 = lan+0x58
	LAN_AP_NW_CFG_S *lan = (LAN_AP_NW_CFG_S *)(data - 0x58);

	uint8_t *start = (uint8_t *)(0x10000);
	uint8_t *end   = (uint8_t *)(start + 0x107800);

	if (intf->search_performed == false) {
		// push {r4-r7, lr}
		uint8_t *func_start = find_function(intf, start, end, "ap_cfg_send_err_code", 0xB5F0);
		// pop {r4-r7, pc}
		uint8_t *func_end = find_short(func_start - 1, end, 0xBDF0) + 2;

		intf->ap_cfg_send_err_code	   = (ap_cfg_send_err_code_t)func_start;
		intf->ap_cfg_send_err_code_end = (void *)func_end;

		find_app_intf(intf, func_start - 1, func_end);

		if (!intf->search_performed)
			return 1;
	}

	switch (command) {
		case 0x01:
			cmd_flash_read(intf, data);
			break;
	}

	return 0;
}
