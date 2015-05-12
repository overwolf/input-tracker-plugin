/*------------------------------------------------------------------------
$Workfile: $
$Archive: $
$Author: Tom Wolf $
$Revision: $
$Date: $

Description:
	
  Copyright © 2013 Overwolf Ltd. All rights reserved.
------------------------------------------------------------------------*/
#include "Event.h"

Event::Event() : m_hEvent(NULL) {
}

Event::~Event() {
  Destroy();
}

bool Event::Create(bool bManualReset, 
                   bool bInitialState,
                   LPCTSTR lpcszName /*= NULL*/) {
  if (IsCreated()) {
    return false;
  }

  m_hEvent = CreateEvent(NULL, 
                         bManualReset?TRUE:FALSE, 
                         bInitialState?TRUE:FALSE,
                         lpcszName);

  return (NULL != m_hEvent);
}

void Event::Destroy() {
  if (IsCreated()) {
    Reset();
    CloseHandle(m_hEvent);
    m_hEvent = NULL;
  }
}

bool Event::IsCreated() {
  return (NULL != m_hEvent);
}


bool Event::Wait(DWORD dwMilliseconds_) {
  if (!IsCreated()) {
    return false;
  }

  return (WAIT_OBJECT_0 == WaitForSingleObject(m_hEvent, dwMilliseconds_));
}

bool Event::Signal() {
  if (!IsCreated()) {
    return false;
  }

  return (TRUE == SetEvent(m_hEvent));
}

bool Event::Reset() {
  if (!IsCreated()) {
    return false;
  }

  return (TRUE == ResetEvent(m_hEvent));
}