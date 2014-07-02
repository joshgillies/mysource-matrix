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
* $Id: js_api_enhanced_function_list.inc,v 1.30 2013/08/13 01:30:48 akarelia Exp $
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
* @version $Revision: 1.30 $
* @package MySource_Matrix_Packages
* @subpackage web_services
*/

class Js_Api_Enhanced_Function_List
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
		syncing_enabled : <?php echo $js_api_asset->attr('sync_context'); ?>,

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

		/**
		* batches multiple JS API calls and executs them in one HTTP Request
		*
		*
		* @param object		options       JSON string of options
		* { 
		*      functions:		json		json of all the function and its respective arguments to be passed in
		*      dataCallback:	function	Custom callback function
		* }
		*/
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

			// Package up the required function calls into string to POST
			var postData = JSON.stringify(options.functions);

			this._doPost(fnName,[
				['functions', this._cleanUrlParam(postData)]
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
		*      asset_id:		string/int	Id of the asset we are getting info for
		*      get_attributes:	boolean		Set this to 1 to also return an assets attributes
		*      dataCallback:	function	Custom callback function
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


		/**
		* Gets child count for the passed asset uptill certain level
		*
		*
		* @param object		options       JSON string of options
		* { 
		*      asset_id:		string/int	Id of the asset we are getting info for
		*      level:			int			Number of levels to check, default : 'all'
		*      dataCallback:	function	Custom callback function
		* }
		*/
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

		/**
		* Gets lineage of the passed asset until the root_folder
		*
		*
		* @param object		options       JSON string of options
		* { 
		*      asset_url:				string		url(including protocol) or the assetid of the asset
		*      significant_link_only:	boolean		if get to get back significant (type_1 and type_2) links only
		*      dataCallback:			function	Custom callback function
		* }
		*/
		getLineage: function(options)
		{
			var fnName = 'getLineage';
			if (arguments.length === 0) {
				return {
					// Set the default values
					defaults: {
						asset_url: 	           '',
						significant_link_only: 1,
						dataCallback:          function(){}
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

			options.significant_link_only = (convertToBoolean(options.significant_link_only)) ? 1 : 0;

			this._doPost(fnName,[
				['asset_url', options.asset_url],
				['significant_link_only', options.significant_link_only]
			],options);

		},// end getLineage

			<?php
		}// end if(get_lineage)
		
		
		if ($js_api_asset->attr('get_lineage_from_url')) {
			?>

		/**
		* Gets a single lineage of the passed url
		*
		*
		* @param object		options       JSON string of options
		* { 
		*      asset_url:				string		url(including protocol) or the assetid of the asset
		*      dataCallback:			function	Custom callback function
		* }
		*/
		getLineageFromUrl: function(options)
		{
			var fnName = 'getLineageFromUrl';
			if (arguments.length === 0) {
				return {
					// Set the default values
					defaults: {
						asset_url: 	           '',
						dataCallback:          function(){}
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

		},// end getLineageFromUrl

			<?php
		}// end if(get_lineage_from_url)
		
		
		if ($js_api_asset->attr('get_url_from_lineage')) {
			?>

		/**
		* Gets a single url based on the lineage passed (asset ids)
		*
		*
		* @param object		options       JSON string of options
		* { 
		*      param array			lineage	asset id array to represent lineage from top to bottom
		*      dataCallback:			function	Custom callback function
		* }
		*/
		getUrlFromLineage: function(options)
		{
			var fnName = 'getUrlFromLineage';
			if (arguments.length === 0) {
				return {
					// Set the default values
					defaults: {
						lineage: 	           [],
						root_url: 	           '',
						protocol:	           '',		
						dataCallback:          function(){}
					},

					// Set the required arguments
					required: ['lineage']
				};
			}// end if

			// Extract some configuration and use it to build
			// parameters for the post call
			var fnConfig = this[fnName].call(this);
			options = this._options(fnConfig.required,fnConfig.defaults,options);

			// Encode the lineage
			var lineageArray = [];
			for (var i = 0, l = options.lineage.length; i<l; i+=1) {
			    if (options.lineage[i].hasOwnProperty("assetid")) {
			       lineageArray.push(options.lineage[i]["assetid"]);
			    }
			    else {
			       lineageArray.push(options.lineage[i]);
			    }
			}
			

			this._doPost(fnName,[
				['lineage', lineageArray.join('\\,')],
				['root_url', options.root_url],
				['protocol', options.protocol]
			],options);

		},// end getUrlFromLineage

			<?php
		}// end if(get_url_from_lineage)
		
		
		
		if ($js_api_asset->attr('get_attributes')) {
			?>

		/**
		* This will get all attribute value
		*
		* @param object		options       JSON string of options
		* { 
		*      asset_id:		string/int	Id of the asset we are getting info for
		*      dataCallback:	function	Custom callback function
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
		*      asset_id:		string/int	Id of the asset we are setting attributes for
		*      attr_name:		string		Name of the attribute to change
		*      attr_val:		string		Value to change the attribute to
		*      dataCallback:	function	Custom callback function
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
		*      asset_id:		string/int	Id of the asset we are setting attributes for
		*      field_info:		json		Attribute name and their respective values to be changed to
		*      dataCallback:	function	Custom callback function
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
		*      asset_id:		string/int	Id of the asset we are getting info for
		*      dataCallback:	function	Custom callback function
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
		*      asset_id:		string/int	Id of the asset we are setting metadata for
		*      field_id:		string/int	The asset id of the metadata field
		*      field_val:		string		The value to set for the metadata field
		*									(Using 'SQ_METADATA_USE_DEFAULT' will set it to default)
		*      dataCallback:	function	Custom callback function
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
		*      asset_id:		string/int	Id of the asset we are setting attributes for
		*      field_info:		json		Metadata field id and their respective values to be changed to
		*									(Using 'SQ_METADATA_USE_DEFAULT' for value will set it to default)
		*      dataCallback:	function	Custom callback function
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
		*      asset_ids:		string/array	A single string asset id, or array of strings for asset ids
		*      dataCallback:	function		Custom callback function
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
		*      asset_id:		string/int	Id of the asset we are setting attributes for
		*      levels:			int			Number of levels to return
		*      type_codes:		array		asset type_code that we want back
		*      link_types:		array		type of links we are looking for
		*                       			Valid Values - SQ_LINK_TYPE_1, SQ_LINK_TYPE_2, SQ_LINK_TYPE_3, SQ_LINK_NOTICE
		*      link_value:      array		link values we are looking for
		*      get_attributes:	boolean		if we are getting non standard attribute values of the assets(FALSE by default)
		*      dataCallback:	function	Custom callback function
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
				['link_types', options.link_types.join('\\,')],
				['link_values', options.link_values.join('\\,')],
				['get_attributes', options.get_attributes]
			],options);

		},// end getChildren

			<?php
		}//end if (get_children)

		if ($js_api_asset->attr('get_parents')) {
			?>

		/**
		* This will return parent asset ids of the passed asset
		*
		* @param object		options       JSON string of options
		* { 
		*      asset_id:		string/int	Id of the asset we are setting attributes for
		*      levels:			int			Number of levels to return
		*      type_codes:		array		asset type_code that we want back
		*      link_types:		array		type of links we are looking for
		*                       			Valid Values - SQ_LINK_TYPE_1, SQ_LINK_TYPE_2, SQ_LINK_TYPE_3, SQ_LINK_NOTICE
		*      link_value:      array		link values we are looking for
		*      get_attributes:	boolean		if we are getting non standard attribute values of the assets(FALSE by default)
		*      dataCallback:	function	Custom callback function
		* }
		*/
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
				['link_types', options.link_types.join('\\,')],
				['link_values', options.link_values.join('\\,')],
				['get_attributes', options.get_attributes]
			],options);

		},// end getParents

			<?php
		}// end if(get_parents)

		if ($js_api_asset->attr('get_permissions')) {
			?>

		/**
		* This returns permissions for an asset
		*
		* @param object		options       JSON string of options
		* { 
		*      asset_id:		string/int	Id of the asset to get permissions for
		*      level:			int			1=READ 2=WRITE 3=ADMIN, default to READ
		*      dataCallback:	function	Custom callback function
		* }
		*/
		getPermissions: function(options)
		{
			var fnName = 'getPermissions';
			if (arguments.length === 0) {
				return {
					// Set the default values
					defaults: {
						asset_id:           null,
						level:              1,
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

		/**
		* Creates an asset
		*
		* @param object		options       JSON string of options
		* { 
		*      parent_id:			string/int		parentid of the asset to create
		*      type_code:			string			type of asset to create
		*      asset_name:			string			name for new asset
		*      link_type:			string			type of link to create
		*      link_value:			string			value of link
		*      sort_order:			int				order in the tree under the parent
		*      is_dependant:		boolean			dependant to parent?
		*      is_exclusive:		boolean			exclusive to parent?
		*      extra_attributes:	boolean			allows additional attributes on create
		*      attributes:			string			String of additional query string containing key/pair values
		*      dataCallback:		function		Custom callback function
		* }
		*/
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
				['extra_attributes', options.extra_attributes + '&' + options.attributes]
			],options);

		},// end createAsset


		/**
		* Creates a file asset
		*
		* @param object		options       JSON string of options
		* { 
		*      parentID:			string/int		parentid of the asset to create
		*      type_code:			string			type of asset to create
		*      friendly_name:		string			name for new asset
		*      link_type:			string			type of link to create
		*      link_value:			string			value of link
		*      dataCallback:		function		Custom callback function
		* }
		*/
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

		/**
		* Returns asset type codes installed on the system
		*
		* @param object		options       JSON string of options
		* { 
		*      dataCallback:	function	Custom callback function
		* }
		*/
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

		/**
		* Gets information for the lock type passed for an asset
		*
		* @param object		options       JSON string of options
		* { 
		*      asset_id:		string/int	Id of the asset to get permissions for
		*      screen_name:		string		The screen to get locks for
		*      dataCallback:	function	Custom callback function
		* }
		*/
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

		/**
		* Acquires locks on the given asset. if the locks were already acquired
		* by current user then they are updated
		*
		* @param object		options       JSON string of options
		* { 
		*      asset_id:		string/int	Id of the asset to get permissions for
		*      screen_name:		string		the screen to get locks on, by default 'all'
		*      dependants_only:	boolean		whether dependants only or all children, defaults to true
		*      force_acquire:	boolean		whether to attempt to forceably acquire the lock or not,  defaults to false
		*      dataCallback:	function	Custom callback function
		* }
		*/
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

		/**
		* Releases locks on the given screen
		*
		* @param object		options       JSON string of options
		* { 
		*      asset_id:		string/int	Id of the asset to get permissions for
		*      screen_name:		string		the screen to release locks for
		*      dataCallback:	function	Custom callback function
		* }
		*/
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

		/**
		* Creates a link between two assets with passed information
		*
		* @param object		options       JSON string of options
		* { 
		*      parent_id:		string/int	Major asset id we are linking from
		*      child_id:		string/int	Minor asset id we are linking to
		*      link_type:		string		Type of link to create
		*      link_value:		string		Value of the link
		*      sort_order:		int			Order in the tree
		*      is_dependant:	boolean		Dependant to parent
		*      is_exclusive:	boolean		Exclusive to parent
		*      dataCallback:	function	Custom callback function
		* }
		*/
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

		/**
		* Removes a link between a parent and child
		*
		* @param object		options       JSON string of options
		* { 
		*      parent_id:		string/int	Major asset id we are linking from
		*      child_id:		string/int	Minor asset id we are linking to
		*      link_type:		string		type of link we are looking for (SQ_LINK_TYPE_1 link by default)
		*									Valid Values - SQ_LINK_TYPE_1, SQ_LINK_TYPE_2, SQ_LINK_TYPE_3, SQ_LINK_NOTICE
		*      link_value:		string		value of link we are looking for ('' by default)
		*      dataCallback:	function	Custom callback function
		* }
		*/
		removeLink: function(options)
		{
			var fnName = 'removeLink';
			if (arguments.length === 0) {
				return {
					// Set the default values
					defaults: {
						parent_id:          null,
						child_id:           null,
						link_type:          'SQ_LINK_TYPE_1',
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


		/**
		* Removes multiple links between a parent and child pairs
		*
		* @param object		options       JSON string of options
		* { 
		*      link_info:		json		json object containing link info as 
		*									per removeLink() function of this API
		*      dataCallback:	function	Custom callback function
		* }
		*/
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

		/**
		* Moves a link from one parent to another
		*
		* @param object		options       JSON string of options
		* { 
		*      old_parent_id:		string/int	Id of old parent
		*      child_id:			string/int	Id of the child
		*      old_link_type:		string		type of link we are looking for (SQ_LINK_TYPE_1 link by default)
		*										Valid Values - SQ_LINK_TYPE_1, SQ_LINK_TYPE_2, SQ_LINK_TYPE_3, SQ_LINK_NOTICE
		*      old_link_value:		string		Value of link we are searching for between given assets ('' by default)
		*      new_parent_id:		string/int	Id of the new parent
		*      new_link_type:       string		type of link we are looking for (SQ_LINK_TYPE_1 link by default)
		*										Valid Values - SQ_LINK_TYPE_1, SQ_LINK_TYPE_2, SQ_LINK_TYPE_3, SQ_LINK_NOTICE
		*      new_link_value:		string		Value of link to use ('' by default)
		*      new_position:		int			The new position
		*      dataCallback:		function	Custom callback function
		* }
		*/
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

		/**
		* Updates a link with provided information
		*
		* @param object		options       JSON string of options
		* { 
		*      parent_id:			string/int	Id of parent
		*      child_id:			string/int	Id of child
		*      existing_link_type:	string		type of link we are looking for (SQ_LINK_TYPE_1 link by default)
		*										Valid Values - SQ_LINK_TYPE_1, SQ_LINK_TYPE_2, SQ_LINK_TYPE_3, SQ_LINK_NOTICE
		*      existing_link_value:	string		existing link value betweent the assets ('' by default)
		*      link_type:   	    string		link type to be updated to (SQ_LINK_TYPE_1 link by default)
		*										Valid Values - SQ_LINK_TYPE_1, SQ_LINK_TYPE_2, SQ_LINK_TYPE_3, SQ_LINK_NOTICE
		*      link_value:			string		Value of link to be updated to ('' by default)
		*      sort_order:			int			The new position
		*      locked:				boolean		the asset link lock status(locked by default)
		*      dataCallback:		function	Custom callback function
		* }
		*/
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


		/**
		* Updates multiple links
		*
		* @param object		options       JSON string of options
		* { 
		*      link_info:		json		object containing all the information
		*									as per updateLink() of this API
		*      dataCallback:	function	Custom callback function
		* }
		*/
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

		/**
		* Returns the link id between a parent and child
		*
		* @param object		options       JSON string of options
		* { 
		*      parent_id:		string/int	Id of parent
		*      child_id:		string/int	Id of child
		*      link_type:		string		type of link we are looking for(SQ_LINK_TYPE_1 link by default)
		*									Valid Values - SQ_LINK_TYPE_1, SQ_LINK_TYPE_2, SQ_LINK_TYPE_3, SQ_LINK_NOTICE
		*      link_value:		string		value of link we are looking for ('' by default)
		*      all_info:		boolean		if we want all the link information or just linkid
		*      dataCallback:	function	Custom callback function
		* }
		*/
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

		/**
		* This will return tree information for children
		*
		* @param object		options       JSON string of options
		* { 
		*      asset_id:		string/int	Id of the asset to get children of
		*      levels:			int			Number of levels to return
		*      dataCallback:	function	Custom callback function
		* }
		*/
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

		/**
		* This will get replacements for the passed in keywords
		*
		* @param object		options       JSON string of options
		* { 
		*      asset_id:		string/int	Id of the asset to get keyword replacements for
		*      keywords_array:	array		Array of keywords to get replacements for
		*      dataCallback:	function	Custom callback function
		* }
		*/
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

		/**
		* This will set Asset to the status that is passed in
		*
		* @param object		options       JSON string of options
		* { 
		*      asset_id:		string/int	Id of the asset to set status to
		*      status:			int			status tha asset is to be set to
		*      cascade:			boolean		if to cascade the status to non-dependant children(false by default)
		*      workflow_stream:	string		workflow stream to be passed in
		*      dataCallback:	function	Custom callback function
		* }
		*/
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

		/**
		* gets webpath(s) of the assetid supplied
		*
		* @param object		options       JSON string of options
		* { 
		*      asset_id:		string/int	Id of the asset to get webpaths for
		*      dataCallback:	function	Custom callback function
		* }
		*/
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

		/**
		* This will set webpath(s) of the assetid supplied
		*
		* @param object		options       JSON string of options
		* { 
		*      asset_id:		string/int	Id of the asset to set webpaths for
		*      paths:			array		new web paths to be assigned to asset
		*      auto_remap:		boolean		if we are auto remaping(default to TRUE)
		*      dataCallback:	function	Custom callback function
		* }
		*/
		setWebPath: function(options)
		{
			var fnName = 'setWebPath';
			if (arguments.length === 0) {
				return {
					// Set the default values
					defaults: {
						asset_id:           null,
						paths:              [],
						auto_remap:         1,
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

		/**
		* This function will tell us if we have a running workflow on the passed assetid
		*
		* @param object		options       JSON string of options
		* { 
		*      asset_id:		string/int	Id of the asset to get workflow schemas on
		*      granted:			mixed		what is the status of workflow we are trying to get (default to NULL)
		*									TRUE => granted, FALSE => denied, NULL => get all
		*      running:			boolean		if we only are getting wokflows that are running (default to FALSE)
		*      dataCallback:	function	Custom callback function
		* }
		*/
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

		/**
		* This function will let user edit content of Editable File type assets
		* File type that can be edited - css_file, xml_file, css_file, text_file, xsl_file, js_file
		* User needs to acquire locks before being able to edit the file
		*
		* @param object		options       JSON string of options
		* { 
		*      asset_id:		string/int	Id of the asset to update content for
		*      content:			string		new content of the asset
		*      dataCallback:	function	Custom callback function
		* }
		*/
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

		/**
		* This function will let user import assets into Matrix from a structured XML file
		* For example for xml file look under System Tools > Import Assets from XML Tool > example.xml
		*
		* @param object		options       JSON string of options
		* { 
		*      asset_id:		string/int	Asset under which the assets are to be imported under
		*      filePath:		string		path to file on the file system
		*      dataCallback:	function	Custom callback function
		* }
		*/
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

			this._doPost(fnName,[
				['id', options.asset_id],
				['filePath', options.filePath]
			],options);

		},// end importAssetsFromXML

			<?php
		}//end if(import_assets)

		if ($js_api_asset->attr('get_roles')) {
			?>

		/**
		* Returns an array of roles and the users/groups which can perform it
		*
		* @param object		options       JSON string of options
		* { 
		*      asset_id:			string/int	the assetid of the asset the role is applied to
		*      role_id:				string/int	the assetid of the the role that is applied
		*      user_id:				string/int	the assetid of the user performing the role
		*      include_assetid:		boolean		whether to include the assetid in the returned array
		*      include_globals:		boolean		whether to query the role view which includes
		*										expanded global roles as individual users
		*      expand_groups:		boolean		when TRUE, any groups defined within a role will be
		*										replaced with the userids in that group
		*										If FALSE, return the groupids
		*      inc_dependants:		boolean		If false it will filter out the dependant assets
		*      include_parents:		boolean		When userid is specified. This will include the roles applied directly to the user and indirectly to user parent groups.
		*      type_codes:		array		When include_assetid is TRUE. This is a type code filter for returned assetids.
		*      strict_type_code:		boolean		Use strict type code for type code filter
		*      dataCallback:		function	Custom callback function
		* }
		*/
		getRoles: function(options)
		{
			var fnName = 'getRoles';
			if (arguments.length === 0) {
				return {
					// Set the default values
					defaults: {
						asset_id:           '',
						role_id:            '',
						user_id:            '',
						include_assetid:    0,
						include_globals:    0,
						expand_groups:      0,
						inc_dependants:     0,
						include_parents:    0,
						type_codes:      [],
						strict_type_code:     1,
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
			
			options.include_assetid = (convertToBoolean(options.include_assetid)) ? 1 : 0;
			options.include_globals = (convertToBoolean(options.include_globals)) ? 1 : 0;
			options.expand_groups = (convertToBoolean(options.expand_groups)) ? 1 : 0;
			options.inc_dependants = (convertToBoolean(options.inc_dependants)) ? 1 : 0;
			options.include_parents = (convertToBoolean(options.include_parents)) ? 1 : 0;
			options.strict_type_code = (convertToBoolean(options.strict_type_code)) ? 1 : 0;

			this._doPost(fnName,[
				['id', options.asset_id],
				['roleid', options.role_id],
				['userid', options.user_id],
				['include_assetid', options.include_assetid],
				['include_globals', options.include_globals],
				['expand_groups', options.expand_groups],
				['inc_dependants', options.inc_dependants],
				['include_parents', options.include_parents],
				['type_codes', options.type_codes],
				['strict_type_code', options.strict_type_code],
			],options);

		},// end getRoles

			<?php
		}//end if(get_roles)

		if ($js_api_asset->attr('execute_html_tidy')) {
			?>

		/**
		* Performs HTMLTidy on the passed content and returns cleaned up code
		*
		* @param object		options       JSON string of options
		* { 
		*      content:			string		the string content that needs to be cleaned up
		*      dataCallback:	function	Custom callback function
		* }
		*/
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


		if ($js_api_asset->attr('clone_asset')) {
			?>

		/**
		* Clones the given asset under the parent passed in
		*
		* @param object		options       JSON string of options
		* {
		*      asset_id:		string/int	asset under which the assets are to be imported under
		*      new_parent		string/int	assetid of the parent to clone under
		*      clone_num		int			number of clones to create under new parent (default is 1)
		*      new_position		int			position to place the newly cloned asset at (default at the bottom)
		*      link_type		string		type of links we want to create for cloned asset (default is SQ_LINK_TYPE_1)
		*									Valid Values - SQ_LINK_TYPE_1, SQ_LINK_TYPE_2, SQ_LINK_TYPE_3
		*      link_value		string		the value of new link we want (default '')
		*      dataCallback:	function	Custom callback function
		* }
		*/
		cloneAsset: function(options)
		{
			var fnName = 'cloneAsset';
			if (arguments.length === 0) {
				return {
					// Set the default values
					defaults: {
						asset_id:           null,
						new_parent:         null,
						clone_num:          1,
						new_position:       -1,
						link_type:          'SQ_LINK_TYPE_1',
						link_value:         '',
						dataCallback:       function(){}
					},

					// Set the required arguments
					required: ['asset_id', 'new_parent']
				};
			}// end if

			// Extract some configuration and use it to build
			// parameters for the post call
			var fnConfig = this[fnName].call(this);
			options = this._options(fnConfig.required,fnConfig.defaults,options);

			this._doPost(fnName,[
					['id', options.asset_id],
					['new_parent', options.new_parent],
					['clone_num', options.clone_num],
					['new_position', options.new_position],
					['link_type', options.link_type],
					['link_value', options.link_value]
			],options);

		},// end cloneAsset

			<?php
		}//end if(clone_asset)


		if ($js_api_asset->attr('show_diff')) {
			?>

		/**
		*
		* gets the content (with or without paint layout) and check for the difference
		*
		* @param object		options       JSON string of options
		* {
		*      assetid_1:		string/int	assetid of first asset
		*      assetid_2:		string/int	assetid of second asset
		*      paint_layout_1:	string		assetid of the paint layout you want to use on asset_1
		*      paint_layout_2:	string		assetid of the paint layout you want to use on asset_2
		*      dataCallback:	function	Custom callback function
		* }
		*/
		showDifference: function(options)
		{
			var fnName = 'showDifference';
			if (arguments.length === 0) {
				return {
					// Set the default values
					defaults: {
						assetid_1:           null,
						assetid_2:           null,
						paint_layout_1:      '',
						paint_layout_2:      '',
						dataCallback:        function(){}
					},

					// Set the required arguments
					required: ['assetid_1', 'assetid_2']
				};
			}// end if

			// Extract some configuration and use it to build
			// parameters for the post call
			var fnConfig = this[fnName].call(this);
			options = this._options(fnConfig.required,fnConfig.defaults,options);

			this._doPost(fnName,[
					['id', options.assetid_1],
					['assetid_2', options.assetid_2],
					['paint_layout_1', options.paint_layout_1],
					['paint_layout_2', options.paint_layout_2]
			],options);

		},// end showDifference

			<?php
		}//end if(show_diff)


		if ($js_api_asset->attr('get_context')) {
			?>

		/**
		* Returns the context we should be using based on the
		* settings on the Context Configuration screen
		*
		* @param object		options       JSON string of options
		* {
		*      all_info:		boolean		wheather to get all information of the context or
		 *                                  just get the id/name pairs(by default set to FALSE)
		*      dataCallback:	function	Custom callback function
		* }
		*/
		getAlternateContext: function(options)
		{
			var fnName = 'getAlternateContext';
			if (arguments.length === 0) {
				return {
					// Set the default values
					defaults: {
						all_info:            0,
						dataCallback:        function(){}
					},

					// Set the required arguments
					required: ['all_info']
				};
			}// end if

			// Extract some configuration and use it to build
			// parameters for the post call
			var fnConfig = this[fnName].call(this);
			options = this._options(fnConfig.required,fnConfig.defaults,options);

			options.all_info = (convertToBoolean(options.all_info)) ? 1 : 0;

			this._doPost(fnName,[
					['all_info', options.all_info]
			],options);

		},// end getAlternateContext


		/**
		* Returns the current context active on the system
		*
		* @param object		options       JSON string of options
		* {
		*      all_info:		boolean		wheather to get all information of the context or
		*                                   just get the id/name pairs(by default set to FALSE)
		*      dataCallback:	function	Custom callback function
		* }
		*/
		getCurrentContext: function(options)
		{
			var fnName = 'getCurrentContext';
			if (arguments.length === 0) {
				return {
					// Set the default values
					defaults: {
						all_info:            0,
						dataCallback:        function(){}
					},

					// Set the required arguments
					required: ['all_info']
				};
			}// end if

			// Extract some configuration and use it to build
			// parameters for the post call
			var fnConfig = this[fnName].call(this);
			options = this._options(fnConfig.required,fnConfig.defaults,options);

			options.all_info = (convertToBoolean(options.all_info)) ? 1 : 0;

			this._doPost(fnName,[
					['all_info', options.all_info]
			],options);

		},// end getCurrentContext


		/**
		* Returns details of all the contexts in a system
		*
		* @param object		options       JSON string of options
		* {
		*      all_info:		boolean		wheather to get all information of the context or
		*                                   just get the id/name pairs(by default set to FALSE)
		*      dataCallback:	function	Custom callback function
		* }
		*/
		getAllContexts: function(options)
		{
			var fnName = 'getAllContexts';
			if (arguments.length === 0) {
				return {
					// Set the default values
					defaults: {
						all_info:            0,
						dataCallback:        function(){}
					},

					// Set the required arguments
					required: ['all_info']
				};
			}// end if

			// Extract some configuration and use it to build
			// parameters for the post call
			var fnConfig = this[fnName].call(this);
			options = this._options(fnConfig.required,fnConfig.defaults,options);

			options.all_info = (convertToBoolean(options.all_info)) ? 1 : 0;

			this._doPost(fnName,[
					['all_info', options.all_info]
			],options);

		},// end getAllContexts

			<?php
		}//end if(get_context)
		
		if ($js_api_asset->attr('set_context')) {
			?>

		/**
		* Set the System to the Passed context
		*
		* @param object		options       JSON string of options
		* {
		*      assetid_1:		string/int	assetid of first asset
		*      dataCallback:	function	Custom callback function
		* }
		*/
		setContext : function(options)
		{
			var fnName = 'setContext';
			if (arguments.length === 0) {
				return {
					// Set the default values
					defaults: {
						context_id:          null,
						dataCallback:        function(){}
					},

					// Set the required arguments
					required: ['context_id']
				};
			}// end if

			// Extract some configuration and use it to build
			// parameters for the post call
			var fnConfig = this[fnName].call(this);
			options = this._options(fnConfig.required,fnConfig.defaults,options);

			this._doPost(fnName,[
					['context_id', options.context_id]
			],options);

		},//end setContext


		/**
		* Restores the previous context
		*
		* @param object		options       JSON string of options
		* {
		*      assetid_1:		string/int	assetid of first asset
		*      dataCallback:	function	Custom callback function
		* }
		*/
		restoreContext : function(options)
		{
			var fnName = 'restoreContext';
			if (arguments.length === 0) {
				return {
					// Set the default values
					defaults: {
						dataCallback:        function(){}
					},

					// Set the required arguments
					required: []
				};
			}// end if

			// Extract some configuration and use it to build
			// parameters for the post call
			var fnConfig = this[fnName].call(this);
			options = this._options(fnConfig.required,fnConfig.defaults,options);

			this._doPost(fnName,[],options);

		},//end restoreContext

			<?php
		}//end if(set_context)

		if ($js_api_asset->attr('get_metadata_schema')) {
			?>

		/**
		*
		* gets the content (with or without paint layout) and check for the difference
		*
		* @param object		options       JSON string of options
		* {
		*      assetid:		string/int		the asset to get the info of the schemas applied to
		*      granted:		boolean			type of access : TRUE = applied, FALSE = denied
		*      cascades:	boolean			does this schema cascade to newly created assets?
		*      dataCallback:	function	Custom callback function
		* }
		*/
		getMetadataSchema: function(options)
		{
			var fnName = 'getMetadataSchema';
			if (arguments.length === 0) {
				return {
					// Set the default values
					defaults: {
						assetid:		null,
						granted:		1,
						cascades:		null,
						dataCallback:	function(){}
					},

					// Set the required arguments
					required: ['assetid']
				};
			}// end if

			if (options.asset_id !== null && typeof(options.asset_id) !== "undefined") {
				options.assetid = options.asset_id;
			}

			// Extract some configuration and use it to build
			// parameters for the post call
			var fnConfig = this[fnName].call(this);
			options = this._options(fnConfig.required,fnConfig.defaults,options);

			options.granted = (convertToBoolean(options.granted)) ? 1 : 0;

			this._doPost(fnName,[
					['id', options.assetid],
					['granted', options.granted],
					['cascades', options.cascades]
			],options);

		},// end getMetadataSchema

			<?php
		}//end if(get_metadata_schema)


		if ($js_api_asset->attr('edit_metadata_schema')) {
			?>

		/**
		*
		* Sets or unsets the passed metadata schema on the asset
		*
		* @param object		options       JSON string of options
		* {
		*      assetid:			string/int		the asset to edit schema on
		*      schemaid:		string/int		the metadata schema to be applied
		*      set:				boolean			set = 1/TRUE; unset = 0/FALSE
		*      granted:			boolean			type of access : TRUE = applied, FALSE = denied
		*      cascades:		boolean			does this schema cascade to newly created assets?
		*      dataCallback:	function		Custom callback function
		* }
		*/
		editMetadataSchema: function(options)
		{
			var fnName = 'editMetadataSchema';
			if (arguments.length === 0) {
				return {
					// Set the default values
					defaults: {
						assetid:		null,
						schemaid:		null,
						set:			'',
						granted:		1,
						cascades:		null,
						dataCallback:	function(){}
					},

					// Set the required arguments
					required: ['assetid', 'schemaid', 'set']
				};
			}// end if

			if (options.asset_id !== null && typeof(options.asset_id) !== "undefined") {
				options.assetid = options.asset_id;
			}

			// Extract some configuration and use it to build
			// parameters for the post call
			var fnConfig = this[fnName].call(this);
			options = this._options(fnConfig.required,fnConfig.defaults,options);

			options.set = (convertToBoolean(options.set)) ? 1 : 0;
			options.granted = (convertToBoolean(options.granted)) ? 1 : 0;

			this._doPost(fnName,[
					['id', options.assetid],
					['schemaid', options.schemaid],
					['set', options.set],
					['granted', options.granted],
					['cascades', options.cascades]
			],options);

		},// end editMetadataSchema

			<?php
		}//end if(edit_metadata_schema)


		if ($js_api_asset->attr('create_form_question')) {
			?>

		/**
		*
		* creates question(s) of a type on the passed formid or the form section id
		*
		* @param object		options       JSON string of options
		* {
		*      assetid:			string/int		the asset of the form or form section to
		*										add the question asset
		*      type_code:		string			the type of question to be added (default: Text)
		*      units:			int				number of question to add of a certain type (default:1)
		*      dataCallback:	function		Custom callback function
		* }
		*/
		createQuestion: function(options)
		{
			var fnName = 'createQuestion';
			if (arguments.length === 0) {
				return {
					// Set the default values
					defaults: {
						assetid:		null,
						type_code:		'text',
						units:			1,
						dataCallback:	function(){}
					},

					// Set the required arguments
					required: ['assetid']
				};
			}// end if

			if (options.asset_id !== null && typeof(options.asset_id) !== "undefined") {
				options.assetid = options.asset_id;
			}

			// Extract some configuration and use it to build
			// parameters for the post call
			var fnConfig = this[fnName].call(this);
			options = this._options(fnConfig.required,fnConfig.defaults,options);

			this._doPost(fnName,[
					['id', options.assetid],
					['type_code', options.type_code],
					['units', options.units]
			],options);

		},// end createQuestion


			<?php
		}//end if(create_form_question)


		if ($js_api_asset->attr('update_form_question')) {
			?>

		/**
		*
		* updates a question of the passed formid or the form section id
		*
		* @param object		options       JSON string of options
		* {
		*      assetid:			string/int		the asset of the form or form section to update
		*      question_id:		string			the asset id of the question to update
		*      field_info:		json			The object containing data that needs to be updated
		*										here you can use attribute name as the key and it's
		*										corresponding value (user needs to make sure they are passing correct datatype)
		*      dataCallback:	function		Custom callback function
		* }
		*/
		updateQuestion: function(options)
		{
			var fnName = 'updateQuestion';
			if (arguments.length === 0) {
				return {
					// Set the default values
					defaults: {
						assetid:		null,
						question_id:	'',
						field_info:		{},
						dataCallback:	function(){}
					},

					// Set the required arguments
					required: ['assetid', 'question_id', 'field_info']
				};
			}// end if

			if (options.asset_id !== null && typeof(options.asset_id) !== "undefined") {
				options.assetid = options.asset_id;
			}

			// Extract some configuration and use it to build
			// parameters for the post call
			var fnConfig = this[fnName].call(this);
			options = this._options(fnConfig.required,fnConfig.defaults,options);

			var question_data = '';
			for (field_name in field_info) {
				if (Object.prototype.toString.call(field_info[field_name]) === '[object Array]') {
					question_data = question_data + '&' + field_name + '=' + field_info[field_name].join('\\,');
				} else {
					question_data = question_data + '&' + field_name + '=' + field_info[field_name];
				}
			}

			this._doPost(fnName,[
					['id', options.assetid],
					['question_id', options.question_id + question_data]
			],options);

		},// end updateQuestion


			<?php
		}//end if(update_form_question)


		if ($js_api_asset->attr('remove_form_question')) {
			?>

		/**
		*
		* removes a question of the passed formid or the form section id
		*
		* @param object		options       JSON string of options
		* {
		*      assetid:			string/int		the asset of the form or form section to
		*										remove a question off
		*      question_id:		string			question that needs to be removed
		*      dataCallback:	function		Custom callback function
		* }
		*/
		removeQuestion: function(options)
		{
			var fnName = 'removeQuestion';
			if (arguments.length === 0) {
				return {
					// Set the default values
					defaults: {
						assetid:		null,
						question_id:	'',
						dataCallback:	function(){}
					},

					// Set the required arguments
					required: ['assetid', 'question_id']
				};
			}// end if

			if (options.asset_id !== null && typeof(options.asset_id) !== "undefined") {
				options.assetid = options.asset_id;
			}

			// Extract some configuration and use it to build
			// parameters for the post call
			var fnConfig = this[fnName].call(this);
			options = this._options(fnConfig.required,fnConfig.defaults,options);

			this._doPost(fnName,[
					['id', options.assetid],
					['question_id', options.question_id],
			],options);

		},// end removeQuestion


			<?php
		}//end if(remove_form_question)


		if ($js_api_asset->attr('update_question_order')) {
			?>

		/**
		*
		* updates the question order on the passed formid or the form section id
		*
		* @param object		options       JSON string of options
		* {
		*      assetid:			string/int		the asset of the form or form section to update
		*      sort_order:		array			new order of the questions
		*      dataCallback:	function		Custom callback function
		* }
		*/
		updateQuestionOrder: function(options)
		{
			var fnName = 'updateQuestionOrder';
			if (arguments.length === 0) {
				return {
					// Set the default values
					defaults: {
						assetid:		null,
						sort_order:		[],
						dataCallback:	function(){}
					},

					// Set the required arguments
					required: ['assetid', 'sort_order']
				};
			}// end if

			if (options.asset_id !== null && typeof(options.asset_id) !== "undefined") {
				options.assetid = options.asset_id;
			}

			// Extract some configuration and use it to build
			// parameters for the post call
			var fnConfig = this[fnName].call(this);
			options = this._options(fnConfig.required,fnConfig.defaults,options);

			this._doPost(fnName,[
					['id', options.assetid],
					['sort_order', options.sort_order.join('\\,')],
			],options);

		},// end updateQuestionOrder


			<?php
		}//end if(update_question_order)


		?>

		/**
		* Shortcut function for sending post data
		*/
		_doPost: function(fnName,data,options)
		{
			var self = this;

			// Add the 'type' parameter for as the calling function name
			data.push(['type',fnName]);
			// Add nonce token
			data.push(['nonce_token','<?php echo ($nonce_token);?>']);
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
