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
* $Id: ecommerce_checkout_management.inc,v 1.5.4.3 2008/04/22 00:23:23 bshkara Exp $
*
*/

require_once SQ_INCLUDE_PATH.'/asset_management.inc';

/**
* Ecommerce_Checkout_Management
*
* @see Asset_Management
*
* @author  Andrei Railean <arailean@squiz.net>
* @version $Revision: 1.5.4.3 $
* @package MySource_Matrix_Packages
* @subpackage ecommerce
*/
class Ecommerce_Checkout_Management extends Asset_Management
{


	/**
	* Constructor
	*
	*/
	function Ecommerce_Checkout_Management(&$pm)
	{
		$this->Asset_Management($pm);

		$this->vars['go_to_confirmation_button_text'] = Array(
															'added'			=> '0.1',
															'type'			=> 'text',
															'default'		=> 'Go To Confirmation',
															'description'	=> 'Go To Confirmation Button Text',
														);

		$this->vars['confirm_order_button_text'] = Array(
													'added'			=> '0.1',
													'type'			=> 'text',
													'default'		=> 'Confirm Order',
													'description'	=> 'Confirm Order Button Text',
												   );

		$this->vars['return_to_checkout_button_text'] = Array(
															'added'			=> '0.1',
															'type'			=> 'text',
															'default'		=> 'Return To Checkout',
															'description'	=> 'Return To Checkout Button Text',
														);

		// taxation variables
		$this->vars['taxation_name'] = Array(
										'added'			=> '0.2',
										'type'			=> 'text',
										'default'		=> 'GST',
										'description'	=> 'Taxation Name',
									   );

		$this->vars['taxation_rate'] = Array(
										'added'			=> '0.2',
										'type'			=> 'int',
										'default'		=> 10,
										'parameters'	=> Array(
															'allow_empty'		=> FALSE,
															'allow_negative'	=> FALSE,
															'range_lower'		=> 0,
															'range_upper'		=> 100,
														   ),
									   );

		$this->vars['flat_charge_assetid'] = Array(
												'added'		=> '0.3',
												'type'		=> 'serialise',
												'default'	=> Array(),
											 );

		$this->vars['tax_charge_assetid'] = Array(
												'added'		=> '0.3',
												'type'		=> 'serialise',
												'default'	=> Array(),
											);

		$this->vars['quantity_attribute'] = Array(
												'added'		=> '0.3',
												'type'		=> 'text',
												'default'	=> '',
											);

		$this->vars['quantity_field'] = Array(
											'added'		=> '0.3',
											'type'		=> 'assetid',
											'default'	=> '',
										);

		$this->vars['enforce_quantities'] = Array(
												'added'			=> '0.3',
												'type'			=> 'boolean',
												'default'		=> 'FALSE',
												'parameters'	=> Array(
																	'allow_empty'	=> FALSE,
																   ),
											);

		$this->vars['quantity_message_cart'] = Array(
												'added'		=> '0.3',
												'type'		=> 'text',
												'default'	=> 'There is an insufficient amount of items in stock to proceed with your order. Your order quantities have been altered to reflect the amount available. Items which are currently not in stock have been deleted from your cart. These items with insufficient quantities are listed below:',
											   );

		$this->vars['quantity_message_checkout'] = Array(
													'added'		=> '0.3',
													'type'		=> 'text',
													'default'	=> 'There is an insufficient amount of items in stock to proceed with your order. Your order has not been processed. The items with insufficient quantities are listed below:',
												   );

		$this->vars['country_question'] = Array(
											'added'		=> '0.3',
											'type'		=> 'assetid',
											'default'	=> '',
											'is_admin'	=> 'TRUE',
										  );

		$this->vars['taxable_countries'] = Array(
												'added'		=> '0.3',
												'type'		=> 'serialise',
												'default'	=> Array(),
												'is_admin'	=> 'TRUE',
											   );

	}//end constructor


	/**
	* Return an array with information about the events that this page is listening for
	*
	* @return boolean
	* @access public
	*/
	function getEventList()
	{
		return Array(
				Array(
					'event_name'					=> 'requestKeywords',
					'broadcast_type_code'			=> 'content_type',
					'broadcast_strict_type_code'	=> FALSE,
					'options'						=> Array(
														'side_of_link'	=> 'major',
														'indirect'		=> TRUE,
														'is_exclusive'	=> NULL,
														'is_dependant'	=> 1,
														'value'			=> '',
														'link_type'		=> SQ_LINK_TYPE_2,
													   ),
				),
			   );

	}//end getEventList()


	/**
	* upgrade function
	*
	* @param string $current_version	current version of the asset
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
