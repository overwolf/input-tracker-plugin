/*
  Input Tracker Plugin
  Copyright (c) 2015 Overwolf Ltd.
*/
#include "nsScriptableObjectInputTracker.h"
#include "utils/Thread.h"
#include "utils/WorkerThread.h"

#include <sstream>

const char* k_OnMouseMove         = "onMouseMove";
const char* k_OnMouseLButtonDown  = "onMouseLButtonDown";
const char* k_OnMouseLButtonUP    = "onMouseLButtonUP";
const char* k_OnMouseRButtonDown  = "onMouseRButtonDown";
const char* k_OnMouseRButtonUP    = "onMouseRButtonUP";
const char* k_OnMouseWheel        = "onMouseWheel";
const char* k_OnMouseHWheel       = "onMouseHWheel";
const char* k_OnKeyDown           = "onKeyDown";
const char* k_OnKeyUp             = "onKeyup";

nsScriptableObjectInputTracker* g_Instance = NULL; 

HHOOK nsScriptableObjectInputTracker::hMouseHook_ = NULL;
HHOOK nsScriptableObjectInputTracker::hKeyboardHook_ = NULL;
DWORD nsScriptableObjectInputTracker::dHookeFailerGLE = 0;


#define REGISTER_METHOD(name, class) { \
  methods_[NPN_GetStringIdentifier(name)] = \
    new class(this, npp_, twichWapper_); \
}

#define REGISTER_EVENT(name) { \
  events_[NPN_GetStringIdentifier(name)] = NULL; \
}

nsScriptableObjectInputTracker::nsScriptableObjectInputTracker(NPP npp) 
 : nsScriptableObjectBase(npp),
   shutting_down_(false) {

  lastForegroundWindow_ = NULL;
  msgToEvent_[WM_MOUSEMOVE]   = NPN_GetStringIdentifier(k_OnMouseMove);
  msgToEvent_[WM_LBUTTONDOWN] = NPN_GetStringIdentifier(k_OnMouseLButtonDown);
  msgToEvent_[WM_LBUTTONUP]   = NPN_GetStringIdentifier(k_OnMouseLButtonUP);
  msgToEvent_[WM_RBUTTONDOWN] = NPN_GetStringIdentifier(k_OnMouseRButtonDown);
  msgToEvent_[WM_RBUTTONUP]   = NPN_GetStringIdentifier(k_OnMouseRButtonUP);
  msgToEvent_[WM_MOUSEWHEEL]  = NPN_GetStringIdentifier(k_OnMouseWheel);
  msgToEvent_[WM_MOUSEHWHEEL] = NPN_GetStringIdentifier(k_OnMouseHWheel);
  msgToEvent_[WM_KEYDOWN]     = NPN_GetStringIdentifier(k_OnKeyDown);
  msgToEvent_[WM_KEYUP]       = NPN_GetStringIdentifier(k_OnKeyUp);


  notifyEvetEvent_[WM_MOUSEMOVE]   = false;
  notifyEvetEvent_[WM_LBUTTONDOWN] = false;
  notifyEvetEvent_[WM_LBUTTONUP]   = false;
  notifyEvetEvent_[WM_RBUTTONDOWN] = false;
  notifyEvetEvent_[WM_RBUTTONUP]   = false;
  notifyEvetEvent_[WM_MOUSEWHEEL]  = false;
  notifyEvetEvent_[WM_MOUSEHWHEEL] = false;
  notifyEvetEvent_[WM_KEYDOWN]     = false;
  notifyEvetEvent_[WM_KEYUP]       = false;

  g_Instance = this;
}

nsScriptableObjectInputTracker::~nsScriptableObjectInputTracker(void) {
  shutting_down_ = true;
  
  if (thread_.get()) {
    thread_->Stop();
  }

 
  if (hook_thread_.get()) {
   PostThreadMessage(hook_thread_->GetThreadId(), WM_QUIT, 0, 0);
   
   Sleep(10);
   hook_thread_->Stop();
  }

  g_Instance = NULL;
                         
  // just to make sure
  if (hMouseHook_ != NULL)
    UnhookWindowsHookEx(hMouseHook_);
  if (hKeyboardHook_)
    UnhookWindowsHookEx(hKeyboardHook_);
  
  hMouseHook_ = NULL;
  hKeyboardHook_ = NULL;
}

bool nsScriptableObjectInputTracker::Init() {

#pragma region public methods
  REGISTER_EVENT(k_OnMouseMove);        
  REGISTER_EVENT(k_OnMouseLButtonDown);
  REGISTER_EVENT(k_OnMouseLButtonUP);   
  REGISTER_EVENT(k_OnMouseRButtonDown); 
  REGISTER_EVENT(k_OnMouseRButtonUP);   
  REGISTER_EVENT(k_OnMouseWheel);
  REGISTER_EVENT(k_OnMouseHWheel);
  REGISTER_EVENT(k_OnKeyDown);
  REGISTER_EVENT(k_OnKeyUp);
#pragma endregion public methods

#pragma region read-only properties
  #pragma endregion read-only properties

  dHookeFailerGLE = 0;
  thread_.reset(new utils::WorkerThread());
  hook_thread_.reset(new utils::WorkerThread());
  hook_thread_->Start();
  hook_thread_->PostTask(std::bind(&nsScriptableObjectInputTracker::HookMainLoop, this));
  return thread_->Start();
}

bool nsScriptableObjectInputTracker::HasMethod(NPIdentifier name) {
#ifdef _DEBUG
  NPUTF8* name_utf8 = NPN_UTF8FromIdentifier(name);
  NPN_MemFree((void*)name_utf8);
#endif

  // does the method exist?
  return (methods_.find(name) != methods_.end());
}

bool nsScriptableObjectInputTracker::Invoke(
  NPIdentifier name, 
  const NPVariant *args, 
  uint32_t argCount, 
  NPVariant *result) {
#ifdef _DEBUG
    NPUTF8* szName = NPN_UTF8FromIdentifier(name);
    NPN_MemFree((void*)szName);
#endif

    // dispatch method to appropriate handler
    MethodsMap::iterator iter = methods_.find(name);

    if (iter == methods_.end()) {
      // should never reach here
      NPN_SetException(this, "bad function called??");
      return false;
    } 

    return (this->*iter->second)(name, args, argCount, result);
}

/************************************************************************/
/* Public properties
/************************************************************************/
bool nsScriptableObjectInputTracker::HasProperty(NPIdentifier name) {
#ifdef _DEBUG
  NPUTF8* name_utf8 = NPN_UTF8FromIdentifier(name);
  NPN_MemFree((void*)name_utf8);
#endif

  // does the property exist?
  if ((properties_.find(name) != properties_.end()))
    return true;

  return events_.find(name) != events_.end();
}

bool nsScriptableObjectInputTracker::GetProperty(
  NPIdentifier name, NPVariant *result) {

  PropertiesMap::iterator iter = properties_.find(name);
  if (iter == properties_.end()) {
    NPN_SetException(this, "unknown property!?");
    return true;
  }

  char *resultString = (char*)NPN_MemAlloc(iter->second.size());
  memcpy(
    resultString, 
    iter->second.c_str(), 
    iter->second.size());

  STRINGN_TO_NPVARIANT(resultString, iter->second.size(), *result);

  return true;
}

bool nsScriptableObjectInputTracker::SetProperty(
  NPIdentifier name, const NPVariant *value) {
   
  EventsMap::iterator iter = events_.find(name);
  if (iter == events_.end()) {
    NPN_SetException(this, "unknown property!?");
    return false;
  }

  if (hMouseHook_ == NULL) {
    std::string error = ("Mouse Hook failed: " + dHookeFailerGLE);
    NPN_SetException(this, error.c_str());
    return false;
  }

  // mark msg id to false/true
  EventIdToNameMap::iterator msgIter = msgToEvent_.begin();
  int messageId = -1;
  while (messageId != 1 && msgIter != msgToEvent_.end()) {
    if (msgIter->second == iter->first) {
      messageId = msgIter->first;
    }
    msgIter++;
  }

  CriticalSectionLocker locker(lockerCS_);

  if (value->type == NPVariantType_Null) {
    //remove event register
    if (iter->second) {
      NPN_ReleaseObject(iter->second);
    }
    iter->second = NULL;
   
    if (messageId != -1)
      notifyEvetEvent_[messageId] = false;

    return true;
  } else if (!NPVARIANT_IS_OBJECT(*value))  {
    NPN_SetException(this, "invalid params passed to event");
    return false;
  }

  if (messageId != -1)
    notifyEvetEvent_[messageId] = true;

  if (iter->second != NULL)
     NPN_ReleaseObject(iter->second); 

  iter->second = NPVARIANT_TO_OBJECT(*value);
  NPN_RetainObject(iter->second);
  return true;
}

LRESULT CALLBACK KeyboardEvent(int nCode,  WPARAM wParam, LPARAM lParam) {
  KBDLLHOOKSTRUCT* pKeyboardStruct = (KBDLLHOOKSTRUCT*) lParam;
  if (pKeyboardStruct != NULL) {
    switch(wParam) {
    case WM_KEYDOWN:
    case WM_KEYUP:
       if (g_Instance->notifyEvetEvent_[wParam]) {
        g_Instance->thread_->PostTask(std::bind(&nsScriptableObjectInputTracker::NotifyKeyboradEvent,
          g_Instance, wParam, lParam));
       }
      break;
    }
  }
  return CallNextHookEx(g_Instance->hKeyboardHook_,
    nCode,wParam,lParam);
}

LRESULT CALLBACK MouseEvent (int nCode, WPARAM wParam, LPARAM lParam)
{
  MSLLHOOKSTRUCT * pMouseStruct = (MSLLHOOKSTRUCT *)lParam;
  if (pMouseStruct != NULL) {
    switch(wParam) {
      case WM_LBUTTONDOWN:
      case WM_LBUTTONUP:
      case WM_MOUSEMOVE :
      case WM_RBUTTONDOWN:
      case WM_RBUTTONUP:
        if (g_Instance->notifyEvetEvent_[wParam]) {
          g_Instance->thread_->PostTask(std::bind(&nsScriptableObjectInputTracker::NotifyMouseEvent,
            g_Instance, wParam, lParam));
          }
        break;

      case WM_MOUSEWHEEL:
      case WM_MOUSEHWHEEL: 
       if (g_Instance->notifyEvetEvent_[wParam]) {
        g_Instance->thread_->PostTask(std::bind(&nsScriptableObjectInputTracker::NotifyMouseWheelEvent,
          g_Instance, wParam, lParam));
       }
       break;
    }
  }
  return CallNextHookEx(g_Instance->hMouseHook_,
    nCode,wParam,lParam);
}

void nsScriptableObjectInputTracker::HookMainLoop()
{
  hMouseHook_ = SetWindowsHookEx ( 
    WH_MOUSE_LL,
    (HOOKPROC) MouseEvent, 
    NULL,                
    NULL                      
    );

  hKeyboardHook_ = SetWindowsHookEx ( 
    WH_KEYBOARD_LL,
    (HOOKPROC) KeyboardEvent, 
    NULL,                
    NULL                      
    );

  if (hMouseHook_ == NULL) {
    dHookeFailerGLE = GetLastError();
  }
  MSG message;
  while (!shutting_down_ && GetMessage(&message,NULL,0,0) ) {
    TranslateMessage( &message );
    DispatchMessage( &message );
  }

  if (hMouseHook_)
    UnhookWindowsHookEx(hMouseHook_);
 
  if (hKeyboardHook_)
    UnhookWindowsHookEx(hKeyboardHook_);
  hMouseHook_ = NULL;
  hKeyboardHook_ = NULL;
}

NPObject* nsScriptableObjectInputTracker::GetEventCallback(WPARAM wParam)
{
  EventsMap::iterator iter = events_.find(msgToEvent_[wParam]);
  if ( iter== events_.end())
    return NULL;

  return iter->second;
}

void nsScriptableObjectInputTracker::NotifyMouseEvent(WPARAM wParam, LPARAM lParam)
{
  NPObject* callback = GetEventCallback(wParam);
  if (callback == nullptr)
    return;

  MSLLHOOKSTRUCT * pMouseStruct = (MSLLHOOKSTRUCT*)lParam;
  if (pMouseStruct == NULL) 
    return;
  
  NPVariant args[3];
  NPVariant ret_val;

  HWND ForegroundWindow  = GetForegroundWindow();
  if (ForegroundWindow != lastForegroundWindow_) {
    char	sWindowName[MAX_PATH] = {0};
    lastForegroundWindow_ = ForegroundWindow;
    GetWindowText(lastForegroundWindow_,sWindowName, MAX_PATH);
    foregroundWindowText_ = sWindowName;
  }

  POINT clientPT = {pMouseStruct->pt.x, pMouseStruct->pt.y};
  ScreenToClient(ForegroundWindow, &clientPT);

  INT32_TO_NPVARIANT(clientPT.x, args[0]);
  INT32_TO_NPVARIANT(clientPT.y, args[1]);

  STRINGN_TO_NPVARIANT(
    foregroundWindowText_.c_str(),
    foregroundWindowText_.size(),
    args[2]);


  // fire callback
  NPN_InvokeDefault(
    __super::npp_, 
    callback, 
    args, 
    3, 
    &ret_val);

  NPN_ReleaseVariantValue(&ret_val);
}

void nsScriptableObjectInputTracker::NotifyMouseWheelEvent(WPARAM wParam, LPARAM lParam)
{
  CriticalSectionLocker locker(lockerCS_);

  NPObject* callback = GetEventCallback(wParam);
  if (callback == nullptr)
    return;

  MSLLHOOKSTRUCT * pMouseStruct = (MSLLHOOKSTRUCT*)lParam;
  if (pMouseStruct == NULL) 
    return;

  NPVariant args[1];
  NPVariant ret_val;

  INT32_TO_NPVARIANT((short)HIWORD(pMouseStruct->mouseData), args[0]);

  // fire callback
  NPN_InvokeDefault(
    __super::npp_, 
    callback, 
    args, 
    1, 
    &ret_val);

  NPN_ReleaseVariantValue(&ret_val);
}

void nsScriptableObjectInputTracker::NotifyKeyboradEvent(WPARAM wParam, LPARAM lParam)
{
  CriticalSectionLocker locker(lockerCS_);

  NPObject* callback = GetEventCallback(wParam);
  if (callback == nullptr)
    return;
  
  KBDLLHOOKSTRUCT* pKeyboardStruct = (KBDLLHOOKSTRUCT*) lParam;
  if (pKeyboardStruct == NULL) 
    return;

  NPVariant args[1];
  NPVariant ret_val;

  INT32_TO_NPVARIANT(pKeyboardStruct->vkCode, args[0]);

  // fire callback
  NPN_InvokeDefault(
    __super::npp_, 
    callback, 
    args, 
    1, 
    &ret_val);

  NPN_ReleaseVariantValue(&ret_val);
}
