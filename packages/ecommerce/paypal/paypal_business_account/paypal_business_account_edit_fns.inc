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
* $Id: paypal_business_account_edit_fns.inc,v 1.2 2012/08/30 00:58:34 ewang Exp $
*
*/


require_once SQ_INCLUDE_PATH.'/asset_edit/asset_edit_fns.inc';

/**
* Paypal_Business_Account_Edit_Fns
*
* Purpose
*
*
* @author  Anh Ta <ata@squiz.co.uk>
* @version $Revision: 1.2 $
* @package MySource_Matrix_Packages
* @subpackage ecommerce
*/
class Paypal_Business_Account_Edit_Fns extends Asset_Edit_Fns
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
	* Paint the interface for Public Certificate Path
	*
	* @param object	&$asset	the asset whose interface we are painting
	* @param object	&$o		the outputter class
	* @param string	$prefix	prefix for the form elements
	*
	* @return boolean
	* @access public
	*/
	function paintPublicCertPath(&$asset, &$o, $prefix)
	{
		$this->_paintPath($asset, $prefix, 'public_cert_path', 'Public Certificate Path');

		return TRUE;

	}//end paintPublicCertPath()


	/**
	* Process the interface for Public Certificate Path
	*
	* @param object	&$asset	the asset whose interface we are painting
	* @param object	&$o		the outputter class
	* @param string	$prefix	prefix for the form elements
	*
	* @return boolean
	* @access public
	*/
	function processPublicCertPath(&$asset, &$o, $prefix)
	{
		return $this->_processPath($asset, $prefix, 'public_cert_path');

	}//end processPublicCertPath()


	/**
	* Paint the interface for Private Key Path
	*
	* @param object	&$asset	the asset whose interface we are painting
	* @param object	&$o		the outputter class
	* @param string	$prefix	prefix for the form elements
	*
	* @return boolean
	* @access public
	*/
	function paintPrivateKeyPath(&$asset, &$o, $prefix)
	{
		$this->_paintPath($asset, $prefix, 'private_key_path', 'Private Key Path');

		return TRUE;

	}//end paintPrivateKeyPath()


	/**
	* Process the interface for Private Key Path
	*
	* @param object	&$asset	the asset whose interface we are painting
	* @param object	&$o		the outputter class
	* @param string	$prefix	prefix for the form elements
	*
	* @return boolean
	* @access public
	*/
	function processPrivateKeyPath(&$asset, &$o, $prefix)
	{
		return $this->_processPath($asset, $prefix, 'private_key_path');

	}//end processPrivateKeyPath()


	/**
	* Paint the interface for Paypal Certificate Path
	*
	* @param object	&$asset	the asset whose interface we are painting
	* @param object	&$o		the outputter class
	* @param string	$prefix	prefix for the form elements
	*
	* @return boolean
	* @access public
	*/
	function paintPaypalCertPath(&$asset, &$o, $prefix)
	{
		$this->_paintPath($asset, $prefix, 'paypal_cert_path', 'Paypal Certificate Path');

		return TRUE;

	}//end paintPaypalCertPath()


	/**
	* Process the interface for Paypal Certificate Path
	*
	* @param object	&$asset	the asset whose interface we are painting
	* @param object	&$o		the outputter class
	* @param string	$prefix	prefix for the form elements
	*
	* @return boolean
	* @access public
	*/
	function processPaypalCertPath(&$asset, &$o, $prefix)
	{
		return $this->_processPath($asset, $prefix, 'paypal_cert_path');

	}//end processPaypalCertPath()


	/**
	* Common method to paint the interface for all paths
	*
	* @param object	&$asset		the asset whose interface we are painting
	* @param string	$prefix		prefix for the form elements
	* @param string $attr_name	the name of the attribute that stores the path
	* @param string $full_name	the full name of the path (used to print out warning message)
	*
	* @return void
	* @access private
	*/
	private function _paintPath(&$asset, $prefix, $attr_name, $full_name)
	{
		$current_path = $asset->attr($attr_name);

		if ($asset->writeAccess('attributes')) {
			text_box($prefix.'_'.$attr_name, $current_path, 80);
		} else {
			if (trim($current_path) == '') {
				echo '<b>[Empty]</b>';
			} else {
				echo '<b>'.$current_path.'</b>';
			}
		}

		if (!is_file($current_path)) {
			?><br /><span class="sq-backend-warning"><?php echo sprintf(translate('The path to %s is not valid'), $full_name); ?></span><?php

		}//end if

	}//end _paintPath()


	/**
	* Common method to process the interface for all paths
	*
	* @param object	&$asset		the asset whose interface we are painting
	* @param string	$prefix		prefix for the form elements
	* @param string $attr_name	the name of the attribute that stores the path
	*
	* @return boolean
	* @access public
	*/
	private function _processPath(&$asset, $prefix, $attr_name)
	{
		if (!$asset->writeAccess('attributes')) return FALSE;
		if (!isset($_POST[$prefix.'_'.$attr_name])) return FALSE;

		return $asset->setAttrValue($attr_name, trim($_POST[$prefix.'_'.$attr_name]));

	}//end _processPath()


}//end class
?>