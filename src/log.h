#define LOG_ALLOW_DEBUG 1
#define LOG_ALLOW_INFO  2
#define LOG_ALLOW_WARN  4

void log_set_verbose_level( int mask );

void log_error( const char *format, ... );
void log_warn( const char *format, ... );
void log_info( const char *format, ... );
void log_debug( const char *format, ... );

void log_warn_bypass( const char *format, ... );
void log_info_bypass( const char *format, ... );
void log_debug_bypass( const char *format, ... );
