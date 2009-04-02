/**
 * This file contain all the SOAP Body used to request the Link Service API.
 * 
 */


/**
 * This operation will delete a link based on the LinkID
 */
function DeleteAssetLink(linkid)
{
	var soapBody	= "\
<ns1:DeleteAssetLink>\
<LinkID>"+linkid+"</LinkID>\
</ns1:DeleteAssetLink>";
	return soapBody;
}//end DeleteAssetLink


/**
 * This operation will return all child links
 */
function GetAllChildLinks(assetid, link_type)
{
	var soapBody	= "\
<ns1:GetAllChildLinks>\
<AssetID>"+assetid+"</AssetID>\
<LinkType>"+link_type+"</LinkType>\
</ns1:GetAllChildLinks>";
	return soapBody;
}//end GetAllChildLinks


/**
 * This operation will return dependant children of the specified criteria
 */
function GetDependantChildren(assetid, type_code, strict_type_code)
{
	var soapBody	= "\
<ns1:GetDependantChildren>\
<AssetID>"+assetid+"</AssetID>\
<TypeCode>"+type_code+"</TypeCode>\
<StrictTypeCode>"+strict_type_code+"</StrictTypeCode>\
</ns1:GetDependantChildren>";
	return soapBody;
}//end GetDependantChildren


/**
 * This operation will return dependant parents of the specified criteria
 */
function GetDependantParents(assetid, type_code, strict_type_code, include_all_dependants)
{
	var soapBody	= "\
<ns1:GetDependantParents>\
<AssetID>"+assetid+"</AssetID>\
<TypeCode>"+type_code+"</TypeCode>\
<StrictTypeCode>"+strict_type_code+"</StrictTypeCode>\
<IncludeAllDependants>"+include_all_dependants+"</IncludeAllDependants>\
</ns1:GetDependantParents>";
	return soapBody;
}//end GetDependantParents


/**
 * This operation will return dependant parents of the specified criteria
 */
function GetLinks(assetid, link_type, type_code, strict_type_code, side_of_link, link_value, dependant, exclusive, sort_by, permission_level, effective)
{
	var soapBody	= "\
<ns1:GetLinks>\
<AssetID>"+assetid+"</AssetID>\
<LinkType>"+link_type+"</LinkType>\
<TypeCode>"+type_code+"</TypeCode>\
<StrictTypeCode>"+strict_type_code+"</StrictTypeCode>\
<SideOfLink>"+side_of_link+"</SideOfLink>\
<LinkValue>"+link_value+"</LinkValue>\
<Dependant>"+link_value+"</Dependant>\
<Exclusive>"+link_value+"</Exclusive>\
<SortBy>"+link_value+"</SortBy>\
<PermissionLevel>"+link_value+"</PermissionLevel>\
<Effective>"+effective+"</Effective>\
</ns1:GetLinks>";
	return soapBody;
}//end GetLinks


/**
 * This operation will return dependant parents of the specified criteria
 */
function MoveLink(assetid, link_type, to_parent_id, to_parent_position)
{
	var soapBody	= "\
<ns1:MoveLink>\
<LinkID>"+assetid+"</LinkID>\
<LinkType>"+link_type+"</LinkType>\
<ToParentID>"+to_parent_id+"</ToParentID>\
<ToParentPosition>"+to_parent_position+"</ToParentPosition>\
</ns1:MoveLink>";
	return soapBody;
}//end MoveLink


/**
 * This operation will update an existing link
 */
function UpdateLink(linkid, link_type, link_value, sort_order)
{
	var soapBody	= "\
<ns1:UpdateLink>\
<LinkID>"+assetid+"</LinkID>\
<LinkType>"+link_type+"</LinkType>\
<LinkValue>"+link_value+"</LinkValue>\
<SortOrder>"+sort_order+"</SortOrder>\
</ns1:UpdateLink>";
	return soapBody;
}//end UpdateLink


/**
 * This function returns the children of a specific asset base on criterions
 */
function GetChildren(assetid, type_code, strict_type_code, dependant, sort_by, permission_level, effective_access, min_depth, max_depth, direct_shadow_only, link_value_wanted)
{
	var soapBody	= "\
<ns1:GetChildren>\
<AssetID>"+assetid+"</AssetID>\
<TypeCode>"+type_code+"</TypeCode>\
<StrictTypeCode>"+strict_type_code+"</StrictTypeCode>\
<Dependant>"+dependant+"</Dependant>\
<SortBy>"+sort_by+"</SortBy>\
<PermissionLevel>"+permission_level+"</PermissionLevel>\
<EffectiveAccess>"+effective_access+"</EffectiveAccess>\
<MinDepth>"+min_depth+"</MinDepth>\
<MaxDepth>"+max_depth+"</MaxDepth>\
<DirectShadowsOnly>"+direct_shadow_only+"</DirectShadowsOnly>\
<LinkValueWanted>"+link_value_wanted+"</LinkValueWanted>\
</ns1:GetChildren>";
	return soapBody;
}//end GetChildren


/**
 * This function returns all the parent of a specific asset
 */
function GetParents(assetid, type_code, strict_type_code, dependant, sort_by, permission_level, effective_access, min_depth, max_depth, direct_shadow_only, link_value_wanted)
{
	var soapBody	= "\
<ns1:GetParents>\
<AssetID>"+assetid+"</AssetID>\
<TypeCode>"+type_code+"</TypeCode>\
<StrictTypeCode>"+strict_type_code+"</StrictTypeCode>\
<Dependant>"+dependant+"</Dependant>\
<SortBy>"+sort_by+"</SortBy>\
<PermissionLevel>"+permission_level+"</PermissionLevel>\
<EffectiveAccess>"+effective_access+"</EffectiveAccess>\
<MinDepth>"+min_depth+"</MinDepth>\
<MaxDepth>"+max_depth+"</MaxDepth>\
<DirectShadowsOnly>"+direct_shadow_only+"</DirectShadowsOnly>\
<LinkValueWanted>"+link_value_wanted+"</LinkValueWanted>\
</ns1:GetParents>";
	return soapBody;
}//end GetParents


/**
 * This function returns all the parent of a specific asset
 */
function GetLinkByAsset(assetid, other_assetid, link_type, link_value, side_of_link, is_dependant, is_exclusive)
{
	var soapBody	= "\
<ns1:GetLinkByAsset>\
<AssetID>"+assetid+"</AssetID>\
<OtherAssetID>"+other_assetid+"</OtherAssetID>\
<LinkType>"+link_type+"</LinkType>\
<LinkValue>"+link_value+"</LinkValue>\
<SideOfLink>"+side_of_link+"</SideOfLink>\
<IsDependant>"+is_dependant+"</IsDependant>\
<IsExclusive>"+is_exclusive+"</IsExclusive>\
</ns1:GetLinkByAsset>";
	return soapBody;
}//end GetLinkByAsset

