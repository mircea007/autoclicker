#include <stdlib.h>  // rand()
#include <unistd.h>  // usleep()
#include <time.h>    // srand( time( NULL ) )
#include <string.h>  // memset()

#include "autoclickers.h"
#include "osdetect.h"

#ifdef OS_IS_UNIX // linux

#define create_thread( thread, function, args ) pthread_create( &thread, NULL, function, (void *)args )
#define create_mutex( mutex ) pthread_mutex_init( &mutex, NULL )
#define lock_mutex( mutex ) pthread_mutex_lock( &mutex )
#define unlock_mutex( mutex ) pthread_mutex_unlock( &mutex )
#define destroy_mutex( mutex ) pthread_mutex_destroy( &mutex )
#define join_thread( thread ) pthread_join( thread, NULL )
#define LPVOID void *
#define thread_ret_type void *
#define THREAD_FUNC_ATTR

// implementation of SyncAutoClicker
SyncAutoClicker::SyncAutoClicker( int button, double cps = DEFAULT_CPS ){
  unsigned int buttons[3] = { Button1, Button2, Button3 };
  btn = buttons[button];
  
  setCPS( cps );

  display = XOpenDisplay( NULL );
  
  if( !display )
    log_error( "Can't open display!\n" );
  
  srand( time( NULL ) );
}

SyncAutoClicker::~SyncAutoClicker(){
  XCloseDisplay( display );
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

#else // windows -- surprisingly little code

#define create_thread( thread, function, args ) thread = CreateThread( NULL, 0, (LPTHREAD_START_ROUTINE) function, (LPVOID)args, 0, NULL )
#define create_mutex( mutex ) !(mutex = CreateMutex( NULL, FALSE, NULL ))
#define lock_mutex( mutex ) WaitForSingleObject( mutex, INFINITE )
#define unlock_mutex( mutex ) ReleaseMutex( mutex )
#define destroy_mutex( mutex ) CloseHandle( mutex )
#define join_thread( thread ) WaitForSingleObject( thread, INFINITE )
#define thread_ret_type DWORD
#define THREAD_FUNC_ATTR WINAPI

// implementation of SyncAutoClicker
SyncAutoClicker::SyncAutoClicker( unsigned int button, double cps = DEFAULT_CPS ) : btn( button ){
  setCPS( cps );

  srand( time( NULL ) );
}

SyncAutoClicker::~SyncAutoClicker(){}

SyncAutoClicker::click(){
  INPUT Inputs[2];

  Inputs[0].type = INPUT_MOUSE;
  Inputs[0].mi.dwFlags = buttons[btn][0];

  Inputs[1].type = INPUT_MOUSE;
  Inputs[1].mi.dwFlags = buttons[btn][1];

  SendInput( 2, Inputs, sizeof( INPUT ) );
}

// https://stackoverflow.com/questions/5801813/c-usleep-is-obsolete-workarounds-for-windows-mingw
void usleep( __int64 usec ){
  HANDLE timer; 
  LARGE_INTEGER ft;

  ft.QuadPart = -(10 * usec); // Convert to 100 nanosecond interval, negative value indicates relative time

  timer = CreateWaitableTimer( NULL, TRUE, NULL ); 
  SetWaitableTimer( timer, &ft, 0, NULL, NULL, 0 ); 
  WaitForSingleObject( timer, INFINITE ); 
  CloseHandle( timer ); 
}

#endif

// os-neutral code

// SyncAutoClicker
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

// AsyncAutoClicker
AsyncAutoClicker::AsyncAutoClicker( int button, double cps = DEFAULT_CPS ) : SyncAutoClicker( button, cps ){
  if( create_mutex( DELAY_mtx ) )
    log_error( "Unable to create DELAY mutex\n" );

  status_flag = WAITING;

  if( create_mutex( status_flag_mtx ) )
    log_error( "Unable to create status_flag mutex\n" );
  
  create_thread( worker_thread, worker, this );
}

AsyncAutoClicker::~AsyncAutoClicker(){
  // set kill flag
  lock_mutex( status_flag_mtx );
  status_flag = EXIT;
  unlock_mutex( status_flag_mtx );
  
  join_thread( worker_thread ); // wait for thread to finish

  destroy_mutex( status_flag_mtx );
  destroy_mutex( DELAY_mtx );
}

void AsyncAutoClicker::setCPS( double cps ){
  int DELAY;

  lock_mutex( DELAY_mtx );

  CPS = cps;
  DELAY = 1'000'000 / CPS;
  MIN_DELAY = DELAY * REL_MIN;
  MAX_DELAY = DELAY * REL_MAX;

  unlock_mutex( DELAY_mtx );
}

void AsyncAutoClicker::start(){
  lock_mutex( status_flag_mtx );
  status_flag = CLICKING;
  unlock_mutex( status_flag_mtx );
}

void AsyncAutoClicker::stop(){
  lock_mutex( status_flag_mtx );
  status_flag = WAITING;
  unlock_mutex( status_flag_mtx );
}

AsyncAutoClicker::ClickerStatus AsyncAutoClicker::getStatus(){
  ClickerStatus retval;
  
  lock_mutex( status_flag_mtx );
  retval = status_flag;
  unlock_mutex( status_flag_mtx );
  
  return retval;
}

thread_ret_type THREAD_FUNC_ATTR AsyncAutoClicker::worker( LPVOID args ){
  AsyncAutoClicker *obj = (AsyncAutoClicker *)args;
  
  log_info( "autoclicker initialized\n" );
  
  lock_mutex( obj->status_flag_mtx );
  while( obj->status_flag != EXIT ){
    if( obj->status_flag == CLICKING )
      obj->click();

    unlock_mutex( obj->status_flag_mtx );
    lock_mutex( obj->DELAY_mtx );

    usleep( obj->MIN_DELAY + rand() % (obj->MAX_DELAY - obj->MIN_DELAY) );

    unlock_mutex( obj->DELAY_mtx );
    lock_mutex( obj->status_flag_mtx );
  }

  return (thread_ret_type)0;
}
