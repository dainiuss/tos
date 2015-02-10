
#include <kernel.h>

#include "disptable.c"


/* Always points to the process (i.e., PCB slot) that currently owns the CPU. */
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
	assert(proc->magic == MAGIC_PCB);
	process_priority = proc->priority;
	if(ready_queue[process_priority == NULL]){
		/* The only process on this priority level */
		ready_queue[process_priority] = proc;
		proc->next = proc;
		proc->prev = proc;
		ready_procs |= 1 << process_priority;
		// 00000001 << process_priority;
		// 1 << 0  same as 1 * 2^0
		// 1 << 1  same as 1 * 2^1
		// 1 << 2  same as 1 * 2^2
		// 1 << 3  same as 1 * 2^3
		// ...
		// 1 << 7  same as 1 * 2^7 = 128
	}
	else {
		/* Other processes are present on this priority level*/
		proc->next = ready_queue[process_priority];
		proc->prev = ready_queue[process_priority]->prev;
		ready_queue[process_priority]->prev->next = proc;
		ready_queue[process_priority]->prev       = proc;
	}

	proc->state = STATE_READY;
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
	assert(proc->magic == MAGIC_PCB);
	process_priority = proc->priority;
	if(proc->next == proc) {
		/* This is the last process on this priority level */
		ready_queue[process_priority] = NULL;
		ready_procs &= ~(1 << process_priority);
	}
	else {
		ready_queue[process_priority] = proc->next;
		proc->next->prev              = proc->prev;
		proc->prev->next              = proc->next;
	}
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


