#include "Thread.h"

//
// Usage: SetThreadName (-1, "MainThread");
//
#include <windows.h>

const DWORD kThreadStopWaitTimeout = 5000;

Thread::Thread(const char* thread_name) :
  thread_name_(thread_name), 
  thread_(NULL) {
}

Thread::~Thread() {
}

void Thread::SetThreadName(const char* thread_name) {
  const DWORD MS_VC_EXCEPTION=0x406D1388;

#pragma pack(push,8)
  typedef struct tagTHREADNAME_INFO
  {
    DWORD dwType; // Must be 0x1000.
    LPCSTR szName; // Pointer to name (in user addr space).
    DWORD dwThreadID; // Thread ID (-1=caller thread).
    DWORD dwFlags; // Reserved for future use, must be zero.
  } THREADNAME_INFO;
#pragma pack(pop)

  THREADNAME_INFO info;
  info.dwType = 0x1000;
  info.szName = thread_name;
  info.dwThreadID = -1;
  info.dwFlags = 0;

  __try {
    RaiseException( MS_VC_EXCEPTION, 0, sizeof(info)/sizeof(ULONG_PTR), (ULONG_PTR*)&info );
  } __except(EXCEPTION_EXECUTE_HANDLER) {
  }
}

bool Thread::Wait(DWORD dwMilliseconds_ /*= INFINITE*/) {
  if (NULL == thread_) {
    return false;
  }

  return (WAIT_OBJECT_0 == WaitForSingleObject(thread_, dwMilliseconds_));
}


// virtual 
bool Thread::Start() {
  if (NULL != thread_) {
    return false;
  }

  stopping_ = false;

  thread_ = 
    CreateThread(NULL, 
                 0,  
                 ThreadProc, 
                 (LPVOID)this, 
                 NULL,
                 NULL);

  return (NULL != thread_);
}

// virtual
bool Thread::Stop() {
  stopping_ = true;
  return true;
}

//static 
DWORD WINAPI Thread::ThreadProc(IN LPVOID parameter) {
  if (NULL == parameter) {
    return 0;
  }

  Thread* p_thread = (Thread*)parameter;
  p_thread->SetThreadName(p_thread->thread_name_.c_str());
  p_thread->ThreadFunction();

  p_thread->thread_ = NULL;
  return 0;
}
