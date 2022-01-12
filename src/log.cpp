#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>

#include "log.h"

// logging
#define ANSI_BOLD   "\e[1m"
#define ANSI_NORMAL "\e[0m"
#define ANSI_RED    "\e[38;5;9m"
#define ANSI_YELLOW "\e[38;5;11m"
#define ANSI_GREEN  "\e[38;5;10m"

#define ERROR_PREFIX   ANSI_RED    "(!)" ANSI_NORMAL " ERROR -> "
#define WARNING_PREFIX ANSI_YELLOW "(!)" ANSI_NORMAL " WARN --> "
#define INFO_PREFIX    ANSI_GREEN  "(!)" ANSI_NORMAL " INFO --> "

void error( const char *format, ... ){
  va_list ap;
  
  fputs( ERROR_PREFIX, stderr );
  va_start( ap, format );
  vfprintf( stderr, format, ap );
  va_end( ap );

  exit( 1 );
}

void warn( const char *format, ... ){
  va_list ap;
  
  fputs( WARNING_PREFIX, stderr );
  va_start( ap, format );
  vfprintf( stderr, format, ap );
  va_end( ap );
}

void info( const char *format, ... ){
  va_list ap;
  
  fputs( INFO_PREFIX, stderr );
  va_start( ap, format );
  vfprintf( stderr, format, ap );
  va_end( ap );
}