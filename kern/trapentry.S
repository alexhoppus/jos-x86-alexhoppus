/* See COPYRIGHT for copyright information. */

#include <inc/mmu.h>
#include <inc/memlayout.h>
#include <inc/trap.h>



###################################################################
# exceptions/interrupts
###################################################################

/* TRAPHANDLER defines a globally-visible function for handling a trap.
 * It pushes a trap number onto the stack, then jumps to _alltraps.
 * Use TRAPHANDLER for traps where the CPU automatically pushes an error code.
 *
 * You shouldn't call a TRAPHANDLER function from C, but you may
 * need to _declare_ one in C (for instance, to get a function pointer
 * during IDT setup).  You can declare the function with
 *   void NAME();
 * where NAME is the argument passed to TRAPHANDLER.
 */
#define TRAPHANDLER(name, num)						\
	.globl name;		/* define global symbol for 'name' */	\
	.type name, @function;	/* symbol type is function */		\
	.align 2;		/* align function definition */		\
	name:			/* function starts here */		\
	pushl $(num);							\
	jmp _alltraps

/* Use TRAPHANDLER_NOEC for traps where the CPU doesn't push an error code.
 * It pushes a 0 in place of the error code, so the trap frame has the same
 * format in either case.
 */
#define TRAPHANDLER_NOEC(name, num)					\
	.globl name;							\
	.type name, @function;						\
	.align 2;							\
	name:								\
	pushl $0;							\
	pushl $(num);							\
	jmp _alltraps

.text

/*
 * Lab 3: Your code here for generating entry points for the different traps.
 */

TRAPHANDLER_NOEC(HANDLER_SYSCALL, T_SYSCALL)
TRAPHANDLER_NOEC(HANDLER_DIVIDE, T_DIVIDE)
TRAPHANDLER_NOEC(HANDLER_SEGNP, T_SEGNP)
TRAPHANDLER(HANDLER_GPFLT, T_GPFLT)
TRAPHANDLER(HANDLER_PGFLT, T_PGFLT)
TRAPHANDLER_NOEC(HANDLER_BRKPT, T_BRKPT)





/*
 * Lab 3: Your code here for _alltraps
 */
        .globl _alltraps;
        .type _alltraps, @function;
	_alltraps:
	pushl %ds
	pushl %es
	pushal
	movl $0x10, %eax
	movw %ax, %es
	movw %ax, %ds
	pushl %esp
	call trap	
