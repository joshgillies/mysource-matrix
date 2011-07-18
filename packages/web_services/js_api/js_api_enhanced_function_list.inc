<?php
/**
* +--------------------------------------------------------------------+
* | This MySource Matrix CMS file is Copyright (c) Squiz Pty Ltd	   |
* | ACN 084 670 600													   |
* +--------------------------------------------------------------------+
* | IMPORTANT: Your use of this Software is subject to the terms of    |
* | the Licence provided in the file licence.txt. If you cannot find   |
* | this file please contact Squiz (www.squiz.net) so we may provide   |
* | you a copy.														   |
* +--------------------------------------------------------------------+
*
* $Id: js_api_enhanced_function_list.inc,v 1.4 2011/07/18 01:10:44 akarelia Exp $
*
*/

/**
* Javascript API Functions (version 2) 
* Specifically designed to imporve the perfomance of EES Module for Matrix
*
* Purpose prints JS file where needed with the function calls that are active
*
*
* @author  Anthony Barnes <abarnes@squiz.com.au>
* @author  Ash Karelia <akarelia@squiz.com.au>
* @version $Revision: 1.4 $
* @package MySource_Matrix_Packages
* @subpackage web_services
*/

class Js_Api_Enhanced_Function_List
{

	public static function paintJSFunctions(JS_Api $js_api_asset)
	{
		$api_url = $js_api_asset->getURL();
?>
/**
* +--------------------------------------------------------------------+
* | This MySource Matrix CMS file is Copyright (c) Squiz Pty Ltd       |
* | ACN 084 670 600                                                    |
* +--------------------------------------------------------------------+
* | IMPORTANT: Your use of this Software is subject to the terms of    |
* | the Licence provided in the file licence.txt. If you cannot find   |
* | this file please contact Squiz (www.squiz.net) so we may provide   |
* | you a copy.                                                        |
* +--------------------------------------------------------------------+
*
*/

(function(gscope){


	/**
	* Convert a JSON string to object
	*/
	function jsonToObj(json)
	{
		// Make the conversion
		if (typeof(JSON) !== 'undefined') {
			return JSON.parse(json);
		}// end if

		// Don't worry, even the creator of JSON says eval is ok here
		return eval('(' + json + ')');

	}// end jsonToObj


	/**
	* Get all the properties of an object as an array
	*
	* @param object    obj     The object to get the parameters for
	*/
	function getProperties(obj)
	{
		var properties = [];
		for (var propName in obj){
			if (obj.hasOwnProperty(propName)){
				properties.push(propName);
			}// end if
		}// end for

		return properties;

	}// end getProperties


	/**
	* Convert a variable to a boolean
	*/
	function convertToBoolean(variable)
	{
		var ret = false;
		if (variable !== null && typeof(variable) !== "undefined") {
			switch (typeof(variable)) {
				case 'number':
					ret = (variable === 1) ? true : false;
					break;

				case 'boolean':
					ret = variable;
					break;

				case 'string':
					var testVar = variable.toLowerCase();
					if (testVar === 'true' || testVar === '1') {
						ret = true;   
					} else if (testVar === 'false' || testVar === '0') {
						ret = false;
					}// end if
					break;
			}// end switch

		}// end if

		return ret;

	}// end convertToBoolean


	/**
	* The API constructor
	*/
	gscope.Squiz_Matrix_API = function(options)
	{
		var self = this;

		if (!options.hasOwnProperty('key') || options.key === '') {
			throw 'An API key is required';
		}// end if

		self.key = options.key;
	};// end construct


	/**
	* API methods
	*/
	gscope.Squiz_Matrix_API.prototype = {

		_http: function(options)
		{
			var self = this;

			// Create the HTTPRequest object
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
						} catch(nosupport) {
							request = false;
						}// end try
					}// end try
				}// end try

				if (!request) {
					throw 'Your browser does not support Ajax';
				}// end if

				return request;

			}// end createRequest


			// Process parameters into a data array
			function data(params)
			{
				// Automatically append 'key' to every request
				var dataArr = ['key=' + self.key];

				for (var i = 0, l = params.length; i<l; i+=1) {
					dataArr.push(params[i][0] + '=' + params[i][1]);
				}// end for
				return dataArr.join("&");

			}// end data

			// Set some defaults for the HTTP Request
			options = self._options(['params'],{
						url:                '<?php echo $api_url; ?>',
						query:              null,
						method:             'POST',
						contentType:        'application/x-www-form-urlencoded',
						params:             [],
						async:              true,
						onSuccess:          function(){},
						onError:            function(){
													throw 'HTTPRequest call failed';
											}
			},options);

			var http = createRequest();
			http.open(options.method,encodeURI(options.url),options.async);
			http.onreadystatechange = function() {
				if (http.readyState === 4) {
					if (http.status === 200) {
						if (typeof(http.responseText) !== 'undefined' || http.responseText !== '' || http.responseText !== null) {
							var response = jsonToObj(http.responseText);
							options.onSuccess.call(this,response);
						} else {
							options.onError.call(this,http);
						}// end if
					} else {
						options.onError.call(this,http);
					}// end if
				}// end if
			};// end onreadstatechange

			http.setRequestHeader("Content-type", options.contentType);


			// Send the request
			http.send(encodeURI(data(options.params)));

		},// end _ajax


		/**
		* Validates options and returns merged data
		*/
		_options: function(required,defaults,options)
		{
			// Required data
			for (var i = 0, l = required.length; i<l; i+=1) {
				if (!options.hasOwnProperty(required[i])) {
					throw 'Required argument "' + required[i] + '" missing';
				}// end if
			}// end for

			// Merge options and defaults
			for (var def in defaults) {
				if (defaults.hasOwnProperty(def)){
					defaults[def] = (options.hasOwnProperty(def)) ? options[def] : defaults[def];
				}//end if
			}// end for

			if (defaults.hasOwnProperty('dataCallback') && typeof(defaults.dataCallback) !== "function") {
				throw 'Data callback must be a function';
			}// end if

			return defaults;

		},// end _options


		/**
		* Cleans a URL parameter to replace #,&,?,+ chars into entities
		*
		* @param string        value       the value to convert
		*/
		_cleanUrlParam: function(value)
		{
			if (typeof(value) !== 'undefined' && value !== null && value.constructor === String && value !== '') {
				return value.replace(/#/g , "%23").replace(/&/g , "%26").replace(/\?/g , "%3F").replace(/\+/g , "%2B");
			}// end if
		
			return value;

		},// end _cleanUrlParam

			<?php
		if ($js_api_asset->attr('batch_requests')) {
			?>

		batchRequest: function(options)
		{
			var fnName = 'batchRequest';
			if (arguments.length === 0) {
				return {
					// Set the default values
					defaults: {
						functions:          null,
						dataCallback:       function(){}
					},

					// Set the required arguments
					required: ['functions']
				};
			}// end if

			// Extract some configuration and use it to build
			// parameters for the post call
			var fnConfig = this[fnName].call(this);
			options = this._options(fnConfig.required,fnConfig.defaults,options);

			// Loop through the functions to validate
			for (var index in options.functions) {
				if (options.functions.hasOwnProperty(index.function)) {
					// Error check that the fnName supplied is a valid function
					if (typeof(this[index.function]) !== "function") {
						throw "The function name " + index.function + " does not exist";
					}// end if
				}// end if
			}// end for

			// Package up the required function calls into string to POST
			var postData = JSON.stringify(options.functions);

			this._doPost(fnName,[
				['functions', postData]
			],options);

		},// end batchRequest

			<?php
		}// end if(batch_request)

		if ($js_api_asset->attr('get_general')) {
			?>

		/**
		* Get general information about an asset
		*
		*
		* @param object		options       JSON string of options
		* { 
		*      asset_id:       Id of the asset we are getting info for
		*      get_attributes: Set this to 1 to also return an assets attributes
		*      dataCallback:	Custom callback function
		* }
		*/
		getGeneral: function(options)
		{
			var fnName = 'getGeneral';
			if (arguments.length === 0) {
				return {
					// Set the default values
					defaults: {
						asset_id:           null,
						get_attributes:     0,
						dataCallback:       function(){}
					},

					// Set the required arguments
					required: ['asset_id']
				};
			}// end if

			// Extract some configuration and use it to build
			// parameters for the post call
			var fnConfig = this[fnName].call(this);
			options = this._options(fnConfig.required,fnConfig.defaults,options);

			options.get_attributes = (convertToBoolean(options.get_attributes)) ? 1 : 0;

			this._doPost(fnName,[
				['id',options.asset_id],
				['get_attributes',options.get_attributes]
			],options);

		},// end getGeneral


		getChildCount: function(options)
		{
			var fnName = 'getChildCount';
			if (arguments.length === 0) {
				return {
					// Set the default values
					defaults: {
						asset_id:           null,
						level:              0,
						dataCallback:       function(){}
					},

					// Set the required arguments
					required: ['asset_id','level']
				};
			}// end if

			// Extract some configuration and use it to build
			// parameters for the post call
			var fnConfig = this[fnName].call(this);
			options = this._options(fnConfig.required,fnConfig.defaults,options);

			this._doPost(fnName,[
				['asset_id', options.asset_id],
				['levels',options.level]
			],options);

		},// end getChildCount

			<?php
		}// end if(get_general)

		if ($js_api_asset->attr('get_lineage')) {
			?>

		getLineage: function(options)
		{
			var fnName = 'getLineage';
			if (arguments.length === 0) {
				return {
					// Set the default values
					defaults: {
						asset_url: 	        '',
						dataCallback:       function(){}
					},

					// Set the required arguments
					required: ['asset_url']
				};
			}// end if

			// Extract some configuration and use it to build
			// parameters for the post call
			var fnConfig = this[fnName].call(this);
			options = this._options(fnConfig.required,fnConfig.defaults,options);

			// Encode the url
			options.asset_url = encodeURI(options.asset_url);

			this._doPost(fnName,[
				['asset_url', options.asset_url]
			],options);

		},// end getLineage

			<?php
		}// end if(get_lineage)

		if ($js_api_asset->attr('get_attributes')) {
			?>

		/**
		* This will get an attribute value
		*
		* @param object		options       JSON string of options
		* { 
		*      asset_id:       Id of the asset we are getting info for
		*      dataCallback:	Custom callback function
		* }
		*/
		getAttributes: function(options)
		{
			var fnName = 'getAttributes';
			if (arguments.length === 0) {
				return {
					// Set the default values
					defaults: {
						asset_id:           null,
						dataCallback:       function(){}
					},

					// Set the required arguments
					required: ['asset_id']
				};
			}// end if

			// Extract some configuration and use it to build
			// parameters for the post call
			var fnConfig = this[fnName].call(this);
			options = this._options(fnConfig.required,fnConfig.defaults,options);

			this._doPost(fnName,[
				['id',options.asset_id]
			],options);

		},// end getAttribute

			<?php
		}// end if(get_attributes)

		if ($js_api_asset->attr('set_attribute')) {
			?>

		/**
		* This will set an attribute value
		*
		* @param object		options       JSON string of options
		* { 
		*      asset_id:       string      Id of the asset we are setting attributes for
		*      attr_name:		string      Name of the attribute to change
		*      attr_val:		string      Value to change the attribute to
		*      dataCallback:	function    Custom callback function
		* }
		*/
		setAttribute: function(options)
		{
			var fnName = 'setAttribute';
			if (arguments.length === 0) {
				return {
					// Set the default values
					defaults: {
						asset_id:           null,
						attr_name:          '',
						attr_val:           '',
						dataCallback:       function(){}
					},

					// Set the required arguments
					required: ['asset_id','attr_name','attr_val']
				};
			}// end if

			// Extract some configuration and use it to build
			// parameters for the post call
			var fnConfig = this[fnName].call(this);
			options = this._options(fnConfig.required,fnConfig.defaults,options);

			this._doPost(fnName,[
				['id',options.asset_id],
				['attr_name',options.attr_name],
				['attr_val',this._cleanUrlParam(options.attr_val)]
			],options);

		},// end setAttribute


		/**
		* Sets the passed attributes for the asset
		*
		* @param object		options       JSON string of options
		* { 
		*      asset_id:		string      Id of the asset we are setting attributes for
		*      field_info:		array       Attribute name and their respective values to be changed to
		*      dataCallback:	function    Custom callback function
		* }
		*/
		setMultipleAttributes: function(options)
		{
			var fnName = 'setMultipleAttributes';
			if (arguments.length === 0) {
				return {
					// Set the default values
					defaults: {
						asset_id:           null,
						field_info:         {},
						dataCallback:       function(){}
					},

					// Set the required arguments
					required: ['asset_id','field_info']
				};
			}// end if

			// Extract some configuration and use it to build
			// parameters for the post call
			var fnConfig = this[fnName].call(this);
			options = this._options(fnConfig.required,fnConfig.defaults,options);

			var field_names = [];
			var field_vals  = [];
			for (var field_name in options.field_info) {
				if (options.field_info.hasOwnProperty(field_name)){
					field_names.push(field_name);
					field_vals.push(this._cleanUrlParam(options.field_info[field_name]));
				}// end if
			}// end of

			this._doPost(fnName,[
				['attr_name',field_names.join('\\,')],
				['attr_val',field_vals.join('\\,')],
				['id',options.asset_id]
			],options);

		},// end setMultipleAttributes

			<?php
		}// end if(set_attribute)

		if ($js_api_asset->attr('get_metadata')) {
			?>

		/**
		* This will get metadata values
		*
		* @param object		options       JSON string of options
		* { 
		*      asset_id:       Id of the asset we are getting info for
		*      dataCallback:	Custom callback function
		* }
		*/
		getMetadata: function(options)
		{
			var fnName = 'getMetadata';
			if (arguments.length === 0) {
				return {
					// Set the default values
					defaults: {
						asset_id:           null,
						dataCallback:       function(){}
					},

					// Set the required arguments
					required: ['asset_id']
				};
			}// end if

			// Extract some configuration and use it to build
			// parameters for the post call
			var fnConfig = this[fnName].call(this);
			options = this._options(fnConfig.required,fnConfig.defaults,options);

			this._doPost(fnName,[
				['id',options.asset_id]
			],options);

		},// end getMetadata

			<?php
		}// end if(get_metadata)

		if ($js_api_asset->attr('set_metadata')) {
			?>

		/**
		* Get the value of a metadata field
		*
		* @param object		options       JSON string of options
		* { 
		*      asset_id:       string      Id of the asset we are setting metadata for
		*      field_id:       string      The asset id of the metadata field
		*      field_val:      string      The value to set for the metadata field
		*      dataCallback:	function    Custom callback function
		* }
		*/
		setMetadata: function(options)
		{
			var fnName = 'setMetadata';
			if (arguments.length === 0) {
				return {
					// Set the default values
					defaults: {
						asset_id:           null,
						field_id:           '',
						field_val:          '',
						dataCallback:       function(){}
					},

					// Set the required arguments
					required: ['asset_id','field_id','field_val']
				};
			}// end if

			// Extract some configuration and use it to build
			// parameters for the post call
			var fnConfig = this[fnName].call(this);
			options = this._options(fnConfig.required,fnConfig.defaults,options);

			this._doPost(fnName,[
				['id',options.asset_id],
				['field_id',options.field_id],
				['field_val',this._cleanUrlParam(options.field_val)]
			],options);

		},// end setMetadata


		/**
		* Set the values of multiple metadata fields
		*
		* @param object		options       JSON string of options
		* { 
		*      asset_id:       string      Id of the asset we are setting attributes for
		*      field_info:     array       Metadata field id and their respective values to be changed to
		*      dataCallback:	function    Custom callback function
		* }
		*/
		setMetadataAllFields: function(options)
		{
			var fnName = 'setMetadataAllFields';
			if (arguments.length === 0) {
				return {
					// Set the default values
					defaults: {
						asset_id:           null,
						field_info:         null,
						dataCallback:       function(){}
					},

					// Set the required arguments
					required: ['asset_id','field_info']
				};
			}// end if

			// Extract some configuration and use it to build
			// parameters for the post call
			var fnConfig = this[fnName].call(this);
			options = this._options(fnConfig.required,fnConfig.defaults,options);

			var field_ids = [];
			var field_vals  = [];
			for (var field_id in options.field_info) {
				if (options.field_info.hasOwnProperty(field_id)){
					field_ids.push(field_id);
					field_vals.push(this._cleanUrlParam(options.field_info[field_id]));
				}// end if
			}// end of

			this._doPost(fnName,[
				['field_id',field_ids.join('\\,')],
				['field_val',field_vals.join('\\,')],
				['id',options.asset_id]
			],options);

		},// end setMetadataAllFields

			<?php
		}// end if(set_metadata)

		if ($js_api_asset->attr('trash_asset')) {
			?>

		/**
		* This will send an asset to the trash
		*
		* @param object		options       JSON string of options
		* { 
		*      asset_ids:      mixed       A single string asset id, or array of strings for asset ids
		*      dataCallback:	function    Custom callback function
		* }
		*/
		trashAsset: function(options)
		{
			var fnName = 'trashAsset';
			if (arguments.length === 0) {
				return {
					// Set the default values
					defaults: {
						asset_ids:          [],
						dataCallback:       function(){}
					},

					// Set the required arguments
					required: ['asset_ids']
				};
			}// end if

			// Extract some configuration and use it to build
			// parameters for the post call
			var fnConfig = this[fnName].call(this);
			options = this._options(fnConfig.required,fnConfig.defaults,options);

			// If a string has been supplied (only 1 asset), then convert it to
			// an array
			if (options.asset_ids.constructor === String) {
				options.asset_ids = [options.asset_ids];
			}// end if

			this._doPost(fnName,[
				['assetid', options.asset_ids.join('\\,')]
			],options);

		},// end trashAsset

			<?php
		}// end if (trash_asset)

		if ($js_api_asset->attr('get_children')) {
			?>

		/**
		* This will return child asset ids of the passed asset
		*
		* @param object		options       JSON string of options
		* { 
		*      asset_id:		string      Id of the asset we are setting attributes for
		*      levels:			number      Number of levels to return
		*      type_codes:		array       asset type_code that we want back
		*      link_types:		array       type of links we are looking for
		*                                   Valid Values - SQ_LINK_TYPE_1, SQ_LINK_TYPE_2, SQ_LINK_TYPE_3, SQ_LINK_NOTICE
		*      link_value:      array       link values we are looking for
		*      get_attributes:	boolean     if we are getting non standard attribute values of the assets(FALSE by default)
		*      dataCallback:	function    Custom callback function
		* }
		*/
		getChildren: function(options)
		{
			var fnName = 'getChildren';
			if (arguments.length === 0) {
				return {
					// Set the default values
					defaults: {
						asset_id:           null,
						levels:             0,
						type_codes:         [],
						link_types:         [],
						link_values:        [],
						get_attributes:     0,
						dataCallback:       function(){}
					},

					// Set the required arguments
					required: ['asset_id']
				};
			}// end if

			// Extract some configuration and use it to build
			// parameters for the post call
			var fnConfig = this[fnName].call(this);
			options = this._options(fnConfig.required,fnConfig.defaults,options);

			if (typeof(options.type_codes) === "string") {
				options.type_codes = [options.type_codes];
			}// end if

			if (typeof(options.link_values) === "string") {
				options.link_values = [options.link_values];
			}// end if

			if (typeof(options.link_types) === "string") {
				options.link_types = [options.link_types];
			}// end if

			options.get_attributes = (convertToBoolean(options.get_attributes)) ? 1 : 0;

			this._doPost(fnName,[
				['id', options.asset_id],
				['levels', options.levels],
				['type_codes', options.type_codes.join('\\,')],
				['link_type', options.link_types.join('\\,')],
				['link_value', options.link_values.join('\\,')],
				['get_attributes', options.get_attributes]
			],options);

		},// end getChildren

			<?php
		}//end if (get_children)

		if ($js_api_asset->attr('get_parents')) {
			?>

		getParents: function(options)
		{
			var fnName = 'getParents';
			if (arguments.length === 0) {
				return {
					// Set the default values
					defaults: {
						asset_id:           null,
						levels:             0,
						type_codes:         [],
						link_types:         [],
						link_values:        [],
						get_attributes:     0,
						dataCallback:       function(){}
					},

					// Set the required arguments
					required: ['asset_id']
				};
			}// end if

			// Extract some configuration and use it to build
			// parameters for the post call
			var fnConfig = this[fnName].call(this);
			options = this._options(fnConfig.required,fnConfig.defaults,options);

			if (typeof(options.type_codes) === "string") {
				options.type_codes = [options.type_codes];
			}// end if

			if (typeof(options.link_values) === "string") {
				options.link_values = [options.link_values];
			}// end if

			if (typeof(options.link_types) === "string") {
				options.link_types = [options.link_types];
			}// end if

			options.get_attributes = (convertToBoolean(options.get_attributes)) ? 1 : 0;

			this._doPost(fnName,[
				['id', options.asset_id],
				['levels', options.levels],
				['type_codes', options.type_codes.join('\\,')],
				['link_type', options.link_types.join('\\,')],
				['link_value', options.link_values.join('\\,')],
				['get_attributes', options.get_attributes]
			],options);

		},// end getParents

			<?php
		}// end if(get_parents)

		if ($js_api_asset->attr('get_permissions')) {
			?>

		getPermissions: function(options)
		{
			var fnName = 'getPermissions';
			if (arguments.length === 0) {
				return {
					// Set the default values
					defaults: {
						asset_id:           null,
						level:              '',
						dataCallback:       function(){}
					},

					// Set the required arguments
					required: ['asset_id']
				};
			}// end if

			// Extract some configuration and use it to build
			// parameters for the post call
			var fnConfig = this[fnName].call(this);
			options = this._options(fnConfig.required,fnConfig.defaults,options);

			this._doPost(fnName,[
				['id', options.asset_id],
				['level', options.level]
			],options);

		},// end getPermissions

			<?php
		}//

		if ($js_api_asset->attr('create_asset')) {
			?>

		createAsset: function(options)
		{
			var fnName = 'createAsset';
			if (arguments.length === 0) {
				return {
					// Set the default values
					defaults: {
						parent_id:          null,
						type_code:          '',
						asset_name:         '',
						link_type:          '',
						link_value:         '',
						sort_order:         -1,
						is_dependant:       0,
						is_exclusive:       0,
						extra_attributes:   0,
						attributes:         '',
						dataCallback:       function(){}
					},

					// Set the required arguments
					required: ['parent_id','type_code','asset_name','link_type']
				};
			}// end if

			// Extract some configuration and use it to build
			// parameters for the post call
			var fnConfig = this[fnName].call(this);
			options = this._options(fnConfig.required,fnConfig.defaults,options);

			options.is_dependant = (convertToBoolean(options.is_dependant)) ? 1 : 0;
			options.is_exclusive = (convertToBoolean(options.is_exclusive)) ? 1 : 0;

			this._doPost(fnName,[
				['id', options.parent_id],
				['type_code', options.type_code],
				['asset_name', this._cleanUrlParam(options.asset_name)],
				['link_type', options.link_type],
				['link_value',this._cleanUrlParam(options.link_value)],
				['sort_order', options.sort_order],
				['is_dependant', options.is_dependant],
				['is_exclusive', options.is_exclusive],
				['extra_attributes', options.extra_attributes + '&' + options.attributes],
			],options);

		},// end createAsset


		createFileAsset: function(options)
		{
			var fnName = 'createFileAsset';
			if (arguments.length === 0) {
				return {
					// Set the default values
					defaults: {
						parentID:           null,
						type_code:          'file',
						friendly_name:      '',
						link_type:          'SQ_LINK_TYPE_1',
						link_value:         '',
						dataCallback:       function(){}
					},

					// Set the required arguments
					required: ['parentID','type_code']
				};
			}// end if

			// Extract some configuration and use it to build
			// parameters for the post call
			var fnConfig = this[fnName].call(this);
			options = this._options(fnConfig.required,fnConfig.defaults,options);

			this._doPost(fnName,[
				['id', options.parentID],
				['type_code', options.type_code],
				['friendly_name',options.friendly_name],
				['link_type', options.link_type],
				['link_value', this._cleanUrlParam(options.link_value)]
			],options);

		},// end createFileAsset

			<?php
		}//end if(create_asset)

		if ($js_api_asset->attr('get_asset_types')) {
			?>

		getAssetTypes: function(options)
		{
			var fnName = 'getAssetTypes';
			if (arguments.length === 0) {
				return {
					// Set the default values
					defaults: {
						dataCallback:       function(){}
					},

					// Set the required arguments
					required: []
				};
			}// end if

			// Extract some configuration and use it to build
			// parameters for the post call
			var fnConfig = this[fnName].call(this);
			options = this._options(fnConfig.required,fnConfig.defaults,options);

			this._doPost(fnName,[
				['id','1']
			],options);

		},// end getAssetTypes

			<?php
		}// end if(get_asset_types)

		if ($js_api_asset->attr('get_locks_info')) {
			?>

		getLocksInfo: function(options)
		{
			var fnName = 'getLocksInfo';
			if (arguments.length === 0) {
				return {
					// Set the default values
					defaults: {
						asset_id:           null,
						screen_name:        'all',
						dataCallback:       function(){}
					},

					// Set the required arguments
					required: ['asset_id']
				};
			}// end if

			// Extract some configuration and use it to build
			// parameters for the post call
			var fnConfig = this[fnName].call(this);
			options = this._options(fnConfig.required,fnConfig.defaults,options);

			this._doPost(fnName,[
				['id',options.asset_id],
				['screen',options.screen_name]
			],options);

		},// end getLocksInfo

			<?php
		}//end if(get_locks_info)

		if ($js_api_asset->attr('acquire_lock')) {
			?>

		acquireLock: function(options)
		{
			var fnName = 'acquireLock';
			if (arguments.length === 0) {
				return {
					// Set the default values
					defaults: {
						asset_id:           null,
						screen_name:        'all',
						dependants_only:    1,
						force_acquire:      0,
						dataCallback:       function(){}
					},
	
					// Set the required arguments
					required: ['asset_id']
				};
			}// end if

			// Extract some configuration and use it to build
			// parameters for the post call
			var fnConfig = this[fnName].call(this);
			options = this._options(fnConfig.required,fnConfig.defaults,options);

			options.dependants_only = (convertToBoolean(options.dependants_only)) ? 1 : 0;
			options.force_acquire = (convertToBoolean(options.force_acquire)) ? 1 : 0;

			this._doPost(fnName,[
				['id', options.asset_id],
				['screen', options.screen_name],
				['dependants_only', options.dependants_only],
				['force_acquire', options.force_acquire]
			],options);

		},// end acquireLock

			<?php
		}//end if(acquire_locks)

		if ($js_api_asset->attr('release_lock')) {
			?>

		releaseLock: function(options)                                       
		{
			var fnName = 'releaseLock';
			if (arguments.length === 0) {
				return {
					// Set the default values
					defaults: {
						asset_id:           null,
						screen_name:        'all',
						dataCallback:       function(){}
					},

					// Set the required arguments
					required: ['asset_id']
				};
			}// end if

			// Extract some configuration and use it to build
			// parameters for the post call
			var fnConfig = this[fnName].call(this);
			options = this._options(fnConfig.required,fnConfig.defaults,options);

			this._doPost(fnName,[
				['id',options.asset_id],
				['screen',options.screen_name]
			],options);

		},// end releaseLock

			<?php
		}//end if(release_locks)

		if ($js_api_asset->attr('create_link')) {
			?>

		createLink: function(options)
		{
			var fnName = 'createLink';
			if (arguments.length === 0) {
				return {
					// Set the default values
					defaults: {
						parent_id:          null,
						child_id:           null,
						link_type:          '',
						link_value:         '',
						sort_order:         -1,
						is_dependant:       0,
						is_exclusive:       0,
						dataCallback:       function(){}
					},

					// Set the required arguments
					required: ['parent_id','child_id','link_value','link_type']
				};
			}// end if

			// Extract some configuration and use it to build
			// parameters for the post call
			var fnConfig = this[fnName].call(this);
			options = this._options(fnConfig.required,fnConfig.defaults,options);

			options.is_dependant = (convertToBoolean(options.is_dependant)) ? 1 : 0;
			options.is_exclusive = (convertToBoolean(options.is_exclusive)) ? 1 : 0;

			this._doPost(fnName,[
				['id', options.child_id],
				['parent_id', options.parent_id],
				['link_type', options.link_type],
				['link_value',options.link_value],
				['sort_order', options.sort_order],
				['is_dependant', options.is_dependant],
				['is_exclusive', options.is_exclusive]
			],options);

		},// end createLink

			<?php
		}//end if(create_link)

		if ($js_api_asset->attr('remove_link')) {
			?>

		removeLink: function(options)
		{
			var fnName = 'removeLink';
			if (arguments.length === 0) {
				return {
					// Set the default values
					defaults: {
						parent_id:          null,
						child_id:           null,
						link_type:          '',
						link_value:         '',
						dataCallback:       function(){}
					},

					// Set the required arguments
					required: ['parent_id','child_id','link_value','link_type']
				};
			}// end if

			// Extract some configuration and use it to build
			// parameters for the post call
			var fnConfig = this[fnName].call(this);
			options = this._options(fnConfig.required,fnConfig.defaults,options);

			this._doPost(fnName,[
				['id', options.child_id],
				['parent_id', options.parent_id],
				['link_type', options.link_type],
				['link_value',this._cleanUrlParam(options.link_value)]
			],options);

		},// end removeLink


		removeMultipleLinks: function(options)
		{
			var fnName = 'removeMultipleLinks';
			if (arguments.length === 0) {
				return {
					// Set the default values
					defaults: {
						link_info:          {},
						dataCallback:       function(){}
					},

					// Set the required arguments
					required: ['link_info']
				};
			}// end if

			// Extract some configuration and use it to build
			// parameters for the post call
			var fnConfig = this[fnName].call(this);
			options = this._options(fnConfig.required,fnConfig.defaults,options);

			if (options.link_info.hasOwnProperty('links')){
				if (options.link_info.links.constructor !== Array) {
					throw '"links" property for function ' + fnName + ' must be an Array';   
				}// end if

				var child_ids   = [];
				var parent_ids  = [];
				var link_types  = [];
				var link_values = [];

				for (var i = 0, l = options.link_info.links.length; i<l; i+=1) {
					child_ids.push(options.link_info.links[i].child);
					parent_ids.push(options.link_info.links[i].parent);
					link_types.push(options.link_info.links[i].link_type);
					link_values.push(this._cleanUrlParam(options.link_info.links[i].link_value));
				}// end for

				this._doPost(fnName,[
					['child_id', child_ids.join('\\,')],
					['parent_id', parent_ids.join('\\,')],
					['link_type', link_types.join('\\,')],
					['link_value',link_values.join('\\,')]
				],options);
			} else {
				throw fnName + ' missing "links" property';
			}// end if

		},// end removeMultipleLinks

			<?php
		}//end if(remove_links)

		if ($js_api_asset->attr('move_link')) {
			?>

		moveLink: function(options)
		{
			var fnName = 'moveLink';
			if (arguments.length === 0) {
				return {
					// Set the default values
					defaults: {
						old_parent_id:      null,
						child_id:           null,
						old_link_type:      'SQ_LINK_TYPE_1',
						old_link_value:     '',
						new_parent_id:      null,
						new_link_type:      'SQ_LINK_TYPE_1',
						new_link_value:     '',
						new_position:        -1,
						dataCallback:       function(){}
					},

					// Set the required arguments
					required: ['old_parent_id','child_id','old_link_type','old_link_value','new_parent_id']
				};
			}// end if

			// Extract some configuration and use it to build
			// parameters for the post call
			var fnConfig = this[fnName].call(this);
			options = this._options(fnConfig.required,fnConfig.defaults,options);

			this._doPost(fnName,[
				['id', options.child_id],
				['old_parent_id', options.old_parent_id],
				['old_link_type', options.old_link_type],
				['old_link_value', this._cleanUrlParam(options.old_link_value)],
				['new_parent_id',options.new_parent_id],
				['new_link_type', options.new_link_type],
				['new_link_value', this._cleanUrlParam(options.new_link_value)],
				['new_position', options.new_position]
			],options);

		},// end moveLink

			<?php
		}//end if(move_link)

		if ($js_api_asset->attr('update_link')) {
			?>

		updateLink: function(options)
		{
			var fnName = 'updateLink';
			if (arguments.length === 0) {
				return {
					// Set the default values
					defaults: {
						parent_id:          null,
						child_id:           null,
						existing_link_type: 'SQ_LINK_TYPE_1',
						existing_link_value:'',
						link_type:          'SQ_LINK_TYPE_1',
						link_value:         '',
						sort_order:         -1,
						locked:             '',
						dataCallback:       function(){}
					},

					// Set the required arguments
					required: ['parent_id','child_id','existing_link_type','existing_link_value']
				};
			}// end if

			// Extract some configuration and use it to build
			// parameters for the post call
			var fnConfig = this[fnName].call(this);
			options = this._options(fnConfig.required,fnConfig.defaults,options);

			this._doPost(fnName,[
				['id', options.child_id],
				['parent_id', options.parent_id],
				['existing_link_type', options.existing_link_type],
				['existing_link_value', this._cleanUrlParam(options.existing_link_value)],
				['link_type',options.link_type],
				['link_value', this._cleanUrlParam(options.link_value)],
				['sort_order', options.sort_order],
				['locked', options.locked]
			],options);

		},// end updateLink


		updateMultipleLinks: function(options)
		{
			var fnName = 'updateMultipleLinks';
				if (arguments.length === 0) {
					return {
						// Set the default values
						defaults: {
							link_info:          {},
							dataCallback:       function(){}
						},

						// Set the required arguments
						required: ['link_info']
					};
				}// end if

				// Extract some configuration and use it to build
				// parameters for the post call
				var fnConfig = this[fnName].call(this);
				options = this._options(fnConfig.required,fnConfig.defaults,options);

				if (options.link_info.hasOwnProperty('links')){
					if (options.link_info.links.constructor !== Array) {
						throw '"links" property for function ' + fnName + ' must be an Array';   
					}// end if

					var child_ids               = [];
					var parent_ids              = [];
					var link_types              = [];
					var link_values             = [];
					var existing_link_types     = [];
					var existing_link_values    = [];
					var sort_orders             = [];
					var link_locks              = [];

					for (var i = 0, l = options.link_info.links.length; i<l; i+=1) {
						child_ids.push(options.link_info.links[i].child);
						parent_ids.push(options.link_info.links[i].parent);
						link_types.push(options.link_info.links[i].link_type);
						link_values.push(this._cleanUrlParam(options.link_info.links[i].link_value));
						existing_link_types.push(options.link_info.links[i].existing_link_type);
						existing_link_values.push(this._cleanUrlParam(options.link_info.links[i].existing_link_value));
						sort_orders.push(options.link_info.links[i].sort_order);
						link_locks.push(options.link_info.links[i].link_lock);
					}// end for

					this._doPost(fnName,[
						['child_id', child_ids.join('\\,')],
						['parent_id', parent_ids.join('\\,')],
						['link_type', link_types.join('\\,')],
						['link_value',link_values.join('\\,')],
						['existing_link_type',existing_link_types.join('\\,')],
						['existing_link_value',existing_link_values.join('\\,')],
						['sort_order',sort_orders.join('\\,')],
						['locked',link_locks.join('\\,')]
					],options);
				} else {
					throw fnName + ' missing "links" property';
				}// end if

		},// end updateMultipleLinks

			<?php
		}//end if(update_link)

		if ($js_api_asset->attr('get_link_id')) {
			?>

		getLinkId: function(options)
		{
			var fnName = 'getLinkId';
				if (arguments.length === 0) {
					return {
						// Set the default values
						defaults: {
							parent_id:          null,
							child_id:           null,
							link_type:          'SQ_LINK_TYPE_1',
							link_value:         '',
							all_info:           0,
							dataCallback:       function(){}
						},

						// Set the required arguments
						required: ['parent_id','child_id']
					};
				}// end if

				// Extract some configuration and use it to build
				// parameters for the post call
				var fnConfig = this[fnName].call(this);
				options = this._options(fnConfig.required,fnConfig.defaults,options);

				options.all_info = (convertToBoolean(options.all_info)) ? 1 : 0;

				this._doPost(fnName,[
					['id', options.child_id],
					['parent_id', options.parent_id],
					['link_type',options.link_type],
					['link_value', this._cleanUrlParam(options.link_value)],
					['all_info',options.all_info]
				],options);

		},// end getLinkId

			<?php
		}//end if(get_link_id)

		if ($js_api_asset->attr('get_asset_tree')) {
			?>

		getAssetTree: function(options)
		{
			var fnName = 'getAssetTree';
			if (arguments.length === 0) {
				return {
					// Set the default values
					defaults: {
						asset_id:           null,
						levels:             0,
						dataCallback:       function(){}
					},

					// Set the required arguments
					required: ['asset_id']
				};
			}// end if

			// Extract some configuration and use it to build
			// parameters for the post call
			var fnConfig = this[fnName].call(this);
			options = this._options(fnConfig.required,fnConfig.defaults,options);

			this._doPost(fnName,[
				['id', options.asset_id],
				['levels', options.levels]
			],options);

		},// end getAssetTree

			<?php
		}//end if(get_asset_tree)

		if ($js_api_asset->attr('get_keywords_replacements')) {
			?>

		getKeywordsReplacements: function(options)
		{
			var fnName = 'getKeywordsReplacements';
			if (arguments.length === 0) {
				return {
					// Set the default values
					defaults: {
						asset_id:           null,
						keywords_array:     [],
						dataCallback:       function(){}
					},

					// Set the required arguments
					required: ['asset_id','keywords_array']
				};
			}// end if

			// Extract some configuration and use it to build
			// parameters for the post call
			var fnConfig = this[fnName].call(this);
			options = this._options(fnConfig.required,fnConfig.defaults,options);

			for (var i = 0, l = options.keywords_array.length; i<l; i+=1) {
				if (options.keywords_array[i] === '' ||	/^%(.*)%$/.test(options.keywords_array[i]) === false) {
					throw 'Incorrect keyword format passed to ' + fnName + ' ("' + options.keywords_array[i] + '")';
				}
			}// end for

			this._doPost(fnName,[
				['id', options.asset_id],
				['keywords', options.keywords_array.join('\\,')]
			],options);

		},// end getKeywordsReplacements

			<?php
		}//end if(get_keywords_replacements)

		if ($js_api_asset->attr('set_asset_status')) {
			?>

		setAssetStatus: function(options)
		{
			var fnName = 'setAssetStatus';
			if (arguments.length === 0) {
				return {
					// Set the default values
					defaults: {
						asset_id:           null,
						status:             '',
						cascade:            0,
						workflow_stream:    '',
						dataCallback:       function(){}
					},

					// Set the required arguments
					required: ['asset_id','status']
				};
			}// end if

			// Extract some configuration and use it to build
			// parameters for the post call
			var fnConfig = this[fnName].call(this);
			options = this._options(fnConfig.required,fnConfig.defaults,options);

			options.cascade = (convertToBoolean(options.cascade)) ? 1 : 0;

			this._doPost(fnName,[
				['id', options.asset_id],
				['workflow_stream', this._cleanUrlParam(options.workflow_stream)],
				['status',options.status],
				['cascade',options.cascade]
			],options);

		},// end setAssetStatus

			<?php
		}//end if(set_asset_status)

		if ($js_api_asset->attr('get_web_path')) {
			?>

		getWebPath: function(options)
		{
			var fnName = 'getWebPath';
			if (arguments.length === 0) {
				return {
					// Set the default values
					defaults: {
						asset_id:           null,
						dataCallback:       function(){}
					},

					// Set the required arguments
					required: ['asset_id']
				};
			}// end if

			// Extract some configuration and use it to build
			// parameters for the post call
			var fnConfig = this[fnName].call(this);
			options = this._options(fnConfig.required,fnConfig.defaults,options);

			this._doPost(fnName,[
				['id', options.asset_id]
			],options);

		},// end getWebPath

			<?php
		}//end if(get_web_path)

		if ($js_api_asset->attr('set_web_path')) {
			?>

		setWebPath: function(options)
		{
			var fnName = 'setWebPath';
			if (arguments.length === 0) {
				return {
					// Set the default values
					defaults: {
						asset_id:           null,
						paths:              [],
						auto_remap:         0,
						dataCallback:       function(){}
					},

					// Set the required arguments
					required: ['asset_id']
				};
			}// end if

			// Extract some configuration and use it to build
			// parameters for the post call
			var fnConfig = this[fnName].call(this);
			options = this._options(fnConfig.required,fnConfig.defaults,options);

			options.auto_remap = (convertToBoolean(options.auto_remap)) ? 1 : 0;

			for (var path in options.paths) {
				if (options.paths[path] !== '') {
					options.paths[path] = this._cleanUrlParam(options.paths[path]);
				}// end if
			}// end for

			this._doPost(fnName,[
				['id', options.asset_id],
				['webpath', options.paths.join('\\,')],
				['auto_remap',options.auto_remap]
			],options);

		},// end setWebPath

			<?php
		}//end if(set_web_path)

		if ($js_api_asset->attr('get_workflow_schema')) {
			?>

		getWorkflowSchema: function(options)
		{
			var fnName = 'getWorkflowSchema';
			if (arguments.length === 0) {
				return {
					// Set the default values
					defaults: {
						asset_id:           null,
						granted:            '',
						running:            0,
						dataCallback:       function(){}
					},

					// Set the required arguments
					required: ['asset_id']
				};
			}// end if

			// Extract some configuration and use it to build
			// parameters for the post call
			var fnConfig = this[fnName].call(this);
			options = this._options(fnConfig.required,fnConfig.defaults,options);

			options.running = (convertToBoolean(options.running)) ? 1 : 0;

			this._doPost(fnName,[
				['id', options.asset_id],
				['granted', options.granted],
				['running',options.running]
			],options);

		},// end getWorkflowSchema

			<?php
		}//end if(get_workflow_schema)

		if ($js_api_asset->attr('set_file_content')) {
			?>

		setContentOfEditableFileAsset: function(options)
		{
			var fnName = 'setContentOfEditableFileAsset';
			if (arguments.length === 0) {
				return {
					// Set the default values
					defaults: {
						asset_id:           null,
						content:            'no_value_provided',
						dataCallback:       function(){}
					},

					// Set the required arguments
					required: ['asset_id','content']
					};
			}// end if

			// Extract some configuration and use it to build
			// parameters for the post call
			var fnConfig = this[fnName].call(this);
			options = this._options(fnConfig.required,fnConfig.defaults,options);

			options.content = escape(options.content);

			this._doPost(fnName,[
				['id', options.asset_id],
				['content', options.content]
			],options);

		},// end setContentOfEditableFileAsset

			<?php
		}//end if(set_file_content)

		if ($js_api_asset->attr('import_assets')) {
			?>

		importAssetsFromXML: function(options)
		{
			var fnName = 'importAssetsFromXML';
			if (arguments.length === 0) {
				return {
					// Set the default values
					defaults: {
						asset_id:           null,
						filePath:           '',
						dataCallback:       function(){}
					},

					// Set the required arguments
					required: ['asset_id','filePath']
				};
			}// end if

			// Extract some configuration and use it to build
			// parameters for the post call
			var fnConfig = this[fnName].call(this);
			options = this._options(fnConfig.required,fnConfig.defaults,options);

			options.content = escape(content);

			this._doPost(fnName,[
				['id', options.asset_id],
				['filePath', options.filePath]
			],options);

		},// end importAssetsFromXML

			<?php
		}//end if(import_assets)

		if ($js_api_asset->attr('get_roles')) {
			?>

		getRoles: function(options)
		{
			var fnName = 'getRoles';
			if (arguments.length === 0) {
				return {
					// Set the default values
					defaults: {
						asset_id:           null,
						role_id:            '',
						user_id:            '',
						include_assetid:    0,
						include_globals:    0,
						expand_groups:      0,
						inc_dependants:     0,
						dataCallback:       function(){}
					},

					// Set the required arguments
					required: ['asset_id','role_id']
				};
			}// end if

			// Extract some configuration and use it to build
			// parameters for the post call
			var fnConfig = this[fnName].call(this);
			options = this._options(fnConfig.required,fnConfig.defaults,options);

			this._doPost(fnName,[
				['id', options.asset_id],
				['roleid', options.role_id],
				['userid', options.user_id],
				['include_assetid', options.include_assetid],
				['include_globals', options.include_globals],
				['expand_groups', options.expand_groups],
				['inc_dependants', options.inc_dependants],
			],options);

		},// end getRoles

			<?php
		}//end if(get_roles)

		if ($js_api_asset->attr('execute_html_tidy')) {
			?>

		executeHTMLTidy: function(options)
		{
			var fnName = 'executeHTMLTidy';
			if (arguments.length === 0) {
				return {
					// Set the default values
					defaults: {
						content:            '',
						dataCallback:       function(){}
					},

					// Set the required arguments
					required: ['content']
				};
			}// end if

			// Extract some configuration and use it to build
			// parameters for the post call
			var fnConfig = this[fnName].call(this);
			options = this._options(fnConfig.required,fnConfig.defaults,options);

			this._doPost(fnName,[
				['content', options.content]
			],options);

		},// end executeHTMLTidy

			<?php
		}//end if(execute_html_tidy)
		?>

		/**
		* Shortcut function for sending post data
		*/
		_doPost: function(fnName,data,options)
		{
			var self = this;

			// Add the 'type' parameter for as the calling function name
			data.push(['type',fnName]);
			self._http({
				params: data,
				onSuccess: function(json) {
					// Every function should have a dataCallback argument
					if (options.hasOwnProperty('dataCallback')) {
						options.dataCallback.call(this,json);
					}// end if
				}
			});

		}// end _doPost


	};// end Squiz_Matrix_API methods


})(window);

		<?php

	}//end paintJSFunctions()


}// end class

?>
