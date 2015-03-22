
#include <kernel.h>

#include "disptable.c"


/* Always points to the process (i.e., PCB slot) that CPU is currently executing */
PROCESS active_proc;


/*
 * Ready queues for all eight priorities.
 * 8 element-sized array ordered by process priority
 * 0 - lowest priority
 * 7 - highest priority
 */
PCB *ready_queue [MAX_READY_QUEUES];

/*
 * The bits in ready_procs tell which ready queue is empty.
 * The MSB (most significant bit) of ready_procs corresponds
 * to ready_queue[7].
 */
unsigned ready_procs;


/*

typedef struct _PCB {
	unsigned       magic;
	unsigned	   used;
	unsigned short priority;
	unsigned short state;
	MEM_ADDR	   esp;
	PROCESS        param_proc;
	void*		   param_data;
	PORT           first_port;
	PROCESS        next_blocked;
	PROCESS        next;
	PROCESS		   prev;
	char*          name;
} PCB;

typedef PCB* PROCESS;

 */

/*
 * add_ready_queue
 *----------------------------------------------------------------------------
 * The process pointed to by p is put the ready queue.
 * The appropriate ready queue is determined by p->priority.
 */

void add_ready_queue (PROCESS proc)
{
	int process_priority;
	volatile int flag;

	DISABLE_INTR(flag);

	assert(proc->magic == MAGIC_PCB);
	process_priority = proc->priority;
	if(ready_queue[process_priority] == NULL){
		/* The only process on this priority level */
		ready_queue[process_priority] = proc;
		proc->next = proc;
		proc->prev = proc;
		ready_procs |= 1 << process_priority;
		// 00000001 << process_priority;
		// 00000001 << 0  same as 1 * 2^0
		// 00000001 << 1  same as 1 * 2^1 = 00000010
		// 00000001 << 2  same as 1 * 2^2 = 00000100
		// 00000001 << 3  same as 1 * 2^3 = 00001000
		// ...
		// 00000001 << 7  same as 1 * 2^7 = 10000000
	}
	else {
		/* Other processes are present on this priority level*/
		/* p1=>p2=>p3=> P =>p4=>p1 */
		proc->next = ready_queue[process_priority];
		proc->prev = ready_queue[process_priority]->prev;
		ready_queue[process_priority]->prev->next = proc;
		ready_queue[process_priority]->prev       = proc;
	}

	proc->state = STATE_READY;

	ENABLE_INTR(flag);
}



/*
 * remove_ready_queue
 *----------------------------------------------------------------------------
 * The process pointed to by p is dequeued from the ready
 * queue.
 */

void remove_ready_queue (PROCESS proc)
{
	int process_priority;
	volatile int flag;

	DISABLE_INTR(flag);

	assert(proc->magic == MAGIC_PCB);
	process_priority = proc->priority;
	if(proc->next == proc) {
		/* This is the last process on this priority level */
		ready_queue[process_priority] = NULL;
		ready_procs &= ~(1 << process_priority);
	}
	else {
		/* p1=>p2=>p3=> P =>p4=>p1 */
		ready_queue[process_priority] = proc->next;
		proc->next->prev              = proc->prev;
		proc->prev->next              = proc->next;
	}

	ENABLE_INTR(flag);
}



/*
 * dispatcher
 *----------------------------------------------------------------------------
 * Determines a new process to be dispatched. The process
 * with the highest priority is taken. Within one priority
 * level round robin is used.
 */

PROCESS dispatcher()
{
	PROCESS  new_process;
	unsigned i;
	volatile int flag;

	DISABLE_INTR(flag);

	/* Find queue with highest priority that is not empty */
	i = table[ready_procs];
	assert(i != -1);
	if(i == active_proc->priority){
		/* Round robin within the same priority level */
		new_process = active_proc->next;
	}
	else {
		/* Dispatch a process at a different priority level */
		new_process = ready_queue[i];
	}

	ENABLE_INTR(flag);
	return new_process;
}



/*
 * resign
 *----------------------------------------------------------------------------
 * The current process gives up the CPU voluntarily. The
 * next running process is determined via dispatcher().
 * The stack of the calling process is setup such that it
 * looks like an interrupt.
 */
void resign()
{
	/*
	 *  -------
	 *  PUSHFL				 ; Push EFALGS
	 *  CLI					 ; Disable interrupts
	 *  POPL    %EAX         ; EAX = EFLAGS
	 *  XCHGL   (%ESP),%EAX  ; Swap return address with EFLAGS, EAX now contains return address
	 *  PUSH    %CS          ; Push CS
	 *  -------
	 *
	 *  PUSHL	%EAX          ; Saving process context
	 *  PUSHL   %ECX
	 *  PUSHL   %EDX
	 *  PUSHL   %EBX
	 *  PUSHL   %EBP
	 *  PUSHL   %ESI
	 *  PUSHL   %EDI
	 */

	asm("pushfl");
	asm("cli");
	asm("popl %eax");
	asm("xchgl (%esp),%eax");
	asm("push %cs");
	asm("pushl %eax");

	asm("pushl %eax");
	asm("pushl %ecx");
	asm("pushl %edx");
	asm("pushl %ebx");
	asm("pushl %ebp");
	asm("pushl %esi");
	asm("pushl %edi");

	/* Save the context pointer SS:ESP to PCB */
	asm("movl %%esp,%0" : "=r" (active_proc->esp) : );

	/* Dispatch new process */
	active_proc = dispatcher();

	/* Restore context pointer SS:ESP */
	asm("movl %0,%%esp" : : "r" (active_proc->esp));

	/* Restore previously saved context
	 *
	 * POPL %EDI
	 * POPL %ESI
	 * POPL %EBP
	 * POPL %EBX
	 * POPL %EDX
	 * POPL %ECX
	 * POPL %EAX
	 * IRET            <= Return to new process
	 */

	asm("popl %edi");
	asm("popl %esi");
	asm("popl %ebp");
	asm("popl %ebx");
	asm("popl %edx");
	asm("popl %ecx");
	asm("popl %eax");
	asm("iret");
}



/*
 * init_dispatcher
 *----------------------------------------------------------------------------
 * Initializes the necessary data structures.
 * MAX_READY_QUEUES = 8
 */

void init_dispatcher()
{
	int i;

	for(i=0; i < MAX_READY_QUEUES; i++){
		ready_queue[i] = NULL;
	}
	ready_procs = 0;

	/* Setup 1st process */
	add_ready_queue(active_proc);

}


