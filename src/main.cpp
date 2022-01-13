#include <stdio.h>   // duh
#include <unistd.h>  // usleep()
#include <signal.h>  // Ctrl+C

#include <pthread.h> // for async stuff

#include "log.h"
#include "mimic.h"

MimicMouseButFaster *copy;

void kill_handle( int s ){
  fputc( '\r', stderr );// hide ^C when run in terminal
  log_info( "Quiting...\n" );
  
  if( copy ){
    delete copy;
    copy = NULL;
  }

  // self destruct
  signal( SIGINT, SIG_DFL );
  raise( SIGINT );
}

int main( int argc, char *argv[] ){
  double cps = DEFAULT_CPS;
  
  log_set_verbose_level( LOG_ALLOW_WARN | LOG_ALLOW_INFO | LOG_ALLOW_DEBUG );
  
  pthread_setname_np( pthread_self(), "main" );

  if( signal( SIGINT, kill_handle ) == SIG_ERR )
    log_error( "Unable to set SIGINT handler\n" );
  
  if( argc > 1 )
    if( sscanf( argv[1], "%lf", &cps ) != 1 )
      log_error( "Clickrate (CPS) value must be a number\n" );
  
  log_debug( "Clickrate (CPS) is set to %lf\n", cps );
  
  copy = new MimicMouseButFaster( cps );

  // low CPU usage infinite loop
  while( 1 )
    usleep( 10'000'000 );
  
  return 0;
}