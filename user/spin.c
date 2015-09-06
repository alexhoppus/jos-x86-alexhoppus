// Test preemption by forking off a child process that just spins forever.
// Let it run for a couple time slices, then kill it.

#include <inc/lib.h>

void
umain(int argc, char **argv)
{
	envid_t env;

	cprintf("I am the parent.  Forking the child...\n");
	if ((env = fork()) == 0) {
		cprintf("I am the child.  Spinning...\n");
		while (1)
			/* do nothing */;
	}

	cprintf("I am the parent.  Running the child...\n");
	cprintf("Yield1\n");
	sys_yield();
	cprintf("Yield2\n");
	sys_yield();
	cprintf("Yield3\n");
	sys_yield();
	cprintf("Yield4\n");
	sys_yield();
	cprintf("Yield5\n");
	sys_yield();
	cprintf("Yield6\n");
	sys_yield();
	cprintf("Yield7\n");
	sys_yield();
	cprintf("Yield8\n");
	sys_yield();

	cprintf("I am the parent.  Killing the child...\n");
	sys_env_destroy(env);
}

