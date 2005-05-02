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
* $Id: JsHttpConnector.js,v 1.5 2005/05/02 02:45:20 tbarrett Exp $
*
*/


/**
* Handle a change in an XMLHttpRequest request object's state by finding the relevant JsHttpConnector object
*
* @access private
* @return void
*/
function _processJsHttpStateChange(id)
{
	var thread = _allJsHttpConnectorThreads[id];
	if (thread.requestObject.readyState == 4) {
		// state is "loaded"
		if (thread.requestObject.status == 200) {
			// status is "OK"
			//try {
				thread.process(thread.requestObject.responseText, thread.requestObject.responseXML);
			//} catch (e) {
				//thread._handleReceiveError(e.message);
			//}
		 } else {
			 thread._handleReceiveError(thread.requestObject.statusText, thread.requestObject.status);
		 }
		delete( _allJsHttpConnectorThreads["t"+thread.id]);
	}

}//end _processJsHttpStateChange()


/**
* Get the next thread id to use
*
* @access public
* @return void
*/
function getNextJsHttpThreadId()
{
	_numJsHttpConnectorThreads++;
	if (_numJsHttpConnectorThreads == MAX_JS_HTTP_THREADS) {
		_numJsHttpConnectorThreads = 0;
	}
	return _numJsHttpConnectorThreads;

}//end getNextJsHttpThreadId()


/**
* Constructor for a JsHttpConnector object
*
* This object wraps around the browser's JsHttpConnectorRequest object and enables
* communication between web pages and servers using JavaScript.
*
* XMLHttpRequest has a number of functions that support XML parsing of the
* server's response; you can get to these by using
* JsHttpConnector.request().xmlFunctionBlah()
*
* Works in Mozilla 1.0, Safari 1.2 and Internet Explorer 5.0+
*
* See matrix_root/docs/example_code/JsHttpConnectorDemo.html for usage
*
* @author	Tom Barrett <tbarrett@squiz.net>, based on previous work by Dmitry Baranovskiy
*
* @return object
* @access public
*/
TJsHttpConnector = function() {

	// Return false if browser support is not there
	try {
		if ((!window.ActiveXObject) && (!window.XMLHttpRequest)) {
			//alert('Browser does not support XMLHttpRequest');
			return false;
		}
	} catch (e) {
		//alert('Browser does not support XMLHttpRequest');
		return false;
	}


	/**
	* Submit a form
	*
	* @param	formid		ID of the form to send
	* @param	func		Function to call to process the server's response
	*
	* @return void
	* @access public
	*/
	this.submitForm = function(formid, func)
	{
		var thread = new JsHttpConnectorThread();
		thread.submitForm(formid, func);
	}


	/**
	* Submit a simple GET request
	*
	* @param	url		URL to submit to
	* @param	func	Function to call to process the server's response
	*
	* @return void
	* @access public
	*/
	this.submitRequest = function(url, func)
	{
		var thread = new JsHttpConnectorThread();
		thread.submitRequest(url, func);
	}


}//end class


/**
* Constructor for a JsHttpConnectorThread object.  These objects are created by
* JsHttpConnector to submit and process a single transaction with the server.
*/
function JsHttpConnectorThread()
{
	// instance variables for this object
	this.isIE = (typeof window.ActiveXObject != "undefined");
	this.requestObject = this.isIE ? new ActiveXObject("Microsoft.XMLHTTP") : new XMLHttpRequest();
	this.id = getNextJsHttpThreadId();

	// register this object with the global list
	_allJsHttpConnectorThreads["t"+this.id] = this;

	
	/**
	* Submit a form
	*
	* @param	formid		ID of the form to send
	* @param	func		Function to call to process the response
	*
	* @return void
	* @access public
	*/
	this.submitForm = function(formid, func)
	{
		var form = document.getElementById(formid);
		if (null === form) {
			this._handleTransmitError('Could not find form with ID '+formid+' to submit');
			return;
		}
		var post = "";
		for (var i = 0; i < form.length; i++) {
			post += form.elements[i].name + "=" + form.elements[i].value + "&";
		}

		if (typeof func == "function") {
			this.process = func;
		}
		this._submitRequest(form.action, post, form.method);

	}//end submitForm()


	/**
	* Send a simple GET request using JsHttpConnector
	*
	* @param	url		Server to send request to
	* @param	func	Function to call to process response
	*
	* @return void
	* @access public
	*/
	this.submitRequest = function(url, func)
	{
		if (typeof func == "function") {
			this.process = func;
		}
		this._submitRequest(url);

	}//end submitRequest()


	/**
	* Send request to specified URL using specified method
	*
	* If you want to parse the result as XML, the server response must include header
	* Content-type: text/xml
	*
	* @param	url				URL to send request to
	* @param	parameters		parameters of the request ['var1=value1&var2=value2&var3=...']
	* @param	method			'POST' or 'GET'
	*
	* @return void
	* @access private
	*/
	this._submitRequest = function(url, parameters, method)
	{
		this.requestObject.onreadystatechange = this.getStateChangeHandler();
		if (typeof parameters == "undefined" || parameters == "") parameters = null;
		if (typeof method == "undefined" || method == "") method = "GET";
		method = method.toUpperCase();

		if (method == "GET" && parameters != null) {
			url += "?" + parameters;
			parameters = null;
		}

		this.requestObject.open(method, url, true);
		if (method == "POST") {
			this.requestObject.setRequestHeader('Content-Type', 'application/x-www-form-urlencoded');
		}
		if (this.isIE && (parameters === null)) {
			this.requestObject.send();
		} else {
			this.requestObject.send(parameters);
		}

	}//end _submitRequest


	/**
	* Process server response - will be overridden
	*
	* @return void
	* @access public
	*/
	this.process = function(responseText, responseXML)
	{
		return;

	}//end process


	/**
	* Handle a transmission error
	*
	* This will be called on error in processing
	*
	* @param	message		error message
	*
	* @return String
	* @access private
	*/
	this._handleTransmitError = function(message)
	{
		alert("JSHTTPCONNECTOR TRANSMIT ERROR: \n"+message);

	}//end _handleTransmitError()


	/**
	* Handle a receive error
	*
	* This will be called on error in processing
	*
	* @param	message		error message
	*
	* @return String
	* @access private
	*/
	this._handleReceiveError = function(message, status)
	{
		var msg = "JSHTTPCONNECTOR RECEIVE ERROR: \n"+message;
		if (typeof status != 'undefined') msg += "\n(HTTP Status = "+status;
		alert(msg);

	}//end _handleReceiveError()


	/**
	* Get a function to attach to our XMLHttpRequest object as its state handler
	*
	* @return object function
	* @access private
	*/
	this.getStateChangeHandler = function()
	{
		return new Function('_processJsHttpStateChange("t'+this.id+'")');
	}

}//end class

var MAX_JS_HTTP_THREADS = 256;

if (typeof _allJsHttpConnectorThreads == "undefined") {
	// collection of all JsHttpConnector threads
	var _allJsHttpConnectorThreads = Array();
	var _numJsHttpConnectorThreads = 0;
}

if (typeof JsHttpConnector == "undefined") {
	// the singleton/"static" JsHttpConnector object
	var JsHttpConnector = new TJsHttpConnector();
}
