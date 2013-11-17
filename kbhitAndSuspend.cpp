#include <stdio.h>
#include <termios.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/time.h>
 
// suspend the process (same as hitting ctrl-z)
//kill(pid, SIGSTOP);

// continue the process
//kill(pid, SIGCONT);
 
void changemode(int);
int  kbhit(void);
int main(void)
{
	changemode(1);
	while(1)
	{
		printf(".");
		int ch;
		if(kbhit())
		{
			ch = getchar();
			if(ch == 'p')
				printf("p is clicked\n");
			else if(ch == 'k')
				printf("k is clicked\n");
			break;
		}
	}
	changemode(0);
	printf("go out of while\n");
}
 
void changemode(int dir)
{
  static struct termios oldt, newt;
 
  if ( dir == 1 )
  {
    tcgetattr( STDIN_FILENO, &oldt);
    newt = oldt;
    newt.c_lflag &= ~( ICANON | ECHO );
    tcsetattr( STDIN_FILENO, TCSANOW, &newt);
  }
  else
    tcsetattr( STDIN_FILENO, TCSANOW, &oldt);
}
 
int kbhit (void)
{
  struct timeval tv;
  fd_set rdfs;
 
  tv.tv_sec = 0;
  tv.tv_usec = 0;
 
  FD_ZERO(&rdfs);
  FD_SET (STDIN_FILENO, &rdfs);
 
  select(STDIN_FILENO+1, &rdfs, NULL, NULL, &tv);
  return FD_ISSET(STDIN_FILENO, &rdfs);
 
}