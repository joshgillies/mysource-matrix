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
* $Id: listing_engine_management.inc,v 1.11 2005/02/16 00:10:48 lwright Exp $
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
* @version $Revision: 1.11 $
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
		
		$this->vars['types']		= Array(
										'added'   => '0.1',
										'type'    => 'serialise',
										'default' => Array(),
										);
		
		$this->vars['sort_by']		= Array(
										'added'	=> '0.1',
										'type'	=> 'selection',
										'parameters' => Array(
											'options' => Array(
												''=> '-- No Sorting --',
												'assetid'=> 'ID',
												'version'=> 'Version',
												'name'=> 'Name',
												'short_name'=> 'Short Name',
												'created'=> 'Created Date',
												'updated'=> 'Last Updated Date',
												'published'=> 'Last Published Date',
												'__attr__'=> 'Asset Attributes...',
											),
											'multiple' => false,
											'allow_empty' => false,
										),
										'default' => 'name'
										);
		
		$this->vars['sort_by_attr'] = Array(
										'added'   => '0.1',
										'type'    => 'serialise',
										'default' => Array(),
										);
		
		$this->vars['reverse_sort']	= Array(
										'added'     => '0.1',
										'type'      => 'boolean',
										'default'   => false,
										'parameters' => Array(
															'allow_empty' => false
														),
										);
		$this->vars['subs_only']	= Array(
										'added'     => '0.1',
										'type'      => 'boolean',
										'default'   => false,
										'parameters' => Array(
															'allow_empty' => false
														),
										);

		$this->vars['direction']	= Array(
										'added'	=> '0.1',
										'type'	=> 'selection',
										'parameters' => Array(
											'options' => Array(
												'up'	=> 'Up the tree',
												'down'	=> 'Down the tree',
											),
											'multiple' => false,
											'allow_empty' => false,
										),
										'default' => 'down'
										);

		$this->vars['group_by']		= Array(
										'added'	=> '0.1',
										'type'	=> 'selection',
										'parameters' => Array(
											'options' => Array(
												'number'		=> 'Standard',
												'letter'		=> 'A-Z listing',
												'random'		=> 'Random',
											),
											'multiple' => false,
											'allow_empty' => false,
										),
										'default' => 'number'
										);
										
		$this->vars['num_per_page']	= Array(
										'added'   => '0.1',
										'type'    => 'int',
										'default' => 0,
										);

		$this->vars['column_layout']	= Array(
										'added'   => '0.1',
										'type'    => 'serialise',
										'default' => Array('0' => '1'),
										);
										
		$this->vars['asset_positions']	= Array(
										'added'   => '0.1',
										'type'    => 'serialise',
										'default' => Array(),
										);

		$this->vars['lineage_seperator'] = Array (
										'added'			=> '0.1',
										'type'			=> 'text',
										'default'		=> ' > ',
										'description'	=> 'Seperator to use when printing the lineage to a search result asset.',
										);
			
	}//end Listing_Engine_Management()


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
		
		$db =& $GLOBALS['SQ_SYSTEM']->db;
		
		$attributes = Array();
		
		foreach($this->vars as $attr_name => $attr_info) {
			if (in_array($attr_name, $search_page_fields)) {
				$result = $this->_hoardAttribute($attr_name, 'search_page');
			} else {
				$result = $this->_hoardAttribute($attr_name, 'page_asset_listing');
			}
			
			foreach($result as $result_info) {
				$attributes[$result_info['attrid']] = $result_info;
			}
		}
		
		$this->_removeAttributes($attributes);
			
		if (!parent::_install()) {
			trigger_error('Installation of Listing Engine Failed', E_USER_WARNING);
			$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');	
			return false;
		}
		
		$this->_restoreAttributes($attributes);
		$GLOBALS['SQ_SYSTEM']->doTransaction('COMMIT');		
		
		return true;
		
	}//end _install()
	
	
	function _upgrade($current_version)
	{
		$search_page_fields = Array('lineage_seperator');
		
		$db =& $GLOBALS['SQ_SYSTEM']->db;
		
		
		if (version_compare($current_version,'0.1','==')) {
			bam('UPGRADING LISTING ENGINE - FROM VERSION '.$current_version);
		
			$attributes = Array();
		
			foreach($this->vars as $attr_name => $attr_info) {
				if (in_array($attr_name, $search_page_fields)) {
					$result = $this->_hoardAttribute($attr_name, 'search_page');
				} else {
					$result = $this->_hoardAttribute($attr_name, 'page_asset_listing');
				}
				
				foreach($result as $resultid => $result_info) {
					$attributes[$resultid] = $result_info;
				}
			}
			
			$this->_removeAttributes($attributes);
			if (!parent::_upgrade($current_version)) {
				trigger_error('Failed upgrading listing engine (from 0.1)');
				return false;
			}
			$this->_restoreAttributes($attributes);
			
			bam('LISTING ENGINE UPGRADE COMPLETE - FROM VERSION '.$current_version);
		}
	
		return true;
		
	}//end _upgrade()
	
	
	function _hoardAttribute($attr_name, $from)
	{
		if (!$GLOBALS['SQ_SYSTEM']->am->installed($from)) return Array();
		
		$db =& $GLOBALS['SQ_SYSTEM']->db;
		
		$sql = 'SELECT * FROM '.SQ_TABLE_RUNNING_PREFIX.'ast_attr WHERE name = '.$db->quote($attr_name).' AND owning_type_code = '.$db->quote($from);
		
		$result = $db->getAll($sql);
		assert_valid_db_result($result);
		
		return $result;
		
	}//end _transferAttribute()
	
	
	function _removeAttributes($attrs)
	{
		if (empty($attrs)) return true;
		
		$db =& $GLOBALS['SQ_SYSTEM']->db;
		
		$attrs_quoted = Array();
		foreach($attrs as $attr => $detail) {
			$attrs_quoted[] = $db->quote($attr); 
		}
		
		$sql = 'DELETE FROM '.SQ_TABLE_RUNNING_PREFIX.'ast_attr WHERE attrid IN ('.implode(', ', $attrs_quoted).')';
		
		$result = $db->query($sql);
		assert_valid_db_result($result);
		
		return true;
		
	}//end _transferAttribute()
	
	
	function _restoreAttributes($attrs)
	{
		if (empty($attrs)) return true;
		
		$db =& $GLOBALS['SQ_SYSTEM']->db;
		
		foreach($attrs as $attrid => $attr) {
			$sql = 'INSERT INTO '.SQ_TABLE_RUNNING_PREFIX.'ast_attr ('.implode(',', array_keys($attr)).') VALUES (';
			
			$fields_quoted = Array();
			foreach($attr as $field_name => $field) {
				if (in_array($field_name, Array('owning_type_code', 'default_type_code', 'parameters_type_code'))) {
					$fields_quoted[$field_name] = $db->quote($this->getAssetType());
				} else if (in_array($field_name, Array('parameters', 'default', 'type'))) {
					$fields_quoted[$field_name] = $db->quote($this->vars[$attr['name']][$field_name]);
				} else {
					$fields_quoted[$field_name] = $db->quote($field);
				}
			}
			
			$sql .= implode(', ', $fields_quoted).')';
			$result = $db->query($sql);
			assert_valid_db_result($result);
			
			/*$sql = 'UPDATE '.SQ_TABLE_RUNNING_PREFIX.'ast_attr set
			owning_type_code = '.$db->quote($this->getAssetType()).', 
			default_type_code = '.$db->quote($this->getAssetType()).', 
			parameters_type_code = '.$db->quote($this->getAssetType()).' 
			parameters = '.$db->quote($this->getAssetType()).' 
			default = '.$db->quote($this->getAssetType()).' 
			WHERE name = '.$db->quote($attr['name']).' AND owning_type_code = '.$db->quote($attr['owning_type_code']);
			$result = $db->query($sql);
			assert_valid_db_result($result);*/
		}
				
		return true;
		
	}//end _transferAttribute()
	
}//end class

?>
