// This file contain the basic soap envelope to be used to construct the SOAP request together with individual APIs SOAP request body.


// Example of Cloning an asset
/*
// This is the location of the SOAP Server which support the CloneAsset method
var location	= "http://192.168.1.44/hnguyen_testing_dev/_web_services/asset_service";
var wsdl		= location+"?WSDL";
var	soapBody	= CloneAsset('74', '56', '59', '1');
var soapRequest	= constructSOAPRequest(soapBody, location);
send(wsdl, soapRequest);
*/


/**
 * This function return the SOAP Envelopve used to encapsulate the SOAP message to be sent to the web services server
 *
 * @param soap_server_url	string	the URL of the SOAP Server
 * @return string
 */
function getRequestParts(soap_server_url)
{
	var request_parts	= { head: '<?xml version="1.0" encoding="UTF-8"?><SOAP-ENV:Envelope xmlns:SOAP-ENV="http://schemas.xmlsoap.org/soap/envelope/" xmlns:ns1="'+soap_server_url+'"><SOAP-ENV:Body>', 
							tail: '</SOAP-ENV:Body></SOAP-ENV:Envelope>'};
	return request_parts;
	
}//end getRequestParts()


/**
 * This function construct the SOAP Request to be sent to the SOAP Server
 * 
 * @param body				string	the body of the SOAP request
 * @param soap_server_url	string	the URL of the SOAP Server
 * @return string
 */
function constructSOAPRequest(body, soap_server_url)
{
	var request_parts	= getRequestParts(soap_server_url);
	var soapMessage	= request_parts['head']+body+request_parts['tail'];
	return soapMessage;
	
}//end constructSOAPRequest()


/**
 * This function construct the XMLHttp Ojbect
 * 
 * @return
 */
function constructXmlHttpObj()
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


/**
 * This function send the SOAP Request to the server to be processed
 *
 * @param host_location
 * @param soapRequest
 * @return
 */
function send(host_location, soapRequest, callback_fnc)
{
	var xmlHttpObj	= constructXmlHttpObj();
	xmlHttpObj.onreadystatechange=function()
	{
		if (xmlHttpObj.readyState == 4) {
			var response	= xmlHttpObj.responseXML;
			if (typeof(callback_fnc) == 'function') {
				callback_fnc(response);
			} else {
				eval(callback_fnc+'(response)');
			}
		}
	}

	xmlHttpObj.open("POST", host_location, true);

	xmlHttpObj.setRequestHeader("Content-Type", "text/xml");

	xmlHttpObj.send(soapRequest);

}//end send()


/**
 * This function break an array of element down into a XML list of elements
 * 
 * @param array_elements array	An Array of elements to be break down into XML
 * @param element_name	string	The name of the element in XML
 * @return
 */
function multiple_elements_to_string(array_elements, element_name)
{
	var result_str	= '';
	for (var i = 0; i < array_elements.length; i++) {
		result_str	+= '<'+element_name+'>'+array_elements[i]+'</'+element_name+'>';
	}//end for
	
	return result_str;
	
}//end multiple_elements_to_string()


/**
 * This function is used to include other JS library file from different services
 * 
 */
function include_service_lib(service_name, system_root)
{
	var file_name	= service_name+'_service_requests.js';
	var full_path	= system_root+'/__data/asset_types/soap_api_'+service_name+'_service/'+file_name;
	document.write('<script type="text/javascript" src="'+ full_path + '"></script>'); 
	
}//end include_service_lib


function get_current_system_root(system_roots)
{	
	var host_location	= window.location.toString();
	var re = new RegExp('^(?:f|ht)tp(?:s)?\://([^/]+)', 'im');
	var hostname	= host_location.match(re)[1].toString();

	var system_root	= '';
	for (var root in system_roots) {
		var host_match	= system_roots[root].match(hostname);
		if (host_match !== null) {
			system_root	= system_roots[root];
			break;
		}//end if
	}//end for

	return system_root;

}//end get_current_system_root()
