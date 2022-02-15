#include <stdlib.h>  // rand()
#include <unistd.h>  // usleep()
#include <time.h>    // srand( time( NULL ) )
#include <string.h>  // memset()

#include "autoclickers.h"

#ifdef OS_IS_UNIX // linux

// event type lookup table
const int XEvent_types[33] = {
  KeyPress,       KeyRelease,       ButtonPress,     ButtonRelease,    MotionNotify,
  EnterNotify,    LeaveNotify,      FocusIn,         FocusOut,         KeymapNotify,
  Expose,         GraphicsExpose,   NoExpose,        CirculateRequest, ConfigureRequest,
  MapRequest,     ResizeRequest,    CirculateNotify, ConfigureNotify,  CreateNotify,
  DestroyNotify,  GravityNotify,    MapNotify,       MappingNotify,    ReparentNotify,
  UnmapNotify,    VisibilityNotify, ColormapNotify,  ClientMessage,    PropertyNotify,
  SelectionClear, SelectionNotify,  SelectionRequest
};

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

// implementation of SyncAutoClicker
SyncAutoClicker::SyncAutoClicker( int button, double cps = DEFAULT_CPS ) : btn( button ){
  setCPS( cps );

  srand( time( NULL ) );
}

SyncAutoClicker::~SyncAutoClicker(){}

void SyncAutoClicker::click(){
  INPUT Inputs[2];

  Inputs[0].type = INPUT_MOUSE;
  Inputs[0].mi.dwFlags = buttons[btn][0];

  Inputs[1].type = INPUT_MOUSE;
  Inputs[1].mi.dwFlags = buttons[btn][1];

  SendInput( 2, Inputs, sizeof( INPUT ) );
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
    usleep( MIN_DELAY + rand() % (MAX_DELAY - MIN_DELAY + 1) );
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

    usleep( obj->MIN_DELAY + rand() % (obj->MAX_DELAY - obj->MIN_DELAY + 1) );

    unlock_mutex( obj->DELAY_mtx );
    lock_mutex( obj->status_flag_mtx );
  }

  return (thread_ret_type)0;
}
