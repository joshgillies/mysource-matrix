/**
 * This file contain all the SOAP Body used to request the Link Service API.
 * 
 */


/**
* Description: This operation will delete a link based on the LinkID
*
* @param array  $request	The request information
* <pre>
* Array (
*		 'LinkID'	 => [The link ID to be deleted],
*        )
* </pre>
*
* @return void
* @access public
*/
function DeleteAssetLink(linkid)
{
	var soapBody	= "\
<ns1:DeleteAssetLink>\
<LinkID>"+linkid+"</LinkID>\
</ns1:DeleteAssetLink>";

	return soapBody;

}//end DeleteLink


/**
* Description: This operation will return all child links
*
* @param array  $request	The request information
* <pre>
* Array (
*		'AssetID'	=> [The Asset to find links under],
*		'LinkType'	=> [The type of link]
*        )
* </pre>
*
* @param string  $assetid  the id of the asset being deleted
*
* @return void
* @access public
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
* Description: This operation will return dependant children of the specified criteria
*
* @param array  $request	The request information
* <pre>
* Array (
*		'AssetID'			=> [The Asset to find dependant children for],
*		'TypeCode'			=> [The desired type code of dependants],
*		'StrictTypeCode'	=> [Strict Type Code, TRUE | FALSE],
*        )
* </pre>
*
* @return void
* @access public
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
* Description: This operation will return dependant parents of the specified criteria
*
* @param array  $request	The request information
* <pre>
* Array (
*		'AssetID'				=> [The Asset to find dependant children for],
*		'TypeCode'				=> [The desired type code of dependants],
*		'StrictTypeCode'		=> [Strict Type Code, TRUE | FALSE],
*		'IncludeAllDependants'	=> [Whether to include all dependants or only top level dependant parents(FALSE)]
*        )
* </pre>
*
* @return void
* @access public
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
* Description: This operation will return dependant parents of the specified criteria
*
* @param array  $request	The request information
* <pre>
* Array (
*		'AssetID'				=> [The Asset to find links for],
*		'LinkType'				=> [Desired link type]
*		'TypeCode'				=> [The desired type code of linked assets],
*		'StrictTypeCode'		=> [Strict Type Code, TRUE | FALSE],
*		'SideOfLink'			=> [Major or Minor link],
*		'LinkValue'				=> [Value of the link],
*		'Dependant'				=> [If not null, either return non-depandant or dependant, if is null return both],
*		'Exclusive'				=> [If not null, either return non-exclusive or exclusive, if is null return both],
*		'SortBy'				=> [Sort By a field in asset table],
*		'PermissionLevel'		=> [Read, Write, Admin],
*		'Effective'				=> [If effective permissions should be considered or not],
*        )
* </pre>
*
* @return void
* @access public
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
* Description: This operation will move a link under a new parent asset
*
* @param array  $request	The request information
* <pre>
* Array (
*		'LinkID'				=> [The ID of the link],
*		'LinkType'				=> [Desired link type]
*		'ToParentID'			=> [New parent of the minor asset of the link],
*		'ToParentPosition'		=> [The position under the new parent],
*        )
* </pre>
*
* @return void
* @access public
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
* Description: This operation will update an existing link
*
* @param array  $request	The request information
* <pre>
* Array (
*		'LinkID'				=> [The ID of the link],
*		'LinkType'				=> [Desired link type]
*		'LinkValue'				=> [The Value of the link],
*		'SortOrder'				=> [The position in the links list that this link should take, if less than zero places at end of list],
*        )
* </pre>
*
* @return void
* @access public
*/
function UpdateLink(linkid, link_type, link_value, sort_order)
{
	var soapBody	= "\
<ns1:UpdateLink>\
<LinkID>"+linkid+"</LinkID>\
<LinkType>"+link_type+"</LinkType>\
<LinkValue>"+link_value+"</LinkValue>\
<SortOrder>"+sort_order+"</SortOrder>\
</ns1:UpdateLink>";

	return soapBody;

}//end UpdateLink


/**
* Description: This function returns the children of a specific asset base on criterions
*
* @param array $asset_info the info array to find children of a specific asset
* <pre>
* Array (
*		'AssetID'			=> [assetid of node],
*		'TypeCode'			=> [type code of the chilren asset],
*		'StrictTypeCode'	=> [strict type code or not],
*		'Dependant'			=> [dependant or not],
*		'SortBy'			=> [asset attribute],
*		'PermissionLevel'	=> [Read, Write, Admin],
*		'EffectiveAccess'	=> [effective permission],
*		'MinDepth'			=> [minimum child level],
*		'MaxDepth'			=> [maximum child level],
*		'DirectShadowsOnly'	=> [whether to get only shadow asset],
*		'LinkValueWanted'	=> [link value],
*        )
* </pre>
*
* @return void
* @access public
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
* Description: This function returns all the parent of a specific asset
*
* @param array $asset_info the info array to find children of a specific asset
* <pre>
* Array (
*		'AssetID'			=> [assetid of node],
*		'TypeCode'			=> [type code of the chilren asset],
*		'StrictTypeCode'	=> [strict type code or not],
*		'Dependant'			=> [dependant or not],
*		'SortBy'			=> [asset attribute],
*		'PermissionLevel'	=> [Read, Write, Admin],
*		'EffectiveAccess'	=> [effective permission],
*		'MinDepth'			=> [minimum child level],
*		'MaxDepth'			=> [maximum child level],
*		'DirectShadowsOnly'	=> [whether to get only shadow asset],
*		'LinkValueWanted'	=> [link value],
*        )
* </pre>
*
* @return void
* @access public
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
* Description: This function returns all the links between two asset
*
* @param array $asset_info the info array to find children of a specific asset
* <pre>
* Array (
*		'AssetID'			=> [assetid of node],
*		'OtherAssetID'		=> [type code of the asset on the other side of the link],
*		'LinkType'			=> [Desired link type],
*		'LinkValue'			=> [The Value of the link],
*		'SideOfLink'		=> [Which side of the link the first assetid is on, Major or Minor],
*		'IsDependant'		=> [The dependant status for all the links must be this (if not null)],
*		'IsExclusive'		=> [The exclusive status for all the links must be this (if not null)],
*        )
* </pre>
*
* @return void
* @access public
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


/**
* Description: This operation will create link between two assets
*
* @param string  $request  The request information
*
* <pre>
* Array (
*		'MajorID'		=> [The asset id of the major asset],
*		'MinorID'		=> [The asset id of the major asset],
* 		'LinkType'		=> [The link type being created],
* 		'LinkValue'		=> [Value of the new link],
* 		'SortOrder'		=> [Minor Asset position under the Major],
* 		'IsDependant'	=> [Whether the minor asset is dependent on the major]
*		'IsExclusive'	=> [Whether the major asset is the minor's only parent]
*        )
* </pre>
*
* @return void
* @access public
*/
function CreateAssetLink(majorid, minorid, link_type, link_value, sort_order, is_dependant, is_exclusive)
{
	var soapBody	= "\
<ns1:CreateAssetLink>\
<MajorID>"+majorid+"</MajorID>\
<MinorID>"+minorid+"</MinorID>\
<LinkType>"+link_type+"</LinkType>\
<LinkValue>"+link_value+"</LinkValue>\
<SortOrder>"+sort_order+"</SortOrder>\
<IsDependant>"+is_dependant+"</IsDependant>\
<IsExclusive>"+is_exclusive+"</IsExclusive>\
</ns1:CreateAssetLink>";
	
	return soapBody;
}//end CreateAssetLink


/**
* Description: This function returns the link lineages by root node
*
* @param string  $request  The request information
*
* <pre>
* Array (
*		'AssetID'		=> [Assetid of an asset to find lineages of],
*		'RootNode'		=> [Assetid of root node],
*        )
* </pre>
*
* @return void
* @access public
*/
function GetLinkLineages(assetid, root_node)
{
	var soapBody	= "\
<ns1:GetLinkLineages>\
<AssetID>"+assetid+"</AssetID>\
<RootNode>"+root_node+"</RootNode>\
</ns1:GetLinkLineages>";
	
	return soapBody;
}//end GetLinkLineages


/**
* Description: This function returns the parents under the tree
*
* @param string  $request  The request information
*
* <pre>
* Array (
*		'AssetID'		=> [Assetid of an asset to find lineages of],
*		'RootID'		=> [Assetid of root node],
*		'MinLevel'		=> [Minimum tree level],
*		'MaxLevel'		=> [Maximum tree level],
*        )
* </pre>
*
* @return void
* @access public
*/
function GetParentsUnderTree(assetid, root_node, min_level, max_level)
{
	var soapBody	= "\
<ns1:GetParentsUnderTree>\
<AssetID>"+assetid+"</AssetID>\
<RootID>"+root_node+"</RootID>\
<MinLevel>"+min_level+"</MinLevel>\
<MaxLevel>"+max_level+"</MaxLevel>\
</ns1:GetParentsUnderTree>";
	
	return soapBody;
}//end GetParentsUnderTree