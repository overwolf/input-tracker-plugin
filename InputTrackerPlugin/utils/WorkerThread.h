/*
  Input Tracker Plugin
  Copyright (c) 2015 Overwolf Ltd.
*/
#ifndef UTILS_WORKER_THREAD_H_
#define UTILS_WORKER_THREAD_H_

#include <queue>
#include <functional>
#include <Windows.h>
#include "CriticalSectionLock.h"

namespace utils {

class WorkerThread {
public:
  typedef std::function<void()> Task;

  WorkerThread();
  virtual ~WorkerThread();

public:
  bool Start();
  bool Stop();
  bool PostTask(Task task_func);
  DWORD GetThreadId() {return threadId_;}
private:
  bool CreateEvents();
  void DestroyEvents();
  void ClearQueue();
  void HandleNewTaskEvent();

  static DWORD WINAPI ThreadProc(IN LPVOID lpParameter_);

private:
  enum ThreadEvents {
    EVENT_STOP = 0,
    EVENT_NEW_TASK,
    EVENTS_COUNT
  };

  // contains two events - one for connecting each pipe
  // and one to stop the server
  HANDLE events_[EVENTS_COUNT];

  // thread running the server
  HANDLE thread_;
  DWORD  threadId_;
  bool stopping_;

  typedef std::queue<Task> TaskQueue;
  TaskQueue task_queue_;

  CriticalSection queue_critical_section_;
};

}; // namespace utils

#endif // UTILS_THREAD_H_