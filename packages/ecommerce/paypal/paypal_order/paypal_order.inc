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
* $Id: paypal_order.inc,v 1.3 2012/08/30 00:58:34 ewang Exp $
*
*/


require_once SQ_INCLUDE_PATH.'/asset.inc';

/**
* Paypal_Order
*
* Purpose
*
*
* @author  Anh Ta <ata@squiz.co.uk>
* @version $Revision: 1.3 $
* @package MySource_Matrix_Packages
* @subpackage ecommerce
*/
class Paypal_Order extends Asset
{


	/**
	* Constructor
	*
	* @param string	$assetid	the asset id to be loaded
	*
	*/
	function __construct($assetid=0)
	{
		$this->_ser_attrs = TRUE;
		parent::__construct($assetid);

	}//end constructor

	
	/**
	* Perform any additional processing required during the creation of this asset
	*
	* Paypal Orders set an initial web path when they are created
	*
	* @param array	&$link	information used to create the initial link
	*
	* @return boolean
	* @access private
	*/
	protected function _createAdditional(Array &$link)
	{
		if (!parent::_createAdditional($link)) return FALSE;

		return $this->makeAndSaveInitialWebPath('paypal_order_'.$this->id, $link);

	}//end _createAdditional()
	
	
	/**
	* Return the keywords for this asset type.
	*
	* This function is called in asset-listing situations like the asset listing, search page etc
	* The return value is in the form:
	* <PRE>
	* Array(
	*   'name' => 'description' => 'the asset name',
	* );
	* </PRE>
	*
	* @return array
	* @access public
	*/
	public function getAvailableKeywords()
	{
		$keywords = parent::getAvailableKeywords();

		$keywords['asset_attribute_summary'] = 'The summary of the Paypal order';
		
		return $keywords;

	}// end getAvailableKeywords()
	
	
	/**
	* Get bodycopy keyword replacement
	* 
	* @param string	$keyword	Keyword to find a replacement for
	*
	* @return string
	* @access public
	* @see _getKeywordReplacement
	*/
	public function getKeywordReplacement($keyword)
	{
		if ($keyword == 'asset_attribute_summary') {
			return $this->getProductSummary();
		}
		
		return parent::getKeywordReplacement($keyword);
		
	}//end getKeywordReplacement()
	
	
	/**
	 * Return the summary of the products' details
	 * 
	 * @return string	The summary of products
	 * @access public
	 */
	public function getProductSummary()
	{
		$products = $this->attr('products');
		$products_string = Array();
		
		foreach ($products as $product) {
			$product_string = Array();
			$product_number = ($product['number'] != '') ? ' (#'.$product['number'].')' : '';
			$product_string['name'] = $product['name'].$product_number;
			$product_string['quantity'] = sprintf('%d', $product['quantity']);
			$product_string['price'] = sprintf('%01.2f', $product['price']);
			$product_string['subtotal'] = sprintf('%01.2f', $product['subtotal']);
			$products_string[] = $product_string;
		}
		
		$name_padding_len = $this->_getPaddingLength($products_string, 'name');
		$quantity_padding_len = $this->_getPaddingLength($products_string, 'quantity');
		$price_padding_len = $this->_getPaddingLength($products_string, 'price');
		$subtotal_padding_len = $this->_getPaddingLength($products_string, 'subtotal');
		
		$format = "%-{$name_padding_len}s%-{$quantity_padding_len}s%-{$price_padding_len}s%-{$subtotal_padding_len}s\n";
		$split_string = str_repeat('-', $name_padding_len + $quantity_padding_len + $price_padding_len + $subtotal_padding_len)."\n";
		
		//print header
		$summary = sprintf($format, 'Name', 'Quantity', 'Price', 'Subtotal');
		$summary .= $split_string;
		
		//print products' details
		foreach ($products_string as $product_string) {
			$summary .= sprintf($format, $product_string['name'], $product_string['quantity'], $product_string['price'], $product_string['subtotal']);
		}
		
		//print total price
		$summary .= $split_string;
		$total = sprintf('%01.2f', $this->attr('total_amount')).' '.$this->attr('currency');
		$summary .= sprintf($format, 'Total', '', '', $total);
		
		return '<pre>'.htmlspecialchars($summary).'</pre>';
		
	}//end getProductSummary()
	
	
	/**
	 * Return the maximum lenth of the $index string in products string. This private method is used in getProductSummary() method
	 * 
	 * @param array		$products_string	The products string array
	 * @param string	$index				The index or key name of the product string array
	 * @return int							The maximum length of the $index string
	 * @access private
	 */
	private function _getPaddingLength($products_string, $index)
	{
		$max_length = strlen($index);
		foreach ($products_string as $product_string) {
			if (strlen($product_string[$index]) > $max_length) {
				$max_length = strlen($product_string[$index]);
			}
		}
		
		return $max_length + 2; //add 2 more padding characters for better view
		
	}//end _getPaddingLength()

}//end class
?>