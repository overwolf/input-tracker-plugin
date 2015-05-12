/*------------------------------------------------------------------------
$Workfile: $
$Archive: $
$Author: Tom Wolf $
$Revision: $
$Date: $

Description:
	
  Copyright © 2013 Overwolf Ltd. All rights reserved.
------------------------------------------------------------------------*/
#ifndef THREAD_H_
#define THREAD_H_

#include <Windows.h>
#include <string>

class Thread {
public:
  Thread(const char* thread_name);
  virtual ~Thread();

public:
  void SetThreadName(const char* threadName);
  bool Wait(DWORD dwMilliseconds_ = INFINITE);

  virtual bool Start();
  virtual bool Stop();

protected:
  inline bool IsStopping() {
    return stopping_;
  }

  inline bool IsRunning() {
    return (NULL != thread_);
  }

  virtual void ThreadFunction() = 0;

private:
  static DWORD WINAPI ThreadProc(IN LPVOID parameter);

private:
  // thread running the server
  HANDLE thread_;

  bool stopping_;

  std::string thread_name_;
};

#endif // THREAD_H_