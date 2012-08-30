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
* $Id: design_area_ecommerce_cart_management.inc,v 1.6 2012/08/30 00:58:33 ewang Exp $
*
*/


require_once SQ_INCLUDE_PATH.'/asset_management.inc';

/**
* Design_Area_Ecommerce_Cart_Management
*
* @see Asset_Management
*
* @author  Blair Robertson <blair@squiz.net>
* @version $Revision: 1.6 $
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
