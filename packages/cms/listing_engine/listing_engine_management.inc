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
* $Id: listing_engine_management.inc,v 1.2 2005/02/11 03:34:39 lwright Exp $
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
* @version $Revision: 1.2 $
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
										'type'	=> 'text',
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
		$search_page_fields = Array('lineage_seperator');
		
		$db =& $GLOBALS['SQ_SYSTEM']->db;
		if (!parent::_install()) return false;
		
		// asset listing isn't here; we don't need to go through this rigmarole
		if (!$GLOBALS['SQ_SYSTEM']->am->installed('page_asset_listing')) return true;
		
		bam('INSTALL LISTING ENGINE');
		
		$attr_names = Array();
		$search_attr_names = Array();
		foreach($this->vars as $attr_name => $attr_info) {
			if (in_array($attr_name, $search_page_fields)) {
				$search_attr_names[] = $db->quote($attr_name);
			} else {
				$attr_names[] = $db->quote($attr_name);
			}
		}
		
		$sql = 'UPDATE '.SQ_TABLE_RUNNING_PREFIX.'ast_attr SET owning_type_code = '.$db->quote($this->getAssetType()).', parameters_type_code = '.$db->quote($this->getAssetType()).' WHERE name IN ('.implode(', ', $attr_names).') AND owning_type_code = '.$db->quote('page_asset_listing');
		
		if (!$db->query($sql)) {
			trigger_error('Unable to complete post-install script - ownership of attributes could not be transferred from asset listing');
			return false;
		}
		
		$sql = 'UPDATE '.SQ_TABLE_RUNNING_PREFIX.'ast_attr SET owning_type_code = '.$db->quote($this->getAssetType()).', parameters_type_code = '.$db->quote($this->getAssetType()).' WHERE name IN ('.implode(', ', $search_attr_names).') AND owning_type_code = '.$db->quote('search_page');
		
		if (!$db->query($sql)) {
			trigger_error('Unable to complete post-install script - ownership of attributes could not be transferred from search page');
			return false;
		}
		
		return true;
		
	}//end _upgrade()
	
	
}//end class

?>
