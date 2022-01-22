#pragma once

#include "autoclickers.h"
#include "os_specific.h"

#ifdef OS_IS_UNIX // linux

#include <pthread.h> // for async stuff

class MimicMouseButFaster {
  protected:
    AsyncAutoClicker *clickers[2];

    static constexpr char *pDevice = (char *)"/dev/input/mice";
    static const int LISTEN_MASK = KeyPressMask | StructureNotifyMask | FocusChangeMask;
    int fd;
    
    Display *display;
    Window curFocus, root;
    int revert;
    
    pthread_t worker_thread; // listens to mouse state
    pthread_t listen_thread; // listens to hotkey state

    // shared variables
    enum ListenerStatus { EXIT, NORMAL } status_flag;
    pthread_mutex_t status_flag_mtx;
    
    int is_active;
    pthread_mutex_t is_active_mtx;
    
    // ugly solution (BUT IT FCKIN WORKS)
    // https://stackoverflow.com/questions/16185286/how-to-detect-mouse-click-events-in-all-applications-in-x11
    static void *worker( void *args );
    
    static void *listen( void *args );
    
    static int catcher( Display *display, XErrorEvent *err );
  
  public:
    MimicMouseButFaster( double cps );

    ~MimicMouseButFaster();
};

#else // windows

class MimicMouseButFaster {
  protected:
    AsyncAutoClicker *clickers[2];

    thread_t worker_thread; // listens to mouse state

    // shared variables
    enum ListenerStatus { EXIT, NORMAL } status_flag;
    mutex_t status_flag_mtx;
    
    static thread_ret_type THREAD_FUNC_ATTR worker( LPVOID args );

  public:
    MimicMouseButFaster( double cps );

    ~MimicMouseButFaster();
};

#endif
