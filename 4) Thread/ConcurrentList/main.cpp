#include <iostream>
#include <pthread.h>
#include "ConcurrentList.h"

pthread_t thread1;
pthread_t thread2;
pthread_mutex_t lock;
ConcurrentList<int> list;

void *add(void* arg)
{
  pthread_mutex_lock(&lock);
  //sleep(1);
  list.pushBack(int(arg));
  pthread_mutex_unlock(&lock);
}

void *read(void* arg)
{
  for(int i = 0; i < 100; i++)
  {
    std::cout << "Leggo il dato ";
    std::cout << list.popFront() << std::endl;
  }
 
  pthread_exit(0);
}

int main(int argc, char **argv) 
{
    std::cout << "Hello!" << std::endl;
    pthread_mutex_init(&lock, NULL);
    pthread_create(&thread1, NULL, read, NULL);
    for(int i = 0; i < 100; i++)
      pthread_create(&thread2, NULL, add, (void*)i);
    
    pthread_join(thread2, NULL);
    pthread_join(thread1, NULL);
    
    list.pushBack(5);
    list.pushBack(10);
    list.pushBack(4);
    list.pushBack(12);
    //Attenzione: pushFront
    list.pushFront(9);
    ConcurrentList<int>::cIterator iter;
    for(iter = list.begin(); iter != list.end(); iter++)
    {
      int i = *iter;
      std::cout << i << std::endl;
    }
    
    std::cout << "Eseguo la cancellazione singola (10)\n";
    
    iter = list.begin();
    iter++; iter++;
    iter = list.erase(iter);
    
    for(iter = list.begin(); iter != list.end(); iter++)
    {
      int i = *iter;
      std::cout << i << std::endl;
    }
    
    std::cout << "Eseguo le cancellazioni multiple (9 - 5) \n"; 
    iter = list.begin();
    iter++; iter++;
    iter = list.erase(list.begin(), iter);
    
    for(iter = list.begin(); iter != list.end(); iter++)
    {
      int i = *iter;
      std::cout << i << std::endl;
    }
    
    return 0;
}
