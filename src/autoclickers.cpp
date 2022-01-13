#include "autoclickers.h"

// implementation of SyncAutoClicker
SyncAutoClicker::SyncAutoClicker( unsigned int button, double cps = DEFAULT_CPS ) : btn( button ){
  setCPS( cps );

  display = XOpenDisplay( NULL );
  
  if( !display )
    log_error( "Can't open display!\n" );
  
  srand( time( NULL ) );
}

SyncAutoClicker::~SyncAutoClicker(){
  XCloseDisplay( display );
}

void SyncAutoClicker::setCPS( double cps ){
  int DELAY;

  CPS = cps;
  DELAY = 1'000'000 / CPS;
  MIN_DELAY = DELAY * REL_MIN;
  MAX_DELAY = DELAY * REL_MAX;
}

void SyncAutoClicker::click(){
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
    log_warn( "Error sending ButtonPress event!\n" );
  XFlush( display );
  usleep( RELEASE_WAIT );

  // Release -- reusing the same event
  event.type = ButtonRelease;
  if( !XSendEvent( display, PointerWindow, True, ButtonReleaseMask, &event ) )
    log_warn( "Error sending ButtonRelease event!\n" );
  XFlush( display );
  usleep( RELEASE_WAIT );
}

void SyncAutoClicker::autoclick( int num ){
  while( num-- ){
    click();
    usleep( MIN_DELAY + rand() % (MAX_DELAY - MIN_DELAY) );
  }
}

// implementation of AsyncAutoClicker
AsyncAutoClicker::AsyncAutoClicker( unsigned int button, double cps = DEFAULT_CPS ) : SyncAutoClicker( button, cps ){
  char thread_name[100] = "clicker-";
  int i = -1;
  
  pthread_mutex_init( &DELAY_mtx, NULL );

  status_flag = WAITING;
  pthread_mutex_init( &status_flag_mtx, NULL );
  
  while( thread_name[++i] );

  if( button == Button1 )
    strcpy( thread_name + i, "left" );
  else
    strcpy( thread_name + i, "right" );

  pthread_create( &worker_thread, NULL, worker, (void *)this );
  pthread_setname_np( worker_thread, thread_name );
}

AsyncAutoClicker::~AsyncAutoClicker(){
  // set kill flag
  pthread_mutex_lock( &status_flag_mtx );
  status_flag = EXIT;
  pthread_mutex_unlock( &status_flag_mtx );
  
  
  // wait for thread to finish
  pthread_join( worker_thread, NULL );

  pthread_mutex_destroy( &status_flag_mtx );
  pthread_mutex_destroy( &DELAY_mtx );
}

void *AsyncAutoClicker::worker( void *args ){
  AsyncAutoClicker *obj = (AsyncAutoClicker *)args;
  
  log_info( "autoclicker initialized\n" );
  
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

void AsyncAutoClicker::setCPS( double cps ){
  int DELAY;

  pthread_mutex_lock( &DELAY_mtx );

  CPS = cps;
  DELAY = 1'000'000 / CPS;
  MIN_DELAY = DELAY * REL_MIN;
  MAX_DELAY = DELAY * REL_MAX;

  pthread_mutex_unlock( &DELAY_mtx );
}

void AsyncAutoClicker::start(){
  pthread_mutex_lock( &status_flag_mtx );
  status_flag = CLICKING;
  pthread_mutex_unlock( &status_flag_mtx );
}

void AsyncAutoClicker::stop(){
  pthread_mutex_lock( &status_flag_mtx );
  status_flag = WAITING;
  pthread_mutex_unlock( &status_flag_mtx );
}

AsyncAutoClicker::ClickerStatus AsyncAutoClicker::getStatus(){
  ClickerStatus retval;
  
  pthread_mutex_lock( &status_flag_mtx );
  retval = status_flag;
  pthread_mutex_unlock( &status_flag_mtx );
  
  return retval;
}
