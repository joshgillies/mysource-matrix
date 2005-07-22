<?php
/**
* +--------------------------------------------------------------------+
* | Squiz.net Commercial Module Licence                                |
* +--------------------------------------------------------------------+
* | Copyright (c) Squiz Pty Ltd (ACN 084 670 600).                     |
* +--------------------------------------------------------------------+
* | This source file is not open source or freely usable and may be    |
* | used subject to, and only in accordance with, the Squiz Commercial |
* | Module Licence.                                                    |
* | Please refer to http://www.squiz.net/licence for more information. |
* +--------------------------------------------------------------------+
*
* $Id: ecommerce_order.inc,v 1.1 2005/07/22 07:28:55 skim Exp $
* $Name: not supported by cvs2svn $
*/

require_once SQ_INCLUDE_PATH.'/asset.inc';

/**
* Ecommerce Order
*
* Purpose
*
* @author Scott Kim <skim@squiz.net>
* @version $Revision:
* @package MySource_Matrix_Packages
* @subpackage ecommerce
*/
class Ecommerce_Order extends Asset
{

	/**
	* Container for products information
	*/
	var $products;


	/**
	* Container for order information e.g. payment, delivery and so on
	*/
	var $processing_data;


	/**
	* Constructor
	*
	* @param string	$assetid	the assetid of the asset to load into this object
	*
	* @return void
	* @access public
	*/
	function Ecommerce_Order($assetid=0)
	{
		$this->products = Array();
		$this->processing_data = Array();

		$this->_ser_attrs = true;
		$this->Asset($assetid);

	}//end constructor


	/**
	* Returns an array of all the permitted links type, the type asset and the cardinality
	* In the form
	*
	*   Array('[link_type]' => Array('[type_code]' => Array('card' => [cardinality], 'exclusive' => [exclusive])));
	* Where:
	*   link_type   = SQ_LINK_TYPE_1|SQ_LINK_TYPE_2|SQ_LINK_TYPE_3|SQ_LINK_NOTICE
	*   cardinality = [max number of links]|'M'
	*   exclusive   = true|false
	*
	* @return array
	* @access private
	*/
	function _getAllowedLinks()
	{
		$allowed_links = parent::_getAllowedLinks();
		$allowed_links[SQ_LINK_NOTICE]['product'] = Array('card' => 'M', 'exclusive' => false);

		return $allowed_links;

	}//end _getAllowedLinks()


	/**
	* Create this asset
	*
	* @param array	&$link		information used to create the initial link
	*
	* @see Asset::create()
	* @return mixed int or false
	* @access public
	*/
	function create(&$link)
	{
		return parent::create($link);

	}//end create()


	/**
	* Set Attribute Value
	*
	* @param $name	name of attribute
	* @param $value	value of attribute
	*
	* @return boolean
	* @access public
	*/
	function setAttrValue($name, $value)
	{
		switch ($name) {
			case 'products':
				$this->_linkProducts($value);
			break;
		}
		return parent::setAttrValue($name, $value);

	}//end setAttrValue()


	/**
	* Link products to this order
	*
	* @param $products	array of products
	*
	* @return boolean
	* @access private
	*/
	function _linkProducts($products)
	{
		if (empty($products)) {
			trigger_localised_error('ECOM0014', E_USER_WARNING);
			return false;
		}

		// set run level
		$run_level_changed = false;
		if (!$GLOBALS['SQ_SYSTEM']->runLevelEnables(SQ_RUN_LEVEL_FORCED)) {
			$GLOBALS['SQ_SYSTEM']->setRunLevel(SQ_RUN_LEVEL_FORCED);
			$run_level_changed = true;
		}

		// add items
		$success = true;
		foreach($products as $id => $value) {
			// put notice link
			$product =& $GLOBALS['SQ_SYSTEM']->am->getAsset($id);

			if ($GLOBALS['SQ_SYSTEM']->am->createAssetLink($this, $product, SQ_LINK_NOTICE) == 0) {
				trigger_localised_error('ECOM0019', E_USER_WARNING);
				$success = false;
			}
			$GLOBALS['SQ_SYSTEM']->am->forgetAsset($product);
		}

		// reset run level to old value
		if ($run_level_changed) {
			$GLOBALS['SQ_SYSTEM']->restoreRunLevel();
		}

		return $success;

	}//end _linkProducts()


	/**
	* Format the summary of this order into table for nice output
	*
	* @return string
	* @access public
	*/
	function getSummary()
	{
		return true;

	}//end getSummary()


}//end class
?>
