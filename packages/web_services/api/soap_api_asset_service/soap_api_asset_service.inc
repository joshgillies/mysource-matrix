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
* $Id: soap_api_asset_service.inc,v 1.34 2013/02/25 04:04:45 akarelia Exp $
*
*/


require_once SQ_PACKAGES_PATH.'/web_services/api/soap_api/soap_api.inc';
require_once SQ_PACKAGES_PATH.'/web_services/api/soap_api_file_retrieval_service/soap_api_file_retrieval_service.inc';
require_once SQ_LIB_PATH.'/html_form/html_form.inc';

/**
* Soap_Api_Asset_Service
*
* Purpose
*
* @author  Edison Wang <ewang@squiz.com.au>
* @author  Huan Nguyen <hnguyen@squiz.net>
* @version $Revision: 1.34 $
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
				'CreateAsset'						=> '1',
				'CloneAsset'						=> '1',
				'GetAllStatuses'					=> '1',
				'GetAsset'							=> '1',
				'GetAssetFromURL'					=> '1',
				'GetAssetTypeAttribute'				=> '1',
				'GetAssetTypeDescendants'			=> '1',
				'GetAssetsInfo'						=> '1',
				'GetAttributeValuesByName'			=> '1',
				'GetAssetAvailableKeywords'			=> '1',
				'GetPaintLayoutAvailableKeywords'	=> '1',
				'GetAssetAvailableStatuses'			=> '1',
				'GetAssetWebPaths'					=> '1',
				'GetAssetURLs'						=> '1',
				'GetPageContents'					=> '1',
				'GetTags'							=> '1',
				'SetAttributeValue'					=> '1',
				'SetTag'							=> '1',
				'SetAssetStatus'					=> '1',
	   			'SetAssetWebPaths'					=> '1',
				'TrashAsset'						=> '1',
				'LoginUser'							=> '1',
				'GetUserIdByUsername'				=> '1',
			   );

    }//end getFunctionList()


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
	function CreateAsset($request)
	{
		$request_info 	= (Array) $request;
		$parentid		= array_get_index($request_info, 'ParentID', '');
		$type_code		= array_get_index($request_info, 'TypeCode', '');
		$type_code		= strtolower($type_code);
		$asset_name		= array_get_index($request_info, 'Name', '');
		$link_type		= array_get_index($request_info, 'LinkType', '1');
		$link_value		= array_get_index($request_info, 'LinkValue', '');
		$sort_order		= array_get_index($request_info, 'SortOrder', -1);
		$is_dependant	= array_get_index($request_info, 'IsDependant', '0');
		$is_exclusive	= array_get_index($request_info, 'IsExclusive', '0');
		$file_content	= array_get_index($request_info, 'FileContentBase64', '');
		$file_name		= array_get_index($request_info, 'FileName', '');
		$attributes		= array_get_index($request_info, 'AttributeInfo', Array());

		$parent_asset = $GLOBALS['SQ_SYSTEM']->am->getAsset($parentid);

		$GLOBALS['SQ_SYSTEM']->am->includeAsset($type_code);
		$object_name = str_replace(' ', '_', ucwords(str_replace('_', ' ', $type_code)));
		$asset = new $object_name();

		if(isset($attributes->AttributeName)) {
			// Set single attribute value
			$asset->setAttrValue($attributes->AttributeName, $attributes->AttributeValue);
		}
		else {
			// Set attribute values for the new asset
			foreach ($attributes as $attribute_info) {
				$attribute_info = (Array) $attribute_info;
				$asset->setAttrValue($attribute_info['AttributeName'], $attribute_info['AttributeValue']);
			}
		}

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


		// Create a file asset
		$type_ancestors = $asset->getTypeAncestors(FALSE);
		if(!empty($file_name) && !empty($file_content) && (in_array('file', $type_ancestors) || $type_code === 'file')) {
			$tmp_file_name	= Soap_Api_File_Retrieval_Service::getRandomFilename($file_name);
			// We are going to write to the Matrix Data tmp dir
			$destination_file	= SQ_DATA_PATH.'/temp/'.$tmp_file_name;
			while (file_exists($destination_file)) {
				sleep(1);
				$tmp_file_name	= Soap_Api_File_Retrieval_Service::getRandomFilename($file_name);
			}//end if
			file_put_contents($destination_file, base64_decode($file_content));
			require_once SQ_FUDGE_PATH.'/general/file_system.inc';
			$file_type		= get_file_type($file_name);
			$info	= Array (
						'name'		=> $file_name,
						'type'		=> $file_type,
						'tmp_name'			=> $destination_file,
						'non_uploaded_file'	=> TRUE,
						'error'		=> 0,
						'size'		=> filesize($destination_file),
					  );
			$asset->_tmp['uploading_file'] = 1;
			$asset->setAttrValue('title', $asset_name);
			$linkid = $asset->create($link_info, $info);

			// Remove the tmp file before we return
			if (file_exists($destination_file)) {
				unlink($destination_file);
			}//end if

		}
		// Create a normal asset
		else {
			// Not all assets have 'name' attribute, e.g. user asset
			if (isset($asset->vars['name']) && ($asset_name != '')) {
				$asset->setAttrValue('name', $asset_name);
			}
			$linkid = $asset->create($link_info);
		}


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
				foreach ($asset->vars as $attr_name => $attr_info) {
					if ($attr_info['type'] == 'password') {
						unset($asset->vars[$attr_name]);
					}
				}
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
		// apply xml element naming rules
		$key = preg_replace('/\s+/', '', $key);
		if (empty($key)) continue;
		if(preg_match('/^xml$/i', $key)) continue;
		if (is_numeric($key)) {
		    $key = '_'.$key;
		}

	    	if (empty($value)) {
	    		$object_xml .=  '<'.$key.' />';
	    	} else {
		        $object_xml .=  '<'.$key.'>';
		        if (is_array($value) || is_object($value)) {
		            $object_xml .= self::objectToXML($value);
		        } else {
		            $object_xml .=  '<![CDATA['.$value.']]>';
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
	function GetAssetURLs($request)
	{
		$request_info 	= (Array) $request;
		$assetid		= array_get_index($request_info, 'AssetID', '');
        $root_path_id	= array_get_index($request_info, 'RootPathAssetID', '');

		if (!empty($assetid)) {
			if (!empty($root_path_id)) {
				$root_path_url = $GLOBALS['SQ_SYSTEM']->am->getAssetURL($root_path_id);
				$assetUrls     = $GLOBALS['SQ_SYSTEM']->am->getAssetURL($assetid, $root_path_url);
				return Array (
						'GetAssetURLsResult'	=> Array($assetUrls),
					   );
			} else {
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
						'GetAssetURLsResult'	=> $urls,
					   );
			}//end else
		} else {
			throw new SoapFault('Server', 'Asset ID is not valid. Please provide a valid Asset ID');
		}//end else

	}//end GetAssetURLs()


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
			$pattern    = '#(http://|https://|^)#';
			$url_split	= preg_split($pattern, $url, -1, PREG_SPLIT_DELIM_CAPTURE);
			$protocol	= rtrim($url_split[1], '://');
			$main_url	= $url_split[2];

			$asset	= $GLOBALS['SQ_SYSTEM']->am->getAssetFromURL($protocol, $main_url, TRUE, TRUE);
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

	}//end GetAssetFromURL()


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
			$return_statuses	= Array();
			$i  = 0;
			foreach ($statuses as $status_val => $description) {
				$return_statuses[$i]['StatusValue']			= $status_val;
				$return_statuses[$i]['StatusDescription']	= $description;
				$i++;
			}//end foreach

			return Array (
					'GetAssetAvailableStatusesResult'	=> $return_statuses,
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
				if ($result) {
					$del_asset = $GLOBALS['SQ_SYSTEM']->am->getAsset($assetid);
					$del_asset->updateLookups();
					$GLOBALS['SQ_SYSTEM']->am->forgetAsset($del_asset);
				}
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

		if (!is_array($details)) $details = Array($details);
		//need to group by 'name' field so make sure its first.
		if (!in_array('name', $details)) {
			array_unshift($details, 'name');
		} else {
			if (count($details) == 1){
				$details = Array ('name', 'type');//need atleast two so we can still group by 'name'.
			} else if ($details[0] != 'name'){
				$details = array_diff($details, Array('name'));
				array_unshift($details, 'name');
			}
		}

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
	* Description: This operation will return all the type descendants of an asset type.
	*
     * @return void
     */
    function GetAssetTypeDescendants($request)
    {
        $request_info   = (Array) $request;
        $typecode       = array_get_index($request_info, 'TypeCode', '');
        if (!empty($typecode)) {
            $descendantTypes = $GLOBALS['SQ_SYSTEM']->am->getTypeDescendants($typecode);

            return Array (
                    'GetAssetTypeDescendantsResult' => $descendantTypes,
                   );
        } else {
            throw new SoapFault('Server', 'TypeCode is not valid. Please provide a valid TypeCode');
        }//end else

    }//end GetAssetTypeDescendants()


    /**
     * This function return the asset information (name, typecode, children assets, lineage) about the requested asset.
     *
     * @param $request
     * @return void
     */
    function GetAssetsInfo($request)
    {
        $request_info = (Array) $request;
        $assetids     = array_get_index($request_info, 'AssetIDs', '');
        $attr_names   = array_get_index($request_info, 'FinderAttributes', '');
        $root_assetid = array_get_index($request_info, 'RootNode', '');

        if (!is_array($attr_names)) {
            $attr_names = Array($attr_names);
        }//end if
		if (!is_array($assetids)) {
			$assetids = Array($assetids);
		}//end if

        if (!empty($assetids) && !empty($attr_names)) {
            $assetsInfo = Array();
            if (in_array('children', $attr_names) === TRUE) {
                foreach ($assetids as $assetid) {
                	$links = $GLOBALS['SQ_SYSTEM']->am->getLinks($assetid, SQ_SC_LINK_BACKEND_NAV, '', TRUE, 'major', NULL, NULL, NULL, 'sort_order');
                	$assetsInfo[$assetid]['AssetChildren'] = Array();
                	foreach ($links as $link_info) {
                		$assetsInfo[$assetid]['AssetChildren'][] = $link_info['minorid'];
                	}//end foreach
                }//end foreach
            }//end if

            $type_codes_ancestor = array();

			$info_results = $GLOBALS['SQ_SYSTEM']->am->getAssetInfo($assetids);
			foreach ($info_results as $assetid => $result) {
				if (in_array('lineage', $attr_names) === TRUE) {
					$treeids = NULL;
					if (!empty($root_assetid)) {
						$root_links = $GLOBALS['SQ_SYSTEM']->am->getLinks($root_assetid, SQ_LINK_TYPE_1, '', TRUE, 'minor');
						foreach($root_links as $link) {
							$treeid_info = $GLOBALS['SQ_SYSTEM']->am->getLinkTreeid($link['linkid']);
							$treeid      = $treeid_info[$link['linkid']][0];
							$treeids[]   = $treeid;
						}//end if
					}//end if

					$lineages        = $GLOBALS['SQ_SYSTEM']->am->getLinkLineages($assetid, 0, $treeids);
					$lineages_result = array();
					foreach ($lineages as $lineage_info) {
						$lineage_details   = array();
						$lineage_details['LinkID']   = $lineage_info['linkid'];
						$lineage_details['LinkType'] = $lineage_info['link_type'];
						$order = 0;
						foreach ($lineage_info['lineage'] as $parentid => $parent_name) {
							$one_lineage = array();
							$one_lineage['AssetID']       = $parentid;
							$one_lineage['AssetName']     = $parent_name;
							$one_lineage['Order']         = $order;
							$lineage_details['Lineage'][] = $one_lineage;
							$order++;
						}//end foreach
						//$lineage_details['Lineage']  = $lineage['lineage'];
						$lineages_result[] = $lineage_details;
					}//end foreach
					$assetsInfo[$assetid]['AssetLineage'] = $lineages_result;
				}//end if

				if (isset($type_codes_ancestor[$result['type_code']]) === TRUE) {
					$ancestor = $type_codes_ancestor[$result['type_code']];
				} else {
					$ancestor = array_pop($GLOBALS['SQ_SYSTEM']->am->getTypeAncestors($result['type_code'], FALSE));
					$type_codes_ancestor[$result['type_code']] = $ancestor;
				}//end if

				$assetsInfo[$assetid]['AssetType']         = $result['type_code'];
				$assetsInfo[$assetid]['AssetTypeAncestor'] = $ancestor;

				$assetsInfo[$assetid]['AssetName'] = $result['name'];
				if ($ancestor !== 'file') {
					if (isset($result['short_name']) === TRUE) {
						$assetsInfo[$assetid]['AssetName'] = $result['short_name'];
					}//end if
				}//end if

				$assetsInfo[$assetid]['AssetID'] = $assetid;
			}//end foreach

            return Array (
                       'GetAssetsInfoResult'  => array_values($assetsInfo),
                   );
        } else {
            throw new SoapFault('Server', 'Please provide assetids and finder attributes.');
        }//end else

    }//end GetAssetsInfo()


    /**
    * Description: This operation will return all available keywords of an asset
    *
    * @param string  $request  The request information
    *
    * <pre>
    * Array (
	*		'AssetID' => [The ID of the asset in query],
    *			'TypeCode' => [The type code of asset],
    *       )
    * </pre>
	*
	* @return void
	* @access public
	*/
	function GetAssetAvailableKeywords($request)
	{
		$request_info 	= (Array) $request;
		$assetid			= array_get_index($request_info, 'AssetID', '');
		$type_code			= array_get_index($request_info, 'TypeCode', '');

		if (!empty($assetid) || !empty($type_code)) {
			if(!empty($assetid)) {
			    $asset	= $GLOBALS['SQ_SYSTEM']->am->getAsset($assetid);
			    $keywords	= $asset->getAvailableKeywords();
			}
			else if (!empty($type_code)) {
			    $object_name = str_replace(' ', '_', ucwords(str_replace('_', ' ', $type_code)));
			    $GLOBALS['SQ_SYSTEM']->am->includeAsset($object_name);
			    $dummy_asset = new $object_name();
			    $keywords = $dummy_asset->getAvailableKeywords();
			    unset($dummy_asset);
			}

			return Array (
					'GetAssetAvailableKeywordsResult'	=> array_keys($keywords),
				   );
		} else {
			throw new SoapFault('Server', 'Asset ID or Type Code is not valid. Please provide a valid Asset ID or Type Code');
		}//end else

	}//end GetAssetAvailableKeywords()


	/**
	* Description: This operation will return all available keywords of an asset
	*
	* @param string  $request  The request information
	*
	* <pre>
	* Array (
	*		'AssetID' => [The ID of the asset in query],
	*			'TypeCode' => [The type code of asset],
	*       )
	* </pre>
	*
	* @return void
	* @access public
	*/
	function GetPaintLayoutAvailableKeywords($request)
	{
		$request_info 	= (Array) $request;
		$assetid		= array_get_index($request_info, 'AssetID', '');
		$type_code		= array_get_index($request_info, 'TypeCode', '');
		$keywords		= Array();

		if (!empty($assetid)) {
			$asset = $GLOBALS['SQ_SYSTEM']->am->getAsset($assetid);
			$type_format_folder = $asset->getFolder();

			$format_to_get = !empty($type_code) ? $type_code : 'default_format';
			$format_link = $GLOBALS['SQ_SYSTEM']->am->getLink($type_format_folder->id, SQ_LINK_TYPE_2, 'paint_layout_bodycopy', TRUE, $format_to_get);

			if (!empty($format_link)) {
				$bodycopy = $GLOBALS['SQ_SYSTEM']->am->getAsset($format_link['minorid'], $format_link['minor_type_code']);
			}

			if ($bodycopy) $bodycopy->getKeywordsDescription($keywords, $type_code);

			return Array (
					'GetPaintLayoutAvailableKeywordsResult'	=> array_keys($keywords),
				   );
		} else {
			throw new SoapFault('Server', 'Please provide a valid Asset ID');
		}//end else

	}//end GetAssetAvailableKeywords()


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
     * This function return the page contents for
     *
     * @return array
     * @access public
     */
	function GetPageContents($request)
	{
		$request_info = (Array) $request;
		$assetid      = array_get_index($request_info, 'AssetID', '');
		$root_path_id = array_get_index($request_info, 'RootNodeID', '');
		if (!empty($assetid)) {
			$asset = $GLOBALS['SQ_SYSTEM']->am->getAsset($assetid);
			$descendantTypes = $GLOBALS['SQ_SYSTEM']->am->getTypeAncestors($asset->type());
			if (in_array('page', $descendantTypes)) {
				$root_path_url = $GLOBALS['SQ_SYSTEM']->am->getAssetURL($root_path_id);
				$pro_asset_url = $GLOBALS['SQ_SYSTEM']->am->getAssetURL($assetid, $root_path_url);
				$asset_url     = preg_replace('/http[s]?:\/\//i', '', $pro_asset_url);
				$web_paths	   = $asset->getWebPaths();
				$final_path	   = array_pop($web_paths);
				foreach ($web_paths as $path) {
					$pattern  = '/\/'.$path.'$/';
        			preg_match($pattern, $url, $matches);
					if (!empty($matches)) {
						$final_path = $path;
						break;
					}//end if
				}//end foreach

				$layout_id = $GLOBALS['SQ_SYSTEM']->am->getValueFromURL($asset_url, 'paint_layout::system::frontend');
				ob_start();
				$asset->printBodyWithPaintLayout($layout_id);
				$content = ob_get_contents();
				ob_end_clean();

				// The remaining keywords, just clean them up because they didnt get replaced.
				$GLOBALS['SQ_SYSTEM']->replaceKeyword($content);
				return Array (
						'PageContent'   => $content,
						'PageURL'		=> $pro_asset_url,
						'PageWebPath'	=> $final_path,
						);
			} else {
                throw new SoapFault('Server', 'This asset is not of type page');
            }//end if

        } else {
            throw new SoapFault('Server', 'Asset ID is not valid. Please provide a valid Asset ID');
        }//end if

    }//end GetPageContents()


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
			if (is_string($assetids) === TRUE) $assetids = array($assetids);
			$attribute_values	= $GLOBALS['SQ_SYSTEM']->am->getAttributeValuesByName($attr_name, $type_code, $assetids);
			$final_attribute_values	= Array();
			$i = 0;
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
	* Description: This operation will return all tags links given an asset id.
	*
	* @param string  $request  The request information
	*
	* <pre>
	* Array (
	    *		'AssetID'	=> [The Asset to find tags links under],
	    *		'ThesaurusIDs'		=> [The thesauruses we use to filter the tags],
	*        )
	* </pre>
	*
	* @return array
	* @access public
	*/
	function GetTags($request)
	{

		$request_info 	= (Array) $request;
		$assetid			= array_get_index($request_info, 'AssetID', '');
		$thesaurus_ids		= array_get_index($request_info, 'ThesaurusIDs', Array());
		if (!is_array($thesaurus_ids)) {
		    $thesaurus_ids = Array($thesaurus_ids);
		}
		if (!empty($assetid)) {
		    $tags = Array();
		    $i = 0;
		    $tag_manager = $GLOBALS['SQ_SYSTEM']->getTagManager();
		    $am = $GLOBALS['SQ_SYSTEM']->am;
		    $tag_links = $tag_manager->getTagLinks($assetid, $thesaurus_ids);
		   foreach ($tag_links as $data) {
		       $term = $am->getAsset($data['minorid']);
		       $id = explode(':', $data['minorid']);
		       $tags[$i]['TagID'] = $data['minorid'];
		       $tags[$i]['TagName'] = $term->name;
		       $tags[$i]['ThesaurusID'] = $id[0];
		       $tags[$i]['Weight'] = $data['value'];
		       $tags[$i]['LinkID'] = $data['linkid'];
		       $i++;
		   }
		    return Array (
					'GetTagsResult'	=> $tags,
				   );
		} else {
			throw new SoapFault('Server', 'Asset ID is not valid. Please provide a valid Asset ID');
		}//end else

	}//end GetTags()


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
	* Description: This operation will return all statuses of the asset in Matrix
	*
	* @param string  $request  The request information
    *
	* @return void
	* @access public
	*/
	function LoginUser($request)
	{
		$request_info = (Array) $request;
		$username = array_get_index($request_info, 'Username', '');
		$password = array_get_index($request_info, 'Password', '');

		$auth_folder = $GLOBALS['SQ_SYSTEM']->am->getSystemAsset('authentication_folder');
		if (is_null($auth_folder)) {
			trigger_localised_error('SYS0249', E_USER_ERROR);
			return;
		}
		$auth_systems = $auth_folder->getAuthSystems();
		// try each auth system in order to see if we can load a user asset
		$user = NULL;
		$user_status = NULL;
		foreach ($auth_systems as $systemid) {
			$system = $GLOBALS['SQ_SYSTEM']->am->getAsset($systemid);
			if (is_null($system)) continue;
			$user = $system->authenticateUser($username, $password);
			if (!is_null($user)) $user_status = $user->status;
			// check that the user exists AND that it can login
			// (ie. it is not yet live, or has been locked)
			if (!is_null($user) && $user->canLogin()) {
				$GLOBALS['SQ_SYSTEM']->loginUser($user);
				break;
			}//end if
		}


		if ($user === NULL) {
			// We couldn't find a user to login as.
			return Array (
					'SessionID'  => '',
					'SessionKey' => '',
				);
		}


		$session_id = session_id();
		$session_key = $GLOBALS['SQ_SYSTEM']->getUniqueSOAPSessionKey();
		$_SESSION['activated'] = 0;

		return Array (
				'SessionID'	 => $session_id,
				'SessionKey' => $session_key,
			   );

	}//end LoginUser()


	/**
	* Description: This operation will return the user asset ID of the request username
	*
	* @param string  $request  The request information
	*
	* @return void
	* @access public
	*/
	function GetUserIdByUsername($request)
	{
		$request_info = (Array) $request;
		$username = array_get_index($request_info, 'Username', '');

		$auth_folder = $GLOBALS['SQ_SYSTEM']->am->getSystemAsset('authentication_folder');
		if (is_null($auth_folder)) {
			trigger_localised_error('SYS0249', E_USER_ERROR);
			return;
		}
		$auth_systems = $auth_folder->getAuthSystems();

		// try each auth system in order to see if we can load a user asset
		$user = NULL;
		foreach ($auth_systems as $systemid) {
			$system = $GLOBALS['SQ_SYSTEM']->am->getAsset($systemid);
			if (is_null($system)) continue;

			$user = $system->locateUser($username);
			if (!is_null($user)) break;
		}


		if (is_null($user)) {
			$assetid = '';
		} else {
			$assetid = $user->id;
		}

		return Array('AssetID' => $assetid);

	}//end GetUserIdByUsername()


	/**
	* Description: This operation will set status for an asset
	*
	* @param string  $request  The request information
	*
    * <pre>
    * Array (
	*		'AssetID'	 		=> [The ID of the asset in query],
	*		'StatusValue'		=> [The asset status to be set],
	*		'DependantsOnly'	=> [Cascade the change to children or just dependants?],
    *        )
    * </pre>
	*
	* @return void
	* @access public
	*/
	function SetAssetStatus($request)
	{
		$request_info = (Array) $request;
		$assetid = array_get_index($request_info, 'AssetID', '');
		$status = array_get_index($request_info, 'StatusValue', '');
		$dependants_only = array_get_index($request_info, 'DependantsOnly', TRUE);

		if (!empty($assetid)) {
			$asset = $GLOBALS['SQ_SYSTEM']->am->getAsset($assetid);
			$statuses = $asset->getAvailableStatii();

			if(!isset($statuses[$status])) {
				throw new SoapFault('Server', $status.' is not a valid status of the asset');
			}

			$running_vars	= Array (
								'assetid'			=> $assetid,
								'new_status'		=> $status,
								'dependants_only'	=> $dependants_only,
							  );

			$hh		= $GLOBALS['SQ_SYSTEM']->getHipoHerder();
			$errors	= $hh->freestyleHipo('hipo_job_edit_status', $running_vars);
			if (empty($errors)) {
				return Array (
						'SetAssetStatusResult'	=> TRUE,
					   );
			} else {
				throw new SoapFault('Server', 'Unable to set asset status');
			}//end else
		} else {
			throw new SoapFault('Server', 'Asset ID is not valid. Please provide a valid Asset ID');
		}//end else

	}//end SetAssetStatus()


	/**
	* Description: This operation will set multiple paths for an asset
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
	function SetAssetWebPaths($request)
	{
		$request_info 	= (Array) $request;
		$assetid			= array_get_index($request_info, 'AssetID', '');
		if (!empty($assetid)) {
			$path			= array_get_index($request_info, 'Path', Array());
			if(!is_array($path)) $path = Array($path);
			$addRemaps			= array_get_index($request_info, 'AddRemaps', TRUE);

			$asset		= $GLOBALS['SQ_SYSTEM']->am->getAsset($assetid);
			$result	= $asset->saveWebPaths($path, $addRemaps);
			return Array (
					'SetAssetWebPathsResult'	=> $result,
				   );
		} else {
			throw new SoapFault('Server', 'Asset ID is not valid. Please provide a valid Asset ID');
		}//end else

	}//end SetAssetWebPaths()


	/**
	* getComplexElements
	*
	* @return array
	* @access public
	*/
	public static function getComplexElements($func_list=Array())
	{
		$asset_type							= self::getArgType('AssetType', 1, 1);
		$asset_types                        = self::getArgType('AssetType', 0, 'unbounded');
		$attribute_info						= self::getArgType('AttributeInfo', 0, 'unbounded', TRUE);
		$attribute_detail					= self::getArgType('AttributeDetail', 0, 'unbounded', TRUE);
		$attribute_properties				= self::getArgType('AttributeProperty', 0, 'unbounded', TRUE);
		$attribute_values_by_name_result	= self::getArgType('AttributeValueInfo', 0, 'unbounded', TRUE);
		$keywords							= self::$string_optional_unbounded;
		$statuses_des_val					= self::getArgType('StatusDescription', 0, 'unbounded', TRUE);
		$asset_finder_info                  = self::getArgType('AssetFinderInfo', 0, 'unbounded', TRUE);
		$urls								= self::$string_optional_unbounded;
		$link_type							= self::getArgType('LinkType', 0, 1);
		$asset_info                         = self::getArgType('AssetInfo', 0, 1);
		$thesaurus_ids	= self::getArgType('string', 0, 'unbounded', TRUE);
		$tag_details			= self::getArgType('TagDetails', 0, 'unbounded', TRUE);

		$complex_types	= Array (
							'CreateAsset'				=> Array (
															'CreateAsset'			=> Array (
											    										'TypeCode'			=> $asset_type,
																						'Name'				=> self::$string_optional,
																						'ParentID'			=> self::$string_non_optional,
																						'LinkType'			=> $link_type,
																						'LinkValue'			=> self::$string_optional,
																						'SortOrder'			=> self::$int_optional,
																						'IsDependant'		=> self::$string_optional,
																						'IsExclusive'		=> self::$string_optional,
																						'FileName'			=> self::$string_optional,
																						'FileContentBase64'	=> self::$string_optional,
																						'AttributeInfo'		=> $attribute_info,
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
							'GetAssetURLs'				=> Array (
															'GetAssetURLs'				=> Array (
																						'AssetID'		=> self::$string_non_optional,
																						'RootPathAssetID'	=> self::$string_optional,
																					   ),
															'GetAssetURLsResponse'		=> Array (
																						'GetAssetURLsResult'=> $urls,
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
																						'GetAssetAvailableStatusesResult'	=> $statuses_des_val,
														   							   ),
														   ),
							'SetAttributeValue'			=> Array (
														   	'SetAttributeValue'	=> Array (
																						'AssetID'			=> self::$string_non_optional,
														   								'AttributeName'		=> self::$string_non_optional,
														   								'AttributeValue'	=> self::$string_non_optional,
														   							   ),
														   	'SetAttributeValueResponse'=> Array (
																						'SetAttributeValueResult'	=> self::$string_optional,
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
							'GetAssetTypeDescendants'     => Array (
																'GetAssetTypeDescendants' => Array (
                                                                                           'TypeCode'           => $asset_type,
																							 ),
																'GetAssetTypeDescendantsResponse'=> Array (
                                                                                        'GetAssetTypeDescendantsResult'       => $asset_types,
                                                                                          ),
                                                           ),
                            'GetAssetsInfo'              => Array (
                                                               'GetAssetsInfo' => Array (
                                                                                            'AssetIDs'         => self::$string_optional_unbounded,
                                                                                            'FinderAttributes' => self::$string_optional_unbounded,
																							'RootNode'		   => self::$string_optional,
                                                                                 ),
                                                               'GetAssetsInfoResponse' => Array (
                                                                                              'GetAssetsInfoResult' => $asset_finder_info,
                                                                                          ),
                                                           ),
							'GetAssetAvailableKeywords'	=> Array (
															'GetAssetAvailableKeywords'	=> Array (
														   								'AssetID'			=> self::$string_optional,
																						'TypeCode'			=> self::getArgType('AssetType', 0, 1),
														   							   ),
														   	'GetAssetAvailableKeywordsResponse'=> Array (
																						'GetAssetAvailableKeywordsResult'	=> $keywords,
														   							   ),
														   ),
							'GetPaintLayoutAvailableKeywords'	=> Array (
															'GetPaintLayoutAvailableKeywords'	=> Array (
														   								'AssetID'			=> self::$string_non_optional,
																						'TypeCode'			=> self::getArgType('AssetType', 0, 1),
														   							   ),
														   	'GetPaintLayoutAvailableKeywordsResponse'=> Array (
																						'GetPaintLayoutAvailableKeywordsResult'	=> $keywords,
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
							'GetTags'			=> Array (
															'GetTags'	=> Array (
														   								'AssetID'			=> self::$string_non_optional,
																						'ThesaurusIDs'		=> $thesaurus_ids,
														   							   ),
														   	'GetTagsResponse'=> Array (
																						'GetTagsResult'	=> $tag_details,
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
							'SetAssetStatus'			=> Array (
															'SetAssetStatus'			=> Array (
											    										'AssetID'			=> self::$string_non_optional,
																						'StatusValue'		=> self::$string_non_optional,
													   	   								'DependantsOnly'	=> self::$boolean_optional,
																					  ),
															'SetAssetStatusResponse'	=> Array (
																						'SetAssetStatusResult'	=> self::$boolean_optional,
																					   ),
														   ),
                            'GetPageContents'           => Array (
                                                               'GetPageContents'        => Array (
                                                                                            'AssetID'       => self::$string_non_optional,
																							'RootNodeID'	=> self::$string_optional,
                                                                                           ),
                                                               'GetPageContentsResponse'=> Array (
                                                                                            'PageContent'   => self::$string_non_optional,
                                                                                            'PageURL'       => self::$string_optional,
																							'PageWebPath'	=> self::$string_optional,
                                                                                           ),
                                                              ),
                            'LoginUser'         => Array (
                                                               'LoginUser'        => Array (
																							  'Username'		=> self::$string_non_optional,
																							  'Password'		=> self::$string_non_optional,
																						     ),
                                                               'LoginUserResponse'=> Array (
                                                                                           	  'SessionID'       => self::$string_non_optional,
                                                                                           	  'SessionKey'      => self::$string_non_optional,
                                                                                             ),
                                                              ),
                            'GetUserIdByUsername'		=> Array (
                                                               'GetUserIdByUsername'        => Array (
                                                                                           	  'Username'	=> self::$string_non_optional,
                                                                                             ),
                                                               'GetUserIdByUsernameResponse'=> Array (
                                                                                           	  'AssetID'		=> self::$string_non_optional,
                                                                                             ),
                                                              ),

							'SetAssetWebPaths'			=> Array (
															'SetAssetWebPaths'			=> Array (
																							'AssetID'		=> self::$string_non_optional,
																							'Path'			=> self::$string_optional_unbounded,
																							'AddRemaps'		=> self::$boolean_optional,
																					       ),
															'SetAssetWebPathsResponse'	=> Array (
																							'SetAssetWebPathsResult'	=> self::$boolean_optional,
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
		$lineages_details = self::getArgType('LineagesDetails', 0, 'unbounded');
		$lineage          = self::getArgType('Lineage', 0, 'unbounded');
		$tag_details	= Array (
							'TagDetails'	=> Array (
												'TagID'			=> self::$string_non_optional,
											   	'TagName'			=> self::$string_non_optional,
												'ThesaurusID'		=> self::$string_non_optional,
												'Weight'			=> self::$string_non_optional,
												'LinkID'			=> self::$string_non_optional,
											   ),
						  );
        $complex_types    = Array (
							'CreateAsset'				=> Array (
														   	'AttributeInfo'				=> Array (
															   								'AttributeName'		=> self::$string_non_optional,
															   								'AttributeValue'	=> self::$string_non_optional,
															   							   ),
													  	   ),
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
							'GetAssetsInfo'		=> Array (
											    'AssetFinderInfo'          => Array (
															'AssetID'           => self::$string_non_optional,
															'AssetType'         => self::$string_optional,
															'AssetTypeAncestor' => self::$string_optional,
															'AssetName'         => self::$string_optional,
															'AssetChildren'     => self::$string_optional_unbounded,
															'AssetLineage'      => $lineages_details,
															),
											    'LineagesDetails'   => Array (
														    'LinkID'    => self::$string_non_optional,
														    'LinkType'  => self::$string_non_optional,
														    'Lineage'   => $lineage,
														    ),
											    'Lineage'           => Array (
														    'AssetID'   => self::$string_non_optional,
														    'AssetName' => self::$string_non_optional,
														    'Order'     => self::$int_non_optional,
														    ),
										),
							'GetTags'			=> $tag_details,
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
		$asset_types_info = $GLOBALS['SQ_SYSTEM']->am->getAssetTypes(TRUE, TRUE);
		$asset_types	  = Array();
		foreach ($asset_types_info as $asset_type_info) {
			$asset_types[] = $asset_type_info['type_code'];
		}//end foreach

		$asset_status = Array (
							SQ_STATUS_ARCHIVED,
							SQ_STATUS_UNDER_CONSTRUCTION,
							SQ_STATUS_PENDING_APPROVAL,
							SQ_STATUS_APPROVED,
							SQ_STATUS_LIVE,
							SQ_STATUS_LIVE_APPROVAL,
							SQ_STATUS_EDITING,
							SQ_STATUS_EDITING_APPROVAL,
							SQ_STATUS_EDITING_APPROVED,
						);

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
										'SetAssetStatus'		=> Array (
																   	'StatusValue'		=> Array (
																   								'restriction_base'	=> 'string',
																   								'enumeration'		=> $asset_status,
																   							),
																   ),
									  );

		$simple_restricted_types_available = parent::processSimpleRestrictedTypes($simple_restricted_types);

		return $simple_restricted_types_available;

	}//end getSimpleRestrictedTypes()


}//end class
?>
