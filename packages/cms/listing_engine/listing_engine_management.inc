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
* $Id: listing_engine_management.inc,v 1.23.2.6 2006/01/11 00:09:02 rong Exp $
*
*/

require_once SQ_INCLUDE_PATH.'/asset_management.inc';

/**
* Listing_Engine_Management
*
* @see Asset_Management
*
* @author  Marc McIntyre <mmcintyre@squiz.net>
* @author  Greg Sherwood <greg@squiz.net>
* @version $Revision: 1.23.2.6 $
* @package MySource_Matrix_Packages
* @subpackage cms
*/
class Listing_Engine_Management extends Asset_Management
{


	/**
	* Constructor
	*
	*/
	function Listing_Engine_Management(&$pm)
	{
		$this->Asset_Management($pm);

		$this->vars['types'] = Array(
								'added'		=> '0.1',
								'type'		=> 'serialise',
								'default'	=> Array(),
							   );

		$this->vars['submit_button_text'] = Array(
												'added'		=> '0.1',
												'type'		=> 'text',
												'default'	=> 'Submit',
											);

		$this->vars['sort_by'] = Array(
									'added'		=> '0.1',
									'type'		=> 'serialise',
									'default'	=> Array(
													0	=> Array (
															'params'	=> Array (
																			'field'	=> 'name',
																		   ),
															'type'		=> 'field',
															'name'		=> 'Name',
														   ),
													1	=> Array (
															'params'	=> Array (
																			'field'	=> 'short_name',
																		   ),
															'type'		=> 'field',
															'name'		=> 'Short Name',
														   ),
													2	=> Array (
															'params'	=> Array (
																			'field'	=> 'created',
																		   ),
															'type'		=> 'field',
															'name'		=> 'Created Date',
														   ),
													3	=> Array (
															'params'	=> Array (
																			'field'	=> 'updated',
																		   ),
															'type'		=> 'field',
															'name'		=> 'Last Updated Date',
														   ),
													4	=> Array (
															'params'	=> Array (
																			'field'	=> 'published',
																		   ),
															'type'		=> 'field',
															'name'		=> 'Last Published Date',
														   ),
													5	=> Array (
															'params'	=> Array (
																			'field'	=> 'type_code',
																		   ),
															'name'		=> 'Asset Type',
															'type'		=> 'field',
														   ),
												   ),
								 );

		$this->vars['default_sort_by'] = Array(
											'added'		=> '0.1',
											'type'		=> 'int',
											'default'	=> 0,
										 );

		$this->vars['reverse_sort'] = Array(
										'added'			=> '0.1',
										'type'			=> 'boolean',
										'default'		=> false,
										'parameters'	=> Array('allow_empty'	=> false,),
									  );

		$this->vars['sort_direction_asc_text'] = Array(
													'added'		=> '0.1',
													'type'		=> 'text',
													'default'	=> 'Ascending',
												 );

		$this->vars['sort_direction_desc_text'] = Array(
													'added'		=> '0.1',
													'type'		=> 'text',
													'default'	=> 'Descending',
												  );

		$this->vars['subs_only'] = Array(
									'added'			=> '0.1',
									'type'			=> 'boolean',
									'default'		=> false,
									'parameters'	=> Array( 'allow_empty' => false,),
								   );

		$this->vars['direction'] = Array(
									'added'			=> '0.1',
									'type'			=> 'selection',
									'default'		=> 'down',
									'parameters'	=> Array(
														'multiple'		=> false,
														'allow_empty'	=> false,
														'options'		=> Array(
																			'up'	=> 'Up the tree',
																			'down'	=> 'Down the tree',
																		   ),
													   ),
								   );

		$this->vars['group_by'] = Array(
									'added'			=> '0.1',
									'type'			=> 'selection',
									'default'		=> 'number',
									'parameters'	=> Array(
														'multiple'		=> false,
														'allow_empty'	=> false,
														'options'		=> Array(
																			'number'	=> 'Standard',
																			'letter'	=> 'A-Z listing',
																			'grouped'	=> 'Custom Grouping',
																			'random'	=> 'Random',
																		   ),
													   ),
								  );

		$this->vars['num_per_page'] = Array(
										'added'		=> '0.1',
										'type'		=> 'int',
										'default'	=> 0,
									  );

		$this->vars['column_layout'] = Array(
										'added'		=> '0.1',
										'type'		=> 'serialise',
										'default'	=> Array('0' => '1'),
									   );

		$this->vars['asset_positions'] = Array(
											'added'		=> '0.1',
											'type'		=> 'serialise',
											'default'	=> Array(),
										 );

		$this->vars['lineage_seperator'] = Array (
											'added'			=> '0.1',
											'type'			=> 'text',
											'default'		=> ' > ',
											'description'	=> 'Seperator to use when printing the lineage to a search result asset.',
										   );

		$this->vars['prev_page_text'] = Array(
											'added'		=> '0.1',
											'type'		=> 'text',
											'default'	=> 'Previous',
										);

		$this->vars['next_page_text'] = Array(
											'added'		=> '0.1',
											'type'		=> 'text',
											'default'	=> 'Next',
										);

		$this->vars['parameter_map'] = Array(
										'added'			=> '0.4',
										'type'			=> 'parameter_map',
										'default'		=> Array(),
										'parameters'	=> Array(
															'parameters'	=> Array(
																				'root_node'	=> 'Replacement Root node for the listing (must be a child of the static root node)',
																			   ),
														   ),
									   );

		$this->vars['asset_grouping'] = Array(
											'added'		=> '0.5',
											'type'		=> 'serialise',
											'default'	=> Array(),
										);
	}//end constructor


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


	/**
	* Installs this asset
	*
	* @return boolean
	* @access public
	*/
	function _install()
	{
		$GLOBALS['SQ_SYSTEM']->doTransaction('BEGIN');

		$search_page_fields = Array('lineage_seperator');
		$db = &$GLOBALS['SQ_SYSTEM']->db;

		$attributes = Array();

		foreach ($this->vars as $attr_name => $attr_info) {
			if (in_array($attr_name, $search_page_fields)) {
				$result = $this->_hoardAttribute($attr_name, 'search_page');
			} else {
				$result = $this->_hoardAttribute($attr_name, 'page_asset_listing');
			}

			foreach ($result as $result_info) {
				$attributes[$result_info['attrid']] = $result_info;
			}
		}

		$this->_removeAttributes($attributes);

		if (!parent::_install()) {
			trigger_localised_error('CMS0038', E_USER_WARNING);
			$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
			return false;
		}

		$this->_restoreAttributes($attributes);
		$GLOBALS['SQ_SYSTEM']->doTransaction('COMMIT');

		return true;

	}//end _install()


	function _upgrade($current_version)
	{
		$db =& $GLOBALS['SQ_SYSTEM']->db;


		if (version_compare($current_version,'0.1','==')) {
			$search_page_fields = Array('lineage_seperator');

			bam('UPGRADING LISTING ENGINE - FROM VERSION '.$current_version);

			$attributes = Array();

			foreach ($this->vars as $attr_name => $attr_info) {
				if (in_array($attr_name, $search_page_fields)) {
					$result = $this->_hoardAttribute($attr_name, 'search_page');
				} else {
					$result = $this->_hoardAttribute($attr_name, 'page_asset_listing');
				}

				foreach ($result as $result_info) {
					$attributes[$result_info['attrid']] = $result_info;
				}
			}

			$this->_removeAttributes($attributes);
			if (!parent::_upgrade($current_version)) {
				trigger_localised_error('CMS0039', E_USER_WARNING, '0.1');
				return false;
			}
			$this->_restoreAttributes($attributes);

			bam('LISTING ENGINE UPGRADE COMPLETE - FROM VERSION '.$current_version);

		}

		if (version_compare($current_version, '0.3', '<')) {
			$search_page_fields = Array('submit_button_text', 'reverse_sort');

			$fields_to_delete = Array('sort_by_attr');

			bam('UPGRADING LISTING ENGINE - FROM VERSION '.$current_version.' TO VERSION 0.3');
			// move reverse sort into sort_direction
			// move sort by to sort by list
			// move sort_by attr to sort by list

			$hoarded_attributes = Array();

			foreach ($this->vars as $attr_name => $attr_info) {
				if (in_array($attr_name, $search_page_fields)) {
					$result = $this->_hoardAttribute($attr_name, 'search_page');
					foreach ($result as $result_info) {
						$hoarded_attributes[$result_info['attrid']] = $result_info;
					}
				}
			}

			$children = $GLOBALS['SQ_SYSTEM']->am->getTypeAssetids($this->getAssetType(), false);

			foreach ($children as $assetid) {
				$asset = &$GLOBALS['SQ_SYSTEM']->am->getAsset($assetid);

				$sort_by = $asset->attr('sort_by');

				if (!empty($sort_by) && $sort_by != '__attr__') {

					$edit_fns = $asset->getEditFns();
					// note that __attr__ has changed to asset_attrib in the new version
					$sort_by_options = $edit_fns->_getSortByOptions($asset);
					$name = array_get_index($sort_by_options, $sort_by, $sort_by);

					$sort_by_list = Array(
										'0'	=> Array(
												'name'		=> $name,
												'type'		=> 'field',
												'params'	=> Array(
																'field'	=> $sort_by,
															   ),
											   ),
									);

				} else if ($sort_by == '__attr__') {
					$sort_by_attr = $asset->attr('sort_by_attr');
					$sort_by_list = Array();

					$attributes = Array();
					// get the attributes for each type
					foreach (array_keys($sort_by_attr) as $type) {
						$attributes[$type] = $GLOBALS['SQ_SYSTEM']->am->getAssetTypeAttributes($type);
					}

					foreach ($sort_by_attr as $type => $attr_name) {
						$attr_type = $attributes[$type][$attr_name];
						$sort_by_list[] = Array(
											'name'		=> $attr_name,
											'type'		=> 'asset_attrib',
											'params'	=> Array(
															'type_code'	=> $type,
															'attr_type'	=> $attr_type,
															'attribute'	=> $attr_name,
														   ),
										  );
					}

				}//end if attr

				$this->_tmp['sort_by'][$assetid] = $sort_by_list;

				$GLOBALS['SQ_SYSTEM']->am->forgetAsset($asset, true);

			}//end foreach listing engines

			$this->_removeAttributes($hoarded_attributes);
			if (!parent::_upgrade($current_version)) {
				trigger_localised_error('CMS0039', E_USER_WARNING, '0.3');
				return false;
			}
			$this->_restoreAttributes($hoarded_attributes);

			$to_save = array_get_index($this->_tmp, 'sort_by', Array());
			foreach ($to_save as $assetid => $sort_by_list) {
				$asset = &$GLOBALS['SQ_SYSTEM']->am->getAsset($assetid);

				$asset->setAttrValue('sort_by', $sort_by_list);

				if (!$asset->saveAttributes()) {
					trigger_localised_error('CMS0026', E_USER_WARNING, '0.3');
					return false;
				}

			}//end foreach listing engines

			// delete unneded attributes
			foreach ($fields_to_delete as $type) {
				$this->_deleteAttribute($type);
			}

		}//end if version 0.3

		if (version_compare($current_version, '0.6', '<')) {
			bam('UPGRADING LISTING ENGINE - FROM VERSION '.$current_version.' TO VERSION 0.6');

			$children = $GLOBALS['SQ_SYSTEM']->am->getTypeAssetids($this->getAssetType(), false);
			foreach ($children as $assetid) {
				$asset = &$GLOBALS['SQ_SYSTEM']->am->getAsset($assetid);

				$asset_positions = $asset->attr('asset_positions');
				if (empty($asset_positions)) continue;

				foreach ($asset_positions as $position => $asset_data) {
					if (is_array($asset_data)) {
						// no upgrade necessary
						continue 2;
					}
				}

				$asset_types = $GLOBALS['SQ_SYSTEM']->am->getAssetInfo(array_values($asset_positions), 'asset', false, 'type_code');
				foreach ($asset_positions as $position => $id) {
					$asset_positions[$position] = Array('id'=>$id, 'type'=>$asset_types[$id]);
				}
				ksort($asset_positions, SORT_NUMERIC);

				echo 'Upgrading asset: '.$asset->name.' ('.$assetid.')'."\n";

				if (!$asset->setAttrValue('asset_positions', $asset_positions)) {
					trigger_localised_error('CMS0039', E_USER_WARNING, '0.6');
					return false;
				}
			}
		}//end if version < 0.6

		if (version_compare($current_version, '0.6.1', '<')) {
			// Adding the group formats folder to all the existing listings didn't happen in the 0.6 update
			bam('UPGRADING LISTING ENGINE - FROM VERSION '.$current_version.' TO VERSION 0.6.1');

			$upgraded_count = 0;
			$GLOBALS['SQ_SYSTEM']->am->includeAsset('folder');

			$children = $GLOBALS['SQ_SYSTEM']->am->getTypeAssetids($this->getAssetType(), false, true);
			foreach ($children as $assetid => $type_code) {
				$asset = &$GLOBALS['SQ_SYSTEM']->am->getAsset($assetid, $type_code);

				$group_formats = &$asset->getFolder('group_formats');

				// folder alread exists...nothing for us to do.
				if (is_null($group_formats)) {

					echo 'Upgrading asset: '.$asset->name.' ('.$assetid.')'."\n";

					// Group Format
					$group_formats =& new Folder();
					$create_link = Array(
									'asset'			=> &$asset,
									'value'			=> 'group_formats',
									'link_type'		=> SQ_LINK_TYPE_2,
									'is_dependant'	=> 1,
									'is_exclusive'	=> 1,
									'sort_order'	=> 2,
								   );

					$group_formats->setAttrValue('name', 'Group Formats');
					if (!$group_formats->create($create_link)) {
						trigger_localised_error('CMS0052', E_USER_WARNING, $asset->name, $asset->id);
					}

					$upgraded_count++;

				}// end if

				$GLOBALS['SQ_SYSTEM']->am->forgetAsset($group_formats);
				unset($group_formats);

				$GLOBALS['SQ_SYSTEM']->am->forgetAsset($asset);
				unset($asset);


			}//end foreach

			bam('Upgraded '.$upgraded_count.' Listing Engine-based assets to version 0.6.1');

		}//end if version < 0.6.1

		return parent::_upgrade($current_version);

	}//end _upgrade()


	function _hoardAttribute($attr_name, $from)
	{
		if (!$GLOBALS['SQ_SYSTEM']->am->installed($from)) {
			return Array();
		}

		$db = &$GLOBALS['SQ_SYSTEM']->db;

		$sql = 'SELECT * FROM sq_ast_attr WHERE name = '.$db->quote($attr_name).' AND owning_type_code = '.$db->quote($from);

		$result = $db->getAll($sql);
		assert_valid_db_result($result);

		return $result;

	}//end _hoardAttribute()


	function _removeAttributes($attrs)
	{
		if (empty($attrs)) return true;
		$db = &$GLOBALS['SQ_SYSTEM']->db;

		$attrs_quoted = Array();
		foreach ($attrs as $attr => $detail) {
			$attrs_quoted[] = $db->quote($attr);
		}
		$GLOBALS['SQ_SYSTEM']->doTransaction('BEGIN');
		$sql = 'DELETE FROM sq_ast_attr WHERE attrid IN ('.implode(', ', $attrs_quoted).')';

		$result = $db->query($sql);
		assert_valid_db_result($result);

		$GLOBALS['SQ_SYSTEM']->doTransaction('COMMIT');

		return true;

	}//end _removeAttributes()


	function _restoreAttributes($attrs)
	{
		if (empty($attrs)) return true;

		$db = &$GLOBALS['SQ_SYSTEM']->db;

		foreach ($attrs as $attrid => $attr) {
			$fields_quoted = Array();
			$keys_quoted = Array();

			foreach ($attr as $field_name => $field) {
				$keys_quoted[$field_name] = $db->quoteIdentifier($field_name);

				if (in_array($field_name, Array('owning_type_code', 'default_type_code', 'parameters_type_code'))) {
					$fields_quoted[$field_name] = $db->quoteSmart($this->getAssetType());
				} else if (in_array($field_name, Array('parameters', 'default', 'type'))) {
					$fields_quoted[$field_name] = $db->quoteSmart($this->vars[$attr['name']][$field_name]);
				} else {
					$fields_quoted[$field_name] = $db->quoteSmart($field);
				}
			}

			$keys   = implode(',', $keys_quoted);
			$fields = implode(',', $fields_quoted);

			$GLOBALS['SQ_SYSTEM']->doTransaction('BEGIN');

			$sql = 'INSERT INTO sq_ast_attr ('.$keys.') VALUES ('.$fields.')';

			$result = $db->query($sql);
			assert_valid_db_result($result);

			$sql = '
				UPDATE sq_ast_attr
				SET
					owning_type_code = '.$db->quote($this->getAssetType()).'
				WHERE
					name = '.$db->quote($attr['name']).'
				AND owning_type_code IN (SELECT type_code FROM sq_ast_typ_inhd WHERE inhd_type_code = '.$db->quote($attr['owning_type_code']).')';
			$result = $db->query($sql);
			assert_valid_db_result($result);

			$GLOBALS['SQ_SYSTEM']->doTransaction('COMMIT');
		}

		return true;

	}//end _restoreAttributes()


}//end class

?>
