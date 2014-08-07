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
* $Id: paypal_configuration_edit_fns.inc,v 1.2 2012/08/30 00:58:34 ewang Exp $
*
*/


require_once SQ_INCLUDE_PATH.'/asset_edit/asset_edit_fns.inc';

/**
* Paypal_Configuration_Edit_Fns
*
* Purpose
*
*
* @author  Anh Ta <ata@squiz.co.uk>
* @version $Revision: 1.2 $
* @package MySource_Matrix_Packages
* @subpackage ecommerce
*/
class Paypal_Configuration_Edit_Fns extends Asset_Edit_Fns
{


	/**
	* Constructor
	*
	*/
	function __construct()
	{
		parent::__construct();

	}//end constructor
	
	
	/**
	* Paint the interface for Paypal Encryption Library Path
	*
	* @param object	&$asset	the asset whose interface we are painting
	* @param object	&$o		the outputter class
	* @param string	$prefix	prefix for the form elements
	*
	* @return boolean
	* @access public
	*/
	function paintEncryptionLibPath(&$asset, &$o, $prefix)
	{
		$current_path = $asset->attr('encryption_lib_path');
		
		if ($asset->writeAccess('attributes')) {
			text_box($prefix.'_encryption_lib_path', $current_path, 80);
		} else {
			if (trim($current_path) == '') {
				echo '<b>[Empty]</b>';
			} else {
				echo '<b>'.$current_path.'</b>';
			}
		}
		
		if (!is_file($current_path)) {
			?><br /><span class="sq-backend-warning"><?php echo translate('path_to_not_valid', 'Paypal Encryption Library'); ?></span><?php
		} else {
			include_once $current_path;
			if (!class_exists('PPCrypto')) {
				?><br /><span class="sq-backend-warning"><?php echo translate('paypal_configuration_encryption_lib_no_required_class'); ?></span><?php
			} else {
				if (!method_exists('PPCrypto', 'signAndEncrypt')) {
					?><br /><span class="sq-backend-warning"><?php echo translate('paypal_configuration_encryption_lib_no_required_method'); ?></span><?php
				}
			}
		}//end if
		
		return TRUE;
		
	}//end paintEncryptionLibPath()


	/**
	* Process the interface for Paypal Encryption Library Path
	*
	* @param object	&$asset	the asset whose interface we are painting
	* @param object	&$o		the outputter class
	* @param string	$prefix	prefix for the form elements
	*
	* @return boolean
	* @access public
	*/
	function processEncryptionLibPath(&$asset, &$o, $prefix)
	{
		if (!$asset->writeAccess('attributes')) return FALSE;
		if (!isset($_POST[$prefix.'_encryption_lib_path'])) return FALSE;
		
		return $asset->setAttrValue('encryption_lib_path', trim($_POST[$prefix.'_encryption_lib_path']));
		
	}//end processEncryptionLibPath()


}//end class
?>