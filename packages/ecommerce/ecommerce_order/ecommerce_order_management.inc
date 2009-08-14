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
* $Id: ecommerce_order_management.inc,v 1.13 2009/08/14 10:16:48 ata Exp $
*
*/


require_once SQ_INCLUDE_PATH.'/asset_management.inc';

/**
* Ecommerce_Order_Management
*
* @see Asset_Management
*
* @author  Scott Kim <skim@squiz.net>
* @version $Revision: 1.13 $
* @package MySource_Matrix_Packages
* @subpackage ecommerce
*/
class Ecommerce_Order_Management extends Asset_Management
{


	/**
	* Constructor
	*
	*/
	function Ecommerce_Order_Management(&$pm)
	{
		$this->Asset_Management($pm);

		$this->vars = Array(
						'summary'			=> Array(
												'added'		=> '0.1',
												'type'		=> 'text',
												'default'	=> '',
											   ),
						'order_history'		=> Array(
												'added'		=> '0.2',
												'type'		=> 'serialise',
												'default'	=> Array(),
											   ),
						'status'			=> Array(
												'added'		=> '0.1',
												'type'		=> 'int',
												'default'	=> 0,
											   ),
						'status_message'	=> Array(
												'added'		=> '0.1',
												'type'		=> 'text',
												'default'	=> '',
											   ),
						'products'			=> Array(
												'added'		=> '0.1',
												'type'		=> 'serialise',
												'default'	=> Array(),
											   ),
						'delivery_id'		=> Array(
												'added'		=> '0.1',
												'type'		=> 'assetid',
												'default'	=> '0',
											   ),
						'delivery_state'	=> Array(
												'added'		=> '0.1',
												'type'		=> 'serialise',
												'default'	=> Array(),
											   ),
						'order_form_value'	=> Array(
												'added'		=> '0.2',
												'type'		=> 'serialise',
												'default'	=> Array(),
											   ),
						'ecom_ref_no'		=> Array(
												'added'		=> '0.2',
												'type'		=> 'text',
												'default'	=> '',
												'is_admin'	=> FALSE,
											   ),
						'transaction_id'	=> Array(
													'added'			=> '0.2',
													'type'			=> 'text',
													'default'		=> '',
													'description'	=> 'The ID of the payment transaction returned by the Payment Gateway',
												   ),
						'billing_name'		=> Array(
													'added'			=> '0.2',
													'type'			=> 'text',
													'default'		=> '',
													'description'	=> 'The billing name returned by the Payment Gateway',
												   ),
						'billing_addr'		=> Array(
													'added'			=> '0.2',
													'type'			=> 'text',
													'default'		=> '',
													'description'	=> 'The billing address returned by the Payment Gateway',
												   ),
						'delivery_name'		=> Array(
													'added'			=> '0.2',
													'type'			=> 'text',
													'default'		=> '',
													'description'	=> 'The delivery name returned by the Payment Gateway',
												   ),
						'delivery_addr'		=> Array(
													'added'			=> '0.2',
													'type'			=> 'text',
													'default'		=> '',
													'description'	=> 'The delivery address returned by the Payment Gateway',
												   ),
						'order_xml'			=> Array(
													'added'			=> '0.2',
													'type'			=> 'text',
													'default'		=> '',
													'description'	=> 'The XML representation of this Order asset',
												   ),
					  );

	}//end constructor


	/**
	* upgrade function
	*
	* @param string	$current_version	current version of the asset
	*
	* @return boolean
	* @access public
	*/
	function _upgrade($current_version)
	{
		// first do things as normal
		$res = parent::_upgrade($current_version);

		return $res;

	}//end _upgrade()


}//end class

?>