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
* $Id: ecommerce_cart.inc,v 1.8 2005/05/25 04:33:18 lwright Exp $
*
*/


require_once SQ_PACKAGES_PATH.'/cms/listing_engine/listing_engine.inc';

/**
* Ecommerce_Cart
*
* Purpose
*   A simple shopping cart used for ecommerce functions
*
*
* @author  Luke Wright <lwright@squiz.net>
* @version $Revision: 1.8 $
* @package MySource_Matrix_Packages
* @subpackage ecommerce
*/
class Ecommerce_Cart extends Listing_Engine
{


	/**
	* Constructor
	*
	* @param string	$assetid	the assetid of the asset to load into this object
	*
	* @return void
	* @access public
	*/
	function Ecommerce_Cart($assetid=0)
	{
		$this->Listing_Engine($assetid);

	}//end constructor


	/**
	* Perform any additional processing required during the creation of this asset
	*
	* Creates the page contents bodycopy
	*
	* @param array	&$link	information used to create the initial link
	*
	* @access private
	* @return boolean
	*/
	function _createAdditional(&$link)
	{
		if (!parent::_createAdditional($link)) return false;

		// add a bodycopy to this page when creating
		$GLOBALS['SQ_SYSTEM']->am->includeAsset('bodycopy');

		$asset = new Bodycopy();
		$copy_link = Array('asset' => &$this, 'value' => 'page_contents' ,'link_type' => SQ_LINK_TYPE_2, 'is_dependant' => 1, 'is_exclusive' => 1);
		$asset->setAttrValue('name', 'Page Contents');

		return $asset->create($copy_link);

	}//end _createAdditional()


	/**
	* Get the list of asset that should be printed
	*
	* This is defined as an abstract method of sorts; it's up to the kids to
	* override this
	* The return array is in the form Array(assetid => type_code) unless
	* we are grouping by letter, in which case the return array is
	* <pre>
	* Array(assetid => Array(
	*						'type_code' => type_code,
	*						'first_letter' => first_letter,
	*						);
	*		);
	* </pre>
	*
	* @return array
	* @access protected
	*/
	function getAssetList()
	{
		if (!isset($_SESSION['sq_cart_contents'])) {
			return Array();
		}

		$cart_contents = $_SESSION['sq_cart_contents'];
		$asset_list = '';

		$group_by = $this->attr('group_by');

		foreach($cart_contents as $product_assetid => $product_info) {
			$product_asset =& $GLOBALS['SQ_SYSTEM']->am->getAsset($product_assetid);
			if ($group_by == 'letter') {
				$asset_list[$product_assetid] = Array(
													'type_code'		=> $product_asset->type(),
													'first_letter'	=> substr($product_asset->attr('name'),0,1),
												);
			} else {
				$asset_list[$product_assetid] = $product_asset->type();
			}
		}

		return $asset_list;

	}//end getAssetList()

	/**
	* Returns an array of all the permitted link type, the type asset and
	* the cardinality
	*
	* @see Asset::_getAllowLinks()
	* @access private
	*/
	function _getAllowedLinks()
	{
		$page_links = parent::_getAllowedLinks();
		$page_links[SQ_LINK_TYPE_1]['folder'] = Array('card' => 'M');
		$page_links[SQ_LINK_TYPE_1]['product'] = Array('card' => 'M');
		$page_links[SQ_LINK_TYPE_2]['bodycopy'] = Array('card' => '1');
		return $page_links;

	}//end _getAllowedLinks()


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
		return (($short_name) ? $this->attr('short_name') : $this->attr('name'));

	}//end _getName()


	/**
	* Prepare cart for first use, if the relevant session vars aren't there
	*
	* @return int	1 if session var needed to be created (ie. cart
	*				needed to be init'ed), 0 if cart already existed
	*				(-1 is reserved for an error condition)
	* @access public
	*/
	function initialise()
	{
		$ret_val = 0;

		if (!isset($_SESSION['sq_cart_contents'])) {
			$_SESSION['sq_cart_contents'] = Array();
			$ret_val = 1;
		}

		return $ret_val;

	}//end initialise()


	/**
	* Unsets the contents of this particular cart
	*
	* If this is the last cart being stored in the session cookie, unset the
	* whole cart contents session variable as well
	*
	* @return boolean	always true (false is reserved for an error condition)
	* @access public
	*/
	function clear()
	{
		unset($_SESSION['sq_cart_contents']);
		return true;

	}//end clear()


	/**
	* Adds an item(s) to the cart
	*
	* @param string	$product_assetid	The assetid of the product to add
	* @param double	$price			The price to record against that product
	* @param int		$quantity		The quantity to add
	*
	* @return boolean	indicating success
	* @access public
	*/
	function add($product_assetid, $price, $quantity=1)
	{
		// quantity is ZERO, assume that this is valid and silently return
		// quantity is LESS THAN ZERO, throw an error
		if ($quantity == 0) {
			return true;
		} else if ($quantity < 0) {
			trigger_localised_error('ECOM0004', E_USER_WARNING);
			return false;
		}

		// check that the passed product assetid is indeed valid and
		// a valid product (or subclass of product)
		assert_valid_assetid($product_assetid);
		$asset_info = $GLOBALS['SQ_SYSTEM']->am->getAssetInfo(Array($product_assetid), 'product', false);

		if (empty($asset_info)) {
			trigger_localised_error('ECOM0005', E_USER_WARNING, $product_assetid);
			return false;
		}

		$cart =& $this->get();
		if (!isset($cart[$product_assetid])) {
			$cart[$product_assetid] = Array('price' => $price, 'quantity' => 0);
		}

		$cart[$product_assetid]['quantity'] += $quantity;

	}//end add()


	/**
	* Removes an item(s) from the cart
	*
	* Assumption: product lines were correctly made using the add() method
	* and therefore, only valid products are found in the cart (because of the
	* add() checking).
	*
	* @param string	$product_assetid	The assetid of the product to remove
	* @param int		$quantity		The quantity of this product to remove -
	*								leave blank to remove all of that product
	*
	* @return boolean	indicating success
	* @access public
	*/
	function remove($product_assetid, $quantity=null)
	{
		$cart =& $this->get();

		// product doesn't exist in cart - if the quantity is zero (or ALL)
		// then silently return, otherwise error
		if (!isset($cart[$product_assetid])) {
			if (is_null($quantity) || ($quantity == 0)) {
				return true;
			} else if ($quantity > 0) {
				trigger_localised_error('ECOM0006', E_USER_WARNING);
				return false;
			}
		}

		// after this point we can assume that product exists in the cart
		// Quantity is null, remove everything
		if (is_null($quantity)) {
			$quantity = $cart[$product_assetid]['quantity'];
		}

		// Qty == 0, silently return - Qty < 0, throw an error
		if ($quantity == 0) {
			return true;
		} else if ($quantity < 0) {
			trigger_localised_error('ECOM0007', E_USER_WARNING);
			return false;
		}

		if ($quantity > $cart[$product_assetid]['quantity']) {
			trigger_localised_error('ECOM0008', E_USER_WARNING);
			return false;
		} else if ($quantity == $cart[$product_assetid]['quantity']) {
			unset($cart[$product_assetid]);
		} else {
			$cart[$product_assetid]['quantity'] -= $quantity;
		}

	}//end remove()


	/**
	* Updtes the quantity of an item(s) in the cart
	*
	* Assumption: product lines were correctly made using the add() method
	* and therefore, only valid products are found in the cart (because of the
	* add() checking).
	*
	* @param string	$product_assetid	The assetid of the product to remove
	* @param int		$quantity		The quantity of this product to remove -
	*								leave blank to remove all of that product
	*
	* @return boolean	indicating success
	* @access public
	*/
	function update($product_assetid, $quantity)
	{
		$cart =& $this->get();

		// if we are not updating anything, silently return succes
		if (array_get_index($cart, $product_assetid, 0) == $quantity) return true;
		if (array_get_index($cart[$product_assetid], 'quantity', 0) == $quantity) return true;

		// product doesn't exist in cart, then silently return, otherwise error
		if (!isset($cart[$product_assetid])) {
			trigger_localised_error('ECOM0009', E_USER_WARNING);
			return false;
		}

		// Qty < 0, throw an error
		if ($quantity < 0) {
			trigger_localised_error('ECOM0010', E_USER_WARNING);
			return false;
		} else if ($quantity == 0) {
			unset($cart[$product_assetid]);
		} else {
			$cart[$product_assetid]['quantity'] = $quantity;
		}

		return true;

	}//end update()


	/**
	* Returns a reference to the session variable that holds this cart
	*
	* This function calls initialise() so the cart contents array will always
	* be there when this is called.
	*
	* @return Array
	* @access public
	*/
	function &get()
	{
		$this->initialise();
		return $_SESSION['sq_cart_contents'];

	}//end get()


	/**
	* Return the sum-total of all the items currently in this cart
	*
	* @return double
	* @access public
	*/
	function total()
	{
		$cart =& $this->get();
		$total = 0.0;
		foreach($cart as $product_assetid => $details) {
			if ($details['price'] != -1) {		// -1 = value not known
				$total += $details['quantity'] * $details['price'];
			}
		}

		return $total;

	}//end total()


	/**
	* Return the count of all the items currently in this cart
	*
	* @return double
	* @access public
	*/
	function count()
	{
		$cart =& $this->get();
		$count = 0;
		foreach($cart as $product_assetid => $details) {
			$count += $details['quantity'];
		}

		return $count;

	}//end count()


	/**
	 * Constructs the carts contents
	 *
	 * @return void
	 * @access public
	 */
	function printContents()
	{
		$bodycopy_link = $GLOBALS['SQ_SYSTEM']->am->getLink($this->id, SQ_LINK_TYPE_2, 'bodycopy', true, 'page_contents');
		if (empty($bodycopy_link)) return false;

		$format_bodycopy = &$GLOBALS['SQ_SYSTEM']->am->getAsset($bodycopy_link['minorid'], $bodycopy_link['minor_type_code']);
		if (is_null($format_bodycopy)) return false;

		require_once SQ_FUDGE_PATH.'/general/text.inc';
		$keywords = $format_bodycopy->getKeywords();
		$replacements = Array();


		if (!in_array('asset_listing', $keywords)) {
			// there is no asset listing keyword in the contents, so we dont
			// need to do all the extra processing for the listing
			$format_bodycopy->printBody();
			return;
		}

		//the children here will refer to the contents of this cart
		$children = $this->getAssetList();
		//we dont want to count position formats without a value
		//just increment the count if were adding it tot he cart
		$num_kids = count($children);
		$this->filterAssetList($children);

		//register a form to allow updating cart contents
		$this->registerFormField('cart_'.$this->id);

		foreach ($keywords as $word) {
			$replacements[$word] = $this->getKeywordReplacement($word);
		}

		//update the cart contents for any updated quantities
		if (!is_null(array_get_index($_REQUEST,'update_quantity',null))) {
			//see if we are updating an individual item or everything
			if(isset($_REQUEST['update_button'])) {
				reset($_REQUEST['update_button']);
				$update_productid = key($_REQUEST['update_button']);

				if ($update_productid == 'all') {
					// updating (potentially) all the items
					foreach($_REQUEST['update_quantity'] as $product_assetid => $new_quantity) {
						if (is_numeric($new_quantity)) {
							// it's quite possible now with position formats that
							// an asset in the cart might not have any quantity,
							// so we now check whether we have to update or add
							if (isset($_SESSION['sq_cart_contents'][$product_assetid])) {
								$this->update($product_assetid, (int)$new_quantity);
								if ($new_quantity == 0) {
									$num_kids--;
								}
							} else {
								$product_asset = $GLOBALS['SQ_SYSTEM']->am->getAsset($product_assetid);
								$this->add($product_assetid, $product_asset->attr('price'), (int)$new_quantity);
								if ($new_quantity > 0) {
									$num_kids++;
								}
							}
						}
					}

				} else {
					// updating only one of the items
					$new_quantity = array_get_index($_REQUEST['update_quantity'], $update_productid, null);
					if (is_numeric($new_quantity)) {
						// adding or updating?
						if (isset($_SESSION['sq_cart_contents'][$update_productid])) {
								$this->update($update_productid, (int)$new_quantity);
								if ($new_quantity == 0) {
									$num_kids--;
								}
							} else {
								$product_asset = $GLOBALS['SQ_SYSTEM']->am->getAsset($update_productid);
								$this->add($update_productid, $product_asset->attr('price'), (int)$new_quantity);
								if ($new_quantity > 0) {
									$num_kids++;
								}
							}
					}
				}
			}

			ob_start();
				?><strong><?php echo translate('ecom_cart_quantities_changed'); ?></strong><?php
				$replacements['cart_updated'] = ob_get_contents();
			ob_end_clean();
		} else {
			$replacements['cart_updated'] = '';
		}

		$num_per_page = $this->attr('num_per_page');
		$result_page = (isset($_REQUEST['result_page'])) ? (int)$_REQUEST['result_page'] : 1;
		if ($result_page <= 0) $result_page = 1;

		// get our page chunk here

		if (empty($children)) {
			$replacements['checkout_cart'] = '';
			$replacements['cart_updated'] = $this->attr('no_items_text');
			$replacements['next_page'] = '';
			$replacements['previous_page'] = '';
			$replacements['total_pages'] = 1;

			// page list is one page
			$replacements['page_list'] = '1';
			$global_contents = '';
			$num_assets_showing = 0;
			$num_kids = 0;
		} else {
			$todo =& $this->getChunk($children, $replacements, $result_page, $num_per_page);
			$num_assets_showing = count($todo);

			ob_start();
				$this->printAssetList($todo);
				$global_contents = ob_get_contents();
			ob_end_clean();
		}

		// if we have any blank keywords, replace them with nothing
		$global_contents = preg_replace('|%[^%\W]+%|', '', $global_contents);

		//Cart must pass on to some sort of ecommerce form
		$form_id = $this->attr('ecom_form_id');
		if (!empty($form_id)) {
			$ecom_form_url = $GLOBALS['SQ_SYSTEM']->am->getAssetURL($form_id);
			ob_start();
				?><input type="button" onclick="location.href = '<?php echo addslashes($GLOBALS['SQ_SYSTEM']->am->getAssetURL($form_id)); ?>'" value="<?php echo $this->attr('checkout_text'); ?>"><?php
				$checkout_cart_button = ob_get_contents();
			ob_end_clean();
		} else {
			$checkout_cart_button = '';
		}

		$field_name = 'result_page';
		$this->registerFormField($field_name);

		$value = array_get_index($_REQUEST, 'result_page', 1);
		hidden_field($field_name, $value);

		//line below see above
		$replacements['checkout_cart'] = $checkout_cart_button;
		$replacements['asset_listing'] = $global_contents;
		$replacements['asset_count'] = $num_kids;
		$replacements['product_count'] = $this->count();
		$replacements['total_value'] = $this->total();
		$replacements['page_asset_count'] = $num_assets_showing;
		$replacements['page_number'] = $result_page;
		$replacements['first_asset_position'] = $num_per_page * ($result_page - 1) + 1;
		$replacements['last_asset_position'] = min($num_kids, $num_per_page * $result_page);
		$replacements['root_nodes'] = implode(',', $this->getRootNodes());

		// print the contents of page - replacing the global keywords
		$format_bodycopy->setKeywordReplacements($replacements);

		ob_start();
			$format_bodycopy->printBody();
		ob_end_flush();

		// clean up after ourselves
		$GLOBALS['SQ_SYSTEM']->am->forgetAsset($bodycopy);

	}//end printContents()


	/**
	* Build a custom submit button for the ecom form since nested
	* forms are illegal html
	*
	* @return string
	* @access public
	*/
	function getSubmitButtonReplacement()
	{
		$prefix = $this->getPrefix();
		$field_name = 'update_button[all]';
		$this->registerFormField($field_name);

		$button_text = $this->attr('submit_button_text');

		ob_start();
			submit_button($field_name, $button_text);
		return ob_get_clean();

	}//end getSubmitButtonReplacement()


	/**
	* Paints the asset to the page
	*
	* @return void
	* @access public
	*/
	/*function printBody()
	{
		?><div align="center"><form action="<?php echo $this->getURL() ?>" method="post"><?php

		if (!is_null(array_get_index($_POST,'update_quantity',null))) {
			foreach($_POST['update_quantity'] as $product_assetid => $new_quantity) {
				if (is_numeric($new_quantity)) {
					$this->update($product_assetid, (int)$new_quantity);
				}
			}
			?><strong><?php echo translate('ecom_cart_quantities_changed'); ?></strong><?php
		}

		require_once SQ_LIB_PATH.'/html_form/html_form.inc';

		$cart = $this->get();
		?>
		<table width="50%">
		<thead><tr>
		<th><<?php echo translate('ecom_cart_product_name'); ?></th>
		<th><?php echo translate('qty'); ?></th><?php
		if ($this->attr('show_price')) {
			?><th><?php echo translate('price');    ?></th><?php
			?><th><?php echo translate('subtotal'); ?></th><?php
		}
		?></thead>
		<?php if ($this->attr('show_price')) {
			// the W3C recommendations say that TFOOT must come before TBODY
			// for some reason, so here it is
			?><tfoot><tr><?php
			?><td style="text-align:right"><strong><?php echo translate('total'); ?></strong></td><?php
			?><td style="text-align:right"><strong><?php echo $this->count(); ?></strong></td><?php
			?><td colspan="2" style="text-align:right"><strong><?php echo '$'.number_format($this->total(),2); ?></strong></td><?php
			?></tr>
			</tfoot><?php
		}
		?><tbody><?php
		if (empty($cart)) {
			?><tr><td colspan="<?php echo $this->attr('show_price') ? 4 : 2 ?>" style="text-align:center"><?php echo translate('ecom_cart_no_items_in_cart'); ?></td></tr><?php
		} else {
			foreach($cart as $product_assetid => $details) {
				?><tr><?php
				$asset_info = $GLOBALS['SQ_SYSTEM']->am->getAssetInfo(Array($product_assetid));
				?><td><a href="<?php echo $GLOBALS['SQ_SYSTEM']->am->getAssetURL($product_assetid) ?>"><?php echo $asset_info[$product_assetid]['name'] ?></a></td><?php
				?><td style="text-align:right"><?php text_box('update_quantity['.$product_assetid.']', $details['quantity'], 3) ?></td><?php
				if ($this->attr('show_price')) {
					?><td style="text-align:right"><?php echo '$'.number_format($details['price'],2) ?></td><?php
					?><td style="text-align:right"><?php echo '$'.number_format($details['quantity'] * $details['price'],2) ?></td><?php
				}
				?></tr><?php
			}
		}
		?></tbody>
		</table>

		<?php if (!empty($cart)) { ?>
			<input type="submit" value="<?php echo translate('ecom_cart_update_quantity'); ?>">
			<?php
			$form_id = $this->attr('ecom_form_id');
			if (!empty($form_id)) {
				$ecom_form_url = $GLOBALS['SQ_SYSTEM']->am->getAssetURL($form_id); ?>
			<input type="button" onclick="location.href = '<?php echo addslashes($ecom_form_url); ?>'" value="<?php echo $this->attr('checkout_text'); ?>">
			<?php } ?>
		<?php } ?>
		</form></div>
		<?php

	}*///end printBody()



}//end class

?>