#pragma once

// prepare for a lot of macro magic

#define LOG_ALLOW_DEBUG 1
#define LOG_ALLOW_INFO  2
#define LOG_ALLOW_WARN  4
#define LOG_WILDCARD    8

// ansi
#define ANSI_BOLD   "\e[1m"
#define ANSI_NORMAL "\e[0m"
#define ANSI_ITALIC "\e[3m"
#define ANSI_RED    "\e[38;5;9m"
#define ANSI_GREEN  "\e[38;5;10m"
#define ANSI_YELLOW "\e[38;5;11m"
#define ANSI_BLUE   "\e[38;5;12m"
#define ANSI_PURPLE "\e[38;5;13m"

#define ERROR_PREFIX   ANSI_RED    "(!) ERROR " ANSI_NORMAL
#define WARNING_PREFIX ANSI_YELLOW "(!) WARN "  ANSI_NORMAL
#define INFO_PREFIX    ANSI_GREEN  "(*) INFO "  ANSI_NORMAL
#define DEBUG_PREFIX   ANSI_PURPLE "(*) DEBUG " ANSI_NORMAL

// fix that i found on stackoverflow for stringizing numerical constatns
#define STRINGIFY_( x ) #x
#define STRINGIFY( x ) STRINGIFY_( x )

#define CONTEXT_STR ANSI_ITALIC "[" __FILE__ ":" STRINGIFY(__LINE__) "] " ANSI_NORMAL

void log_set_verbose_level( unsigned int mask );

void _log_error( const char *format, ... );
void log_base( unsigned int priority, const char *format, ... );

// error
#define log_error( FORMAT, ... ) _log_error( ERROR_PREFIX CONTEXT_STR FORMAT __VA_OPT__(,) __VA_ARGS__ )

#define log_warn( FORMAT, ... )  log_base( LOG_ALLOW_WARN,  WARNING_PREFIX CONTEXT_STR FORMAT __VA_OPT__(,) __VA_ARGS__ )
#define log_info( FORMAT, ... )  log_base( LOG_ALLOW_INFO,  INFO_PREFIX    CONTEXT_STR FORMAT __VA_OPT__(,) __VA_ARGS__ )
#define log_debug( FORMAT, ... ) log_base( LOG_ALLOW_DEBUG, DEBUG_PREFIX   CONTEXT_STR FORMAT __VA_OPT__(,) __VA_ARGS__ )

// bypass functions
#define log_warn_bypass( FORMAT, ... )  log_base( LOG_WILDCARD, WARNING_PREFIX CONTEXT_STR FORMAT __VA_OPT__(,) __VA_ARGS__ )
#define log_info_bypass( FORMAT, ... )  log_base( LOG_WILDCARD, INFO_PREFIX    CONTEXT_STR FORMAT __VA_OPT__(,) __VA_ARGS__ )
#define log_debug_bypass( FORMAT, ... ) log_base( LOG_WILDCARD, DEBUG_PREFIX   CONTEXT_STR FORMAT __VA_OPT__(,) __VA_ARGS__ )
