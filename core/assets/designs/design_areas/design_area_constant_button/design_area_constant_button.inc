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
* $Id: design_area_constant_button.inc,v 1.27 2012/08/30 01:09:06 ewang Exp $
*
*/


require_once SQ_CORE_PACKAGE_PATH.'/designs/design_area/design_area.inc';

/**
* Design_Area_Constant_Button
*
* Purpose
*
*
* @author  Blair Robertson <blair@squiz.net>
* @version $Revision: 1.27 $
* @package MySource_Matrix_Packages
* @subpackage __core__
*/
class Design_Area_Constant_Button extends Design_Area
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
	* Prints code in place of the AREA tag
	*
	* The protected version of this function, rather than this one, should be overridden by children
	*
	* @return void
	* @access protected
	*/
	protected function _printInitCode()
	{
		$assetid = (int) $this->attr('assetid');
		$url     = $this->attr('url');

		// return true because it just means that they haven't set anything yet
		if (empty($assetid) && empty($url)) {
			return TRUE;
		}

		// if there is an assetid then do some prep work before printing the area
		if (!empty($assetid)) {
			echo '<', '?php
			$constant_btn_'.$this->attr('id_name').'_url = $am->getAssetURL('.$assetid.').((SQ_LIMBO_EDITING) ? \'/\'.SQ_CONF_LIMBO_SUFFIX.\'?SQ_BACKEND_PAGE=main\' : \'\');
			?', '>';
		}// end if

	}//end _printInitCode()


	/**
	* Can we initialise this DA?
	*
	* @return boolean
	* @access protected
	*/
	protected function _canInit()
	{
		return ($this->attr('url')) || ($this->attr('assetid'));

	}//end _canInit()


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
		switch ($var) {
			case 'href' :
				$assetid = (int) $this->attr('assetid');

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