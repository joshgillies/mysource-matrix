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
* $Id: ajax.js,v 1.3 2012/08/30 00:59:19 ewang Exp $
*
*/

function constructXmlHttpOjb()
{
	var xmlHttp;

	try {
		// FF, Opera, Safari
		xmlHttp = new XMLHttpRequest();
	} catch (e) {
		try {
			// IE
			xmlHttp = new ActiveXObject("Msxml2.XMLHTTP");
		} catch (e) {
			try {
				xmlHttp = new ActiveXObject("Microsoft.XMLHTTP");
			} catch (e) {
				alert("Your browser does not support AJAX");
				return false;
			}
		}
	}//end try catch

	return xmlHttp;
}

function sendRequest(url, call_back_func)
{
	var xmlHttp	= constructXmlHttpOjb();
	var response = null;
	xmlHttp.onreadystatechange=function()
	{
		if (xmlHttp.readyState == 4) {
			response	= xmlHttp.responseText;
			eval(call_back_func+'(response)');
		}
	}
	xmlHttp.open("GET", url, true);
	xmlHttp.send(null);

}//end sendRequest()


function updateSynchInterface(response)
{
	var update_text = document.getElementById('update_text');
	if (response == 1) {
		update_text.style.color	= 'green';
		update_text.innerHTML = 'Successfully Synchronized';
	} else if (response == 0) {
		update_text.style.color	= 'red';
		update_text.innerHTML = 'No Records Available For Synchronization';
	}//end else if
}//end updateSynchInterface()


function updateCheckInterface(response)
{
	var update_text = document.getElementById('update_text');
	eval(response);
	if (numUpdate !== false) {
		if (numUpdate === 0) {
			update_text.style.color	= 'green';
			update_text.innerHTML = 'No Update Is Required';
		} else {
			update_text.style.color	= 'red';
			update_text.innerHTML = numUpdate+' out-of-date Records';
		}//end else
	} else {
		update_text.style.color	= 'red';
		update_text.innerHTML = 'Unable to check for update. Your cache might be turned off.';
	}//end else

}//end updateInterface()
