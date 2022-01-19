#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <pthread.h>

#include "log.h"

static unsigned int mask;// initialy allow everything

void log_set_verbose_level( unsigned int _mask ){
  mask = _mask | LOG_WILDCARD;
}

void _log_error( const char *format, ... ){
  va_list ap;

  va_start( ap, format );
  vfprintf( stderr, format, ap );
  va_end( ap );

  exit( 1 );
}

void log_base( unsigned int priority, const char *format, ... ){
  if( !(mask & priority) ) return;
  
  va_list ap;

  va_start( ap, format );
  vfprintf( stderr, format, ap );
  va_end( ap );
}

