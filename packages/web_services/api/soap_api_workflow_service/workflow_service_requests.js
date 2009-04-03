/**
 * This file contain all the SOAP Body used to request the Workflow Service API.
 * 
 */

/**
 * This operation will start the workflow for the asset in query
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
 * This operation will cancel the current running workflow of an asset
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
 * This operation will complete the current running workflow of an asset
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
 * This operation will approve an asset in workflow
 */
function ApproveAssetInWorkflow(assetid)
{
	var soapBody	= "\
<ns1:ApproveAssetInWorkflow>\
<AssetID>"+assetid+"</AssetID>\
</ns1:ApproveAssetInWorkflow>";
	return soapBody;
}//end ApproveAssetInWorkflow


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