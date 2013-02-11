/**
 * This file contain all the SOAP Body used to request the System Service API.
 * 
 */


/**
* Description: This operation will start the workflow for the asset in query
*
* @param string  $request  The request information
* <pre>
* Array (
*		'AssetID'	 		=> [The ID of the asset in query],
*        )
* </pre>
*
* @return void
* @access public
*/
function GetMatrixVersion()
{
	var soapBody	= "\
<ns1:GetMatrixVersion>\
</ns1:GetMatrixVersion>";

	return soapBody;

}//end StartWorkflow
