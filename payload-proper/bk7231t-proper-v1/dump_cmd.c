/* Copyright (c) Kuba Szczodrzyński 2022-09-26. */

#include "dump.h"

void cmd_send_response(FW_INTERFACE *intf, uint32_t request_id, uint32_t address, uint8_t *data, uint16_t len) {
	int fd = intf->socket(1);
	if (fd < 0)
		return;
	LOG("Got fd %01x, send to %08x\n", fd, address);

	uint32_t crc = intf->crc32(0, data, len);
	data -= 8;
	len += 8;
	uint32_t *data32 = (uint32_t *)data;
	data32[0]		 = request_id;
	data32[1]		 = crc;

	int ret = intf->sendto(fd, data, len, address, 6667);
	LOG("ret=%01x, len=%01x\n", ret, len);

	intf->close(fd);
}

void cmd_flash_read(FW_INTERFACE *intf, uint32_t request_id, uint32_t address, uint32_t *data32) {
	// data32[0] - request ID
	// data32[1] - return IP address
	// data32[2] - buffer address
	// data32[3] - flash offset
	// data32[4] - read length
	// data32[5] - max packet length

	uint8_t *buf	= (uint8_t *)data32[2];
	uint32_t *buf32 = (uint32_t *)buf;

	uint32_t offset = data32[3];
	uint32_t length = data32[4];
	uint32_t maxlen = data32[5];

	LOG("Read %x from %06x\n", length, offset);

	while (length && maxlen) {
		uint32_t to_read = MIN(length, maxlen);

		buf32[0] = offset;
		buf32[1] = to_read;

		intf->flash_read(buf + 8, offset, to_read);
		cmd_send_response(intf, request_id, address, buf, to_read + 8);

		offset += to_read;
		length -= to_read;
	}
}

void cmd_stop_timer(FW_INTERFACE *intf, uint32_t request_id, uint32_t address, uint32_t *data32) {
	// data32[0] - request ID
	// data32[1] - return IP address
	// data32[2] - buffer address
	// data32[3] - timer count + mode
	// data32[*] - timer ID/period

	uint32_t *buf32 = (uint32_t *)data32[2];

	uint32_t timer_id;
	bool period_mode	= data32[3] >> 8;
	uint8_t timer_count = data32[3] & 0xFF;

	uint16_t loops = 0;

	for (uint8_t i = 0; i < timer_count; i++) {
		uint32_t timer_id = data32[4 + i];
		uint32_t period	  = timer_id;

		if (period_mode) {
			uint32_t *handle = intf->sys_timer_handle;
			while (handle[4] != period) {
				DEBUG("i=%01x, hndl=%x, intf hndl=%x\n", i, handle, intf->sys_timer_handle);
				handle = (uint32_t *)(handle[0]);
				if (handle == intf->sys_timer_handle) {
					DEBUG("END\n");
					// break after checking all timers
					break;
				}
				if (++loops > 100)
					return;
			}
			if (handle[4] != period) {
				buf32[i] = -1;
				continue;
			}
			DEBUG("FOUND period=%x\n", period);
			timer_id = handle[7] & 0xFFFF;
		}

		buf32[i] = intf->sys_stop_timer(timer_id);
		LOG("Stop tmr %02x ret=%x\n", timer_id, buf32[i]);
	}

	cmd_send_response(intf, request_id, address, (uint8_t *)buf32, timer_count * 4);
}
