<?php
/**
* +--------------------------------------------------------------------+
* | This MySource Matrix CMS file is Copyright (c) Squiz Pty Ltd	   |
* | ABN 77 084 670 600												   |
* +--------------------------------------------------------------------+
* | IMPORTANT: Your use of this Software is subject to the terms of    |
* | the Licence provided in the file licence.txt. If you cannot find   |
* | this file please contact Squiz (www.squiz.com.au) so we may provide|
* | you a copy.														   |
* +--------------------------------------------------------------------+
*
* $Id: ecommerce_common.inc,v 1.10 2013/09/09 07:51:56 cupreti Exp $
*
*/

/*
* Ecommerce Package Library Functions
*
* This file contains global functions that are applicable to both Ecom Form and Ecom Cart/Checkout
*
* @author  Chiranjivi Upreti <cupreti@squiz.com.au>
* @version $Revision: 1.10 $
* @package MySource_Matrix_Packages
* @subpackage ecommerce
*/
class Ecommerce_Common
{
	
	/**
	* Return the promotional code assetid(s) if it is linked to the passed asset 
	* and has the promotional code matching the passed promotional code string
	*
	* @param object	$assetid	The asset to get promotional code asset from
	* @param string	$promo_code	The promotional code string to search
	*
	* @return mixed
	* @access public
	*/
	public static function getPromotionalCodeAssetIds($assetid, $promo_code)
	{	
		$promo_code_assetids = Array();	
		
		$asset = $GLOBALS['SQ_SYSTEM']->am->getAsset($assetid);
		if (is_null($asset)) {
			return $promo_code_assetids;
		}
		
		$links = $GLOBALS['SQ_SYSTEM']->am->getLinks($asset->id, SQ_LINK_NOTICE, 'promotional_code', TRUE, 'minor', $promo_code);		
		foreach ($links as $link) {
			$promo_code_asset = $GLOBALS['SQ_SYSTEM']->am->getAsset($link['majorid']);
			if (!is_null($promo_code_asset) && ($promo_code_asset->status == SQ_STATUS_LIVE)) {
				$promo_code_assetids[$link['majorid']] = 1;
			}
			$GLOBALS['SQ_SYSTEM']->am->forgetAsset($promo_code_asset);
		}
		
		$GLOBALS['SQ_SYSTEM']->am->forgetAsset($asset);
		
		return array_keys($promo_code_assetids);
		
	}//end getPromotionalCodeAssetids()
	
	
	/**
	* Get the discount of promotional code on an asset with price and quantity
	*
	* @param string $promo_code_assetids    Asset IDs of the promotional code
	* @param float  $price                  The price of the asset to apply the promotional code
	* @param int    $quantity               The quantity of the asset to apply the promtional code
	*
	* @return float
	* @access public
	*/
	public static function getPromotionalDiscount($promo_code_assetids, $price, $quantity)
	{
		$promotional_info = self::getApplicablePromotionalInfo($promo_code_assetids, $price, $quantity);		

		return $promotional_info['discount'];

	}//end getPromotionalCodeDiscount()
	
	
	/**
	* Get the "free shipping" status of promotional code on an asset with price and quantity
	*
	* @param string $promo_code_assetids    Asset IDs of the promotional code
	* @param float  $price                  The price of the asset to apply the promotional code
	* @param int    $quantity               The quantity of the asset to apply the promtional code
	*
	* @return boolean
	* @access public
	*/
	public static function getPromotionalFreeShipping($promo_code_assetids, $price, $quantity)
	{
		$promotional_info = self::getApplicablePromotionalInfo($promo_code_assetids, $price, $quantity);		

		return $promotional_info['free_shipping'];

	}//end getPromotionalFreeShipping()	

	
	/**
	* Get the promotional offers info after applying the promotional code on an asset with price and quantity
	*
	* @param string $promo_code_assetids    Asset IDs of the promotional code
	* @param float  $price                  The price of the asset to apply the promotional code
	* @param int    $quantity               The quantity of the asset to apply the promtional code
	*
	* @return float
	* @access public
	*/
	public static function getApplicablePromotionalInfo($promo_code_assetids, $price, $quantity)
	{		
		// Based on currently available promotional rules we can have two types of promotions
		// 1. Price discount (fixed price, percentage) and 2. Free shipping		
		
		// If different types of rules are added later, this needs to be updated accourdingly
		$asset_data = Array(
						'price' => $price, 
						'quantity' => $quantity,
						'discount' => 0,
						'free_shipping' => FALSE,
					);
	
		$discount = 0;
		$free_shipping = FALSE;		
		foreach($promo_code_assetids as $promo_code_assetid) {
			$promo_code_asset = $GLOBALS['SQ_SYSTEM']->am->getAsset($promo_code_assetid);			
			
			// "$asset_data" is passed by reference
			// The rule->process() adds the promotional offer data to this variable
			$promo_code_asset->process($asset_data);			
			$GLOBALS['SQ_SYSTEM']->am->forgetAsset($promo_code_asset);
		}
		
		$offers = Array(
					'discount' => $asset_data['discount'],
					'free_shipping' => $asset_data['free_shipping'],
				);

		return $offers;

	}//end getApplicablePromotionalInfo()	


}//end class