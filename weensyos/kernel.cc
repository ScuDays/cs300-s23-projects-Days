#include "kernel.hh"
#include "k-apic.hh"
#include "k-vmiter.hh"
#include <atomic>

// kernel.cc
//
//    This is the kernel.


// INITIAL PHYSICAL MEMORY LAYOUT
//
//  +-------------- Base Memory --------------+
//  v                                         v
// +-----+--------------------+----------------+--------------------+---------/
// |     | Kernel      Kernel |       :    I/O | App 1        App 1 | App 2
// |     | Code + Data  Stack |  ...  : Memory | Code + Data  Stack | Code ...
// +-----+--------------------+----------------+--------------------+---------/
// 0  0x40000              0x80000 0xA0000 0x100000             0x140000
//                                             ^
//                                             | \___ PROC_SIZE ___/
//                                      PROC_START_ADDR

#define PROC_SIZE 0x40000       // initial state only

proc ptable[NPROC];             // array of process descriptors
                                // Note that `ptable[0]` is never used.
proc* current;                  // pointer to currently executing proc

#define HZ 100                  // timer interrupt frequency (interrupts/sec)
static std::atomic<unsigned long> ticks; // # timer interrupts so far
uintptr_t currentAddress;

// Memory state
//    Information about physical page with address `pa` is stored in
//    `pages[pa / PAGESIZE]`. In the handout code, each `pages` entry
//    holds an `refcount` member, which is 0 for free pages.
//    You can change this as you see fit.

pageinfo pages[NPAGES];


[[noreturn]] void schedule();
[[noreturn]] void run(proc* p);
void exception(regstate* regs);
uintptr_t syscall(regstate* regs);
void memshow();


// kernel(command)
//    Initialize the hardware and processes and start running. The `command`
//    string is an optional string passed from the boot loader.

static void process_setup(pid_t pid, const char* program_name);

void kernel(const char* command) {
    // Initialize hardware.
    init_hardware();
    log_printf("Starting WeensyOS\n");

    // Initialize timer interrupt.
    ticks = 1;
    init_timer(HZ);

    // Clear screen.
    console_clear();

    // (re-)Initialize the kernel page table.
    for (vmiter it(kernel_pagetable); it.va() < MEMSIZE_PHYSICAL; it += PAGESIZE) {

    /** Modified code*/
      if (it.va() != 0) {
        //对于内核的页面，用户不应该具有访问权限
             if(it.va() < 0x100000){
                if(it.va() == CONSOLE_ADDR){
                    it.map(it.va(), PTE_P | PTE_W | PTE_U);
                    //log_printf("Kernel :VA %p maps to PA %p with PERMS %p, %p, %p\n", it.va(), it.pa(), PTE_P, PTE_W, PTE_U);

                }
              else{
                it.map(it.va(), PTE_P | PTE_W);
                // log_printf("Kernel :VA %p maps to PA %p with PERMS %p, %p\n", it.va(), it.pa(), PTE_P, PTE_W);
              }
             }
             else{
            it.map(it.va(), PTE_P | PTE_W | PTE_U);
             //log_printf("Kernel :VA %p maps to PA %p with PERMS %p, %p, %p\n", it.va(), it.pa(), PTE_P, PTE_W, PTE_U);
            }
        } else {
            // nullptr is inaccessible even to the kernel
            it.map(it.va(), 0);
        }
    }
    // Set up process descriptors.
    for (pid_t i = 0; i < NPROC; i++) {
        ptable[i].pid = i;
        ptable[i].state = P_FREE;
    }
    if (command && program_loader(command).present()) {
        process_setup(1, command);
    } else {
        process_setup(1, "allocator");
        process_setup(2, "allocator2");
        process_setup(3, "allocator3");
        process_setup(4, "allocator4");
    }

    // Switch to the first process using run().
    run(&ptable[1]);
}


// kalloc(sz)
//    Kernel memory allocator. Allocates `sz` contiguous bytes and
//    returns a pointer to the allocated memory (the physical address of
//    the newly allocated memory), or `nullptr` on failure.
//
//    The returned memory is initialized to 0xCC, which corresponds to
//    the x86 instruction `int3` (this may help you debug). You can
//    reset it to something more useful.
//
//    On WeensyOS, `kalloc` is a page-based allocator: if `sz > PAGESIZE`
//    the allocation fails; if `sz < PAGESIZE` it allocates a whole page
//    anyway.
//
//    The stencil code returns the next allocatable free page it can find,
//    but it never reuses pages or supports freeing memory (you'll have to
//    change this at some point).

static uintptr_t next_alloc_pa;

void* kalloc(size_t sz) {
    if (sz > PAGESIZE) {
        return nullptr;
    }

    while (next_alloc_pa < MEMSIZE_PHYSICAL) {
        uintptr_t pa = next_alloc_pa;
        next_alloc_pa += PAGESIZE;

        if (allocatable_physical_address(pa)
            && !pages[pa / PAGESIZE].used()) {
            pages[pa / PAGESIZE].refcount = 1;
            memset((void*) pa, 0xCC, PAGESIZE);
            return (void*) pa;
        }
    }
    return nullptr;
}


// kfree(kptr)
//    Frees `kptr`, which must have been previously returned by `kalloc`.
//    If `kptr == nullptr` does nothing.

void kfree(void* kptr) {
    // Placeholder code below - you will have to implement `kfree`!
    (void) kptr;
    assert(false);
}


void process_setup(pid_t pid, const char* program_name) {
    init_process(&ptable[pid], 0);
    //The process_setup function should allocate a new, initially empty page 
    //table for the process. Do this by calling kalloc() (to allocate the 
    //level-4 page table page) 
    x86_64_pagetable* proc_pagetable = (x86_64_pagetable*) kalloc(PAGESIZE);

    if((void*) proc_pagetable == (void*) nullptr) {
        return;
    }

    ptable[pid].pagetable = proc_pagetable;

    //Copy the mappings from kernel_pagetable into this new page table using 
    //vmiter::map

    //Then calling memset (to ensure the page table is empty).
    memset(ptable[pid].pagetable, 0, PAGESIZE);
    ptable[pid].pagetable = proc_pagetable;

    // vmiter iterator(ptable[pid].pagetable);
    // for(vmiter it(kernel_pagetable); it.va() < PROC_START_ADDR; it += PAGESIZE) {
        
    //     // Get the physical address and permissions of the mapping
    //     uint64_t pa = it.pa();
    //     uint64_t perm = it.perm();

    //     // Map the virtual address to the physical address with the same permissions in the destination page table
    //     vmiter(ptable[pid].pagetable, it.va()).map(pa, perm);
    
    // }

       
for(vmiter kernel_It(kernel_pagetable, 0), process_It(proc_pagetable, 0)  ;kernel_It.va() < PROC_START_ADDR; kernel_It += PAGESIZE ,process_It += PAGESIZE){
        int PTEP = 0;
        int PTEW = 0;
        int PTEU = 0;
        if(kernel_It.present())PTEP = 1;
        if(kernel_It.writable())PTEW = 1;
        if(kernel_It.user())PTEU = 1;
       
        if(PTEP == 1 && PTEW ==0 && PTEU==0){
            process_It.try_map(process_It.va(), PTE_P);
             //log_printf("this is pid %d,VA %p maps to PA %p with PERMS %p, %p, %p\n", pid, process_It.va(), kernel_It.pa(), PTE_P, 0, 0);
    }
      if(PTEP == 1 && PTEW ==1 && PTEU==0){
            process_It.try_map(process_It.va(), PTE_P|PTE_W);
            // log_printf("this is pid %d,VA %p maps to PA %p with PERMS %p, %p, %p\n", pid, process_It.va(), kernel_It.pa(), PTE_P, PTE_W, 0);
    }
      if(PTEP == 1 && PTEW ==1 && PTEU==1){
            process_It.try_map(process_It.va(), PTE_P| PTE_W| PTE_U);
            // log_printf("this is pid %d,VA %p maps to PA %p with PERMS %p, %p, %p\n", pid, process_It.va(), kernel_It.pa(), PTE_P, PTE_W, PTE_U);
    }
    }



    // Initialize `program_loader`.
    // The `program_loader` is an iterator that visits segments of executables.
    program_loader loader(program_name);

    // Using the loader, we're going to start loading segments of the program binary into memory
    // (recall that an executable has code/text segment, data segment, etc).

    // First, for each segment of the program, we allocate page(s) of memory.
    for (loader.reset(); loader.present(); ++loader) {
        for (uintptr_t a = round_down(loader.va(), PAGESIZE);
             a < loader.va() + loader.size();
             a += PAGESIZE) {
            // `a` is the virtual address of the current segment's page.
            vmiter it(ptable[pid].pagetable, a);    

            //assert(!pages[a / PAGESIZE].used());

            // Read the description on the `pages` array if you're confused about what it is.
            // Here, we're directly getting the page that has the same physical address as the
            // virtual address `a`, and claiming that page by incrementing its reference count
            // (you will have to change this later).
            // pages[alloc_pa / PAGESIZE].refcount = 1;
            uintptr_t alloc_pa = (uintptr_t)(kalloc(PAGESIZE));
            it.try_map(alloc_pa, PTE_P | PTE_W * loader.writable() | PTE_U);
        }
    }

    // We now copy instructions and data into memory that we just allocated.
    for (loader.reset(); loader.present(); ++loader) {
        memset((void*) vmiter(ptable[pid].pagetable, loader.va()).pa(), 0, loader.size());
        memcpy((void*) vmiter(ptable[pid].pagetable, loader.va()).pa(), loader.data(), loader.data_size());
    }

    // Set %rip and mark the entry point of the code.
    ptable[pid].regs.reg_rip = loader.entry();


    // We also need to allocate a page for the stack.
    //uintptr_t stack_addr = PROC_START_ADDR + PROC_SIZE * pid - PAGESIZE;
    uintptr_t stack_addr = MEMSIZE_VIRTUAL - PAGESIZE;
    
    //assert(!pages[stack_addr / PAGESIZE].used());


    // Again, we're using the physical page that has the same address as the `stack_addr` to
    // maintain the one-to-one mapping between physical and virtual memory (you will have to change
    // this later).
    
    //pages[stack_addr / PAGESIZE].refcount = 1;

    vmiter stack_it(ptable[pid].pagetable, stack_addr);
    stack_it.try_map(kalloc(PAGESIZE), PTE_P | PTE_W | PTE_U);

    //You can map the stack when the kernel sets up the stack for the process.

    // Set %rsp to the start of the stack.
    ptable[pid].regs.reg_rsp = stack_addr + PAGESIZE;

    // Finally, mark the process as runnable.
    ptable[pid].state = P_RUNNABLE;
}



// exception(regs)
//    Exception handler (for interrupts, traps, and faults).
//    You should *not* have to edit this function.
//
//    The register values from exception time are stored in `regs`.
//    The processor responds to an exception by saving application state on
//    the kernel's stack, then jumping to kernel assembly code (see
//    k-exception.S). That code saves more registers on the kernel's stack,
//    then calls exception(). This way, the process can be resumed right where
//    it left off before the exception. The pushed registers are popped and
//    restored before returning to the process (see k-exception.S).
//
//    Note that hardware interrupts are disabled when the kernel is running.

void exception(regstate* regs) {
    // Copy the saved registers into the `current` process descriptor.
    current->regs = *regs;
    regs = &current->regs;

    // It can be useful to log events using `log_printf`.
    // Events logged this way are stored in the host's `log.txt` file.
    /* log_printf("proc %d: exception %d at rip %p\n",
                current->pid, regs->reg_intno, regs->reg_rip); */

    // Show the current cursor location and memory state (unless this is a kernel fault).
    console_show_cursor(cursorpos);
    if (regs->reg_intno != INT_PF || (regs->reg_errcode & PFERR_USER)) {
        memshow();
    }

    // If Control-C was typed, exit the virtual machine.
    check_keyboard();


    // Actually handle the exception.
    switch (regs->reg_intno) {

    case INT_IRQ + IRQ_TIMER:
        ++ticks;
        lapicstate::get().ack();
        schedule();
        break;                  /* will not be reached */

    case INT_PF: {
        // Analyze faulting address and access type.
        uintptr_t addr = rdcr2();
        const char* operation = regs->reg_errcode & PFERR_WRITE
                ? "write" : "read";
        const char* problem = regs->reg_errcode & PFERR_PRESENT
                ? "protection problem" : "missing page";

        if (!(regs->reg_errcode & PFERR_USER)) {
            panic("Kernel page fault for %p (%s %s, rip=%p)!\n",
                  addr, operation, problem, regs->reg_rip);
        }
        console_printf(CPOS(24, 0), 0x0C00,
                       "Process %d page fault for %p (%s %s, rip=%p)!\n",
                       current->pid, addr, operation, problem, regs->reg_rip);
        current->state = P_BROKEN;
        break;
    }

    default:
        panic("Unexpected exception %d!\n", regs->reg_intno);

    }

    // Return to the current process (or run something else).
    if (current->state == P_RUNNABLE) {
        run(current);
    } else {
        schedule();
    }
}


// syscall(regs)
//    System call handler.
//
//    The register values from system call time are stored in `regs`.
//    The return value, if any, is returned to the user process in `%rax`.
//
//    Note that hardware interrupts are disabled when the kernel is running.

// Headers for helper functions used by syscall.
int syscall_page_alloc(uintptr_t addr);
pid_t syscall_fork();
void syscall_exit();

uintptr_t syscall(regstate* regs) {
    // Copy the saved registers into the `current` process descriptor.
    current->regs = *regs;
    regs = &current->regs;

    // It can be useful to log events using `log_printf`.
    // Events logged this way are stored in the host's `log.txt` file.
    /* log_printf("proc %d: syscall %d at rip %p\n",
                  current->pid, regs->reg_rax, regs->reg_rip); */

    // Show the current cursor location and memory state (unless this is a kernel fault).
    console_show_cursor(cursorpos);
    memshow();

    // If Control-C was typed, exit the virtual machine.
    check_keyboard();

    // Actually handle the exception.
    switch (regs->reg_rax) {

    case SYSCALL_PANIC:
        panic(nullptr); // does not return

    case SYSCALL_GETPID:
        return current->pid;

    case SYSCALL_YIELD:
        current->regs.reg_rax = 0;
        schedule(); // does not return

    case SYSCALL_PAGE_ALLOC:
        return syscall_page_alloc(current->regs.reg_rdi);

    case SYSCALL_FORK:
        return syscall_fork();

    case SYSCALL_EXIT:
        syscall_exit();
        schedule(); // does not return

    default:
        panic("Unexpected system call %ld!\n", regs->reg_rax);

    }

    panic("Should not get here!\n");
}




// int syscall_page_alloc(uintptr_t addr) {
//     assert(!pages[addr / PAGESIZE].used());
//     // Currently we're simply using the physical page that has the same address
//     // as `addr` (which is a virtual address).

//     /** changed code*/
//     //判断是否在可以分配的内存范围内
//     vmiter CurrentVmt(kernel_pagetable, addr);
//     if(addr < PROC_START_ADDR | addr >= MEMSIZE_VIRTUAL){
//         return -1;
//     }
//     //判断是否具有访问权限
//     if(CurrentVmt.user() == 0 ){
//         return -1;
//     }
//     /** behind*/

//     pages[addr / PAGESIZE].refcount = 1;
//     memset((void*) addr, 0, PAGESIZE);
//     return 0;
// }


// syscall_page_alloc(addr)
//    Helper function that handles the SYSCALL_PAGE_ALLOC system call.
//    This function implement the specification for `sys_page_alloc`
//    in `u-lib.hh` (but in the stencil code, it does not - you will
//    have to change this).
int syscall_page_alloc(uintptr_t addr) {
    // Check if the address is within the range of the application region of memory.
    if (addr < PROC_START_ADDR || addr >= MEMSIZE_VIRTUAL) {
        return -1;
    }   
 
    //assert(!pages[addr / PAGESIZE].used());
    
    vmiter it(current->pagetable, addr);

    uintptr_t alloc_pa = (uintptr_t)(kalloc(PAGESIZE));

    if ((void *) alloc_pa == nullptr) {
        return -1;
    }

    //pages[addr / PAGESIZE].refcount = 1;

    memset((void*) alloc_pa, 0, PAGESIZE);

    it.try_map(alloc_pa, PTE_P | PTE_W | PTE_U);
    //log_printf("pid %d va addr:%p to pa addr: %p\n ",current->pid,addr, alloc_pa );
    return 0;
}


// syscall_fork()
//    Handles the SYSCALL_FORK system call. This function
//    implements the specification for `sys_fork` in `u-lib.hh`.
pid_t syscall_fork() {
    // Implement for Step 5!
    int child_pid = -1;
    for(int i = 1; i < NPROC; i++){
        if(ptable[i].state == P_FREE){
            child_pid = i;
            break;
        }
    }
    if(child_pid == -1){
        return -1;
    }

    ptable[child_pid].pid = child_pid;
    x86_64_pagetable* process_pagetable = (x86_64_pagetable*)kalloc(PAGESIZE);
    memset(process_pagetable, 0, PAGESIZE);
    ptable[child_pid].pagetable = process_pagetable;
   
    for(vmiter it(current->pagetable, 0), process_it(process_pagetable, 0); it.va() < MEMSIZE_VIRTUAL; it += PAGESIZE, process_it += PAGESIZE){
        bool user_access =it.user();
        uint64_t this_perm = it.perm();
        if(it.present() && !it.writable()){
            process_it.try_map(it.pa(), this_perm);
            pages[(uintptr_t)it.pa() / PAGESIZE].refcount += 1;
        }
        else if(user_access == true){
            if(it.va() == CONSOLE_ADDR){
                process_it.try_map(CONSOLE_ADDR, this_perm);
            }
            else{
                uintptr_t child_pa = (uintptr_t)(kalloc(PAGESIZE));
                memcpy((void*) child_pa, (void*) it.pa(), PAGESIZE);
                process_it.try_map(child_pa, this_perm);
            }
        }
        else {
            process_it.try_map(it.pa(), this_perm);
        }
        
    }

    ptable[child_pid].state = P_RUNNABLE;        // process state (see above)
    ptable[child_pid].regs = current->regs;      // process's current registers
    ptable[child_pid].regs.reg_rax = 0;
    ptable[current->pid].regs.reg_rax = child_pid;
    
    return child_pid;
}




// syscall_exit()
//    Handles the SYSCALL_EXIT system call. This function
//    implements the specification for `sys_exit` in `u-lib.hh`.
void syscall_exit() {
    // Implement for Step 7!
    panic("Unexpected system call %ld!\n", SYSCALL_EXIT);
}

// schedule
//    Picks the next process to run and then run it.
//    If there are no runnable processes, spins forever.
//    You should *not* have to edit this function.

void schedule() {
    pid_t pid = current->pid;
    for (unsigned spins = 1; true; ++spins) {
        pid = (pid + 1) % NPROC;
        if (ptable[pid].state == P_RUNNABLE) {
            run(&ptable[pid]);
        }

        // If Control-C was typed, exit the virtual machine.
        check_keyboard();

        // If spinning forever, show the memviewer.
        if (spins % (1 << 12) == 0) {
            memshow();
            log_printf("%u\n", spins);
        }
    }
}


// run(p)
//    Runs process `p`. This involves setting `current = p` and calling
//    `exception_return` to restore its page table and registers.
//    You should *not* have to edit this function.

void run(proc* p) {
    assert(p->state == P_RUNNABLE);
    current = p;

    // Check the process's current pagetable.
    check_pagetable(p->pagetable);

    // This function is defined in k-exception.S. It restores the process's
    // registers then jumps back to user mode.
    exception_return(p);

    // should never get here
    while (true) {
    }
}


// memshow()
//    Draws a picture of memory (physical and virtual) on the CGA console.
//    Switches to a new process's virtual memory map every 0.25 sec.
//    Uses `console_memviewer()`, a function defined in `k-memviewer.cc`.
//    You should *not* have to edit this function.

void memshow() {
    static unsigned last_ticks = 0;
    static int showing = 0;

    // switch to a new process every 0.25 sec
    if (last_ticks == 0 || ticks - last_ticks >= HZ / 2) {
        last_ticks = ticks;
        showing = (showing + 1) % NPROC;
    }

    proc* p = nullptr;
    for (int search = 0; !p && search < NPROC; ++search) {
        if (ptable[showing].state != P_FREE
            && ptable[showing].pagetable) {
            p = &ptable[showing];
        } else {
            showing = (showing + 1) % NPROC;
        }
    }

    extern void console_memviewer(proc* vmp);
    console_memviewer(p);
}
