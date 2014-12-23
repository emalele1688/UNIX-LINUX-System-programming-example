#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <string.h>


/* Questo esempio dimostra che qual'ora si decide di impostare segnali di interruzione per le slow-syscall 
 * oppure si prevede o si programma l'arrivo di un segnale in un determinato punto del programma,
 * allora bisogna prendere determinate accortezza 
 */

void alarm_hand(int a)
{
  return;
}

int m_sleep(unsigned int second)
{
  struct sigaction new_action, old_action;
  sigset_t old_mask, stop_mask, sleep_mask;
  
  memset(&new_action, 0, sizeof(new_action));
  memset(&old_action, 0, sizeof(old_action));  
  
  /* Set alarm action */
  sigemptyset(&new_action.sa_mask);
  new_action.sa_handler = alarm_hand;
  sigaction(SIGALRM, &new_action, &old_action);
  sigaction(SIGINT, &new_action, &old_action);
  
  /* WARNING:
   * Se il timer scade prima di mettere il processo in pausa il processo entrerà in pausa perenne (race condition)
   * Quindi blocco il segnale SIGALRM fino a che non entro in pausa
   */
  sigemptyset(&stop_mask);
  sigaddset(&stop_mask, SIGALRM);
  //Aggiungo alla maschera dei segnali bloccati anche il segnale SIGALRM
  sigprocmask(SIG_BLOCK, &stop_mask, &old_mask);
  
  //Predispongo il timer
  alarm(second);
  
  /* Porto il processo in pausa sbloccando il segnale SIGALRM durante la pausa */
  sleep_mask = old_mask;
  sigdelset(&sleep_mask, SIGALRM); //Elimino l'eventuale presenza del SIGALRM nella vecchia maschera dei segnali bloccati
  sigaddset(&sleep_mask, SIGINT); //Durante il periodo in cui il processo è in pausa blocco l'arrivo del segnale SIGINT
  sigsuspend(&sleep_mask); //Il processo viene bloccato ed i segnali presenti in sleep_mask non permetteranno di riprendere l'esecuzione del processo
  
  /* Ripristino la maschera dei segnali bloccati, e ripristino l'azione per SIGALRM */
  sigprocmask(SIG_SETMASK, &old_mask, NULL);
  sigaction(SIGALRM, &old_action, NULL);
  
  //Ritorna il tempo mancante in caso di sblocco anticipato
  return alarm(0);  
}

int main(int argn, char* args)
{
  int ret = m_sleep(2);
  printf("Tempo rimanente da aspettare %d\n", ret);
  return 0;
}
