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
* $Id: ecommerce_checkout_management.inc,v 1.19.6.2 2013/10/02 02:09:17 cupreti Exp $
*
*/

require_once SQ_INCLUDE_PATH.'/asset_management.inc';

/**
* Ecommerce_Checkout_Management
*
* @see Asset_Management
*
* @author  Andrei Railean <arailean@squiz.net>
* @version $Revision: 1.19.6.2 $
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
															'is_contextable'	=> TRUE,
														);

		$this->vars['confirm_order_button_text'] = Array(
													'added'			=> '0.1',
													'type'			=> 'text',
													'default'		=> 'Confirm Order',
													'description'	=> 'Confirm Order Button Text',
													'is_contextable'	=> TRUE,
												   );

		$this->vars['return_to_checkout_button_text'] = Array(
															'added'			=> '0.1',
															'type'			=> 'text',
															'default'		=> 'Return To Checkout',
															'description'	=> 'Return To Checkout Button Text',
															'is_contextable'	=> TRUE,
														);

		// taxation variables
		$this->vars['taxation_name'] = Array(
										'added'			=> '0.2',
										'type'			=> 'text',
										'default'		=> 'GST',
										'description'	=> 'Taxation Name',
										'is_contextable'	=> TRUE,
									   );

		$this->vars['taxation_rate'] = Array(
										'added'			=> '0.2',
										'type'			=> 'float',
										'default'		=> 10,
										'parameters'	=> Array(
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

		$this->vars['quantity_selector'] = Array(
												'added'			=> '0.3',
												'type'			=> 'selection',
												'default'		=> 'metadata',
												'parameters'	=> Array(
																	'multiple'		=> FALSE,
																	'allow_empty'	=> FALSE,
																	'options'		=> Array(
																						'metadata'	=> 'Metadata Text Field',
																						'attribute'	=> 'Asset Attribute',
																					   ),
																   ),
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
												'is_contextable'	=> TRUE,
											   );

		$this->vars['quantity_message_checkout'] = Array(
													'added'		=> '0.3',
													'type'		=> 'text',
													'default'	=> 'There is an insufficient amount of items in stock to proceed with your order. Your order has not been processed. The items with insufficient quantities are listed below:',
													'is_contextable'	=> TRUE,
												   );

		$this->vars['country_question'] = Array(
											'added'		=> '0.3',
											'type'		=> 'assetid',
											'default'	=> '',
											'is_admin'	=> 'TRUE',
										  );

		$this->vars['accept_empty_country_question'] = Array(
																'added'			=> '0.4',
																'type'			=> 'boolean',
																'default'		=> FALSE,
																'is_admin'		=> TRUE,
																'description' 	=> 'This attribute indicates whether delivery prices per weight metric are applied even without a country question specified',
																'parameters'	=> Array(
																							'allow_empty'	=> FALSE,
																						),
															);

		$this->vars['taxable_countries'] = Array(
												'added'		=> '0.3',
												'type'		=> 'serialise',
												'default'	=> Array(),
												'is_admin'	=> 'TRUE',
											   );

		$this->vars['item_weight_selector'] = Array(
												'added'			=> '0.3',
												'type'			=> 'selection',
												'default'		=> 'metadata',
												'parameters'	=> Array(
																	'multiple'		=> FALSE,
																	'allow_empty'	=> FALSE,
																	'options'		=> Array(
																						'metadata'	=> 'Metadata Text Field',
																						'attribute'	=> 'Asset Attribute',
																					   ),
																   ),
											);

		$this->vars['item_weight_metadata_source'] = Array(
														'added'		=> '0.3',
														'type'		=> 'assetid',
														'default'	=> '',
														'is_admin'	=> TRUE,
													 );

		$this->vars['item_weight_attribute_source'] = Array(
														'added'		=> '0.3',
														'type'		=> 'text',
														'default'	=> '',
														'is_admin'	=> TRUE,
													  );

		$this->vars['delivery_weight_fee_mapping'] = Array(
														'added'		=> '0.3',
														'type'		=> 'serialise',
														'default'	=> Array(),
														'is_admin'	=> FALSE,
													 );

		$this->vars['default_delivery_weight_fee'] = Array(
														'added'		=> '0.3',
														'type'		=> 'float',
														'default'	=> 0.0,
														'is_admin'	=> TRUE,
													 );

		$this->vars['default_max_product_quantity'] = Array(
														'added'			=> '0.4',
														'type'			=> 'int',
														'default'		=> '',
														'is_admin'		=> TRUE,
														'parameters'	=> Array(
																					'allow_negative'	=> FALSE,
																					'allow_empty'		=> TRUE,
																					'range_lower'		=> 1,
																				),
													 );

		$this->vars['international_delivery_fee_exempt_countries'] = Array(
																		'added'		=> '0.3',
																		'type'		=> 'serialise',
																		'default'	=> Array(),
																		'is_admin'	=> 'TRUE',
																	 );

		$this->vars['international_delivery_fee'] = Array(
														'added'		=> '0.3',
														'type'		=> 'float',
														'default'	=> 0.0,
														'is_admin'	=> TRUE,
													);

		$this->vars['conditional_delivery_fees'] = Array(
																'added'		=> '0.3',
																'type'		=> 'serialise',
																'default'	=> Array(),
																'is_admin'	=> TRUE,
															);

		$this->vars['customise_receipt'] = Array(
											'added'		=> '0.3',
											'type'		=> 'boolean',
											'default'	=> FALSE,
											'is_admin'	=> TRUE,
										   );

		$this->vars['currency_symbol'] = Array(
											'added'		=> '0.3',
											'type'		=> 'text',
											'default'	=> '$',
											'is_admin'	=> TRUE,
										 );

		$this->vars['float_precision'] = Array(
											'added'		=> '0.3',
											'type'		=> 'int',
											'default'	=> '2',
											'is_admin'	=> TRUE,
										 );
										 
		$this->vars['max_delivery_fee'] = Array(
											'added'		=> '0.4',
											'type'		=> 'float',
											'default'	=> 0.0,
											'is_admin'	=> TRUE,
										 );

		$this->vars['refund_info_text'] = Array(
                                            'added'			=> '0.4',
                                            'type'			=> 'text',
                                            'default'		=> '',
                                            'description'	=> 'Text to display if some products in cart are non-refundable',
                                            );
		$this->vars['actions'] = Array(
										'added'		=> '0.4',
										'type'		=> 'serialise',
										'default'	=> Array(),
										'description' => 'Actions that needs to be executed at the end of checkout',
									);
									
		$this->vars['apply_tax_on_delivery'] = Array(
												'added'		=> '0.4',
												'type'		=> 'boolean',
												'default'	=> FALSE,
												'parameters'	=> Array(
																	'allow_empty'	=> FALSE,
																   ),
												'description' => 'Whether calculate tax over delivery cost',
										   );
									

		$this->vars['delivery_includes_tax'] = Array(
												'added'		=> '0.4',
												'type'		=> 'boolean',
												'default'	=> FALSE,
												'parameters'	=> Array(
																	'allow_empty'	=> FALSE,
																   ),
												'description' => 'Whether delivery cost includes tax',
										   );
										   
		$this->vars['promotional_code_button_text'] = Array(
															'added'			=> '0.4',
															'type'			=> 'text',
															'default'		=> 'Apply Promotion',
															'description'	=> 'Apply Promition Button Text',
															'is_contextable'	=> TRUE,
														);
		
		$this->vars['promotional_code_field_size'] = Array(
															'added'			=> '0.4',
															'type'			=> 'int',
															'default'		=> '20',
															'description'	=> 'Promition text input field size',
															'is_contextable'	=> FALSE,
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
		// ensure that the correct order submission folders are created underneath all Ecommerce_Checkout assets (introduced in 0.4)
		// also move all existing orders underneath the Completed Orders folder

		if (version_compare($current_version, '0.3', '<=')) {
			pre_echo('STARTING ECOMMERCE CHECKOUT UPGRADE - VERSION 0.4');

			// check each Ecommerce_Checkout asset
			$sql = 'SELECT assetid
					FROM sq_ast
					WHERE type_code = '.MatrixDAL::quote('ecommerce_checkout');
			try {
				$checkout_assetids = MatrixDAL::executeSqlAssoc($sql);
			} catch (Exception $e) {
				throw new Exception('Unable to locate Ecommerce_Checkout assets due to database error: '.$e->getMessage());
			}

			$sub_assets = Array(
							'pending_orders'	=> 'folder',
							'orders_to_review'	=> 'folder',
						  );

			foreach ($checkout_assetids as $row => $info) {
				$checkout = $GLOBALS['SQ_SYSTEM']->am->getAsset($info['assetid']);

				// create 2 folders for each asset
				foreach ($sub_assets as $link_value => $type) {
					$asset = new $type();
					$copy_link = Array(
									'asset'			=> $checkout,
									'value'			=> $link_value,
									'link_type'		=> SQ_LINK_TYPE_2,
									'is_dependant'	=> 1,
									'is_exclusive'	=> 1,
								 );

					$asset->setAttrValue('name', ucwords(str_replace('_',' ', $link_value)));
					if (!$asset->create($copy_link)) return FALSE;
				}

				// update the existing Orders folder
				$order_folder_link = $GLOBALS['SQ_SYSTEM']->am->getLink($checkout->id, SQ_LINK_TYPE_2, 'folder', TRUE, 'orders');
				if (!$GLOBALS['SQ_SYSTEM']->am->updateLink($order_folder_link['linkid'], SQ_LINK_TYPE_2, 'completed_orders')) return FALSE;
				// change the name
				$order_folder = $GLOBALS['SQ_SYSTEM']->am->getAsset($order_folder_link['minorid']);
				$order_folder->setAttrValue('name', 'Completed Orders');
				$order_folder->saveAttributes();
			}

			pre_echo('COMPLETED ECOMMERCE CHECKOUT UPGRADE - VERSION 0.4');

		}

		// first do things as normal
		$res = parent::_upgrade($current_version);

		return $res;

	}//end _upgrade()


}//end class

?>
