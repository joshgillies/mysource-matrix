/**
 * This file contain all the SOAP Body used to request the Search Service API.
 * 
 */


/**
 * This operation allow basic searching of content
 */
function BasicSearch(asset_types, limit, rootids, result_format, statuses)
{
	var asset_types_str	= multiple_elements_to_string(asset_types, "AssetTypes");
	var rootids_str		= multiple_elements_to_string(rootids, "RootIDs");
	var statuses_str	= multiple_elements_to_string(statuses, "Statuses");
	
	var soapBody	= "<ns1:BasicSearch>"+
asset_types_str+
"<Limit>"+limit+"</Limit>"+
rootids_str+
"<ResultFormat>"+result_format+"</ResultFormat>"+
statuses_str+
"</ns1:BasicSearch>";
	return soapBody;
	
}//end BasicSearch()


/**
 * This operation allow advanced searching of content
 */
function AdvancedSearch(asset_types, exclude_words, field_logic, limit, result_format, rootids, root_logic, statuses)
{
	var asset_types_str		= multiple_elements_to_string(asset_types, "AssetTypes");
	var exclude_words_str	= multiple_elements_to_string(exclude_words, "ExcludeWords");
	var rootids_str			= multiple_elements_to_string(rootids, "RootIDs");
	var statuses_str		= multiple_elements_to_string(statuses, "Statuses");
	
	var soapBody	= "<ns1:AdvancedSearch>"+
asset_types_str+
exclude_words_str+
"<FieldLogic>"+field_logic+"</FieldLogic>"+
"<Limit>"+limit+"</Limit>"+
"<ResultFormat>"+result_format+"</ResultFormat>"+
rootids_str+
"<RootLogic>"+root_logic+"</RootLogic>"+
statuses_str+
"</ns1:AdvancedSearch>";
	return soapBody;

}//end AdvancedSearch()


/**
 * This operation allow reindexing of indexable components of an asset
 */
function ReIndex(assetid, components)
{
	var components_str		= multiple_elements_to_string(components, "IndexComponents");
	
	var soapBody	= "\
<ns1:ReIndex>\
<AssetID>"+assetid+"</AssetID>\
"+components_str+"\
</ns1:ReIndex>";
	return soapBody;
	
}//end ReIndex()