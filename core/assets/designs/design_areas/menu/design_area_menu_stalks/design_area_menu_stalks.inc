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
* $Id: design_area_menu_stalks.inc,v 1.32 2012/08/30 01:09:06 ewang Exp $
*
*/


require_once SQ_CORE_PACKAGE_PATH.'/designs/design_areas/menu/design_area_menu_recursive/design_area_menu_recursive.inc';

/**
* Design_Area_Menu_Stalks
*
* Purpose
*
*
* @author  Blair Robertson <blair@squiz.net>
* @version $Revision: 1.32 $
* @package MySource_Matrix_Packages
* @subpackage __core__
*/
class Design_Area_Menu_Stalks extends Design_Area_Menu_Recursive
{
	/**
	* The width of the arrow image
	* @var int
	*/
	public $arrow_width  = 4;

	/**
	* The height of the arrow image
	* @var int
	*/
	public $arrow_height = 6;


	/**
	* Set the value for the passed variable to the passed value
	*
	* @param string	$name	the name of the attribute
	* @param string	$value	the new value of the attribute
	*
	* @return boolean
	* @access public
	*/
	public function setAttrValue($name, $value)
	{
		// make sure the aligment is lower case
		switch ($name) {
			case 'alignment' :
				$value = strtolower($value);
			break;

			case 'min_height' :
				$value = (int) $value;
				if ($value <= 0) {
					trigger_localised_error('CORE0177', sprintf(translate('The "%s" attribute must be set to a number greater than zero'), $name), E_USER_WARNING)
					return FALSE;
				}
			break;
		}//end switch

		return parent::setAttrValue($name, $value);

	}//end setAttrValue()


	/**
	* Prints the recursive fn that will be used by this menu to printout its details
	* NOTE: to be overridden by our children
	*
	* @return void
	* @access public
	*/
	public function printRecursiveFn()
	{
		$use_blanks  = !$this->attr('show_stalks');
		$cell_class = '';
		if ($this->settingExists('class')) {
			ob_start();
			$this->printVar('settings.class', Array());
			$cell_class = ' class="'.ob_get_contents().'"';
			ob_end_clean();
		}//end if


		ob_start();
		echo '
				<td'.$cell_class.' style="vertical-align: top;"><img alt="" src="';
		if ($use_blanks) {
			echo '<', '?php echo $blank_src; ?', '>';
		} else {
			echo '<', '?php echo (($'.$this->attr('id_name').'_current_setting == \'current\') ? $stalk_prefix.\'arrow.png\' : $blank_src); ?', '>';

		}
		echo '" width="'.$this->arrow_width.'" height="'.$this->attr('min_height').'" /></td>';
		$arrow_contents = ob_get_contents();
		ob_end_clean();



		ob_start();
		if ($asset_contents = $this->attr('asset_contents')) {
			$this->_tmp['outputting_asset_section'] = TRUE;
			foreach ($asset_contents as $item) {
				$this->_printContentItemCode($item);
			}
			unset($this->_tmp['outputting_asset_section']);
		}//end if

		$asset_contents = '
				<td'.$cell_class.' style="width: 100%; vertical-align: top; text-align: '.$this->attr('alignment').';'.(($this->attr('word_wrap')) ? '' : ' white-space: nowrap;').'">'.ob_get_contents().'</td>
		';
		ob_end_clean();

		// if we are using ellipsis, then we need to include general/general.inc
		if ($this->attr('ellipsis_length') > 0) {
			echo 'require_once SQ_FUDGE_PATH.\'/general/general.inc\';';
		}

		$unwanted_types = $this->attr('unwanted_asset_types');
		$all_type_codes = Array();
		foreach ($unwanted_types as $type) {
			if (!empty($type)) {
				$all_type_codes = array_merge($all_type_codes, $GLOBALS['SQ_SYSTEM']->am->getTypeDescendants($type, TRUE));
			}
		}
		$unwanted_types = $all_type_codes;

		echo '

		function '.$this->attr('id_name').'_recursive_fn($level, $parentid, $'.$this->attr('id_name').'_base_url, $depth=Array())
		{
			global $RECURSIVE_MENU_ASSET_LINEAGE;
			static $'.$this->attr('id_name').'_settings = '.var_export($this->_getSettingsArray(), TRUE).';

			$blank_src    = sq_web_path(\'lib\').\'/web/images/blank.gif\';
			$stalk_prefix = sq_web_path(\'data\').\'/'.$this->data_path_suffix.'/\';

			// The indenting level, stalk on/off
			$depth[] = true;
			$depth_count = count($depth);

			$'.$this->attr('id_name').'_lineage_assetid = (empty($RECURSIVE_MENU_ASSET_LINEAGE[$level + 1])) ? 0 : $RECURSIVE_MENU_ASSET_LINEAGE[$level + 1][\'assetid\'];
			$'.$this->attr('id_name').'_lineage_setting = (count($RECURSIVE_MENU_ASSET_LINEAGE) - 1 == $level + 1) ? \'current\' : \'hierarchy\';
			$'.$this->attr('id_name').'_urls = $GLOBALS[\'SQ_SYSTEM\']->am->getAssetURL(array_keys($GLOBALS[\'MENU_DATA\']['.$this->_safeStringLiteral($this->attr('id_name')).'][$parentid]), $'.$this->attr('id_name').'_base_url);

#			pre_echo("Lineage Setting : $'.$this->attr('id_name').'_lineage_setting");
#			pre_echo("Lineage Assetid : $'.$this->attr('id_name').'_lineage_assetid");
			$num_kids = count($GLOBALS[\'MENU_DATA\'][\''.$this->attr('id_name').'\'][$parentid]);
			$i = 0;
			foreach ($GLOBALS[\'MENU_DATA\']['.$this->_safeStringLiteral($this->attr('id_name')).'][$parentid] as $'.$this->attr('id_name').'_row) {
				$i++;
				$end = ($i == $num_kids);';

		if (!empty($unwanted_types)) {
			$unwanted_types_literal_array = 'Array("'.implode('", "', $unwanted_types).'")';
			echo '
				if (in_array($'.$this->attr('id_name').'_row[\'type_code\'], '.$unwanted_types_literal_array.')) continue ;';
		}//end if

		echo '
				// if not live, we have to get the proper object, sorry
				if (((int) $'.$this->attr('id_name').'_row[\'status\'] & SQ_STATUS_LIVE) == 0) {
					$'.$this->attr('id_name').'_obj = $GLOBALS[\'SQ_SYSTEM\']->am->getAsset($'.$this->attr('id_name').'_row[\'assetid\'], $'.$this->attr('id_name').'_row[\'type_code\']);
				} else {
					unset($'.$this->attr('id_name').'_obj);
				}
				$'.$this->attr('id_name').'_current_setting = ($'.$this->attr('id_name').'_lineage_assetid == $'.$this->attr('id_name').'_row[\'assetid\']) ? $'.$this->attr('id_name').'_lineage_setting : \'normal\';
#				pre_echo("CURRENT SETTING : ".$'.$this->attr('id_name').'_current_setting);
			?', '>
			<div style="text-align: '.$this->attr('alignment').';">
			<table cellpadding="0" cellspacing="0" border="0" width="100%">
				<tr>
			';

		if ($this->attr('alignment') == 'right') {
			echo $asset_contents;
		}

		if ($this->attr('indent')) {

			if ($this->attr('alignment') == 'right') {
				echo $arrow_contents;
			}

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
					<td'.$cell_class;

					if ($use_blanks) {
						echo ' style="vertical-align: top;"><img alt="" src="<', '?php echo $blank_src; ?', '>" width="'.$this->attr('indent').'" height="'.$this->attr('min_height').'" /></td>';
					} else {
						// if there is the need to a stalk
						// and if we are at the end of a sub listing and
						// not it the indent closest to the text (which will have the end branch)
						echo '<', '?php
						if ($depth[$j] && !($end && ($j == $depth_count - 1)))
							echo \' style="vertical-align:top; background-image:url(\', $stalk_prefix, \'stalk.png); background-repeat: repeat-y;"\';

						?', '>><img alt="" src="<', '?php
						if ($j != $depth_count - 1) {
							echo $blank_src.\'"\';
						} elseif ($end) {
							echo $stalk_prefix, \'endbranch.png"\';
						} else {
							echo $stalk_prefix, \'branch.png"\';
						}

						?', '> width="'.$this->attr('indent').'" height="'.$this->attr('min_height').'" /></td>';

					}//end if $use_blanks
		echo '
				<', '?php
				}//end for depth count

			?', '>';

			if ($this->attr('alignment') == 'left') {
				echo $arrow_contents;
			}

		}//end if

		if ($this->attr('alignment') == 'left') {
			echo $asset_contents;
		}

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
				if (('.$recurse_condition.') && !empty($GLOBALS[\'MENU_DATA\']['.$this->_safeStringLiteral($this->attr('id_name')).'][$'.$this->attr('id_name').'_row[\'assetid\']])) {
					if ($end) $depth[$depth_count - 1] = false;
					'.$this->attr('id_name').'_recursive_fn($level + 1, $'.$this->attr('id_name').'_row[\'assetid\'], $'.$this->attr('id_name').'_urls[$'.$this->attr('id_name').'_row[\'assetid\']], $depth);
				} // end if

			}//end for

		}//end '.$this->attr('id_name').'_recursive_fn()

		';

	}//end printRecursiveFn()


	/**
	* Print code that initialises this design area and should always be printed in place of the AREA tag
	*
	* @return void
	* @access protected
	*/
	protected function _printInitCode()
	{
		if ($this->attr('show_stalks') && $this->attr('indent') && $this->attr('min_height')) {
			if (!$this->_createStalkImages($this)) return FALSE;
		}
		return parent::_printInitCode();

	}//end _printInitCode()


	/**
	* Creates the stalk images for this asset, based upon what the current settings are
	*
	* @return boolean
	* @access private
	*/
	protected function _createStalkImages()
	{

		$bg_colour = $this->attr('stalk_bg_colour');
		$fg_colour = $this->attr('stalk_fg_colour');

		$w = $this->attr('indent');
		$h = $this->attr('min_height');

		if (!$w || !$h) {
			return FALSE;
		}


		$direction = ($this->attr('alignment') == 'left') ? 'right' : 'left';

		$image_path = $this->data_path_public;

		//------------------------------------//

		require_once SQ_FUDGE_PATH.'/general/file_system.inc';
		require_once SQ_FUDGE_PATH.'/colour/colour.inc';

		if (!create_directory($image_path)) return FALSE;

		$transparent = FALSE;
		if (trim($bg_colour) == '') {
			// get a nice contrasting shade so we don't pick a colour to close to the FG
			// to use for transperancy
			$bg_colour = Colour::contrastingShade($fg_colour);
			$transparent = TRUE;
		}

		$bg_rgb = Colour::htmlToRgb($bg_colour, FALSE);
		$fg_rgb = Colour::htmlToRgb($fg_colour, FALSE);

		foreach (Array('stalk', 'arrow', 'branch', 'endbranch') as $image_name) {

			// create the image
			$img = ImageCreate(($image_name == 'arrow') ? $this->arrow_width : $w, $h);
			$bg  = ImageColorAllocate($img, $bg_rgb['r'], $bg_rgb['g'], $bg_rgb['b']);
			$fg  = ImageColorAllocate($img, $fg_rgb['r'], $fg_rgb['g'], $fg_rgb['b']);

			if ($transparent) ImageColorTransparent($img, $bg);

			if ($image_name == 'branch' || $image_name == 'endbranch' ) {
				if ($direction == 'left') {
					$x1 = 0;
					$x2 = floor($w / 2);
				} else {
					$x1 = floor($w / 2);
					$x2 = $w - 1;
				}
				ImageFilledRectangle($img, $x1, floor($h / 2), $x2, floor($h / 2), $fg);
			}//end if

			if ($image_name == 'branch' || $image_name == 'stalk' ) {
				ImageFilledRectangle($img, floor($w / 2), 0, floor($w / 2), $h - 1, $fg);
			}//end if

			if ($image_name == 'endbranch') {
				ImageFilledRectangle($img, floor($w / 2), 0, floor($w / 2), floor($h / 2), $fg);
			}//end if

			if ($image_name == 'arrow') {
				if ($direction == 'left') {
					$x1  = $this->arrow_width - 1;
					$multiplier = -1;
				} else {
					$x1  = 0;
					$multiplier = 1;
				}
				$y1 = floor($h / 2) - ($this->arrow_height / 2);

				$points = Array(
							$x1,
							$y1,
							$x1 + ($multiplier * ($this->arrow_width - 1)),
							$y1 + ($this->arrow_height / 2),
							$x1,
							$y1 + $this->arrow_height,
						  );
				ImageFilledPolygon($img, $points, 3, $fg);
			}//end if

			ImagePng($img, $image_path.'/'.$image_name.'.png');
			ImageDestroy($img);

		}//end foreach

		return TRUE;

	}//end _createStalkImages()


}//end class
?>
