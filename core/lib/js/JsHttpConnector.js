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
* $Id: JsHttpConnector.js,v 1.4 2005/03/24 00:48:46 tbarrett Exp $
*
*/

/**
* Constructor of the JsHttpConnector object
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
* @author	Dmitry Baranovskiy	<dbaranovskiy@squiz.net>
*
* @return object
* @access public
*/
TJsHttpConnector = function()
{

	this.isIE = false;
	this.responseXML = null;
	this.temp = null;


	/**
	* Submit a form using JsHttpConnector
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
		var post = "";
		for (var i = 0; i < form.length; i++) {
			post += form.elements[i].name + "=" + form.elements[i].value + "&";
		}

		if (typeof func == "function") {
			this.temp = this.process;
			this.process = func;
		}
		this.loadXMLDoc(form.action, post, form.method);

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
			this.temp = this.process;
			this.process = func;
		}
		this.loadXMLDoc(url);
	}


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
	* @access public
	*/
	this.loadXMLDoc = function(url, parameters, method)
	{
		if (typeof parameters == "undefined" || parameters == "") parameters = null;
		if (typeof method == "undefined" || method == "") method = "GET";
		else method = method.toUpperCase();

		if (method == "GET" && parameters != null) {
			url += "?" + parameters;
			parameters = null;
		}

		if (Global_XMLHttp_Request) {
			Global_XMLHttp_Request.open(method, url, true);
			if (method == "POST") {
				Global_XMLHttp_Request.setRequestHeader('Content-Type', 'application/x-www-form-urlencoded');
			}
			if (this.isIE) {
				if (parameters == null) {
					Global_XMLHttp_Request.send();
				}
				else Global_XMLHttp_Request.send(parameters);
			} else {
				Global_XMLHttp_Request.send(parameters);
			}
		}

	}//end loadXMLDoc


	/**
	* See if the browser supports HttpRequest
	*
	* @return boolean
	* @access public
	*/
	this.isBrowserOk = function()
	{
		try {
			if (window.JsHttpConnectorRequest) return true;
			if (window.ActiveXObject) return true;
			return false;
		}
		catch(e) {
			return false;
		}

	}//end isBrowserOk


	/**
	* Get the native XMLHttp_Request object
	*
	* @return object
	* @access public
	*/
	this.request = function()
	{
		return Global_XMLHttp_Request;

	}//end request


	/**
	* Process received XML
	*
	* @return void
	* @access public
	*/
	this.process = function(responseText)
	{
		return;

	}//end process


	/**
	* Get content of the response
	*
	* @return String
	* @access public
	*/
	this.getResponseText = function()
	{
		return Global_XMLHttp_Request.responseText;

	}//end getResponseText()


	/**
	* Handle a processing error
	*
	* This will be called on error in processing (could be overwriten)
	*
	* @param	message		error message
	*
	* @return String
	* @access public
	*/
	this.processError = function(message)
	{
		//do nothing;

	}//end processError()


	/**
	* Handle a receiving error
	*
	* Will be called on error in receiving (could be overwriten)
	*
	* @param	message		error message
	* @param	status		status code
	*
	* @return String
	* @access public
	*/
	this.receiveError = function(message, status)
	{
		// do nothing;

	}//end receiveError()


	this.initGlobal = function()
	{
		if (window.XMLHttpRequest) {
			Global_XMLHttp_Request = new XMLHttpRequest();
			Global_XMLHttp_Request.onreadystatechange = processJsHttpStateChange;
		} else if (window.ActiveXObject) {
			this.isIE = true;
			Global_XMLHttp_Request = new ActiveXObject("Microsoft.XMLHTTP");
			Global_XMLHttp_Request.onreadystatechange = processJsHttpStateChange;
		}
	}//end initGlobal()

	this.initGlobal();

}//end of constructor for class JsHttpConnector


/**
* Handle onreadystatechange event of request object
*
* @return void
* @access public
*/
function processJsHttpStateChange()
{
	// only if req shows "loaded"
	if (Global_XMLHttp_Request.readyState == 4) {
		// only if "OK"
		if (Global_XMLHttp_Request.status == 200) {
			JsHttpConnector.responseXML = Global_XMLHttp_Request.responseXML;
			try {
				JsHttpConnector.process(JsHttpConnector.getResponseText());
				if (JsHttpConnector.temp != null) {
					JsHttpConnector.process	= JsHttpConnector.temp;
					JsHttpConnector.temp	= null;
				}
			}
			catch (e) {
				JsHttpConnector.processError(e.message);
			}
		 } else {
			 JsHttpConnector.receiveError(Global_XMLHttp_Request.statusText, Global_XMLHttp_Request.status);
		 }
	}

}//end processJsHttpStateChange()

// global request and XML document object
var Global_XMLHttp_Request = null;

// if no JsHttpConnector variable created, create it.
if (typeof JsHttpConnector == "undefined") var JsHttpConnector = new TJsHttpConnector();