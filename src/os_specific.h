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

#define LPVOID void *
#define thread_ret_type void *
#define THREAD_FUNC_ATTR
#define thread_t pthread_t
#define mutex_t pthread_mutex_t

#define create_thread( thread, function, args ) pthread_create( &thread, NULL, function, (void *)args )
#define create_mutex( mutex ) pthread_mutex_init( &mutex, NULL )
#define lock_mutex( mutex ) pthread_mutex_lock( &mutex )
#define unlock_mutex( mutex ) pthread_mutex_unlock( &mutex )
#define destroy_mutex( mutex ) pthread_mutex_destroy( &mutex )
#define join_thread( thread ) pthread_join( thread, NULL )

#else // windows

#define thread_ret_type DWORD
#define THREAD_FUNC_ATTR WINAPI
#define thread_t HANDLE
#define mutex_t HANDLE

#define create_thread( thread, function, args ) thread = CreateThread( NULL, 0, (LPTHREAD_START_ROUTINE) function, (LPVOID)args, 0, NULL )
#define create_mutex( mutex ) !(mutex = CreateMutex( NULL, FALSE, NULL ))
#define lock_mutex( mutex ) WaitForSingleObject( mutex, INFINITE )
#define unlock_mutex( mutex ) ReleaseMutex( mutex )
#define destroy_mutex( mutex ) CloseHandle( mutex )
#define join_thread( thread ) WaitForSingleObject( thread, INFINITE )

#endif
