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
* $Id: design_area_menu_recursive.inc,v 1.30 2012/08/30 01:09:06 ewang Exp $
*
*/


require_once SQ_CORE_PACKAGE_PATH.'/designs/design_areas/menu/design_area_menu_type/design_area_menu_type.inc';

/**
* Design_Area_Menu_Recursive
*
* Purpose
*
*
* @author  Blair Robertson <blair@squiz.net>
* @version $Revision: 1.30 $
* @package MySource_Matrix_Packages
* @subpackage __core__
*/
class Design_Area_Menu_Recursive extends Design_Area_Menu_Type
{


	/**
	* Constructor
	*
	* @param string	$assetid	The asset ID to load
	*
	*/
	function __construct($assetid=0)
	{
		parent::__construct($assetid);

	}//end constructor


	/**
	* Performs any outputting needed to by all design areas of this menu type
	* NOTE: assumes PHP tags are open
	*
	* @return void
	* @access public
	*/
	public function printStaticInitOutput()
	{
		parent::printStaticInitOutput();

		// Stuff that is only needed once per design file
		if (empty($GLOBALS['SQ_'.__CLASS__.__FUNCTION__.'_RUN_'.$GLOBALS['DESIGN_BEING_GENERATED']->id])) {
			echo '
			// because we need to be able to get to this with the recurse fn
			$GLOBALS[\'RECURSIVE_MENU_ASSET_LINEAGE\'] = &$ASSET_LINEAGE;
			';
			$GLOBALS['SQ_'.__CLASS__.__FUNCTION__.'_RUN_'.$GLOBALS['DESIGN_BEING_GENERATED']->id] = TRUE;
		}

	}//end printStaticInitOutput()


	/**
	* Performs any outputting needed to intialise this instance of the menu type
	*
	* @return void
	* @access public
	*/
	public function printInitOutput()
	{
		parent::printInitOutput();
		$this->printRecursiveFn();

	}//end printInitOutput()


	/**
	* Prints the recursive fn that will be used by this menu to printout its details
	* NOTE: to be overridden by our children, this can be used as an example of how to recurse
	*
	* @return void
	* @access public
	*/
	public function printRecursiveFn()
	{
		echo '

		function '.$this->attr('id_name').'_recursive_fn($level, $parentid, $'.$this->attr('id_name').'_base_url)
		{
			global $RECURSIVE_MENU_ASSET_LINEAGE;
			static $'.$this->attr('id_name').'_settings = '.var_export($this->_getSettingsArray(), TRUE).';

			$'.$this->attr('id_name').'_lineage_assetid = (empty($RECURSIVE_MENU_ASSET_LINEAGE[$level + 1])) ? 0 : $RECURSIVE_MENU_ASSET_LINEAGE[$level + 1][\'assetid\'];
			$'.$this->attr('id_name').'_lineage_setting = (count($RECURSIVE_MENU_ASSET_LINEAGE) - 1 == $level + 1) ? \'current\' : \'hierarchy\';
			$'.$this->attr('id_name').'_urls = $GLOBALS[\'SQ_SYSTEM\']->am->getAssetURL(array_keys($GLOBALS[\'MENU_DATA\'][\''.$this->attr('id_name').'\'][$parentid]), $'.$this->attr('id_name').'_base_url);

			?', '>
			<ul>
			<', '?php
			$num_kids = count($GLOBALS[\'MENU_DATA\'][\''.$this->attr('id_name').'\'][$parentid]);
			$i = 0;
			foreach ($GLOBALS[\'MENU_DATA\'][\''.$this->attr('id_name').'\'][$parentid] as $'.$this->attr('id_name').'_row) {
				$i++;
				$end = ($i == $num_kids);
				// if not live, we have to get the proper object, sorry
				if (((int) $'.$this->attr('id_name').'_row[\'status\'] & SQ_STATUS_LIVE) == 0) {
					$'.$this->attr('id_name').'_obj = $GLOBALS[\'SQ_SYSTEM\']->am->getAsset($'.$this->attr('id_name').'_row[\'assetid\'], $'.$this->attr('id_name').'_row[\'type_code\']);
				} else {
					unset($'.$this->attr('id_name').'_obj);
				}
				$'.$this->attr('id_name').'_current_setting = ($'.$this->attr('id_name').'_lineage_assetid == $'.$this->attr('id_name').'_row[\'assetid\']) ? $'.$this->attr('id_name').'_lineage_setting : \'normal\';
#				pre_echo("CURRENT SETTING : ".$'.$this->attr('id_name').'_current_setting);

			?', '>
			';
		if ($asset_contents = $this->attr('asset_contents')) {
			$this->_tmp['outputting_asset_section'] = TRUE;
			foreach ($asset_contents as $item) {
				$this->_printContentItemCode($item);
			}
			unset($this->_tmp['outputting_asset_section']);
		}//end if
		echo '
			<', '?php
		';

		// if we are to show a minimum number of levels, add an extra bit to the condition

		$recurse_condition = '$'.$this->attr('id_name').'_current_setting != \'normal\'';


		if ($this->attr('min_num_levels') > 0) {
			$recurse_condition = '$level < '.($this->attr('min_num_levels') + $this->attr('level') - 1).' || '.$recurse_condition;
		}

		if ($this->attr('max_num_levels') > 0) {
			$recurse_condition = '$level < '.($this->attr('max_num_levels') + $this->attr('level') - 1).' && ('.$recurse_condition.')';
		}
		echo '
				if (('.$recurse_condition.') && !empty($GLOBALS[\'MENU_DATA\'][\''.$this->attr('id_name').'\'][$'.$this->attr('id_name').'_row[\'assetid\']])) {
					'.$this->attr('id_name').'_recursive_fn($level + 1, $'.$this->attr('id_name').'_row[\'assetid\'], $'.$this->attr('id_name').'_urls[$'.$this->attr('id_name').'_row[\'assetid\']]);
				} // end if

			}//end for
			?', '>
			</ul>
			<', '?php

		}//end '.$this->attr('id_name').'_recursive_fn()

		';

	}//end printRecursiveFn()


	/**
	* Outputs the way for this menu type to get their information and place it in the
	* $GLOBALS['MENU_DATA'] array
	*
	* @return void
	* @access public
	*/
	public function printDataCollection()
	{
		$parent_menu = $this->getParentMenu();
		$ignore_permissions = $this->attr('ignore_permissions') ? ' , TRUE' : ' , FALSE';
		
		echo '
		$'.$this->attr('id_name').'_num_repeats = count($ASSET_LINEAGE) - $'.$this->attr('id_name').'_level;
		';

		if ($this->attr('min_num_levels') > 0) {
			echo '
		if ($'.$this->attr('id_name').'_num_repeats < '.((int) $this->attr('min_num_levels')).') $'.$this->attr('id_name').'_num_repeats = '.((int) $this->attr('min_num_levels')).';
			';
		}
		if ($this->attr('max_num_levels') > 0) {
			echo '
		if ($'.$this->attr('id_name').'_num_repeats > '.((int) $this->attr('max_num_levels')).') $'.$this->attr('id_name').'_num_repeats = '.((int) $this->attr('max_num_levels')).';
			';
		}

		// if we aren't a sub menu, or if our parent is only showing subs if we are in the lineage,
		// we just need a blank assetids array for now
		if (is_null($parent_menu) || $parent_menu->attr('show_subs') == 'on_current') {
			echo '
		$'.$this->attr('id_name').'_assetids = Array();';
		// otherwise our parent wants all kids printed for everything so grab its assetids
		} else {
			echo '
		$'.$this->attr('id_name').'_assetids = $'.$parent_menu->attr('id_name').'_assetids;';
		}//end if

		echo '
		for ($'.$this->attr('id_name').'_i = 0; $'.$this->attr('id_name').'_i < $'.$this->attr('id_name').'_num_repeats; $'.$this->attr('id_name').'_i++) {

		';

		// if there is a minimum number of levels, then display everything for all those levels
		// and only the current lineage thereafter
		if ($this->attr('min_num_levels') > 0) {
			// if we have no parent, or they only want us to display on current,
			// then for the first round we need to read from the asset lineage
			if (is_null($parent_menu) || $parent_menu->attr('show_subs') == 'on_current') {
				$min_levels_extra_cond = '$'.$this->attr('id_name').'_i > 0 &&';
			} else {
				$min_levels_extra_cond = '';
			}

			echo '
			if ('.$min_levels_extra_cond.' $'.$this->attr('id_name').'_i < '.((int) $this->attr('min_num_levels')).') {
				$'.$this->attr('id_name').'_assetids_cond = $'.$this->attr('id_name').'_assetids;
			} else {
				$'.$this->attr('id_name').'_assetids_cond = Array((int) $ASSET_LINEAGE[$'.$this->attr('id_name').'_level + $'.$this->attr('id_name').'_i][\'assetid\']);
			}
			';
			$majorids = '$'.$this->attr('id_name').'_assetids_cond';
		} else {
			$majorids = 'Array((int) $ASSET_LINEAGE[$'.$this->attr('id_name').'_level + $'.$this->attr('id_name').'_i][\'assetid\'])';
		}

		echo '
			unset($'.$this->attr('id_name').'_assetids);
			$'.$this->attr('id_name').'_assetids = menu_get_assets(\''.$this->attr('id_name').'\', '.$majorids.' '.$ignore_permissions.');

		}//end for '.$this->attr('id_name').'_i

		';

	}//end printDataCollection()


	/**
	* Outputs the var either as straight HTML or PHP code, depending on the type
	*
	* @param string	$var			the name of the var to print out
	* @param array	$args			array of any extra arguments that existed in the print command
	* @param string	$value_prefix	PHP code to wrap put before the value before outputting it
	* @param string	$value_suffix	PHP code to wrap put after  the value before outputting it
	*
	* @return void
	* @access public
	*/
	public function printVar($var, Array $args, $value_prefix='', $value_suffix='')
	{
		if ($var == 'asset_short_name' && $this->attr('ellipsis_length') > 0) {
			$value_prefix = 'ellipsisize('.$value_prefix;
			$value_suffix = $value_suffix.', '.$this->attr('ellipsis_length').' - (2 * $depth_count))';
		}//end if

		parent::printVar($var, $args, $value_prefix, $value_suffix);

	}//end printVar()


	/**
	* Print code that outputs a given element from this DA's contents
	*
	* @param array	$element	An entry from our contents array
	*
	* @return void
	* @access private
	*/
	protected function _printContentItemCode(Array $element)
	{
		switch (array_get_index($element, 'operation')) {
			case 'asset' :

				echo '<', '?php
				';

				$parent_menu = $this->getParentMenu();
				// if we aren't a sub menu
				if (is_null($parent_menu)) {
					$parent_assetid_name = '$ASSET_LINEAGE[$'.$this->attr('id_name').'_level][\'assetid\']';

					echo '
				$'.$this->attr('id_name').'_base_url = (empty($ASSET_LINEAGE[$'.$this->attr('id_name').'_level])) ? \'\' : current_protocol().\'://\'.$ASSET_LINEAGE[$'.$this->attr('id_name').'_level][\'url\'];
					';

				// else  we are a sub menu
				} else {
					$parent_assetid_name = '$'.$parent_menu->attr('id_name').'_row[\'assetid\']';

					echo '
				$'.$this->attr('id_name').'_base_url = $'.$parent_menu->attr('id_name').'_urls[$'.$parent_menu->attr('id_name').'_row[\'assetid\']];
					';

				}//end if

				echo '
				'.$this->attr('id_name').'_recursive_fn($'.$this->attr('id_name').'_level, '.$parent_assetid_name.', $'.$this->attr('id_name').'_base_url);
				?', '>';

				return TRUE;


			case 'sub' :
				// there ain't going to be any subs :)
				return TRUE;

			default :
				return parent::_printContentItemCode($element);
		}//end switch

	}//end _printContentItemCode()


}//end class
?>
