<?php
/**
* +--------------------------------------------------------------------+
* | This MySource Matrix CMS file is Copyright (c) Squiz Pty Ltd       |
* | ABN 77 084 670 600                                                 |
* +--------------------------------------------------------------------+
* | IMPORTANT: Your use of this Software is subject to the terms of    |
* | the Licence provided in the file licence.txt. If you cannot find   |
* | this file please contact Squiz (www.squiz.com.au) so we may provide|
* | you a copy.                                                        |
* +--------------------------------------------------------------------+
*
* $Id: js_api_management.inc,v 1.29 2013/08/13 01:30:48 akarelia Exp $
*
*/

require_once SQ_INCLUDE_PATH.'/asset_management.inc';

/**
* Javascript API Management
*
* Purpose
*
*
* @author  Nic Hubbard <nic@zedsaid.com>
* @version $Revision: 1.29 $
* @package MySource_Matrix_Packages
* @subpackage web_services
*/
class JS_Api_Management extends Asset_Management
{


    /**
    * Constructor
    *
    */
    function __construct(Package_Manager $pm)
    {
        Asset_Management::__construct($pm);


		$this->vars	= Array (
						'types'						=> Array(
														'added'			=> '0.1',
														'type'			=> 'serialise',
														'default'		=> Array(),
										   	   			),
						'types_restriction'			=> Array(
														'added'			=> '0.1',
														'type'			=> 'serialise',
														'default'		=> Array(),
										   	   			),
						'name'						=> Array(
														'added'			=> '0.1',
														'type'			=> 'text',
														'default'		=> '',
														'description'	=> 'The full name of the page',
														'update_fn'		=> '',
														'is_admin'		=> FALSE,
										   	   			),
						'short_name'				=> Array(
														'added'			=> '0.1',
														'type'			=> 'text',
														'default'		=> '',
														'description'	=> 'The short name of the page',
														'update_fn'		=> '',
														'is_admin'		=> FALSE,
										       			),
						'root_node'					=> Array(
														'added'			=> '0.2',
														'type'			=> 'assetid',
														'default'		=> '',
														'description'	=> 'The root node to restrict to. DEPRECIATED in v0.2 and up',
														'update_fn'		=> '',
														'is_admin'		=> FALSE,
											   			),
						'api_key'					=> Array(
														'added'			=> '0.1',
														'type'			=> 'text',
														'default'		=> '',
														'description'	=> 'The API Key',
														'update_fn'		=> '',
														'is_admin'		=> FALSE,
											   			),
						'create_attributes'			=> Array(
														'added'			=> '0.1',
														'type'			=> 'boolean',
														'default'		=> FALSE,
														'parameters'	=> Array(
																			'allow_empty'	=> FALSE,
																		   ),
											   			),
						'allow_json'				=> Array(
														'added'			=> '0.1',
														'type'			=> 'boolean',
														'default'		=> TRUE,
														'parameters'	=> Array(
																			'allow_empty'	=> FALSE,
																		   ),
											   			),
						'ignore_permissions'		=> Array(
														'added'			=> '0.2',
														'type'			=> 'boolean',
														'default'		=> FALSE,
														'parameters'	=> Array(
																			'allow_empty'	=> FALSE,
																		   ),
											   			),
						'get_general'				=> Array(
														'added'			=> '0.1',
														'type'			=> 'boolean',
														'default'		=> FALSE,
														'parameters'	=> Array(
																			'allow_empty'	=> FALSE,
																		   ),
											   			),
						'get_attributes'			=> Array(
														'added'			=> '0.1',
														'type'			=> 'boolean',
														'default'		=> FALSE,
														'parameters'	=> Array(
																			'allow_empty'	=> FALSE,
																		   ),
											   			),
						'set_attribute'				=> Array(
														'added'			=> '0.1',
														'type'			=> 'boolean',
														'default'		=> FALSE,
														'parameters'	=> Array(
																			'allow_empty'	=> FALSE,
																		   ),
											   			),
						'get_metadata'				=> Array(
														'added'			=> '0.1',
														'type'			=> 'boolean',
														'default'		=> FALSE,
														'parameters'	=> Array(
																			'allow_empty'	=> FALSE,
																		   ),
											   			),
						'set_metadata'				=> Array(
														'added'			=> '0.1',
														'type'			=> 'boolean',
														'default'		=> FALSE,
														'parameters'	=> Array(
																			'allow_empty'	=> FALSE,
																		   ),
											   			),
						'trash_asset'				=> Array(
														'added'			=> '0.1',
														'type'			=> 'boolean',
														'default'		=> FALSE,
														'parameters'	=> Array(
																			'allow_empty'	=> FALSE,
																		   ),
											   			),
						'get_parents'				=> Array(
														'added'			=> '0.1',
														'type'			=> 'boolean',
														'default'		=> FALSE,
														'parameters'	=> Array(
																			'allow_empty'	=> FALSE,
																		   ),
											   			),
						'get_children'				=> Array(
														'added'			=> '0.1',
														'type'			=> 'boolean',
														'default'		=> FALSE,
														'parameters'	=> Array(
																			'allow_empty'	=> FALSE,
																		   ),
											   			),
						'get_permissions'			=> Array(
														'added'			=> '0.1',
														'type'			=> 'boolean',
														'default'		=> FALSE,
														'parameters'	=> Array(
																			'allow_empty'	=> FALSE,
																		   ),
											   			),
						'create_asset'				=> Array(
														'added'			=> '0.1',
														'type'			=> 'boolean',
														'default'		=> FALSE,
														'parameters'	=> Array(
																			'allow_empty'	=> FALSE,
																		   ),
											   			),
						'get_asset_types'			=> Array(
														'added'			=> '0.1',
														'type'			=> 'boolean',
														'default'		=> FALSE,
														'parameters'	=> Array(
																			'allow_empty'	=> FALSE,
																		   ),
											   			),
						'get_locks_info'			=> Array(
														'added'			=> '0.4',
														'type'			=> 'boolean',
														'default'		=> FALSE,
														'parameters'	=> Array(
																			'allow_empty'	=> FALSE,
																		   ),
											   			),
						'acquire_lock'				=> Array(
														'added'			=> '0.1',
														'type'			=> 'boolean',
														'default'		=> FALSE,
														'parameters'	=> Array(
																			'allow_empty'	=> FALSE,
																		   ),
											   			),
						'release_lock'				=> Array(
														'added'			=> '0.1',
														'type'			=> 'boolean',
														'default'		=> FALSE,
														'parameters'	=> Array(
																			'allow_empty'	=> FALSE,
																		   ),
											   			),
						'create_link'				=> Array(
														'added'			=> '0.1',
														'type'			=> 'boolean',
														'default'		=> FALSE,
														'parameters'	=> Array(
																			'allow_empty'	=> FALSE,
																		   ),
											   			),
						'remove_link'				=> Array(
														'added'			=> '0.1',
														'type'			=> 'boolean',
														'default'		=> FALSE,
														'parameters'	=> Array(
																			'allow_empty'	=> FALSE,
																		   ),
											   			),
						'move_link'					=> Array(
														'added'			=> '0.1',
														'type'			=> 'boolean',
														'default'		=> FALSE,
														'parameters'	=> Array(
																			'allow_empty'	=> FALSE,
																		   ),
											   			),
						'update_link'				=> Array(
														'added'			=> '0.1',
														'type'			=> 'boolean',
														'default'		=> FALSE,
														'parameters'	=> Array(
																			'allow_empty'	=> FALSE,
																		   ),
											   			),
						'get_link_id'				=> Array(
														'added'			=> '0.1',
														'type'			=> 'boolean',
														'default'		=> FALSE,
														'parameters'	=> Array(
																			'allow_empty'	=> FALSE,
																		   ),
											   			),
						'get_asset_tree'			=> Array(
														'added'			=> '0.1',
														'type'			=> 'boolean',
														'default'		=> FALSE,
														'parameters'	=> Array(
																			'allow_empty'	=> FALSE,
																		   ),
											   			),
						'get_keywords_replacements'	=> Array(
														'added'			=> '0.4',
														'type'			=> 'boolean',
														'default'		=> FALSE,
														'parameters'	=> Array(
																			'allow_empty'	=> FALSE,
																		   ),
											   			),
						'set_asset_status'			=> Array(
														'added'			=> '0.4',
														'type'			=> 'boolean',
														'default'		=> FALSE,
														'parameters'	=> Array(
																			'allow_empty'	=> FALSE,
																		   ),
														),
						'get_web_path'			=> Array(
														'added'			=> '0.5',
														'type'			=> 'boolean',
														'default'		=> FALSE,
														'parameters'	=> Array(
																			'allow_empty'	=> FALSE,
																		   ),
														),
						'set_web_path'			=> Array(
														'added'			=> '0.5',
														'type'			=> 'boolean',
														'default'		=> FALSE,
														'parameters'	=> Array(
																			'allow_empty'	=> FALSE,
																		   ),
														),
						'force_simple_edit'		=> Array(
														'added'			=> '0.5',
														'type'			=> 'boolean',
														'default'		=> FALSE,
														'parameters'	=> Array(
																			'allow_empty'	=> FALSE,
																		   ),
														),
						'get_workflow_schema'	=> Array(
														'added'			=> '0.5',
														'type'			=> 'boolean',
														'default'		=> FALSE,
														'parameters'	=> Array(
																			'allow_empty'	=> FALSE,
																		   ),
														),
						'set_file_content'		=> Array(
														'added'			=> '0.6',
														'type'			=> 'boolean',
														'default'		=> FALSE,
														'parameters'	=> Array(
																			'allow_empty'	=> FALSE,
																		   ),
														),
						'import_assets'			=> Array(
														'added'			=> '0.6',
														'type'			=> 'boolean',
														'default'		=> FALSE,
														'parameters'	=> Array(
																			'allow_empty'	=> FALSE,
																		   ),
														),
						'get_roles'				=> Array(
														'added'			=> '0.5',
														'type'			=> 'boolean',
														'default'		=> FALSE,
														'parameters'	=> Array(
																			'allow_empty'	=> FALSE,
																		   ),
														),
						'execute_html_tidy'		=> Array(
														'added'			=> '0.6',
														'type'			=> 'boolean',
														'default'		=> FALSE,
														'parameters'	=> Array(
																			'allow_empty'=> FALSE,
																		   ),
														),
						'use_enhanced_js'	=> Array(
														'added'			=> '0.6',
														'type'			=> 'boolean',
														'default'		=> TRUE,
														'parameters'	=> Array(
																			'allow_empty'=> FALSE,
																		   ),
														),
						'get_lineage'		=> Array(
														'added'			=> '0.6',
														'type'			=> 'boolean',
														'default'		=> FALSE,
														'parameters'	=> Array(
																			'allow_empty'=> FALSE,
																		   ),
														),
						'get_lineage_from_url'		=> Array(
														'added'			=> '0.6',
														'type'			=> 'boolean',
														'default'		=> FALSE,
														'parameters'	=> Array(
																			'allow_empty'=> FALSE,
																		   ),
														),
						'get_url_from_lineage'		=> Array(
														'added'			=> '0.6',
														'type'			=> 'boolean',
														'default'		=> FALSE,
														'parameters'	=> Array(
																			'allow_empty'=> FALSE,
																		   ),
														),

						'batch_requests'	=> Array(
														'added'			=> '0.6',
														'type'			=> 'boolean',
														'default'		=> FALSE,
														'parameters'	=> Array(
																			'allow_empty'=> FALSE,
																		   ),
														),
						'clone_asset'		=> Array(
														'added'			=> '0.6',
														'type'			=> 'boolean',
														'default'		=> FALSE,
														'parameters'	=> Array(
																			'allow_empty'=> FALSE,
																		   ),
														),
						'show_diff'			=> Array(
														'added'			=> '0.6',
														'type'			=> 'boolean',
														'default'		=> FALSE,
														'parameters'	=> Array(
																			'allow_empty'=> FALSE,
																		   ),
														),
						'get_context'		=> Array(
														'added'			=> '0.6',
														'type'			=> 'boolean',
														'default'		=> FALSE,
														'parameters'	=> Array(
																			'allow_empty'=> FALSE,
																		   ),
														),
						'set_context'		=> Array(
														'added'			=> '0.6',
														'type'			=> 'boolean',
														'default'		=> FALSE,
														'parameters'	=> Array(
																			'allow_empty'=> FALSE,
																		   ),
														),
						'get_metadata_schema'		=> Array(
														'added'			=> '0.6',
														'type'			=> 'boolean',
														'default'		=> FALSE,
														'parameters'	=> Array(
																			'allow_empty'=> FALSE,
																		   ),
														),
						'edit_metadata_schema'		=> Array(
														'added'			=> '0.6',
														'type'			=> 'boolean',
														'default'		=> FALSE,
														'parameters'	=> Array(
																			'allow_empty'=> FALSE,
																		   ),
														),
						'sync_context'				=> Array(
														'added'			=> '0.6',
														'type'			=> 'boolean',
														'default'		=> FALSE,
														'parameters'	=> Array(
																			'allow_empty'=> FALSE,
																		   ),
														),
						'create_form_question'		=> Array(
														'added'			=> '0.6',
														'type'			=> 'boolean',
														'default'		=> FALSE,
														'parameters'	=> Array(
																			'allow_empty'=> FALSE,
																		   ),
														),
						'update_form_question'		=> Array(
														'added'			=> '0.6',
														'type'			=> 'boolean',
														'default'		=> FALSE,
														'parameters'	=> Array(
																			'allow_empty'=> FALSE,
																		   ),
														),
						'remove_form_question'		=> Array(
														'added'			=> '0.6',
														'type'			=> 'boolean',
														'default'		=> FALSE,
														'parameters'	=> Array(
																			'allow_empty'=> FALSE,
																		   ),
														),
						'update_question_order'		=> Array(
														'added'			=> '0.6',
														'type'			=> 'boolean',
														'default'		=> FALSE,
														'parameters'	=> Array(
																			'allow_empty'=> FALSE,
																		   ),
														),
						'get_ga_statistics'	    	=> Array(
														'added'			=> '0.7',
														'type'			=> 'boolean',
														'default'		=> FALSE,
														'parameters'	=> Array(
																			'allow_empty'=> FALSE,
																		   ),
														),
						'get_ga_experiments'	   	=> Array(
														'added'			=> '0.9',
														'type'			=> 'boolean',
														'default'		=> FALSE,
														'parameters'	=> Array(
																			'allow_empty'=> FALSE,
																		   ),
														),
						'get_ga_goals'	   			=> Array(
														'added'			=> '0.10',
														'type'			=> 'boolean',
														'default'		=> FALSE,
														'parameters'	=> Array(
																			'allow_empty'=> FALSE,
																		   ),
														),
						'get_system_information'  	=> Array(
														'added'			=> '0.9',
														'type'			=> 'boolean',
														'default'		=> FALSE,
														'parameters'	=> Array(
																			'allow_empty'=> FALSE,
																		   ),
														),
						'validate_user'  			=> Array(
														'added'			=> '0.11',
														'type'			=> 'boolean',
														'default'		=> FALSE,
														'parameters'	=> Array(
																			'allow_empty'=> FALSE,
																		   ),
														),
						'get_indexing_contents'  	=> Array(
														'added'			=> '0.11',
														'type'			=> 'boolean',
														'default'		=> FALSE,
														'parameters'	=> Array(
																			'allow_empty'=> FALSE,
																		   ),
														),
						'set_permission'			=> Array(
														'added'			=> '0.12',
														'type'			=> 'boolean',
														'default'		=> FALSE,
														'parameters'	=> Array(
																			'allow_empty'	=> FALSE,
																		   ),
											 			),

                        'morph_asset'			=> Array(
                                                'added'			=> '0.13',
                                                'type'			=> 'boolean',
                                                'default'		=> FALSE,
                                                'parameters'	=> Array(
                                                                    'allow_empty'	=> FALSE,
                                                                   ),
                                                ),
					  );

	}//end constructor


	/**
	* Upgrade
	*
	* @param string	$current_version	the version of the asset that is currenly installed
	*
	* @return boolean
	* @access public
	*/
	function _upgrade($current_version)
	{
		if (!parent::_upgrade($current_version)) return FALSE;

		$asset_name = strtoupper($this->_pm->getTypeInfo($this->getAssetType(), 'name'));

		if (version_compare($current_version, '0.2', '<')) {
			// version 0.1 -> 0.2
			// Root nodes changed from assetid to links
			pre_echo('STARTING '.$asset_name.' UPGRADE - FROM VERSION 0.1');
			$apis = $GLOBALS['SQ_SYSTEM']->am->getTypeAssetids('js_api', FALSE, TRUE);
			$count = 0;
			foreach ($apis as $assetid => $type_code) {
				$asset = $GLOBALS['SQ_SYSTEM']->am->getAsset($assetid, 'js_api');
				if (is_null($asset)) continue;

				// Current links if applicable
				$root_links = $GLOBALS['SQ_SYSTEM']->am->getLinks($asset->id, SQ_LINK_NOTICE, '', FALSE, 'major', 'root');
				$current_root_node_id = $asset->attr('root_node');

				// Set a new notice link for the current root node if not already
				if (!empty($current_root_node_id)) {
					$exists = FALSE;
					foreach ($root_links as $root_link) {
						if ($root_link['minorid'] == $current_root_node_id) {
							$exists = TRUE;
						}//end if
					}//end foreach

					if (!$exists) {
						$current_root_node = $GLOBALS['SQ_SYSTEM']->am->getAsset($current_root_node_id);
						if (!is_null($current_root_node)) {
							$GLOBALS['SQ_SYSTEM']->am->acquireLock($asset->id, 'links');
							$new_link = $GLOBALS['SQ_SYSTEM']->am->createAssetLink($asset, $current_root_node, SQ_LINK_NOTICE, 'root');
							if (!empty($new_link)) $count++;
							$GLOBALS['SQ_SYSTEM']->am->releaseLock($asset->id, 'links');
						}//end if
					}//end if
				}//end if

				$GLOBALS['SQ_SYSTEM']->am->forgetAsset($asset);
				unset($asset);

			}//end foreach
			pre_echo('Updated - '.$count.' Javascript Api(s)...');
			pre_echo($asset_name.' UPGRADE COMPLETE - FROM VERSION 0.1');

		}//end if - upgrade to version 0.2

		if (version_compare($current_version, '0.3', '<')) {

			// Generate API key for those had empty default key
			pre_echo('STARTING '.$asset_name.' UPGRADE - TO VERSION 0.3 - FIXING Default API Key');
			$apis = $GLOBALS['SQ_SYSTEM']->am->getTypeAssetids('js_api', FALSE, TRUE);
			$count = 0;
			foreach ($apis as $assetid => $type_code) {
				$asset = $GLOBALS['SQ_SYSTEM']->am->getAsset($assetid, 'js_api');
				if (is_null($asset)) continue;
				$key = $asset->attr('api_key');
				if(empty($key)) {
					// regenerate api key
                    include_once SQ_FUDGE_PATH.'/general/security.inc';
					$new_key = security_rand(1000000000, 9999999999);
					$GLOBALS['SQ_SYSTEM']->setRunLevel(SQ_RUN_LEVEL_FORCED);
					$asset->setAttrValue('api_key', $new_key);
					$asset->saveAttributes();
					$GLOBALS['SQ_SYSTEM']->restoreRunLevel();
					pre_echo('JS API Asset #'.$asset->id.' has a new API key '.$new_key);
					$count ++;
				}

				$GLOBALS['SQ_SYSTEM']->am->forgetAsset($asset);
				unset($asset);

			}//end foreach
			pre_echo('Updated - '.$count.' Javascript Api(s)...');
			pre_echo($asset_name.' UPGRADE COMPLETE - TO VERSION 0.3');

		}//end if - upgrade to version 0.3





		// A common way to add new attributes (including endpoints), as that's all
		// that's been done since 0.4. :) They'll get set to FALSE by default.
		$new_attributes = Array(
			'get_locks_info'            => '0.4',
			'get_keywords_replacements' => '0.4',
			'set_asset_status'          => '0.4',
			'get_web_path'              => '0.5',
			'force_simple_edit'         => '0.5',
			'get_workflow_schema'       => '0.5',
			'set_file_contents'         => '0.6',
			'import_assets'             => '0.6',
			'get_ga_statistics'         => '0.7',
			'get_system_information'    => '0.8',
			'get_ga_experiments'        => '0.9',
			'get_ga_goals'              => '0.10',
			'validate_user'             => '0.11',
			'get_indexing_contents'     => '0.11',
		);

		// Work out the latest version from the new attributesm and use that
		// as the basis for jumping into the next upgrade block.
		$latest_version = array_reduce($new_attributes, function($value, $carry) {
			if ($carry === NULL) {
				return $value;
			} else if (version_compare($value, $carry, '>') === TRUE) {
				return $value;
			} else {
				return $carry;
			}
		});

		if (version_compare($current_version, $latest_version, '<')) {

			pre_echo('STARTING JS API UPGRADE - FROM VERSION '.$current_version.' TO '.$latest_version);
				$root_folder = $GLOBALS['SQ_SYSTEM']->am->getSystemAsset('root_folder');
				$children = $GLOBALS['SQ_SYSTEM']->am->getChildren($root_folder->id, $this->getAssetType(), FALSE);

				pre_echo('Upgrading '.count($children).' assets...');
				foreach ($children as $assetid => $type) {
					$asset = $GLOBALS['SQ_SYSTEM']->am->getAsset($assetid, $this->getAssetType());
					if (is_null($asset)) continue;

					if (!$GLOBALS['SQ_SYSTEM']->am->acquireLock($asset->id, 'attributes')) {
						trigger_localised_error('CMS0025', sprintf(translate('JS API Upgrade [to %1$s] FAILED - Unable to Acquire Lock on "%2$s" (#%3$s)', $latest_version), $asset->name, $asset->id), E_USER_WARNING);
						return FALSE;
					}

					foreach ($new_attributes as $attr => $version) {
						if (version_compare($current_version, $version, '<') === TRUE) {
							$asset->setAttrValue($attr, FALSE);
						}
					}

					if (!$asset->saveAttributes()) {
						trigger_localised_error('CMS0026', sprintf(translate('JS API Upgrade [to %s] FAILED'), $latest_version), E_USER_WARNING);
						return FALSE;
					}

					$GLOBALS['SQ_SYSTEM']->am->releaseLock($asset->id, 'attributes');
				}//end foreach

			pre_echo('UPGRADE JS API COMPLETE - FROM VERSION '.$current_version.' TO '.$latest_version);

		}//end if - upgrade to version 0.x

		return TRUE;

	}//end _upgrade()


}//end class
?>
