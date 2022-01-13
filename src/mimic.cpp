#include "mimic.h"

void *MimicMouseButFaster::worker( void *args ){
  MimicMouseButFaster *obj = (MimicMouseButFaster *)args;
  int bytes, left = 0, right = 0, newleft, newright;
  unsigned char data[3];
  
  log_info( "mouse listener initialized\n" );

  pthread_mutex_lock( &obj->status_flag_mtx );
  while( obj->status_flag != EXIT ){
    pthread_mutex_unlock( &obj->status_flag_mtx );

    bytes = read( obj->fd, data, sizeof( data ) );
    
    pthread_mutex_lock( &obj->is_active_mtx );
    if( bytes > 0 && obj->is_active ){
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
    pthread_mutex_unlock( &obj->is_active_mtx );

    pthread_mutex_lock( &obj->status_flag_mtx );
  }

  return NULL;
}

void *MimicMouseButFaster::listen( void *args ){
  MimicMouseButFaster *obj = (MimicMouseButFaster *)args;
  XEvent event;
  unsigned int hotkey = XKeysymToKeycode( obj->display, XK_Caps_Lock );
  
  log_info( "hotkey listener initialized\n" );
  
  pthread_mutex_lock( &obj->status_flag_mtx );
  while( obj->status_flag != EXIT ){
    pthread_mutex_unlock( &obj->status_flag_mtx );
    
    XNextEvent( obj->display, &event );
    
    //info( "event\n" );

    pthread_mutex_lock( &obj->is_active_mtx );
    switch( event.type ){
      case DestroyNotify:
        //warn( "window destroy!\n" );
        XGetInputFocus( obj->display, &obj->curFocus, &obj->revert );
        XSelectInput( obj->display, obj->curFocus, LISTEN_MASK );
        break;
      case FocusOut:
        if( obj->curFocus != obj->root )
          XSelectInput( obj->display, obj->curFocus, 0 );
        XGetInputFocus( obj->display, &obj->curFocus, &obj->revert );
        if( obj->curFocus == PointerRoot )
          obj->curFocus = obj->root;
        XSelectInput( obj->display, obj->curFocus, LISTEN_MASK );

        break;
      case KeyPress:
        if( event.xkey.keycode == hotkey ){
          obj->is_active ^= 1;
          log_debug( "autoclick turned %s\n", obj->is_active ? "on" : "off");
        }
        break;
      /*default:
        //log_warn( "unknown event: searching in lookup table...\n" );
        int i = -1;
        while( (++i) < 33 && XEvent_types[i] != event.type );
        
        if( i >= 33 )
          log_warn( "uncaught event %d\n", event.type );
        else
          log_warn( "event is at index %d\n", i );
        break;*/
    }
    pthread_mutex_unlock( &obj->is_active_mtx );

    pthread_mutex_lock( &obj->status_flag_mtx );
  }

  return NULL;
}

MimicMouseButFaster::MimicMouseButFaster( double cps = DEFAULT_CPS ){
  clickers[0] = new AsyncAutoClicker( Button1, cps );
  clickers[1] = new AsyncAutoClicker( Button3, cps );

  fd = open( pDevice, O_RDWR );
  
  if( fd == -1 )
    log_error( "Coudn't open %s\n", pDevice );
  
  status_flag = NORMAL;
  pthread_mutex_init( &status_flag_mtx, NULL );
  
  is_active = 0;
  pthread_mutex_init( &is_active_mtx, NULL );

  display = XOpenDisplay( NULL );
  if( !display )
    log_error( "Can't open display!\n" );

  root = DefaultRootWindow( display );

  XGetInputFocus( display, &curFocus, &revert );
  XSelectInput( display, curFocus, LISTEN_MASK );

  pthread_create( &worker_thread, NULL, worker, (void *)this );
  pthread_setname_np( worker_thread, "worker" );

  pthread_create( &listen_thread, NULL, listen, (void *)this );
  pthread_setname_np( listen_thread, "listen" );
}

MimicMouseButFaster::~MimicMouseButFaster(){
  pthread_mutex_lock( &status_flag_mtx );
  status_flag = EXIT;
  pthread_mutex_unlock( &status_flag_mtx );
  
  pthread_join( worker_thread, NULL );// wait for thread to exit
  pthread_join( listen_thread, NULL );// wait for thread to exit

  pthread_mutex_destroy( &status_flag_mtx );
  pthread_mutex_destroy( &is_active_mtx );
  
  close( fd );
  
  XCloseDisplay( display );

  delete clickers[0];
  delete clickers[1];
}

