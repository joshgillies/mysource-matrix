/**
 * This file contain all the SOAP Body used to request the Metadata Service API.
 * 
 */


/**
 * This operation will apply or revoke a metadata schema from an asset
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
 * This operation will regenerate metadata for all asset which has a metadata schema applied on
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
 * This operation will regenerate metadata for the specified asset
 */
function RegenerateMetadataAsset(assetid)
{
	var soapBody	= "\
<ns1:RegenerateMetadataSchema>\
<AssetID>"+assetid+"</AssetID>\
</ns1:RegenerateMetadataSchema>";
	return soapBody;
}//end RegenerateMetadataAsset


/**
 * This operation will set the value for a metadata field of an asset
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
 * This operation will set the default value for a metadata field
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
 * This operation will return the metadata value for a metadata field of an asset
 */
function GetMetadataValueByAssetID(assetid, fieldid)
{
	var soapBody	= "\
<ns1:GetMetadataValueByAssetID>\
<AssetID>"+assetid+"</AssetID>\
<FieldID>"+fieldid+"</FieldID>\
</ns1:GetMetadataValueByAssetID>";
	return soapBody;
}//end GetMetadataValueByAssetID


/**
 * This operation will return the metadata schemas applied on the asset
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
 * This operation will return the metadata fields which belong to a metadata schema
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
 * This operation will return the metadata fields which belong to a metadata schema
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
 * This operation will return the values of the field names which belong to an asset
 */
function GetMetadataFieldsOfSchema(assetid)
{
	var soapBody	= "\
<ns1:GetMetadataFieldsOfSchema>\
<AssetID>"+assetid+"</AssetID>\
<FieldNames>"+field_names+"</FieldNames>\
</ns1:GetMetadataFieldsOfSchema>";
	return soapBody;
}//end GetMetadataFieldsOfSchema
