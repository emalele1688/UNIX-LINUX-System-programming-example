#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <execinfo.h>


/* Obtain a backtrace and print it to stdout. */
void print_trace(int sig)
{
  void *array[10];
  size_t size, i;
  char **strings;

  size = backtrace(array, 10);
  strings = backtrace_symbols(array, size);

  printf ("Obtained %zd stack frames.\n", size);

  for (i = 0; i < size; i++)
     printf("%s\n", strings[i]);

  free(strings);
  
  exit(-1);
}

void compute(void)
{
  //sim compute
  usleep(10);
}

void fault(void)
{
  char *p = (char*)0x1;
  *p = 0;
}

/* A dummy function to make the backtrace more interesting. */
void dummy(void)
{
  compute();
  fault();
}

int main(void)
{
  signal(SIGSEGV, print_trace);
  dummy();
  return 0;
}