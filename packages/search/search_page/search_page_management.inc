<?php
/**
* +--------------------------------------------------------------------+
* | Squiz.net Open Source Licence                                      |
* +--------------------------------------------------------------------+
* | Copyright (c), 2003 Squiz Pty Ltd (ABN 77 084 670 600).            |
* +--------------------------------------------------------------------+
* | This source file may be used subject to, and only in accordance    |
* | with, the Squiz Open Source Licence Agreement found at             |
* | http://www.squiz.net/licence.                                      |
* | Make sure you have read and accept the terms of that licence,      |
* | including its limitations of liability and disclaimers, before     |
* | using this software in any way. Your use of this software is       |
* | deemed to constitute agreement to be bound by that licence. If you |
* | modify, adapt or enhance this software, you agree to assign your   |
* | intellectual property rights in the modification, adaptation and   |
* | enhancement to Squiz Pty Ltd for use and distribution under that   |
* | licence.                                                           |
* +--------------------------------------------------------------------+
*
* $Id: search_page_management.inc,v 1.22 2005/02/11 00:48:55 lwright Exp $
*
*/

require_once SQ_INCLUDE_PATH.'/asset_management.inc';

/**
* Search_Page_Management
*
* @see Asset_Management
*
* @author  Dominic Wong <dwong@squiz.net>
* @version $Revision: 1.22 $
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

		$this->vars = Array(
			'search_status' => Array(
				'added'			=> '0.3',
				'type'			=> 'serialise',
				'default'		=> Array(),
				'description'	=> 'Statuses that are being searched.',
			),
			'lineage_seperator' => Array (
				'added'			=> '0.1',
				'type'			=> 'text',
				'default'		=> ' > ',
				'description'	=> 'Seperator to use when printing the lineage to a search result asset.',
			),
			'submit_button_text' => Array (
				'added'			=> '0.1',
				'type'			=> 'text',
				'default'		=> 'Search',
				'description'	=> 'Text that would appear on the search page submit button',
			),
			'sis_button_text' => Array (
				'added'			=> '0.5',
				'type'			=> 'text',
				'default'		=> 'Search',
				'description'	=> 'Text that would appear on the search in search submit button',
			),
			'fields' => Array(
				'added'			=> '0.1',
				'type'			=> 'serialise',
				'default'		=> Array(),
				'description'	=> 'Search fields.',
			),
			'categories' => Array(
				'added'			=> '0.4',
				'type'			=> 'serialise',
				'default'		=> Array(),
				'description'	=> 'Search categories.',
			),
			'no_category_text' => Array(
				'added'			=> '0.4',
				'type'			=> 'text',
				'default'		=> 'No Category Selected',
				'description'	=> 'The text that appears for a "no category selected" options.',
			),
			'search_types' => Array(
				'added'			=> '0.1',
				'type'			=> 'serialise',
				'default'		=> Array(),
				'description'	=> 'Searchable asset types',
			),
			'search_styles' => Array(
				'added'			=> '0.1',
				'type'			=> 'serialise',
				'default'		=> Array(),
				'description'	=> 'Styles of the search fields',
			),
			'use_picker' => Array (
				'added'			=> '0.6',
				'type'			=> 'boolean',
				'default'		=> false,
				'description'	=> 'Use the JS date picker or not',
				'parameters' => Array(
									'allow_empty' => false
									),
			),
			'logic' => Array(
				'added'			=> '0.1',
				'type'			=> 'selection',
				'default'		=> 'AND',
				'description'	=> 'Whether to include all or any fields.',
				'parameters'	=> Array(
					'options' => Array(
						'AND'	=> 'Match on ALL fields',
						'OR'	=> 'Match on at least ONE field',
					),
					'multiple' => false,
					'allow_empty' => false,
				),
			),
			'reverse_sort' => Array(
				'added'			=> '0.4',
				'type'			=> 'boolean',
				'default'		=> false,
				'description'	=> 'Reverses the sort order field',
				'parameters' => Array(
										'allow_empty' => false
										),
			),
		);
		
		// change default sort by to a blank, which means sort
		// by relevance score
		$this->vars['sort_by']		= Array(
										'added'	=> '0.1',
										'type'	=> 'text',
										'default' => ''
										);
										
		// change default number of results per page to 10
		$this->vars['num_per_page']	= Array(
										'added'   => '0.1',
										'type'    => 'int',
										'default' => 10,
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

		if ($current_version == '0.1') {
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

		if ($current_version == '0.5') {
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
					if (isset($field_data['name'])) unset($field_data['name']);

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
					trigger_error('Search Page Upgrade [to 0.6] failed - Couldn\'t save attributes', E_USER_WARNING);
					return false;
				}

				$GLOBALS['SQ_SYSTEM']->am->forgetAsset($asset);
			}
			bam('SEARCH PAGE UPGRADE COMPLETE - FROM VERSION 0.5');
		}//end version 0.5 upgrade
		
		
		if ($current_version == '0.7') {
			// version 0.7->0.8
			// changed the format of the fields array
			bam('STARTING SEARCH PAGE UPGRADE - FROM VERSION 0.7');
			$children = $GLOBALS['SQ_SYSTEM']->am->getTypeAssetids($this->getAssetType(), false);
			foreach($children as $assetid) {
				$asset =& $GLOBALS['SQ_SYSTEM']->am->getAsset($assetid);
				
				// copy over the number of results per page to the corresponding
				// listing engine variable
				$asset->setAttrValue('num_per_page',  $asset->attr('results_per_page'));
				
				// create the type and posistion formatting folders
				$GLOBALS['SQ_SYSTEM']->am->includeAsset('folder');

				$sub_assets= Array(
					'type_formats'     => 'folder',
					'position_formats' => 'folder',
				);

				$type_formats = null;
				foreach ($sub_assets as $name => $type) {
					$sub_asset = new $type();
					$copy_link = Array('asset' => &$asset, 'value' => $name ,'link_type' => SQ_LINK_TYPE_2, 'is_dependant' => 1, 'is_exclusive' => 1);
		
					$sub_asset->setAttrValue('name', ucwords(str_replace('_',' ', $name)));
					if (!$sub_asset->create($copy_link)) return false;
		
					if ($name == 'type_formats') $type_formats = $sub_asset;
		
					$GLOBALS['SQ_SYSTEM']->am->forgetAsset($sub_asset);
					unset($sub_asset);
				}
				
				// NOW, convert the 'result list' formats stuff into the default
				// type format
				$ri_link = $GLOBALS['SQ_SYSTEM']->am->getLink($asset->id, SQ_LINK_TYPE_2, 'bodycopy', false, 'result_element');
				bam($ri_link);
				
				$ri_asset = $GLOBALS['SQ_SYSTEM']->am->getAsset($ri_link['minorid']);
				
				$ri_asset->setAttrValue('name', 'Default Format');
				$ri_asset->saveAttributes();
				
				$updates = Array(
								'majorid' => $db->quote($type_formats->id),
								'value'   => $db->quote('default_format'),
							);
				
				if (!$GLOBALS['SQ_SYSTEM']->rollbackUpdate('ast_lnk', $updates, 'linkid = '.$db->quote($ri_link['linkid']))) {
					trigger_error('Could not upgrade Search Page (to 0.8) - could not update link of default query', E_USER_WARNING);
					return false;
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
					foreach($types as $type_code => $value) {
						$types[$type_code] = true;
					}
					
					$asset->setAttrValue('types', $types);
				
				}
				
				
				if (!$asset->saveAttributes()) {
					trigger_errore('Search Page Upgrade [to 0.8] FAILED - Couldn\'t save attributes', E_USER_WARNING);
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
					'event_name'                 => 'requestKeywords',
					'broadcast_type_code'        => 'content_type_wysiwyg',
					'broadcast_strict_type_code' => true,
					'options' => Array(
						'side_of_link'  => 'major',
						'indirect'      => true,
						'is_exclusive'  => null,
						'is_dependant'  => 1,
						'value'         => '',
						'link_type'     => SQ_LINK_TYPE_2 | SQ_LINK_TYPE_1,
					),
				),
		);

	}//end getEventList()


}//end class

?>