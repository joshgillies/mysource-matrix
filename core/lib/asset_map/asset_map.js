/**
* +--------------------------------------------------------------------+
* | Squiz.net Open Source Licence                                      |
* +--------------------------------------------------------------------+
* | Copyright (c), 2003 Squiz Pty Ltd (ABN 77 084 670 600).            |
* +--------------------------------------------------------------------+
* | This source file may be used subject to, and only in accordance    |
* | with, the Squiz Open Source Licence Agreement found at             |
* | http://www.squiz.net/licence.                                      |
* | Make sure you have read and accept the terms of that licence,      |
* | including its limitations of liability and disclaimers, before     |
* | using this software in any way. Your use of this software is       |
* | deemed to constitute agreement to be bound by that licence. If you |
* | modify, adapt or enhance this software, you agree to assign your   |
* | intellectual property rights in the modification, adaptation and   |
* | enhancement to Squiz Pty Ltd for use and distribution under that   |
* | licence.                                                           |
* +--------------------------------------------------------------------+
*
* $Id: asset_map.js,v 1.1 2004/01/16 00:34:30 mmcintyre Exp $
* $Name: not supported by cvs2svn $
*/


/*--         START GLOBALS            --*/


var ASSET_MAP_FLASH_CHECKED = false;
var FLASH_TO_JS_CALL_BACK_FNS = {};
var IE_FLASH_VERSION  = 6;
var MOZ_FLASH_VERSION = 7;
var flash_InternetExplorer = (navigator.appName.indexOf("Microsoft") != -1);


// Hook for Internet Explorer
if (navigator.appName && navigator.appName.indexOf("Microsoft") != -1 &&
	navigator.userAgent.indexOf("Windows") != -1 && navigator.userAgent.indexOf("Windows 3.1") == -1) {
		document.write('<SCRIPT LANGUAGE="VBScript"\> \n');
		document.write('on error resume next \n');
		document.write('Sub asset_map_FSCommand(ByVal command, ByVal args)\n');
		document.write('  call asset_map_DoFSCommand(command, args)\n');
		document.write('end sub\n');
		document.write('</SCRIPT\> \n');
}


/*--         END GLOBALS             --*/


/**
* Initalises the asset map
*
* Checks the browser flavour and version and make some desicions based on that information
* about whether communication should be stable between the javascript and the asset map
*/
function init_asset_map() {
	if (matches = navigator.userAgent.match(/MSIE ([0-9.]+)/)) {
		if (matches[1] < '6.0') {
			alert('You need to use Internet Explorer 6.0 or above for the communication between the Asset Map and the Javascript');
		} else {
			document.write('<SCR' + 'IPT LANGUAGE=VBScript\> \n'); //FS hide this from IE4.5 Mac by splitting the tag
			document.write('on error resume next \n');
			document.write('ASSET_MAP_FLASH_CHECKED = ( IsObject(CreateObject("ShockwaveFlash.ShockwaveFlash.' + IE_FLASH_VERSION + '")))\n');
			document.write('</SCR' + 'IPT\> \n');
			if (!ASSET_MAP_FLASH_CHECKED) {
				alert('You need to use to have version ' + IE_FLASH_VERSION + ' of Flash installed for the communication between the Asset Map and the Javascript');
			}
		}

	} else {

		var firebird_re = /(Firebird)\/([0-9.]+)/;
		var moz_re      = /^Mozilla\/5\.0.*rv:([^)]+)\)/;

		if ((matches = navigator.userAgent.match(firebird_re)) || (matches = navigator.userAgent.match(moz_re))) {
			if (matches[1] == 'Firebird' && matches[2] < '0.6.1') {
				alert('You need to use Firebird 0.6.1 or above for the communication between the Asset Map and the Javascript');

			} else if (matches[1] == 'Mozilla' && matches[2] < '1.4') {
				alert('You need to use Mozilla 1.4 or above for the communication between the Asset Map and the Javascript');

			} else {

				var plugin = (navigator.mimeTypes && navigator.mimeTypes["application/x-shockwave-flash"]) ? navigator.mimeTypes["application/x-shockwave-flash"].enabledPlugin : 0;
				if (plugin) {
					var words = navigator.plugins["Shockwave Flash"].description.split(" ");
					for (var i = 0; i < words.length; ++i)
					{
						if (isNaN(parseInt(words[i]))) continue;
						var MM_PluginVersion = words[i]; 
					}
					ASSET_MAP_FLASH_CHECKED = (MM_PluginVersion >= MOZ_FLASH_VERSION);
				}

				if (!ASSET_MAP_FLASH_CHECKED) {
					alert('You need to use to have version ' + MOZ_FLASH_VERSION + ' of Flash installed for the communication between the Asset Map and the Javascript');
				}

			}// end if

		// we don't know about this browser, ah well may as well give it a go ...
		} else {
			alert('You are using an untested browser there is no guarantee that the communication between the Asset Map and the Javascript will be successful');
			ASSET_MAP_FLASH_CHECKED = true;

		}// end if

	}// end if
}//end init_asset_map()


/**
* Handle all the the FSCommand messages in a Flash movie
*/
function asset_map_DoFSCommand(command, args)
{
	//var asset_mapObj = flash_InternetExplorer ? asset_map : document.asset_map;
	if (command == "flashToJsCall") {
		flashToJsCall(args);
	}
}//end asset_map_DoFSCommand()


/*
* Reload the passed assetid in the flash
*/
function reload_asset(assetid)
{
	var asset_mapObj = document.asset_map;
	//alert("Reload Assetid : " + assetid);
	jsToFlashCall(asset_mapObj, 'reload_asset', {assetid: assetid});

}//end reload_asset()


/*
* Reload the assetids represented in the flash
*/
function reload_assets(assetids_xml)
{
	var asset_mapObj = document.asset_map;
	jsToFlashCall(asset_mapObj, 'reload_assets', {assetids_xml: assetids_xml});

}//end reload_assets()


/*
* Reload the passed assetid in the flash
*/
function refresh_internal_messages()
{
	var asset_mapObj = document.asset_map;
	jsToFlashCall(asset_mapObj, 'refresh_mail', {});

}//end refresh_internal_messages()


/**
* highlight the link path
*/
function select_path(link_path)
{
	var asset_mapObj = document.asset_map;
	jsToFlashCall(asset_mapObj, 'select_path', {link_path: link_path});

}//end select_path()


/**
* Reload the passed assetid in the flash
*/
function add_messages(xml)
{
	var asset_mapObj = document.asset_map;
	//alert("Add Message : " + xml);
	jsToFlashCall(asset_mapObj, 'add_message', {msgs_xml: xml});

}//end add_messages()


/* 
* Reload the passed assetid in the flash
*/
function asset_map_popup(params)
{
	var popup_win = window.open(params.url, 'sq_asset_map_popup', 'toolbar=0,menubar=0,location=0,status=0,scrollbars=1,resizable=1,width=650,height=400');

}//end asset_map_popup()
registerFlashToJsCall('asset_map_popup', asset_map_popup);

/**
* Open up a help window
*/
function open_help(params)
{
	var popup_win = window.open("<?php echo sq_web_path('lib'); ?>/web/asset_map_help.php", 'sq_asset_map_popup', 'toolbar=0,menubar=0,location=0,status=0,scrollbars=1,resizable=1,width=580,height=180');

}//end open_help()
registerFlashToJsCall('open_help', open_help);



/** 
* Open up a help window
*/
function open_legend(params)
{
	var popup_win = window.open("<?php echo sq_web_path('lib'); ?>/web/asset_map_key.php", 'sq_asset_map_popup', 'toolbar=0,menubar=0,location=0,status=0,scrollbars=1,resizable=1,width=350,height=300');

}//end open_legend()
registerFlashToJsCall('open_legend', open_legend);


  //////////////////////////////////
 //   Asset Finder Functions     //
//////////////////////////////////



var ASSET_FINDER_FIELD_NAME = null;
var ASSET_FINDER_FIELD_SAFE_NAME = null;
var ASSET_FINDER_DONE_FUNCTION = null;
var ASSET_FINDER_OBJ = null;

/**
* set the finder object that initiated the asset finder
*
* @param finder the finder that initiated the asset finder
*/
function set_finder(finder) {
	ASSET_FINDER_OBJ = finder;

}//end set_finder()


/**
* Activated by the pressing of the "Change" button to start the asset finder mode in the flash menu
*
* @param string	$name			the name of the hidden field
* @param string	$safe_name		the name prefix for all the other form elements associated with the
* @param string	$type_codes		the typecodes seperated by a pipe
*
* @access public
*/
function asset_finder_change_btn_press(name, safe_name, type_codes, done_fn)
{
	ASSET_FINDER_DONE_FUNCTION = done_fn;

	if (ASSET_FINDER_FIELD_NAME != null && ASSET_FINDER_FIELD_NAME != name) {
		alert('The asset finder is currently in use');
		return;
	}

	// no name ? we must be starting the asset finder
	if (ASSET_FINDER_FIELD_NAME == null) {
		ASSET_FINDER_FIELD_NAME = name;
		ASSET_FINDER_FIELD_SAFE_NAME = safe_name;
		
		// simple temporary hack to find if we are using the java or the flash
		if (null != (document.getElementById('asset_map'))) {
			flash_asset_finder_start(type_codes);
		} else if (null != (document.getElementById('sq_asset_map'))) {
			asset_finder_start('asset_finder_done', type_codes);
		} else {
			alert("Could not find the asset map");
			return false;
		}

		ASSET_FINDER_OBJ.set_button_value(ASSET_FINDER_FIELD_SAFE_NAME + '_change_btn', 'Cancel');

	// else we must be cancelling the asset finder
	} else {
		asset_finder_cancel();
		ASSET_FINDER_OBJ.set_button_value(ASSET_FINDER_FIELD_SAFE_NAME + '_change_btn', 'Change');
		ASSET_FINDER_FIELD_NAME = null;
		ASSET_FINDER_FIELD_SAFE_NAME = null;
	}

}// end asset_finder_change_btn_press()


/**
* Call-back fns that stops the asset finder
*
* @param Array params the params array
* @param string label the label to give this asset
* @param string url the url of this asset
*
* @access public
*/
function asset_finder_done(params, label, url)
{
	if (ASSET_FINDER_FIELD_NAME == null) return;

	// we are in flash
	if (params.assetid) {
		// if we get a -1 they cancelled, do nothing
		if (params.assetid != -1) {
			ASSET_FINDER_OBJ.set_hidden_field(ASSET_FINDER_FIELD_NAME + '[assetid]', params.assetid);
			ASSET_FINDER_OBJ.set_hidden_field(ASSET_FINDER_FIELD_NAME + '[url]', params.url);
			ASSET_FINDER_OBJ.set_text_field(ASSET_FINDER_FIELD_SAFE_NAME + '_label', (params.assetid == 0) ? '' : params.label + ' (Id : #' + params.assetid + ')');
		}
	} else {
		var assetid = params;
		// we are in java, params is actually the assetid
		if (assetid != -1) {
			ASSET_FINDER_OBJ.set_hidden_field(ASSET_FINDER_FIELD_NAME + '[assetid]',assetid);
			ASSET_FINDER_OBJ.set_hidden_field(ASSET_FINDER_FIELD_NAME + '[url]', url);
			ASSET_FINDER_OBJ.set_text_field(ASSET_FINDER_FIELD_SAFE_NAME + '_label', (assetid == 0) ? '' : label + ' (Id : #' + assetid + ')');
		}
	}

	ASSET_FINDER_OBJ.set_button_value(ASSET_FINDER_FIELD_SAFE_NAME + '_change_btn', 'Change');
	ASSET_FINDER_FIELD_NAME = null;
	ASSET_FINDER_FIELD_SAFE_NAME = null;
	if (ASSET_FINDER_DONE_FUNCTION !== null) ASSET_FINDER_DONE_FUNCTION();

}//end asset_finder_done()


/**
* Starts the asset finder and lets the asset map know that we are now in asset finder mode
* 
* @param String		fn				the function to call when done
* @param String		type_codes		the type codes to restrict the asset finder to
*
* @access public
*/
var ASSET_FINDER_CALL_BACK = null;
function asset_finder_start(fn, type_codes)
{
	var asset_mapObj = document.getElementById('sq_asset_map');
	var params = new Array();
	params["callback_fn"] = fn;
	params["type_codes"] = type_codes;
	
	jsToJavaCall(asset_mapObj, 'asset_finder', 'assetFinderStarted', params);

}//end asset_finder_start()


/*
* Initialises the asset finder in the flash
*
* @param Function	fn			the call-back function to with the results
* @param string		type_codes	the type codes seperated by a pipe ( | )
*
*/
function flash_asset_finder_start(type_codes)
{
	var asset_mapObj = document.getElementById('asset_map');

	registerFlashToJsCall('asset_finder_done', asset_finder_done)
	jsToFlashCall(asset_mapObj, 'asset_finder', {action: 'start', type_codes: type_codes});

}//end flash_asset_finder_start()


/**
* Alerts the asset map that asset finder mode has been canceled
*
* @access public
*/
function asset_finder_cancel() {

	// simple temporary hack to find if we are using the java or the flash
	if (null != (document.getElementById('asset_map'))) {
		var asset_mapObj = document.getElementById('asset_map');
		jsToFlashCall(asset_mapObj, 'asset_finder', {action: 'cancel'});
	} else if (null != (document.getElementById('sq_asset_map'))) {
		var asset_mapObj = document.getElementById('sq_asset_map');
		params = new Array();
		jsToJavaCall(asset_mapObj, 'asset_finder', 'assetFinderStopped', params);
	} else {
		alert("Could not find the asset map");
		return false;
	}

}//end asset_finder_cancel()


/**
* Activated by the pressing of the "Clear" button
*
* @param string	$name			the name of the hidden field
* @param string	$safe_name		the name prefix for all the other form elements associated with the
*
* @access public
*/
function asset_finder_clear_btn_press(name, safe_name)
{
	ASSET_FINDER_OBJ.set_hidden_field(name + '[assetid]', 0);
	ASSET_FINDER_OBJ.set_hidden_field(name + '[url]', '');
	ASSET_FINDER_OBJ.set_text_field(safe_name + '_label', '');

}// end asset_finder_clear_btn_press()


/**
* Activated by the pressing of the "Reset" button
*
* @param string	$name			the name of the hidden field
* @param string	$safe_name		the name prefix for all the other form elements associated with the
*
* @access public
*/
function asset_finder_reset_btn_press(name, safe_name, assetid, label)
{
	ASSET_FINDER_OBJ.set_hidden_field(name + '[assetid]', assetid);
	ASSET_FINDER_OBJ.set_text_field(safe_name + '_label', label);

}// end asset_finder_reset_btn_press()


/**
* Activated by on an unload event to cancel the asset finder if we are currently looking
*
* @access public
*/
function asset_finder_onunload()
{
	// got a name ? we must be finding assets, cancel it
	if (ASSET_FINDER_FIELD_NAME != null) {
		asset_finder_cancel();
	}
}// end asset_finder_onunload()
ASSET_FINDER_OTHER_ONUNLOAD = (window.onunload) ? window.onunload : new Function;
window.onunload = asset_finder_onunload;


  ////////////////////////////////
 //  Asset Map External Calls  //
////////////////////////////////


/**
* This function works in partnership with the ExternalCall flash class.
* What it allows is the execution of commands inside the flash that
* you are unable to do normally.
*
*/
function jsToFlashCall(swObj, cmd, params)
{
	if (!ASSET_MAP_FLASH_CHECKED) return;

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
				//alert(FLASH_TO_JS_CALL_BACK_FNS[FLASH_TO_JS_CALL.cmd]);

				if (navigator.userAgent.match(/MSIE/)) {
					FLASH_TO_JS_CALL_BACK_FNS[FLASH_TO_JS_CALL.cmd](FLASH_TO_JS_CALL.params);
				} else {
					// because of some wierd thing with Moz/Firebird and flash we need to get this on a separate thread
					// than this call from the flash
					setTimeout(FLASH_TO_JS_CALL_BACK_FNS[FLASH_TO_JS_CALL.cmd], 100, FLASH_TO_JS_CALL.params);
				}
				// reset the storage units
				FLASH_TO_JS_CALL = null;
			}

		break;
	}// end switch
}// end flashToJsCall()


/**
* Makes an extenal call to java
*
* @param object		$asset_mapObj	the java applet
* @param string		$type			the type of request
* @param String		$command		the command 
* @param Array		$params			the params to pass to java
*/
function jsToJavaCall(asset_mapObj, type, command, params)
{
	// it just doesn't get much easier than this
	params = var_serialise(params);
	asset_mapObj.jsToJavaCall(type, command, params);

}//end jsToJavaCall();