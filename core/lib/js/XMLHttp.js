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
* $Id: XMLHttp.js,v 1.3 2005/01/20 13:10:35 brobertson Exp $
*
*/


/**
* Constructor of the XMLHttp object
*
* This object enables communication between web pages and servers using JavaScript.
* Works in Mozilla 1.0, Safari 1.2 and Internet Explorer 5.0+
*
* Usage Examples:
*
*   * To submit a form without refreshing the browser:
*
*        <form id="quiet_form" method="POST" action="http://www.example.com">
*          <!-- form contents -->
*          <input type="button" value="submit" onclick="submitQuietForm()" />
*        </form>
*        <script type="text/javscript" src="XMLHttp.js"></script>
*        <script type="text/javascript">
*           function showOK() {
*              window.status = 'Form submitted';
*              setTimeout("window.status='';", 2000);
*           }
*           function submitQuietForm()
*           {
*              // Tell XMLHttp to submit the form and use the showOK function
*              // to process the response from the server
*              XMLHttp.submitForm("quiet_form", showOK);
*           }
*        </script>
*
*   * To get some content from the server and put it in the document:
*
*        <div id="dynamic_content"></div>
*        <input type="button" value="Get the Latest" onclick="updateDynamicDiv()" />
*        <script type="text/javscript" src="XMLHttp.js"></script>
*        <script type="text/javascript">

*           // override XMLHttp's process function to do what we want
*           XMLHttp.process = function() {
*              document.getElementById('dynamic_content').innerHTML = this.getXML();
*           }
*
*           function updateDynamicDiv()
*           {
*              // tell XMLHttp to retrieve a document, which will be processed
*              // by XMLHttp.process() when it arrives
*              XMLHttp.loadXMLDoc('http://www.timeanddate.com/worldclock/');
*           }
*
*        </script>
*         
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
	* Send request to specified URL using specified method
	*
	* ??? Content should have type "text/xml" ???
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
	* Find out if the browser supports the XMLHttp object
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
	* Get the XMLHttpRequest object
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
	this.process = function()
	{
		alert(this.getXML());

	}//end process


	/**
	* Get value of the specified tag in the response document
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
	* Get list of nodes in the response with the given tag name
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
	* Get value of Element
	*
	* @param	element		DOM element
	*
	* @return String
	* @access public
	*/
	this.valueOf = function(element)
	{
		return element.firstChild.nodeValue;

	}//end valueOf()


	/**
	* Get content of the response
	*
	* @return String
	* @access public
	*/
	this.getXML = function()
	{
		return Global_XMLHttp_Request.responseText;

	}//end getXML()


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


	/**
	* Send form data via XMLHttp
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

	}//end submitForm()

}//end of constructor for class XMLHttp


/**
* Handle onreadystatechange event of request object
*
* @return void
* @access public
*/
function processReqChange() 
{
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

}//end processReqChange()


// global request and XML document objects
var Global_XMLHttp_Request = null;

// if no XMLHttp variable created, create it.
if (typeof XMLHttp == "undefined") var XMLHttp = new TXMLHttp();
