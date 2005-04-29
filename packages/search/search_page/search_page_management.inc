<?php
/**
* +--------------------------------------------------------------------+
* | Squiz.net Commercial Module Licence                                |
* +--------------------------------------------------------------------+
* | Copyright (c) Squiz Pty Ltd (ACN 084 670 600).                     |
* +--------------------------------------------------------------------+
* | This source file is not open source or freely usable and may be    |
* | used subject to, and only in accordance with, the Squiz Commercial |
* | Module Licence.                                                    |
* | Please refer to http://www.squiz.net/licence for more information. |
* +--------------------------------------------------------------------+
*
* $Id: search_page_management.inc,v 1.38 2005/04/29 05:38:41 gsherwood Exp $
*
*/

require_once SQ_INCLUDE_PATH.'/asset_management.inc';

/**
* Search_Page_Management
*
* @see Asset_Management
*
* @author  Dominic Wong <dwong@squiz.net>
* @version $Revision: 1.38 $
* @package MySource_Matrix_Packages
* @subpackage search
*/
class Search_Page_Management extends Asset_Management
{


	/**
	* Constructor
	*
	*/
	function Search_Page_Management(&$pm)
	{
		$this->Asset_Management($pm);

		$this->vars['search_status'] = Array(
										'added'			=> '0.3',
										'type'			=> 'serialise',
										'default'		=> Array(),
										'description'	=> 'Statuses that are being searched.',
									   );

		$this->vars['sis_button_text'] = Array(
											'added'			=> '0.5',
											'type'			=> 'text',
											'default'		=> 'Search',
											'description'	=> 'Text that would appear on the search in search submit button',
										 );

		$this->vars['fields'] = Array(
									'added'			=> '0.1',
									'type'			=> 'serialise',
									'default'		=> Array(),
									'description'	=> 'Search fields.',
								);

		$this->vars['categories'] = Array(
										'added'			=> '0.4',
										'type'			=> 'serialise',
										'default'		=> Array(),
										'description'	=> 'Search categories.',
									);

		$this->vars['no_category_text'] = Array(
											'added'			=> '0.4',
											'type'			=> 'text',
											'default'		=> 'No Category Selected',
											'description'	=> 'The text that appears for a "no category selected" options.',
										  );

		$this->vars['search_types'] = Array(
										'added'			=> '0.1',
										'type'			=> 'serialise',
										'default'		=> Array(),
										'description'	=> 'Searchable asset types',
									  );

		$this->vars['search_styles'] = Array(
										'added'			=> '0.1',
										'type'			=> 'serialise',
										'default'		=> Array(),
										'description'	=> 'Styles of the search fields',
									   );

		$this->vars['use_picker'] = Array(
										'added'			=> '0.6',
										'type'			=> 'boolean',
										'default'		=> false,
										'description'	=> 'Use the JS date picker or not',
										'parameters'	=> Array('allow_empty' => false),
									);

		$this->vars['logic'] = Array(
								'added'			=> '0.1',
								'type'			=> 'selection',
								'default'		=> 'AND',
								'description'	=> 'Whether to include all or any fields.',
								'parameters'	=> Array(
													'multiple'		=> false,
													'allow_empty'	=> false,
													'options'		=> Array(
																		'AND'	=> 'Match on ALL fields',
																		'OR'	=> 'Match on at least ONE field',
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
		if (!parent::_upgrade($current_version)) return false;

		$db =& $GLOBALS['SQ_SYSTEM']->db;

		if (version_compare($current_version, '0.1', '<=')) {
			// version 0.1->0.2
			// changed the root assetid from storing an assetid to storing a NOTICE link
			// to the root assetid - that way if the root asset is purged it automatically deletes
			// the association
			bam('STARTING SEARCH PAGE UPGRADE - FROM VERSION 0.1');
			$children = $GLOBALS['SQ_SYSTEM']->am->getChildren(1, $this->getAssetType(), false);
			foreach ($children as $assetid => $type) {
				$asset = &$GLOBALS['SQ_SYSTEM']->am->getAsset($assetid);
				if (is_null($asset)) continue;

				// get the current root asset id and make sure it exists
				$current_root_assetid = $asset->attr('root_assetid');
				if (empty($current_root_assetid)) continue;
				$root_asset_info = $GLOBALS['SQ_SYSTEM']->am->getAssetInfo(Array($current_root_assetid));

				// create the notice link
				if (!empty($root_asset_info)) {
					$root_asset =& $GLOBALS['SQ_SYSTEM']->am->getAsset($current_root_assetid);
					if (!$GLOBALS['SQ_SYSTEM']->am->createAssetLink($asset, $root_asset, SQ_LINK_NOTICE, 'root')) {
						bam('SEARCH PAGE UPGRADE FAILED');
						return false;
					}
				}

				$GLOBALS['SQ_SYSTEM']->am->forgetAsset($asset);
			}
			bam('SEARCH PAGE UPGRADE COMPLETE - FROM VERSION 0.1');
		}//end version 0.1 upgrade

		if (version_compare($current_version, '0.5', '<=')) {
			// version 0.5->0.6
			// changed the format of the fields array
			bam('STARTING SEARCH PAGE UPGRADE - FROM VERSION 0.5');
			$children = $GLOBALS['SQ_SYSTEM']->am->getChildren(1, $this->getAssetType(), false);
			foreach ($children as $assetid => $type) {
				$asset = &$GLOBALS['SQ_SYSTEM']->am->getAsset($assetid);
				if (is_null($asset)) continue;

				$fields = $asset->attr('fields');

				$new_fields = Array();
				foreach ($fields as $field_name => $field_data) {
					// strip out the additional name field
					if (isset($field_data['name'])) {
						unset($field_data['name']);
					}

					// metadata fields need their params reworked
					if ($field_data['type'] == 'metadata') {
						$field_data['params'] = Array('assetid' => $field_data['params']['metadata_field']['assetid']);
					}

					if ($field_data['type'] == 'asset_attrib') {
						$new_params = Array();
						$new_params['asset_type'] = $field_data['params']['searchtype'];

						// [0] - name, [1] - type, [2] - id
						$attr_parts = explode('__', $field_data['params']['type_attr']);
						$new_params['attrid'] = $attr_parts[2];

						$field_data['params'] = $new_params;
					}

					$new_fields[$field_name] = $field_data;
				}
				$asset->setAttrValue('fields', $new_fields);
				if (!$asset->saveAttributes()) {
					trigger_localised_error('SCH0018', E_USER_WARNING);
					return false;
				}

				$GLOBALS['SQ_SYSTEM']->am->forgetAsset($asset);
			}
			bam('SEARCH PAGE UPGRADE COMPLETE - FROM VERSION 0.5');
		}//end version 0.5 upgrade


		if (version_compare($current_version, '0.7', '<=')) {
			// version 0.7->0.8
			// changed the format of the fields array
			bam('STARTING SEARCH PAGE UPGRADE - FROM VERSION 0.7');
			$children = $GLOBALS['SQ_SYSTEM']->am->getTypeAssetids($this->getAssetType(), false);
			foreach ($children as $assetid) {
				$asset =& $GLOBALS['SQ_SYSTEM']->am->getAsset($assetid);

				// copy over the number of results per page to the corresponding
				// listing engine variable
				$asset->setAttrValue('num_per_page',  $asset->attr('results_per_page'));

				// create the type and posistion formatting folders
				$GLOBALS['SQ_SYSTEM']->am->includeAsset('folder');

				$sub_assets= Array(
								'type_formats'		=> 'folder',
								'position_formats'	=> 'folder',
							 );

				$type_formats = null;
				foreach ($sub_assets as $name => $type) {
					$link = $GLOBALS['SQ_SYSTEM']->am->getLink($asset->id, SQ_LINK_TYPE_2, $type, true, $name);
					if (!is_null($link)) {
						trigger_localised_error('SCH0019', E_USER_WARNING, ucwords(str_replace('_', ' ', $name.' '.$type)));
						continue;
					}

					$sub_asset = new $type();
					$copy_link = Array('asset' => &$asset, 'value' => $name ,'link_type' => SQ_LINK_TYPE_2, 'is_dependant' => 1, 'is_exclusive' => 1);

					$sub_asset->setAttrValue('name', ucwords(str_replace('_',' ', $name)));
					if (!$sub_asset->create($copy_link)) return false;

					if ($name == 'type_formats') {
						$type_formats = $sub_asset;
					}

					$GLOBALS['SQ_SYSTEM']->am->forgetAsset($sub_asset);
					unset($sub_asset);
				}

				// NOW, convert the 'result list' formats stuff into the default
				// type format
				$ri_link = $GLOBALS['SQ_SYSTEM']->am->getLink($asset->id, SQ_LINK_TYPE_2, 'bodycopy', false, 'result_element');

				if (is_null($ri_link)) {
					trigger_localised_error('SCH0020', E_USER_WARNING);
					continue;
				} else {

					$ri_asset = &$GLOBALS['SQ_SYSTEM']->am->getAsset($ri_link['minorid']);

					$ri_asset->setAttrValue('name', 'Default Format');
					$ri_asset->saveAttributes();

					$updates = Array(
								'majorid'	=> $db->quote($type_formats->id),
								'value'		=> $db->quote('default_format'),
							   );

					$sql = 'UPDATE
								sq_ast_lnk
							SET
								majorid	= '.$db->quoteSmart($type_formats->id).',
								value	= '.$db->quoteSmart('default_format').'
							WHERE
								linkid = '.$db->quoteSmart($ri_link['linkid']);

					$result = $db->query($sql);
					assert_valid_db_result($result);

				}

				// now move search types if any are set
				$types = $asset->attr('search_types');
				if ((count($types) > 0) && !is_bool(reset($types))) {

					// search_types attribute is now listing engine's
					// types attribute
					$types = $asset->attr('search_types');

					// flip the types array to get typecodes as keys, then set them
					// to inherit types (as this was what Search Manager used to do)
					$types = array_flip($types);
					foreach ($types as $type_code => $value) {
						$types[$type_code] = true;
					}

					$asset->setAttrValue('types', $types);

				}


				if (!$asset->saveAttributes()) {
					trigger_localised_error('SCH0021', E_USER_WARNING);
					return false;
				}

				$GLOBALS['SQ_SYSTEM']->am->forgetAsset($asset);
			}

			bam('SEARCH PAGE UPGRADE COMPLETE - FROM VERSION 0.7');

		}//end version 0.7 upgrade

		return true;

	}//end _upgrade()


	/**
	* Return an array with information about the events that this page is listening for
	*
	* @access public
	* @return boolean
	*/
	function getEventList()
	{
		return Array(
				Array(
					'event_name'					=> 'requestKeywords',
					'broadcast_type_code'			=> 'content_type',
					'broadcast_strict_type_code'	=> false,
					'options'						=> Array(
														'side_of_link'	=> 'major',
														'indirect'		=> true,
														'is_exclusive'	=> null,
														'is_dependant'	=> 1,
														'value'			=> '',
														'link_type'		=> SQ_LINK_TYPE_2 | SQ_LINK_TYPE_1,
													   ),
				),
			   );

	}//end getEventList()


}//end class

?>