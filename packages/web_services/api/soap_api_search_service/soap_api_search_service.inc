<?php
/**
* +- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - +
* | This MySource Matrix CMS file is Copyright (c) Squiz Pty Ltd       |
* | ACN 084 670 600													   |
* +- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - +
* | IMPORTANT: Your use of this Software is subject to the terms of    |
* | the Licence provided in the file licence.txt. If you cannot find   |
* | this file please contact Squiz (www.squiz.com.au) so we may provide|
* | you a copy.                                                        |
* +- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - +
*
* $Id: soap_api_search_service.inc,v 1.10 2012/08/30 00:59:45 ewang Exp $
*
*/


require_once SQ_PACKAGES_PATH.'/web_services/api/soap_api/soap_api.inc';

/**
* Soap_Api_Search_Service
*
* Purpose
*
*
* @author  Huan Nguyen <hnguyen@squiz.net>
* @version $Revision: 1.10 $
* @package MySource_Matrix_Packages
* @subpackage web_services
*/
class Soap_Api_Search_Service extends Soap_Api
{


    /**
    * Constructor
    *
    * @param int    $assetid    the asset id to be loaded
    *
    */
    function __construct($assetid=0)
    {
        parent::__construct($assetid);

    }//end constructor


    /**
    * Returns a function list of the asset let the children handle this
    *
    * @return string
    * @access private
    * @see Asset::_getName()
    */
    public function getFunctionList()
    {
        return Array(
				'BasicSearch'		=> '1',		// Does not accept search term
        		'AdvancedSearch'	=> '1',
        		'ReIndex'			=> '1',
			   );

    }//end getFunctionList()


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
	*		'ExcludeRootNodes'	=> [Whether to exclude root nodes in search result],
    *        )
    * </pre>
	*
	* @return void
	* @access public
	*/
	function BasicSearch($request)
	{
		$request_info = (Array) $request;
		$search_asset_types			= array_get_index($request_info, 'AssetTypes', '');
		$search_limit				= array_get_index($request_info, 'Limit', '');
		$search_roots				= array_get_index($request_info, 'RootIDs', '');
		$search_result_format		= array_get_index($request_info, 'ResultFormat', '');
		$search_restrict_statuses	= array_get_index($request_info, 'Statuses', '');
		$exclude_root_nodes			= self::getBooleanIndex($request_info, 'ExcludeRootNodes', FALSE);

		if (!is_array($search_restrict_statuses)) {
			$search_restrict_statuses	= Array ($search_restrict_statuses);
		}//end if

		if (isset($search_asset_types->AssetType)) {
			$search_asset_types	= Array ($search_asset_types);
		}//end if

		$final_types_array	= Array();
		foreach ($search_asset_types as $type_info)
		{
			$final_types_array[strtolower($type_info->AssetType)]	= $type_info->Inherit;
		}//end foreach

		if (!is_array($search_roots)) {
			$search_roots	= Array ($search_roots);
		}//end if


		$installed	= $GLOBALS['SQ_SYSTEM']->am->installed('search_manager');
		if ($installed) {
			$sm = $GLOBALS['SQ_SYSTEM']->am->getSystemAsset('search_manager');
			$search_info = Array(
							'limit'			=> $search_limit,
							'asset_types'	=> $final_types_array,
							'statuses'		=> $search_restrict_statuses,
							'roots'			=> $search_roots,
						   );

			$results = $sm->processBasicSearch($search_info, TRUE);
			$final_result	= Array();
			if (!empty($results)) {
				foreach ($results as $assetid => $info) {
					if ($exclude_root_nodes) {
						if (in_array($assetid, $search_roots)) {
							unset($results[$assetid]);
							continue;
						}//end if
					}//end if
					if (!empty($search_result_format)) {
						$final_result[]	=	self::formatAssetResult($assetid, $search_result_format);
					} else {
						$final_result[] = $assetid;
					}
				}//end foreach
			}

			return Array (
					'BasicSearchResult'	=> $final_result,
				   );

		} else {
			throw new SoapFault('Server', 'Search Manager is not installed');
		}//end else

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
	*		'SearchFields'		=> [Search fields],
	*		'ExcludeRootNodes'	=> [Whether to exclude root nodes in search result],
    *        )
    * </pre>
	*
	* @return void
	* @access public
	*/
	function AdvancedSearch($request)
	{
		$request_info = (Array) $request;
		$search_asset_types			= array_get_index($request_info, 'AssetTypes', '');
		$exclude_words				= array_get_index($request_info, 'ExcludeWords', '');
		$search_field_logic			= array_get_index($request_info, 'FieldLogic', 'AND');
		$search_word_logic			= array_get_index($request_info, 'WordLogic', 'AND');
		$search_limit				= array_get_index($request_info, 'Limit', '');
		$search_result_format		= array_get_index($request_info, 'ResultFormat', '');
		$search_roots				= array_get_index($request_info, 'RootIDs', '');
		$root_logic					= array_get_index($request_info, 'RootLogic', 'AND');
		$search_restrict_statuses	= array_get_index($request_info, 'Statuses', '');
		$search_fields				= array_get_index($request_info, 'SearchFields', '');
		$exclude_root_nodes			= self::getBooleanIndex($request_info, 'ExcludeRootNodes', FALSE);

		if (!is_array($search_restrict_statuses)) {
			$search_restrict_statuses	= Array ($search_restrict_statuses);
		}//end if

		$installed	= $GLOBALS['SQ_SYSTEM']->am->installed('search_manager');
		if ($installed) {
			$sm = $GLOBALS['SQ_SYSTEM']->am->getSystemAsset('search_manager');

			$field_name_count	= 0;
			$final_search_field	= Array();
			if (!empty($search_fields)) {
				if (isset($search_fields->SearchTerm)) {
					$search_fields = Array ($search_fields);
				}//end if
				foreach ($search_fields as $field_info) {
					$new_field_info	= Array();
					$new_field_info['word_logic']	= $field_info->WordLogic;
					$new_field_info['words']		= $field_info->SearchTerm;
					$new_field_info['data_sources']	= Array();

					if (isset($field_info->DataSources->FieldType)) {
						$field_info->DataSources	= Array (
														$field_info->DataSources,
													  );
					}//end if

					foreach ($field_info->DataSources as $data_source) {
						if ($data_source->FieldType == 'Metadata') {
							$data_source_new	= Array (
													'type'		=> 'metadata',
													'params'	=> Array (
																	'assetid'		=> $data_source->MetadataOption->MetadataFieldID,
																   ),
												  );
						} else if ($data_source->FieldType == 'Attribute') {
							$data_source_new	= Array (
													'type'		=> 'asset_attrib',
													'params'	=> Array (
																	'asset_type'	=> $data_source->AttributeOption->AssetType,
																	'attrid'		=> $data_source->AttributeOption->AttributeID,
																   ),
												  );
						} else if ($data_source->FieldType == 'Standard') {
							$data_source_new	= Array (
													'type'		=> 'standard',
													'params'	=> Array (
																	'field'			=> $data_source->StandardOption->FieldName,
																   ),
												  );
						} else if ($data_source->FieldType == 'IncludeAll') {
							$data_source_new	= Array (
													'type'		=> 'include_all',
													'params'	=> Array (),
												  );
						}
						$new_field_info['data_sources'][]	= $data_source_new;

					}//end foreach

					if (!isset($new_field_info['name'])) {
						$new_field_info['name']	= 'FieldName'.$field_name_count;
					}//end if
					$final_search_field['fieldname'.$field_name_count]	= $new_field_info;
					$field_name_count++;
				}//end foreach
			}//end if

			if (isset($search_asset_types->AssetType)) {
				$search_asset_types	= Array ($search_asset_types);
			}//end if
			$final_types_array	= Array();
			foreach ($search_asset_types as $type_info)
			{
				$final_types_array[strtolower($type_info->AssetType)]	= $type_info->Inherit;
			}//end foreach

			if (!is_array($search_roots)) {
				$search_roots	= Array ($search_roots);
			}//end if

			$search_info	= Array(
								'asset_types'	=> $final_types_array,
								'field_logic'	=> $search_field_logic,
								'limit'			=> $search_limit,
								'statuses'		=> $search_restrict_statuses,
								'roots'			=> $search_roots,
								'root_logic'	=> $root_logic,
								'statuses'		=> $search_restrict_statuses,
								'fields'		=> $final_search_field,
							   );
			if (!empty($exclude_words)) {
				if (!is_array($exclude_words)) {
					$exclude_words	= Array ($exclude_words);
				}//end if
				$search_info['exclude']	= $exclude_words;
			}//end if

			$results = $sm->processSearch($search_info, TRUE);
			$final_result	= Array();
			if (!empty($results)) {
				$i = 0;
				foreach ($results as $index	=> $asset_info) {
					if ($exclude_root_nodes) {
						if (in_array($asset_info['assetid'], $search_roots)) {
							unset($results[$index]);
							continue;
						}//end if
					}//end if

					if (!empty($search_result_format)) {
						$asset_print_formatted	= self::formatAssetResult($asset_info['assetid'], $search_result_format);
					} else {
						$asset_print_formatted	= $asset_info['assetid'];
					}//end else

					$final_result[$i]['AssetDisplay']	= $asset_print_formatted;
					$final_result[$i]['SearchScore']	= $asset_info['search_score'];
					$final_result[$i]['Source']			= $asset_info['source'];
					$i++;
				}//end foreach
			}

			return Array (
					'AdvancedSearchResult'	=> $final_result,
				   );
		} else {
			throw new SoapFault('Server', 'Search Manager is not installed');
		}//end else

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
	function ReIndex($request)
	{
		$request_info	= (Array) $request;
		$reindex_assetid	= array_get_index($request_info, 'AssetID', '');
		$reindex_components	= array_get_index($request_info, 'IndexComponents', 'all');

		if (!empty($reindex_assetid)) {
			$reindex_asset	= $GLOBALS['SQ_SYSTEM']->am->getAsset($reindex_assetid);
			if (empty($reindex_assetid)) {
				throw new SoapFault('Server', 'There Is No Asset In The System With AssetID '.$reindex_assetid);
			}//end if
			$installed	= $GLOBALS['SQ_SYSTEM']->am->installed('search_manager');
			if ($installed) {
				$sm = $GLOBALS['SQ_SYSTEM']->am->getSystemAsset('search_manager');
				$sm->reindexAsset($reindex_asset, Array($reindex_components));

				return Array (
						'ReIndexResult'	=> TRUE,
					   );
			} else {
				throw new SoapFault('Server', 'Search Manager is not installed');
			}//end else
		}  else {
			throw new SoapFault('Server', 'No Asset ID was found to reindex');
		}//end else

	}//end ReIndex()


	/**
	* This function return an array of asset printed as the format defined in $search_result_format
	*
	* @return array
	* @access private
	*/
	private static function formatAssetResult($assetid, $search_result_format)
	{
		$formatted_assets	= Array();
		require_once SQ_FUDGE_PATH.'/general/text.inc';
		$replacement_keywords	= Array();

		$asset	= $GLOBALS['SQ_SYSTEM']->am->getAsset($assetid);
		$tmp_format	= $search_result_format;
		$keywords	= extract_keywords($search_result_format);
		foreach ($keywords as $keyword) {
			$replacement	=	$asset->getKeywordReplacement($keyword);
			if ($replacement == '%'.$keyword.'%') {
				$replacement = '';
			}//end if
			$replacement_keywords[$keyword]	= $replacement;

		}//end foreach
		$asset_result_print = replace_keywords($tmp_format, $replacement_keywords);

		return $asset_result_print;

	}//end formatAssetResult()


	/**
	* getComplexElements
	*
	* @return array
	* @access public
	*/
	public static function getComplexElements($func_list=Array())
	{
		$obj_optional			= self::getArgType('AssetObject', 0, 1);
		$statuses				= self::getArgType('AssetStatus', 0, 'unbounded');
		$search_logic			= self::getArgType('SearchLogicValue', 0, 1, TRUE);
		$search_fields			= self::getArgType('SearchField', 0, 'unbounded', TRUE);
		$asset_types			= self::getArgType('AssetTypes', 0, 'unbounded');
		$root_nodes				= self::getArgType('string', 0, 'unbounded', TRUE);
		$search_result			= self::getArgType('string', 0, 'unbounded', TRUE);
		$advanced_search_result	= self::getArgType('AdvancedResult', 0, 'unbounded', TRUE);
		$exclude_words			= self::getArgType('string', 0, 'unbounded', TRUE);
		$index_component		= self::getArgType('IndexComponent', 0, 1);

		$complex_types	= Array (
							'BasicSearch'		=> Array (
													'BasicSearch'				=> Array (
											    									'AssetTypes'		=> $asset_types,
																					'Limit'				=> self::$string_non_optional,
																					'Statuses'			=> $statuses,
																					'RootIDs'			=> $root_nodes,
																					'ExcludeRootNodes'	=> self::$boolean_optional,
																					'ResultFormat'		=> self::$string_optional,
																				   ),
													'BasicSearchResponse'		=> Array (
																					'BasicSearchResult'	=> $search_result,
																				   ),
												   ),
							'AdvancedSearch'	=> Array (
													'AdvancedSearch'			=> Array (
											    									'AssetTypes'		=> $asset_types,
												   									'ExcludeWords'		=> $exclude_words,
																					'FieldLogic'		=> self::$string_optional,
																					'Limit'				=> self::$string_non_optional,
																					'ResultFormat'		=> self::$string_optional,
																					'RootIDs'			=> $root_nodes,
												   									'RootLogic'			=> $search_logic,
												   									'ExcludeRootNodes'	=> self::$boolean_optional,
																					'Statuses'			=> $statuses,
												   									'SearchFields'		=> $search_fields,
												   								   ),
												   	'AdvancedSearchResponse'	=> Array (
																					'AdvancedSearchResult'	=> $advanced_search_result,
												   								   ),
												   ),
							'ReIndex'			=> Array (
													'ReIndex'					=> Array (
																					'AssetID'			=> self::$string_non_optional,
												   									'IndexComponents'	=> $index_component,
																				   ),
													'ReIndexResponse'			=> Array (
																					'ReIndexResult'		=> self::$boolean_non_optional,
																				   ),
												   ),

						  );

		$complex_types_available = parent::getComplexElements($complex_types);

		return $complex_types_available;

	}//end getComplexElements();


    /**
    * getComplexTypes: return all the complex types used by the API
    *
    * @return array
    * @access public
    */
    public static function getComplexTypes($func_list=Array())
    {
    	$data_sources		= self::getArgType('DataSource', 0, 'unbounded');
    	$metadata_option	= self::getArgType('MetadataSearchOption', 0, 'unbounded');
    	$attribute_option	= self::getArgType('AttributeSearchOption', 0, 'unbounded');
    	$standard_option	= self::getArgType('StandardSearchOption', 0, 'unbounded');
    	$search_field_type	= self::getArgType('SearchFieldType', 1, 1);
		$assetType_optional		= self::getArgType('AssetType', 0, 'unbounded');
		$asset_type_inhr		= Array (
    								'AssetType'	=> $assetType_optional,
    								'Inherit'	=> self::$string_non_optional,
    							  );

        $complex_types    = Array (
							'AdvancedSearch'		=> Array (
        													'SearchField'			=> Array (
        																				'SearchTerm'			=> self::$string_non_optional,
																						'WordLogic'				=> self::$string_non_optional,
        																				'DataSources'			=> $data_sources,
        																			   ),
                                           			     	'DataSource'			=> Array (
																						'FieldType'				=> $search_field_type,
																						'MetadataOption'		=> $metadata_option,
																						'AttributeOption'		=> $attribute_option,
        																			   	'StandardOption'		=> $standard_option,
                                           			       							   ),
                                           			       	'MetadataSearchOption'	=> Array (
                                           			       								'MetadataFieldID'		=> self::$string_optional,
                                           			       							   ),
                                           			      	'AttributeSearchOption'	=> Array (
        																				'AssetType'				=> self::$string_optional,
                                           			       								'AttributeID'			=> self::$string_optional,
                                           			       							   ),
                                           			      	'StandardSearchOption'	=> Array (
        																				'FieldName'				=> self::$string_optional,
                                           			       							   ),

                                           			       	'AdvancedResult'		=> Array (
                                           			       								'AssetDisplay'			=> self::$string_optional,
                                           			       								'SearchScore'			=> self::$string_optional,
                                           			       								'Source'				=> self::$string_optional,
                                           			       							   ),
															'AssetTypes'			=> $asset_type_inhr,
                                           			   ),
							'BasicSearch'			=> Array (
                                           			   		'AssetTypes'			=> $asset_type_inhr,
                                           			   ),
                          );

        $complex_types_available = parent::getComplexElements($complex_types);

        return $complex_types_available;

    }//end getComplexTypes()


	/**
	* getSimpleRestrictedTypes
	*
	* @return array
	* @access public
	*/
	public static function getSimpleRestrictedTypes($func_list=Array())
	{
		$asset_types_info 			= $GLOBALS['SQ_SYSTEM']->am->getAssetTypes(TRUE, TRUE);
		$asset_types	= Array();
		foreach ($asset_types_info as $asset_type_info) {
			$asset_types[]	= $asset_type_info['type_code'];
		}//end foreach

		$simple_restricted_types	= Array (
										'BasicSearch'	=> Array (
															'AssetType'			=> Array (
																					'restriction_base'	=> 'string',
																					'enumeration'		=> $asset_types,
																				   ),
															'AssetStatus'		=> Array (
																				   	'restriction_base'	=> 'string',
																					'enumeration'		=> Array (
																											'1','2','4','8','16','32','64','128','256',
																				   						   ),
																				   ),
															'SearchLogicValue'	=> Array (
																					'restriction_base'	=> 'string',
																					'enumeration'		=> Array (
																				   							'AND', 'OR',
																			 	  						   ),
																				   ),
														   ),
										'ReIndex'		=> Array (
														   	'IndexComponent'	=> Array (
														   							'restriction_base'	=> 'string',
														   							'enumeration'		=> Array (
														   													'asset', 'metadata', 'all'
														   												   ),
														   						   ),
														   ),
										'AdvancedSearch'=> Array (
														   	'SearchFieldType'	=> Array (
														   							'restriction_base'	=> 'string',
														   							'enumeration'		=> Array (
														   													'Metadata', 'Attribute', 'Standard', 'IncludeAll',
														   												   ),
														   						   ),
															'AssetType'			=> Array (
																					'restriction_base'	=> 'string',
																					'enumeration'		=> $asset_types,
																				   ),
														   ),
									  );

		$simple_restricted_types_available = parent::processSimpleRestrictedTypes($simple_restricted_types);

		return $simple_restricted_types_available;

	}//end getSimpleRestrictedTypes()


}//end class
?>
