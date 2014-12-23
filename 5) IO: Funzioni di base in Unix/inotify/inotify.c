#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sys/inotify.h>
#include <sys/types.h>

#define EVENT_SIZE	(sizeof(struct inotify_event))
#define EVENT_BUFFER	(1024 * (EVENT_SIZE + 16))

#define EXIT()				\
{					\
  printf("%s\n", strerror(errno));	\
  exit(-1);				\
}

/* inotify permette di ricevere eventi qual'ora un inode viene creato/modificato/eliminato */

int main(int argn, char *argv[])
{
  struct inotify_event* event;
  ssize_t length;
  int inotify_ds, watched_ds, i;
  char buffer[EVENT_BUFFER];
  
  if( (inotify_ds = inotify_init()) == -1)
    EXIT();
  
  if( (watched_ds = inotify_add_watch(inotify_ds, ".", IN_MODIFY | IN_CREATE | IN_DELETE)) == -1)
    EXIT();
  
  while(1)
  {
    memset(buffer, 0, EVENT_BUFFER);
    
    if( (length = read(inotify_ds, &buffer, EVENT_BUFFER)) == -1)
    {
      if(errno == EINTR)
	continue;
      else
	EXIT();
    }
  
    i = 0;
    while(i < length)
    {
      event = (struct inotify_event*) buffer + i;
      
      printf("watched descriptor %i\n", event->wd);
      
      if(event->len)
      {
	if(event->mask & IN_CREATE) 
	  printf("The file %s was created.\n", event->name);
	else if(event->mask & IN_DELETE)
	  printf("The file %s was deleted.\n", event->name);
	else if(event->mask & IN_MODIFY)
	  printf("The file %s was modified.\n", event->name);
      }
      
      i += EVENT_SIZE + event->len;
    }
  }
}
