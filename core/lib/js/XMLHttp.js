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
* $Id: XMLHttp.js,v 1.1 2004/11/25 03:43:12 dbaranovskiy Exp $
* $Name: not supported by cvs2svn $
*/

/**
* Constructor of the XMLHttp object
* Object realize communication between web page and server using JavaScript
* and browser abilities.
* Works in Mozilla 1.0, Safari 1.2 and Internet Explorer 5.0
*
* @author	Dmitry Baranovskiy	<dbaranovskiy@squiz.net>
*
* @return object
* @access public
*/
TXMLHttp = function()
{

	this.isIE = false;
	this.responseXML = null;
	this.temp = null;

/**
* sends request and loads content by URL using method
* content should have type "text/xml"
* @param	url				URL
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

		if (window.XMLHttpRequest) {
			Global_XMLHttp_Request = new XMLHttpRequest();
			Global_XMLHttp_Request.onreadystatechange = processReqChange;
			Global_XMLHttp_Request.open(method, url, true);
			if (method == "POST") {
				Global_XMLHttp_Request.setRequestHeader('Content-Type', 'application/x-www-form-urlencoded');
			}
			Global_XMLHttp_Request.send(parameters);
		} else if (window.ActiveXObject) {
			this.isIE = true;
			Global_XMLHttp_Request = new ActiveXObject("Microsoft.XMLHTTP");
			if (Global_XMLHttp_Request) {
				Global_XMLHttp_Request.onreadystatechange = processReqChange;
				Global_XMLHttp_Request.open(method, url, true);
				if (method == "POST") {
					Global_XMLHttp_Request.setRequestHeader('Content-Type', 'application/x-www-form-urlencoded');
				}
				if (parameters == null) {
					Global_XMLHttp_Request.send();
				}
				else Global_XMLHttp_Request.send(parameters);
			}
		}

	}//end loadXMLDoc


/**
* check is browser supports XMLHttp object
*
* @return boolean
* @access public
*/
	this.isBrowserOk = function()
	{
		try {
			return (window.XMLHttpRequest || window.ActiveXObject)
		}
		catch(e) {
			return false;
		}

	}//end isBrowserOk


/**
* return XMLHttpRequest object
*
* @return object
* @access public
*/
	this.request = function()
	{
		return Global_XMLHttp_Request;

	}//end request


/**
* process XML receiving
*
* @return void
* @access public
*/
	this.process = function()
	{
		alert(this.getXML());

	}//end process


/**
* returns value of the node
*
* @param	tag		tag name
*
* @return DOMString
* @access public
*/
	this.getTagValue = function(tag)
	{
		return this.responseXML.getElementsByTagName(tag)[0].firstChild.nodeValue;

	}//end getTagValue


/**
* return list of nodes in current tag
*
* @param	tag		tag name
*
* @return NodeList
* @access public
*/
	this.getTagValues = function(tag)
	{
		return this.responseXML.getElementsByTagName(tag);

	}//end getTagValues


/**
* returns value of Element
*
* @param	element		DOM element
*
* @return String
* @access public
*/
	this.valueOf = function(element)
	{
		return element.firstChild.nodeValue;

	}//end valueOf


/**
* returns content of the response
*
* @return String
* @access public
*/
	this.getXML = function()
	{
		return Global_XMLHttp_Request.responseText;

	}//end getXML


/**
* will be called on error in processing (could be overwriten)
*
* @param	message		error message
*
* @return String
* @access public
*/
	this.processError = function(message)
	{
		//do nothing;
	}//end processError


/**
* will be called on error in receiving (could be overwriten)
*
* @param	message		error message
* @param	status		status code
*
* @return String
* @access public
*/
	this.receiveError = function(message, status)
	{
		//do nothing;
	}//end receiveError


/**
* sends form data via XMLHttp
*
* @param	formid		form id
* @param	func		handler
*
* @return void
* @access public
*/
	this.submitForm = function(formid, func)
	{
		var form = document.getElementById(formid);
		var post = "";
		for (var i = 0; i < form.length; i++) {
			post += form.elements[i].id + "=" + form.elements[i].value + "&";
		}

		if (typeof func == "function") {
			this.temp = this.process;
			this.process = func;
		}

		this.loadXMLDoc(form.action, post, form.method);

	}//end submitForm
}


/**
* handle onreadystatechange event of request object
*
* @return void
* @access public
*/
function processReqChange() {
		// only if req shows "loaded"
		if (Global_XMLHttp_Request.readyState == 4) {
			// only if "OK"
			if (Global_XMLHttp_Request.status == 200) {
				XMLHttp.responseXML = Global_XMLHttp_Request.responseXML;
				try {
					XMLHttp.process();
					if (XMLHttp.temp != null) {
						XMLHttp.process	= XMLHttp.temp;
						XMLHttp.temp	= null;
					}
				}
				catch (e) {
					XMLHttp.processError(e.message);
				}
			 } else {
				 XMLHttp.receiveError(Global_XMLHttp_Request.statusText, Global_XMLHttp_Request.status);
			 }
		}

	}


// global request and XML document objects
var Global_XMLHttp_Request = null;

// if no XMLHttp variable created, create it.
if (typeof XMLHttp == "undefined") var XMLHttp = new TXMLHttp();
