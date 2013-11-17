#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
 
void h_sigstop ( int sig ) {
  printf("Never happens (%d)\n",sig);
}
void h_sigcont ( int sig ) {
  printf("Huh? what? (%d)\n",sig);
}
 
int main( void )
{
  int i;
  printf("Running as PID=%d\n",getpid());
  signal(SIGCONT,h_sigcont);
  signal(SIGSTOP,h_sigstop);
  for ( i = 0 ; i < 10 ; i++ ) {
    printf("Loop=%d\n",i);
    if ( i == 5 ) kill(getpid(),SIGSTOP);
    sleep(2);
  }
  return 0;
}