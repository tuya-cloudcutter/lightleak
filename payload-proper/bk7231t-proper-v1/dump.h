/* Copyright (c) Kuba Szczodrzyński 2022-09-25. */

#pragma once

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#define MIN(x, y) (((x) < (y)) ? (x) : (y))

typedef int (*flash_read_t)(uint8_t *buf, uint32_t offset, uint32_t size);
typedef int (*flash_write_t)(uint8_t *buf, uint32_t offset, uint32_t size);
typedef int (*flash_erase_t)(uint32_t offset);
typedef int (*printf_t)(const char *fmt, ...);
typedef int (*crc32_t)(uint32_t init, uint8_t *buf, uint32_t size);

typedef int (*ap_cfg_send_err_code_t)();
typedef int (*tuya_hal_net_socket_create_t)(uint8_t type);
typedef int (*tuya_hal_net_set_reuse_t)(int fd);
typedef int (*tuya_hal_net_set_boardcast_t)(int fd);
typedef int (*tuya_hal_net_send_to_t)(int fd, void *buf, uint32_t nbytes, uint32_t addr, uint16_t port);
typedef int (*tuya_hal_net_close_t)(int fd);
typedef int (*sys_stop_timer_t)(int timer_id);

typedef struct {
	uint32_t search_performed;
	// bootloader
	flash_read_t flash_read;
	flash_write_t flash_write;
	flash_erase_t flash_erase;
	printf_t printf;
	crc32_t crc32;
	void *reserved[16];
	// app
	ap_cfg_send_err_code_t ap_cfg_send_err_code;
	tuya_hal_net_socket_create_t socket;
	tuya_hal_net_set_reuse_t reuse;
	tuya_hal_net_set_boardcast_t broadcast;
	tuya_hal_net_send_to_t sendto;
	tuya_hal_net_close_t close;
	sys_stop_timer_t sys_stop_timer;
} FW_INTERFACE;

typedef struct {
	uint8_t ssid[33 - 8];
	uint8_t s_len;
	uint8_t passwd[65];
	uint8_t p_len;
	uint8_t token[17];
	uint8_t t_len;
} SSID_PASSWORD_TOKEN;

typedef struct {
	uint32_t thread;
	uint8_t recv_buf[256];
	uint8_t ap_cfg_token[64];
	uint32_t fd;
	uint16_t log_ack_timer;
	uint16_t send_log_mid;
	void *finish_cb;
	uint32_t log_len1;
	uint8_t *log_buf1;
	SSID_PASSWORD_TOKEN spt;
	uint8_t dummy1;
	uint8_t dummy2;
	uint32_t log_len2;
	uint8_t *log_buf2;
} LAN_AP_NW_CFG_S;

// dump_search.c
uint8_t *find_data(uint8_t *start, uint8_t *end, uint8_t *data, uint8_t len);
uint8_t *find_word(uint8_t *start, uint8_t *end, uint32_t number);
uint8_t *find_short(uint8_t *start, uint8_t *end, uint16_t number);
uint8_t *find_short_rev(uint8_t *start, uint8_t *end, uint16_t number);
uint8_t *parse_branch(uint16_t *data);
uint8_t *find_function(FW_INTERFACE *intf, uint8_t *start, uint8_t *end, char *string, uint16_t push_opcode);
void find_app_intf(FW_INTERFACE *intf, uint8_t *start, uint8_t *end);
// dump_util.c
uint8_t strlen(char *str);
// dump_cmd.c
void cmd_send_response(FW_INTERFACE *intf, uint32_t request_id, uint32_t address, uint8_t *data, uint16_t len);
void cmd_flash_read(FW_INTERFACE *intf, uint32_t request_id, uint32_t address, uint32_t *data32);
void cmd_stop_timer(FW_INTERFACE *intf, uint32_t request_id, uint32_t address, uint32_t *data32);
