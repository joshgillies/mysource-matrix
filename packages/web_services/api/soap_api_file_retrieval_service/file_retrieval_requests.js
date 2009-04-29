/**
 * This file contain all the SOAP Body used to request the File Retrieval Service API.
 * 
 */

/**
* Description: This operation allow transfer of file with content base64 encoded
*
* @param array  $request	The request information
* <pre>
* Array (
*		 'AssetID'	 => [The AssetID of the file to download],
*        )
* </pre>
*
* @return void
* @access public
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
* Description: This operation return the information about the file based on the assetid
*
* @param array  $request	The request information
* <pre>
* Array (
*		 'AssetID'	 => [The AssetID of the file to get information for],
*        )
* </pre>
*
* @return void
* @access public
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
* Description: This operation allow upload of file into an existing File Asset
*
* @param array  $request	The request information
* <pre>
* Array (
*		'AssetID'			=> [The AssetID of the file asset to upload the file into],
* 		'FileContentBase64'	=> [The content of the file base64 encoded],
* 		'FileName'			=> [The name of the file being uploaded],
*        )
* </pre>
*
* @return void
* @access public
*/
function Upload(assetid)
{
	var soapBody	= "\
<ns1:Upload>\
<AssetID>"+assetid+"</AssetID>\
</ns1:Upload>";

	return soapBody;

}//end Upload
