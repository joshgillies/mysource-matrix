/**
 * This file contain all the SOAP Body used to request the Metadata Service API.
 * 
 */


/**
* Description: This operation will apply or revoke a metadata schema from an asset
*
* @param array  $request	The request information
* <pre>
* Array (
*		 'AssetID'	 	=> [The asset to apply the schema on],
*		 'SchemaID'		=> [The metadata scheme being applied],
*		 'Grant'		=> [Apply, Deny, Revoke],
*        )
* </pre>
*
* @return void
* @access public
*/
function SetMetadataSchema(assetid, schemaid, grant)
{
	var soapBody	= "\
<ns1:SetMetadataSchema>\
<AssetID>"+assetid+"</AssetID>\
<SchemaID>"+schemaid+"</SchemaID>\
<Grant>"+grant+"</Grant>\
</ns1:SetMetadataSchema>";
	return soapBody;
}//end SetMetadataSchema


/**
* Description: This operation will regenerate metadata for the specified asset
*
* @param array  $request	The request information
* <pre>
* Array (
*		 'AssetID'		=> [The asset to regenerate metadata for],
*        )
* </pre>
*
* @return void
* @access public
*/
function RegenerateMetadataSchema(schemaid)
{
	var soapBody	= "\
<ns1:RegenerateMetadataSchema>\
<SchemaID>"+schemaid+"</SchemaID>\
</ns1:RegenerateMetadataSchema>";
	return soapBody;
}//end RegenerateMetadataSchema


/**
* Description: This operation will regenerate metadata for the specified asset
*
* @param array  $request	The request information
* <pre>
* Array (
*		 'AssetID'		=> [The asset to regenerate metadata for],
*        )
* </pre>
*
* @return void
* @access public
*/
function RegenerateMetadataAsset(assetid)
{
	var soapBody	= "\
<ns1:RegenerateMetadataAsset>\
<AssetID>"+assetid+"</AssetID>\
</ns1:RegenerateMetadataAsset>";
	return soapBody;
}//end RegenerateMetadataAsset


/**
* Description: This operation will set the value for a metadata field of an asset
*
* @param array $request	The request information
* <pre>
* Array (
*		 'AssetID'		=> [The asset to regenerate metadata for],
*		 'FieldID'		=> [The metadata field id],
*		 'NewValue'		=> [The new value for the field],
*        )
* </pre>
*
* @return void
* @access public
*/
function SetAssetMetadata(assetid, field_id, new_value)
{
	var soapBody	= "\
<ns1:SetAssetMetadata>\
<AssetID>"+assetid+"</AssetID>\
<FieldID>"+field_id+"</FieldID>\
<NewValue>"+new_value+"</NewValue>\
</ns1:SetAssetMetadata>";
	return soapBody;
}//end SetAssetMetadata


/**
* Description: This operation will set the value for a metadata fieldis of an asset
*
* @param array $request	The request information
* <pre>
* Array (
*		 'AssetID'		=> [The asset to regenerate metadata for],
*		 'MetadataInfo' => [metadata field id/value pairs],
*        )
* </pre>
*
* @return void
* @access public
*/
function SetMultipleMetadataFields(assetid, metadata_info)
{
	var soapBody	= "\
<ns1:SetMultipleMetadataFields>\
<AssetID>"+assetid+"</AssetID>";

	var meta_str = "";
	for (var i in metadata_info) {
		meta_str += "\
<MetadataInfo>\
<FieldID>"+metadata_info[i][0]+"</FieldID>\
<FieldValue>"+metadata_info[i][1]+"</FieldValue>\
</MetadataInfo>";
	}

	soapBody += meta_str +"\
</ns1:SetMultipleMetadataFields>";

	return soapBody;
}//end SetAssetMetadata


/**
* Description: This operation will set the default value for a metadata field
*
* @param array $request	The request information
* <pre>
* Array (
*		 'FieldID'			=> [The metadata field id],
*		 'NewDefaultValue'	=> [The new default value for the field],
*        )
* </pre>
*
* @return void
* @access public
*/
function SetMetadataFieldDefaultValue(field_id, new_default_value)
{
	var soapBody	= "\
<ns1:SetMetadataFieldDefaultValue>\
<FieldID>"+field_id+"</FieldID>\
<NewDefaultValue>"+new_default_value+"</NewDefaultValue>\
</ns1:SetMetadataFieldDefaultValue>";
	return soapBody;
}//end SetMetadataFieldDefaultValue


/**
* Description: This operation will return the metadata value for a metadata field of an asset
*
* @param array $request	The request information
* <pre>
* Array (
*		 'AssetID'			=> [The Asset in query],
*		 'FieldID'			=> [The metadata field ID],
*        )
* </pre>
*
* @return void
* @access public
*/
function GetMetadataValueByIDs(assetid, fieldid)
{
	var soapBody	= "\
<ns1:GetMetadataValueByIDs>\
<AssetID>"+assetid+"</AssetID>\
<FieldID>"+fieldid+"</FieldID>\
</ns1:GetMetadataValueByIDs>";

	return soapBody;

}//end GetMetadataValueByIDs


/**
* Description: This operation will return the metadata schemas applied on the asset
*
* @param array $request	The request information
* <pre>
* Array (
*		 'AssetID'			=> [The Asset in query],
*        )
* </pre>
*
* @return void
* @access public
*/
function GetSchemasOnAsset(assetid)
{
	var soapBody	= "\
<ns1:GetSchemasOnAsset>\
<AssetID>"+assetid+"</AssetID>\
</ns1:GetSchemasOnAsset>";
	return soapBody;
}//end GetSchemasOnAsset


/**
* Description: This operation will return the metadata fields ids which belong to a metadata schema
*
* @param array $request	The request information
* <pre>
* Array (
*		 'SchemaID'			=> [The metadata schema in query],
*        )
* </pre>
*
* @return void
* @access public
*/
function GetMetadataFieldsOfSchema(schemaid)
{
	var soapBody	= "\
<ns1:GetMetadataFieldsOfSchema>\
<SchemaID>"+schemaid+"</SchemaID>\
</ns1:GetMetadataFieldsOfSchema>";
	return soapBody;
}//end GetMetadataFieldsOfSchema


/**
* Description: This operation will return the metadata fields details which belong to a metadata schema
*
* @param array $request	The request information
* <pre>
* Array (
*		 'SchemaID'			=> [The metadata schema in query],
*        )
* </pre>
*
* @return void
* @access public
*/
function GetMetadataFieldsDetailOfSchema(schemaid)
{
	var soapBody	= "\
<ns1:GetMetadataFieldsDetailOfSchema>\
<SchemaID>"+schemaid+"</SchemaID>\
</ns1:GetMetadataFieldsDetailOfSchema>";
	return soapBody;
}//end GetMetadataFieldsDetailOfSchema


/**
* Description: This operation will return the values of the field names which belong to an asset
*
* @param array $request	The request information
* <pre>
* Array (
*		'AssetID'			=> [The asset which has the metadata field],
*		'FieldNames'		=> [The metadata field names],
*        )
* </pre>
*
* @return void
* @access public
*/
function GetMetadataFieldValues(assetid, field_names)
{
	var soapBody	= "\
<ns1:GetMetadataFieldValues>\
<AssetID>"+assetid+"</AssetID>\
<FieldNames>"+field_names+"</FieldNames>\
</ns1:GetMetadataFieldValues>";
	
	return soapBody;
	
}//end GetMetadataFieldValues
