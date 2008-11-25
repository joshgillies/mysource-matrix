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
* $Id: order.inc,v 1.8 2008/11/25 00:48:05 mbrydon Exp $
*
*/

require_once SQ_PACKAGES_PATH.'/cms/form/form_submission/form_submission.inc';
require_once SQ_PACKAGES_PATH.'/ecommerce/orders/order_line/order_line.inc';

/**
* Order
*
* Purpose
*
*
* @author  Luke Wright <lwright@squiz.net>
* @version $Revision: 1.8 $
* @package MySource_Matrix_Packages
* @subpackage ecommerce
*/
class Order extends Form_Submission
{


	/**
	* Constructor
	*
	* @param string	$assetid	the assetid of the asset to load into this object
	*
	* @return void
	* @access public
	*/
	function Order($assetid=0)
	{
		$this->_ser_attrs = true;
		$this->Asset($assetid);

	}//end constructor


	/**
	* Returns an array of all the permitted link type, the type asset and
	* the cardinality
	*
	* @see Asset::_getAllowLinks()
	* @access private
	*/
	function _getAllowedLinks()
	{
		$order_links = parent::_getAllowedLinks();
		$order_links[SQ_LINK_TYPE_3]['order_line'] = Array('card' => 'M');
		return $order_links;

	}//end _getAllowedLinks()


	/**
	* Calculate a simple total of this order
	*
	* @return double
	* @access public
	*/
	function total()
	{
		$total = 0.0;

		$lines = $this->getLines();
		foreach($lines as $assetid) {
			$line =& $GLOBALS['SQ_SYSTEM']->am->getAsset($assetid);
			$total += $line->attr('quantity') * $line->attr('price');
		}

		return $total;

	}//end total()


	/**
	* Calculate a simple count of items of this order
	*
	* @return int
	* @access public
	*/
	function itemCount()
	{
		$count = 0.0;

		$lines = $this->getLines();
		foreach($lines as $assetid) {
			$line =& $GLOBALS['SQ_SYSTEM']->am->getAsset($assetid);
			$count += $line->attr('quantity');
		}

		return $count;

	}//end itemCount()


	/**
	* Calculate a simple count of lines in this order
	*
	* @return int
	* @access public
	*/
	function lineCount()
	{
		$lines = $this->getLines();
		return count($lines);

	}//end lineCount()


	/**
	* Gets an array of order lines residing underneath this asset
	*
	* Order lines are to be TYPE_3 linked to its parent order, as it does not
	* need to be separately viewed on the backend
	*
	* @return Array(string)	an array of assetids
	* @access public
	*/
	function getLines()
	{
		$links = $GLOBALS['SQ_SYSTEM']->am->getLinks($this->id, SQ_LINK_TYPE_3, 'order_line', false);

		$assetids = Array();

		foreach($links as $link) {
			$assetids[] = $link['minorid'];
		}

		return $assetids;

	}//end getLines()


	/**
	* Creates an Order Line asset and links it underneath this asset
	*
	* @param int $product_assetid   the assetid of the product this orderline represents
	* @param int $price				the per item price of the product
	* @param int $quantity			quantity of product requested   default: 1
	*
	* @return boolean				indicating success
	* @access public
	*/
	function createLine($product_assetid, $price, $quantity=1)
	{
		// create and populate a new order line
		$new_order_line = new Order_Line();

		$new_order_line->setAttrValue('quantity', $quantity);
		$new_order_line->setAttrValue('price', $price);
		$new_order_line->setAttrValue('product_assetid', $product_assetid);

		// prepare type 3 link between order and orderline
		$link = array('asset' => &$this, 'link_type' => SQ_LINK_TYPE_3, 'is_dependant' => 1, 'is_exclusive' =>  1);

		// attempt to link orderline to order
		$GLOBALS['SQ_SYSTEM']->changeDatabaseConnection('db2');
		$GLOBALS['SQ_SYSTEM']->doTransaction('BEGIN');
		$GLOBALS['SQ_SYSTEM']->setRunLevel(SQ_RUN_LEVEL_FORCED);
		$success = $new_order_line->create($link);
		$GLOBALS['SQ_SYSTEM']->restoreRunLevel();

		if ($success) {
			$GLOBALS['SQ_SYSTEM']->doTransaction('COMMIT');
		} else {
			$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
		}

		$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();
		return $success;

	}//end createLine()


	/**
	* Build an order based on items in a cart
	*
	* @param object Ecommerce_Cart	&$cart		the cart to convert
	* @param boolean				$clear_after	whether the cart should be
	*												cleared after transferring
	*
	* @return boolean	indicating success
	* @access public
	*/
	function buildLinesFromCart($cart_items, $clear_after=true)
	{
		// cart items is passed in straight from $_SESSION['sq_cart_contents']
		foreach($_SESSION['sq_cart_contents'] as $product_assetid => $product_info) {
			$this->createLine($product_assetid, $product_info['price'], $product_info['quantity']);
		}

		// are we clearing the cart afterwards
		if ($clear_after) {
			unset($_SESSION['sq_cart_contents']);
		}

		return true;

	}//end buildLinesFromCart()


}//end class

?>