<?php
/**
* +--------------------------------------------------------------------+
* | Squiz.net Commercial Module Licence                                |
* +--------------------------------------------------------------------+
* | Copyright (c) Squiz Pty Ltd (ACN 084 670 600).                     |
* +--------------------------------------------------------------------+
* | This source file is not open source or freely usable and may be    |
* | used subject to, and only in accordance with, the Squiz Commercial |
* | Module Licence.                                                    |
* | Please refer to http://www.squiz.net/licence for more information. |
* +--------------------------------------------------------------------+
*
* $Id: design_area_ecommerce_cart_management.inc,v 1.3 2005/08/04 01:33:47 lwright Exp $
*
*/


require_once SQ_INCLUDE_PATH.'/asset_management.inc';

/**
* Design_Area_Ecommerce_Cart_Management
*
* @see Asset_Management
*
* @author  Blair Robertson <blair@squiz.net>
* @version $Revision: 1.3 $
* @package MySource_Matrix_Packages
* @subpackage __core__
*/
class Design_Area_Ecommerce_Cart_Management extends Asset_Management
{


	/**
	* Constructor
	*
	*/
	function Design_Area_Ecommerce_Cart_Management(&$pm)
	{
		$this->Asset_Management($pm);

		$this->vars = Array(
							'cart'		=> Array(
											'added'			=> '0.1',
											'type'			=> 'assetid',
											'default'		=> 0,
											'description'	=> 'The assetid of the cart that we are connected with',
										   ),
							'ecom_form' => Array(
											'added'			=> '0.1',
											'type'			=> 'assetid',
											'default'		=> 0,
											'description'	=> 'The assetid of the ecommerce checkout page that we are connected with',
										   ),
						   );

	}//end constructor


}//end class
?>
