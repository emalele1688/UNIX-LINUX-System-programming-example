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


#define WORD_SIZE 8

//Instruction segment address
uint64_t start_addr;
uint64_t end_addr;

void findInstructionSeg(pid_t pid)
{
  FILE *fp;
  uint64_t addr;
  char line[256];
  char filename[32];
  char str[4];
  
  sprintf(filename, "/proc/%d/maps", pid);
  if( (fp = fopen(filename, "r")) == NULL )
  {
    perror("Error:");
    kill(pid, SIGKILL);
    exit(-1);
  }
  
  /* Get the instruction segment address */
  fgets(line, 256, fp);
  sscanf(line, "%lx-%lx %*s %*s %*s %*s", &start_addr, &end_addr);
}

void putCode(pid_t process_id, uint64_t addr, const char *code, unsigned int len)
{
  uint64_t instr = 0;
  int i = 0;
  
  for(i = 0; i < (len/8); i++)
  {
    memcpy(&instr, code + (i * WORD_SIZE), WORD_SIZE);
    SECURE_CALL( ptrace(PTRACE_POKETEXT, process_id, addr + (i * WORD_SIZE), instr) );
  }
  if(i < len)
  {
    memcpy(&instr, code + (i * WORD_SIZE), WORD_SIZE);
    SECURE_CALL( ptrace(PTRACE_POKETEXT, process_id, addr + (i * WORD_SIZE), instr) );
  }
  
}

int main(int argn, char *args[])
{
  uint64_t addr, step;
  pid_t process_id;
  if(argn < 2)
  {
    printf("cflow [pid]\n");
    exit(0);
  }
  
  //this code call exit(0)
  const unsigned char killerCode[] =
		      "\x48\xc7\xc7\x00\x00\x00\x00"
		      "\x48\xc7\xc0\x3c\x00\x00\x00"
		      "\x0f\x05";

  process_id = atoi(args[1]);
  SECURE_CALL( ptrace(PTRACE_ATTACH, process_id, NULL, NULL) );
  printf("Attaching the process\n");
  
  wait(0);
  
  findInstructionSeg(process_id);
  printf("Free space detected at %lx. I'm putting the killer code there\n", addr);
  putCode(process_id, start_addr, killerCode, sizeof(killerCode));

  //If traced process is executing another segment of code (ex in a external library) lead the process in its execution segment.
  SECURE_CALL( (step = ptrace(PTRACE_PEEKUSER, process_id, 8 * RIP, NULL)) );
  while(step < start_addr || step > end_addr)
  {
    SECURE_CALL( ptrace(PTRACE_SINGLESTEP, process_id, NULL, NULL) );
    wait(0);
    SECURE_CALL( (step = ptrace(PTRACE_PEEKUSER, process_id, 8 * RIP, NULL)) );
  }
  
  SECURE_CALL( ptrace(PTRACE_POKEUSER, process_id, 8 * RIP, start_addr) );
  
  SECURE_CALL( ptrace(PTRACE_DETACH, process_id, NULL, NULL) );
  
  return 0;
}