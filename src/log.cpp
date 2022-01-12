#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <pthread.h>

#include "log.h"

// logging
#define ANSI_BOLD   "\e[1m"
#define ANSI_NORMAL "\e[0m"
#define ANSI_ITALIC "\e[3m"
#define ANSI_RED    "\e[38;5;9m"
#define ANSI_YELLOW "\e[38;5;11m"
#define ANSI_GREEN  "\e[38;5;10m"

#define ERROR_PREFIX   ANSI_RED    "(!) ERROR " ANSI_NORMAL ANSI_ITALIC "[%s] " ANSI_NORMAL "--> "
#define WARNING_PREFIX ANSI_YELLOW "(!) WARN " ANSI_NORMAL ANSI_ITALIC "[%s] " ANSI_NORMAL "--> "
#define INFO_PREFIX    ANSI_GREEN  "(*) INFO " ANSI_NORMAL ANSI_ITALIC "[%s] " ANSI_NORMAL "--> "

void error( const char *format, ... ){
  va_list ap;
  char thread_name[100];
  
  pthread_getname_np( pthread_self(), thread_name, 100 );
  fprintf( stderr, ERROR_PREFIX, thread_name );

  va_start( ap, format );
  vfprintf( stderr, format, ap );
  va_end( ap );

  exit( 1 );
}

void warn( const char *format, ... ){
  va_list ap;
  char thread_name[100];
  
  pthread_getname_np( pthread_self(), thread_name, 100 );
  fprintf( stderr, WARNING_PREFIX, thread_name );

  va_start( ap, format );
  vfprintf( stderr, format, ap );
  va_end( ap );
}

void info( const char *format, ... ){
  va_list ap;
  char thread_name[100];
  
  pthread_getname_np( pthread_self(), thread_name, 100 );
  fprintf( stderr, INFO_PREFIX, thread_name );

  va_start( ap, format );
  vfprintf( stderr, format, ap );
  va_end( ap );
}
