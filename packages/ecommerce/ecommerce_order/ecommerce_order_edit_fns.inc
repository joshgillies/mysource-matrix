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
* $Id: ecommerce_order_edit_fns.inc,v 1.13 2012/08/30 00:58:33 ewang Exp $
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
* @version $Revision: 1.13 $
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
			echo translate('None');

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

		return FALSE;

	}//end paintOrderSummary()


	/**
	* Paints where this order has been
	*
	* @param object	Asset				&$rootid	asset being painted
	* @param object Backend_Outputter	&$o			backend outputter
	* @param string						$prefix		prefix for the html doc element name
	*
	* @return boolean
	* @access public
	*/
	function paintOrderHistory(&$asset, &$o, $prefix)
	{
		$order_history = $asset->attr('order_history');
		?>
			<table class="sq-backend-table">
				<tr>
					<td class="sq-backend-table-header-header">Time</td>
					<td class="sq-backend-table-header-header">Activity</td>
				</tr>
		<?php
			foreach ($order_history as $time => $activity) {
				echo '<tr>';
					echo '<td>'.$time.'</td>';
					echo '<td>'.$activity.'</td>';
				echo '</tr>';
			}
			echo '</table>';

		return FALSE;

	}//end paintOrderHistory()


	/**
	* Paints the transaction ID generated during payment processing
	*
	* @param object	Asset				&$rootid	asset being painted
	* @param object Backend_Outputter	&$o			backend outputter
	* @param string						$prefix		prefix for the html doc element name
	*
	* @return boolean
	* @access public
	*/
	function paintEcomRefNo(&$asset, &$o, $prefix)
	{
		echo '<pre>'.$asset->attr('ecom_ref_no').'</pre>';

		return FALSE;

	}//end paintEcomRefNo()


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

		return FALSE;

	}//end paintOrderStatus()


	/**
	* Paints the transaction ID generated during payment processing
	*
	* @param object	Asset				&$rootid	asset being painted
	* @param object Backend_Outputter	&$o			backend outputter
	* @param string						$prefix		prefix for the html doc element name
	*
	* @return boolean
	* @access public
	*/
	function paintTransactionID(&$asset, &$o, $prefix)
	{
		echo '<pre>'.$asset->attr('transaction_id').'</pre>';

		return FALSE;

	}//end paintTransactionID()


	/**
	* Paints the XML representation of ecommerce order
	*
	* @param object	Asset				&$rootid	asset being painted
	* @param object Backend_Outputter	&$o			backend outputter
	* @param string						$prefix		prefix for the html doc element name
	*
	* @return boolean
	* @access public
	*/
	function paintOrderXML(&$asset, &$o, $prefix)
	{
		$order_xml = $asset->attr('order_xml');

		if ($asset->writeAccess('attributes')) {
			text_area($prefix.'_order_xml', $order_xml, 80, 30);
		} else {
			echo '<pre>'.htmlspecialchars($order_xml).'</pre>';
		}

		return FALSE;

	}//end paintOrderXML()



	/**
	* Paints the XML representation of ecommerce order
	*
	* @param object	Asset				&$rootid	asset being painted
	* @param object Backend_Outputter	&$o			backend outputter
	* @param string						$prefix		prefix for the html doc element name
	*
	* @return boolean
	* @access public
	*/
	function paintTransactionResponse(&$asset, &$o, $prefix)
	{
		$order_xml = $asset->attr('transaction_response');

		echo '<pre>'.htmlspecialchars($order_xml).'</pre>';

		return FALSE;

	}//end paintTransactionResponse()



}//end class

?>