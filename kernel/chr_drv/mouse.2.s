# 1 "mouse.S"
# 1 "<built-in>"
# 1 "<command-line>"
# 1 "mouse.S"

# 1 "../../include/linux/config.h" 1





















# 36 "../../include/linux/config.h"

# 47 "../../include/linux/config.h"

# 2 "mouse.S" 2

.globl mouse_interrupt
mouse_interrupt:
	
	pushl %eax
	pushl %ebx
	pushl %ecx
	pushl %edx
	push %ds
	push %es
	//需要压栈%es
	movl $0x10,%eax
	mov %ax,%ds 
	mov %ax,%es
	
	xor %eax,%eax
	inb $0x60,%al	
	pushl %eax
	call readmouse
	addl $4,%esp 

	movb $0x20,%al 
	outb %al,$0xA0 
	outb %al,$0x20 
//8259A 操作命令字 EOI
//向 8259A 从片发送 EOI
//向 8259A 主片发送 EOI
	pop %es
	pop %ds
	popl %edx
	popl %ecx
	popl %ebx
	popl %eax
	iret

