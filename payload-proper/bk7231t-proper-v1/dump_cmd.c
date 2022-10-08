/* Copyright (c) Kuba Szczodrzyński 2022-09-26. */

#include "dump.h"

void cmd_send_response(FW_INTERFACE *intf, uint32_t request_id, uint32_t address, uint8_t *data, uint16_t len) {
	int fd = intf->socket(1);
	if (fd < 0)
		return;
	intf->printf("Got fd %01x, send to %08x\n", fd, address);

	uint32_t crc = intf->crc32(0, data, len);
	data -= 8;
	len += 8;
	uint32_t *data32 = (uint32_t *)data;
	data32[0]		 = request_id;
	data32[1]		 = crc;

	int ret = intf->sendto(fd, data, len, address, 6667);
	intf->printf("ret=%01x, len=%01x\n", ret, len);

	intf->close(fd);
}

void cmd_flash_read(FW_INTERFACE *intf, uint8_t *data) {
	uint16_t *data16 = (uint16_t *)data;
	uint32_t *data32 = (uint32_t *)data;

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

	while (length && maxlen) {
		uint32_t to_read = MIN(length, maxlen);

		buf32[0] = offset;
		buf32[1] = to_read;

		intf->flash_read(buf + 8, offset, to_read);
		cmd_send_response(intf, data32[0], data32[1], buf, to_read + 8);

		offset += to_read;
		length -= to_read;
	}
}
