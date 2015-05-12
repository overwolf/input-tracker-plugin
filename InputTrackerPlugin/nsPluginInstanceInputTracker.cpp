/*
  Input Tracker Plugin
  Copyright (c) 2015 Overwolf Ltd.
*/
#include "nsPluginInstanceInputTracker.h"
#include "nsScriptableObjectInputTracker.h" // our specific API
#include "utils/CriticalSectionLock.h"
// we use this to force our plugin container to shut down
// when no one is using it.  Browsers try to keep the plugin
// open for optimization reasons - we don't want it
int nsPluginInstanceInputTracker::ref_count_ = 0;
////////////////////////////////////////
//
// nsPluginInstanceInputTracker class implementation
//
nsPluginInstanceInputTracker::nsPluginInstanceInputTracker(NPP instance) :
  nsPluginInstanceBase(),
  instance_(instance),
  initialized_(FALSE),
  scriptable_object_(nullptr) {

  nsPluginInstanceInputTracker::ref_count_++;
}

nsPluginInstanceInputTracker::~nsPluginInstanceInputTracker() {
  nsPluginInstanceInputTracker::ref_count_--;

  if (0 == nsPluginInstanceInputTracker::ref_count_) {
    PostQuitMessage(0);
  }
}

// NOTE:
// ------
// Overwolf plugins should not implement windows - NPAPI will
// probably be removed in the near feature and will be changed
// by a different method that will only support non-visual
// plugins
NPBool nsPluginInstanceInputTracker::init(NPWindow* window) {
  // no GUI to init in windowless case
  initialized_ = TRUE;

  return TRUE;
}

void nsPluginInstanceInputTracker::shut() {

  {
    CriticalSectionLocker lock(scriptable_object_CS_);
    if (nullptr != scriptable_object_) {
      NPN_ReleaseObject(scriptable_object_);
      scriptable_object_ = NULL;
    }


    initialized_ = FALSE;
  }
}

NPBool nsPluginInstanceInputTracker::isInitialized() {
  return initialized_;
}

// here we supply our scriptable object
NPError nsPluginInstanceInputTracker::GetValue(
  NPPVariable variable, void* ret_value) {
  
  NPError rv = NPERR_INVALID_PARAM;

  switch (variable) {
    case NPPVpluginScriptableNPObject:
    {
      if (nullptr == scriptable_object_) {
        {

          CriticalSectionLocker lock(scriptable_object_CS_);
          scriptable_object_ = 
            NPN_CreateObject(
            instance_, 
            GET_NPOBJECT_CLASS(nsScriptableObjectInputTracker));

          NPN_RetainObject(scriptable_object_);

       ((nsScriptableObjectInputTracker*)scriptable_object_)->Init();
        *(NPObject **)ret_value = scriptable_object_;
        }
      }

      rv = NPERR_NO_ERROR;
      return rv;
    }
    default:
      break;
  }

  return rv;
}
