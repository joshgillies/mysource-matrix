<?php 
/**
* +--------------------------------------------------------------------+
* | Squiz.net Open Source Licence                                      |
* +--------------------------------------------------------------------+
* | Copyright (c), 2003 Squiz Pty Ltd (ABN 77 084 670 600).            |
* +--------------------------------------------------------------------+
* | This source file may be used subject to, and only in accordance    |
* | with, the Squiz Open Source Licence Agreement found at             |
* | http://www.squiz.net/licence.                                      |
* | Make sure you have read and accept the terms of that licence,      |
* | including its limitations of liability and disclaimers, before     |
* | using this software in any way. Your use of this software is       |
* | deemed to constitute agreement to be bound by that licence. If you |
* | modify, adapt or enhance this software, you agree to assign your   |
* | intellectual property rights in the modification, adaptation and   |
* | enhancement to Squiz Pty Ltd for use and distribution under that   |
* | licence.                                                           |
* +--------------------------------------------------------------------+
*
* $Id: product.inc,v 1.3 2005/01/26 09:49:21 brobertson Exp $
*
*/


require_once SQ_CORE_PACKAGE_PATH.'/page/page.inc';
require_once SQ_FUDGE_PATH.'/general/text.inc';

/**
* Product
* 
* product class description here
*
* @author  David Heppell <dheppell@squiz.net>
* @version $Revision: 1.3 $
* @package MySource_Matrix_Packages
* @subpackage ecommerce
*/
 
class Product extends Page
{
	/**
	* Constructor
	*
	* @param int	$assetid	the asset id to be loaded
	*
	* @return void
	* @access public
	*/
	function Product($assetid=0)
	{
		$this->_ser_attrs = true;
		$this->Page($assetid);
		
	}//end Product()
		

	/**
	* Load the vars for this asset/ asset type into the vars array
	*
	* If the asset has no asset ID, this function just loads the default values for the asset type.
	*
	* @access private
	* @return void
	*/
	function _loadVars()
	{
		// include price class so unserialize() doesn't barf
		require_once SQ_PACKAGES_PATH.'/ecommerce/price/product_price.inc';
		parent::_loadVars();

	}// end _loadVars()


	/**
	* Returns name of the asset
	*
	* @param boolean	$short_name	whether or not we are after the short name or the full name
	*
	* @see Asset::_getName()
	* @return string
	* @access private
	*/
	function _getName($short_name=false)
	{
		return $this->attr('name');

	}// end _getName()
	

	/**
	* Queue the setting of the value of an attribute
	*
	* @param string		$name	the name of the attribute
	* @param string		$value	the new value of the attribute
	*
	* @return boolean
	* @access public
	* @see saveAttributes()
	* @see Asset::setAttrValue()
	*/
	function setAttrValue($name, $value)
	{
		switch($name) {
			case 'price' :
				trigger_error('The price cannot be set directly. It must be set using the price_obj variable', E_USER_WARNING);
				return false;
				break;
			case 'price_obj' :
				if (!parent::setAttrValue($name, $value)) return false;
				$price_obj = $this->attr('price_obj');
				if (!parent::setAttrValue('price', $price_obj->value)) return false;
				return true;
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
		//if we just submitted this form
		if(!is_null($qty = array_get_index($_POST, 'quantity', null))) {
			
			if ($qty > 0) {
				$this->processSelection();				
			}
		
			//if form submitted but invalid quantity
			else if($qty < 1 || !is_int($qty)) {
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
			echo replace_keywords($this->attr('description'), $this->getAssetKeywords());
		?>
		</form>
		</div>
		<?php
		
	}//end printBody()
	
	/**
	* Handles finding which cart to add the order to
	*
	* @return boolean	indicating success
	* @access public
	*/
	function processSelection() 
	{

		$cart_assetid = $this->getActiveCartAssetid();

		if ($cart_assetid === false) {
			trigger_error('There was no active cart found', E_USER_WARNING);
			return false;
		}

		if (!is_numeric($qty = $_POST['quantity'])) {
			trigger_error('Invalid quantity entered', E_USER_WARNING);
			return false;
		}

		//get the cart asset
		$cart =& $GLOBALS['SQ_SYSTEM']->am->getAsset($cart_assetid);
					
		//update item and quantity to cart
		if (isset($_SESSION['sq_cart_contents'][$this->id])) {
			$cart->update($this->id, (int)$qty);
		} else {
			$cart->add($this->id, $this->attr('price'), (int)$qty);
		}
		return true;
		
	}//end processSelection()


	/**
	* Get the assetid of the active cart, depending on the current lineage
	*
	* @return mixed(string|boolean)	the assetid of the active cart or false
	*								if nothing is found
	* @access public
	*/
	function getActiveCartAssetid()
	{
		$parents = array_keys($GLOBALS['SQ_SYSTEM']->am->getParents($this->id, 'ecommerce_cart', false));

		switch (count($parents)) {
			case 0:
				return false;
			break;

			case 1:
				return reset($parents);
			break;

			default:
				// get current lineage and reverse it so the closest assets are first
				$lineage = array_reverse($GLOBALS['SQ_SYSTEM']->am->getLineageFromURL());

				// return the first cart we find, ie. the one closest to us
				foreach($lineage as $lineage_item) {
					if (false !== ($index = array_search($lineage_item['assetid'], $parents))) {
						return $parents[$index];
					}
				}

				// can't find any of the carts!
				return false;

			break;
		}

	}//end getActiveCartAssetid()
	
	
	/**
	*Returns a list of keyword/replacements
	*
	* @return array $keywords
	* @access public
	*/
	function getAssetKeywords() 
	{
		$old_keywords = parent::getAssetKeywords();
		
		$keywords = array(
						  'description'			=> array('value' =>$this->attr('description')),
						  'short_description'	=> array('value' =>$this->attr('short_description')),
						  'details'				=> array('value' =>$this->attr('details')),
						  'name'				=> array('value' =>$this->attr('name')),
						  'price'				=> array('value' =>$this->attr('price')),
						  'url'					=> array('value' =>$this->getURL()),
						  );
		
		// only expose the image stuff if we are a real asset (and not a dunmmy
		// one, as used in eg. the asset listing)
		if ($this->id) {

			// Adjust the quantity/commit keywords depending in whether there are
			// items in the cart already
			$qty = array_get_index($_SESSION['sq_cart_contents'], $this->id, Array('quantity' => 0));
	
			if ($qty['quantity'] != 0) {
				$keywords['quantity_field']['value'] = '<input type="text" name="update_quantity['.$this->id.']" size="10" value="'.$qty['quantity'].'" />';
				$keywords['commit_button']['value'] = '<input type="submit" value="Update Quantity" />';
			} else {
				$keywords['quantity_field']['value'] = '<input type="text" name="quantity" size="10" />';
				$keywords['commit_button']['value'] = '<input type="submit" value="Add to Cart" />';
			}
			
			$images = array_keys($GLOBALS['SQ_SYSTEM']->am->getChildren($this->id, 'image', false));
			
			foreach($images as $image_assetid) {
				$keywords['image_'.$image_assetid]['value'] = '<img src="'.$GLOBALS['SQ_SYSTEM']->am->getAssetURL($image_assetid).'">';
			}
		}
		
		return array_merge($old_keywords, $keywords);

	}//end getAssetKeywords()



}//end class
?>
