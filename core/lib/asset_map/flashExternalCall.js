/**
* Copyright (c) 2003 - Squiz Pty Ltd
*
* $Id: flashExternalCall.js,v 1.7 2003/09/26 05:26:32 brobertson Exp $
* $Name: not supported by cvs2svn $
*/

/**
* This function works in partnership with the ExternalCall flash class.
* What it allows is the execution of commands inside the flash that
* you are unable to do normally.
*
*/

function jsToFlashCall(swObj, cmd, params)
{
	 if (navigator.appName.indexOf("Microsoft") == -1) {
		alert('Moz reloaded stuffed SetVariable() is not a fn');
		return;
	 }
//	alert('js to flash calling with ' + swObj + "/" + cmd + "/" + params);
	swObj.SetVariable('_root.external_call.cmd', cmd);

	for(i in params) {
		var name  = escape(i);
		var value = escape(params[i]);
		swObj.SetVariable('_root.external_call.add_param', name + '=' + value);
	}// end for

	swObj.SetVariable('_root.external_call.exec', 'true');

}// end jsToFlashCall()

/**
* This function works in partnership with the ExternalCall flash class.
* What it allows is the execution of commands inside the flash that
* you are unable to do normally.
*
*/
var FLASH_TO_JS_CALL_BACK_FNS = {};
function registerFlashToJsCall(cmd, fn)
{
	FLASH_TO_JS_CALL_BACK_FNS[cmd] = fn;
}// end flashToJsCall()

/**
* This function works in partnership with the ExternalCall flash class.
* What it allows is the execution of commands inside the flash that
* you are unable to do normally.
*
*/
var FLASH_TO_JS_CALL = null;
function flashToJsCall(arg)
{

	var str = new String(arg);
	var pieces = str.match(/^([a-z_]+):\/\/(.*)$/);

	switch(pieces[1]) {
		case "cmd" :
			if (FLASH_TO_JS_CALL !== null) return;
			var cmd = pieces[2];
			if (FLASH_TO_JS_CALL_BACK_FNS[cmd] == undefined) {
				alert('Command "' + cmd + '" has not been registered, unable to perform flashToJsCall');
				return;
			}
			FLASH_TO_JS_CALL = {cmd: cmd, params: {}}
			break;
		case "add_param" :
			if (FLASH_TO_JS_CALL !== null) {
				var tmp = pieces[2].split("=", 2);
				var name  = unescape(tmp[0]);
				var value = unescape(tmp[1]);
				FLASH_TO_JS_CALL.params[name] = value;
			}
			break;
		case "exec" :
			if (FLASH_TO_JS_CALL !== null && pieces[2] == "true") {
				//alert("onExternalCall : " + FLASH_TO_JS_CALL.cmd);
				//for(var i in FLASH_TO_JS_CALL.params) alert("params -> " + i + " : " + FLASH_TO_JS_CALL.params[i]);
				FLASH_TO_JS_CALL_BACK_FNS[FLASH_TO_JS_CALL.cmd](FLASH_TO_JS_CALL.params);
				// reset the storage units
				FLASH_TO_JS_CALL = null;
			}

		break;
	}// end switch
}// end flashToJsCall()

