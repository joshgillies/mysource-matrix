/**
 * This file contain all the SOAP Body used to request the Design Lookup Service API.
 * 
 */


/**
* Description: This operation will apply a design to an asset
*
* @param array  $request	The request information
* <pre>
* Array (
*		 'DesignID'	 				=> [The Design ID],
*		 'AssetID'			 		=> [The AssetID of the asset being applied the design],
*		 'DesignType'				=> [Type of Design: Frontend, OverrideFrontend, Login, OverrideLogin, UserDefined, OverrideUserDefined],
*        'UserDefinedDesignName'    => [Name of the new User Defined design],

*        )
* </pre>
*
* @return string
* @access public
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
* Description: This operation will remove a design to an asset
*
* @param array  $request	The request information
* <pre>
* Array (
*		 'DesignID'	 				=> [The Design ID],
*		 'AssetID'			 		=> [The AssetID of the asset being applied the design],
*		 'DesignType'				=> [Type of Design: Frontend, OverrideFrontend, Login, OverrideLogin, UserDefined, OverrideUserDefined],
*        'UserDefinedDesignName'    => [Name of the new User Defined design],

*        )
* </pre>
*
* @return void
* @access public
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
* Description: This operation returns the effective design of an URL
*
* @param array $asset_info the info array to find children of a specific asset
* <pre>
* Array (
*		'URL'					=> [The URL in question],
*		'DesignType'			=> [Type of Design: Frontend, OverrideFrontend, Login, OverrideLogin, UserDefined, OverrideUserDefined],
*		'UserDefinedDesignName'	=> [Name of the new User Defined design],
*        )
* </pre>
*
* @return void
* @access public
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
* Description: This operation will apply a paintlayout to an asset
*
* @param array $request		the request information
* <pre>
* Array (
*		'PaintLayoutID'		=> [The assetid of the paintlayout],
*		'AssetID'			=> [The asset being applied paintlayout to],
*		'PaintLayoutType'	=> [Type of paint layout Frontend or OverrideFrontend or UserDefined],
		'AssetUrl'			=> [The asset url to apply the paint layout to (url specific)],
*		'PaintLayoutCode'   => [Layout code for the user defined layout],
*        )
* </pre>
*
* @return void
* @access public
*/
function ApplyAssetPaintLayout(paint_layout_id, assetid, paint_layout_type, asset_url, paint_layout_code)
{
	var soapBody	= "\
<ns1:ApplyAssetPaintLayout>\
<PaintLayoutID>"+paint_layout_id+"</PaintLayoutID>\
<AssetID>"+assetid+"</AssetID>\
<PaintLayoutType>"+paint_layout_type+"</PaintLayoutType>\
<AssetUrl>"+asset_url+"</AssetUrl>\
<PaintLayoutCode>"+paint_layout_code+"</PaintLayoutCode>\
</ns1:ApplyAssetPaintLayout>";

	return soapBody;

}//end ApplyAssetPaintLayout


/**
* Description: This operation will remove a paintlayout to an asset
*
* @param array $request		the request information
* <pre>
* Array (
*		'PaintLayoutID'		=> [The assetid of the paintlayout],
*		'AssetID'			=> [The asset being remove paintlayout from],
*		'PaintLayoutType'	=> [Type of paint layout Frontend or OverrideFrontend or UserDefined],
		'AssetUrl'			=> [The asset url to apply the paint layout to (url specific)],
*		'PaintLayoutCode'   => [Layout code for the user defined layout],
*        )
* </pre>
*
* @return void
* @access public
*/
function RemoveAssetPaintLayout(paint_layout_id, assetid, paint_layout_type, asset_url, paint_layout_code)
{
	var soapBody	= "\
<ns1:RemoveAssetPaintLayout>\
<PaintLayoutID>"+paint_layout_id+"</PaintLayoutID>\
<AssetID>"+assetid+"</AssetID>\
<PaintLayoutType>"+paint_layout_type+"</PaintLayoutType>\
<AssetUrl>"+asset_url+"</AssetUrl>\
<PaintLayoutCode>"+paint_layout_code+"</PaintLayoutCode>\
</ns1:RemoveAssetPaintLayout>";

	return soapBody;

}//end RemoveAssetPaintLayout


/**
* Description: This operation returns the paint layout applied to the given url
*
* @param array $asset_info the info array to find children of a specific asset
* <pre>
* Array (
*		'URL'					=> [The URL in question],
*		'PaintLayoutType'		=> [Type of paint layout Frontend or OverrideFrontend or UserDefined],
*		'PaintLayoutCode'		=> [Layout code for the user defined layout],
*        )
* </pre>
*
* @return void
* @access public
*/
function GetPaintLayoutFromURL(url, layout_type, layout_code)
{
	var soapBody	= "\
<ns1:GetPaintLayoutFromURL>\
<URL>"+url+"</URL>\
<PaintLayoutType>"+layout_type+"</PaintLayoutType>\
<PaintLayoutCode>"+layout_code+"</PaintLayoutCode>\
</ns1:GetPaintLayoutFromURL>";

	return soapBody;

}//end GetPaintLayoutFromURL