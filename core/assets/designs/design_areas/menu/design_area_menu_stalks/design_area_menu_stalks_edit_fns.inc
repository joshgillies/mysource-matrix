<?php

require_once SQ_CORE_PACKAGE_PATH.'/designs/design_areas/menu2/design_area_menu2_recursive/design_area_menu2_recursive_edit_fns.inc';

/**
* Design_Area_Menu2_Stalks_Edit_Fns
*
* Purpose
*
*
* @author  Blair Robertson <blair@squiz.net>
* @version $Version$ - 1.0
* @package Resolve_Packages
* @subpackage __core__
*/
class Design_Area_Menu2_Stalks_Edit_Fns extends Design_Area_Menu2_Recursive_Edit_Fns
{

	/**
	* Creates the stalk images for this asset, based upon what the current settings are
	*
	* @param object Asset	$asset		the asset to which we belong
	*
	* @return boolean
	* @access private
	*/
	function _createStalkImages(&$asset)
	{

		$bg_colour = $asset->attr('stalk_bg_colour');
		$fg_colour = $asset->attr('stalk_fg_colour');

		$w = $asset->attr('indent');
		$h = $asset->attr('min_height');

		if (!$w || !$h) return false;


		$direction = ($asset->attr('alignment') == 'left') ? 'right' : 'left';

		$image_path = $asset->data_path;

		//------------------------------------//

		require_once SQ_FUDGE_PATH.'/general/file_system.inc';
		require_once SQ_FUDGE_PATH.'/colour/colour.inc';

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
			$img = ImageCreate(($image_name == 'arrow') ? $asset->arrow_width : $w, $h);
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
					$x1  = $asset->arrow_width - 1;
					$multiplier = -1;
				} else {
					$x1  = 0;
					$multiplier = 1;
				}
				$y1 = floor($h / 2) - ($asset->arrow_height / 2);

				$points = Array($x1, $y1, // x1, y1
								$x1 + ($multiplier * ($asset->arrow_width - 1)),  $y1 + ($asset->arrow_height / 2), // x2, y2
								$x1,  $y1 + $asset->arrow_height, // x3, y3
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
