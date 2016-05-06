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
* $Id: ecommerce_order.inc,v 1.20 2012/08/30 00:58:33 ewang Exp $
*
*/

require_once SQ_INCLUDE_PATH.'/asset.inc';

define('SQ_ECOM_ORDER_STATUS_PROCESSING', 0);
define('SQ_ECOM_ORDER_STATUS_SUCCESS', 1);
define('SQ_ECOM_ORDER_STATUS_FAILED', 2);


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
	* Constructor
	*
	* @param string	$assetid	the assetid of the asset to load into this object
	*
	* @return void
	* @access public
	*/
	function Ecommerce_Order($assetid=0)
	{
		$this->_ser_attrs = true;
		$this->Asset($assetid);

	}//end constructor


	/**
	* Perform any additional processing required during the creation of this asset
	*
	* Orders add a default web path when they are created
	*
	* @param array	&$link	information used to create the initial link
	*
	* @access private
	* @return boolean
	*/
	function _createAdditional(&$link)
	{
		if (!parent::_createAdditional($link)) return false;

		// set an initial web path
		$initial_path = $this->id;
		require_once SQ_INCLUDE_PATH.'/general_occasional.inc';
		$valid_paths = make_valid_web_paths(Array($initial_path));
		$good_paths = $GLOBALS['SQ_SYSTEM']->am->webPathsInUse($link['asset'], $valid_paths, $this->id, true);
		return $this->saveWebPaths($good_paths);

	}//end _createAdditional()


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
	* @param boolean $filter whether to filter the value content.
	*
	* @return boolean
	* @access public
	*/
	function setAttrValue($name, $value, $filter=NULL)
	{
		switch ($name) {
			case 'products':
				$GLOBALS['SQ_SYSTEM']->changeDatabaseConnection('db2');
				$GLOBALS['SQ_SYSTEM']->doTransaction('BEGIN');

				$this->_linkProducts($value);

				$GLOBALS['SQ_SYSTEM']->doTransaction('COMMIT');
				$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();
			break;
		}
		return parent::setAttrValue($name, $value, $filter);

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
			trigger_localised_error('ECOM0014', translate('Ecommerce Order can not be created without products'), E_USER_WARNING);
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
		foreach ($products as $id => $value) {
			// put notice link
			$product = $GLOBALS['SQ_SYSTEM']->am->getAsset($id);

			if ($GLOBALS['SQ_SYSTEM']->am->createAssetLink($this, $product, SQ_LINK_NOTICE) == 0) {
				trigger_localised_error('ECOM0019', translate('Ecommerce Order can not create Notice link to Product'), E_USER_WARNING);
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
	* Set the status of order
	*
	* @param $status	status constant defined in order
	*
	* @return boolean
	* @access public
	*/
	function setOrderStatus($status, $message='')
	{
		// set status
		if (is_numeric($status)) {

			switch ($status) {
				case SQ_ECOM_ORDER_STATUS_PROCESSING :
					$new_status = SQ_ECOM_ORDER_STATUS_PROCESSING;
					if (empty($message)) {
						$status_message = translate('Processing');

					} else {
						$status_message = $message;
					}
				break;
				case SQ_ECOM_ORDER_STATUS_SUCCESS :
					$new_status = SQ_ECOM_ORDER_STATUS_SUCCESS;
					$status_message = translate('Completed');

				break;
				case SQ_ECOM_ORDER_STATUS_FAILED :
					$new_status = SQ_ECOM_ORDER_STATUS_FAILED;
					$status_message = translate('Failed');

				break;
				default:
					return false;
				break;
			}

			$success = true;

			$GLOBALS['SQ_SYSTEM']->changeDatabaseConnection('db2');
			$GLOBALS['SQ_SYSTEM']->doTransaction('BEGIN');

			// set runlevel
			$GLOBALS['SQ_SYSTEM']->setRunLevel(SQ_RUN_LEVEL_FORCED);

			// set status
			$this->setAttrValue('status', $new_status);

			// set status message
			if (!empty($status_message)) {
				$this->setAttrValue('status_message', $status_message);
			} else {
				$success = false;
			}

			// save attributes
			if (!$GLOBALS['SQ_SYSTEM']->am->acquireLock($this->id, 'attributes')) {
				$success = false;
			} else if (!$this->saveAttributes()) {
				$success = false;
			}
			$GLOBALS['SQ_SYSTEM']->am->releaseLock($this->id, 'attributes');

			// restore runlevel
			$GLOBALS['SQ_SYSTEM']->restoreRunLevel();

			if ($success) {
				$GLOBALS['SQ_SYSTEM']->doTransaction('COMMIT');
			} else {
				$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
			}

			$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();
			return $success;

		} else {
			return false;
		}

	}//end setOrderStatus()


	/**
	* Return the status of order
	*
	* @return array
	* @access public
	*/
	function getOrderStatus()
	{
		$result = Array(
					'status'			=> $this->attr('status'),
					'status_message'	=> $this->attr('status_message'),
				  );

		return $result;

	}//end getOrderStatus()


//--        KEYWORDS        --//


	/**
	* Return the keywords for this asset type.
	*
	* This function is called in order to get the list of available keywords
	* for this asset.
	*
	* The return value is in the form:
	* <pre>
	* Array(
	*   'keyword' => 'description'
	* );
	* </pre>
	*
	* @access public
	* @return array
	*/
	function getAvailableKeywords()
	{
		$keywords = parent::getAvailableKeywords();

		$keywords['checkout_assetid']	= translate('Assetid of checkout form which created this order');

		$keywords['order_summary']		= translate('Summary of this order');

		$keywords['status']				= translate('Status of ecommerce order');

		$keywords['status_message']		= translate('Message which is related to the status');

		$keywords['edit_url']			= translate('URL to the page where to edit this ecommerce order');

		$keywords['order_xml']			= translate('XML representation of this order');

		$keywords['order_grand_total']	= translate('The grand total price of this Order asset');


		return $keywords;

	}//end getAvailableKeywords()


	/**
	* Get checkout asset id
	*
	* Get the asset id of checkout that created this order
	*
	* @return int
	* @access private
	*/
	function _getCheckoutAssetId()
	{
		if (!isset($this->_tmp['checkout_asset'])) {
			$link_to_checkout = $GLOBALS['SQ_SYSTEM']->am->getLink($this->id, SQ_LINK_NOTICE, 'ecommerce_checkout', true, 'checkout', 'minor');
			$this->_tmp['checkout_asset'] = array_get_index($link_to_checkout, 'majorid', 0);
		}

		return $this->_tmp['checkout_asset'];

	}//end _getCheckoutAssetId()


	/**
	* Get checkout_assetid keyword replacement
	*
	* This function assumes there is only one checkout form associated with this ecommerce order.
	*
	* @return int
	* @access public
	*/
	function getCheckoutAssetidKeywordReplacement()
	{
		return $this->_getCheckoutAssetId();

	}//end getCheckoutAssetidKeywordReplacement()


	/**
	* Get order_summary keyword replacement
	*
	* @return string
	* @access public
	*/
	function getOrderSummaryKeywordReplacement()
	{
		return $this->attr('summary');

	}//end getOrderSummaryKeywordReplacement()


	/**
	* Get status keyword replacement
	*
	* @return int
	* @access public
	*/
	function getStatusKeywordReplacement()
	{
		return $this->attr('status');

	}//end getStatusKeywordReplacement()


	/**
	* Get status_message keyword replacement
	*
	* @return int
	* @access public
	*/
	function getStatusMessageKeywordReplacement()
	{
		return $this->attr('status_message');

	}//end getStatusMessageKeywordReplacement()


	/**
	* Get edit_url keyword replacement
	*
	* @return int
	* @access public
	*/
	function getEditUrlKeywordReplacement()
	{
		// get ecommerce checkout asset id
		$id = $this->_getCheckoutAssetId();

		// get url of checkout form and append extra variable
		$url = $GLOBALS['SQ_SYSTEM']->am->getAssetURL($id);

		$connector = '';
		if (empty($url)) {
			return '';
		} else {
			strstr($url, '?') ? $connector = '&' : $connector = '?';
		}
		$url .= $connector.'edit_order_id='.$this->id;

		return '<a href="'.$url.'">'.$url.'</a>';

	}//end getEditUrlKeywordReplacement()


	/**
	* Get order_xml keyword replacement
	*
	* @return string
	* @access public
	*/
	function getOrderXmlKeywordReplacement()
	{
		return $this->attr('order_xml');

	}//end getOrderXmlKeywordReplacement()


	/**
	* Get order_grand_total keyword replacement
	*
	* @return int
	* @access public
	*/
	function getOrderGrandTotalKeywordReplacement()
	{
		$order_xml = trim($this->attr('order_xml'));
		if (!empty($order_xml)) {
			//Prior to the commit 'c6011db', the asset attribute values were stored without being encoded
			$xml_string = strpos($order_xml, '&lt;') === 0 ? htmlspecialchars_decode($this->attr('order_xml'), ENT_QUOTES) : $order_xml;

			$xml_obj = @new SimpleXMLElement($xml_string);
			if ($xml_obj) {
				$grand_total = isset($xml_obj->cart->grand_total) ? $xml_obj->cart->grand_total : '';
				return $grand_total;
			}
		}

		return '';

	}//end getCheckoutAssetidKeywordReplacement()


	/**
	* Get the delivery method related to this order, if set
	*
	* @return object Ecommerce_Delivery_Method or null
	* @access public
	*/
	function &getDeliveryMethod()
	{
		$method = null;
		if ($this->attr('delivery_id') != 0) {
			$method =& $GLOBALS['SQ_SYSTEM']->am->getAsset($this->attr('delivery_id'));
		}

		return $method;

	}//end getDeliveryMethod()


	/**
    * Append a message to the Audit History Log of this Order
    *
    * @param $msg	The message to be added
    * @returns void
    * @access public
    */
    function appendAuditLog($msg)
    {
        $order_history = $this->attr('order_history');
        $index = date('r');
        while (isset($order_history[$index])) {
            $index .= ' ';
        }
        $order_history[$index] = $msg;
        $this->setAttrValue('order_history', $order_history);

        $GLOBALS['SQ_SYSTEM']->setRunLevel(SQ_RUN_LEVEL_FORCED);

        	$this->saveAttributes();

        $GLOBALS['SQ_SYSTEM']->restoreRunLevel();

    }//end appendAuditLog()


}//end class

?>
