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
* $Id: design_area_ecommerce_cart.inc,v 1.4 2005/06/20 05:33:48 lwright Exp $
* $Name: not supported by cvs2svn $
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
				$cart_assetid = (int) $this->attr('cart');
				$url = $GLOBALS['SQ_SYSTEM']->am->getAssetURL($cart_assetid);
				echo $url;
				break;

			case 'form_href' :
				$cart_assetid = (int) $this->attr('cart');
				$url = $GLOBALS['SQ_SYSTEM']->am->getAssetURL($cart_assetid);
				echo $url;
				break;

			case 'items' :
				$cart_assetid = $this->attr('cart');
				echo '<','?php';
				?>
				$cart =& $GLOBALS['SQ_SYSTEM']->am->getAsset(<?php echo $cart_assetid ?>);
				echo ' '.($cart->count() == 1) ? translate('item') : translate('items');
				<?php
				echo '?','>';
				break;

			case 'count' :
				$cart_assetid = $this->attr('cart');
				echo '<','?php';
				?>
				$cart =& $GLOBALS['SQ_SYSTEM']->am->getAsset(<?php echo $cart_assetid ?>);
				echo $cart->count();
				<?php
				echo '?','>';
				break;

			case 'total' :
				$cart_assetid = $this->attr('cart');
				echo '<','?php';
				?>
				$cart =& $GLOBALS['SQ_SYSTEM']->am->getAsset(<?php echo $cart_assetid ?>);
				echo number_format($cart->total(),2);
				<?php
				echo '?','>';
				break;


			default:
				parent::printVar($var, $args);

		}

	}//end printVar()


}//end class

?>
