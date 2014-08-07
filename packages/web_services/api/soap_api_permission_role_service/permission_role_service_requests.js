/**
 * This file contain all the SOAP Body used to request the Permission and Role Service API.
 * 
 */


/**
* Description: This operation will check whether the requesting user have a certain access level to a particular asset
*
* @param array  $request	The request information
* <pre>
* Array (
*		 'AssetID'	 		=> [type code of new asset],
*		 'PermissionLevel'	=> [name for new asset],
*        )
* </pre>
*
* @return void
* @access public
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
* Description: This operation will set the permission for a user on an asset
*
* @param array $asset_info the info array to find children of a specific asset
* <pre>
* Array (
*		'AssetID'			=> [The asset in query],
*		'UserID'			=> [The user which the permission is applying for],
*		'PermissionLevel'	=> [Read Write Admin],
*		'Grant'				=> [Apply, Deny, Revoke],
*		'Cascade'			=> [Whether to cascade permission down to children],
*        )
* </pre>
*
* @return void
* @access public
*/
function SetPermission(assetid, userid, permission_level, grant, cascade)
{
	var soapBody	= "\
<ns1:SetPermission>\
<AssetID>"+assetid+"</AssetID>\
<UserID>"+userid+"</UserID>\
<PermissionLevel>"+permission_level+"</PermissionLevel>\
<Grant>"+grant+"</Grant>\
<Cascade>"+cascade+"</Cascade>\
</ns1:SetPermission>";

	return soapBody;

}//end SetPermission


/**
* Description: This operation will return the permission set for on an asset
*
* @param array $asset_info the info array to find children of a specific asset
* <pre>
* Array (
*		'AssetID'			=> [The asset in query],
*		'PermissionLevel'	=> [Read Write Admin],
*		'Granted'			=> [Apply, Deny, Revoke],
*		'AndGreater'		=> [Check for effective permission (write = read + write)],
*		'ExpandGroups'		=> [Return userids inside a group instead just groupid],
*		'AllInfo'			=> [Apply, Deny, Revoke],
*        )
* </pre>
*
* @return void
* @access public
*/
function GetPermission(assetid, permission_level, granted, effective, expand_groups, all_info)
{
	var soapBody	= "\
<ns1:GetPermission>\
<AssetID>"+assetid+"</AssetID>\
<PermissionLevel>"+permission_level+"</PermissionLevel>\
<Granted>"+granted+"</Granted>\
<AndGreater>"+effective+"</AndGreater>\
<ExpandGroups>"+expand_groups+"</ExpandGroups>\
<AllInfo>"+all_info+"</AllInfo>\
</ns1:GetPermission>";

	return soapBody;

}//end GetPermission


/**
* Description: This operation will return an array of roles and the users/groups which can perform it
*
* @param array $asset_info the info array to find children of a specific asset
* <pre>
* Array (
*		'AssetID'				=> [The assetid of the asset the role is applied to],
*		'RoleID'				=> [The assetid of the the role that is applied],
*		'UserID'				=> [The assetid of the user performing the role],
*		'IncludeAssetID'		=> [Whether to include the assetid in the returned array],
*		'IncludeGlobals'		=> [Whether to query the role view which includes expanded global roles as individual users],
*		'IncludeDependants'		=> [If false it will filter out the dependant assets],
*		'ExpandGroups'			=> [when TRUE, any groups defined within a role will be replaced with the userids in that group]
*        )
* </pre>
*
* @return void
* @access public
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
* Description: This operation will set role for an user to perform on an asset.
*
* @param array $asset_info the info array to find children of a specific asset
* <pre>
* Array (
*		'AssetID'				=> [The assetid of the asset the role is applied to],
*		'RoleID'				=> [The assetid of the the role that is applied],
*		'UserID'				=> [The assetid of the user performing the role],
*		'Action'				=> [Add | Delete Role],
*		'GlobalRole'			=> [Global or non-global role],
*		'Cascade'				=> [Whether to cascade the role down to children],
*        )
* </pre>
*
* @return void
* @access public
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


