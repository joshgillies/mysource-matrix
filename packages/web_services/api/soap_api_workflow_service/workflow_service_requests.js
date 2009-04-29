/**
 * This file contain all the SOAP Body used to request the Workflow Service API.
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
function StartWorkflow(assetid)
{
	var soapBody	= "\
<ns1:StartWorkflow>\
<AssetID>"+assetid+"</AssetID>\
</ns1:StartWorkflow>";

	return soapBody;

}//end StartWorkflow


/**
* Description: This operation will cancel the current running workflow of an asset.
* If the asset is in LIVE APPROVAL status, it will turn into SAFE EDIT status.
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
function CancelWorkflow(assetid)
{
	var soapBody	= "\
<ns1:CancelWorkflow>\
<AssetID>"+assetid+"</AssetID>\
</ns1:CancelWorkflow>";

	return soapBody;

}//end CancelWorkflow


/**
* Description: This operation will complete the current running workflow of an asset
*
* @param array $asset_info the info array to find children of a specific asset
* <pre>
* Array (
*		'AssetID'	 		=> [The ID of the asset in query],
*        )
* </pre>
*
* @return void
* @access public
*/
function CompleteWorkflow(assetid)
{
	var soapBody	= "\
<ns1:CompleteWorkflow>\
<AssetID>"+assetid+"</AssetID>\
</ns1:CompleteWorkflow>";

	return soapBody;

}//end CompleteWorkflow


/**
* Description: This operation will approve an asset in workflow
*
* @param array $asset_info the array contain the asset ID of the asset to be approved
* <pre>
* Array (
*		'AssetID'	 		=> [The ID of the asset in query],
*        )
* </pre>
*
* @return void
* @access public
*/
function ApproveAssetInWorkflow(assetid, workflow_message)
{
	var soapBody	= "\
<ns1:ApproveAssetInWorkflow>\
<AssetID>"+assetid+"</AssetID>\
<WorkflowMessage>"+workflow_message+"</WorkflowMessage>\
</ns1:ApproveAssetInWorkflow>";

	return soapBody;

}//end ApproveAssetInWorkflow


/**
* Description: This operation will bring a asset in live status to safe edit status
*
* @param array $asset_info the array contain the asset ID of the asset to be put in safe edit
* <pre>
* Array (
*		'AssetID'	 		=> [The ID of the asset in query],
*        )
* </pre>
*
* @return void
* @access public
*/
function SafeEditAsset(assetid)
{
	var soapBody	= "\
<ns1:SafeEditAsset>\
<AssetID>"+assetid+"</AssetID>\
</ns1:SafeEditAsset>";

	return soapBody;

}//end SafeEditAsset


/**
 * This operation will apply or revoke a workflow schema on an asset
 */
function SetWorkflowSchema(assetid, schemaid, grant)
{
	var soapBody	= "\
<ns1:SetWorkflowSchema>\
<AssetID>"+assetid+"</AssetID>\
<SchemaID>"+schemaid+"</SchemaID>\
<Grant>"+grant+"</Grant>\
</ns1:SetWorkflowSchema>";

	return soapBody;

}//end SetWorkflowSchema
