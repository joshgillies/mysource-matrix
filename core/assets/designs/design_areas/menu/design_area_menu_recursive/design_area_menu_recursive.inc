<?php

require_once SQ_CORE_PACKAGE_PATH.'/designs/design_areas/menu/design_area_menu_type/design_area_menu_type.inc';

/**
* Design_Area_Menu_Recursive
*
* Purpose
*
*
* @author  Blair Robertson <blair@squiz.net>
* @version $Version$ - 1.0
* @package Resolve_Packages
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

		function '.$this->attr('id_name').'_recursive_fn($level, $parentid, $data, $'.$this->attr('id_name').'_base_url)
		{
			global $RECURSIVE_MENU_PAGE_LINEAGE;
			static $'.$this->attr('id_name').'_settings = '.var_export($this->_getSettingsArray(), true).';

			$'.$this->attr('id_name').'_lineage_assetid = (empty($RECURSIVE_MENU_PAGE_LINEAGE[$level + 1])) ? 0 : $RECURSIVE_MENU_PAGE_LINEAGE[$level + 1][\'assetid\'];
			$'.$this->attr('id_name').'_lineage_setting = (count($RECURSIVE_MENU_PAGE_LINEAGE) - 1 == $level + 1) ? \'current\' : \'hierarchy\';

			?', '>
			<ul>
			<', '?php
#			pre_echo("Lineage Setting : $'.$this->attr('id_name').'_lineage_setting");
#			pre_echo("Lineage Assetid : $'.$this->attr('id_name').'_lineage_assetid");
			for($i = 0, $num_kids = count($data[$parentid]); $i < $num_kids; $i++) {
				$end = ($i == $num_kids - 1);
				$'.$this->attr('id_name').'_row = $data[$parentid][$i];
				$'.$this->attr('id_name').'_current_setting = ($'.$this->attr('id_name').'_lineage_assetid == $'.$this->attr('id_name').'_row[\'assetid\']) ? $'.$this->attr('id_name').'_lineage_setting : \'normal\';
#				pre_echo("CURRENT SETTING : ".$'.$this->attr('id_name').'_current_setting);

			?', '>
			';
		if ($page_contents = $this->attr('page_contents')) {
			$edit_fns = $this->getEditFns();
			if (!$edit_fns->_outputContents($page_contents, $this, $design)) {
				return false;
			}
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
				if (('.$recurse_condition.') && !empty($data[$'.$this->attr('id_name').'_row[\'assetid\']])) {
					'.$this->attr('id_name').'_recursive_fn($level + 1, $'.$this->attr('id_name').'_row[\'assetid\'], $data, $'.$this->attr('id_name').'_base_url.\'/\'.$'.$this->attr('id_name').'_row[\'path\']);
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
	* $'.$this->attr('id_name').'_data array
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
		$'.$this->attr('id_name').'_data = Array();
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
				$'.$this->attr('id_name').'_assetids_cond = \'IN (\'.implode(\',\', $'.$this->attr('id_name').'_assetids).\')\';
			} else {
				$'.$this->attr('id_name').'_assetids_cond = \'= \'.((int) $PAGE_LINEAGE[$'.$this->attr('id_name').'_level + $'.$this->attr('id_name').'_i][\'assetid\']);
			}
			';
			$thisids_cond = '\'.$'.$this->attr('id_name').'_assetids_cond.\'';
		} else {
			$thisids_cond = '= \'.((int) $PAGE_LINEAGE[$'.$this->attr('id_name').'_level + $'.$this->attr('id_name').'_i][\'assetid\']).\'';
		}

		echo '

			$sql = \'SELECT a.assetid, l.majorid, a.type_code, a.name, a.short_name, p.path
					FROM sq_asset a
						INNER JOIN sq_asset_path p on a.assetid = p.assetid
						INNER JOIN sq_asset_link l on a.assetid = l.minorid
					WHERE p.sort_order = 0
					  AND l.majorid '.$thisids_cond.'
					  AND (l.link_type & \'.$db->quote(SQ_LINK_FRONTEND_NAV).\') > 0
					ORDER BY l.majorid, l.sort_order\';
#		pre_echo($sql);
			$'.$this->attr('id_name').'_result = $db->query($sql);
			if (DB::isError($'.$this->attr('id_name').'_result)) {
				trigger_error($'.$this->attr('id_name').'_result->getMessage().\'<br/>\'.$'.$this->attr('id_name').'_result->getUserInfo(), E_USER_ERROR);
			}
			unset($'.$this->attr('id_name').'_assetids);
			$'.$this->attr('id_name').'_assetids = Array();
			while (DB_OK === $'.$this->attr('id_name').'_result->fetchInto($'.$this->attr('id_name').'_row)) {
				$'.$this->attr('id_name').'_assetids[] = (int) $'.$this->attr('id_name').'_row[\'assetid\'];
				if (!isset($'.$this->attr('id_name').'_data[$'.$this->attr('id_name').'_row[\'majorid\']])) $'.$this->attr('id_name').'_data[$'.$this->attr('id_name').'_row[\'majorid\']] = Array();
				$'.$this->attr('id_name').'_data[$'.$this->attr('id_name').'_row[\'majorid\']][] = $'.$this->attr('id_name').'_row;

			}
#				pre_echo($'.$this->attr('id_name').'_assetids);
		}// end for '.$this->attr('id_name').'_i
#pre_echo($'.$this->attr('id_name').'_data);

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
