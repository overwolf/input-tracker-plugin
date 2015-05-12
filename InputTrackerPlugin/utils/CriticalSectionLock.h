/*------------------------------------------------------------------------
$Workfile: $
$Archive: $
$Author: Tom Wolf $
$Revision: $
$Date: $

Description:
	
  Copyright © 2013 Overwolf Ltd. All rights reserved.
------------------------------------------------------------------------*/
#ifndef COMMON_UTILS_CRITICAL_SECTION_LOCK_H_
#define COMMON_UTILS_CRITICAL_SECTION_LOCK_H_

#include <Windows.h>

class CriticalSectionLock {
public:
    CriticalSectionLock(CRITICAL_SECTION& criticalSection);
    virtual ~CriticalSectionLock();

private:
    CRITICAL_SECTION& m_criticalSection;
}; // class CriticalSectionLock


class CriticalSection {
public:
    CriticalSection();
    virtual ~CriticalSection();

    void lock();
    void unlock();
private:
    CRITICAL_SECTION m_criticalSection;
}; // class CriticalSectionLock

class CriticalSectionLocker {
public:
    CriticalSectionLocker(CriticalSection& criticalSection);
    virtual ~CriticalSectionLocker();

private:
    CriticalSection& m_criticalSection;
}; // class CriticalSectionLock

#endif // COMMON_UTILS_CRITICAL_SECTION_LOCK_H_