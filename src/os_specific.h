#ifdef _WIN32
#define OS_IS_WINDOWS
#endif

#ifdef __unix__
#define OS_IS_UNIX
#endif

#ifdef __APPLE__
#define OS_IS_MACOS
#endif

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

#else // windows

#define create_thread( thread, function, args ) thread = CreateThread( NULL, 0, (LPTHREAD_START_ROUTINE) function, (LPVOID)args, 0, NULL )
#define create_mutex( mutex ) !(mutex = CreateMutex( NULL, FALSE, NULL ))
#define lock_mutex( mutex ) WaitForSingleObject( mutex, INFINITE )
#define unlock_mutex( mutex ) ReleaseMutex( mutex )
#define destroy_mutex( mutex ) CloseHandle( mutex )
#define join_thread( thread ) WaitForSingleObject( thread, INFINITE )
#define thread_ret_type DWORD
#define THREAD_FUNC_ATTR WINAPI

// sleep function
// https://stackoverflow.com/questions/5801813/c-usleep-is-obsolete-workarounds-for-windows-mingw
void usleep( __int64 usec );

#endif
