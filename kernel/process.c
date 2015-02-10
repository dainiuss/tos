
#include <kernel.h>


PCB pcb[MAX_PROCS];
PCB *next_free_pcb;


PORT create_process (void (*ptr_to_new_proc) (PROCESS, PARAM),
		     int prio,
		     PARAM param,
		     char *name)
{
	MEM_ADDR esp;
	PROCESS new_process;

	new_process = next_free_pcb;
	next_free_pcb = new_process->next;

	new_process->magic      = MAGIC_PCB;
	new_process->used       = TRUE;
	new_process->state      = STATE_READY;
	new_process->priority   = prio;
	new_process->first_port = NULL;
	new_process->name       = name;

	/* Compute linear address of new process' system stack */
	/* Start at 640kB and go down, 30kB for each process stack */
	esp = 640 * 1024 - (new_process - pcb) * 30 * 1024;


#define PUSH(x)   esp -= 4; poke_l(esp, (LONG) x);

	/* Initialize the stack for the new process */
	PUSH(param);           /* First data */
	PUSH(new_process);     /* Self */
	PUSH(0);               /* Dummy return address */
	PUSH(ptr_to_new_proc); /* Entry point of new process */
	PUSH(0);			   /* EAX */
	PUSH(0);			   /* ECX */
	PUSH(0);			   /* EDX */
	PUSH(0);			   /* EBX */
	PUSH(0);			   /* EBP */
	PUSH(0);			   /* ESI */
	PUSH(0);			   /* EDI */

#undef PUSH


	new_process->esp = esp;

	add_ready_queue(new_process);


	return NULL;
}


PROCESS fork()
{
    // Dummy return to make gcc happy
    return (PROCESS) NULL;
}


void print_process_header(WINDOW* wnd)
{
    wprintf(wnd, "State           Active Prio Name\n");
    wprintf(wnd, "------------------------------------------------\n");
}


void print_process_details(WINDOW* wnd, PROCESS p)
{
	static const char *state[] =
	{ "READY          ",
	  "SEND_BLOCKED   ",
	  "REPLY_BLOCKED  ",
	  "RECEIVE_BLOCKED",
	  "MESSAGE_BLOCKED",
	  "INTR_BLCOKED   "
	};

	if(!p->used){
		wprintf(wnd, "PCB slot unused!\n");
		return;
	}

	/* State */
	wprintf(wnd, state[p->state]);

	/* Check for active_proc */
	if(p == active_proc){
		wprintf(wnd, " *       ");
	}
	else{
		wprintf(wnd, "         ");
	}

	/* Priority */
	wprintf(wnd, "  %2d", p->priority);

	/* Name */
	wprintf(wnd, " %s\n", p->name);

}


void print_process(WINDOW* wnd, PROCESS p)
{
	print_process_header(wnd);
	print_process_details(wnd, p);
}

void print_all_processes(WINDOW* wnd)
{
	int i;
	PCB *p = pcb;

	print_process_header(wnd);
	for(i=0; i < MAX_PROCS; i++, p++){
		if(!p->used){
			continue;
		}
		print_process_details(wnd, p);
	}
}


/* initialize the global variables associated with process creation */
void init_process()
{
	int i;

	/* Clear all PCBs */
	for(i=1; i < MAX_PROCS; i++){
		pcb[i].magic = 0;
		pcb[i].used = FALSE;
	}

	/* Create free list; 1st entry used for boot process */
	for(i=1; i < MAX_PROCS -1; i++){
		pcb[i].next = &pcb[i+1];
	}
	pcb[MAX_PROCS - 1].next = NULL;
	next_free_pcb = &pcb[1];

	/* Define pcb[0] for this process */
	active_proc = pcb;
	pcb[0].magic      = MAGIC_PCB;
	pcb[0].used       = TRUE;
	pcb[0].priority   = 1;
	pcb[0].state      = STATE_READY;
	pcb[0].first_port = NULL;
	pcb[0].name       = "Boot process";


}


