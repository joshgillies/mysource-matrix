<?php
/**
* +- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - +
* | This MySource Matrix Module file is Copyright (c) Squiz Pty Ltd    |
* | ACN 084 670 600													   |
* +- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - +
* | IMPORTANT: This Module is not available under an open source	   |
* | license and consequently distribution of this and any other files  |
* | that comprise this Module is prohibited. You may only use this	   |
* | Module if you have the written consent of Squiz.				   |
* +- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - +
*
* $Id: soap_api_asset_service.inc,v 1.1.2.2 2009/05/01 05:50:25 hnguyen Exp $
*
*/

check_ssv_licence();
require_once SQ_PACKAGES_PATH.'/web_services/api/soap_api/soap_api.inc';

/**
* Soap_Api_Asset_Service
*
* Purpose
*
*
* @author  Huan Nguyen <hnguyen@squiz.net>
* @version $Revision: 1.1.2.2 $
* @package MySource_Matrix_Packages
* @subpackage web_services
*/
class Soap_Api_Asset_Service extends Soap_Api
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
				'CreateAsset'				=> '1',
				'CloneAsset'				=> '1',
        		'GetAllStatuses'			=> '1',
				'GetAsset'					=> '1',
				'GetAssetFromURL'			=> '1',
        		'GetAssetTypeAttribute'		=> '1',
				'GetAttributeValuesByName'	=> '1',
        		'GetAvailableKeywords'		=> '1',
				'GetAssetAvailableStatuses'	=> '1',
        		'GetAssetWebPaths'			=> '1',
				'GetURLs'					=> '1',
				'SetAttributeValue'			=> '1',
        		'SetTag'					=> '1',
        		'TrashAsset'				=> '1',
			   );

    }//end getFunctionList()


	/**
	* Description: This operation will create an asset of a specific type under a specific location
	*
    * @param array  $request	The request information
    * <pre>
    * Array (
	*		 'TypeCode'		=> [type code of new asset],
	*		 'Name'			=> [name for new asset],
	*		 'ParentID'		=> [parentid of the new parent],
    *        'LinkType'		=> [LinkType],
    *        'LinkValue'	=> [link value],
    *        'SortOrder'	=> [link sort order],
    *        'IsDependant'	=> [0|1],
    *        'IsExclusive'	=> [0|1]
    *        )
    * </pre>
	*
	* @return void
	* @access public
	*/
	function CreateAsset($request)
	{
		$request_info 	= (Array) $request;
		$parentid		= array_get_index($request_info, 'ParentID', '');
		$type_code		= array_get_index($request_info, 'TypeCode', '');
		$type_code		= strtolower($type_code);
		$asset_name		= array_get_index($request_info, 'Name', '');
		$link_type		= array_get_index($request_info, 'LinkType', '1');
		$link_value		= array_get_index($request_info, 'LinkValue', '');
		$sort_order		= array_get_index($request_info, 'SortOrder', '');
		$is_dependant	= array_get_index($request_info, 'IsDependant', '0');
		$is_exclusive	= array_get_index($request_info, 'IsExclusive', '0');

		$parent_asset = $GLOBALS['SQ_SYSTEM']->am->getAsset($parentid);

		$GLOBALS['SQ_SYSTEM']->am->includeAsset($type_code);
		$object_name = str_replace(' ', '_', ucwords(str_replace('_', ' ', $type_code)));
		$asset = new $object_name();
		$asset->setAttrValue('name', $asset_name);

		$request_info['asset']	= $parent_asset;

		$link_info	= Array (
						'asset'			=> $parent_asset,
						'name'			=> $asset_name,
						'link_type'		=> $link_type,
						'value'			=> $link_value,
						'sort_order'	=> $sort_order,
						'is_dependant'	=> $is_dependant,
						'is_exclusive'	=> $is_exclusive,
					  );

		$linkid = $asset->create($link_info);
		if ($linkid) {
			return Array (
					'CreateMessage' => 'Asset was created successfully. Link id #'.$linkid,
					'NewAssetID'	=> $asset->id,
					'LinkID'		=> $linkid,
				   );
		}//end if

		throw new SoapFault('Server', 'Unable to create asset');

	}//end CreateAsset()


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
	function GetAsset($request)
	{
		$request_info 	= (Array) $request;
		$assetid		= array_get_index($request_info, 'AssetID', '');
		if (!empty($assetid)) {
			$asset	= $GLOBALS['SQ_SYSTEM']->am->getAsset($assetid);
			$asset_xml	= '';
			if (!is_null($asset)) {
				$asset_xml	= self::objectToXML($asset);
			}//end if
			return Array (
					'GetAssetResult'	=> $asset_xml,
				   );
		} else {
			throw new SoapFault('Server', 'Asset ID is not valid. Please provide a valid Asset ID');
		}//end else

	}//end GetAsset()


	/**
	* This recursive function construct XML from a PHP object
	*
	* @param $object	object	The PHP object being converted to XML
	*
	* @return string
	* @access private
	*/
	private static function objectToXML($object)
	{
	    $object_xml = NULL;
	    foreach ($object as $key    => $value) {
	    	if (empty($value)) {
	    		$object_xml .=  '<'.$key.' />';
	    	} else {
		        $object_xml .=  '<'.$key.'>';
		        if (is_array($value)) {
		            $object_xml .= self::objectToXML($value);
		        } else {
		            $object_xml .=  $value;
		        }//end else
		        $object_xml .=  '</'.$key.'>';
	    	}//end else
	    }//end foreach
	    if (is_object($object)) {
	        $root_element_name  = get_class($object);
	        $object_xml = '<'.$root_element_name.'>'.$object_xml.'</'.$root_element_name.'>';
	    }//end if

	    return $object_xml;

	}//end if


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
	function GetURLs($request)
	{
		$request_info 	= (Array) $request;
		$assetid		= array_get_index($request_info, 'AssetID', '');
		if (!empty($assetid)) {
			$urls_info	= $GLOBALS['SQ_SYSTEM']->am->getURLs($assetid);
			$urls	= Array();
			foreach ($urls_info as $info) {
				$protocol	= '';
				if ($info['http'] == '1') {
					$urls[]	= 'http://'.$info['url'];
				}//end if
				if ($info['https'] == '1') {
					$urls[]	= 'https://'.$info['url'];
				}//end if
			}//end foreach
			return Array (
					'GetURLsResult'	=> $urls,
				   );
		} else {
			throw new SoapFault('Server', 'Asset ID is not valid. Please provide a valid Asset ID');
		}//end else

	}//end GetURLs()


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
	function GetAssetFromURL($request)
	{
		$request_info 	= (Array) $request;
		$url			= array_get_index($request_info, 'URL', '');
		if (!empty($url)) {
			$pattern    = '/(http|https):\/\//';
			$url_split	= preg_split($pattern, $url, -1, PREG_SPLIT_DELIM_CAPTURE);
			$protocol	= $url_split[1];
			$main_url	= $url_split[2];
			$asset	= $GLOBALS['SQ_SYSTEM']->am->getAssetFromURL($protocol, $main_url);
			$asset_xml	= '';
			if (!is_null($asset)) {
				$asset_xml	= self::objectToXML($asset);
			}//end if
			return Array (
					'GetAssetFromURLResult'	=> $asset_xml,
				   );
		} else {
			throw new SoapFault('Server', 'The URL must not be empty');
		}//end else

	}//end GetURLs()


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
	function GetAssetAvailableStatuses($request)
	{
		$request_info 	= (Array) $request;
		$assetid			= array_get_index($request_info, 'AssetID', '');
		if (!empty($assetid)) {
			$asset	= $GLOBALS['SQ_SYSTEM']->am->getAsset($assetid);
			$statuses	= $asset->getAvailableStatii();
			return Array (
					'GetAssetAvailableStatusesResult'	=> array_keys($statuses),
				   );
		} else {
			throw new SoapFault('Server', 'Asset ID is not valid. Please provide a valid Asset ID');
		}//end else

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
	function SetAttributeValue($request)
	{
		$request_info 	= (Array) $request;
		$assetid			= array_get_index($request_info, 'AssetID', '');
		$attr_name			= array_get_index($request_info, 'AttributeName', '');
		$attr_val			= array_get_index($request_info, 'AttributeValue', '');
		if (!empty($assetid) && !empty($attr_name) && !empty($attr_val)) {
			$asset	= $GLOBALS['SQ_SYSTEM']->am->getAsset($assetid);
			$GLOBALS['SQ_SYSTEM']->am->acquireLock($assetid, 'attributes');
			$asset->setAttrValue($attr_name, $attr_val);
			$asset->saveAttributes();
			$GLOBALS['SQ_SYSTEM']->am->releaseLock($assetid, 'attributes');

			$GLOBALS['SQ_SYSTEM']->am->forgetAsset($asset);
			$asset	= $GLOBALS['SQ_SYSTEM']->am->getAsset($assetid);
			$new_val	= $asset->attr($attr_name);

			return Array (
					'SetAttributeValueResult'	=> $new_val,
				   );
		} else {
			throw new SoapFault('Server', 'Please make sure to provide AssetID, AttributeName and AttributeValue');
		}//end else

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
	function TrashAsset($request)
	{
		$request_info 	= (Array) $request;
		$assetid			= array_get_index($request_info, 'AssetID', '');
		if (!empty($assetid)) {
			$already_in_trash	= $GLOBALS['SQ_SYSTEM']->am->assetInTrash($assetid, TRUE);

			$GLOBALS['SQ_SYSTEM']->am->acquireLock($assetid, 'all');
			if ($already_in_trash) {
				throw new SoapFault('Server', 'Asset is already in trash');
			} else {
				$result	= $GLOBALS['SQ_SYSTEM']->am->trashAsset($assetid);
			}//end else
			$GLOBALS['SQ_SYSTEM']->am->releaseLock($assetid, 'all');

			return Array (
					'TrashAssetResult'	=> $result,
				   );
		} else {
			throw new SoapFault('Server', 'Asset ID is not valid. Please provide a valid Asset ID');
		}//end else

	}//end TrashAsset()


	/**
	* Description: This operation will clone an asset to a specified location
	*
    * @param string  $request  The request information
    *
    * <pre>
    * Array (
	*		'AssetID'	 		=> [The ID of the asset in query],
	*		'NumberOfClone'		=> [How many new clone assets]
	*		'NewParentID'		=> [The new parent]
	*		'NumberOfClone'		=> [Number of asset to be cloned],
    *        )
    * </pre>
	*
	* @return void
	* @access public
	*/
	function CloneAsset($request)
	{
		// There is a know issue here while executing a long request, if the number of clones are too many
		// then the client will timeout as the server is still processing the request, the way to fix it is
		// to increase the default_socket_timeout in php.ini from 60(secs) to which ever threshold needed.

		$request_info		= (Array) $request;
		$assetid			= array_get_index($request_info, 'AssetID', '');
		$new_parent_id		= array_get_index($request_info, 'NewParentID', '');
		$clone_num			= array_get_index($request_info, 'NumberOfClone', 1);
		$new_position		= array_get_index($request_info, 'PositionUnderNewParent', -1);
		$link_type			= array_get_index($request_info, 'LinkType', 1);
		if (!empty($assetid) && !empty($new_parent_id)) {
			$hh	= $GLOBALS['SQ_SYSTEM']->getHipoHerder();
			$running_vars	= Array (
								'assets'		=> Array (
													$assetid			=> Array (
																			'number_of_clones'	=> 1,
																		   ),
												   ),
								'to_parent_assetid'	=> $new_parent_id,
								'to_parent_pos'		=> $new_position,
								'link_type'			=> $link_type,
								'value'				=> '',
							  );

			$all_errors	= Array();
			for ($i = 0; $i < $clone_num; $i++) {
				// Need to reset the running vars again as it is passed by reference to the hipo job
				$running_vars_copy	= $running_vars;
				$errors	= $hh->freestyleHipo('hipo_job_clone_assets', $running_vars_copy);

				$all_errors	+= $errors;
			}//end for
			if (empty($all_errors)) {
				$result	= 'Asset Cloned';
				return Array (
						'CloneAssetResult'	=> $result,
					   );
			} else {
				throw new SoapFault('Server', 'Unable To Clone Asset');
			}

		} else {
			throw new SoapFault('Server', 'Asset ID is not valid. Please provide a valid Asset ID');
		}//end else

	}//end CloneAsset()


	/**
	* Description: This operation will return all attributes belong to an asset type
	*
    * @param string  $request  The request information
    *
    * <pre>
    * Array (
	*		'TypeCode'	 		=> [The ID of the asset in query],
	*		'AttributeDetail'	=> [The Details of the attribute],
    *        )
    * </pre>
	*
	* @return void
	* @access public
	*/
	function GetAssetTypeAttribute($request)
	{
		$request_info 	= (Array) $request;
		$type_code			= array_get_index($request_info, 'TypeCode', '');
		$details			= array_get_index($request_info, 'AttributeDetail', Array ('name', 'type'));

		if (!empty($type_code)) {
			$attribute_details	= $GLOBALS['SQ_SYSTEM']->am->getAssetTypeAttributes($type_code, $details);
			$i = 0;
			$final_attribute_results	= Array();
			foreach ($attribute_details as $attribute_name	=> $attribute_type_info) {
				$final_attribute_results[$i]['AttributeName']	= $attribute_name;
				if (isset($attribute_type_info['attrid'])) {
					$final_attribute_results[$i]['AttributeID']	= $attribute_type_info['attrid'];
				}//end if
				if (isset($attribute_type_info['type_code'])) {
					$final_attribute_results[$i]['AttributeTypeCode']	= $attribute_type_info['type_code'];
				}//end if
				if (isset($attribute_type_info['type'])) {
					$final_attribute_results[$i]['AttributeType']	= $attribute_type_info['type'];
				}//end if
				if (isset($attribute_type_info['owning_type_code'])) {
					$final_attribute_results[$i]['AttributeOwningTypeCode']	= $attribute_type_info['owning_type_code'];
				}//end if
				if (isset($attribute_type_info['parameters_type_code'])) {
					$final_attribute_results[$i]['AttributeParametersTypeCode']	= $attribute_type_info['parameters_type_code'];
				}//end if
				if (isset($attribute_type_info['parameters_val'])) {
					$final_attribute_results[$i]['AttributeParametersValue']	= $attribute_type_info['parameters_val'];
				}//end if
				if (isset($attribute_type_info['default_type_code'])) {
					$final_attribute_results[$i]['AttributeDefaultTypeCode']	= $attribute_type_info['default_type_code'];
				}//end if
				if (isset($attribute_type_info['default_val'])) {
					$final_attribute_results[$i]['AttributeDefaultValue']	= $attribute_type_info['default_val'];
				}//end if
				if (isset($attribute_type_info['description'])) {
					$final_attribute_results[$i]['AttributeDescription']	= $attribute_type_info['description'];
				}//end if
				if (isset($attribute_type_info['is_admin'])) {
					$final_attribute_results[$i]['AttributeIsAdmin']	= $attribute_type_info['is_admin'];
				}//end if
				if (isset($attribute_type_info['uniq'])) {
					$final_attribute_results[$i]['AttributeUnique']	= $attribute_type_info['uniq'];
				}//end if
				$i++;
			}//end foreach

			return Array (
					'GetAssetTypeAttributeResult'	=> $final_attribute_results,
				   );
		} else {
			throw new SoapFault('Server', 'Type code is not valid. Please provide a valid type code');
		}//end else

	}//end GetAssetTypeAttribute()


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
	function GetAvailableKeywords($request)
	{
		$request_info 	= (Array) $request;
		$assetid			= array_get_index($request_info, 'AssetID', '');
		if (!empty($assetid)) {
			$asset	= $GLOBALS['SQ_SYSTEM']->am->getAsset($assetid);
			$keywords	= $asset->getAvailableKeywords();
			return Array (
					'GetAvailableKeywordsResult'	=> array_keys($keywords),
				   );
		} else {
			throw new SoapFault('Server', 'Asset ID is not valid. Please provide a valid Asset ID');
		}//end else

	}//end GetAvailableKeywords()


	/**
	* Description: This operation will return all webpaths belong to an asset
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
	function GetAssetWebPaths($request)
	{
		$request_info 	= (Array) $request;
		$assetid			= array_get_index($request_info, 'AssetID', '');
		if (!empty($assetid)) {
			$asset		= $GLOBALS['SQ_SYSTEM']->am->getAsset($assetid);
			$web_path	= $asset->getWebPaths();
			return Array (
					'GetAssetWebPathsResult'	=> $web_path,
				   );
		} else {
			throw new SoapFault('Server', 'Asset ID is not valid. Please provide a valid Asset ID');
		}//end else

	}//end GetAssetWebPaths()



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
	function GetAttributeValuesByName($request)
	{
		$request_info 	= (Array) $request;
		$assetids			= array_get_index($request_info, 'AssetIDs', '');
		$attr_name			= array_get_index($request_info, 'AttributeName', '');
		$type_code			= array_get_index($request_info, 'TypeCode', '');
		if (!empty($assetids) && !empty($attr_name) && !empty($type_code)) {
			$attribute_values	= $GLOBALS['SQ_SYSTEM']->am->getAttributeValuesByName($attr_name, $type_code, $assetids);
			$final_attribute_values	= Array();
			$i	= 0;
			foreach ($attribute_values as $assetid => $attribute_value) {
				$final_attribute_values[$i]['AssetID']			= $assetid;
				$final_attribute_values[$i]['AttributeValue']	= $attribute_value;
				$i++;
			}//end foreach
			return Array (
					'GetAttributeValuesByNameResult'	=> $final_attribute_values,
				   );
		} else {
			throw new SoapFault('Server', 'Make sure to provide AssetIDs, Attribute Name, and Asset Type Code');
		}//end else

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
	function SetTag($request)
	{
		$request_info 	= (Array) $request;
		$assetid			= array_get_index($request_info, 'AssetID', '');
		$thesaurus_id		= array_get_index($request_info, 'ThesaurusID', '');
		$tag_name			= array_get_index($request_info, 'TagName', '');
		$weight				= array_get_index($request_info, 'Weight', '1');
		$cascade			= array_get_index($request_info, 'CascadeTagChange', 0);
		if (!empty($assetid)) {
			$asset					= $GLOBALS['SQ_SYSTEM']->am->getAsset($assetid);
			$tag_manager			= $GLOBALS['SQ_SYSTEM']->getTagManager();
			$restrict_tag_source	= $tag_manager->attr('restrict_tag_source');
			$tagging_thesaurus		= $tag_manager->attr('thesaurus');
			if ($restrict_tag_source) {
				if ($thesaurus_id != $tagging_thesaurus) {
					throw new SoapFault('Server', 'Tag Source Restriction is enabled. The provided thesaurus can not be used to tag.');
				}//end if
			}//end if

			$thesaurus	= $GLOBALS['SQ_SYSTEM']->am->getAsset($thesaurus_id);
			$tag_id		= $thesaurus->getTermIdByName($tag_name);

			if ($cascade) {
				$hh = $GLOBALS['SQ_SYSTEM']->getHipoHerder();
				$tag_vars	= Array();
				$tag_vars[]	= Array (
								'tag_id'	=> $thesaurus_id.':'.$tag_id,
								'action'	=> 'add',
								'weight'	=> $weight,
							  );
				$vars		= Array (
								'assets'	=> Array (
												$asset->id	=> Array (
																'type_code'	=> $asset->type(),
															   ),
											   ),
								'tag_changes'	=> $tag_vars,
							  );
				$errors	= $hh->freestyleHipo('hipo_job_edit_tags', $vars);
			} else {
				$errors	= $tag_manager->setTag($assetid, $thesaurus_id.':'.$tag_id, $weight);
				if ($errors) $errors = NULL;
			}//end else

			if (empty($errors)) {
				return Array (
						'SetTagResult'	=> TRUE,
					   );
			} else {
				throw new SoapFault('Server', 'Unable to tag asset');
			}//end else
		} else {
			throw new SoapFault('Server', 'Asset ID is not valid. Please provide a valid Asset ID');
		}//end else

	}//end SetTagByName()


	/**
	* Description: This operation will return all statuses of the asset in Matrix
	*
    * @param string  $request  The request information
    *
	* @return void
	* @access public
	*/
	function GetAllStatuses($request)
	{
		// We are not asking for any input, we are just going to return all the status in the system
		require_once SQ_INCLUDE_PATH.'/general_occasional.inc';
		$status_names 		= get_status_names();
		$return_statuses	= Array();
		$i	= 0;
		foreach ($status_names as $status_val => $description) {
			$return_statuses[$i]['StatusValue']			= $status_val;
			$return_statuses[$i]['StatusDescription']		= $description;
			$i++;
		}//end foreach
		return Array (
				'GetAllStatusesResult'	=> $return_statuses,
			   );

	}//end GetAllStatus()


	/**
	* getComplexElements
	*
	* @return array
	* @access public
	*/
	public static function getComplexElements($func_list=Array())
	{
		$asset_type							= self::getArgType('AssetType', 0, 1);
		$attribute_detail					= self::getArgType('AttributeDetail', 0, 'unbounded', TRUE);
		$attribute_properties				= self::getArgType('AttributeProperty', 0, 'unbounded', TRUE);
		$attribute_values_by_name_result	= self::getArgType('AttributeValueInfo', 0, 'unbounded', TRUE);
		$keywords							= self::$string_optional_unbounded;
		$status_optional					= self::getArgType('Status', 0, 'unbounded', TRUE);
		$statuses_des_val					= self::getArgType('StatusDescription', 0, 'unbounded', TRUE);
		$urls								= self::$string_optional_unbounded;
		$link_type							= self::getArgType('LinkType', 0, 1);

		$complex_types	= Array (
							'CreateAsset'				=> Array (
															'CreateAsset'			=> Array (
											    										'TypeCode'		=> $asset_type,
																						'Name'			=> self::$string_non_optional,
																						'ParentID'		=> self::$string_non_optional,
																						'LinkType'		=> $link_type,
																						'LinkValue'		=> self::$string_optional,
																						'SortOrder'		=> self::$int_optional,
																						'IsDependant'	=> self::$string_optional,
																						'IsExclusive'	=> self::$string_optional,
																					  ),
															'CreateAssetResponse'	=> Array (
																						'NewAssetID'		=> self::$string_optional,
																					  	'CreateMessage'		=> self::$string_optional,
																					   ),
														   ),
							'GetAsset'					=> Array (
															'GetAsset'				=> Array (
																						'AssetID'		=> self::$string_non_optional,
																					   ),
															'GetAssetResponse'		=> Array (
																						'GetAssetResult'	=> self::$string_optional,
																					   ),
														   ),
							'GetURLs'					=> Array (
															'GetURLs'				=> Array (
																						'AssetID'		=> self::$string_non_optional,
																					   ),
															'GetURLsResponse'		=> Array (
																						'GetURLsResult'			=> $urls,
																					   ),
														   ),
							'GetAssetFromURL'			=> Array (
														   	'GetAssetFromURL'		=> Array (
																						'URL'			=> self::$string_non_optional,
														   							   ),
														   	'GetAssetFromURLResponse'=> Array (
																						'GetAssetFromURLResult'			=> self::$string_optional,
														   							   ),
														   ),
							'GetAssetAvailableStatuses'	=> Array (
														   	'GetAssetAvailableStatuses'	=> Array (
																						'AssetID'			=> self::$string_non_optional,
														   							   ),
														   	'GetAssetAvailableStatusesResponse'=> Array (
																						'GetAssetAvailableStatusesResult'	=> $status_optional,
														   							   ),
														   ),
							'SetAttributeValue'			=> Array (
														   	'SetAttributeValue'	=> Array (
																						'AssetID'			=> self::$string_non_optional,
														   								'AttributeName'		=> self::$string_non_optional,
														   								'AttributeValue'	=> self::$string_non_optional,
														   							   ),
														   	'SetAttributeValueResponse'=> Array (
																						'SetAttributeValueResult'	=> $status_optional,
														   							   ),
														   ),
							'TrashAsset'				=> Array (
														   	'TrashAsset'			=> Array (
																						'AssetID'			=> self::$string_non_optional,
														   							   ),
														   	'TrashAssetResponse'	=> Array (
																						'TrashAssetResult'	=> self::$boolean_optional,
														   							   ),
														   ),
							'CloneAsset'				=> Array (
														   	'CloneAsset'			=> Array (
																						'AssetID'			=> self::$string_non_optional,
														   								'NewParentID'		=> self::$string_non_optional,
														   								'NumberOfClone'		=> self::$int_optional,
														   								'PositionUnderNewParent'=> self::$int_optional,
														   								'LinkType'			=> $link_type,
														   							   ),
														   	'CloneAssetResponse'	=> Array (
																						'CloneAssetResult'	=> self::$string_optional,
														   							   ),
														   ),
							'GetAssetTypeAttribute'		=> Array (
															'GetAssetTypeAttribute'	=> Array (
														   								'TypeCode'			=> $asset_type,
														   								'AttributeDetail'	=> $attribute_detail,
														   							   ),
														   	'GetAssetTypeAttributeResponse'=> Array (
																						'GetAssetTypeAttributeResult'		=> $attribute_properties,
														   							   ),
														   ),
							'GetAvailableKeywords'		=> Array (
															'GetAvailableKeywords'	=> Array (
														   								'AssetID'			=> self::$string_non_optional,
														   							   ),
														   	'GetAvailableKeywordsResponse'=> Array (
																						'GetAvailableKeywordsResult'	=> $keywords,
														   							   ),
														   ),
							'GetAttributeValuesByName'	=> Array (
															'GetAttributeValuesByName'	=> Array (
														   								'AssetIDs'			=> self::$string_optional_unbounded,
														   								'TypeCode'			=> $asset_type,
														   								'AttributeName'		=> self::$string_non_optional,
														   							   ),
														   	'GetAttributeValuesByNameResponse'=> Array (
																						'GetAttributeValuesByNameResult'	=> $attribute_values_by_name_result,
														   							   ),
														   ),

							'GetAssetWebPaths'			=> Array (
															'GetAssetWebPaths'	=> Array (
														   								'AssetID'			=> self::$string_non_optional,
														   							   ),
														   	'GetAssetWebPathsResponse'=> Array (
																						'GetAssetWebPathsResult'	=> self::$string_optional_unbounded,
														   							   ),
														   ),
							'SetTag'					=> Array (
													   		'SetTag'				=> Array (
																						'AssetID'		=> self::$string_non_optional,
													   									'ThesaurusID'	=> self::$string_non_optional,
													   									'TagName'		=> self::$string_non_optional,
													   									'Weight'		=> self::$string_optional,
													   									'CascadeTagChange'=> self::$string_optional,
													   								   ),
													   		'SetTagResponse'		=> Array (
																						'SetTagResult'	=> self::$boolean_optional,
													   								   ),
													   	   ),
							'GetAllStatuses'			=> Array (
															'GetAllStatuses'			=> Array (

													   	   							   ),
													   		'GetAllStatusesResponse'	=> Array (
																						'GetAllStatusesResult'	=> $statuses_des_val,
													   	   							   ),
													   	   ),
						  );

		$complex_types_available = parent::getComplexElements($complex_types);

		return $complex_types_available;


	}//end getComplexTypes();


    /**
    * getComplexTypes: return all the complex types used by the API
    *
    * @return array
    * @access public
    */
    public static function getComplexTypes($func_list=Array())
    {

        $complex_types    = Array (
							'GetAllStatuses'			=> Array (
															'StatusDescription'			=> Array (
													   	   							  	 	'StatusValue'		=> self::$string_optional,
													   	   								   	'StatusDescription'	=> self::$string_optional,
													   	   								   ),
													   	   ),
							'GetAttributeValuesByName'	=> Array (
														   	'AttributeValueInfo'		=> Array (
															   								'AssetID'			=> self::$string_non_optional,
															   								'AttributeValue'	=> self::$string_optional,
															   							   ),
													  	   ),
							'GetAssetTypeAttribute'		=> Array (
														   	'AttributeProperty'		=> Array (
															   								'AttributeName'					=> self::$string_non_optional,
															   								'AttributeID'					=> self::$string_optional,
															   								'AttributeType'					=> self::$string_optional,
															   								'AttributeOwningTypeCode'		=> self::$string_optional,
															   								'AttributeParametersTypeCode'	=> self::$string_optional,
															   								'AttributeParametersValue'		=> self::$string_optional,
															   								'AttributeDefaultTypeCode'		=> self::$string_optional,
															   								'AttributeDefaultValue'			=> self::$string_optional,
															   								'AttributeDescription'			=> self::$string_optional,
															   								'AttributeIsAdmin'				=> self::$boolean_optional,
															   								'AttributeUnique'				=> self::$boolean_optional,
															   							   ),
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
		$attribute_details	= Array (
								'attrid',
								'type_code',
								'owning_type_code',
								'name',
								'type',
								'uniq',
								'parameters_type_code',
								'parameters_val',
								'default_type_code',
								'default_val',
								'description',
								'is_admin',
							  );

		$common_types				= Array (
										'LinkType'	=> Array (
														'restriction_base'	=> 'string',
														'enumeration'		=> Array (
															   					'1', '2', '4', '8'
															   				   ),
													   ),
									  );


		$simple_restricted_types	= Array (
										'CreateAsset'			=> Array (
																	'AssetType'			=> Array (
																							'restriction_base'	=> 'string',
																							'enumeration'		=> $asset_types,
																						   ),
																	'CreditCardNumber'	=> Array (
																							'restriction_base'	=> 'string',
																							'pattern'			=> '\d{3}-\d{2}-\d{4}',
																						   ),
																	'LinkType'			=> $common_types['LinkType'],
																   ),
										'CreateLink'			=> Array (
																	'LinkType'		=> $common_types['LinkType'],
																   ),
										'GetAssetTypeAttribute'	=> Array (
																	'AttributeDetail'	=> Array (
																							'restriction_base'	=> 'string',
																							'enumeration'		=> $attribute_details,
																						   ),
																   ),
									  );

		$simple_restricted_types_available = parent::processSimpleRestrictedTypes($simple_restricted_types);

		return $simple_restricted_types_available;

	}//end getSimpleRestrictedTypes()


}//end class
?>
