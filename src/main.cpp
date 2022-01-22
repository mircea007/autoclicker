#include <stdio.h>   // duh
#include <signal.h>  // Ctrl+C
#include <string.h>  // strcmp()

#include "log.h"
#include "mimic.h"
#include "os_specific.h"

#ifdef OS_IS_UNIX

#include <unistd.h>  // usleep()

#endif

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

const int NUM_LONG_ARGS = 2;

struct LongFormatArgument {
  const char *name;
  int num_param; // right now only works with 0 or 1
  double value;
} long_args[NUM_LONG_ARGS] = {
  { "cps", 1, DEFAULT_CPS },
  { "help", 0, 0 },
};

int main( int argc, char *argv[] ){
  int frecv[128];// frequency of each single-character option
  int arg, i;
  int do_warn, do_info, do_debug, do_help;
  
  for( i = 0 ; i < 128 ; i++ )
    frecv[i] = 0;
  
  // parse command line arguments
  arg = 1;
  while( arg < argc ){
    if( argv[arg][0] != '-' )
      log_warn_bypass( "Unknown argument '%s'\n", argv[arg] );
    else if( argv[arg][1] != '-' ){// each character is an option
      i = 1;
      while( argv[arg][i] )
        frecv[(int)argv[arg][i++]]++;// add each character to frequency
    }else{// long format option => the argument is an option and may have an extra argument imediately after
      // ineficient (in terms of completixy)
      // if in the future i add more parameters 
      // then i will use a trie
      
      i = 0;
      while( i < NUM_LONG_ARGS && strcmp( argv[arg] + 2, long_args[i].name ) )
        i++;
      
      if( i >= NUM_LONG_ARGS )
        log_warn( "Unrecognized parameter %s\n", argv[arg] );
      
      if( !long_args[i].num_param )
        long_args[i].value++;// increase frequency
      else{
        if( arg + 1 < argc ){
          arg++;
          if( sscanf( argv[arg], "%lf", &long_args[i].value ) != 1 )
            log_error( "Value for parameter %s must be a number\n", argv[arg] );
        }else
          log_error( "Missing value for parameter %s\n", argv[arg] );
      }
    }
    
    arg++;
  }
  
  do_warn = !!frecv['w'];
  do_info = !!frecv['i'];
  do_debug = !!frecv['d'];
  do_help = !!(frecv['h'] || long_args[1].value);

  frecv['w'] = frecv['i'] = frecv['d'] = frecv['h'] = 0;
  
  if( do_help ){
    printf("Help page not yet written -- contribute on github:\n  https://github.com/mircea007/autoclicker\n");
    return 0;
  }
  
  for( i = 0 ; i < 128 ; i++ )
    if( frecv[i] )
      log_warn( "Unknown flag %c\n", i );
  
  log_set_verbose_level(
    (LOG_ALLOW_WARN  * do_warn) |
    (LOG_ALLOW_INFO  * do_info) |
    (LOG_ALLOW_DEBUG * do_debug)
  );

  if( signal( SIGINT, kill_handle ) == SIG_ERR )
    log_error( "Unable to set SIGINT handler\n" );
  
  log_debug( "Clickrate (CPS) is set to %lf\n", long_args[0].value );
  
  copy = new MimicMouseButFaster( long_args[0].value );

  // low CPU usage infinite loop
  while( 1 )
    usleep( 10'000'000 );
  
  return 0;
}