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
* $Id: design_area_ecommerce_cart.inc,v 1.15 2012/08/30 00:58:33 ewang Exp $
*
*/

require_once SQ_CORE_PACKAGE_PATH.'/designs/design_area/design_area.inc';

/**
* Design_Area_Ecommerce_Cart
*
* Purpose
*
*
* @author  Blair Robertson <blair@squiz.net>
* @version $Version$ - 1.0
* @package MySource_Matrix_Packages
* @subpackage __core__
*/
class Design_Area_Ecommerce_Cart extends Design_Area
{


	/*
	* Outputs the var either as straight HTML or PHP code, depending on the type
	*
	* @param string	$var			the name of the var to print out
	* @param Array	$args			array of any extra arguments that existed in the print command
	* @param string	$value_prefix	PHP code to wrap put before the value before outputting it
	* @param string	$value_suffix	PHP code to wrap put after  the value before outputting it
	*
	* @return void
	* @access public
	*/
	function printVar($var, $args, $value_prefix='', $value_suffix='')
	{
		switch ($var) {
			case 'cart_href' :
				echo '<','?php';
				?>
					$url = $GLOBALS['SQ_SYSTEM']->am->getAssetURL(<?php echo $this->attr('cart'); ?>);
					echo $url;
				<?php
				echo '?','>';
				break;

			case 'checkout_href' :
				echo '<','?php';
				?>
					$url = $GLOBALS['SQ_SYSTEM']->am->getAssetURL(<?php echo $this->attr('ecom_form'); ?>);
					echo $url;
				<?php
				echo '?','>';
				break;

			case 'items' :
				echo '<','?php';
				?>
					require_once SQ_PACKAGES_PATH.'/ecommerce/lib/ecommerce_cart_processor.inc';
					$cart = new Ecommerce_Cart_Processor(<?php echo $this->attr('ecom_form'); ?>);
					echo (($cart->getCount() == 1) ? translate('item') : translate('items'));


				<?php
				echo '?','>';
				break;

			case 'count' :
				echo '<','?php';
				?>
					require_once SQ_PACKAGES_PATH.'/ecommerce/lib/ecommerce_cart_processor.inc';
					$cart = new Ecommerce_Cart_Processor(<?php echo $this->attr('ecom_form'); ?>);
					echo $cart->getCount();
				<?php
				echo '?','>';
				break;

			case 'total' :
				echo '<','?php';
				?>
					require_once SQ_PACKAGES_PATH.'/ecommerce/lib/ecommerce_cart_processor.inc';
					$cart = new Ecommerce_Cart_Processor(<?php echo $this->attr('ecom_form'); ?>);
					echo number_format($cart->getTotal(), 2);
				<?php
				echo '?','>';
				break;


			default:
				parent::printVar($var, $args);

		}

	}//end printVar()


}//end class

?>
