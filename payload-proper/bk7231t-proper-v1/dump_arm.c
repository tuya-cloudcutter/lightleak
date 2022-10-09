/* Copyright (c) Kuba Szczodrzyński 2022-10-09. */

#include "dump.h"

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

uint32_t *parse_ldr_pc(uint16_t *data, uint8_t *reg) {
	uint32_t address = (uint32_t)data;

	uint8_t opc = *data >> 8;
	if (((opc >> 3) & 0b01001) != 0b01001)
		return NULL;
	if (reg)
		*reg = opc & 0b111;

	uint8_t immed = *data & 0xFF;

	uint32_t target = address + 2 + (immed << 2);
	return (uint32_t *)((((target - 1) / 4) + 1) * 4);
}
