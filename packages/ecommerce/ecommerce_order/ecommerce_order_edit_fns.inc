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
* $Id: ecommerce_order_edit_fns.inc,v 1.8 2006/12/07 05:26:28 bcaldwell Exp $
*
*/

require_once SQ_INCLUDE_PATH.'/asset_edit/asset_edit_fns.inc';

/**
* Ecommerce_Order_Edit_Fns
*
* Purpose
*
*
* @author  Scott Kim <skim@squiz.net>
* @version $Revision: 1.8 $
* @package MySource_Matrix_Packages
* @subpackage ecommerce
*/
class Ecommerce_Order_Edit_Fns extends Asset_Edit_Fns
{


	/**
	* Constructor
	*
	* @return void
	* @access public
	*/
	function Ecommerce_Order_Edit_Fns()
	{
		$this->Asset_Edit_Fns();

	}//end constructor


	/**
	* Paints the summary of ecommerce order
	*
	* @param object	Asset				&$rootid	asset being painted
	* @param object Backend_Outputter	&$o			backend outputter
	* @param string						$prefix		prefix for the html doc element name
	*
	* @return boolean
	* @access public
	*/
	function paintDeliveryMethod(&$asset, &$o, $prefix)
	{
		$am =& $GLOBALS['SQ_SYSTEM']->am;

		$assetid = $asset->attr('delivery_id');
		if (!empty($assetid)) {
			echo get_asset_tag_line($assetid, 'details');
		} else {
			echo translate('none');
		}

	}


	/**
	* Paints the summary of ecommerce order
	*
	* @param object	Asset				&$rootid	asset being painted
	* @param object Backend_Outputter	&$o			backend outputter
	* @param string						$prefix		prefix for the html doc element name
	*
	* @return boolean
	* @access public
	*/
	function paintOrderSummary(&$asset, &$o, $prefix)
	{
		echo '<pre>'.$asset->attr('summary').'</pre>';

		return false;

	}//end paintOrderSummary()


	/**
	* Paints the summary of ecommerce order
	*
	* @param object	Asset				&$rootid	asset being painted
	* @param object Backend_Outputter	&$o			backend outputter
	* @param string						$prefix		prefix for the html doc element name
	*
	* @return boolean
	* @access public
	*/
	function paintInternalSummary(&$asset, &$o, $prefix)
	{
		echo '<pre>'.$asset->attr('internal_summary').'</pre>';

		return false;

	}//end paintOrderSummary()


	/**
	* Paints the status of ecommerce order
	*
	* @param object	Asset				&$rootid	asset being painted
	* @param object Backend_Outputter	&$o			backend outputter
	* @param string						$prefix		prefix for the html doc element name
	*
	* @return boolean
	* @access public
	*/
	function paintOrderStatus(&$asset, &$o, $prefix)
	{
		echo '<pre>'.$asset->attr('status_message').'</pre>';

		return false;

	}//end paintOrderStatus()


}//end class

?>
