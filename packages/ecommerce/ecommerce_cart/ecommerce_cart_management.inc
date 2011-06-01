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
* $Id: ecommerce_cart_management.inc,v 1.13 2008/11/25 00:48:03 mbrydon Exp $
*
*/


require_once SQ_INCLUDE_PATH.'/asset_management.inc';

/**
* Ecommerce_Cart_Management
*
* @see Asset_Management
*
* @author  Luke Wright <lwright@squiz.net>
* @version $Revision: 1.13 $
* @package MySource_Matrix_Packages
* @subpackage ecommerce
*/
class Ecommerce_Cart_Management extends Asset_Management
{


	/**
	* Constructor
	*
	*/
	function Ecommerce_Cart_Management(&$pm)
	{
		$this->Asset_Management($pm);

		$this->vars = Array(
						'name'					=> Array(
													'added'			=> '0.0.1',
													'type'			=> 'text',
													'default'		=> '',
													'description'	=> 'The full name of the cart',
													'update_fn'		=> '',
												   ),
						'short_name'			=> Array(
													'added'			=> '0.0.1',
													'type'			=> 'text',
													'default'		=> '',
													'description'	=> 'The short name of the cart',
													'update_fn'		=> '',
												   ),
						'ecommerce_checkout_id'	=> Array(
													'added'			=> '0.0.1',
													'type'			=> 'assetid',
													'default'		=> 0,
													'description'	=> 'The assetid of the ecommerce checkout asset that we are connected with',
												   ),
						'show_price'			=> Array(
													'added'			=> '0.0.1',
													'type'			=> 'boolean',
													'default'		=> TRUE,
													'description'	=> 'Whether to show the price columns',
													'parameters'	=> Array(
																		'allow_empty'	=> FALSE,
																	   ),
													'update_fn'		=> '',
												   ),
						'checkout_text'			=> Array(
													'added'			=> '0.0.1',
													'type'			=> 'text',
													'default'		=> 'Proceed to Checkout',
													'description'	=> 'The text on the "checkout" button',
												   ),
						'cart_updated_text'		=> Array(
													'added'			=> '0.0.1',
													'type'			=> 'text',
													'default'		=> 'Your Cart has been Updated',
													'description'	=> 'The text displayed when the cart is updated',
												   ),
						'no_items_text'			=> Array(
													'added'			=> '0.2',
													'type'			=> 'text',
													'default'		=> 'There are currently no items in your cart',
													'description'	=> 'The text that displays instead of the asset listing, when you have no items in your cart',
												   ),
						'parameter_map'			=> Array(
													'added'			=> '0.4',
													'type'			=> 'parameter_map',
													'default'		=> Array(),
													'description'	=> 'Dynamically maps an request var to an attribute',
													'parameters'	=> Array(
																		'parameters'	=> Array(
																							'add_item'	=> 'Add an item to the cart',
																						   ),
																	   ),
												   ),
						'asset_selections'		=> Array(
													'added'			=> '0.3',
													'type'			=> 'serialise',
													'default'		=> Array(
																		'Quantity'	=> Array(
																						'type'		=> 'textbox',
																						'options'	=> Array(
																										'allow_multiple'	=> 0,
																										'allow_negative'	=> 0,
																										'restrict_range'	=> Array(
																																'enable'	=> 0,
																																'min'		=> 0,
																																'max'		=> 10,
																															   ),
																									   ),
																						'permanent'	=> 1,
																					   ),
																		'Remove'	=> Array(
																						'type'		=> 'radio',
																						'options'	=> Array(
																										'allow_multiple'	=> 1,
																										'allow_negative'	=> 0,
																										'restrict_range'	=> Array(
																																'enable'	=> 0,
																																'min'		=> 0,
																																'max'		=> 10,
																															   ),
																									   ),
																						'permanent'	=> 1,
																					   ),
																	   ),
													'description'	=> 'Sets of named checkboxes, drop-down lists, etc',
												   ),
						'use_local_cart'		=> Array(
													'added'			=> '0.4',
													'type'			=> 'boolean',
													'default'		=> FALSE,
													'description'	=> 'If yes, use the ecommerce_checkout local cart instead of the global cart',
													'parameters'	=> Array(
																		'allow_empty'	=> FALSE,
																	   ),
													'update_fn'		=> '',
												   ),
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
														'link_type'		=> SQ_LINK_TYPE_2 | SQ_LINK_TYPE_1,
													   ),
				),
			   );

	}//end getEventList()


}//end class

?>