<?php

require_once SQ_CORE_PACKAGE_PATH.'/designs/design_areas/menu/design_area_menu_recursive/design_area_menu_recursive.inc';

/**
* Design_Area_Menu_Stalks
*
* Purpose
*
*
* @author  Blair Robertson <blair@squiz.net>
* @version $Version$ - 1.0
* @package MySource_Matrix_Packages
* @subpackage __core__
*/
class Design_Area_Menu_Stalks extends Design_Area_Menu_Recursive
{
	/**
	* The width of the arrow image
	* @var int
	*/
	var $arrow_width  = 4;

	/**
	* The height of the arrow image
	* @var int
	*/
	var $arrow_height = 6;


	/**
	* Set the value for the passed variable to the passed value
	*
	* @param string		$name				the name of the attribute
	* @param string		$value				the new value of the attribute
	*
	* @return boolean
	* @access public
	*/
	function setAttrValue($name, $value)
	{
		// make sure the aligment is lower case
		switch($name) { 
			case 'alignment' :
				$value = strtolower($value);
				break;
			
			case 'min_height' : 
				if ($value <= 0) {
					trigger_error('The "'.$name.'" attribute must be set to a number greater than zero');
					return false;
				}
				break;
		}// end switch
		return parent::setAttrValue($name, $value);
	}//end setAttrValue()


	/**
	* Prints the recursive fn that will be used by this menu to printout its details
	* NOTE: to be overridden by our children
	*
	* @param object Design	$design			the design for which we are currently printing
	*
	* @access public
	*/
	function printRecursiveFn(&$design)
	{

		$use_blanks  = !$this->attr('show_stalks');
		$cell_class = '';
		if ($this->settingExists('class')) { 
			ob_start();
			$this->printVar('settings.class', Array());
			$cell_class = ' class="'.ob_get_contents().'"';
			ob_end_clean();
		}// end if


		ob_start();
		echo '
				<td'.$cell_class.' valign="top"><img src="';
		if ($use_blanks) {
			echo '<', '?php $blank_src; ?', '>';
		} else {
			echo '<', '?php echo (($'.$this->attr('id_name').'_current_setting == \'current\') ? $stalk_prefix.\'arrow.png\' : $blank_src); ?', '>';
			
		}
		echo '" width="'.$this->arrow_width.'" height="'.$this->attr('min_height').'"></td>';
		$arrow_contents = ob_get_contents();
		ob_end_clean();



		ob_start();
		if ($page_contents = $this->attr('page_contents')) {
			$edit_fns = $this->getEditFns();
			if (!$edit_fns->_outputContents($page_contents, $this, $design)) {
				return false;
			}
		}//	end if
		$page_contents = '
				<td'.$cell_class.(($this->attr('word_wrap')) ? '' : ' nowrap').' width="100%" valign="top" align="'.$this->attr('alignment').'">'.ob_get_contents().'</td>
		';
		ob_end_clean();

		// if we are using ellipsis, then we need to include general/general.inc
		if ($this->attr('ellipsis_length') > 0) { 
			echo 'require_once SQ_FUDGE_PATH.\'/general/general.inc\';';
		}
		
		echo '

		function '.$this->attr('id_name').'_recursive_fn($level, $parentid, $'.$this->attr('id_name').'_base_url, $depth=Array())
		{
			global $RECURSIVE_MENU_PAGE_LINEAGE;
			static $'.$this->attr('id_name').'_settings = '.var_export($this->_getSettingsArray(), true).';

			$blank_src    = sq_web_path(\'lib\').\'/web/images/blank.gif\';
			$stalk_prefix = sq_web_path(\'data\').\'/assets/'.$this->type().'/'.$this->id.'/\';

			// The indenting level, stalk on/off
			$depth[] = true; 
			$depth_count = count($depth);

			$'.$this->attr('id_name').'_lineage_assetid = (empty($RECURSIVE_MENU_PAGE_LINEAGE[$level + 1])) ? 0 : $RECURSIVE_MENU_PAGE_LINEAGE[$level + 1][\'assetid\'];
			$'.$this->attr('id_name').'_lineage_setting = (count($RECURSIVE_MENU_PAGE_LINEAGE) - 1 == $level + 1) ? \'current\' : \'hierarchy\';
			$'.$this->attr('id_name').'_urls = $GLOBALS[\'SQ_SYSTEM\']->am->getAssetHref(array_keys($GLOBALS[\'MENU_DATA\'][$parentid]), $'.$this->attr('id_name').'_base_url);

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
			<div align="'.$this->attr('alignment').'">
			<table cellpadding="0" cellspacing="0" border="0" width="100%">
				<tr>
			';

		if ($this->attr('alignment') == 'right') echo $page_contents;

		if ($this->attr('indent')) {

			if ($this->attr('alignment') == 'right') echo $arrow_contents;

			echo '
			<', '?php
			';

			// depending on the alignment differs how we loop through the array
			if ($this->attr('alignment') == 'left') {
				echo '
				for ($j = 0; $j < $depth_count; $j++) {
				';
			} else {
				echo '
				for ($j = $depth_count - 1; $j >= 0; $j--) {
				';
			}

			echo '?', '> 
					<td'.$cell_class.' valign="top"';

					if ($use_blanks) {
						echo '><img src="<', '?php $blank_src; ?', '>" width="'.$this->attr('indent').'" height="'.$this->attr('min_height').'"></td>';
					} else {
						// if there is the need to a stalk
						// and if we are at the end of a sub listing and 
						// not it the indent closest to the text (which will have the end branch)
						echo '<', '?php
						if ($depth[$j] && !($end && ($j == $depth_count - 1)))
							echo \' background="\', $stalk_prefix, \'stalk.png"\';

						?', '>><img src="<', '?php
						if ($j != $depth_count - 1) { 
							echo $blank_src;
						} elseif ($end) { 
							echo $stalk_prefix, \'endbranch.png"\';
						} else {
							echo $stalk_prefix, \'branch.png"\';
						}

						?', '>" width="'.$this->attr('indent').'" height="'.$this->attr('min_height').'"></td>';

					}// end if $use_blanks
		echo '
				<', '?php
				}// end for depth count

			?', '>';

			if ($this->attr('alignment') == 'left') echo $arrow_contents;

		}// end indent

		if ($this->attr('alignment') == 'left') echo $page_contents;

		echo '

					</tr>
				</table>
				</div>
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
					if ($end) $depth[$depth_count - 1] = false;
					'.$this->attr('id_name').'_recursive_fn($level + 1, $'.$this->attr('id_name').'_row[\'assetid\'], $'.$this->attr('id_name').'_urls[$'.$this->attr('id_name').'_row[\'assetid\']], $depth);
				} // end if

			}// end for
			?', '>
			</ul>
			<', '?php

		}// end '.$this->attr('id_name').'_recursive_fn()

		';

	} // end printRecursiveFn()


	/**
	* Outputs either the straight HTML or PHP code that is needed for this design area to be represented
	*
	* @param object Design	$design the design for which we are currently printing
	*
	* @return boolean
	* @access public
	*/
	function printArea(&$design)
	{
		if ($this->attr('show_stalks') && $this->attr('indent') && $this->attr('min_height')) {
			$edit_fns = $this->getEditFns();
			if (!$edit_fns->_createStalkImages($this)) return false;
		}

		return parent::printArea($design);

	}//end printArea()


}//end class
?>
