/*
  Input Tracker Plugin
  Copyright (c) 2015 Overwolf Ltd.
*/
#include "WorkerThread.h"

using namespace utils;

const DWORD kStopThreadTimeoutMS = 10000;

WorkerThread::WorkerThread() : 
  thread_(nullptr),
  stopping_(false) {
}

WorkerThread::~WorkerThread() {

}

bool WorkerThread::Start() {
  if (nullptr != thread_) {
    return false;
  }

  if (!CreateEvents()) {
    return false;
  }

  ClearQueue();

  stopping_ = false;

  thread_ = 
    CreateThread(nullptr, 
                 0,  
                 ThreadProc, 
                 (LPVOID)this, 
                 0,
                 &threadId_);

  return (nullptr != thread_);
}

bool WorkerThread::Stop() {
  if (nullptr == thread_) {
    return true;
  }

  // make sure we don't process more messages
  {
    CriticalSectionLocker lock(queue_critical_section_);
    stopping_ = true;
  }

  SetEvent(events_[EVENT_STOP]);
  bool ret = 
    (WAIT_OBJECT_0 == WaitForSingleObject(thread_, kStopThreadTimeoutMS));
  DestroyEvents();
  return ret;
}

bool WorkerThread::PostTask(Task task_func) {
  CriticalSectionLocker lock(queue_critical_section_);
  
  if (NULL == thread_) {
    return false;
  }

  if (stopping_) {
    return false;
  }

  task_queue_.push(task_func);

  return (TRUE == SetEvent(events_[EVENT_NEW_TASK]));
}

bool WorkerThread::CreateEvents() {
  events_[EVENT_STOP] = 
    CreateEvent(nullptr, 
                TRUE,    // manual reset event 
                FALSE,   // initial state = NOT signaled
                nullptr);   // unnamed event object

  events_[EVENT_NEW_TASK] = 
    CreateEvent(nullptr, 
                FALSE,    // NOT a manual reset event 
                FALSE,    // initial state = not signaled
                nullptr);   // unnamed event object 

  for (int i = 0; i < EVENTS_COUNT; i++) {
    if (nullptr == events_[i]) {
      return false;
    }
  }

  return true;
}

void WorkerThread::DestroyEvents() {
  for (int i = 0; i < EVENTS_COUNT; i++) {
    if (nullptr != events_[i]) {
      CloseHandle(events_[i]);
      events_[i] = nullptr;
    }
  }
}

void WorkerThread::ClearQueue() {
  CriticalSectionLocker lock(queue_critical_section_);

  TaskQueue empty_queue;
  std::swap(task_queue_, empty_queue);
}

void WorkerThread::HandleNewTaskEvent() {
  while (!task_queue_.empty()) {
    Task task;

    {
      CriticalSectionLocker lock(queue_critical_section_);

      // don't continue processing messages if we are stopping
      // the pipe
      if (stopping_) {
        return;
      }

      task = task_queue_.front();
      task_queue_.pop();
    }

    if (!task._Empty()) {
      task();
    }
  }
}

DWORD WINAPI WorkerThread::ThreadProc(IN LPVOID lpParameter_) {
  WorkerThread* p_thread_queue = (WorkerThread*)lpParameter_;

  if (nullptr == lpParameter_) {
    return 0;
  }

  bool continue_running = true;

  while (continue_running) { 
    DWORD wait = 
      WaitForMultipleObjectsEx(EVENTS_COUNT,
                               p_thread_queue->events_,
                               FALSE,
                               INFINITE,  // waits indefinitely 
                               TRUE);     // alertable wait enabled 

    int event_index = (wait - WAIT_OBJECT_0);

    // check if we are to be stopped
    if ((EVENT_STOP == event_index) ||
        (event_index < 0) || 
        (event_index > EVENTS_COUNT)) {
        continue_running = false;
    } else {
      // new task
      p_thread_queue->HandleNewTaskEvent();
    }
  }

  // cleanup
  p_thread_queue->ClearQueue();
  p_thread_queue->thread_ = nullptr;
  
  return 0;
}
