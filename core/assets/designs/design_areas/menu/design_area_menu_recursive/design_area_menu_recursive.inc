<?php
/**
* Copyright (c) 2003 - Squiz Pty Ltd
*
* $Id: design_area_menu_recursive.inc,v 1.13 2003/09/30 00:27:18 gsherwood Exp $
* $Name: not supported by cvs2svn $
*/


require_once SQ_CORE_PACKAGE_PATH.'/designs/design_areas/menu/design_area_menu_type/design_area_menu_type.inc';

/**
* Design_Area_Menu_Recursive
*
* Purpose
*
*
* @author  Blair Robertson <blair@squiz.net>
* @version $Version$ - 1.0
* @package MySource_Matrix_Packages
* @subpackage __core__
*/
class Design_Area_Menu_Recursive extends Design_Area_Menu_Type
{

	/**
	* Performs any outputting needed to by all design areas of this menu type
	* NOTE: assumes PHP tags are open
	*
	* @param object Design	$design			the design for which we are currently printing
	*
	* @access public
	*/
	function printStaticInitOutput(&$design)
	{
		parent::printStaticInitOutput($design);

		// Stuff that is only needed once per design file
		if (empty($GLOBALS['SQ_'.__CLASS__.__FUNCTION__.'_RUN_'.$design->id])) {
			echo '
			// because we need to be able to get to this with the recurse fn
			$GLOBALS[\'RECURSIVE_MENU_PAGE_LINEAGE\'] = &$PAGE_LINEAGE;
			';
			$GLOBALS['SQ_'.__CLASS__.__FUNCTION__.'_RUN_'.$design->id] = true;
		}

	} // printStaticInitOutput()


	/**
	* Performs any outputting needed to intialise this instance of the menu type
	*
	* @param object Design	$design			the design for which we are currently printing
	*
	* @access public
	*/
	function printInitOutput(&$design)
	{
		parent::printInitOutput($design);
		$this->printRecursiveFn($design);
	}// end printInitOutput()


	/**
	* Prints the recursive fn that will be used by this menu to printout its details
	* NOTE: to be overridden by our children, this can be used as an example of how to recurse
	*
	* @param object Design	$design			the design for which we are currently printing
	*
	* @access public
	*/
	function printRecursiveFn(&$design)
	{
		echo '

		function '.$this->attr('id_name').'_recursive_fn($level, $parentid, $'.$this->attr('id_name').'_base_url)
		{
			global $RECURSIVE_MENU_PAGE_LINEAGE;
			static $'.$this->attr('id_name').'_settings = '.var_export($this->_getSettingsArray(), true).';

			$'.$this->attr('id_name').'_lineage_assetid = (empty($RECURSIVE_MENU_PAGE_LINEAGE[$level + 1])) ? 0 : $RECURSIVE_MENU_PAGE_LINEAGE[$level + 1][\'assetid\'];
			$'.$this->attr('id_name').'_lineage_setting = (count($RECURSIVE_MENU_PAGE_LINEAGE) - 1 == $level + 1) ? \'current\' : \'hierarchy\';
			$'.$this->attr('id_name').'_urls = $GLOBALS[\'SQ_SYSTEM\']->am->getAssetHref(array_keys($GLOBALS[\'MENU_DATA\'][$parentid]), $'.$this->attr('id_name').'_base_url);

			?', '>
			<ul>
			<', '?php
#			pre_echo("Lineage Setting : $'.$this->attr('id_name').'_lineage_setting");
#			pre_echo("Lineage Assetid : $'.$this->attr('id_name').'_lineage_assetid");
			$num_kids = count($GLOBALS[\'MENU_DATA\'][$parentid]);
			$i = 0;
			foreach($GLOBALS[\'MENU_DATA\'][$parentid] as $'.$this->attr('id_name').'_row) {
				$i++;
				$end = ($i == $num_kids);
				// if not live, we have to get the proper object, sorry
				if (((int) $'.$this->attr('id_name').'_row[\'status\'] & SQ_STATUS_LIVE) == 0) {
					$'.$this->attr('id_name').'_obj = &$GLOBALS[\'SQ_SYSTEM\']->am->getAsset($'.$this->attr('id_name').'_row[\'assetid\'], $'.$this->attr('id_name').'_row[\'type_code\']);
				} else {
					unset($'.$this->attr('id_name').'_obj);
				}
				$'.$this->attr('id_name').'_current_setting = ($'.$this->attr('id_name').'_lineage_assetid == $'.$this->attr('id_name').'_row[\'assetid\']) ? $'.$this->attr('id_name').'_lineage_setting : \'normal\';
#				pre_echo("CURRENT SETTING : ".$'.$this->attr('id_name').'_current_setting);

			?', '>
			';
		if ($page_contents = $this->attr('page_contents')) {
			$edit_fns = $this->getEditFns();
			$asset->_tmp['outputting_page_section'] = true;
			if (!$edit_fns->_outputContents($page_contents, $this, $design)) return false;
			unset($asset->_tmp['outputting_page_section']);
		}//	end if
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
				if (('.$recurse_condition.') && !empty($GLOBALS[\'MENU_DATA\'][$'.$this->attr('id_name').'_row[\'assetid\']])) {
					'.$this->attr('id_name').'_recursive_fn($level + 1, $'.$this->attr('id_name').'_row[\'assetid\'], $'.$this->attr('id_name').'_urls[$'.$this->attr('id_name').'_row[\'assetid\']]);
				} // end if

			}// end for
			?', '>
			</ul>
			<', '?php

		}// end '.$this->attr('id_name').'_recursive_fn()

		';

	} // end printRecursiveFn()


	/**
	* Outputs the way for this menu type to get their information and place it in the
	* $GLOBALS['MENU_DATA'] array
	*
	* @param object Design	$design the design for which we are currently printing
	*
	* @access public
	*/
	function printDataCollection(&$design)
	{
		$parent_menu = &$this->getParentMenu();

		echo '
		$'.$this->attr('id_name').'_num_repeats = count($PAGE_LINEAGE) - $'.$this->attr('id_name').'_level;
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
		}// end if

		echo '
#pre_echo("NR:  ".$'.$this->attr('id_name').'_num_repeats);
		for($'.$this->attr('id_name').'_i = 0; $'.$this->attr('id_name').'_i < $'.$this->attr('id_name').'_num_repeats; $'.$this->attr('id_name').'_i++) {

		';

		// if there is a minimum number of levels, then display everything for all those levels
		// and only the current lineage thereafter
		if ($this->attr('min_num_levels') > 0) {
			// if we have no parent, or they only want us to display on current,
			// then for the first round we need to read from the page lineage
			if (is_null($parent_menu) || $parent_menu->attr('show_subs') == 'on_current') {
				$min_levels_extra_cond = '$'.$this->attr('id_name').'_i > 0 &&';
			} else {
				$min_levels_extra_cond = '';
			}

			echo '
			if ('.$min_levels_extra_cond.' $'.$this->attr('id_name').'_i < '.((int) $this->attr('min_num_levels')).') {
				$'.$this->attr('id_name').'_assetids_cond = $'.$this->attr('id_name').'_assetids;
			} else {
				$'.$this->attr('id_name').'_assetids_cond = Array((int) $PAGE_LINEAGE[$'.$this->attr('id_name').'_level + $'.$this->attr('id_name').'_i][\'assetid\']);
			}
			';
			$majorids = '$'.$this->attr('id_name').'_assetids_cond';
		} else {
			$majorids = 'Array((int) $PAGE_LINEAGE[$'.$this->attr('id_name').'_level + $'.$this->attr('id_name').'_i][\'assetid\'])';
		}

		echo '
			unset($'.$this->attr('id_name').'_assetids);
			$'.$this->attr('id_name').'_assetids = menu_get_assets('.$majorids.');

		}// end for '.$this->attr('id_name').'_i

		';

	}//end printDataCollection()


	/*
	* Outputs the var either as straight HTML or PHP code, depending on the type
	*
	* @param string	$var			the name of the var to print out
	* @param Array	$args			array of any extra arguments that existed in the print command
	* @param string	$value_prefix	PHP code to wrap put before the value before outputting it
	* @param string	$value_suffix	PHP code to wrap put after  the value before outputting it
	*
	* @access public
	*/
	function printVar($var, $args, $value_prefix='', $value_suffix='')
	{
		if ($var == 'page_short_name' && $this->attr('ellipsis_length') > 0) {
			$value_prefix = 'ellipsisize('.$value_prefix;
			$value_suffix = $value_suffix.', '.$this->attr('ellipsis_length').' - (2 * $depth_count))';
		}// end if

		parent::printVar($var, $args, $value_prefix, $value_suffix);
	}//end printVar()

}//end class
?>
