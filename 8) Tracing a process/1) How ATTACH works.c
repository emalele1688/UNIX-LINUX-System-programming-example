#include <unistd.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sys/ptrace.h>
#include <sys/wait.h>
#include <sys/user.h>
#include <sys/reg.h>


#define SECURE_CALL(_syscall)				\
do{							\
  if(_syscall == -1)					\
  {							\
    fprintf(stderr, "%s\n", strerror(errno));		\
    exit(-1);						\
  }							\
}while(0)



void get_instruction(pid_t process_id, long instruction_addr, uint64_t *ret_instruction)
{
  /* PTRACE_PEEKTEXT => It's like PTRACE_PEEKDATA. It gets data from memory at address 'addr' (third parameter of ptrace)
   * REMEMBER: Linux doesn't have separate area for text and data
   */
  SECURE_CALL( (*ret_instruction = ptrace(PTRACE_PEEKTEXT, process_id, instruction_addr, NULL)) );
}

void put_instruction(pid_t process_id, long instruction_addr, const uint64_t new_instruction)
{
  /* PTRACE_POKETEXT => It's like PTRACE_POKEDATA. It puts a data (which is length a word size) in the memory pointed from addr
   * REMEMBER: Linux doesn't have separate area for text and data
   */
  SECURE_CALL( (ptrace(PTRACE_POKETEXT, process_id, instruction_addr, new_instruction)) );
}

int main(int argn, char *argv[])
{
  uint64_t sv_instruction;
  pid_t traced_process_id;
  struct user_regs_struct regs;
  
  /* The INT 3 instruction is defined for use by debuggers to temporarily replace an instruction in a running program in order to set a breakpoint.
   * Its opcode is (0xcc)
   * When the kernel reveive a INT 3 interrupt it sends a SIGCHLD signal to the tracing process (this process)
   */
  
  char instruction_trap[8];
  memset(instruction_trap, 0, 8);
  
  instruction_trap[0] = 0xcc;

  memset(&regs, 0, sizeof(struct user_regs_struct));
  
  if(argn != 2)
  {
    printf("usage: %s <pid to be traced>\n", argv[0]);
    exit(0);
  }
  
  traced_process_id = atoi(argv[1]);
  
  /* PTRACE_ATTACH => Trace the process identified by traced_process_id. The traced process set its parent to the tracing process (this process).
   * When a process is attached to a tracing process a SIGSTP is sent to the tracing process in order to stop it.
   */
  SECURE_CALL( ptrace(PTRACE_ATTACH, traced_process_id, NULL, NULL) );
  printf("Start tracing\n");
  
  wait(0);
  
  // PTRACE_GETREGS => Get the registers state of the traced process
  SECURE_CALL( ptrace(PTRACE_GETREGS, traced_process_id, NULL, &regs) );

  // Get instruction from the text memory
  get_instruction(traced_process_id, regs.rip, &sv_instruction);
  printf("Instruction %lx replaced with trap instruction (INT 3) at address %lx\n", sv_instruction, (uint64_t)regs.rip);
  
  // Put the trap instruction into the text memory
  put_instruction(traced_process_id, regs.rip, (uint64_t)instruction_trap);
  
  // PTRACE_CONT => Restart the execute of the traced process
  SECURE_CALL( ptrace(PTRACE_CONT, traced_process_id, NULL, NULL) );
  
  // Now, I'm waiting the system execute the trap instruction
  wait(0);
  
  uint64_t ins;
  SECURE_CALL( (ins = ptrace(PTRACE_PEEKUSER, traced_process_id, 8 * RIP, NULL)) );
  printf("The process is stopped at address %lx. Press enter to restore it\n", ins);
  getchar();
  
  // Restore the trap instruction with the original instruction
  put_instruction(traced_process_id, regs.rip, sv_instruction);
  
  // PTRACE_SETREGS => Setting the rip register to the instruction executed when the process attached (8 byte first the trap instruction)
  SECURE_CALL( ptrace(PTRACE_SETREGS, traced_process_id, NULL, &regs) );
  
  // Let's the process continue
  SECURE_CALL( ptrace(PTRACE_DETACH, traced_process_id, NULL, NULL) );
    
  return 0;
}
