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
* $Id: design_area_constant_button.inc,v 1.14.2.1 2004/03/02 18:35:51 brobertson Exp $
* $Name: not supported by cvs2svn $
*/


require_once SQ_CORE_PACKAGE_PATH.'/designs/design_area/design_area.inc';

/**
* Design_Area_Constant_Button
*
* Purpose
*
*
* @author  Blair Robertson <blair@squiz.net>
* @version $Version$ - 1.0
* @package MySource_Matrix_Packages
* @subpackage __core__
*/
class Design_Area_Constant_Button extends Design_Area
{

	/*
	* Outputs either the straight HTML or PHP code that is needed for this design area to be represented
	*
	* @param object Design	$design the design for which we are currently printing
	*
	* @return boolean
	* @access public
	*/
	function printArea(&$design)
	{
		$assetid = $this->attr('assetid');
		$url     = $this->attr('url');

		// return true because it just means that they haven't set anything yet
		if (empty($assetid) && empty($url)) return true;

		// if there is an assetid then do some prep work before printing the area
		if (!empty($assetid)) {
			echo '<', '?php
			$constant_btn_'.$this->attr('id_name').'_asset = &$GLOBALS[\'SQ_SYSTEM\']->am->getAsset('.$assetid.');
			if (is_null($constant_btn_'.$this->attr('id_name').'_asset)) {
				$constant_btn_'.$this->attr('id_name').'_url = \'\';
			} else {
				$constant_btn_'.$this->attr('id_name').'_url = $constant_btn_'.$this->attr('id_name').'_asset->getHref();
			}// end if
			?', '>';
		}// end if

		return parent::printArea($design);

	}//end printArea()


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
		switch($var) {
			case 'href' :
				$assetid = $this->attr('assetid');

				if (empty($assetid)) {
					$url_code = $this->attr('url');
				} else {
					$url_code = '<'.'?php echo $constant_btn_'.$this->attr('id_name').'_url; ?'.'>';
				}

				// if we are poping up in a new window, adjust the vars accordingly
				if ($this->attr('new_window')) {
					$width  = (int) $this->attr('new_window_width');
					$height = (int) $this->attr('new_window_height');
					 // some defaults just in case
					if ($width  <= 0) $width  = 640;
					if ($height <= 0) $height = 600;
					$url_code = 'Javascript: new_window = window.open(\''.$url_code.'\', \'constant_btn_'.$this->attr('id_name').'\', \'toolbar=no,width='.$width.',height='.$height.',titlebar=false,scrollbars=yes\');new_window.focus();';
				}// end if

				echo 'href="'.$url_code.'"';
				break;

			default :
				parent::printVar($var, $args);

		}// end switch

	}//end printVar()


}//end class

?>