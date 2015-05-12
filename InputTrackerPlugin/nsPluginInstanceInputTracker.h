/*
  Input Tracker Plugin
  Copyright (c) 2015 Overwolf Ltd.
*/
#ifndef nsPluginInstanceInputTracker_H_
#define nsPluginInstanceInputTracker_H_

#include <memory> // smart ptrs
#include <windows.h> // required for pluginbase.h
#include "plugin_common/pluginbase.h" // nsPluginInstanceBase
#include "utils/CriticalSectionLock.h"


class nsPluginInstanceInputTracker : public nsPluginInstanceBase {
public:
  nsPluginInstanceInputTracker(NPP instance);
  virtual ~nsPluginInstanceInputTracker();

public:
  NPBool init(NPWindow* window);
  void shut();
  NPBool isInitialized();

  NPError GetValue(NPPVariable variable, void* scriptable_object_);


private:
  NPP instance_;
  NPBool initialized_;
  
  CriticalSection scriptable_object_CS_;

  NPObject* scriptable_object_;

  static int ref_count_;
};

#endif // nsPluginInstanceInputTracker_H_