/**
 * This file contain all the SOAP Body used to request the Asset Service API.
 * 
 */


/**
* Description: This operation will create an asset of a specific type under a specific location
*
* @param array  $request	The request information
* <pre>
* Array (
*        'TypeCode'				=> [type code of new asset],
*        'Name'					=> [name for new asset],
*        'ParentID'				=> [parentid of the new parent],
*        'LinkType'				=> [LinkType],
*        'LinkValue'			=> [link value],
*        'SortOrder'			=> [link sort order],
*        'IsDependant'			=> [0|1],
*        'IsExclusive'			=> [0|1],
*        'FileName'				=> [name of the file],
*        'FileContentBase64'	=> [base64 encoded file content],
*        'AttributeInfo'		=> [attribute name/value pairs]
*        )
* </pre>
*
* @return void
* @access public
*/
function CreateAsset(parentid, type_code, name, link_type, link_value, sort_order, is_dependant, is_exclusive, file_name, file_content_base64, attribute_info)
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
<FileName>"+file_name+"</FileName>\
<FileContentBase64>"+file_content_base64+"</FileContentBase64>";
	
	var attr_str = "";
	for (var i in attribute_info) {
		attr_str += "\
<AttributeInfo>\
<AttributeName>"+attribute_info[i][0]+"</AttributeName>\
<AttributeValue>"+attribute_info[i][1]+"</AttributeValue>\
</AttributeInfo>";
	}

	soapBody += attr_str+"\
</ns1:CreateAsset>";

	return soapBody;
	
}//end CreateAsset


/**
* Description: This operation will return an asset object based on assetid
*
* @param array  $request	The request information
* <pre>
* Array (
*		 'AssetID'		=> [Asset ID of the asset in query],
*        )
* </pre>
*
* @return void
* @access public
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
* Description: This operation will return all URLs associated with an asset
*
* @param string  $request  The request information
*
* <pre>
* Array (
*		 'AssetID'	 => [The asset id we are trying to get url for],
*        )
* </pre>
*
* @return void
* @access public
*/
function GetAssetURLs(assetid)
{
	var soapBody	= "\
<ns1:GetAssetURLs>\
<AssetID>"+assetid+"</AssetID>\
</ns1:GetAssetURLs>";
	return soapBody;	
}//end GetAssetURLs


/**
* Description: This operation will return an asset object based on a URL
*
* @param string  $request  The request information
*
* <pre>
* Array (
*		 'URL'	 => [The URL belongs to the asset being searched for],
*        )
* </pre>
*
* @return void
* @access public
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
* Description: This operation will return all available statuses of an asset based on assetid
*
* @param string  $request  The request information
*
* <pre>
* Array (
*		 'AssetID'	 => [The ID of the asset in query],
*        )
* </pre>
*
* @return void
* @access public
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
* Description: This operation will return set the attribute value of an asset based on assetid and attribute name
*
* @param string  $request  The request information
*
* <pre>
* Array (
*		'AssetID'	 		=> [The ID of the asset in query],
* 		'AttributeName'		=> [The name of the attribute],
* 		'AttributeValue'	=> [The new value of the attribute],
*        )
* </pre>
*
* @return void
* @access public
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
* Description: This operation will send an asset to the trash
*
* @param string  $request  The request information
*
* <pre>
* Array (
*		'AssetID'	 		=> [The ID of the asset in query],
*        )
* </pre>
*
* @return void
* @access public
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
* Description: This operation will clone an asset to a specified location
*
* @param string  $request  The request information
*
* <pre>
* Array (
*		'AssetID'	 			 => [The ID of the asset in query],
*		'NewParentID'			 => [The new parent]
*		'NumberOfClone'			 => [Number of asset to be cloned],
*		'PositionUnderNewParent' => [Position Under New Parent (sort order)]
*		'LinkType'				 => [Creation link type]
*        )
* </pre>
*
* @return void
* @access public
*/
function CloneAsset(assetid, new_parentid, num_clone, position, link_type)
{
	var soapBody	= "\
<ns1:CloneAsset>\
<AssetID>"+assetid+"</AssetID>\
<NewParentID>"+new_parentid+"</NewParentID>\
<NumberOfClone>"+num_clone+"</NumberOfClone>\
<PositionUnderNewParent>"+position+"</PositionUnderNewParent>\
<LinkType>"+link_type+"</LinkType>\
</ns1:CloneAsset>";
	return soapBody;
	
}//end CloneAsset


/**
* Description: This operation will return all attributes belong to an asset type
*
* @param string  $request  The request information
*
* <pre>
* Array (
*		'TypeCode'	 		=> [The ID of the asset in query],
*		'AttributeDetail'	=> [The Details of the attribute e.g details = new Array('name', 'attrid')],
*        )
* </pre>
*
* @return void
* @access public
*/
function GetAssetTypeAttribute(type_code, details)
{
	var soapBody	= "\
<ns1:GetAssetTypeAttribute>\
<TypeCode>"+type_code+"</TypeCode>";

for (var i in details) {
	soapBody += "\
<AttributeDetail>"+details[i]+"</AttributeDetail>";
}

soapBody += "\
</ns1:GetAssetTypeAttribute>";
	return soapBody;
	
}//end GetAssetTypeAttribute()


/**
* Description: This operation will return all webpaths belong to an asset type
*
* @param string  $request  The request information
*
* <pre>
* Array (
*		'AssetID'	 		=> [The ID of the asset in query],
*        )
* </pre>
*
* @return void
* @access public
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
* Description: This operation will return all available keywords of an asset
*
* @param string  $request  The request information
*
* <pre>
* Array (
*		'AssetID'	 		=> [The ID of the asset in query],
*        )
* </pre>
*
* @return void
* @access public
*/
function GetAssetAvailableKeywords(assetid)
{
	var soapBody	= "\
<ns1:GetAssetAvailableKeywords>\
<AssetID>"+assetid+"</AssetID>\
</ns1:GetAssetAvailableKeywords>";
	return soapBody;
}//end GetAssetAvailableKeywords()


/**
* Description: This operation will return all available keywords of an asset on Paint Layout
*
* @param string  $request  The request information
*
* <pre>
* Array (
*		'AssetID'	 		=> [The ID of the asset in query],
*		'TypeCode'			=> [The type code to get keywords for]
*        )
* </pre>
*
* @return void
* @access public
*/
function GetPaintLayoutAvailableKeywords(assetid, type_code)
{
	var soapBody	= "\
<ns1:GetPaintLayoutAvailableKeywords>\
<AssetID>"+assetid+"</AssetID>\
<TypeCode>"+type_code+"</TypeCode>\
</ns1:GetPaintLayoutAvailableKeywords>";

	return soapBody;
}//end GetPaintLayoutAvailableKeywords()


/**
* Description: This operation will return all attributes belong to an asset type classified by attribute name
*
* @param string  $request  The request information
*
* <pre>
* Array (
*		'AssetIDs'	 		=> [The ID of the asset in query],
*		'AttributeName'		=> [The Name of the attribute in query],
*		'TypeCode'			=> [The Type Code of the asset],
*        )
* </pre>
*
* @return array
* @access public
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
* Description: This operation will set tag for an asset
*
* @param string  $request  The request information
*
* <pre>
* Array (
*		'AssetIDs'	 		=> [The ID of the asset in query],
*		'ThesaurusID'		=> [The ID of the thesaurus where the tag is from],
*		'TagName'			=> [The tag name]
*		'Weight'			=> [Weight of the tag on the asset]
*		'CascadeTagChange	=> [Cascade the tag to all children]
*        )
* </pre>
*
* @return void
* @access public
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
* Description: This operation will set tag for an asset
*
* @param string  $request  The request information
*
* <pre>
* Array (
*		'AssetIDs'	 		=> [The ID of the asset in query],
*		'ThesaurusIDs'		=> [The ID of the thesaurus where the tag is from],
*        )
* </pre>
*
* @return void
* @access public
*/
function GetTags(assetid, thesaurus_ids)
{
	var soapBody	= "\
<ns1:SetTag>\
<AssetID>"+assetid+"</AssetID>\
<ThesaurusIDs>"+thesaurus_ids+"</ThesaurusIDs>\
</ns1:SetTag>";
	return soapBody;
}//end GetTags()


/**
* Description: This operation will return all statuses of the asset in Matrix
*
* @param string  $request  The request information
*
* @return void
* @access public
*/
function GetAllStatuses()
{
	var soapBody	= "\
<ns1:GetAllStatuses>\
</ns1:GetAllStatuses>";
	return soapBody;	
}//end GetAllStatuses()


/**
* Description: This function return the asset information (name, typecode, children assets, lineage) about the requested asset.
*
* @param array  $request	The request information
* <pre>
* Array (
*        'AssetIDs'				=> [The ID of the asset in query],
*        'FinderAttributes'		=> [children|lineage],
*        'RootNode'				=> [The ID of the root asset in query],
*        )
* </pre>
*
* @return void
* @access public
*/
function GetAssetsInfo(assetid, finder_attr, root_assetid)
{
	var soapBody	= "\
<ns1:GetAssetsInfo>\
<AssetIDs>"+assetid+"</AssetIDs>\
<FinderAttributes>"+finder_attr+"</FinderAttributes>\
<RootNode>"+root_assetid+"</RootNode>\
</ns1:GetAssetsInfo>";
	return soapBody;
	
}//end GetAssetsInfo


/**
* Description: This operation will return all the type descendants of an asset type.
*
* @param string  $request  The request information
*
* <pre>
* Array (
*		'TypeCode'	 		=> [The ID of the asset in query]
*        )
* </pre>
*
* @return void
* @access public
*/
function GetAssetTypeDescendants(type_code)
{
	var soapBody	= "\
<ns1:GetAssetTypeDescendants>\
<TypeCode>"+type_code+"</TypeCode>\
</ns1:GetAssetTypeDescendants>";
	return soapBody;
	
}//end GetAssetTypeDescendants()


/**
* Description: This function return the page contents for asset in query
*
* @param array  $request	The request information
* <pre>
* Array (
*        'AssetID'				=> [The ID of the asset in query],
*        'RootNodeID'			=> [The ID of the root asset in query],
*        )
* </pre>
*
* @return void
* @access public
*/
function GetPageContents(assetid, root_assetid)
{
	var soapBody	= "\
<ns1:GetPageContents>\
<AssetID>"+assetid+"</AssetID>\
<RootNodeID>"+root_assetid+"</RootNodeID>\
</ns1:GetPageContents>";
	return soapBody;
	
}//end GetPageContents


/**
* Description: This operation will return a SessionID and SessionKey on successful validation
*
* @param array  $request	The request information
* <pre>
* Array (
*        'Username'
*        'Password'	
*        )
* </pre>
*
* @return void
* @access public
*/
function LoginUser(username, password)
{
	var soapBody	= "\
<ns1:LoginUser>\
<Username>"+username+"</Username>\
<Password>"+password+"</Password>\
</ns1:LoginUser>";
	return soapBody;
	
}//end LoginUser


/**
* Description: This operation will return the user asset ID of the request username
*
* @param array  $request	The request information
* <pre>
* Array (
*        'Username'
*        )
* </pre>
*
* @return void
* @access public
*/
function GetUserIdByUsername(username)
{
	var soapBody	= "\
<ns1:GetUserIdByUsername>\
<Username>"+username+"</Username>\
</ns1:GetUserIdByUsername>";
	return soapBody;
	
}//end GetUserIdByUsername


/**
* Description: This operation will set status for an asset
*
* @param array  $request	The request information
* <pre>
* Array (
*        'AssetID'			=> [The ID of the asset in query],
*        'StatusValue'		=> [e.g Live = 16],
*        'DependantsOnly'	=> [0|1],
*        )
* </pre>
*
* @return void
* @access public
*/
function SetAssetStatus(assetid, status, dependants_only)
{
	var soapBody	= "\
<ns1:SetAssetStatus>\
<AssetID>"+assetid+"</AssetID>\
<StatusValue>"+status+"</StatusValue>\
<DependantsOnly>"+dependants_only+"</DependantsOnly>\
</ns1:SetAssetStatus>";
	return soapBody;
	
}//end SetAssetStatus


/**
* Description: This operation will set webpath(s) for an asset
*
* @param array  $request	The request information
* <pre>
* Array (
*        'AssetID'			=> [The ID of the asset in query],
*        'Path'				=> Array [this_path, that_path],
*        'AddRemaps'		=> [0|1],
*        )
* </pre>
*
* @return void
* @access public
*/
function SetAssetWebPaths(assetid, paths, add_remaps)
{
	var soapBody	= "\
<ns1:SetAssetWebPaths>\
<AssetID>"+assetid+"</AssetID>\
<AddRemaps>"+add_remap+"</AddRemaps>";

	var attr_str = "";
	for (var i in paths) {
		attr_str += "\
<Path>"+paths[i]+"</Path>\
";
	}

	soapBody += attr_str+"\
</ns1:SetAssetWebPaths>";
	return soapBody;

}//end SetAssetWebPaths

