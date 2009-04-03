/**
 * This file contain all the SOAP Body used to request the Design Lookup Service API.
 * 
 */


/**
 * This operation will apply a design to an asset
 */
function ApplyDesign(designid, assetid, design_type, user_defined_design_name)
{
	var soapBody	= "\
<ns1:ApplyDesign>\
<DesignID>"+designid+"</DesignID>\
<AssetID>"+assetid+"</AssetID>\
<DesignType>"+design_type+"</DesignType>\
<UserDefinedDesignName>"+user_defined_design_name+"</UserDefinedDesignName>\
</ns1:ApplyDesign>";
	return soapBody;
}//end ApplyDesign


/**
 * This operation will remove a design to an asset
 */
function RemoveDesign(designid, assetid, design_type, user_defined_design_name)
{
	var soapBody	= "\
<ns1:RemoveDesign>\
<DesignID>"+designid+"</DesignID>\
<AssetID>"+assetid+"</AssetID>\
<DesignType>"+design_type+"</DesignType>\
<UserDefinedDesignName>"+user_defined_design_name+"</UserDefinedDesignName>\
</ns1:RemoveDesign>";
	return soapBody;	
}//end RemoveDesign


/**
 * This operation returns the effective design of an URL
 */
function GetDesignFromURL(url, design_type, user_defined_design_name)
{
	var soapBody	= "\
<ns1:GetDesignFromURL>\
<URL>"+url+"</URL>\
<DesignType>"+design_type+"</DesignType>\
<UserDefinedDesignName>"+user_defined_design_name+"</UserDefinedDesignName>\
</ns1:GetDesignFromURL>";
	return soapBody;	
}//end GetDesignFromURL


/**
 * This operation will apply a paintlayout to an asset
 */
function ApplyAssetPaintLayout(paint_layout_id, assetid, paint_layout_type)
{
	var soapBody	= "\
<ns1:ApplyAssetPaintLayout>\
<PaintLayoutID>"+paint_layout_id+"</PaintLayoutID>\
<AssetID>"+assetid+"</AssetID>\
<PaintLayoutType>"+paint_layout_type+"</PaintLayoutType>\
</ns1:ApplyAssetPaintLayout>";
	return soapBody;	
}//end ApplyAssetPaintLayout


/**
 * This operation will remove a paintlayout to an asset
 */
function RemoveAssetPaintLayout(paint_layout_id, assetid, paint_layout_type)
{
	var soapBody	= "\
<ns1:RemoveAssetPaintLayout>\
<PaintLayoutID>"+paint_layout_id+"</PaintLayoutID>\
<AssetID>"+assetid+"</AssetID>\
<PaintLayoutType>"+paint_layout_type+"</PaintLayoutType>\
</ns1:RemoveAssetPaintLayout>";
	return soapBody;	
}//end RemoveAssetPaintLayout
