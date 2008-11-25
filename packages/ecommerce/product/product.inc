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
* $Id: product.inc,v 1.16 2008/11/25 00:48:06 mbrydon Exp $
*
*/

require_once SQ_CORE_PACKAGE_PATH.'/page/page.inc';
require_once SQ_FUDGE_PATH.'/general/text.inc';
require_once dirname(__FILE__).'/../interfaces/buyable/buyable.inc';

/**
* Product
*
* product class description here
*
* @author  David Heppell <dheppell@squiz.net>
* @version $Revision: 1.16 $
* @package MySource_Matrix_Packages
* @subpackage ecommerce
*/
//register_implementation('product', 'buyable');
class Product extends Page implements Buyable
{


	/**
	* Constructor
	*
	* @param int	$assetid	the asset id to be loaded
	*
	* @return void
	* @access public
	*/
	function __construct($assetid=0)
	{
		$this->_ser_attrs = TRUE;
		parent::__construct($assetid);

	}//end constructor



	/**
	* Load the vars for this asset/ asset type into the vars array
	*
	* If the asset has no asset ID, this function just loads the default values for the asset type.
	*
	* @return void
	* @access private
	*/
	function _loadVars()
	{
		// include price class so unserialize() doesn't barf
		require_once SQ_PACKAGES_PATH.'/ecommerce/price/product_price.inc';
		parent::_loadVars();

	}//end _loadVars()


	/**
	* Returns name of the asset
	*
	* @param boolean	$short_name	whether or not we are after the short name or the full name
	*
	* @see Asset::_getName()
	*
	* @return string
	* @access private
	*/
	function _getName($short_name=FALSE)
	{
		return $this->attr('name');

	}//end _getName()


	/**
	* Queue the setting of the value of an attribute
	*
	* @param string	$name	the name of the attribute
	* @param string	$value	the new value of the attribute
	*
	* @return boolean
	* @access public
	* @see saveAttributes()
	* @see Asset::setAttrValue()
	*/
	function setAttrValue($name, $value)
	{
		switch ($name) {
			case 'price' :
				$price_obj = $this->attr('price_obj');
				$price_obj->value = $value;
				return $this->setAttrValue('price_obj', $price_obj);
			break;
			case 'price_obj' :
				if (!parent::setAttrValue($name, $value)) return FALSE;
				$price_obj = $this->attr('price_obj');
				if (!parent::setAttrValue('price', $price_obj->value)) {
					return FALSE;
				}
				return TRUE;
			break;
			default :
				return parent::setAttrValue($name, $value);
		}

	}//end setAttrValue()


	/**
	* Prints out the Frontend for this asset
	* it's up to the kids to override
	*
	* @return void
	* @access public
	*/
	function printFrontend()
	{
		// if we just submitted this form
		if (!is_null($qty = array_get_index($_POST, 'quantity', NULL))) {

			if ($qty > 0) {
				$this->processSelection();
			// if form submitted but invalid quantity
			} else if ($qty < 1 || !is_int($qty)) {
				// invalid selection - set flag here;
			}
		}

		parent::printFrontend();

	}//end printFrontend()


	/**
	* Prints out the body of the asset
	*
	* @return void
	* @access public
	*/
	function printBody()
	{
		?>
		<form action="<?php echo $this->getURL(); ?>" method="post">
		<?php

			$replacement_array = Array();
			$keywords = $this->getAssetKeywords();
			// getAssetKeyword returns a array with the structure
			// $keywords = array( key => array( description => $decription,
			//                                  value => $value,
			//                                )
			//                  )
			// but..
			// replace_keywords expect array ( key => value)
			// so we rebuild this structure
			foreach ($keywords as $keyword => $info) {
				$replacement_array[$keyword] = $info['value'];
			}
			$product_description = $this->attr('description');
			echo replace_keywords($product_description, $replacement_array);

		?>
		</form>
		</div>
		<?php

	}//end printBody()


	/**
	* Handles finding which cart to add the order to and return a boolean indicating success
	*
	* @return boolean
	* @access public
	*/
	function processSelection()
	{

		$cart_assetid = $this->getActiveCartAssetid();

		if ($cart_assetid === FALSE) {
			trigger_localised_error('ECOM0011', E_USER_WARNING);
			return FALSE;
		}

		if (!is_numeric($qty = $_POST['quantity'])) {
			trigger_localised_error('ECOM0012', E_USER_WARNING);
			return FALSE;
		}

		// get the cart asset
		$cart =& $GLOBALS['SQ_SYSTEM']->am->getAsset($cart_assetid);

		// update item and quantity to cart
		if (isset($_SESSION['sq_cart_contents'][$this->id])) {
			$cart->update($this->id, (int)$qty);
		} else {
			$cart->add($this->id, $this->attr('price'), (int)$qty);
		}
		return TRUE;

	}//end processSelection()


	/**
	* Get the assetid of the active cart, depending on the current lineage
	*
	* @return mixed {string|boolean}	the assetid of the active cart or false if nothing is found
	* @access public
	*/
	function getActiveCartAssetid()
	{
		$parents = array_keys($GLOBALS['SQ_SYSTEM']->am->getParents($this->id, 'ecommerce_cart', FALSE));

		switch (count($parents)) {
			case 0:
				return FALSE;
			break;

			case 1:
				return reset($parents);
			break;

			default:
				// get current lineage and reverse it so the closest assets are first
				$lineage = array_reverse($GLOBALS['SQ_SYSTEM']->am->getLineageFromURL());

				// return the first cart we find, ie. the one closest to us
				foreach ($lineage as $lineage_item) {
					if (FALSE !== ($index = array_search($lineage_item['assetid'], $parents))) {
						return $parents[$index];
					}
				}

				// can't find any of the carts!
				return FALSE;

			break;
		}

	}//end getActiveCartAssetid()


	/**
	* Return the keywords with their replacements
	*
	* The return value is in the form:
	* <pre>
	* Array(
	*     'name' => Array (
	*          'value' => 'page',
	*          'description' => 'the asset name',
	*     ),
	* )
	* </pre>
	*
	* @param boolean $descriptions if TRUE descriptions will be included in the array
	*
	* @return array
	* @access public
	*/
	function getAssetKeywords($descriptions=FALSE)
	{
		$keywords = parent::getAssetKeywords($descriptions);

		// only expose the image stuff if we are a real asset (and not a dunmmy
		// one, as used in the asset listing to put our keywords in a drop down)
		if ($this->id) {

			// Adjust the quantity/commit keywords depending in whether there are
			// items in the cart already
			if (isset($_SESSION['sq_cart_contents'])) {

				$qty = array_get_index($_SESSION['sq_cart_contents'], $this->id, Array('quantity' => 0));
				if ($qty['quantity'] != 0) {
					$keywords['quantity_field']['value'] = '<input type="text" name="update_quantity['.$this->id.']" size="10" value="'.$qty['quantity'].'" />';
					$keywords['commit_button']['value'] = '<input type="submit" value="Update Quantity" />';
				} else {
					$keywords['quantity_field']['value'] = '<input type="text" name="quantity" size="10" />';
					$keywords['commit_button']['value'] = '<input type="submit" value="Add to Cart" />';
				}

			}

			$images = array_keys($GLOBALS['SQ_SYSTEM']->am->getChildren($this->id, 'image', FALSE));
			foreach ($images as $image_assetid) {
				$keywords['image_'.$image_assetid]['value'] = '<img src="'.$GLOBALS['SQ_SYSTEM']->am->getAssetURL($image_assetid).'">';
			}

		}

		return $keywords;

	}//end getAssetKeywords()


	/**
	* Return the product's price
	*
	* @return array
	* @access public
	*/
	public function getPrice() {
		$price_obj = $this->attr('price_obj');
		if (is_null($price_obj->value)) return 0;
		return $price_obj->value;

	}//end getPrice()


}//end class
?>
