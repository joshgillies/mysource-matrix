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


function getRequestParts(soap_server_url)
{
	var request_parts	= { head: '<?xml version="1.0" encoding="UTF-8"?><SOAP-ENV:Envelope xmlns:SOAP-ENV="http://schemas.xmlsoap.org/soap/envelope/" xmlns:ns1="'+soap_server_url+'"><SOAP-ENV:Body>', 
							tail: '</SOAP-ENV:Body></SOAP-ENV:Envelope>'};
	return request_parts;
	
}//end getRequestParts()


function constructSOAPRequest(body, soap_server_url)
{
	var request_parts	= getRequestParts(soap_server_url);
	var soapMessage	= request_parts['head']+body+request_parts['tail'];
	return soapMessage;
	
}//end constructSOAPRequest()


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


function send(location, soapRequest)
{
	var xmlHttpObj	= constructXmlHttpObj();
	xmlHttpObj.open("POST", location, false);

	xmlHttpObj.setRequestHeader("Content-Type", "text/xml");

	xmlHttpObj.send(soapRequest);

	xmlHttpObj.onreadystatechange=function()
	{
		if (xmlHttp.readyState == 4) {
			var response	= xmlHttp.responseXML;
			console.info(response);
		}
	}
	
}//end send()


function multiple_elements_to_string(array_elements, element_name)
{
	var result_str	= '';
	for (var i = 0; i < statuses.length; i++) {
		result_str	+= '<'+element_name+'>'+statuses[i]+'</'+element_name+'>';
	}//end for
	
	return result_str;
	
}//end multiple_elements_to_string()
