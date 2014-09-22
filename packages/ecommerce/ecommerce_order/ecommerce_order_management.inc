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
* $Id: ecommerce_order_management.inc,v 1.17 2012/08/30 00:58:33 ewang Exp $
*
*/


require_once SQ_INCLUDE_PATH.'/asset_management.inc';

/**
* Ecommerce_Order_Management
*
* @see Asset_Management
*
* @author  Scott Kim <skim@squiz.net>
* @version $Revision: 1.17 $
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
												'is_admin'	=> FALSE,
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
													'is_admin'	=> FALSE,
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
						'order_grand_total'			=> Array(
													'added'			=> '0.3',
													'type'			=> 'float',
													'default'		=> 0.0,
													'description'	=> 'The grand total price of this Order asset',
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
		if (version_compare($current_version, '0.2', '<=')) {
			pre_echo('STARTING ECOMMERCE CHECKOUT UPGRADE - VERSION 0.3');

				$root_folder = $GLOBALS['SQ_SYSTEM']->am->getSystemAsset('root_folder');
				$children = $GLOBALS['SQ_SYSTEM']->am->getChildren($root_folder->id, $this->getAssetType(), FALSE);

				pre_echo('Upgrading '.count($children).' assets...');
				foreach ($children as $assetid => $type) {
					$asset = $GLOBALS['SQ_SYSTEM']->am->getAsset($assetid, $this->getAssetType());
					if (is_null($asset)) continue;
					$products_info = $asset->attr('products');

					$ecomm_checkout_id = array_keys($GLOBALS['SQ_SYSTEM']->am->getParents($assetid, 'ecommerce_checkout'));
					if (empty($ecomm_checkout_id)) continue;
					$ecomm_checkout = $GLOBALS['SQ_SYSTEM']->am->getAsset($ecomm_checkout_id[0]);
					if (is_null($ecomm_checkout)) continue;

					if (!$GLOBALS['SQ_SYSTEM']->am->acquireLock($asset->id, 'attributes')) {
						trigger_localised_error('CMS0025', sprintf(translate('Asset Builder Upgrade [from %1$s] FAILED - Unable to Acquire Lock on "%2$s" (#%3$s)', '0.2'), $asset->name, $asset->id), E_USER_WARNING)
						return FALSE;
					}

					$total_cost = 0;
					$flat_charges_total = 0;

					$cart = new Ecommerce_Cart_Processor();
					$flat_charges_total = $ecomm_checkout->getFlatCharges();

					$cart->setCartContainer($products_info);
					$total_cost = $cart->getGrandTotalExDelivery($ecomm_checkout->attr('taxation_rate')/100, FALSE, $flat_charges_total);

					unset($cart);

					$asset->setAttrValue('order_grand_total', $total_cost);
					if (!$asset->saveAttributes()) {
						trigger_localised_error('CMS0026', sprintf(translate('Asset Builder Upgrade [from %s] FAILED'), '0.2'), E_USER_WARNING)
						return FALSE;
					}

					$GLOBALS['SQ_SYSTEM']->am->releaseLock($asset->id, 'attributes');
				}

			pre_echo('UPGRADE COMPLETE - FROM VERSION '.$current_version.' TO 0.3');
		}
		// first do things as normal
		$res = parent::_upgrade($current_version);

		return $res;

	}//end _upgrade()


}//end class

?>
