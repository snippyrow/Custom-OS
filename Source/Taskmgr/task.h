struct task_state {
    uint32_t eax; // general purpose eax-edx
    uint32_t ebx;
    uint32_t ecx;
    uint32_t edx;
    uint32_t esp; // stack pointer
    uint32_t eflags; // flags
    uint32_t edi;
    uint32_t esi;
} __attribute__ ((packed));


// Do not restore CPU, just jump (8 bytes long)
// good if multiple length of 16
struct task_object {
    uint32_t esp; // stack pointer
    uint32_t ret_addr;
    uint32_t PID;
    uint32_t esp_ptr;
    uint16_t flags; // 0 if dead process
    uint16_t reserved;
} __attribute__ ((packed));

task_state state;

uint32_t MAX_PROCESSES = 32;
#define STACK_LEN 1024

// maximum 32 tasks, though alternating PID's
task_object tasklist[32];
task_state taskstates[32]; // linked with the tasklist by index

uint32_t concurrent_PID = 0;

// ensure all processes get a turn
uint16_t index_used = 0;

extern "C" void gen_stub();
// Save everything to a valid task, usually called immediatly before a switch
extern "C" bool process_kill(uint32_t in_pid);
void shell_memory_render();
//extern "C" void process_check();
uint32_t pid = 0;

uint32_t foundIndex = 0;
uint32_t ret_addr;
uint32_t tmp_esp;
uint32_t var_esp;
bool foundTask;

bool processEnabled = true;