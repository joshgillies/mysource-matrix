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
* $Id: search_page_management.inc,v 1.19 2004/09/14 01:48:21 gsherwood Exp $
* $Name: not supported by cvs2svn $
*/

require_once SQ_INCLUDE_PATH.'/asset_management.inc';

/**
* Search_Page_Management
*
* @see Asset_Management
*
* @author  Dominic Wong <dwong@squiz.net>
* @version $Version$ - 1.0
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
			'results_per_page' => Array (
				'added'			=> '0.1',
				'type'			=> 'int',
				'default'		=> 10,
				'description'	=> 'Default results per page when searching.',
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
			'sort_by' => Array(
				'added'			=> '0.4',
				'type'			=> 'selection',
				'default'		=> '',
				'description'	=> 'the order in which assets are sorted',
				'parameters'	=> Array(
					'options' => Array(
						''				=> 'Relevance',
						'name'			=> 'Name',
						'short_name'	=> 'Short name',
						'created'		=> 'Created date',
						'updated'		=> 'Last updated date',
						'published'		=> 'Last published date',
						'type_code'		=> 'Asset Type',
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
		}//end version 0.1 upgrade

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
						'exclusive'     => null,
						'dependant'     => 1,
						'value'         => '',
						'link_type'     => SQ_LINK_TYPE_2 | SQ_LINK_TYPE_1,
					),
				),
		);

	}//end getEventList()


}//end class

?>