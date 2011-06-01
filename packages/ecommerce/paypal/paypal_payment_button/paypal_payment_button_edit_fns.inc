<?php 
/**
* +--------------------------------------------------------------------+
* | This MySource Matrix CMS file is Copyright (c) Squiz Pty Ltd       |
* | ACN 084 670 600                                                    |
* +--------------------------------------------------------------------+
* | IMPORTANT: Your use of this Software is subject to the terms of    |
* | the Licence provided in the file licence.txt. If you cannot find   |
* | this file please contact Squiz (www.squiz.net) so we may provide   |
* | you a copy.                                                        |
* +--------------------------------------------------------------------+
*
* $Id: paypal_payment_button_edit_fns.inc,v 1.2 2009/03/30 18:11:23 ata Exp $
*
*/


require_once SQ_CORE_PACKAGE_PATH.'/page/page_edit_fns.inc';

/**
* Paypal_Payment_Button_Edit_Fns
*
* Purpose
*
*
* @author  Anh Ta <ata@squiz.co.uk>
* @version $Revision: 1.2 $
* @package MySource_Matrix_Packages
* @subpackage ecommerce
*/
class Paypal_Payment_Button_Edit_Fns extends Page_Edit_Fns
{

	/**
	* Constructor
	*
	*/
	function __construct()
	{
		parent::__construct();
		$this->static_screens['details']['force_unlock'] = FALSE;
		
	}//end constructor

	
	/**
	* Paint the interface for extra variables
	*
	* @param object	&$asset	the asset whose interface we are painting
	* @param object	&$o		the outputter class
	* @param string	$prefix	prefix for the form elements
	*
	* @return boolean
	* @access public
	*/
	function paintExtraVars(&$asset, &$o, $prefix)
	{
		$current_vars = $asset->attr('extra_vars');
		
		if ($asset->writeAccess('attributes')) {
			text_area($prefix.'_extra_vars', $current_vars, 80, 30);
		} else {
			if ($current_vars == '') {
				echo '<b>[Empty]</b>';
			} else {
				echo '<b>'.str_replace("\n", '<br />', $current_vars).'</b>';
			}
		}
		
		return TRUE;
		
	}//end paintExtraVars()


	/**
	* Process the interface for extra variables
	*
	* @param object	&$asset	the asset whose interface we are painting
	* @param object	&$o		the outputter class
	* @param string	$prefix	prefix for the form elements
	*
	* @return boolean
	* @access public
	*/
	function processExtraVars(&$asset, &$o, $prefix)
	{
		if (!$asset->writeAccess('attributes')) return FALSE;
		if (!isset($_POST[$prefix.'_extra_vars'])) return FALSE;
		
		$extra_vars = $_POST[$prefix.'_extra_vars'];
		$extra_vars = trim($extra_vars);
		//replace Windows newline character with Unix newline character because Paypal decryption only accept the Unix format
		$extra_vars = str_replace("\r\n", "\n", $extra_vars);
		
		return $asset->setAttrValue('extra_vars', $extra_vars);
		
	}//end processExtraVars()
	

}//end class
?>