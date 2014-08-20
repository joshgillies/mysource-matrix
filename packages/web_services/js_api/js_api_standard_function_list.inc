<?php
/**
* +--------------------------------------------------------------------+
* | This MySource Matrix CMS file is Copyright (c) Squiz Pty Ltd	   |
* | ACN 084 670 600													   |
* +--------------------------------------------------------------------+
* | IMPORTANT: Your use of this Software is subject to the terms of    |
* | the Licence provided in the file licence.txt. If you cannot find   |
* | this file please contact Squiz (www.squiz.com.au) so we may provide|
* | you a copy.														   |
* +--------------------------------------------------------------------+
*
* $Id: js_api_standard_function_list.inc,v 1.21 2013/08/13 01:30:48 akarelia Exp $
*
*/

/**
* Javascript API Functions
*
* Purpose prints JS file where needed with the function calls that are active
*
*
* @author  Nic Hubbard <nic@zedsaid.com>
* @author  Ash Karelia <akarelia@squiz.com.au>
* @version $Revision: 1.21 $
* @package MySource_Matrix_Packages
* @subpackage web_services
*/

class Js_Api_Standard_Function_List
{

	public static function paintJSFunctions(JS_Api $js_api_asset)
	{
		$api_url = $js_api_asset->getURL();
		$nonce_token = get_unique_token();
		?>

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
*/

/**
* This will create an ajax request
*
* @version $Revision: 0.1
*/
function createRequest()
{
	var request;
	try {
		request = new XMLHttpRequest();
	} catch (trymicrosoft) {
		try {
			request = new ActiveXObject("Msxml2.XMLHTTP");
		} catch (othermicrosoft) {
			try {
				request = new ActiveXObject("Microsoft.XMLHTTP");
			} catch (failed) {
				request = false;
			}//end catch
		}//end catch
	}//end catch

	if (!request) {
		alert('Your browser does not support Ajax');
	}

	return request;

}//end createRequest


/**
* Checks to see if a variable is set
*
* @param string		obj		The variable we check
*
* @version $Revision: 0.1
*/
function isset(obj)
{
	// Check to see if a variable or array item is set
	if (typeof(obj) == 'undefined' || obj == null) {
		return false;
	} else {
		return true;
	}

}//end isset


/**
* Turns JSON into a javascript object
*
* @param string		json			The JSON string to convert
*
* @version $Revision: 0.1
*/
function jsonToObj(json)
{
	// Make the conversion
	// Don't worry, even the creator of JSON says eval is ok here
	var jsonObj = eval('(' + json + ')');

	return jsonObj;

}//end jsonToObj


/**
* Our default callback
*
* @param string		ajaxRequest		The ajax function
* @param string		dataCallback	Callback that happens after success
*
* @version $Revision: 0.1
*/
function success(ajaxRequest, dataCallback)
{
	if (ajaxRequest.readyState == 4) {
		if (ajaxRequest.status == 200) {
			if (ajaxRequest.responseText !== '' && ajaxRequest.responseText !== 'undefined' && ajaxRequest.responseText !== null) {
				var response = jsonToObj(ajaxRequest.responseText);
				// Custom callback
				dataCallback(response);
			}//end if

		}//end if

	}//end if

}//end success


/**
* This will return our api key
*
* @param string		api_key		The api key of our Javascript API Asset
*
* @version $Revision: 0.1
*/
function setApiKey(api_key)
{
	// Make this into a global variable
	window.api_key = api_key;

}//end setApiKey


/**
* Make our ajax request
*
* @param string		url				The url to send to the server
* @param boolean	receive			Should we even use a callback
* @param function	dataCallback	Custom callback function
*
* @version $Revision: 0.1
*/
function makeRequest(url, receive, dataCallback)
{
	//split url to url and parameters
	urlarray = url.split("?");
	// Create an instance of our ajax object
	var ajaxRequest = createRequest();
	// Open request
	ajaxRequest.open('POST', encodeURI(urlarray[0]), true);
	// Should we use a callback?
	if (receive) {
		// Custom callback
		ajaxRequest.onreadystatechange = function() {
			success(ajaxRequest, dataCallback);
		};
	}//end if
	ajaxRequest.setRequestHeader("Content-type", "application/x-www-form-urlencoded");
	var postString = urlarray[1] + '&nonce_token=<?php echo $nonce_token; ?>';
	ajaxRequest.send(encodeURI(postString));

}//end makeRequest


/**
* Recursive helper function to write out all properties of an object
*
* @param object		obj				The JSON object
* @param object		parent			Parent JSON object
*
* @version $Revision: 0.2
*/
function dumpObj(obj, parent) {
	// Go through all the properties of the passed-in object
	for (var i in obj) {
		if (parent) {
			var msg = parent + '.' + i + ' => ' + obj[i] + '<br />';
		} else {
			var msg = i + " => " + obj[i] + "<br>";
		}
		// Write it out
		document.write(msg);
		// Check if we need to go deeper
		if (typeof obj[i] == 'object') {
			// Write opening
			document.write('<div style="padding-left:20px;">');
			if (parent) {
				dumpObj(obj[i], parent + '.' + i);
			} else {
				dumpObj(obj[i], i);
			}
			// Write closing
			document.write('</div>');
		}//end if

	}//end for

}//end dumpObj

		<?php
		if ($js_api_asset->attr('get_general')) {
			?>

/**
* This will return general information about the asset
*
* @param string		asset_id		Id of the asset we are getting info for
* @param boolean	get_attributes	if we are getting non standard attribute values of the assets(FALSE by default)
* @param function	dataCallback	Custom callback function
*
* @version $Revision: 0.1
*/
function getGeneral(asset_id, get_attributes, dataCallback)
{
	// Create blank function
	var dataCallback = typeof(dataCallback) != 'undefined' ? dataCallback : function() {};
	var get_attributes = isset(get_attributes) ? get_attributes : 0;

	// Build our string
	var url = '<?php echo $api_url; ?>' + '?key=' + api_key + '&type=getGeneral&id=' + asset_id + '&get_attributes=' + get_attributes;

	// Make our request
	makeRequest(url, true, dataCallback);

}//end getGeneral


/**
* This will get the Child count for the passed in asset
*
* @param string		asset_id		Id of the asset to get children of
* @param number		level			Number of levels to return, default all
* @param function	dataCallback	Custom callback function
*
*
*/
function getChildCount(asset_id, level, dataCallback)
{
	// Create blank function
	var dataCallback = typeof(dataCallback) != 'undefined' ? dataCallback : function() {};
	// Check to see if we have set any levels
	var level = isset(level) ? level : 0;

	// Build our string
	var url = '<?php echo $api_url; ?>' + '?key=' + api_key + '&type=getChildCount&asset_id=' + asset_id + '&levels=' + level;

	// Make our request
	makeRequest(url, true, dataCallback);

}//end getChildCount()

			<?php
		}// end if(get_general)

		if ($js_api_asset->attr('get_attributes')) {
			?>

/**
* This will return attributes of the specific asset
*
* @param string		asset_id		Id of the asset we are getting info for
* @param function	dataCallback	Custom callback function
*
* @version $Revision: 0.1
*/
function getAttributes(asset_id, dataCallback)
{
	// Create blank function
	var dataCallback = typeof(dataCallback) != 'undefined' ? dataCallback : function() {};

	// Build our string
	var url = '<?php echo $api_url; ?>' + '?key=' + api_key + '&type=getAttributes&id=' + asset_id;

	// Make our request
	makeRequest(url, true, dataCallback);

}//end getAttributes

			<?php
		}// end if(get_attributes)

		if ($js_api_asset->attr('set_attribute')) {
			?>

/**
* This will set an attribute value
*
* @param string		asset_id		Id of the asset we are getting info for
* @param string		attr_name		Name of the attribute to change
* @param string		attr_val		Value to change the attribute to
* @param function	dataCallback	Custom callback function
*
* @version $Revision: 0.1
*/
function setAttribute(asset_id, attr_name, attr_val, dataCallback)
{
	// Create blank function
	var dataCallback = typeof(dataCallback) != 'undefined' ? dataCallback : function() {};

	// Build our string
	var url = '<?php echo $api_url; ?>' + '?key=' + api_key + '&type=setAttribute&id=' + asset_id + '&attr_name=' + attr_name + '&attr_val=' + attr_val.replace(/#/g , "%23").replace(/&/g , "%26").replace(/\?/g , "%3F").replace(/\+/g , "%2B");

	// Make our request
	makeRequest(url, true, dataCallback);

}//end setAttribute


/**
* This will set an multiple attributes value for an Asset
*
* @param string		asset_id		Id of the asset we are getting info for
* @param array		field_info		Attribute name and their respective values to be changed to
* @param function	dataCallback	Custom callback function
*
* @version $Revision: 0.1
*/
function setMultipleAttributes(asset_id, field_info, dataCallback)
{
	// Create blank function
	var dataCallback = typeof(dataCallback) != 'undefined' ? dataCallback : function() {};

	var field_names = '';
	var field_vals = '';
	for (var field_name in field_info) {
		// construct our query strings to be passed
		if (field_name == '') continue;
		field_names = field_names + field_name + '\\,';
		field_vals = field_vals + field_info[field_name].replace(/&/g , "%26").replace(/#/g , "%23").replace(/\?/g , "%3F").replace(/\+/g , "%2B") + '\\,' ;
	}

	// remove the trailing "\,"
	field_names = field_names.substring(0, field_names.length-2);
	field_vals = field_vals.substring(0,field_vals.length-2);

	// Build our string
	var url = '<?php echo $api_url; ?>' + '?key=' + api_key + '&type=setMultipleAttributes&id=' + asset_id + '&attr_name=' + field_names + '&attr_val=' + field_vals;

	// Make our request
	makeRequest(url, true, dataCallback);

}//end setMultipleAttributes

			<?php
		}// end if(set_attribute)

		if ($js_api_asset->attr('get_metadata')) {
			?>

/**
* This will return a metadata value for the passed metadata name
*
* @param string		asset_id		Id of the asset we are getting info for
* @param function	dataCallback	Custom callback function
*
* @version $Revision: 0.1
*/
function getMetadata(asset_id, dataCallback)
{
	// Create blank function
	var dataCallback = typeof(dataCallback) != 'undefined' ? dataCallback : function() {};

	// Build our string
	var url = '<?php echo $api_url; ?>' + '?key=' + api_key + '&type=getMetadata&id=' + asset_id;

	// Make our request
	makeRequest(url, true, dataCallback);

}//end getMetadata

			<?php
		}// end if(get_metadata)

		if ($js_api_asset->attr('set_metadata')) {
			?>

/**
* This will set a metadata value
*
* @param string		asset_id		Id of the asset we are getting info for
* @param string		field_id		Id of the metadata field we want to set for an asset
* @param string		$field_val		Value that needs to be set for metadata field
* @param function	dataCallback	Custom callback function
*
* @version $Revision: 0.1
*/
function setMetadata(asset_id, field_id, field_val, dataCallback)
{
	// Create blank function
	var dataCallback = typeof(dataCallback) != 'undefined' ? dataCallback : function() {};

	// Build our string
	var url = '<?php echo $api_url; ?>' + '?key=' + api_key + '&type=setMetadata&id=' + asset_id + '&field_id=' + field_id + '&field_val=' + field_val.replace(/#/g , "%23").replace(/&/g , "%26").replace(/\?/g , "%3F").replace(/\+/g , "%2B");

	// Make our request
	makeRequest(url, true, dataCallback);

}//end setMetadata


/**
* Set metadata values of multiple fields for an asset
*
* @param string		asset_id		Id of asset to set metadata for
* @param array 		field_info		Field Ids and their values
* @param function	dataCallback	Custom callback function
*
* @version $Revision: 0.1
*/
function setMetadataAllFields(asset_id, field_info, dataCallback)
{
	// Create blank function
	var dataCallback = typeof(dataCallback) != 'undefined' ? dataCallback : function() {};

	var field_ids = '';
	var field_vals = '';
	for (var field_id in field_info) {
		// construct our query strings to be passed
		field_ids = field_ids + field_id + '\\,';
		field_vals = field_vals + field_info[field_id].replace(/&/g , "%26").replace(/#/g , "%23").replace(/\?/g , "%3F").replace(/\+/g , "%2B") + '\\,' ;
	}

	// remove the trailing "\,"
	field_ids = field_ids.substring(0, field_ids.length-2);
	field_vals = field_vals.substring(0,field_vals.length-2);

	// Build our string
	var url = '<?php echo $api_url; ?>' + '?key=' + api_key + '&type=setMetadata&id=' + asset_id + '&field_id=' + field_ids + '&field_val=' + field_vals;

	// Make our request
	makeRequest(url, true, dataCallback);

}//end setMetadataAllFields()

			<?php
		}// end if(set_metadata)

		if ($js_api_asset->attr('trash_asset')) {
			?>

/**
* This will send an asset to the trash
*
* @param array | string		asset_ids		Id of the asset(s) to delete
* @param function			dataCallback	Custom callback function
*
* @version $Revision: 0.1
*/
function trashAsset(asset_ids, dataCallback)
{
	if (typeof(asset_ids) != 'string') {
		// Create blank function
		var dataCallback = typeof(dataCallback) != 'undefined' ? dataCallback : function() {};
		var ids = '';

		for (var index in asset_ids) {
			// construct our query strings to be passed
			ids = ids + asset_ids[index] + '\\,';
		}

		// remove the trailing "\,"
		ids = ids.substring(0, ids.length-2);
	} else {
		ids = asset_ids;
	}

	// Build our string
	var url = '<?php echo $api_url; ?>' + '?key=' + api_key + '&type=trashAsset&assetid=' + ids;

	// Make our request
	makeRequest(url, true, dataCallback);

}//end trashAsset

			<?php
		}// end if (trash_asset)

		if ($js_api_asset->attr('get_children')) {
			?>

/**
* This will return child asset ids of the passed asset
*
* @param string		asset_id		Id of the asset to get children of
* @param number		levels			Number of levels to return
* @param array		type_codes		asset type_code that we want back
* @param array		link_type		type of links we are looking for
*									Valid Values - SQ_LINK_TYPE_1, SQ_LINK_TYPE_2, SQ_LINK_TYPE_3, SQ_LINK_NOTICE
* @param array		link_values		link values allowed on the asset returned
* @param boolean	get_attributes	if we are getting non standard attribute values of the assets(FALSE by default)
* @param function	dataCallback	Custom callback function
*
*
* NOTE : type_code, link_type and link_values are empty by default. So matrix will return everything
*		 also these options work for direct links only i.e. levels = 1
*
* @version $Revision: 0.2
*
*/
function getChildren(asset_id, levels, type_codes, link_types, link_values, get_attributes, dataCallback)
{
	// Create blank function
	var dataCallback = typeof(dataCallback) != 'undefined' ? dataCallback : function() {};
	var type_code = '';
	if(isset(type_codes) && type_codes != '') {
		if(typeof(type_codes) == 'object' ) {
			for (var idx in type_codes) {
				type_code = type_code + type_codes[idx] + '\\,';
			}
			type_code = type_code.substring(0, type_code.length-2);
		} else {
			type_code = type_codes;
		}
	}

	var link_type = '';
	if(isset(link_types) && link_types != '') {
		if(typeof(link_types) == 'object' ) {
			for (var idx in link_types) {
				link_type = link_type + link_types[idx] + '\\,';
			}
			link_type = link_type.substring(0, link_type.length-2)
		} else {
			link_type = link_types;
		}
	}

	var link_value = '';
	if(isset(link_values) && link_values != '') {
		if(typeof(link_values) == 'object' ) {
			for (var idx in link_values) {
				link_value = link_value + link_values[idx] + '\\,';
			}
			link_value = link_value.substring(0, link_value.length-2);
		} else {
			link_value = link_values;
		}
	}

	// Check to see if we have set any levels
	var levels = typeof(levels) != 'undefined' ? levels : 0;
	var get_attributes = isset(get_attributes) ? get_attributes : 0;

	// Build our string
	var url = '<?php echo $api_url; ?>' +
				'?key=' + api_key +
				'&type=getChildren&id=' + asset_id +
				'&levels=' + levels +
				'&type_codes=' + type_code +
				'&link_types=' + link_type +
				'&link_values=' + link_value +
				'&get_attributes=' + get_attributes;

	// Make our request
	makeRequest(url, true, dataCallback);

}//end getChildren

			<?php
		}//end if (get_children)

		if ($js_api_asset->attr('get_parents')) {
			?>

/**
* This return parents of the passed id
*
* @param string		asset_id		Id of the asset to get parents of
* @param number		levels			Number of levels to return
* @param array		type_codes		asset type_code that we want back
* @param array		link_type		type of links we are looking for
*									Valid Values - SQ_LINK_TYPE_1, SQ_LINK_TYPE_2, SQ_LINK_TYPE_3, SQ_LINK_NOTICE
* @param array		link_values		link values allowed on the asset returned
* @param boolean	get_attributes	if we are getting non standard attribute values of the assets(FALSE by default)
* @param function	dataCallback	Custom callback function
*
*
* NOTE : type_code, link_type and link_values are empty by default. So matrix will return everything
*		 also these options work for direct links only i.e. levels = 1
*
* @version $Revision: 0.2
*
*/
function getParents(asset_id, levels, type_codes, link_types, link_values, get_attributes, dataCallback)
{
	// Create blank function
	var dataCallback = typeof(dataCallback) != 'undefined' ? dataCallback : function() {};
	var type_code = '';
	if(isset(type_codes) && type_codes != '') {
		if(typeof(type_codes) == 'object' ) {
			for (var idx in type_codes) {
				type_code = type_code + type_codes[idx] + '\\,';
			}
			type_code = type_code.substring(0, type_code.length-2);
		} else {
			type_code = type_codes;
		}
	}

	var link_type = '';
	if(isset(link_types) && link_types != '') {
		if(typeof(link_types) == 'object' ) {
			for (var idx in link_types) {
				link_type = link_type + link_types[idx] + '\\,';
			}
			link_type = link_type.substring(0, link_type.length-2);
		} else {
			link_type = link_types;
		}
	}

	var link_value = '';
	if(isset(link_values) && link_values != '') {
		if(typeof(link_values) == 'object' ) {
			for (var idx in link_values) {
				link_value = link_value + link_values[idx] + '\\,';
			}
			link_value = link_value.substring(0, link_value.length-2);
		} else {
			link_value = link_values;
		}
	}

	// Check to see if we have set any levels
	var levels = typeof(levels) != 'undefined' ? levels : 0;
	var get_attributes = isset(get_attributes) ? get_attributes : 0;

	// Build our string
	var url = '<?php echo $api_url; ?>' + '?key=' + api_key +
				'&type=getParents&id=' + asset_id +
				'&levels=' + levels +
				'&type_codes=' + type_code +
				'&link_types=' + link_type +
				'&link_values=' + link_value +
				'&get_attributes=' + get_attributes;


	// Make our request
	makeRequest(url, true, dataCallback);

}//end getParents

			<?php
		}// end if(get_parents)

		if ($js_api_asset->attr('get_permissions')) {
			?>

/**
* This returns permissions for an asset
*
* @param string		asset_id		Id of the asset to get permissions for
* @param string		level			1=READ 2=WRITE 3=ADMIN, default to READ
* @param function	dataCallback	Custom callback function
*
* @version $Revision: 0.2
*/
function getPermissions(asset_id, level, dataCallback)
{
	// Create blank function
	var dataCallback = typeof(dataCallback) != 'undefined' ? dataCallback : function() {};
	var level = (isset(level) && level != '') ? level : 1 ;

	// Build our string
	var url = '<?php echo $api_url; ?>' + '?key=' + api_key + '&type=getPermissions&id=' + asset_id + '&level=' + level;

	// Make our request
	makeRequest(url, true, dataCallback);

}//end getPermissions

			<?php
		}//

		if ($js_api_asset->attr('create_asset')) {
			?>

/**
* Creates an asset
*
* @param integer	parent_id			Parentid of the new parent
* @param string		type_code			Type code of new asset
* @param string		asset_name			Name for new asset
* @param integer	link_type			Type of link to create
* @param string		link_value			Value of the link
* @param integer	sort_order			Order in the tree
* @param integer	is_dependant		Dependant to parent
* @param integer	is_exclusive		Exclusive to parent
* @param integer	extra_attributes	Allows additional attributes
* @param string		attributes			String of additional query string containing key/pair values
*
* @version $Revision: 0.2
*/
function createAsset(parent_id, type_code, asset_name, link_type, link_value, sort_order, is_dependant, is_exclusive, extra_attributes, attributes, dataCallback)
{
	// Create blank function
	var dataCallback = typeof(dataCallback) != 'undefined' ? dataCallback : function() {};

	// Check to see if we have set values
	if (!isset(link_type)) link_type = '';
	if (!isset(link_value)) link_value = '';
	if (!isset(sort_order)) sort_order = '';
	if (!isset(is_dependant)) is_dependant = '';
	if (!isset(is_exclusive)) is_exclusive = '';
	if (!isset(extra_attributes)) extra_attributes = '';
	if (!isset(attributes)) attributes = '';

	// Build our string
	var url =	'<?php echo $api_url; ?>' + '?key=' + api_key + '&type=createAsset' +
				'&id=' + parent_id +
				'&type_code=' + type_code +
				'&asset_name=' + asset_name.replace(/&/g , "%26").replace(/#/g , "%23").replace(/\?/g , "%3F").replace(/\+/g , "%2B") +
				'&link_type=' + link_type +
				'&link_value=' + link_value.replace(/&/g , "%26").replace(/#/g , "%23").replace(/\?/g , "%3F").replace(/\+/g , "%2B") +
				'&sort_order=' + sort_order +
				'&is_dependant=' + is_dependant +
				'&is_exclusive=' + is_exclusive +
				'&extra_attributes=' + extra_attributes +
				'&' + attributes;

	// Make our request
	makeRequest(url, true, dataCallback);

}//end createAsset


/**
* This function will create file type asset of the type_code provided
*
* @param	string		parentID		asset ID of parent where the new asset will be linked to
* @param	string		type_code		type_code of the new asset(default to 'file')
* @param	string		friendly_name	friendly name of the new asset to be created
* @param	string		link_type		type of link to create to the parent for this new asset(SQ_LINK_TYPE_1 link by default)
*										Valid Values - SQ_LINK_TYPE_1, SQ_LINK_TYPE_2, SQ_LINK_TYPE_3, SQ_LINK_NOTICE
* @param	string		link_value		value of link to create to the parent for this new asset('' by default)
* @param	function 	dataCallback	Custom callback function
*
*
*/
function createFileAsset(parentID, type_code, friendly_name, link_type, link_value, dataCallback)
{
	// Create blank function
	var dataCallback = typeof(dataCallback) != 'undefined' ? dataCallback : function() {};
	var type_code = (isset(type_code) && type_code != '' ) ? type_code : 'file';
	var friendly_name  = isset(friendly_name) ? friendly_name : '';
	var link_type = (isset(link_type) && link_type != '') ? link_type : SQ_LINK_TYPE_1;
	var link_value = isset(link_value) ? link_value : '';

	var url = '<?php echo $api_url; ?>' +
				'?key=' + api_key +
				'&type=createFileAsset&id=' + parentID +
				'&type_code=' + type_code +
				'&friendly_name=' + friendly_name +
				'&link_type=' + link_type +
				'&link_value=' + link_value;

	// Make our request
	makeRequest(url, true, dataCallback);


}//end createFileAsset()

			<?php
		}//end if(create_asset)

		if ($js_api_asset->attr('get_asset_types')) {
			?>

/**
* Returns asset type codes installed on the system
*
* @param function	dataCallback	Custom callback function
*
* @version $Revision: 0.2
*/
function getAssetTypes(dataCallback)
{
	// Create blank function
	var dataCallback = typeof(dataCallback) != 'undefined' ? dataCallback : function() {};

	// Build our string
	var url = '<?php echo $api_url; ?>' + '?key=' + api_key + '&type=getAssetTypes&id=1';

	// Make our request
	makeRequest(url, true, dataCallback);

}//end getAssetTypes

			<?php
		}// end if(get_asset_types)

		if ($js_api_asset->attr('get_locks_info')) {
			?>

/**
* Gets information for lock type passed
*
* @param string		asset_id		Id of the asset to get locks for
* @param string		screen_name		The screen to get locks for
* @param function	dataCallback	Custom callback function
*
*
*/
function getLocksInfo(asset_id, screen_name, dataCallback)
{
	// Create blank function
	var dataCallback = typeof(dataCallback) != 'undefined' ? dataCallback : function() {};

	// If the user does not set it, we get all locks
	if (!isset(screen_name) || screen_name == '') screen_name = 'all';

	// Build our string
	var url = '<?php echo $api_url; ?>' + '?key=' + api_key + '&type=getLocksInfo&id=' + asset_id + '&screen=' + screen_name;

	// Make our request
	makeRequest(url, true, dataCallback);

}//end getLocksInfo()

			<?php
		}//end if(get_locks_info)

		if ($js_api_asset->attr('acquire_lock')) {
			?>
/**
* Acquires locks on the given asset. if the locks were already acquired
* by current user then they are updated
*
* @param string		asset_id		Id of the asset to get locks for
* @param string		screen_name		The screen to get locks for
* @param boolean 	dependants_only	whether dependants only or all children, defaults to true
* @param boolean 	force_acquire	whether to attempt to forceably acquire the lock or not,  defaults to false
* @param function	dataCallback	Custom callback function
*
* @version $Revision: 0.2
*/
function acquireLock(asset_id, screen_name, dependants_only, force_acquire, dataCallback)
{
	var dependants_only = isset(dependants_only) && ( dependants_only.toString().toLowerCase() != 'null' && dependants_only != '' ) ? dependants_only : 1;
	var force_acquire = isset(force_acquire) && ( force_acquire.toString().toLowerCase() != 'null' && force_acquire != '' ) ? force_acquire : 0;

	// Create blank function
	var dataCallback = typeof(dataCallback) != 'undefined' ? dataCallback : function() {};

	// we cannot pass TRUE or FALSE as string so lets do our conversion
	// but try conversion only if its a string still
	if (typeof(dependants_only) == 'string') {
		if (dependants_only.toLowerCase() == 'false') {
			dependants_only = 0;
		} else if (dependants_only.toLowerCase() == 'true') {
			dependants_only = 1;
		}
	}

	if (typeof(force_acquire) == 'string') {
		if (force_acquire.toLowerCase() == 'false') {
			force_acquire = 0;
		} else if (force_acquire.toLowerCase() == 'true') {
			force_acquire = 1;
		}
	}

	// If the user does not set it, we get all locks
	if (!isset(screen_name) || screen_name == '') screen_name = 'all';

	// Build our string
	var url = '<?php echo $api_url; ?>' + '?key=' + api_key + '&type=acquireLock&id=' + asset_id + '&screen=' + screen_name + '&dependants_only=' + dependants_only + '&force_acquire=' + force_acquire;

	// Make our request
	makeRequest(url, true, dataCallback);

}//end acquireLock

			<?php
		}//end if(acquire_locks)

		if ($js_api_asset->attr('release_lock')) {
			?>


/**
* Releases a lock
*
* @param string		asset_id		Id of the asset to release locks for
* @param string		screen_name		The screen to release locks for
* @param function	dataCallback	Custom callback function
*
* @version $Revision: 0.2
*/
function releaseLock(asset_id, screen_name, dataCallback)
{
	// Create blank function
	var dataCallback = typeof(dataCallback) != 'undefined' ? dataCallback : function() {};

	// If the user does not set it, we get all locks
	if (!isset(screen_name) || screen_name == '') screen_name = 'all';

	// Build our string
	var url = '<?php echo $api_url; ?>' + '?key=' + api_key + '&type=releaseLock&id=' + asset_id + '&screen=' + screen_name;

	// Make our request
	makeRequest(url, true, dataCallback);

}//end releaseLock

			<?php
		}//end if(release_locks)

		if ($js_api_asset->attr('create_link')) {
			?>

/**
* Creates a link between two assets with passed information
*
* @param integer	parent_id		Major asset id we are linking from
* @param integer	child_id		Minor asset id we are linking to
* @param integer	link_type		Type of link to create
* @param string		link_value		Value of the link
* @param integer	sort_order		Order in the tree
* @param integer	is_dependant	Dependant to parent
* @param integer	is_exclusive	Exclusive to parent
*
* @version $Revision: 0.2
*/
function createLink(parent_id, child_id, link_type, link_value, sort_order, is_dependant, is_exclusive, dataCallback)
{
	// Create blank function
	var dataCallback = typeof(dataCallback) != 'undefined' ? dataCallback : function() {};

	// Check to see if we have set values
	if (!isset(link_type)) link_type = '1';
	if (!isset(link_value)) link_value = '';
	if (!isset(sort_order)) sort_order = '';
	if (!isset(is_dependant)) is_dependant = '';
	if (!isset(is_exclusive)) is_exclusive = '';

	// Build our string
	var url =	'<?php echo $api_url; ?>' + '?key=' + api_key + '&type=createLink' +
				'&id=' + child_id +
				'&parent_id=' + parent_id +
				'&link_type=' + link_type +
				'&link_value=' + link_value +
				'&sort_order=' + sort_order +
				'&is_dependant=' + is_dependant +
				'&is_exclusive=' + is_exclusive;

	// Make our request
	makeRequest(url, true, dataCallback);

}//end createLink

			<?php
		}//end if(create_link)

		if ($js_api_asset->attr('remove_link')) {
			?>

/**
* Removes a link between a parent and child
*
* @param string		parent_id		Id of the parent
* @param string		child_id		Id of the child
* @param string     link_type       type of link we are looking for (SQ_LINK_TYPE_1 link by default)
									Valid Values - SQ_LINK_TYPE_1, SQ_LINK_TYPE_2, SQ_LINK_TYPE_3, SQ_LINK_NOTICE
* @param string     link_value      value of link we are looking for ('' by default)
* @param function	dataCallback	Custom callback function
*
* @version $Revision: 0.2
*/
function removeLink(parent_id, child_id, link_type, link_value, dataCallback)
{
	// Create blank function
	var dataCallback = typeof(dataCallback) != 'undefined' ? dataCallback : function() {};
	var link_type = (isset(link_type) && link_type != '' ) ? link_type.toUpperCase() : 'SQ_LINK_TYPE_1';
	var link_value = isset(link_value) ? link_value : '';

	// Build our string
	var url = '<?php echo $api_url; ?>' + '?key=' + api_key + '&type=removeLink&id=' + child_id + '&parent_id=' + parent_id + '&link_type=' + link_type + '&link_value=' + link_value.replace(/&/g , "%26").replace(/#/g , "%23").replace(/\?/g , "%3F").replace(/\+/g , "%2B");

	// Make our request
	makeRequest(url, true, dataCallback);

}//end removeLink


/**
* Removes multiple links between parent and child pairs
*
* @param json object	link_info		array of link_info
* example :
* 		var link_info = {
* 			"links":[
* 				{
* 					"parent": parent,
* 					"child": child,
* 					"link_type": link_type,
*					"link_value": link_value,
* 				},
* 				{
* 					"parent": parent_2,
* 					"child": child_2,
* 					"link_type": link_type_2,
*					"link_value": link_value,
* 				},
* 				{
* 					"parent": parent_3,
* 					"child": child_3,
* 					"link_type": link_type_3,
*					"link_value": link_value,
* 				},
* 			]
* 		 };
*
* values in JSON object
* 		string   	  	parent			Id of the parent
* 		string    		child			Id of the child
* 		string    		link_type		type of link we are looking for(SQ_LINK_TYPE_1 link by default)
*										Valid Values - SQ_LINK_TYPE_1, SQ_LINK_TYPE_2, SQ_LINK_TYPE_3, SQ_LINK_NOTICE
* 		string 	    	link_value		value of link we are looking for ('' by default)
* @param function		dataCallback	Custom callback function
*
*/
function removeMultipleLinks(link_info, dataCallback)
{
	// Create blank function
	var dataCallback = typeof(dataCallback) != 'undefined' ? dataCallback : function() {};
	var parentid = '';
	var childid = '';
	var link_type = '';
	var link_value = '';

	for (var x = 0; x < link_info.links.length; x++) {
		link_typ = isset(link_info.links[x].link_type) ?  link_info.links[x].link_type.toUpperCase() : 'SQ_LINK_TYPE_1' ;
		link_val = isset(link_info.links[x].link_value) ?  link_info.links[x].link_value : '' ;

		parentid = parentid + link_info.links[x].parent + '\\,';
		childid = childid + link_info.links[x].child + '\\,';
		link_type = link_type + link_typ + '\\,';
		link_value = link_value + link_val.replace(/&/g , "%26").replace(/#/g , "%23").replace(/\?/g , "%3F").replace(/\+/g , "%2B") + '\\,';
	}

	// Build our string
	var url = '<?php echo $api_url; ?>' + '?key=' + api_key + '&type=removeMultipleLinks&parent_id=' + parentid.substring(0, parentid.length-2) +
				'&child_id=' + childid.substring(0, childid.length-2) +
				'&link_type=' + link_type.substring(0, link_type.length-2) +
				'&link_value=' + link_value.substring(0, link_value.length-2)

	// Make our request
	makeRequest(url, true, dataCallback);

}//end removeMultipleLinks

			<?php
		}//end if(remove_links)

		if ($js_api_asset->attr('move_link')) {
			?>

/**
* Moves a link from one parent to another
*
* @param string		old_parent_id			Id of the old parent
* @param string		child_id				Id of the child
* @param string		old_link_type			Type of link we are searching for between given assets (SQ_LINK_TYPE_1 link by default)
*											Valid Values - SQ_LINK_TYPE_1, SQ_LINK_TYPE_2, SQ_LINK_TYPE_3
* @param string		old_link_value			Value of link we are searching for between given assets ('' by default)
* @param string		new_parent_id			Id of the new parent
* @param string		new_link_type			Type of link to use (SQ_LINK_TYPE_1 link by default)
* 											Valid Values - SQ_LINK_TYPE_1, SQ_LINK_TYPE_2, SQ_LINK_TYPE_3
* @param string		new_link_value			Value of link to use ('' by default)
* @param string		new_position			The new position
* @param function	dataCallback			Custom callback function
*
* @version $Revision: 0.2
*/
function moveLink(old_parent_id, child_id, old_link_type, old_link_value, new_parent_id, new_link_type, new_link_value, new_position, dataCallback)
{
	// Create blank function
	var dataCallback = typeof(dataCallback) != 'undefined' ? dataCallback : function() {};

	// Check to see if we have set values
	var old_link_type = (isset(old_link_type) && old_link_type != '' ) ? old_link_type.toUpperCase() : 'SQ_LINK_TYPE_1';
	var old_link_value = isset(old_link_value) ? old_link_value.replace(/&/g , "%26").replace(/#/g , "%23").replace(/\?/g , "%3F").replace(/\+/g , "%2B") : '';

	var new_link_type = (isset(new_link_type) && new_link_type != '' ) ? new_link_type.toUpperCase() : 'SQ_LINK_TYPE_1';
	var new_link_value = isset(new_link_value) ? new_link_value.replace(/&/g , "%26").replace(/#/g , "%23").replace(/\?/g , "%3F").replace(/\+/g , "%2B") : '';
	new_position      = (isset(new_position) && new_position != '') ? new_position : '0';

	// Build our string
	var url =	'<?php echo $api_url; ?>' + '?key=' + api_key + '&type=moveLink' +
				'&id=' + child_id +
				'&old_parent_id=' + old_parent_id  +
				'&old_link_type=' + old_link_type  +
				'&old_link_value='+ old_link_value +
				'&new_parent_id=' + new_parent_id  +
				'&new_link_type=' + new_link_type  +
				'&new_link_value='+ new_link_value +
				'&new_position='  + new_position;

	// Make our request
	makeRequest(url, true, dataCallback);

}//end moveLink

			<?php
		}//end if(move_link)

		if ($js_api_asset->attr('update_link')) {
			?>

/**
* Updates a link with provided information
*
* @param string		parent_id				id of the parent
* @param string		child_id				id of the child
* @param string		existing_link_type		existing link type betweent the assets (SQ_LINK_TYPE_1 link by default)
*											Valid Values - SQ_LINK_TYPE_1, SQ_LINK_TYPE_2, SQ_LINK_TYPE_3, SQ_LINK_NOTICE
* @param string		existing_link_value		existing link value betweent the assets ('' by default)
* @param string		link_type				link type to be updated to (SQ_LINK_TYPE_1 link by default)
*											Valid Values - SQ_LINK_TYPE_1, SQ_LINK_TYPE_2, SQ_LINK_TYPE_3, SQ_LINK_NOTICE
* @param string		link_value				link value to be updated to ('' by default)
* @param string		new_position			the new position
* @param string		locked					the asset link lock status(locked by default)
* @param function	dataCallback			custom callback function
*
* @version $Revision: 0.2
*/
function updateLink(parent_id, child_id, existing_link_type, existing_link_value, link_type, link_value, sort_order, locked, dataCallback)
{
	// Create blank function
	var dataCallback = typeof(dataCallback) != 'undefined' ? dataCallback : function() {};

	// Check to see if we have set values
	existing_link_type = (isset(existing_link_type) && existing_link_type != '' ) ? existing_link_type.toUpperCase() : 'SQ_LINK_TYPE_1';
	existing_link_value= isset(existing_link_value) ? existing_link_value : '';

	link_type = (isset(link_type) && link_type != '' ) ? link_type.toUpperCase() : 'SQ_LINK_TYPE_1';
	link_value= isset(link_value) ? link_value : '';

	sort_order= isset(sort_order) ? sort_order : '';
	locked    = isset(locked) ? locked : '';

	// Build our string
	var url =	'<?php echo $api_url; ?>' + '?key=' + api_key + '&type=updateLink' +
				'&id=' 					+ child_id +
				'&parent_id=' 			+ parent_id +
				'&existing_link_type='	+ existing_link_type +
				'&existing_link_value='	+ existing_link_value.replace(/&/g , "%26").replace(/#/g , "%23").replace(/\?/g , "%3F").replace(/\+/g , "%2B") +
				'&link_type=' 			+ link_type +
				'&link_value=' 			+ link_value.replace(/&/g , "%26").replace(/#/g , "%23").replace(/\?/g , "%3F").replace(/\+/g , "%2B") +
				'&sort_order=' 			+ sort_order +
				'&locked=' 				+ locked;

	// Make our request
	makeRequest(url, true, dataCallback);

}//end updateLink


/**
* Updates multiple links
*
* @param json object	link_info		Array of all the link information
* example :
* 		var link_info = {
* 			"links":[
* 				{
* 					"parent": parent,
* 					"child": child,
*					"existing_link_type": ext_link_type,
*					"existing_link_value": ext_link_value,
* 					"link_type": link_type,
* 					"link_value": link_value,
* 					"sort_order": sort_order,
* 					"link_lock": link_lock,
* 				},
* 				{
* 					"parent": parent_2,
* 					"child": child_2,
*					"existing_link_type": ext_link_type_2,
*					"existing_link_value": ext_link_value_2,
* 					"link_type": link_type_2,
* 					"link_value": link_value_2,
* 					"sort_order": sort_order_2,
* 					"link_lock": link_lock_2,
* 				},
* 				{
* 					"parent": parent_3,
* 					"child": child_3,
*					"existing_link_type": ext_link_type_3,
*					"existing_link_value": ext_link_value_3,
* 					"link_type": link_type_3,
* 					"link_value": link_value_3,
* 					"sort_order": sort_order_3,
* 					"link_lock": link_lock_3,
* 				},
* 			]
* 		 };
*
* values in JSON object
*       string          parent				Id of the parent
*       string          child				Id of the child
*       string          existing_link_type	type of link we are looking for(SQ_LINK_TYPE_1 link by default)
*											Valid Values - SQ_LINK_TYPE_1, SQ_LINK_TYPE_2, SQ_LINK_TYPE_3, SQ_LINK_NOTICE
*       string          existing_link_value	value of link we are looking for ('' by default)
*       string          link_type			type of link we changing to (SQ_LINK_TYPE_1 link by default)
*											Valid Values - SQ_LINK_TYPE_1, SQ_LINK_TYPE_2, SQ_LINK_TYPE_3, SQ_LINK_NOTICE
*       string          link_value			value of link we are changing it to ('' by default)
*		string			sort_order			sort order we want to update the link to
*		boolean			link_lock			link lock value we want to set it to(default to TRUE)
*
* @param function		dataCallback	Custom callback function
*
* @version $Revision: 0.2
*/
function updateMultipleLinks(link_info, dataCallback)
{
	// Create blank function
	var dataCallback = typeof(dataCallback) != 'undefined' ? dataCallback : function() {};
	var parentid = '';
	var childid = '';
	var existing_link_type = '';
	var existing_link_value= '';
	var link_type = '';
	var link_value = '';
	var sort_order = '';
	var locked = '';

	for (var x = 0; x < link_info.links.length; x++) {
		existing_link_typ	= (isset(link_info.links[x].existing_link_type) && link_info.links[x].existing_link_type != '' ) ?  link_info.links[x].existing_link_type.toUpperCase() : 'SQ_LINK_TYPE_1' ;
		existing_link_val	= isset(link_info.links[x].existing_link_value) ?  link_info.links[x].existing_link_value : '' ;
		link_typ			= isset(link_info.links[x].link_type) ?  link_info.links[x].link_type.toUpperCase() : 'SQ_LINK_TYPE_1' ;
		link_val			= isset(link_info.links[x].link_value) ?  link_info.links[x].link_value : '' ;
		sort_ord			= isset(link_info.links[x].sort_order) ?  link_info.links[x].sort_order : '' ;
		lock				= isset(link_info.links[x].link_lock) ?  link_info.links[x].link_lock : '' ;

		parentid			= parentid + link_info.links[x].parent + '\\,';
		childid				= childid + link_info.links[x].child + '\\,';
		existing_link_type	= existing_link_type + existing_link_typ +'\\,';
		existing_link_value = existing_link_value + existing_link_val.replace(/&/g , "%26").replace(/#/g , "%23").replace(/\?/g , "%3F").replace(/\+/g , "%2B") +'\\,';
		link_type			= link_type + link_typ + '\\,';
		link_value			= link_value + link_val.replace(/&/g , "%26").replace(/#/g , "%23").replace(/\?/g , "%3F").replace(/\+/g , "%2B") + '\\,';
		sort_order			= sort_order + sort_ord + '\\,';
		locked				= locked + lock + '\\,';
	}

	// Build our string
	var url =	'<?php echo $api_url; ?>' + '?key=' + api_key + '&type=updateMultipleLinks' +
				'&parent_id=' + parentid.substring(0, parentid.length-2) +
				'&child_id=' + childid.substring(0, childid.length-2) +
				'&existing_link_type=' + existing_link_type.substring(0, existing_link_type.length-2) +
				'&existing_link_value=' + existing_link_value.substring(0, existing_link_value.length-2) +
				'&link_type=' + link_type.substring(0, link_type.length-2) +
				'&link_value=' + link_value.substring(0, link_value.length-2) +
				'&sort_order=' + sort_order.substring(0, sort_order.length-2) +
				'&locked=' + locked.substring(0, locked.length-2);

	// Make our request
	makeRequest(url, true, dataCallback);

}//end updateMultipleLinks()

			<?php
		}//end if(update_link)

		if ($js_api_asset->attr('get_link_id')) {
			?>

/**
* Returns the link id between a parent and child
*
* @param string		parent_id		Id of the parent
* @param string		child_id		Id of the child
* @param string		link_type		type of link we are looking for(SQ_LINK_TYPE_1 link by default)
									Valid Values - SQ_LINK_TYPE_1, SQ_LINK_TYPE_2, SQ_LINK_TYPE_3, SQ_LINK_NOTICE
* @param string		link_value		value of link we are looking for ('' by default)
* @param boolean	all_info		if we want all the link information or just linkid
* @param function	dataCallback	Custom callback function
*
* @version $Revision: 0.2
*/
function getLinkId(parent_id, child_id, link_type, link_value, all_info, dataCallback)
{
	// Create blank function
	var dataCallback = typeof(dataCallback) != 'undefined' ? dataCallback : function() {};
	var all_info = isset(all_info) ? all_info : 0;
	var link_type = (isset(link_type) && link_type != '' ) ? link_type.toUpperCase() : 'SQ_LINK_TYPE_1';
	var link_value = isset(link_value) ? link_value	: '';

	// Build our string
	var url = '<?php echo $api_url; ?>' + '?key=' + api_key + '&type=getLinkId&id=' + child_id + '&parent_id=' + parent_id + '&all_info=' + all_info + '&link_type=' + link_type + '&link_value=' + link_value.replace(/&/g , "%26").replace(/#/g , "%23").replace(/\?/g , "%3F").replace(/\+/g , "%2B");

	// Make our request
	makeRequest(url, true, dataCallback);

}//end getLinkId

			<?php
		}//end if(get_link_id)

		if ($js_api_asset->attr('get_asset_tree')) {
			?>

/**
* This will return tree information for children
*
* @param string		asset_id		Id of the asset to get children of
* @param number		levels			Number of levels to return
* @param function	dataCallback	Custom callback function
*
* @version $Revision: 0.2
*/
function getAssetTree(asset_id, levels, dataCallback)
{
	// Create blank function
	var dataCallback = typeof(dataCallback) != 'undefined' ? dataCallback : function() {};

	// Check to see if we have set any levels
	var levels = typeof(levels) != 'undefined' ? levels : 0;

	// Build our string
	var url = '<?php echo $api_url; ?>' + '?key=' + api_key + '&type=getAssetTree&id=' + asset_id + '&levels=' + levels;

	// Make our request
	makeRequest(url, true, dataCallback);

}//end getAssetTree

			<?php
		}//end if(get_asset_tree)

		if ($js_api_asset->attr('get_keywords_replacements')) {
			?>

/**
* This will get replacements for the passed in keywords
*
* @param string		asset_id		Id of the asset to get keyword replacements for
* @param array		keywords		Array of keywords to get replacements for
* @param function	dataCallback	Custom callback function
*
*/
function getKeywordsReplacements(asset_id, keywords_array, dataCallback)
{
	// Create blank function
	var dataCallback = typeof(dataCallback) != 'undefined' ? dataCallback : function() {};

	var keywords = '';
	for (var keyword in keywords_array) {
		// construct our query strings to be passed
		if (keywords_array[keyword] == '') continue;
		keywords = keywords + keywords_array[keyword] + '\\,';
	}

	// remove the trailing "\,"
	keywords = keywords.substring(0, keywords.length-2);

	// Build our string
	var url = '<?php echo $api_url; ?>' + '?key=' + api_key + '&type=getKeywordsReplacements&id=' + asset_id + '&keywords=' + keywords;

	// Make our request
	makeRequest(url, true, dataCallback);

}//end getKeywordsReplacements()

			<?php
		}//end if(get_keywords_replacements)

		if ($js_api_asset->attr('set_asset_status')) {
			?>

/**
* This will set Asset to the status that is passed in
*
* @param string		asset_id		id of the asset to set status to
* @param int		status			status tha asset is to be set to
* @param boolean	cascade			if to cascade the status to non-dependant children(false by default)
* @param string		workflow_stream workflow stream to be passed in
* @param function	dataCallback	custom callback function
*
*
*/
function setAssetStatus(asset_id, status, cascade, workflow_stream, dataCallback)
{
	// Create blank function
	var dataCallback = typeof(dataCallback) != 'undefined' ? dataCallback : function() {};
	var workflow_stream  = isset(workflow_stream) ? workflow_stream : '';
	var cascade = isset(cascade) && ( cascade.toString().toLowerCase() != 'null' && cascade != '' ) ?  cascade : 0;

	// we cannot pass TRUE or FALSE as string so lets do our conversion
	// but try conversion only if its a string still
	if (typeof(cascade) == 'string') {
		if (cascade.toLowerCase() == 'false') {
			cascade = 0;
		} else if (cascade.toLowerCase() == 'true') {
			cascade = 1;
		}
	}

	// Build our string
	var url = '<?php echo $api_url; ?>' + '?key=' + api_key + '&type=setAssetStatus&id=' + asset_id + '&status=' + status + '&cascade=' + cascade + '&workflow_stream=' + workflow_stream.replace(/&/g , "%26").replace(/#/g , "%23").replace(/\?/g , "%3F").replace(/\+/g , "%2B");

	// Make our request
	makeRequest(url, true, dataCallback);

}//end setAssetStatus()

			<?php
		}//end if(set_asset_status)

		if ($js_api_asset->attr('get_web_path')) {
			?>

/**
* This will get webpath(s) of the assetid supplied
*
* @param string		asset_id		Id of the asset to get webpaths for
* @param function	dataCallback	Custom callback function
*
*
*/
function getWebPath(asset_id, dataCallback)
{
	// Create blank function
	var dataCallback = typeof(dataCallback) != 'undefined' ? dataCallback : function() {};

	// Build our string
	var url = '<?php echo $api_url; ?>' + '?key=' + api_key + '&type=getWebPath&id=' + asset_id;

	// Make our request
	makeRequest(url, true, dataCallback);

}//end getWebPath()

			<?php
		}//end if(get_web_path)

		if ($js_api_asset->attr('set_web_path')) {
			?>

/**
* This will set webpath(s) of the assetid supplied
*
* @param string		asset_id		Id of the asset to set webpaths for
* @param array		paths			new web paths to be assigned to asset
* @param boolean	auto_remap		if we auto remaping(default to TRUE)
* @param function	dataCallback	Custom callback function
*
*
*/
function setWebPath(asset_id, paths, auto_remap ,dataCallback)
{
	// Create blank function
	var dataCallback = typeof(dataCallback) != 'undefined' ? dataCallback : function() {};
	var auto_remap = isset(auto_remap) && ( auto_remap.toString().toLowerCase() != 'null' && auto_remap != '' ) ?  auto_remap : 1;

	// we cannot pass TRUE or FALSE as string so lets do our conversion
	// but try conversion only if its a string still
	if (typeof(auto_remap) == 'string') {
		if (auto_remap.toLowerCase() == 'false') {
			auto_remap = 0;
		} else if (auto_remap.toLowerCase() == 'true') {
			auto_remap = 1;
		}
	}

	var webpath = '';
	for (var path in paths) {
		// construct our query strings to be passed
		webpath = webpath + paths[path].replace(/&/g , "%26").replace(/#/g , "%23").replace(/\?/g , "%3F").replace(/\+/g , "%2B") + '\\,';
	}

	// remove the trailing "\,"
	webpath = webpath.substring(0, webpath.length-2);


	// Build our string
	var url = '<?php echo $api_url; ?>' + '?key=' + api_key + '&type=setWebPath&id=' + asset_id + '&webpath=' + webpath + '&auto_remap=' + auto_remap ;

	// Make our request
	makeRequest(url, true, dataCallback);

}//end setWebPath()

			<?php
		}//end if(set_web_path)

		if ($js_api_asset->attr('get_workflow_schema')) {
			?>

/**
* This function will tell us if we have a runnig workflow on the passed assetid
*
* @param string		asset_id		Id of the asset to get workflow schemas on
* @param boolean	granted			what is the status of workflow we are trying to get (default to NULL)
*									TRUE => granted, FALSE => denied, NULLL => get all
* @param boolean	running			if we only are getting wokflows that are running (default to FALSE)
* @param function	dataCallback	Custom callback function
*
*
*/
function getWorkflowSchema(asset_id, granted, running, dataCallback)
{
	// Create blank function
	var dataCallback = typeof(dataCallback) != 'undefined' ? dataCallback : function() {};
	var granted = isset(granted) && ( granted.toString().toLowerCase() != 'null' && granted != '' ) ? granted : '';
	var running = isset(running) && ( running.toString().toLowerCase() != 'null' && running != '' ) ? running : 0;

	// we cannot pass TRUE or FALSE as string so lets do our conversion
	// but try conversion only if its a string still
	if (typeof(granted) == 'string') {
		if (granted.toLowerCase() == 'false') {
			granted = 0;
		} else if (granted.toLowerCase() == 'true') {
			granted = 1;
		}
	}
	if (typeof(running) == 'string') {
		if (running.toLowerCase() == 'false') {
			running = 0;
		} else if (running.toLowerCase() == 'true') {
			running = 1;
		}
	}

	// Build our string
	var url = '<?php echo $api_url; ?>' + '?key=' + api_key + '&type=getWorkflowSchema&id=' + asset_id + '&granted=' + granted + '&running=' + running;

	// Make our request
	makeRequest(url, true, dataCallback);

}//end getWorkflowSchema()

			<?php
		}//end if(get_workflow_schema)

		if ($js_api_asset->attr('set_file_content')) {
			?>

/**
* This function will let user edit content of Editable File type assets
* File type that can be edited - css_file, xml_file, css_file, text_file, xsl_file, js_file
* User needs to acquire locks before being able to edit the file
*
*
* @param	string		assetID 		Id of the asset to update content for
* @param	string		content			new content of the asset
* @param	function	dataCallback	Custom callback function
*
*/
function setContentOfEditableFileAsset(assetID, content, dataCallback)
{
	// Create blank function
	var dataCallback = typeof(dataCallback) != 'undefined' ? dataCallback : function() {};

	var content = isset(content) ? escape(content) : 'no_value_provided';

	var url = '<?php echo $api_url; ?>' +
				'?key=' + api_key +
				'&type=setContentOfEditableFileAsset&id=' + assetID +
				'&content=' + content ;

	// Make our request
	makeRequest(url, true, dataCallback);


}// end setContentOfEditableFileAsset()

			<?php
		}//end if(set_file_content)

		if ($js_api_asset->attr('import_assets')) {
			?>

/**
* This function will let user import assets into Matrix from a structured XML file
* For example for xml file look under System Tools > Import Assets from XML Tool > example.xml
*
*
* @param	string		assetID 		Asset under which the assets are to be imported under
* @param	string		filePath		path to file on the file system
* @param	function	dataCallback	Custom callback function
*
*/
function importAssetsFromXML(assetID, filePath, dataCallback)
{
	// Create blank function
	var dataCallback = typeof(dataCallback) != 'undefined' ? dataCallback : function() {};

	var file = isset(file) ? content : 'no_file_provided';
	var url = '<?php echo $api_url; ?>' +
				'?key=' + api_key +
				'&type=importAssetsFromXML&id=' + assetID +
				'&filePath=' + filePath;

	// Make our request
	makeRequest(url, true, dataCallback);

}// end importAssetsFromXML()

			<?php
		}//end if(import_assets)

		if ($js_api_asset->attr('get_roles')) {
			?>

/**
* Returns an array of roles and the users/groups which can perform it
*
* @param int		assetid				the assetid of the asset the role is applied to
* @param int		roleid				the assetid of the the role that is applied
* @param int		userid				the assetid of the user performing the role
* @param boolean	include_assetid		whether to include the assetid in the returned array
* @param boolean	include_globals		whether to query the role view which includes
*										expanded global roles as individual users
* @param boolean	expand_groups		when TRUE, any groups defined within a role will be
*										replaced with the userids in that group.
*										If FALSE, return the groupids
* @param boolean	inc_dependants		If false it will filter out the dependant assets
* @param boolean	include_parents		When userid is specified. This will include the roles applied directly to the user and indirectly to user parent groups.
* @param array	type_codes			When include_assetid is TRUE. This is a type code filter for returned assetids.
* @param boolean	strict_type_code		Use strict type code for type code filter
* @param function	dataCallback		Custom callback function
*
* @return array
* @access public
*/
function getRoles(assetid, roleid, userid, include_assetid, include_globals, expand_groups, inc_dependants, include_parents, type_codes, strict_type_code,  dataCallback)
{
	// Create blank function
	var dataCallback = typeof(dataCallback) != 'undefined' ? dataCallback : function() {};
	var assetid = isset(assetid) ? assetid : '';
	var roleid = isset(roleid) ? roleid : '';
	var userid = isset(userid) ? userid : '';

	var include_assetid = isset(include_assetid) && ( include_assetid.toString().toLowerCase() != 'null' && include_assetid != '' ) ? include_assetid : 0;
	var include_globals = isset(include_globals) && ( include_globals.toString().toLowerCase() != 'null' && include_globals != '' ) ? include_globals : 0;
	var expand_groups = isset(expand_groups) && ( expand_groups.toString().toLowerCase() != 'null' && expand_groups != '' ) ? expand_groups : 0;
	var inc_dependants = isset(inc_dependants) && ( inc_dependants.toString().toLowerCase() != 'null') ? inc_dependants : 1;
	var include_parents = isset(include_parents) && ( include_parents.toString().toLowerCase() != 'null' && include_parents != '' ) ? include_parents : 0;
	var type_code = '';
	if(isset(type_codes) && type_codes != '') {
		if(typeof(type_codes) == 'object' ) {
			for (var idx in type_codes) {
				type_code = type_code + type_codes[idx] + '\\,';
			}
			type_code = type_code.substring(0, type_code.length-2);
		} else {
			type_code = type_codes;
		}
	}
	var strict_type_code = isset(strict_type_code) && ( strict_type_code.toString().toLowerCase() != 'null' && strict_type_code != '' ) ? strict_type_code : 1;

	// Build our string
	var url = '<?php echo $api_url; ?>' + '?key=' + api_key + '&type=getRoles&id=' + assetid
	 + '&roleid=' + roleid
	 + '&userid=' + userid
	 + '&include_assetid=' + include_assetid
	 + '&include_globals=' + include_globals
	 + '&expand_groups=' + expand_groups
	 + '&inc_dependants=' + inc_dependants
	 + '&include_parents=' + include_parents
	 + '&type_codes=' + type_code
	 + '&strict_type_code=' + strict_type_code;

	// Make our request
	makeRequest(url, true, dataCallback);

}//end getRoles

			<?php
		}//end if(get_roles)

		if ($js_api_asset->attr('execute_html_tidy')) {
			?>

/**
* Performs HTMLTidy on the passed content and returns cleaned up code
*
* @param string		content			the string content that needs to be cleaned up
* @param function	dataCallback	Custom callback function
*
* @return string
* @access public
*/
function executeHTMLTidy(content, dataCallback)
{
	// Create blank function
	var dataCallback = typeof(dataCallback) != 'undefined' ? dataCallback : function() {};
	var content = isset(content) ? content : '';

	// Build our string
	var url = '<?php echo $api_url; ?>' + '?key=' + api_key + '&type=executeHTMLTidy&content=' + content.replace(/&/g , "%26").replace(/#/g , "%23").replace(/\?/g , "%3F").replace(/\+/g , "%2B");

	// Make our request
	makeRequest(url, true, dataCallback);

}//end executeHTMLTidy()

			<?php
		}//end if(execute_html_tidy)

		if ($js_api_asset->attr('get_lineage')) {
			?>

/**
* Gets lineage based on the url passed for the asset
*
* @param string		asset_url				the asset url we are getting lineage from
* @param boolean	significant_link_only	if get to get back significant (type_1 and type_2) links only
* @param function	dataCallback			Custom callback function
*
* @return string
* @access public
*/
function getLineage(asset_url, significant_link_only, dataCallback)
{
	// Create blank function
	var dataCallback = typeof(dataCallback) != 'undefined' ? dataCallback : function() {};
	var asset_url = isset(asset_url) ? asset_url.replace(/&/g , "%26").replace(/#/g , "%23").replace(/\?/g , "%3F").replace(/\+/g , "%2B") : '';
	var significant_link_only = isset(significant_link_only) && (significant_link_only.toString().toLowerCase() != 'null' && significant_link_only != '' ) ? significant_link_only : 1;

	// we cannot pass TRUE or FALSE as string so lets do our conversion
	// but try conversion only if its a string still
	if (typeof(significant_link_only) == 'string') {
		if(significant_link_only.toLowerCase() == 'false') {
			significant_link_only = 0;
		} else if (significant_link_only.toLowerCase() == 'true') {
			significant_link_only = 1;
		}
	}

	// Build our string
	var url = '<?php echo $api_url; ?>' + '?key=' + api_key + '&type=getLineage&asset_url=' + asset_url + '&significant_link_only=' + significant_link_only;

	// Make our request
	makeRequest(url, true, dataCallback);

}//end getLineage()


			<?php
		}//end if(get_lineage)


				if ($js_api_asset->attr('get_lineage_from_url')) {
			?>

/**
* Returns an array of assets info in the order that they propagate out from the given URL.
*
* @param string		asset_url				the asset url we are getting lineage from
* @param function	dataCallback			Custom callback function
*
* @return string
* @access public
*/
function getLineageFromUrl(asset_url, dataCallback)
{
	// Create blank function
	var dataCallback = typeof(dataCallback) != 'undefined' ? dataCallback : function() {};
	var asset_url = isset(asset_url) ? asset_url.replace(/&/g , "%26").replace(/#/g , "%23").replace(/\?/g , "%3F").replace(/\+/g , "%2B") : '';
	var significant_link_only = isset(significant_link_only) && (significant_link_only.toString().toLowerCase() != 'null' && significant_link_only != '' ) ? significant_link_only : 1;


	// Build our string
	var url = '<?php echo $api_url; ?>' + '?key=' + api_key + '&type=getLineageFromUrl&asset_url=' + asset_url;

	// Make our request
	makeRequest(url, true, dataCallback);

}//end getLineageFromUrl()


			<?php
		}//end if(get_lineage_from_url)


		if ($js_api_asset->attr('get_url_from_lineage')) {
			?>

/**
* Given an array of asset ids in the order that they propagate out from site, it returns an array of URL(s) that matches the lineage.
*
* @param array	lineage				asset id array to represent lineage from top to bottom
* @param string	root_url				optional parameter to further filter results against multiple root urls
* @param string	protocol				optional parameter to further filter results for protocol (http, https)
* @param function	dataCallback			Custom callback function
*
* @return string
* @access public
*/
function getUrlFromLineage(lineage, root_url, protocol, dataCallback)
{
	// Create blank function
	var dataCallback = typeof(dataCallback) != 'undefined' ? dataCallback : function() {};
	var root_url = isset(root_url) && ( root_url.toString().toLowerCase() != 'null' && root_url != '' ) ? encodeURI(root_url) : '';
	var protocol = isset(protocol) && ( protocol.toString().toLowerCase() != 'null' && protocol != '' ) ? encodeURI(protocol) : '';

	var lineageArray = [];
	for (var i = 0, l = lineage.length; i<l; i+=1) {
	    if(lineage[i].hasOwnProperty("assetid")) {
		lineageArray.push(lineage[i]["assetid"]);
	    }
	    else {
		lineageArray.push(lineage[i]);
    	    }
	}

	// Build our string
	var url = '<?php echo $api_url; ?>' + '?key=' + api_key + '&type=getUrlFromLineage&lineage=' + lineageArray.join('\\,') + '&root_url=' + root_url + '&protocol=' + protocol;

	// Make our request
	makeRequest(url, true, dataCallback);

}//end getUrlFromLineage()


			<?php
		}//end if(get_url_from_lineage)


		if ($js_api_asset->attr('batch_requests')) {
			?>

/**
* batches multiple JS API calls and executs them in one HTTP Request
*
* @param json		function		json of all the function and its respective arguments to be passed in
* @param function	dataCallback	Custom callback function
*
* @return string
* @access public
*/
function batchRequest(functions, dataCallback)
{
	// Create blank function
	var dataCallback = typeof(dataCallback) != 'undefined' ? dataCallback : function() {};

	// Loop through the functions to validate
	for (var index in functions) {
		fnName = functions[index].function;
		// Error check that the function name supplied is a valid function
		if (!(window[fnName])) {
			throw "The function name " + functions[index].function + " does not exist";
		}
	}

	// Package up the required function calls into string to POST
	var postData = JSON.stringify(functions);

	var url = '<?php echo $api_url; ?>' + '?key=' + api_key + '&type=batchRequest&functions=' + postData.replace(/&/g , "%26").replace(/#/g , "%23").replace(/\?/g , "%3F").replace(/\+/g , "%2B");

	// Make our request
	makeRequest(url, true, dataCallback);

}//end batchRequest()

			<?php
		}//end if(batch_requests)

		if ($js_api_asset->attr('clone_asset')) {
			?>

/**
* clones the passed asset under a new parent
*
* @param assetid		string			id of asset we want to clone
* @param new_parent		string			assetid of the parent to clone under
* @param clone_num		int				number of clones to create under new parent (default is 1)
* @param new_position	int				position to place the newly cloned asset at (default at the bottom)
* @param link_type		string			type of links we want to create for cloned asset(default is SQ_LINK_TYPE_1)
*										Valid Values - SQ_LINK_TYPE_1, SQ_LINK_TYPE_2, SQ_LINK_TYPE_3
* @param link_value		string			the value of new link we want (default '')
* @param function		dataCallback	Custom callback function
*
* @return string
* @access public
*/
function cloneAsset(assetid, new_parent, clone_num, new_position, link_type, link_value, dataCallback)
{
	// Create blank function
	var dataCallback = typeof(dataCallback) != 'undefined' ? dataCallback : function() {};

	var clone_num	 = (isset(clone_num) && clone_num != '') ? clone_num : 1
	var new_position = (isset(new_position) && new_position != '') ? new_position : -1;
	var link_type	 = (isset(link_type) && link_type != '') ? link_type : 'SQ_LINK_TYPE_1';
	var link_value	 = (isset(link_value)) ? link_value : '';

	var url = '<?php echo $api_url; ?>' + '?key=' + api_key
				+ '&type=cloneAsset&id=' + assetid
				+ '&new_parent=' + new_parent
				+ '&clone_num=' + clone_num
				+ '&new_position=' + new_position
				+ '&link_value=' + link_value
				+ '&link_type=' + link_type;

	// Make our request
	makeRequest(url, true, dataCallback);

}//end cloneAsset()

			<?php
		}//end if(clone_asset)

		if ($js_api_asset->attr('show_diff')) {
			?>

/**
* gets the content (with or without paint layout) and check for the difference
*
* @param assetid_1			string			assetid of first asset
* @param assetid_2			string			assetid of second asset
* @param paint_layout_1		string			assetid of the paint layout you want to use on asset_1
* @param paint_layout_2		string			assetid of the paint layout you want to use on asset_2
* @param function			dataCallback	Custom callback functio
*
* @return string
* @access public
*/
function showDifference(assetid_1, assetid_2, paint_layout_1, paint_layout_2, dataCallback)
{
	// Create blank function
	var dataCallback = typeof(dataCallback) != 'undefined' ? dataCallback : function() {};
	paint_layout_1 = (isset(paint_layout_1)) ? paint_layout_1 : '';
	paint_layout_2 = (isset(paint_layout_2)) ? paint_layout_2 : '';


	var url = '<?php echo $api_url; ?>' + '?key=' + api_key
				+ '&type=showDifference&id=' + assetid_1
				+ '&assetid_2=' + assetid_2
				+ '&paint_layout_1=' + paint_layout_1
				+ '&paint_layout_2=' + paint_layout_2;

	// Make our request
	makeRequest(url, true, dataCallback);

}//end showDifference()

			<?php
		}//end if(show_diff)

		if ($js_api_asset->attr('get_context')) {
			?>

/**
* Returns the context we should be using based on the
* settings on the Context Configuration screen
*
* @param boolean		all_info		wheather to get all information of the context or
*										just get the id/name pairs(by default set to FALSE)
* @param function		dataCallback	Custom callback function
*
* @return string
* @access public
*/
function getAlternateContext(all_info, dataCallback)
{
	// Create blank function
	var dataCallback = typeof(dataCallback) != 'undefined' ? dataCallback : function() {};

	var all_info = isset(all_info) && (all_info.toString().toLowerCase() != 'null' && all_info.toString() != '' ) ? all_info : 0;

	if (typeof(all_info) == 'string') {
		if(all_info.toLowerCase() == 'false') {
			all_info = 0;
		} else if (all_info.toLowerCase() == 'true') {
			all_info = 1;
		}
	}


	var url = '<?php echo $api_url; ?>' + '?key=' + api_key +
			  '&type=getAlternateContext' +
			  '&all_info=' + all_info;

	// Make our request
	makeRequest(url, true, dataCallback);

}//end getAlternateContext()


/**
* Returns details of all the contexts in a system
*
* @param boolean		all_info		wheather to get all information of the context of
*										just get the id/name pairs(by default set to FALSE)
* @param function		dataCallback	Custom callback function
*
* @return string
* @access public
*/
function getAllContexts(all_info, dataCallback)
{
	// Create blank function
	var dataCallback = typeof(dataCallback) != 'undefined' ? dataCallback : function() {};

	var all_info = isset(all_info) && (all_info.toString().toLowerCase() != 'null' && all_info.toString() != '' ) ? all_info : 0;

	if (typeof(all_info) == 'string') {
		if(all_info.toLowerCase() == 'false') {
			all_info = 0;
		} else if (all_info.toLowerCase() == 'true') {
			all_info = 1;
		}
	}


	var url = '<?php echo $api_url; ?>' + '?key=' + api_key +
			  '&type=getAllContexts' +
			  '&all_info=' + all_info;

	// Make our request
	makeRequest(url, true, dataCallback);

}//end getAllContexts()


/**
* Returns the current context active on the system
*
* @param boolean		all_info		wheather to get all information of the context of
*										just get the id/name pairs(by default set to FALSE)
* @param function		dataCallback	Custom callback function
*
* @return string
* @access public
*/
function getCurrentContext(all_info, dataCallback)
{
	// Create blank function
	var dataCallback = typeof(dataCallback) != 'undefined' ? dataCallback : function() {};

	var all_info = isset(all_info) && (all_info.toString().toLowerCase() != 'null' && all_info.toString() != '' ) ? all_info : 0;

	if (typeof(all_info) == 'string') {
		if(all_info.toLowerCase() == 'false') {
			all_info = 0;
		} else if (all_info.toLowerCase() == 'true') {
			all_info = 1;
		}
	}


	var url = '<?php echo $api_url; ?>' + '?key=' + api_key +
			  '&type=getCurrentContext' +
			  '&all_info=' + all_info;

	// Make our request
	makeRequest(url, true, dataCallback);

}//end getCurrentContext()

			<?php
		}//end if(get_context)


		if ($js_api_asset->attr('set_context')) {
			?>

/**
* Set the System to the Passed context
*
* @param string			context_id		the id of the context we want to activate
* @param function		dataCallback	Custom callback functio
*
* @return string
* @access public
*/
function setContext(context_id, dataCallback)
{
	// Create blank function
	var dataCallback = typeof(dataCallback) != 'undefined' ? dataCallback : function() {};

	var url = '<?php echo $api_url; ?>' + '?key=' + api_key +
			  '&type=setContext' +
			  '&context_id=' + context_id;

	// Make our request
	makeRequest(url, true, dataCallback);

}//end setContext()


/**
* Restores the previous context
*
* @param string			context_id		the id of the context we want to activate
* @param function		dataCallback	Custom callback functio
*
* @return string
* @access public
*/
function restoreContext(dataCallback)
{
	// Create blank function
	var dataCallback = typeof(dataCallback) != 'undefined' ? dataCallback : function() {};

	var url = '<?php echo $api_url; ?>' + '?key=' + api_key +
			  '&type=restoreContext';

	// Make our request
	makeRequest(url, true, dataCallback);

}//end restoreContext()

			<?php
		}//end if(set_context)

		if ($js_api_asset->attr('get_metadata_schema')) {
			?>
/*
* Get the info of the Metadata Schema applied to the asset
*
* @param string		assetid			the asset to get the info of the schemas applied to
* @param boolean    granted    		type of access : TRUE = applied, FALSE = denied
* @param boolean    cascades   		does this schema cascade to newly created assets?
* @param function	dataCallback	custom callback function
*
* @return array
* @access public
*/
function getMetadataSchema(assetid, granted, cascades, dataCallback)
{
	// Create blank function
	var dataCallback = typeof(dataCallback) != 'undefined' ? dataCallback : function() {};

	granted = isset(granted) && (granted.toString().toLowerCase() != 'null' && granted.toString() != '') ? granted.toString() : '1';
	cascades = isset(cascades) && (cascades.toString().toLowerCase() != 'null' && cascades.toString() != '') ? cascades.toString() : '';

	// we cannot pass TRUE or FALSE as string so lets do our conversion
	if (granted.toLowerCase() == 'false' || granted.toLowerCase() == '0') {
		granted = 0;
	} else {
		granted = 1;
	}

	var url = '<?php echo $api_url; ?>' + '?key=' + api_key
				+ '&type=getMetadataSchema&id=' + assetid
				+ '&granted=' + granted;
	if (cascades.toLowerCase() == 'false' || cascades.toLowerCase() == '0') {
		url = url + '&cascades=0';
	} else if (cascades.toLowerCase() == 'true' || cascades.toLowerCase() == '1') {
		url = url + '&cascades=1';
	}

	// Make our request
	makeRequest(url, true, dataCallback);

}//end getMetadataSchema()

			<?php
		}//end if(get_metadata_schema)


		if ($js_api_asset->attr('edit_metadata_schema')) {
			?>
/*
* Sets or unsets the passed metadata schema on the asset
*
* @param string		assetid			the asset to edit schemas on
* @param string		schemaid		the metadata schema to be applied
* @param string		set				set = 1; unset = 0
* @param boolean    granted    		type of access : TRUE = applied, FALSE = denied
* @param boolean    cascades   		does this schema cascade to newly created assets?
* @param function	dataCallback	custom callback function
*
* @return array
* @access public
*/
function editMetadataSchema(assetid, schemaid, set, granted, cascades, dataCallback)
{
	// Create blank function
	var dataCallback = typeof(dataCallback) != 'undefined' ? dataCallback : function() {};

	granted = isset(granted) && (granted.toString().toLowerCase() != 'null' && granted.toString() != '') ? granted.toString() : '1';
	set = isset(set) && (set.toString().toLowerCase() != 'null' && set.toString() != '') ? set.toString() : '';
	cascades = isset(cascades) && (cascades.toString().toLowerCase() != 'null' && cascades.toString() != '') ? cascades.toString() : '';

	// we cannot pass TRUE or FALSE as string so lets do our conversion
	if (granted.toLowerCase() == 'false' || granted.toLowerCase() == '0') {
		granted = 0;
	} else {
		granted = 1;
	}

	if (set.toLowerCase() == 'false' || set.toLowerCase() == '0') {
		set = 0;
	} else {
		set = 1;
	}

	var url = '<?php echo $api_url; ?>' + '?key=' + api_key
				+ '&type=editMetadataSchema&id=' + assetid
				+ '&schemaid=' + schemaid
				+ '&set=' + set
				+ '&granted=' + granted;

	if (cascades.toLowerCase() == 'false' || cascades.toLowerCase() == '0') {
		url = url + '&cascades=0';
	} else if (cascades.toLowerCase() == 'true' || cascades.toLowerCase() == '1') {
		url = url + '&cascades=1';
	}

	// Make our request
	makeRequest(url, true, dataCallback);

}//end editMetadataSchema()

			<?php
		}//end if(edit_metadata_schema)


		if ($js_api_asset->attr('create_form_question')) {
			?>
/*
* creates question(s) of a type on the passed formid or the form section id
*
* @param string		assetid			the asset of the form or form section to
*									add the question asset
* @param string		type_code		the type of question to be added (default: Text)
* @param int		units			number of question to add of a certain type (default:1)
*
* @return array
* @access public
*/
function createQuestion(assetid, type_code, units, dataCallback)
{
	// Create blank function
	var dataCallback = typeof(dataCallback) != 'undefined' ? dataCallback : function() {};

	type_code = isset(type_code) && (type_code.toString().toLowerCase() != 'null' && type_code.toString() != '') ? type_code : 'text';
	units = isset(units) && (units.toString().toLowerCase() != 'null' && units.toString() != '') ? units : 1;

	var url = '<?php echo $api_url; ?>' + '?key=' + api_key
				+ '&type=createQuestion&id=' + assetid
				+ '&type_code=' + type_code
				+ '&units=' + units;

	// Make our request
	makeRequest(url, true, dataCallback);

}//end createQuestion()

			<?php
		}//end if(create_form_question)


		if ($js_api_asset->attr('update_form_question')) {
			?>
/*
* updates a question of the passed formid or the form section id
*
* @param string		assetid			the asset of the form or form section to
*									update the question on
* @param string		question_id		the asset id of the question to update
* @param json		field_info		The object containing data that needs to be updated
*									here you can use attribute name as the key and it's
*									corresponding value (user needs to make sure they are passing correct datatype)
*
* @return array
* @access public
*/
function updateQuestion(assetid, question_id, field_info, dataCallback)
{
	// Create blank function
	var dataCallback = typeof(dataCallback) != 'undefined' ? dataCallback : function() {};

	var url = '<?php echo $api_url; ?>' + '?key=' + api_key
				+ '&type=updateQuestion&id=' + assetid
				+ '&question_id=' + question_id;

	for (field_name in field_info) {
		if (Object.prototype.toString.call(field_info[field_name]) === '[object Array]') {
			url = url + '&' + field_name + '=' + field_info[field_name].join('\\,');
		} else {
			url = url + '&' + field_name + '=' + field_info[field_name];
		}
	}

	// Make our request
	makeRequest(url, true, dataCallback);

}//end updateQuestion()

			<?php
		}//end if(update_form_question)


		if ($js_api_asset->attr('remove_form_question')) {
			?>
/*
* removes a question of the passed formid or the form section id
*
* @param string		assetid			the asset of the form or form section to
*									remove the question off
* @param string		question_id		the asset id of the question to remove
*
*
* @return array
* @access public
*/
function removeQuestion(assetid, question_id, dataCallback)
{
	// Create blank function
	var dataCallback = typeof(dataCallback) != 'undefined' ? dataCallback : function() {};

	var url = '<?php echo $api_url; ?>' + '?key=' + api_key
				+ '&type=removeQuestion&id=' + assetid
				+ '&question_id=' + question_id;


	// Make our request
	makeRequest(url, true, dataCallback);

}//end removeQuestion()

			<?php
		}//end if(remove_form_question)


		if ($js_api_asset->attr('update_question_order')) {
			?>
/*
* updates the question order on the passed formid or the form section id
*
* @param string		assetid			the asset of the form or form section to update
* @param array		sort_order		new order of the questions
*
*
* @return array
* @access public
*/
function updateQuestionOrder(assetid, sort_order, dataCallback)
{
	// Create blank function
	var dataCallback = typeof(dataCallback) != 'undefined' ? dataCallback : function() {};

	var url = '<?php echo $api_url; ?>' + '?key=' + api_key
				+ '&type=updateQuestionOrder&id=' + assetid
				+ '&sort_order=' + sort_order.join('\\,');

	// Make our request
	makeRequest(url, true, dataCallback);

}//end updateQuestionOrder()

			<?php
		}//end if(update_question_order)


		if ($js_api_asset->attr('get_ga_statistics')) {
			?>

		/**
		* Makes a statistics request.
		*
		* Required:
		* - either duration or start_date is required but not both
		* - dimensions are required even if empty; no more than 7 allowed
	    * - metrics are required and must be between 1 and 10 of them
		*
		* Defaults:
		* - request.end_date defaults to today's date if omitted
		* - sort_by and filter are empty if omitted
		* - force defaults to false
		*
		* Notes:
		* - duration is ISO 8601 period format, eg. "P14D" for 14 days, "P2M"
		*   for 2 months. Sets start date to "take end_date, subtract duration,
		*   add one day".
		* - Setting start date, on the other hand, is inclusive.
		* - metrics also can be set to 'SQ_SITE_METRICS' or 'SQ_PAGE_METRICS'
		*   for a set of standard site or page metrics.
		* - sort_by array are
		* - filters are ANDed together. Filters can also be specified as an array
		*   of subfilters which are ORed together first.
		*
		* @param {String} assetid                       The ID of the asset
		* @param {Object} request                       Request info.
		* @param {String} request.end_date              The end date.
		* @param {String} request.start_date            The start date.
		* @param {String} request.duration              The period duration.
		* @param {Array.String} request.dimensions      Dimensions to retrieve.
		* @param {String|Array.String} request.metrics  Metrics to retrieve.
		* @param {Array.String} request.sort_by         Sort order of results.
		* @param {Array.(String|Array.String)} request.filter  Filters.
		* @param {Function} dataCallback                Custom callback function.
		*/
		getGoogleAnalyticsStatistics: function(assetid, request, dataCallback)
		{
			// Create blank function
            var dataCallback = typeof(dataCallback) != 'undefined' ? dataCallback : function() {};

            var url = '<?php echo $api_url; ?>' + '?key=' + api_key
                        + '&type=getGoogleAnalyticsStatistics&id=' + assetid;

            for (field_name in request) {
                if (Object.prototype.toString.call(request[field_name]) === '[object Array]') {
                    url = url + '&' + field_name + '=' + request[field_name].join('\\,');
                } else {
                    url = url + '&' + field_name + '=' + request[field_name];
                }
            }

            // Make our request
            makeRequest(url, true, dataCallback);

		},// end getGoogleAnalyticsStatistics


			<?php
		}//end if(get_statistics)


		if ($js_api_asset->attr('get_system_information')) {
			?>
/*
* updates the question order on the passed formid or the form section id
*
* @param string		assetid			the asset of the form or form section to update
* @param array		sort_order		new order of the questions
*
*
* @return array
* @access public
*/
function getSystemInformation(assetid, sort_order, dataCallback)
{
	// Create blank function
	var dataCallback = typeof(dataCallback) != 'undefined' ? dataCallback : function() {};

	var url = '<?php echo $api_url; ?>' + '?key=' + api_key
				+ '&type=getSystemInformation';

	// Make our request
	makeRequest(url, true, dataCallback);

}//end updateQuestionOrder()

			<?php
		}//end if(update_question_order)


	}//end paintJSFunctions()

}// end class

?>
