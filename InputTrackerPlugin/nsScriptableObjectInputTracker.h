/*
  Input Tracker Plugin
  Copyright (c) 2015 Overwolf Ltd.
*/
#ifndef nsScriptableObjectInputTracker_H_
#define nsScriptableObjectInputTracker_H_

#include "nsScriptableObjectBase.h"
#include "utils/CriticalSectionLock.h"
#include <map>


namespace utils {
  class WorkerThread; // forward declaration
}

class nsScriptableObjectInputTracker : public nsScriptableObjectBase {
public:
  nsScriptableObjectInputTracker(NPP npp);
  virtual ~nsScriptableObjectInputTracker(void);

public:
  bool Init();

// nsScriptableObjectBase overrides
public:
  virtual bool HasMethod(NPIdentifier name);
  virtual bool Invoke(
    NPIdentifier name, 
    const NPVariant *args, 
    uint32_t argCount, 
    NPVariant *result);
  virtual bool HasProperty(NPIdentifier name);
  virtual bool GetProperty(NPIdentifier name, NPVariant *result);
  virtual bool SetProperty(NPIdentifier name, const NPVariant *value);


private:
  void HookMainLoop();

  NPObject*  GetEventCallback(WPARAM wParam);
  void NotifyMouseEvent(WPARAM wParam, LPARAM lParam);
  void NotifyMouseWheelEvent(WPARAM wParam, LPARAM lParam);
  void NotifyKeyboradEvent(WPARAM wParam, LPARAM lParam);

  friend LRESULT CALLBACK KeyboardEvent(int nCode,  WPARAM wParam, LPARAM lParam);
  friend LRESULT CALLBACK MouseEvent (int nCode, WPARAM wParam, LPARAM lParam);
// member variables
private:
  // defines a generic method
  typedef bool (nsScriptableObjectInputTracker::*MethodHandle)(
    NPIdentifier,
    const NPVariant*, 
    uint32_t, 
    NPVariant*);

  // holds the public methods
  typedef std::map<NPIdentifier, MethodHandle> MethodsMap;
  MethodsMap methods_;

  // holds the public methods
  typedef std::map<NPIdentifier, std::string> PropertiesMap;
  PropertiesMap properties_;
  
  typedef std::map<int, NPIdentifier> EventIdToNameMap;
  EventIdToNameMap msgToEvent_;

  typedef std::map<int, bool> MessageIdToNameMap;
  MessageIdToNameMap notifyEvetEvent_;

  typedef std::map<NPIdentifier, NPObject* > EventsMap;
  EventsMap events_;

  CriticalSection lockerCS_;
  // good idea for when having an autonomous thread sending callbacks
  bool shutting_down_;
  
  // this allows us to run our code on a separate thread than the 
  // main browser thread - to be more responsive
  std::auto_ptr<utils::WorkerThread> thread_;

  std::auto_ptr<utils::WorkerThread> hook_thread_;

  static HHOOK hMouseHook_;
  static HHOOK hKeyboardHook_;
  static DWORD dHookeFailerGLE;
  HWND lastForegroundWindow_;
  std::string foregroundWindowText_;
};

// declare our NPObject-derived scriptable object class
DECLARE_NPOBJECT_CLASS_WITH_BASE(
  nsScriptableObjectInputTracker, 
  AllocateNpObject<nsScriptableObjectInputTracker>);


#endif // nsScriptableObjectInputTracker_H_
