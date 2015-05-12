#include "CriticalSectionLock.h"


CriticalSectionLock::CriticalSectionLock(CRITICAL_SECTION& criticalSection) :
  m_criticalSection(criticalSection) {
  
  EnterCriticalSection(&m_criticalSection);
}

CriticalSectionLock::~CriticalSectionLock() {
  LeaveCriticalSection(&m_criticalSection);
}

CriticalSection::CriticalSection()
{
  InitializeCriticalSection(&m_criticalSection);
}

CriticalSection::~CriticalSection()
{
  DeleteCriticalSection(&m_criticalSection);
}

void CriticalSection::lock()
{
  EnterCriticalSection(&m_criticalSection);
}

void CriticalSection::unlock()
{
  LeaveCriticalSection(&m_criticalSection);
}

CriticalSectionLocker::CriticalSectionLocker(CriticalSection& criticalSection) :
m_criticalSection(criticalSection) {
  m_criticalSection.lock();
}

CriticalSectionLocker::~CriticalSectionLocker() {
  m_criticalSection.unlock();
}
