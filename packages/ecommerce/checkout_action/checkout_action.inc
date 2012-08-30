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
* $Id: checkout_action.inc,v 1.2 2012/08/30 00:58:33 ewang Exp $
*
*/

require_once SQ_INCLUDE_PATH.'/asset.inc';


/**
* Checkout_Action
*
* @author  Chiranjivi Upreti <cupreti@squiz.com.au>
* @version $Revision: 1.2 $
* @package MySource_Matrix_Packages
* @subpackage ecommerce
*/
class Checkout_Action extends Asset
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
	* Paint inline interface
	*
	*
	*/
	public static function paintInlineInterface(Ecommerce_Checkout &$checkout, $settings, Backend_Outputter $o, $prefix)
	{


	}//end paintInlineInterface()


	/**
	* Process inline interface
	*
	*
	*/
	public static function processInlineInterface(Ecommerce_Checkout &$checkout, &$settings, Backend_Outputter $o, $prefix)
	{


	}//end processInlineInterface()


	/**
	* Paint summary description
	*
	*
	*/
	public static function paintSummary(Ecommerce_Checkout &$checkout, $settings, Backend_Outputter $o, $prefix)
	{


	}//end paintSummary()


	/**
	* Is this action valid?
	*
	*
	*/
	public static function isValid(Ecommerce_Checkout &$checkout, $settings)
	{


	}//end paintSummary()



	/**
	* Execute form action
	*
	*/
	public static function execute(Ecommerce_Checkout &$checkout, Ecommerce_Order &$order, $settings)
	{


	}//end execute()


}//end class

?>