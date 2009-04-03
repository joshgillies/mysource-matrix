/**
 * This file contain all the SOAP Body used to request the Workflow Service API.
 * 
 */


/**
 * This operation will check whether the requesting user have a certain access level to a particular asset
 */
function HasAccess(assetid, permission_level)
{
	var soapBody	= "\
<ns1:HasAccess>\
<AssetID>"+assetid+"</AssetID>\
<PermissionLevel>"+permission_level+"</PermissionLevel>\
</ns1:HasAccess>";
	return soapBody;
}//end HasAccess


/**
 * This operation will set the permission for a user on an asset
 */
function SetPermission(assetid, userid, permission_level, cascade)
{
	var soapBody	= "\
<ns1:SetPermission>\
<AssetID>"+assetid+"</AssetID>\
<UserID>"+userid+"</UserID>\
<PermissionLevel>"+permission_level+"</PermissionLevel>\
<Cascade>"+cascade+"</Cascade>\
</ns1:SetPermission>";
	return soapBody;
}//end SetPermission


/**
 * This operation will return the permission set for on an asset
 */
function GetPermission(assetid, permission_level, granted, effective, expand_groups, all_info)
{
	var soapBody	= "\
<ns1:GetPermission>\
<AssetID>"+assetid+"</AssetID>\
<PermissionLevel>"+permission_level+"</PermissionLevel>\
<Granted>"+granted+"</Granted>\
<Effective>"+effective+"</Effective>\
<ExpandGroups>"+expand_groups+"</ExpandGroups>\
<AllInfo>"+all_info+"</AllInfo>\
</ns1:GetPermission>";
	return soapBody;
}//end GetPermission


/**
 * This operation will return an array of roles and the users/groups which can perform it
 */
function GetRole(assetid, roleid, userid, include_assetid, include_globals, include_dependants, expand_groups)
{
	var soapBody	= "\
<ns1:GetRole>\
<AssetID>"+assetid+"</AssetID>\
<RoleID>"+roleid+"</RoleID>\
<UserID>"+userid+"</UserID>\
<IncludeAssetID>"+include_assetid+"</IncludeAssetID>\
<IncludeGlobals>"+include_globals+"</IncludeGlobals>\
<IncludeDependants>"+include_dependants+"</IncludeDependants>\
<ExpandGroups>"+expand_groups+"</ExpandGroups>\
</ns1:GetRole>";
	return soapBody;
}//end GetRole


/**
 * This operation will set role for an user to perform on an asset.
 */
function SetRole(assetid, roleid, userid, action, global_role, cascade)
{
	var soapBody	= "\
<ns1:SetRole>\
<AssetID>"+assetid+"</AssetID>\
<RoleID>"+roleid+"</RoleID>\
<UserID>"+userid+"</UserID>\
<Action>"+action+"</Action>\
<GlobalRole>"+global_role+"</GlobalRole>\
<Cascade>"+cascade+"</Cascade>\
</ns1:SetRole>";
	return soapBody;
}//end SetRole


