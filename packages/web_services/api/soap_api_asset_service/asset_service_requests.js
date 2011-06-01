/**
 * This file contain all the SOAP Body used to request the Asset Service API.
 * 
 */

/**
 * This operation will create an asset of a specific type under a specific location
 */
function CreateAsset(parentid, type_code, name, link_type, link_value, sort_order, is_dependant, is_exclusive)
{
	var soapBody	= "\
<ns1:CreateAsset>\
<ParentID>"+parentid+"</ParentID>\
<TypeCode>"+type_code+"</TypeCode>\
<Name>"+name+"</Name>\
<LinkType>"+link_type+"</LinkType>\
<LinkValue>"+link_value+"</LinkValue>\
<SortOrder>"+sort_order+"</SortOrder>\
<IsDependant>"+is_dependant+"</IsDependant>\
<IsExclusive>"+is_exclusive+"</IsExclusive>\
</ns1:CreateAsset>";
	return soapBody;
}//end CreateAsset


/**
 * This operation will return an asset object based on assetid
 */
function GetAsset(assetid)
{
	var soapBody	= "\
<ns1:GetAsset>\
<AssetID>"+assetid+"</AssetID>\
</ns1:GetAsset>";
	return soapBody;	
}//end GetAsset


/**
 * This operation will return all URLs associated with an asset
 */
function GetURLs(assetid)
{
	var soapBody	= "\
<ns1:GetURLs>\
<AssetID>"+assetid+"</AssetID>\
</ns1:GetURLs>";
	return soapBody;	
}//end GetURLs


/**
 * This operation will return an asset object based on a URL
 */
function GetAssetFromURL(url)
{
	var soapBody	= "\
<ns1:GetAssetFromURL>\
<URL>"+url+"</URL>\
</ns1:GetAssetFromURL>";
	return soapBody;
}//end GetAssetFromURL


/**
 * This operation will return all available statuses of an asset based on assetid
 */
function GetAssetAvailableStatuses(assetid)
{
	var soapBody	= "\
<ns1:GetAssetAvailableStatuses>\
<AssetID>"+assetid+"</AssetID>\
</ns1:GetAssetAvailableStatuses>";
	return soapBody;	
}//end GetAssetAvailableStatuses()

/**
 * This operation will return all available statuses of an asset based on assetid
 */
function SetAttributeValue(assetid, attribute_name, attribute_value)
{
	var soapBody	= "\
<ns1:SetAttributeValue>\
<AssetID>"+assetid+"</AssetID>\
<AttributeName>"+attribute_name+"</AttributeName>\
<AttributeValue>"+attribute_value+"</AttributeValue>\
</ns1:SetAttributeValue>";
	return soapBody;
}//end SetAttributeValue()




/**
 * This operation will create link between two assets
 */
function CreateLink(majorid, minorid, link_type, link_value, sort_order, is_dependant, is_exclusive)
{
	var soapBody	= "\
<ns1:CreateLink>\	
<MajorID>"+majorid+"</MajorID>\
<MinorID>"+minorid+"</MinorID>\
<LinkType>"+link_type+"</LinkType>\
<LinkValue>"+link_value+"</LinkValue>\
<SortOrder>"+sort_order+"</SortOrder>\
<IsDependant>"+is_dependant+"</IsDependant>\
<IsExclusive>"+is_exclusive+"</IsExclusive>\
</ns1:CreateLink>";
	return soapBody;
}


/**
 * This operation will send an asset to the trash
 */
function TrashAsset(assetid)
{
	var soapBody	= "\
<ns1:TrashAsset>\
<AssetID>"+assetid+"</AssetID>\
</ns1:TrashAsset>";
	return soapBody;
}//end TrashAsset()


/**
 * This operation will clone an asset to a specified location
 */
function CloneAsset(assetid, parentid, new_parentid, num_clone)
{
	var soapBody	= "\
<ns1:CloneAsset>\
<AssetID>"+assetid+"</AssetID>\
<CurrentParentID>"+parentid+"</CurrentParentID>\
<NewParentID>"+new_parentid+"</NewParentID>\
<NumberOfClone>"+num_clone+"</NumberOfClone>\
</ns1:CloneAsset>";
	return soapBody;
	
}//end CloneAsset

/**
 * This operation will return all attributes belong to an asset type
 */
function GetAssetTypeAttribute(type_code, details)
{
	var soapBody	= "\
<ns1:GetAssetTypeAttribute>\
<TypeCode>"+type_code+"</TypeCode>\
<Details>"+details+"</Details>\
</ns1:GetAssetTypeAttribute>";
	return soapBody;
}//end GetAssetTypeAttribute()


/**
 * This operation will return all webpaths belong to an asset type
 */
function GetAssetWebPaths(assetid)
{
	var soapBody	= "\
<ns1:GetAssetWebPaths>\
<AssetID>"+assetid+"</AssetID>\
</ns1:GetAssetWebPaths>";
	return soapBody;
}//end GetAssetWebPaths()


/**
 * This operation will return all available keywords of an asset
 */
function GetAvailableKeywords(assetid)
{
	var soapBody	= "\
<ns1:GetAvailableKeywords>\
<AssetID>"+assetid+"</AssetID>\
</ns1:GetAvailableKeywords>";
	return soapBody;
}//end GetAvailableKeywords()

/**
 * This operation will return all attributes belong to an asset type classified by attribute name
 */
function GetAttributeValuesByName(assetids, attribute_name, type_code)
{
	var soapBody	= "\
<ns1:GetAttributeValuesByName>\
<AssetIDs>"+assetids+"</AssetIDs>\
<AttributeName>"+attribute_name+"</AttributeName>\
<TypeCode>"+type_code+"</TypeCode>\
</ns1:GetAttributeValuesByName>";
	return soapBody;
}//end GetAttributeValuesByName()


/**
 * This operation will set tag for an asset
 */
function SetTag(assetid, thesaurus_id, tag_name, weight, cascade_tag_change)
{
	var soapBody	= "\
<ns1:SetTag>\
<AssetID>"+assetid+"</AssetID>\
<ThesaurusID>"+thesaurus_id+"</ThesaurusID>\
<TagName>"+tag_name+"</TagName>\
<Weight>"+weight+"</Weight>\
<CascadeTagChange>"+cascade_tag_change+"</CascadeTagChange>\
</ns1:SetTag>";
	return soapBody;
}//end SetTag()


/**
 * Description: This operation will return all statuses in Matrix
 */
function GetAllStatuses()
{
	var soapBody	= "\
<ns1:GetAllStatuses>\
</ns1:GetAllStatuses>";
	return soapBody;	
}//end GetAllStatuses()
