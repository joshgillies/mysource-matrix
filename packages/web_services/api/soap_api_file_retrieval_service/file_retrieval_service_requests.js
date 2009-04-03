/**
 * This file contain all the SOAP Body used to request the File Retrieval Service API.
 * 
 */

/**
 * This operation allow transfer of file with content base64 encoded
 */
function Download(assetid)
{
	var soapBody	= "\
<ns1:Download>\
<AssetID>"+assetid+"</AssetID>\
</ns1:Download>";
	return soapBody;
}//end Download


/**
 * This operation return the information about the file based on the assetid
 */
function GetFileInformation(assetid)
{
	var soapBody	= "\
<ns1:GetFileInformation>\
<AssetID>"+assetid+"</AssetID>\
</ns1:GetFileInformation>";
	return soapBody;
}//end GetFileInformation


/**
 * This operation allow upload of file into an existing File Asset
 */
function Upload(assetid)
{
	var soapBody	= "\
<ns1:Upload>\
<AssetID>"+assetid+"</AssetID>\
</ns1:Upload>";
	return soapBody;
}//end Upload