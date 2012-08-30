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
* $Id: payment_gateway_utransact_edit_fns.inc,v 1.2 2012/08/30 00:58:34 ewang Exp $
*
*/


require_once dirname(__FILE__).'/../../payment_gateway/payment_gateway_edit_fns.inc';

/**
* Payment_Gateway_UTransact_Edit_Fns
*
* @author  Chiranjivi Upreti <cupreti@squiz.com.au>
* @version $Revision: 1.2 $
* @package MySource_Matrix_Packages
* @subpackage ecommerce
*/
class Payment_Gateway_UTransact_Edit_Fns extends Payment_Gateway_Edit_Fns
{


	/**
	* Paint the interface for creating containers in the bodycopy
	*
	* @param object	&$asset	the asset whose interface we are painting
	* @param object	&$o		the Backend_Outputter class
	* @param string	$prefix	prefix for the form elements
	*
	* @return boolean
	* @access public
	*/
	function paintBodycopyInfo(&$asset, &$o, $prefix)
	{
		$bodycopy_asset = $asset->getBodycopy('display_format');

		$o->openRaw();
		echo '<div style="padding:1ex;">';
		echo get_asset_tag_line($bodycopy_asset->id, 'contents');
		echo '</div>';
		$o->closeRaw();

	}//end paintBodycopyInfo()


	/**
	* Paint the interface for creating containers in the bodycopy
	*
	* @param object	&$asset	the asset whose interface we are painting
	* @param object	&$o		the Backend_Outputter class
	* @param string	$prefix	prefix for the form elements
	*
	* @return boolean
	* @access public
	*/
	function paintBodycopy(&$asset, &$o, $prefix)
	{
		$write_access = $asset->writeAccess('menu');

		$bodycopy = $asset->getBodycopy('display_format');
		$bodycopy_edit_fns = $bodycopy->getEditFns();

		$o->openRaw();
		$bodycopy_edit_fns->paintBodycopy($bodycopy, $o, $bodycopy->getPrefix());
		$o->closeRaw();

		return $write_access;

	}//end paintBodycopy()


	/**
	* Processes the interface for creating containers in the bodycopy
	*
	* @param object	&$asset	the asset to which we belong
	* @param object	&$o		the Backend_Outputter class
	* @param string	$prefix	prefix for the form element
	*
	* @return boolean
	* @access public
	*/
	function processBodycopy(&$asset, &$o, $prefix)
	{
		if (!$asset->writeAccess('menu')) return FALSE;

		$bodycopy = $asset->getBodycopy('display_format');
		$bodycopy_edit_fns = $bodycopy->getEditFns();

		$status = $bodycopy_edit_fns->processBodycopy($bodycopy, $o, $bodycopy->getPrefix());

		return $status;

	}//end processBodycopy()

	
	/**
	* Paints the Utransact login password field
	*
	* @param object	&$asset	asset being painted
	* @param object	&$o		backend outputter
	* @param string	$prefix	prefix for the html doc element name
	*
	* @return boolean
	* @access public
	*/
	function paintAccountPassword(&$asset, &$o, $prefix)
	{   
		if ($asset->writeAccess('attributes')) {
            password_box($prefix.'_password', '', 20);
		} else {
			echo "Password not shown";
		}
		
		return TRUE;

	}//end paintAccountPassword()


	/**
	* Process the Utransact login password field
	*
	* @param object	&$asset	asset being painted
	* @param object	&$o		backend outputter
	* @param string	$prefix	prefix for the html doc element name
	*
	* @return boolean
	* @access public
	*/
	function processAccountPassword(&$asset, &$o, $prefix)
	{   
		if (!isset($_POST[$prefix.'_password'])) {
			return FALSE;
		}
		
		if (!$GLOBALS['SQ_SYSTEM']->am->acquireLock($asset->id, 'attributes')) {
            return FALSE;
         }
		
		$success = FALSE;
		
		// Set the attribute
		$val = isset($_POST[$prefix.'_password']) ? $_POST[$prefix.'_password'] : '';
		
        if ($val && $asset->setAttrValue('password', $val)) {
            $success = $asset->saveAttributes();
        }

		return $success;

	}//end processAccountPassword()


}//end class

?>