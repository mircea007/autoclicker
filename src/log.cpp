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
#define ANSI_GREEN  "\e[38;5;10m"
#define ANSI_YELLOW "\e[38;5;11m"
#define ANSI_BLUE   "\e[38;5;12m"
#define ANSI_PURPLE "\e[38;5;13m"

#define ERROR_PREFIX   ANSI_RED    "(!) ERROR " ANSI_NORMAL ANSI_ITALIC "[%s] " ANSI_NORMAL "--> "
#define WARNING_PREFIX ANSI_YELLOW "(!) WARN "  ANSI_NORMAL ANSI_ITALIC "[%s] " ANSI_NORMAL "--> "
#define INFO_PREFIX    ANSI_GREEN  "(*) INFO "  ANSI_NORMAL ANSI_ITALIC "[%s] " ANSI_NORMAL "--> "
#define DEBUG_PREFIX   ANSI_PURPLE "(*) DEBUG " ANSI_NORMAL ANSI_ITALIC "[%s] " ANSI_NORMAL "--> "

#define LOG_INDEX_WARN 0
#define LOG_INDEX_INFO 1
#define LOG_INDEX_DEBUG 2

int allow[3];

void log_set_verbose_level( int mask ){
  allow[LOG_INDEX_WARN]  = !!(mask & LOG_ALLOW_WARN);
  allow[LOG_INDEX_INFO]  = !!(mask & LOG_ALLOW_INFO);
  allow[LOG_INDEX_DEBUG] = !!(mask & LOG_ALLOW_DEBUG);
}

void log_error( const char *format, ... ){
  va_list ap;
  char thread_name[100];
  
  pthread_getname_np( pthread_self(), thread_name, 100 );
  fprintf( stderr, ERROR_PREFIX, thread_name );

  va_start( ap, format );
  vfprintf( stderr, format, ap );
  va_end( ap );

  exit( 1 );
}

void log_warn( const char *format, ... ){
  if( !allow[LOG_INDEX_WARN] );
  
  va_list ap;
  char thread_name[100];
  
  pthread_getname_np( pthread_self(), thread_name, 100 );
  fprintf( stderr, WARNING_PREFIX, thread_name );

  va_start( ap, format );
  vfprintf( stderr, format, ap );
  va_end( ap );
}

void log_info( const char *format, ... ){
  if( !allow[LOG_INDEX_INFO] );
  
  va_list ap;
  char thread_name[100];
  
  pthread_getname_np( pthread_self(), thread_name, 100 );
  fprintf( stderr, INFO_PREFIX, thread_name );

  va_start( ap, format );
  vfprintf( stderr, format, ap );
  va_end( ap );
}

void log_debug( const char *format, ... ){
  if( !allow[LOG_INDEX_DEBUG] );
  
  va_list ap;
  char thread_name[100];
  
  pthread_getname_np( pthread_self(), thread_name, 100 );
  fprintf( stderr, DEBUG_PREFIX, thread_name );

  va_start( ap, format );
  vfprintf( stderr, format, ap );
  va_end( ap );
}

void log_warn_bypass( const char *format, ... ){
  va_list ap;
  char thread_name[100];
  
  pthread_getname_np( pthread_self(), thread_name, 100 );
  fprintf( stderr, WARNING_PREFIX, thread_name );

  va_start( ap, format );
  vfprintf( stderr, format, ap );
  va_end( ap );
}

void log_info_bypass( const char *format, ... ){
  va_list ap;
  char thread_name[100];
  
  pthread_getname_np( pthread_self(), thread_name, 100 );
  fprintf( stderr, INFO_PREFIX, thread_name );

  va_start( ap, format );
  vfprintf( stderr, format, ap );
  va_end( ap );
}

void log_debug_bypass( const char *format, ... ){
  va_list ap;
  char thread_name[100];
  
  pthread_getname_np( pthread_self(), thread_name, 100 );
  fprintf( stderr, DEBUG_PREFIX, thread_name );

  va_start( ap, format );
  vfprintf( stderr, format, ap );
  va_end( ap );
}
