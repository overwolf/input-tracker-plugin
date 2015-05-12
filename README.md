InputTrackerPlugin
======================
This is a Skeleton NPAPI project that compiles (VS2010) to a DLL that you can later reference in your app.  In order to make it compile, you'll need to download the xulrunner-sdk.

You can get xulrunner-sdk from: https://developer.mozilla.org/en/docs/Gecko_SDK.

NOTE: Was successfully tested with version Gecko 1.9.2 (Firefox 3.6).

Unzip the xulrunner-sdk folder side-by-side with the InputTrackerPlugin folder - the project references this folder (see Additional Include Directories).

NOTE: we make sure to compile the plugin /MT so there are no dependencies on the C runtime libraries.


Events
======
* To unregister event just set listener to null (e.g plugin().onMouseLButtonDow = null)

plugin().onMouseLButtonDown = function (x,y, windowText) {
	console.log("onMouseLButtonDown: ", x,y, windowText);
};

plugin().onMouseLButtonUP = function (x,y, windowText) {
	console.log("onMouseLButtonUP: ", x,y, windowText);
};

*use only when necessary (performance hit)

plugin().onMouseMove = function (x,y) {
        console.log("onMouseMove: ", x,y);
};	


plugin().onMouseRButtonDown = function (x,y) {
	console.log("onMouseRButtonDown: ", x,y);
};

plugin().onMouseRButtonUP = function (x,y) {
	console.log("onMouseRButtonUP: ", x,y);
};


plugin().onMouseWheel = function (e) {
	console.log("onMouseMouseWheel", e);
};
	

plugin().onMouseHWheel = function (e) {
	console.log("onMouseHMouseWheel", e);
};
	

plugin().onKeyDown = function (e) {
	console.log("onKeyDown", e);
};
	

plugin().onKeyup = function (e) {
	console.log("onKeyup", e);
};


Sample Overwolf WebApp
======================
This is an unpacked Overwolf WebApp that utilizes the Overwolf sample plugin.

NOTE: the InputTrackerPlugin.dll  file is placed inside of the WebApp's directory - so if you recompile it, you'll need to place the new DLL file inside the folder (overriding the existing dll file).

Please review the manifest.json file to view how to reference the NPAPI file.

Please, don't hesitate to ask us questions in our developers forum: http://forums.overwolf.com/.

The Overwolf Team
