#include "task.h"

// return a valid PID
// Create a task that starts executing at a memory location

// When switching to another process, a switch command comes in. That contains a from/to PID, and everything is saved/loaded.
// If the OS re-calls control during a program, everything is saved first.
// The system stack is also loaded

// When the CPU task scheduler calls a switch, the current process should be saved immediatly (sent directly by ch0_hook)

// When the program calls a syscall, the stack can be kept on as the user stack for that time.
// If the OS calls for a return later during a syscall or interrupt, none can happen until the cpu is done with the syscall

// For the times where it needs to update registers, and return, switch to pure asm

// usestakc is for using the existing stack
uint32_t process_create(uint32_t ptr, bool usestack) {
    uint32_t stack_ptr;
    uint16_t list_ptr;
    bool taskfound = false;
    for (uint16_t i=0;i<MAX_PROCESSES;i++) {
        if (!tasklist[i].flags) {
            // dead process, can be overwritten
            if (!usestack) {
                stack_ptr = malloc(STACK_LEN);
            }
            list_ptr = i;
            taskfound = true;
            break;
        }
    }
    if (!taskfound) {
        return 0; // maximum tasks exceeded
    }
    if (!stack_ptr) {
        return 0; // out of memory
    }
    //task_object task;
    pid++;

    tasklist[list_ptr].PID = pid;
    tasklist[list_ptr].ret_addr = ptr; // just start at the pointer
    if (!usestack) {
        tasklist[list_ptr].esp = (stack_ptr + STACK_LEN); // since the stack decrements, start on top
        tasklist[list_ptr].esp_ptr = (stack_ptr + STACK_LEN);
    }
    tasklist[list_ptr].flags = 1;
    uint32_t esp;
    if (usestack) {
        __asm__ (
            "mov %0, %%esp;"         // Move eax to 'result'
            : "=r"(esp)            // Output operand: result variable
            :                        // No input operands
            : "%eax"                  // Clobbered register: eax is modified
        );
        tasklist[list_ptr].esp = esp;
        tasklist[list_ptr].esp_ptr = esp;
    }
    //memcpy(&tasklist[list_ptr], &task, sizeof(task_object));
    
    // wait for the kernel to switch to the task passivly
    
    return pid;
}

bool process_kill(uint32_t in_pid) {
    for (uint16_t i=0;i<MAX_PROCESSES;i++) {
        // If task exists and has a matching PID
        if (tasklist[i].flags && tasklist[i].PID == in_pid) {
            tasklist[i].flags = 0; // prevent it from being called
            // The task scheduler will free up the stack and memory when it finds it is unused
            return 1; // success
        }
    }
    return 0; // failed
}

// called on every PIT
void process_check() {
    //shell_tty_print("W\n");
    //shell_memory_render();
    // First, back up the current cpu state
    foundIndex = 0;
    foundTask = false;
    // Since this is an interrupt with a function called within, go up 4 on the stack and grab the return address.
    // Usually called during an iret, we can re-use it later

    // Four to leave function
    uint16_t old_index;
    bool isOld = false;
    // Concurrent PID starts as 0, there are no tasks with a PID of 0
    for (uint16_t i=0;i<MAX_PROCESSES;i++) {
        if (tasklist[i].flags && tasklist[i].PID == concurrent_PID) {
            old_index = i;
            isOld = true;
            memcpy(&taskstates[i], &state, sizeof(task_state)); // copy CPU state to the current process
            break;
        }
    }

    // Now search for another workable process and switch there. Also free any dead stacks
    if (index_used >= MAX_PROCESSES) {
        index_used = 0;
    }

    for (uint16_t i=index_used;i<MAX_PROCESSES;i++) {
        // free the stack on any dead processes
        if (!tasklist[i].flags && tasklist[i].esp) {
            free(tasklist[i].esp_ptr, STACK_LEN);
            tasklist[i].esp = 0;
        }
        if (tasklist[i].PID != concurrent_PID && tasklist[i].flags && !foundTask) {
            foundIndex = i;
            foundTask = true;
            index_used = i;
        }
    }
    
    // Found a task to jump to
    if (foundTask) {
        if (isOld) {
            tasklist[old_index].ret_addr = ret_addr;
            tasklist[old_index].esp = tmp_esp; // from before we got an interrupt
        }
        concurrent_PID = tasklist[foundIndex].PID;
        //shell_tty_print(uhex32_str(concurrent_PID));
        //shell_memory_render();
        
        // Download the new process area, set the return address there and return from interrupt
        
        return;
    } else {
        // No other process found, or none exists, continue execution
        return;
    }
    
    return;
}