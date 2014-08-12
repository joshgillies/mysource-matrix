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
* $Id: paypal_ipn_receiver_edit_fns.inc,v 1.2 2012/08/30 00:58:34 ewang Exp $
*
*/


require_once SQ_CORE_PACKAGE_PATH.'/page/page_edit_fns.inc';

/**
* Paypal_IPN_Receiver_Edit_Fns
*
* Purpose
*
*
* @author  Anh Ta <ata@squiz.co.uk>
* @version $Revision: 1.2 $
* @package MySource_Matrix_Packages
* @subpackage ecommerce
*/
class Paypal_IPN_Receiver_Edit_Fns extends Page_Edit_Fns
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
	* Paint the interface for setting the target asset
	*
	* @param Site				$asset	the asset whose interface we are painting
	* @param Backend_Outputter	$o		the Backend Outputter object
	* @param string				$prefix	prefix for the form element
	*
	* @return boolean
	* @access public
	*/
	public function paintTargetAsset(Paypal_IPN_Receiver $asset, Backend_Outputter $o, $prefix)
	{
		return $this->paintNoticeLinkedAsset($asset, 'target_asset');

	}//end paintIndexPage()


	/**
	* Processes the setting of the target asset
	*
	* @param Site				$asset	the asset whose interface we are painting
	* @param Backend_Outputter	$o		the Backend Outputter object
	* @param string				$prefix	prefix for the form element
	*
	* @return boolean
	* @access public
	*/
	public function processTargetAsset(Paypal_IPN_Receiver $asset, Backend_Outputter $o, $prefix)
	{
		return $this->processNoticeLinkedAsset($asset, 'target_asset');

	}//end processIndexPage()
	
	
}//end class
?>