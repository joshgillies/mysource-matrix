/**
* +--------------------------------------------------------------------+
* | This MySource Matrix CMS file is Copyright (c) Squiz Pty Ltd       |
* | ABN 77 084 670 600                                                 |
* +--------------------------------------------------------------------+
* | IMPORTANT: Your use of this Software is subject to the terms of    |
* | the Licence provided in the file licence.txt. If you cannot find   |
* | this file please contact Squiz (www.squiz.com.au) so we may provide|
* | you a copy.                                                        |
* +--------------------------------------------------------------------+
*
* $Id: asset_map.js,v 1.33 2013/08/29 23:47:09 lwright Exp $
*
*/


var SQ_REFRESH_ASSETIDS = "";


/**
* Initalises the asset map
*
* Checks the browser flavour and version and make some desicions based on that information
* about whether communication should be stable between the javascript and the asset map
*
* @param
*/
function init_asset_map(codebase, archive, parameters, width, height)
{
	var supported = test_java_map_support();
	if (supported === true) {
		var appletDiv = document.createElement('applet');
		appletDiv.id = 'sq_asset_map';
		appletDiv.setAttribute('width', width);
		appletDiv.setAttribute('height', height);
		appletDiv.setAttribute('code', codebase);
		appletDiv.setAttribute('archive', archive);
		appletDiv.setAttribute('mayscript', 'true');

		var paramNames  = [];
		var paramTopTag = document.createElement('param');
		paramTopTag.setAttribute('name', 'parameter.params');
		var paramTags = [paramTopTag];

		for (x in parameters) {
			paramNames.push(x);
			var thisTag = document.createElement('param');
			thisTag.setAttribute('name', x);
			thisTag.setAttribute('value', parameters[x]);
			paramTags.push(thisTag);
		}

		paramTopTag.setAttribute('value', paramNames.join(','));
		for (var i = 0; i < paramTags.length; i++) {
			appletDiv.appendChild(paramTags[i]);
		}

		document.getElementById('asset_map').appendChild(appletDiv);
	}

}//end init_asset_map()


/**
* Test Java asset map support.
*
* Checks the browser flavour and version and emit an alert if not supported.
* Supported: IE6+, any Chrome, Safari 1.2.1, Gecko 1.4 (includes all Firefox >= 1.0)
*/
function test_java_map_support() {
	var supported = false;
	var version   = null;
	var browser   = navigator.userAgent;
	if (browser.indexOf('Trident/') !== -1) {
		// Trident/ tokens are available in IE8 or higher. We need this because
		// IE11+ will not use the "MSIE" token.
		supported = true;
	} else if (browser.indexOf('MSIE ') !== -1) {
		version = /MSIE ([\d.]+)/.exec(browser);
		if (version && (parseFloat(version[1]) >= 6)) {
			supported = true;
		} else {
			alert(js_translate('ie6_or_above_required'));
		}
	} else if (browser.indexOf('Chrome/') !== -1) {
		// Our support for webkit-based browsers goes further back than
		// Chrome 1, so allow all Chromium-based browsers.
		supported = true;
	} else if (browser.indexOf('AppleWebKit/') !== -1) {
		// Other Webkit browsers. Safari 1.2.1 or equivalent.
		version = /AppleWebKit\/([\d.]+)/.exec(browser);
		if (version && (parseFloat(version[1]) >= 125.1)) {
			supported = true;
		} else {
			alert(tjs_translate('safari121_or_above_required'));
		}
	} else if (browser.indexOf('Gecko/') !== -1) {
		// Other Gecko-based browsers.
		version = /rv:([\d.]+)/.exec(browser);
		if (version && (parseFloat(version[1]) >= 1.4)) {
			supported = true;
		} else {
			alert(js_translate('mozilla14_or_above_required'));
		}
	} else {
		alert(js_translate('using_untested_browser'));
	}

	return supported;
}//end test_java_map_supported()

/**
* Opens a new window with a HIPO job in it
*
*/
function open_hipo(url)
{
	window.focus();
	var popup = window.open(url, 'hipo_job', 'width=650,height=400,scrollbars=1,toolbar=0,menubar=0,location=0,resizable=1');
	popup.focus();

}//end open_hipo()


/**
* Returns the java applet object
*
* @return &object the java applet object
*/
function get_java_applet_object()
{
	return document.sq_asset_map;

}//end get_java_applet_object()


/**
* Add the passed list of assetids to the array of IDs to refresh
*
*/
function reload_assets(assetids)
{
	if (typeof assetids !== 'string') {
		assetids = assetids.join('|');
	}

	if (SQ_REFRESH_ASSETIDS != "") {
		SQ_REFRESH_ASSETIDS += "|";
	}
	SQ_REFRESH_ASSETIDS += assetids;

}//end reload_assets()


/*
* Add the passed assetid to the array of IDs to refresh
*
*/
function reload_asset(assetid)
{
	if (SQ_REFRESH_ASSETIDS != "") {
		SQ_REFRESH_ASSETIDS += "|";
	}
	SQ_REFRESH_ASSETIDS += assetid;

}//end reload_asset()


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
	var params_str = '';
	for (var i = 0; i < params.length; i++) {
		params_str += params[i];
		if (params.length - 1 != i) {
			params_str += '~';
		}
	}
	asset_mapObj.jsToJavaCall(type, command, params_str);

}//end jsToJavaCall();




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
function set_finder(finder)
{
	ASSET_FINDER_OBJ = finder;

}//end set_finder()


/**
* Activated by changing the value of the assetid input box
*
* @param string	$name			the name of the hidden field
* @param string	$safe_name		the name prefix for all the other form elements associated with the
* @param string	$type_codes		the typecodes separated by a pipe
* @param string value			the value of the new assetid
*
* @access public
*/
function asset_finder_assetid_changed(name, safe_name, type_codes, done_fn, value)
{

	ASSET_FINDER_OBJ.set_hidden_field(name + '[assetid]', value);


}//end asset_finder_assetid_changed()



/**
* Activated by the pressing of the "Change" button to start the asset finder mode in the flash menu
*
* @param string	$name			the name of the hidden field
* @param string	$safe_name		the name prefix for all the other form elements associated with the
* @param string	$type_codes		the typecodes separated by a pipe
*
* @access public
*/
function asset_finder_change_btn_press(name, safe_name, type_codes, done_fn)
{
	resizer_frame = window.top.frames['sq_resizer'];

	ASSET_FINDER_DONE_FUNCTION = done_fn;

	if (ASSET_FINDER_FIELD_NAME != null && ASSET_FINDER_FIELD_NAME != name) {
		alert(js_translate('asset_finder_in_use'));
		return;
	}

	// no name ? we must be starting the asset finder
	if (ASSET_FINDER_FIELD_NAME == null) {
		ASSET_FINDER_FIELD_NAME = name;
		ASSET_FINDER_FIELD_SAFE_NAME = safe_name;

		if (resizer_frame) {
			ASSET_FINDER_WAS_HIDDEN = resizer_frame.hidden;
			if (resizer_frame.hidden) {
				resizer_frame.toggleFrame();
			}
		}
		asset_finder_start('asset_finder_done', type_codes);

		ASSET_FINDER_OBJ.set_button_value(ASSET_FINDER_FIELD_SAFE_NAME + '_change_btn', js_translate('cancel'));

	// else we must be cancelling the asset finder
	} else {
		asset_finder_cancel();
		ASSET_FINDER_OBJ.set_button_value(ASSET_FINDER_FIELD_SAFE_NAME + '_change_btn', js_translate('change'));
		ASSET_FINDER_FIELD_NAME = null;
		ASSET_FINDER_FIELD_SAFE_NAME = null;

		if (resizer_frame) {
			if (ASSET_FINDER_WAS_HIDDEN && !resizer_frame.hidden) {
				resizer_frame.toggleFrame();
			}
		}
		ASSET_FINDER_WAS_HIDDEN = null;
	}

}//end asset_finder_change_btn_press()


/**
* Call-back fns that stops the asset finder
*
* @param Array params the params array
* @param string label the label to give this asset
* @param string url the url of this asset
* @param string linkid
* @param string type_code
*
* @access public
*/
function asset_finder_done(params, label, url, linkid, type_code)
{
	resizer_frame = window.top.frames['sq_resizer'];
	if (resizer_frame) {
		if (ASSET_FINDER_WAS_HIDDEN && !resizer_frame.hidden) {
			resizer_frame.toggleFrame();
		}
	}
	ASSET_FINDER_WAS_HIDDEN = null;

	var win = ASSET_FINDER_OBJ.window;
	win.focus();

	if (ASSET_FINDER_FIELD_NAME == null) return;
	var assetid = params;

	// add the last selected link to cookie. Asset Map will expand the tree to that asset
	document.cookie = 'lastSelectedLinkId=' + escape(linkid);
	document.cookie = 'lastSelectedAssetId=' + escape(assetid);

	// we are in java, params is actually the assetid
	if (assetid != -1) {
		ASSET_FINDER_OBJ.set_hidden_field(ASSET_FINDER_FIELD_NAME + '[assetid]', assetid);
		ASSET_FINDER_OBJ.set_hidden_field(ASSET_FINDER_FIELD_NAME + '[url]', url);
		ASSET_FINDER_OBJ.set_hidden_field(ASSET_FINDER_FIELD_NAME + '[linkid]', linkid);
		ASSET_FINDER_OBJ.set_hidden_field(ASSET_FINDER_FIELD_NAME + '[type_code]', type_code);
		ASSET_FINDER_OBJ.set_text_field(ASSET_FINDER_FIELD_SAFE_NAME + '_label', (assetid == 0) ? '' : label);
		ASSET_FINDER_OBJ.set_text_field(ASSET_FINDER_FIELD_SAFE_NAME + '_assetid', (assetid == 0) ? '' : assetid );
	}

	ASSET_FINDER_OBJ.set_button_value(ASSET_FINDER_FIELD_SAFE_NAME + '_change_btn', js_translate('change'));
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
	var params = type_codes.split('|');
	var asset_mapObj = get_java_applet_object();
	jsToJavaCall(asset_mapObj, 'asset_finder', 'assetFinderStarted', params);

}//end asset_finder_start()


/**
* Starts the asset locator
*
* @param String		asset_ids	Lineage asset ids
*
* @access public
*/
function asset_locator_start(asset_ids_and_sorts_orders)
{
	var params = asset_ids_and_sorts_orders.split('~');
	var asset_mapObj = get_java_applet_object();
	jsToJavaCall(asset_mapObj, 'asset_locator', '', params);

}//end asset_finder_start()

/**
* Alerts the asset map that asset finder mode has been canceled
*
* @access public
*/
function asset_finder_cancel() {

	var asset_mapObj = get_java_applet_object();
	params = new Array();
	jsToJavaCall(asset_mapObj, 'asset_finder', 'assetFinderStopped', params);

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
	ASSET_FINDER_OBJ.set_text_field(safe_name + '_assetid', '');

}//end asset_finder_clear_btn_press()


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
	ASSET_FINDER_OBJ.set_text_field(safe_name + '_assetid', assetid);

}//end asset_finder_reset_btn_press()


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

		// reset the asset finder button
		ASSET_FINDER_OBJ.set_button_value(ASSET_FINDER_FIELD_SAFE_NAME + '_change_btn', js_translate('change'));
		ASSET_FINDER_FIELD_NAME = null;
		ASSET_FINDER_FIELD_SAFE_NAME = null;

		if (resizer_frame) {
			if (ASSET_FINDER_WAS_HIDDEN && !resizer_frame.hidden) {
				resizer_frame.toggleFrame();
			}
		}
		ASSET_FINDER_WAS_HIDDEN = null;
	}

}//end asset_finder_onunload()


ASSET_FINDER_OTHER_ONUNLOAD = (window.onunload) ? window.onunload : new Function;
window.onunload = asset_finder_onunload;


/**
* resize the asset map based on the current size of the containing window
*
* @access public
*/
function resizeAssetMap() {
	var frameHeight = document.body.clientHeight;
	var assetMap = document.getElementById('sq_asset_map');
	var newHeight = frameHeight - 70;

	// no point throwing a js error if the assetMap isn't defined (maybe java isn't installed)
	if (assetMap) {
		// negative size = badness
		if (newHeight > 0) {
			assetMap.style.height = newHeight;
		}
	}
}//end resizeAssetMap()

var resizeAssetMapTimeout = null;


/**
* Used as an intermediary to resizeAssetMap() to ensure
* we don't resize the java applet too often
*
* @access public
*/
function resizeAssetMapTrigger() {
	// clear any old resize events
	clearTimeout(resizeAssetMapTimeout);
	// set a new one in the near future
	resizeAssetMapTimeout = setTimeout("resizeAssetMap()", 50);
}//end resizeAssetMapTrigger()

window.onresize = resizeAssetMapTrigger;

