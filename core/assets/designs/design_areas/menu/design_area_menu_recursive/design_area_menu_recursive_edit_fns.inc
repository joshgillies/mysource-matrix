<?php

require_once SQ_CORE_PACKAGE_PATH.'/designs/design_areas/menu/design_area_menu_type/design_area_menu_type_edit_fns.inc';

/**
* Design_Area_Menu_Recursive_Edit_Fns
*
* Purpose
*
*
* @author  Blair Robertson <blair@squiz.net>
* @version $Version$ - 1.0
* @package Resolve::packages::__core__
*/
class Design_Area_Menu_Recursive_Edit_Fns extends Design_Area_Menu_Type_Edit_Fns
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
	* Takes the contents for the sub menu element and attempts to process it, 
	* is a separate fn to allow menu types that don't want to allow sub menu's to override
	*
	* @param object Asset	$asset			the asset to which we belong
	* @param string			$design_area	the menu type design area that the sub menu is supposed to be
	* @param array			$contents		the sub menu's contents array that we generated with _parseString()
	*
	* @return boolean
	* @access private
	* @see Design_Area_Menu_Type_Edit_Fns::_processContents();
	*/
	function _processSubMenu(&$asset, &$contents)
	{
		// we just want to ignore any stupid sub menu's in our page contents
		trigger_error('You are unable to set sub menu\'s on the recursive menu type', E_USER_WARNING);
		return false;
	}// end _processSubMenu()

	/**
	* Performs any outputting needed to intialise menu type outputs
	*
	* @access private
	*/
	function _initDesignFileOutput() {
		if (!empty($GLOBALS['SQ_'.__CLASS__.__FUNCTION__.'_RUN'])) return;

		parent::_initDesignFileOutput();

		echo '<', '?php
			// because we need to be able to get to these with the recurse fn
			$GLOBALS[\'PAGE_LINEAGE\'] = &$PAGE_LINEAGE;
			$GLOBALS[\'ASSET\'] = &$ASSET;
		?', '>';

		$GLOBALS['SQ_'.__CLASS__.__FUNCTION__.'_RUN'] = true;

	} // _initDesignFileOutput()


	/**
	* Outputs the PHP/HTML code that will hopefully form part of the creating the design file
	*
	* @param Array			$contents	the parse contents to use in the output
	* @param object Asset	$asset		the asset to which we belong
	* @param object Design	$design		the design object that is painting us
	*
	* @return boolean
	* @access private
	*/
	function _createDesignFileOutput($contents, &$asset, &$design)
	{
		$this->_initDesignFileOutput();
		if ($asset->attr('show_stalks') && $asset->attr('indent') && $asset->attr('min_height')) {
			if (!$this->_createStalkImages($asset)) return false;
		}
		return parent::_createDesignFileOutput($contents, $asset, $design);

	}// end _createDesignFileOutput()

	/**
	* Used by _outputContents() to deal with a print tag
	*
	* @param array			$element	the print element tag
	* @param object Asset	$asset		the asset to which we belong
	* @param object Design	$design		the design object that is painting us
	*
	* @access private
	* @see Design_Area_Edit_Fns::_outputContents()
	*/
	function _outputElement($element, &$asset, &$design)
	{
		switch($element['operation']) {
			case 'page' :
				$use_blanks  = !$asset->attr('show_stalks');
				$cell_class = '';
				if ($asset->settingExists('class')) { 
					ob_start();
					$asset->printVar('settings.class', Array());
					$cell_class = ' class="'.ob_get_contents().'"';
					ob_end_clean();
				}// end if


				ob_start();
				echo '
						<td'.$cell_class.' valign="top"><img src="';
				if ($use_blanks) {
					echo '<', '?php $blank_src; ?', '>';
				} else {
					echo '<', '?php echo (($'.$asset->attr('id_name').'_current_setting == \'current\') ? $stalk_prefix.\'arrow.png\' : $blank_src); ?', '>';
					
				}
				echo '" width="'.$this->arrow_width.'" height="'.$asset->attr('min_height').'"></td>';
				$arrow_contents = ob_get_contents();
				ob_end_clean();



				ob_start();
				// if there is no contents for the page, then let's use one I prepared earlier :)
				if (empty($element['contents'])) {
					require_once SQ_FUDGE_PATH.'/general/file_system.inc';
					$str = file_to_string(dirname(__FILE__).'/default_page_contents.html');
					$contents = $this->_parseString($str);
					if (!$this->_outputContents($contents, $asset, $design)) {
						ob_end_clean();
						return false;
					}
				} else {
					if (!$this->_outputContents($element['contents'], $asset, $design)) {
						ob_end_clean();
						return false;
					}
				}//	end if
				$page_contents = '
						<td'.$cell_class.(($asset->attr('word_wrap')) ? '' : ' nowrap').' width="100%" valign="top" align="'.$asset->attr('alignment').'">'.ob_get_contents().'</td>
				';
				ob_end_clean();

				echo '<', '?php 
				';
				// if we are using ellipsis, then we need to include general/general.inc
				if ($asset->attr('ellipsis_length') > 0) { 
					echo 'require_once SQ_FUDGE_PATH.\'/general/general.inc\';';
				}
				echo '

			function '.$asset->attr('id_name').'_recursive_fn($level, $links, $depth=Array())
			{
				global $PAGE_LINEAGE;
				static $'.$asset->attr('id_name').'_settings = '.var_export($asset->attr('settings'), true).';

				$am = &$GLOBALS[\'SQ_SYSTEM\']->am;
				$blank_src    = sq_web_path(\'lib\').\'/web/images/blank.gif\';
				$stalk_prefix = sq_web_path(\'data\').\'/'.$asset->type().'/'.$asset->id.'/\';

				// The indenting level, stalk on/off
				$depth[] = true; 
				$depth_count = count($depth);

				// if this menu is for the first level of the menu then the 
				// $ASSET could very well be a site asset in which case we need to set the details
				// to represent it\'s index page
				// if the asset that is printing this body is a site, 
				// then use it\'s index page for the lineage asset
				if ($level == 0 && $GLOBALS[\'ASSET\']->type() == \'site\') { 
					$index_link = $GLOBALS[\'ASSET\']->getSpecialPageLink(\'index\');
					$'.$asset->attr('id_name').'_lineage_assetid = (empty($index_link)) ? 0 : $index_link[\'minorid\'];
					$'.$asset->attr('id_name').'_lineage_setting = \'current\';

				} else {
					$'.$asset->attr('id_name').'_lineage_assetid = (empty($PAGE_LINEAGE[$level + 1])) ? 0 : $PAGE_LINEAGE[$level + 1][\'assetid\'];
					$'.$asset->attr('id_name').'_lineage_setting = (count($PAGE_LINEAGE) - 1 == $level + 1) ? \'current\' : \'hierarchy\';

				}// end if level == 0 and asset type == site
				';

				echo '
#				pre_echo("Lineage Setting : $'.$asset->attr('id_name').'_lineage_setting");
#				pre_echo("Lineage Assetid : $'.$asset->attr('id_name').'_lineage_assetid");
				for($i = 0, $num_links = count($links); $i < $num_links; $i++) {
					$end = ($i == $num_links - 1);
					$'.$asset->attr('id_name').'_kid = &$am->getAsset($links[$i][\'minorid\'], $links[$i][\'type_code\']);
					$'.$asset->attr('id_name').'_current_setting = ($'.$asset->attr('id_name').'_lineage_assetid == $'.$asset->attr('id_name').'_kid->id) ? $'.$asset->attr('id_name').'_lineage_setting : \'normal\';
#					pre_echo("CURRENT SETTING : ".$'.$asset->attr('id_name').'_current_setting);

				?', '> 
				<div align="'.$asset->attr('alignment').'">
				<table cellpadding="0" cellspacing="0" border="0" width="100%">
					<tr>
				';

			if ($asset->attr('alignment') == 'right') echo $page_contents;

			if ($asset->attr('indent')) {

				if ($asset->attr('alignment') == 'right') echo $arrow_contents;

				echo '
				<', '?php
				';

				// depending on the alignment differs how we loop through the array
				if ($asset->attr('alignment') == 'left') {
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
							echo '><img src="<', '?php $blank_src; ?', '>" width="'.$asset->attr('indent').'" height="'.$asset->attr('min_height').'"></td>';
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

							?', '>" width="'.$asset->attr('indent').'" height="'.$asset->attr('min_height').'"></td>';

						}// end if $use_blanks
			echo '
					<', '?php
					}// end for depth count

				?', '>';

				if ($asset->attr('alignment') == 'left') echo $arrow_contents;

			}// end indent

			if ($asset->attr('alignment') == 'left') echo $page_contents;

			echo '

						</tr>
					</table>
					</div>
					<', '?php

				';

			// if we are to show a minimum number of levels, add an extra bit to the condition

			$recurse_condition = '$'.$asset->attr('id_name').'_current_setting != \'normal\'';


			$levels_in = ($asset->attr('level') > 0) ? '($level - '.$asset->attr('level').')' : '$level';
			if ($asset->attr('min_num_levels') > 0) {
				$recurse_condition = $levels_in.' < '.$asset->attr('min_num_levels').' || '.$recurse_condition;
			}

			if ($asset->attr('max_num_levels') > 0) {
				$recurse_condition = $levels_in.' < '.$asset->attr('max_num_levels').' && ('.$recurse_condition.')';
			}
			echo '
					if ('.$recurse_condition.') {
						$'.$asset->attr('id_name').'_kid_links = $'.$asset->attr('id_name').'_kid->getLinks(SQ_LINK_NAV);
						if (!empty($'.$asset->attr('id_name').'_kid_links)) {
							if ($end) $depth[$depth_count - 1] = false;
							'.$asset->attr('id_name').'_recursive_fn($level + 1, $'.$asset->attr('id_name').'_kid_links, $depth);
						}// end if kid links

					} // end if current_setting != normal

				}// end foreach links

			}// end '.$asset->attr('id_name').'_recursive_fn()

			';

			echo '

			'.$asset->attr('id_name').'_recursive_fn($'.$asset->attr('id_name').'_level, $'.$asset->attr('id_name').'_links);

			?', '> ';

				return true;


			case 'sub' :
				# there ain't going to be any subs :)
				return true;

			default :
				return parent::_outputElement($element, $asset, $design);
		}// end switch

	}// end _outputElement()

	/**
	* Creates the stalk images for this asset, based upon what the current settings are
	*
	* @param object Asset	$asset		the asset to which we belong
	*
	* @return boolean
	* @access private
	*/
	function _createStalkImages($asset) 
	{

		$bg_colour = $asset->attr('stalk_bg_colour');
		$fg_colour = $asset->attr('stalk_fg_colour');

		$w = $asset->attr('indent');
		$h = $asset->attr('min_height');

		if (!$w || !$h) return;


		$direction = ($asset->attr('alignment') == 'left') ? 'right' : 'left';

		$image_path = $asset->data_path;

		//------------------------------------//

		require_once '/home/brobertson/resolve/fudge/general/file_system.inc';
		require_once '/home/brobertson/resolve/fudge/colour/colour.inc';

		if (!create_directory($image_path)) return false;

		$transparent = false;
		if (trim($bg_colour) == '') {
			// get a nice contrasting shade so we don't pick a colour to close to the FG 
			// to use for transperancy
			$bg_colour = Colour::contrastingShade($fg_colour);
			$transparent = true;
		}

		$bg_rgb = Colour::htmlToRgb($bg_colour, false);
		$fg_rgb = Colour::htmlToRgb($fg_colour, false);

		foreach(Array('stalk', 'arrow', 'branch', 'endbranch') as $image_name) {

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
			}// end if

			if ($image_name == 'branch' || $image_name == 'stalk' ) { 
				ImageFilledRectangle($img, floor($w / 2), 0, floor($w / 2), $h - 1, $fg);
			}// end if

			if ($image_name == 'endbranch') { 
				ImageFilledRectangle($img, floor($w / 2), 0, floor($w / 2), floor($h / 2), $fg);
			}// end if

			if ($image_name == 'arrow') { 
				if ($direction == 'left') {
					$x1  = $this->arrow_width - 1;
					$multiplier = -1;
				} else {
					$x1  = 0;
					$multiplier = 1;
				}
				$y1 = floor($h / 2) - ($this->arrow_height / 2);

				$points = Array($x1, $y1, // x1, y1
								$x1 + ($multiplier * ($this->arrow_width - 1)),  $y1 + ($this->arrow_height / 2), // x2, y2
								$x1,  $y1 + $this->arrow_height, // x3, y3
								);
				ImageFilledPolygon($img, $points, 3, $fg);
			}// end if

			ImagePng($img, $image_path.'/'.$image_name.'.png'); 
			ImageDestroy($img);

		}// end foreach

		return true;

	}// end _createStalkImages();

}//end class
?>
