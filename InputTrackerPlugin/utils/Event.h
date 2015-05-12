/*------------------------------------------------------------------------
$Workfile: $
$Archive: $
$Author: Tom Wolf $
$Revision: $
$Date: $

Description:
	
  Copyright © 2013 Overwolf Ltd. All rights reserved.
------------------------------------------------------------------------*/
#ifndef EVENT_H_
#define EVENT_H_

#include <Windows.h>

class Event {
public:
  Event();
  virtual ~Event();

public:
  bool Create(bool bManualReset, 
              bool bInitialState,
              LPCTSTR lpcszName = NULL);
  void Destroy();

  bool IsCreated();

  bool Wait(DWORD dwMilliseconds_ = INFINITE);
  bool Signal();
  bool Reset();

private:
  HANDLE m_hEvent;
};

#endif