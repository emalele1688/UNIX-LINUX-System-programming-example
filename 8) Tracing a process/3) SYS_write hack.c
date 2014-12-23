/* WARNING: Compatibile solo su architectura X86_64
 * testato con Intel I7 core 2600k
 */

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/ptrace.h>		//For ptrace
#include <sys/syscall.h>	//For SYS_write etc
#include <sys/user.h>		//For register access (in realtà non serve includerla)
#include <sys/reg.h>		//define ORIG_RAX (15) for X86_64 architecture

#define DEFAULT_SIZE	1024
#define WORD_SIZE	sizeof(long)

#define EXIT(errno)				\
do {						\
    fprintf(stderr, "%s\n", strerror(errno));		\
    exit(-1);					\
} while(0)


pid_t child;


void reverse(char *string, unsigned int len)
{
  int i, j;
  char temp;
  
  if(string[len - 1] == '\n')
    len -= 1;
  
  for(i = 0, j = len - 1; i < j; i++, j--)
  {
    temp = string[i];
    string[i] = string[j];
    string[j] = temp;
  }
}

void hackTheString(char *string, long dataPtr, unsigned int len)
{
  long strbl;
  int i = 0;
  
  const unsigned int j = len / WORD_SIZE;
  
  /* ------ Get the string from traced process memory ------ */
  while(i < j)
  {
    /* PTRACE_PEEKDATA => return a word in the traced data memory process pointed by dataPtr (string pointer passed as second parameter to write syscall) */
    strbl = ptrace(PTRACE_PEEKDATA, child, dataPtr + (i * WORD_SIZE), NULL);
    memcpy(string + (i * WORD_SIZE), &strbl, WORD_SIZE);
    i++;
  }
  if(j < len)
  {
    strbl = ptrace(PTRACE_PEEKDATA, child, dataPtr + (i * WORD_SIZE), NULL);
    memcpy(string + (i* WORD_SIZE), &strbl, len - (i * WORD_SIZE));
  }
 
  /* ------ Reverse the string ------ */
  reverse(string, len);
  
  /* ------ Put the string into the traced process memory ------ */
  i = 0;
  strbl = 0;
  while(i < j)
  {
    /* PTRACE_POKEDATA => Put a word in the data memory of traced process. The word is set at 'addr' address (third argument of ptrace) and its value is get from 'data = strbl' (fourth argument of ptrace)  */
    memcpy(&strbl, string + (i * WORD_SIZE), WORD_SIZE);
    ptrace(PTRACE_POKEDATA, child, dataPtr + (i * WORD_SIZE), strbl);
    i++;
  }
  if(j < len)
  {
    strbl = 0;
    memcpy(&strbl, string + (i * WORD_SIZE), len - (i * WORD_SIZE));
    ptrace(PTRACE_POKEDATA, child, dataPtr + (i * WORD_SIZE), strbl);
  }
}

void tracedProcess(const char *path)
{ 
  child = fork();
  
  if(child == -1)
    EXIT(errno);
  else if(child != 0)
    return;
  
  //Child process will be traced by this process (parent)
  if( ptrace(PTRACE_TRACEME, 0, NULL, NULL) == -1 )
    EXIT(errno);

  //Execute child process
  if( execl(path, path, NULL) == -1 )
    EXIT(errno);
}

int main(int argn, char *args[])
{
  char *string;
  long orig_rax;
  long params[2];
  int status = 0, toggle = 0;
  unsigned long string_max_size;
    
  if(argn < 2)
  {
    printf("tracer [executable]\n");
    exit(0);
  }
  
  //Start child
  tracedProcess(args[1]);
  
  //This area save the incoming string from child process
  string = malloc(DEFAULT_SIZE);
  string_max_size = DEFAULT_SIZE;
  
  while(1)
  {
    //wait for the end of child .. OR child call a syscall
    if(wait(&status) == -1)
    {
      if(errno == EINTR)
	continue;
      else
	EXIT(0);
    }

    if(WIFEXITED(status))
    {
      printf("exit\n");
      break;
    }
    
    /* What syscall called the child process? 
     * PTRACE_PEEKUSER => Read a word at offset addr in the tracee's user memory. It holds information about register and other process information
     * Here, I get the value of rax when syscall (the assembly routine) was called
     */
    orig_rax = ptrace(PTRACE_PEEKUSER, child, WORD_SIZE * ORIG_RAX, NULL); 
    if(orig_rax == -1)
    {
      fprintf(stderr, "%s\n", strerror(errno));
      kill(child, SIGKILL);
      exit(-1);
    }
    
    //Is It a write syscall and is It starting now?
    if(orig_rax == SYS_write && toggle == 0)
    {
      toggle = 1;
      
      //We left information about file descriptor. It is not important
      //params[0] = ptrace(PTRACE_PEEKUSER, child, WORD_SIZE * RDI, NULL);	//write file descriptor
      
      //get rsi register
      params[0] = ptrace(PTRACE_PEEKUSER, child, WORD_SIZE * RSI, NULL);	//write data pointer
      //get rdx register
      params[1] = ptrace(PTRACE_PEEKUSER, child, WORD_SIZE * RDX, NULL);	//write length
      
      if(params[1] > string_max_size)
      {
	string = realloc(string, params[2]);
	string_max_size = params[2];
      }
      
      memset(string, 0, string_max_size);
      //Start string hacking
      hackTheString(string, params[0], params[1]);
    }
    else if(orig_rax == SYS_write && toggle == 1)
      toggle = 0;
    
    ptrace(PTRACE_SYSCALL, child, 0, 0);
  }
  
  return 0;
} 
