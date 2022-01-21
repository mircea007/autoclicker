#include <stdlib.h>  // rand()
#include <unistd.h>  // usleep()
#include <time.h>    // srand( time( NULL ) )
#include <string.h>  // memset()

#include "autoclickers.h"
#include "osdetect.h"

// will get rid of duplicated code with os-specific macros for threading

#ifdef OS_IS_UNIX // linux

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
  if( pthread_mutex_init( &status_flag_mtx, NULL ) )
    log_error( "Unable to create status_flag mutex\n" );
  
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

#else // windows -- surprisingly little code

// implementation of SyncAutoClicker
SyncAutoClicker::SyncAutoClicker( unsigned int button, double cps = DEFAULT_CPS ) : btn( button ){
  setCPS( cps );

  srand( time( NULL ) );
}

SyncAutoClicker::~SyncAutoClicker(){}

void SyncAutoClicker::setCPS( double cps ){
  int DELAY;

  CPS = cps;
  DELAY = 1'000'000 / CPS;
  MIN_DELAY = DELAY * REL_MIN;
  MAX_DELAY = DELAY * REL_MAX;
}

void SyncAutoClicker::autoclick( int num ){
  while( num-- ){
    click();
    usleep( MIN_DELAY + rand() % (MAX_DELAY - MIN_DELAY) );
  }
}

SyncAutoClicker::click(){
  INPUT Inputs[2];

  Inputs[0].type = INPUT_MOUSE;
  Inputs[0].mi.dwFlags = buttons[btn][0];

  Inputs[1].type = INPUT_MOUSE;
  Inputs[1].mi.dwFlags = buttons[btn][1];

  SendInput( 2, Inputs, sizeof( INPUT ) );
}

// implementation of AsyncAutoClicker
AsyncAutoClicker::AsyncAutoClicker( unsigned int button, double cps = DEFAULT_CPS ) : SyncAutoClicker( button, cps ){
  DELAY_mtx = CreateMutex( NULL, FALSE, NULL );
  if( !DELAY_mtx )
    log_error( "Unable to create DELAY mutex\n" );

  status_flag = WAITING;
  
  status_flag_mtx = CreateMutex( NULL, FALSE, NULL );
  if( !status_flag_mtx )
    log_error( "Unable to create status_flag mutex\n" );

  worker_thread = CreateThread( NULL, 0, (LPTHREAD_START_ROUTINE) worker, (LPVOID)this, 0, &ThreadID );
}

AsyncAutoClicker::~AsyncAutoClicker(){
  // set kill flag
  WaitForSingleObject( status_flag_mtx, INFINITE );
  status_flag = EXIT;
  ReleaseMutex( status_flag_mtx );

  // wait for thread to finish
  WaitForSingleObject( worker_thread, INFINITE );

  CloseHandle( status_flag_mtx );
  CloseHandle( DELAY_mtx );
}

DWORD WINAPI AsyncAutoClicker::worker( LPVOID args ){
  AsyncAutoClicker *obj = (AsyncAutoClicker *)args;
  
  log_info( "autoclicker initialized\n" );
  
  WaitForSingleObject( obj->status_flag_mtx, INFINITE );
  while( obj->status_flag != EXIT ){
    if( obj->status_flag == CLICKING )
      obj->click();

    ReleaseMutex( obj->status_flag_mtx );
    WaitForSingleObject( obj->DELAY_mtx, INFINITE );

    usleep( obj->MIN_DELAY + rand() % (obj->MAX_DELAY - obj->MIN_DELAY) );

    ReleaseMutex( obj->DELAY_mtx );
    WaitForSingleObject( obj->status_flag_mtx, INFINITE );
  }

  return 0;
}

void AsyncAutoClicker::setCPS( double cps ){
  int DELAY;

  WaitForSingleObject( DELAY_mtx, INFINITE );

  CPS = cps;
  DELAY = 1'000'000 / CPS;
  MIN_DELAY = DELAY * REL_MIN;
  MAX_DELAY = DELAY * REL_MAX;

  ReleaseMutex( DELAY_mtx );
}

void AsyncAutoClicker::start(){
  WaitForSingleObject( status_flag_mtx, INFINITE );
  status_flag = CLICKING;
  ReleaseMutex( status_flag_mtx );
}

void AsyncAutoClicker::stop(){
  WaitForSingleObject( status_flag_mtx, INFINITE );
  status_flag = WAITING;
  ReleaseMutex( status_flag_mtx );
}

AsyncAutoClicker::ClickerStatus AsyncAutoClicker::getStatus(){
  ClickerStatus retval;
  
  WaitForSingleObject( status_flag_mtx, INFINITE );
  retval = status_flag;
  ReleaseMutex( status_flag_mtx );
  
  return retval;
}

#endif
