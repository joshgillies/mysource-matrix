/**
 * This file contain all the SOAP Body used to request the Search Service API.
 * 
 */


/**
* Description: This operation allow basic searching of content
*
* @param array  $request_info	$request information
* <pre>
* Array (
* 		'AssetTypes'		=> [Asset Types in Matrix]
* 		'Limit'				=> [The maximum number of results returned]
* 		'RootIDs'			=> [Search under the specified root nodes]
* 		'ResultFormat'		=> [The final output of the web service. E.g: %asset_name% - %asset_created%]
*		'Statuses'			=> [Only search for the specified statuses],
*		'ExcludeRootNodes'  => [Whether to exclude root nodes in search result],
*        )
* </pre>
*
* @return void
* @access public
*/
function BasicSearch(asset_types, limit, rootids, result_format, statuses, exclude_root_nodes)
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
"<ExcludeRootNodes>"+exclude_root_nodes+"</ExcludeRootNodes>"+
"</ns1:BasicSearch>";

	return soapBody;
	
}//end BasicSearch()


/**
* Description: This operation allow advanced searching of content
*
* @param array  $request_info	$request information
* <pre>
* Array (
* 		'AssetTypes'		=> [Asset Types in Matrix]
* 		'ExcludeWords'		=> [Words to be excluded from the search]
* 		'FieldLogic'		=> [Either AND or OR results between fields]
* 		'Limit'				=> [The maximum number of results returned]
* 		'RootIDs'			=> [Search under the specified root nodes]
* 		'RootLogic'			=> [Either AND or OR results between rootnodes]
* 		'ResultFormat'		=> [The final output of the web service. E.g: %asset_name% - %asset_created%]
*		'Statuses'			=> [Only search for the specified statuses],
*		'SearchFields'      => [Search fields],
*		'ExcludeRootNodes'  => [Whether to exclude root nodes in search result],
*        )
* </pre>
*
* @return void
* @access public
*/
function AdvancedSearch(asset_types, exclude_words, field_logic, limit, result_format, rootids, root_logic, statuses, search_fields, exclude_root_nodes)
{
	var asset_types_str		= multiple_elements_to_string(asset_types, "AssetTypes");
	var exclude_words_str	= multiple_elements_to_string(exclude_words, "ExcludeWords");
	var rootids_str			= multiple_elements_to_string(rootids, "RootIDs");
	var statuses_str		= multiple_elements_to_string(statuses, "Statuses");
	
	/* Example of Search Fields 

    var standard_field = Array (
                        'WordLogic'     => 'OR',
                        'SearchTerm'    => 'temp bubble',
                        'DataSources'   => Array (
                                            'FieldType'         => 'Standard',
                                            'StandardOption'    => Array (
                                                                    'FieldName' => 'name',
                                                                   ),
                                           ),
                      	);

	Result in the following XML:
 
	<SearchFields>
		<SearchTerm>temp bubble</SearchTerm>
		<WordLogic>OR</WordLogic>
		<DataSources>
			<FieldType>Standard</FieldType>
			<StandardOption>
				<FieldName>name</FieldName>
			</StandardOption>
		</DataSources>
	</SearchFields>
	*/	
	
	var field_option		= '';
	if (search_fields['DataSources']['StandardOption'] !== null) {
		field_option	= 	'<StandardOption>'+
								'<FieldName>'+search_fields['DataSources']['StandardOption']['FieldName']+'</FieldName>'+
							'</StandardOption>';
	}//end if
	if (search_fields['DataSources']['MetadataOption'] !== null) {
		field_option	= 	'<MetadataOption>'+
								'<MetadataFieldID>'+search_fields['DataSources']['MetadataOption']['MetadataFieldID']+'</MetadataFieldID>'+
							'</MetadataOption>';
	}//end if
	
	var search_field_string	=	'<SearchTerm>'+search_fields['SearchTerm']+'</SearchTerm>'+
	'<WordLogic>'+search_fields['WordLogic']+'</WordLogic>'+
	'<DataSources>'+
		'<FieldType>'+search_fields['DataSources']['FieldType']+'</FieldType>'+
		field_option+
	'</DataSources>';	
	
	var soapBody	= "<ns1:AdvancedSearch>"+
asset_types_str+
exclude_words_str+
"<FieldLogic>"+field_logic+"</FieldLogic>"+
"<Limit>"+limit+"</Limit>"+
"<ResultFormat>"+result_format+"</ResultFormat>"+
rootids_str+
"<RootLogic>"+root_logic+"</RootLogic>"+
statuses_str+
"<SearchFields>"+search_field_string+"</SearchFields>"+
"<ExcludeRootNodes>"+exclude_root_nodes+"</ExcludeRootNodes>"+
"</ns1:AdvancedSearch>";

	return soapBody;

}//end AdvancedSearch()


/**
* Description: This operation allow reindexing of indexable components of an asset
*
* @param array  $request_info	$request information
* <pre>
* Array (
* 		'AssetID'			=> [Asset ID of the asset to be reindexed]
* 		'IndexComponents'	=> [asset, metadata, or all]
*        )
* </pre>
*
* @return void
* @access public
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
