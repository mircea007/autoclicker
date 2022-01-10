#include <stdio.h>   // duh
#include <stdlib.h>  // exit(), rand()
#include <string.h>  // memset()
#include <unistd.h>  // usleep()
#include <time.h>    // srand( time( NULL ) )
#include <fcntl.h>   // to read form mouse device file
#include <signal.h>  // Ctrl+C
#include <stdarg.h>  // for printf wrapper functions

// X11
#include <X11/X.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>

#include <pthread.h> // for async stuff

const double DEFAULT_CPS = 20.0;

// logging

#define ANSI_BOLD "\e[1m"
#define ANSI_NORMAL "\e[0m"
#define ANSI_RED "\e[38;5;9m"
#define ANSI_YELLOW "\e[38;5;11m"
#define ANSI_GREEN "\e[38;5;10m"

#define ERROR_PREFIX   ANSI_RED    "(!)" ANSI_NORMAL " ERROR ---> "
#define WARNING_PREFIX ANSI_YELLOW "(!)" ANSI_NORMAL " WARNING -> "
#define INFO_PREFIX    ANSI_GREEN  "(!)" ANSI_NORMAL " INFO ----> "

static inline void error( const char *format, ... ){
  va_list ap;
  
  fputs( ERROR_PREFIX, stderr );
  va_start( ap, format );
  vfprintf( stderr, format, ap );
  va_end( ap );

  exit( 1 );
}

static inline void warn( const char *format, ... ){
  va_list ap;
  
  fputs( WARNING_PREFIX, stderr );
  va_start( ap, format );
  vfprintf( stderr, format, ap );
  va_end( ap );
}

static inline void info( const char *format, ... ){
  va_list ap;
  
  fputs( INFO_PREFIX, stderr );
  va_start( ap, format );
  vfprintf( stderr, format, ap );
  va_end( ap );
}

// https://gist.github.com/pioz/726474
class SyncAutoClicker {
  protected:
    double CPS;
    int MIN_DELAY; // delays are in microseconds
    int MAX_DELAY;
    static const int RELEASE_WAIT = 100;
    static constexpr double REL_MIN = 0.5;
    static constexpr double REL_MAX = 1.5;
    Display *display;
    unsigned int btn;

    inline void click(){
      // create event
      XEvent event;
      
      memset( &event, 0, sizeof( event ) );
      event.xbutton.button      = btn;
      event.xbutton.same_screen = True;
      event.xbutton.subwindow   = DefaultRootWindow( display );
      
      while( event.xbutton.subwindow ){
        event.xbutton.window = event.xbutton.subwindow;
        XQueryPointer(
          display,
          event.xbutton.window,
          &event.xbutton.root,
          &event.xbutton.subwindow,
          &event.xbutton.x_root,
          &event.xbutton.y_root,
          &event.xbutton.x,
          &event.xbutton.y,
          &event.xbutton.state
        );
      }

      // Press
      event.type = ButtonPress;
      if( !XSendEvent( display, PointerWindow, True, ButtonPressMask, &event ) )
        fprintf( stderr, "Error to send the event!\n" );
      XFlush( display );
      usleep( RELEASE_WAIT );

      // Release -- reusing the same event
      event.type = ButtonRelease;
      if( !XSendEvent( display, PointerWindow, True, ButtonReleaseMask, &event ) )
        fprintf( stderr, "Error to send the event!\n" );
      XFlush( display );
      usleep( RELEASE_WAIT );
    }

  public:
    inline SyncAutoClicker( unsigned int button, double cps = DEFAULT_CPS ) : btn( button ){
      setCPS( cps );

      display = XOpenDisplay( NULL );
      
      if( !display )
        error( "Can't open display!\n" );
      
      srand( time( NULL ) );
    }
    
    inline void setCPS( double cps ){
      int DELAY;

      CPS = cps;
      DELAY = 1'000'000 / CPS;
      MIN_DELAY = DELAY * REL_MIN;
      MAX_DELAY = DELAY * REL_MAX;
    }

    inline ~SyncAutoClicker(){
      XCloseDisplay( display );
    }

    void autoclick( int num ){
      while( num-- ){
        click();
        usleep( MIN_DELAY + rand() % (MAX_DELAY - MIN_DELAY) );
      }
    }
};

class AsyncAutoClicker : public SyncAutoClicker {
  protected:
    pthread_t worker_thread;

    // shared variables
    enum ClickerStatus { EXIT, CLICKING, WAITING } status_flag;
    pthread_mutex_t status_flag_mtx;

    //int MIN_DELAY, MAX_DELAY; // <--- already declared in parrent class
    pthread_mutex_t DELAY_mtx;
    
    static void *worker( void *args ){
      AsyncAutoClicker *obj = (AsyncAutoClicker *)args;
      
      pthread_mutex_lock( &obj->status_flag_mtx );
      while( obj->status_flag != EXIT ){
        if( obj->status_flag == CLICKING )
          obj->click();

        pthread_mutex_unlock( &obj->status_flag_mtx );
        pthread_mutex_lock( &obj->DELAY_mtx );

        usleep( obj->MIN_DELAY + rand() % (obj->MAX_DELAY - obj->MIN_DELAY) );

        pthread_mutex_unlock( &obj->DELAY_mtx );
        pthread_mutex_lock( &obj->status_flag_mtx );
      }

      return NULL;
    }

  public:
    inline AsyncAutoClicker( unsigned int button, double cps = DEFAULT_CPS ) : SyncAutoClicker( button, cps ){
      status_flag = WAITING;
      pthread_mutex_init( &status_flag_mtx, NULL );
      
      pthread_mutex_init( &DELAY_mtx, NULL );

      pthread_create( &worker_thread, NULL, worker, (void *)this );
    }
    
    inline void setCPS( double cps ){
      int DELAY;

      pthread_mutex_lock( &DELAY_mtx );

      CPS = cps;
      DELAY = 1'000'000 / CPS;
      MIN_DELAY = DELAY * REL_MIN;
      MAX_DELAY = DELAY * REL_MAX;

      pthread_mutex_unlock( &DELAY_mtx );
    }

    inline ~AsyncAutoClicker(){
      // set kill flag
      pthread_mutex_lock( &status_flag_mtx );
      status_flag = EXIT;
      pthread_mutex_unlock( &status_flag_mtx );
      
      
      // wait for thread to finish
      pthread_join( worker_thread, NULL );

      pthread_mutex_destroy( &status_flag_mtx );
      pthread_mutex_destroy( &DELAY_mtx );
    }

    void start(){
      pthread_mutex_lock( &status_flag_mtx );
      status_flag = CLICKING;
      pthread_mutex_unlock( &status_flag_mtx );
    }

    void stop(){
      pthread_mutex_lock( &status_flag_mtx );
      status_flag = WAITING;
      pthread_mutex_unlock( &status_flag_mtx );
    }
};

// ugly solution (BUT IT FCKIN WORKS)
// https://stackoverflow.com/questions/16185286/how-to-detect-mouse-click-events-in-all-applications-in-x11
class MimicMouseButFaster {
  protected:
    AsyncAutoClicker *clickers[2];

    static constexpr char *pDevice = (char *)"/dev/input/mice";
    int fd;
    
    pthread_t worker_thread;

    // shared variables
    enum ListenerStatus { EXIT, NORMAL } status_flag;
    pthread_mutex_t status_flag_mtx;
    
    static void *worker( void *args ){
      MimicMouseButFaster *obj = (MimicMouseButFaster *)args;
      int bytes, left = 0, right = 0, newleft, newright;
      unsigned char data[3];
      
      pthread_mutex_lock( &obj->status_flag_mtx );
      while( obj->status_flag != EXIT ){
        pthread_mutex_unlock( &obj->status_flag_mtx );
  
        bytes = read( obj->fd, data, sizeof( data ) );
        
        if( bytes > 0 ){
          newleft = !!(data[0] & 0x1);
          newright = !!(data[0] & 0x2);
          
          switch( left + 2 * newleft ){
            case 2:
              obj->clickers[0]->start();
              break;
            case 1:
              obj->clickers[0]->stop();
              break;
          }
          
          switch( right + 2 * newright ){
            case 2:
              obj->clickers[1]->start();
              break;
            case 1:
              obj->clickers[1]->stop();
              break;
          }
          
          left = newleft;
          right = newright;
        }

        pthread_mutex_lock( &obj->status_flag_mtx );
      }

      return NULL;
    }
  
  public:
    MimicMouseButFaster( double cps = DEFAULT_CPS ){
      clickers[0] = new AsyncAutoClicker( Button1, cps );
      clickers[1] = new AsyncAutoClicker( Button3, cps );

      fd = open(pDevice, O_RDWR);
      
      if( fd == -1 )
        error( "Coudn't open %s\n", pDevice );
      
      status_flag = NORMAL;
      pthread_mutex_init( &status_flag_mtx, NULL );
      
      pthread_create( &worker_thread, NULL, worker, (void *)this );
    }
    
    ~MimicMouseButFaster(){
      pthread_mutex_lock( &status_flag_mtx );
      status_flag = EXIT;
      pthread_mutex_unlock( &status_flag_mtx );
      
      pthread_join( worker_thread, NULL );// wait for thread to exit

      pthread_mutex_destroy( &status_flag_mtx );
      
      close( fd );

      delete clickers[0];
      delete clickers[1];
    }
};

MimicMouseButFaster *copy;

void kill_handle( int s ){
  fputc( '\r', stderr );// hide ^C when run in terminal
  info( "Quiting...\n" );
  
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

  if( signal( SIGINT, kill_handle ) == SIG_ERR )
    error( "Unable to set SIGINT handler\n" );
  
  if( argc > 1 )
    if( sscanf( argv[1], "%lf", &cps ) != 1 )
      error( "Clickrate (CPS) value must be a number\n" );
  
  info( "Clickrate (CPS) is set to %lf\n", cps );
  
  copy = new MimicMouseButFaster( cps );

  while( 1 );// run while program is not interupted
  
  return 0;
}