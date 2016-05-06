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
* $Id: payment_gateway_datacash_edit_fns.inc,v 1.2 2012/08/30 00:58:34 ewang Exp $
*
*/


require_once dirname(__FILE__).'/../../payment_gateway/payment_gateway_edit_fns.inc';
require_once SQ_DATA_PATH.'/private/conf/tools.inc';

/**
* Payment_Gateway_Datacash_Edit_Fns
*
*
* @author  Anh Ta <ata@squiz.co.uk>
* @version $Revision: 1.2 $
* @package MySource_Matrix_Packages
* @subpackage ecommerce
*/
class Payment_Gateway_Datacash_Edit_Fns extends Payment_Gateway_Edit_Fns
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
		$bodycopy_asset =& $asset->getBodycopy('display_format');

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

		$bodycopy =& $asset->getBodycopy('display_format');
		$bodycopy_edit_fns =& $bodycopy->getEditFns();

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

		$bodycopy =& $asset->getBodycopy('display_format');
		$bodycopy_edit_fns =& $bodycopy->getEditFns();

		$status = $bodycopy_edit_fns->processBodycopy($bodycopy, $o, $bodycopy->getPrefix());

		return $status;

	}//end processBodycopy()


	/**
	* Paint the interface for Datacash API Path
	*
	* @param object	&$asset	the asset whose interface we are painting
	* @param object	&$o		the outputter class
	* @param string	$prefix	prefix for the form elements
	*
	* @return boolean
	* @access public
	*/
	function paintDatacashAPIPath(&$asset, &$o, $prefix)
	{
		$current_path = trim(SQ_TOOL_DATACASH_API_PATH);
		if (!$current_path) {
			echo '<b>[Empty]</b>';
		} else {
			echo '<b>'.$current_path.'</b>';
		}

		if (!is_file($current_path)) {
			?><br /><span class="sq-backend-warning"><?php echo sprintf(translate('The path to %s is not valid'), 'Datacash PHP API'); ?></span><?php

		} else {
			include_once $current_path;
			if (!class_exists('DataCash_Document')) {
				?><br /><span class="sq-backend-warning"><?php echo translate('The Datacash API does not have the required DataCash_Document class'); ?></span><?php

			} else if (!method_exists('DataCash_Document', 'readDocumentFromString')) {
				?><br /><span class="sq-backend-warning"><?php echo translate('The Datacash API\'s DataCash_Document class does not contain the required readDocumentFromString() method'); ?></span><?php

			}
		}//end if

		return TRUE;

	}//end paintDatacashAPIPath()


	/**
	* Paint the interface for CardInfo Directory Path
	*
	* @param object	&$asset	the asset whose interface we are painting
	* @param object	&$o		the outputter class
	* @param string	$prefix	prefix for the form elements
	*
	* @return boolean
	* @access public
	*/
	function paintCardInfoDirPath(&$asset, &$o, $prefix)
	{
		$current_path = trim(SQ_TOOL_DATACASH_CARDINFO_DIR_PATH);

		if (!$current_path) {
			echo '<b>[Empty]</b>';
		} else {
			echo '<b>'.$current_path.'</b>';
		}

		if (!is_dir($current_path)) {
			?><br /><span class="sq-backend-warning"><?php echo sprintf(translate('The path to %s is not valid'), 'Datacash CardInfo Directory'); ?></span><?php

		} else if (!is_file($current_path.'/CardInfo1.bin') || !is_file($current_path.'/CardInfo2.bin')) {
			?><br /><span class="sq-backend-warning"><?php echo translate('The Datacash CardInfo Directory does not contain CardInfo1.bin or CardInfo2.bin file'); ?></span><?php

		}

		return TRUE;

	}//end paintCardInfoDirPath()

}//end class

?>
