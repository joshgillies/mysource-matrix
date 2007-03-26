<?php
/**
* +--------------------------------------------------------------------+
* | This MySource Matrix Module file is Copyright (c) Squiz Pty Ltd    |
* | ACN 084 670 600                                                    |
* +--------------------------------------------------------------------+
* | IMPORTANT: This Module is not available under an open source       |
* | license and consequently distribution of this and any other files  |
* | that comprise this Module is prohibited. You may only use this     |
* | Module if you have the written consent of Squiz.                   |
* +--------------------------------------------------------------------+
*
* $Id: payment_gateway_esec_edit_fns.inc,v 1.6 2007/03/26 04:05:33 arailean Exp $
*
*/


require_once dirname(__FILE__).'/../../payment_gateway/payment_gateway_edit_fns.inc';

/**
* Payment_Gateway_Esec_Edit_Fns
*
* Empty class for support
*
*
* @author  Andrei Railean		<arailean@squiz.net>
* @author  Dmitry Baranovskiy	<dbaranovskiy@squiz.net>
* @version $Revision: 1.6 $
* @package MySource_Matrix_Packages
* @subpackage ecommerce
*/
class Payment_Gateway_Esec_Edit_Fns extends Payment_Gateway_Edit_Fns
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
		$bodycopy_asset =& $asset->getBodycopy();-

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

		$bodycopy =& $asset->getBodycopy();
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

		$bodycopy =& $asset->getBodycopy();
		$bodycopy_edit_fns =& $bodycopy->getEditFns();

		$status = $bodycopy_edit_fns->processBodycopy($bodycopy, $o, $bodycopy->getPrefix());

		return $status;

	}//end processBodycopy()


}//end class

?>