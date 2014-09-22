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
* $Id: ecommerce_cart.inc,v 1.50 2013/08/15 05:25:30 ewang Exp $
*
*/

require_once SQ_PACKAGES_PATH.'/cms/listing_engine/listing_engine.inc';
require_once SQ_PACKAGES_PATH.'/ecommerce/lib/ecommerce_cart_processor.inc';

define('SQ_CART_PRESET_QUANTITY', 'Quantity');
define('SQ_CART_PRESET_REMOVE', 'Remove');


/**
* Ecommerce_Cart
*
* Purpose
*   A simple shopping cart used for ecommerce functions
*
*
* @author  Luke Wright <lwright@squiz.net>
* @author  Robert Howard <rhoward@squiz.net>
* @version $Revision: 1.50 $
* @package MySource_Matrix_Packages
* @subpackage ecommerce
*/
class Ecommerce_Cart extends Listing_Engine
{


	/**
	 * The ecommerce cart processor linked with this Ecommerce cart asset
	 *
	 * @var Ecommerce_Cart_Processor
	 */
	var $_cart = NULL;


	/**
	* Whether there are insufficient product quantities
	* @var boolean
	*/
	var $insufficient_quantities = FALSE;


	/**
	* Current assets which have insufficient quantities
	* @var object
	*/
	var $insufficient_quantity_assets = NULL;


	/**
	* Constructor
	*
	* @param string	$assetid	the assetid of the asset to load into this object
	*
	* @return void
	* @access public
	*/
	function __construct($assetid=0)
	{
		parent::__construct($assetid);

	}//end constructor


	/**
	* Creates bodycopies for this asset
	*
	* Overridden, as the Ecommerce_Cart uses the 'no_items_text' attribute instead of a No Results bodycopy
	*
	* @return boolean
	* @access protected
	*/
	function _createBodycopies()
	{
		$GLOBALS['SQ_SYSTEM']->am->includeAsset('bodycopy');
		$asset = new Bodycopy();
		$copy_link = Array('asset' => &$this, 'value' => 'page_contents' ,'link_type' => SQ_LINK_TYPE_2, 'is_dependant' => 1, 'is_exclusive' => 1);
		$asset->setAttrValue('name', 'Page Contents');
		return $asset->create($copy_link);

	}//end _createBodycopies()


	/**
	* Gets the current cart. If it doesn't yet exist, create a new one
	*
	* @return object
	* @access private
	*/
	function &_getCart()
	{
		if (is_null($this->_cart)) {
			$cart = new Ecommerce_Cart_Processor($this->attr('ecommerce_checkout_id'));

			if ($this->attr('use_local_cart')) {
				$cart->setCartContainer($_SESSION['sq_local_cart_contents']['cart_contents']);
			}

			$this->_cart = $cart;
		}

		return $this->_cart;

	}//end _getCart()


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
		$cart =& $this->_getCart();
		$cart_contents = $cart->getCart();

		$asset_list = Array();

		$group_by = $this->attr('group_by');

		foreach ($cart_contents as $product_assetid => $product_info) {
			$product_asset = $GLOBALS['SQ_SYSTEM']->am->getAsset($product_assetid);
			if (!empty($product_asset)) {
				if ($group_by == 'letter') {
					$asset_list[$product_assetid] = Array( 0 => Array(
																	'type_code'		=> $product_asset->type(),
																	'first_letter'	=> substr($product_asset->attr('name'), 0, 1),
																)
													);
				} else {
					$asset_list[$product_assetid] = Array( 0 => Array(
																	'type_code' => $product_asset->type()
																	)
														);
				}
			}
		}

		if (empty($asset_list)) $asset_list = '';

		return $asset_list;

	}//end getAssetList()


	/**
	* Returns an array of all the permitted link type, the type asset and the cardinality
	*
	* @return array
	* @access private
	* @see Asset::_getAllowLinks()
	*/
	function _getAllowedLinks()
	{
		$page_links = parent::_getAllowedLinks();
		$page_links[SQ_LINK_TYPE_1]['folder']   = Array('card' => 'M');
		$page_links[SQ_LINK_TYPE_1]['product']  = Array('card' => 'M');
		$page_links[SQ_LINK_TYPE_2]['bodycopy'] = Array('card' => 'M');
		return $page_links;

	}//end _getAllowedLinks()


	/**
	* Returns name of the asset
	*
	* @param boolean	$short_name	whether or not we are after the shortname or the full name
	* @param int		$contextid	what context to return the name from
	*
	* @return string
	* @access private
	* @see Asset::_getName()
	*/
	protected function _getName($short_name=FALSE, $contextid=NULL)
	{
		// No context specified, using the current context
		if ($contextid === NULL) {
			$contextid = $GLOBALS['SQ_SYSTEM']->getContextId();
		}//end if

		// Obtain the attribute value for Name from the specified Context
		$attr = ($short_name) ? 'short_name' : 'name';
		$values = $GLOBALS['SQ_SYSTEM']->am->getAttributeValuesByName($attr, $this->type(), Array($this->id), $contextid);
		if (empty($values) === TRUE) {
			return parent::_getName($short_name, $contextid);
		} else {
			return $values[$this->id];
		}

	}//end _getName()


	/**
	* Prints out the Frontend for this asset
	*
	* @return void
	* @access public
	*/
	function printFrontend()
	{

		// Tie the process and following paint together using db2 in order to
		// avoid possible replication slowdown on db1
		$GLOBALS['SQ_SYSTEM']->changeDatabaseConnection('db2');
		if($this->readAccess())
		    $this->processAssetSelections();
		parent::printFrontend();

		$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();

	}//end printFrontend()


	/**
	* Gets the list of current root nodes
	*
	* @return array
	* @access public
	*/
	function getRootNodes()
	{
		return Array();

	}//end getRootNodes()


	/**
	* Constructs the carts contents
	*
	* @return void
	* @access public
	*/
	function printContents()
	{
		$bodycopy_link = $GLOBALS['SQ_SYSTEM']->am->getLink($this->id, SQ_LINK_TYPE_2, 'bodycopy', TRUE, 'page_contents');
		if (empty($bodycopy_link)) return FALSE;

		$format_bodycopy = $GLOBALS['SQ_SYSTEM']->am->getAsset($bodycopy_link['minorid'], $bodycopy_link['minor_type_code']);
		if (is_null($format_bodycopy)) return FALSE;

		require_once SQ_FUDGE_PATH.'/general/text.inc';
		$keywords = $format_bodycopy->getKeywords();
		$replacements = Array();

		if (!in_array('asset_listing', $keywords)) {
			// there is no asset listing keyword in the contents, so we dont
			// need to do all the extra processing for the listing
			$format_bodycopy->printBody();
			return;
		}

		// the children here will refer to the contents of this cart
		$children = $this->getAssetList();
		$num_kids = count($children);
		$this->filterAssetList($children);

		// the list is sorted and positions set, now we can split to groups
		if ($this->attr('group_by') == 'grouped') {
			$this->groupAssetsRecursively($this->attr('asset_grouping'), $children, $children);
			$this->sortGroups($this->attr('asset_grouping'), $children, $children);
		}

		foreach ($keywords as $word) {
			$replacements[$word] = $this->getKeywordReplacement($word);
		}

		// get any extra keyword replacements from this asset
		$replacements = $this->getContentsKeywordReplacements($keywords) + $replacements;

		if (!empty($this->_tmp['cart_updated'])) {
			$replacements['cart_updated'] = $this->attr('cart_updated_text');
		} else {
			$replacements['cart_updated'] = '';
		}

		$num_per_page = $this->_getNumPerPage();

		$asset_result_page_var   = 'result_'.$this->id.'_result_page';
		$generic_result_page_var = 'result_page';

		// have [assetid]_result_page take precedence over result_page
		$result_page = 1;
		if (isset($_REQUEST[$generic_result_page_var])) {
			$result_page = (int)$_REQUEST[$generic_result_page_var];
		}
		if (isset($_REQUEST[$asset_result_page_var])) {
			$result_page = (int)$_REQUEST[$asset_result_page_var];
		}

		if ($result_page <= 0) $result_page = 1;

		// get our page chunk here
		if (empty($children)) {
			$replacements['asset_listing'] = $this->attr('no_items_text');
			$replacements['next_page'] = '';
			$replacements['previous_page'] = '';
			$replacements['total_pages'] = 1;

			// page list is one page
			$replacements['page_list'] = '1';
			$global_contents = $replacements['asset_listing'];
			$num_assets_showing = 0;
			$num_kids = 0;
			?>
			<script type="text/javascript">
			<!--
				select_list = [];
			//-->
			</script>
			<?php
		} else {
			$todo =& $this->getChunk($children, $replacements, Array(), $result_page, $num_per_page);
			$num_assets_showing = count($todo);

			ob_start();
				$this->printAssetList($todo);
				$global_contents = ob_get_contents();
			ob_end_clean();
		}

		// if we have any blank keywords, replace them with nothing
		$global_contents = preg_replace('|%[^%\W]+%|', '', $global_contents);

		// cart must pass on to an ecommerce checkout
		$checkout_id = $this->attr('ecommerce_checkout_id');
		$checkout_cart_button = '';
		$checkout_cart_href   = '';
		if (!empty($checkout_id) && !empty($children)) {
			if (!$GLOBALS['SQ_SYSTEM']->am->assetExists($checkout_id)) {
				trigger_localised_error('ECOM0003', E_USER_WARNING, $checkout_id);
			} else {
				$checkout_cart_href = $GLOBALS['SQ_SYSTEM']->am->getAssetURL($checkout_id);
				ob_start();
					?><input type="button" onclick="location.href = '<?php echo addslashes($checkout_cart_href); ?>'" value="<?php echo $this->attr('checkout_text'); ?>" /><?php
					$checkout_cart_button = ob_get_contents();
				ob_end_clean();
			}
		}


		// pagination

		if ($num_per_page > 0) {
			$this->registerFormField('result_page');
			hidden_field('result_page', $result_page);
		}

		// line below see above
		$replacements['checkout_cart_button']  = $checkout_cart_button;
		$replacements['checkout_cart_href']    = $checkout_cart_href;


		if (in_array('cart_update_button', $keywords)) {
			$replacements['cart_update_button']    = $this->getSubmitButtonKeywordReplacement();
		}
		$replacements['asset_listing']         = $global_contents;
		$replacements['asset_count']           = $num_kids;
		$replacements['page_asset_count']      = $num_assets_showing;
		$replacements['page_number']           = $result_page;
		$replacements['first_asset_position']  = $num_per_page * ($result_page - 1) + 1;
		$replacements['last_asset_position']   = min($num_kids, $num_per_page * $result_page);
		$replacements['root_nodes']            = implode(',', $this->getRootNodes());

		$replacements['select_all_js_code'] = $this->getSelectAllJSCodeKeywordReplacement();

		// print the contents of page - replacing the global keywords
		$format_bodycopy->setKeywordReplacements($replacements);

		ob_start();
			if (!in_array('select_all_js_code', $keywords)) {
				// there is no asset listing keyword in the contents, so we dont
				// need to do all the extra processing for the listing
				echo $this->getSelectAllJSCodeKeywordReplacement();
			}
			$format_bodycopy->printBody();
		ob_end_flush();

	}//end printContents()


	/**
	* Provide the default values for each of the selection fields
	* Ecom grabs its values from the cart
	*
	* @param string	$selection_name	the id of the asset selection
	* @param string	$assetid		the assetid of the item to grab out of the cart
	*
	* @return int
	* @access public
	*/
	function getAssetSelectionValue($selection_name, $assetid)
	{
		$cart =& $this->_getCart();
		$cart_contents = $cart->getCart();

		$selections = $this->attr('asset_selections');

		// if the hardcoded asset selection 'Quantity'
		if ($selection_name == 'Quantity' && isset($cart_contents[$assetid]['quantity'])) {
			return $cart_contents[$assetid]['quantity'];
		}

		// fallback
		return 0;

	}//end getAssetSelectionValue()


	/**
	* Prints the individual asset in the asset list
	* {description}
	* This is being overridden to cache the results of each asset, and add
	* the score and result number keyword replacements
	*
	* Note that the second parameter is score and not type code. This is because
	* we send the search results which is (assetid => score) instead of
	* (assetid => type_code). It still works - and allows us to add the score
	* keyword easily - but we turn it into the correct format when sending to
	* the listing engine.
	*
	* @param string	$assetid		The assetid of the asset we are printing
	* @param string	$list_position	the position of this asset in the list
	* @param array	$keywords		any keywords to replace when printing
	*
	* @return void
	* @access protected
	*/
	function _printAsset($assetid, $list_position, $keywords=Array())
	{
		ob_start();
		parent::_printAsset($assetid, $list_position, $keywords);
		$element_contents = ob_get_contents();
		ob_end_clean();

		// summon the quantities
		$cart =& $this->_getCart();
		$cart_contents = $cart->getCart();

		$element_replacements = Array(
									'product_quantity'	=> $cart_contents[$assetid]['quantity'],
								);
		$sub_total = $cart_contents[$assetid]['quantity'] * $cart_contents[$assetid]['price'];
		$element_replacements['product_total'] = sprintf("%.{$this->attr('float_precision')}f", $sub_total);
		$element_contents = replace_keywords($element_contents, $element_replacements);

		echo $element_contents;

	}//end _printAsset()


	/**
	* Returns a list of extra keywords that should be replaced for a particular
	* listing asset, in the format: Array(keyword name => replacement)
	*
	* This allows search page, for instance, to define its result number and
	* score fields so they don't blank out when keywords are replaced in listing
	* engine.
	*
	* @return array
	* @access public
	*/
	function getExtendedAssetKeywordReplacements()
	{
		// We are only replacing them with their own replacement so they can be
		// cached out like this, then score and result number can be added later
		return Array(
				'product_quantity'	=> '%product_quantity%',
				'product_total'		=> '%product_total%',
			   );

	}//end getExtendedAssetKeywordReplacements()


	/**
	* Preprocess the Asset Selection values before displaying the page or redirecting
	* Overwritten by the cart to handle the processing of cart updates using
	* preset asset selections and dynamic parameters
	*
	* @return boolean
	* @access public
	*/
	function processAssetSelections()
	{
		$cart =& $this->_getCart();
		$cart_contents = $cart->getCart();

		$parameter_map = $this->getAttribute('parameter_map');
		$this->_tmp['cart_updated'] = FALSE;

		$add_items = $parameter_map->getParameterValue('add_item');
		if (!empty($add_items)) {
			foreach ($add_items as $assetid => $quantity) {
				if (empty($assetid) || !strlen($quantity)) {
					continue;
				}

				if (isset($cart_contents[$assetid]['quantity'])) {
					// we updated the quantity for the product
					$amount = $cart_contents[$assetid]['quantity'];
					$asset = $GLOBALS['SQ_SYSTEM']->am->getAsset($assetid);
					if ($asset instanceof Donation) {
						//if the asset is a Donation, remove the existing amount to use the update amount only. Do not confuse the users how much he has donated
						$amount = 0;
					}

					if ($cart->updateItem($assetid, $this->_validateItemUpdate(SQ_CART_PRESET_QUANTITY, $amount+$quantity, $assetid))) {
						$this->_tmp['cart_updated'] = TRUE;
					}

				} else if ($this->_validateItemUpdate(SQ_CART_PRESET_QUANTITY, $quantity, $assetid) == $quantity) {
					// only add if it's not going to break our Quantity selection
					if ($cart->addItem($assetid, $quantity)) {
						$this->_tmp['cart_updated'] = TRUE;
					}
				}
			}
		}

		// internal, preset asset selections
		// quantity (update)
		if (isset($_REQUEST[SQ_CART_PRESET_QUANTITY])) {
			$quantity = $_REQUEST[SQ_CART_PRESET_QUANTITY];
			if (is_array($quantity)) {
				// checkbox, textbox, or drop-down
				$asset_list = $this->getAssetList();
				foreach ($asset_list as $assetid => $type_code) {
					$amount = 0;
					if (isset($quantity[$assetid])) {
						$amount = $quantity[$assetid];
					}
					if ($cart->updateItem($assetid, $this->_validateItemUpdate(SQ_CART_PRESET_QUANTITY, $amount, $assetid))) {
						$this->_tmp['cart_updated'] = TRUE;
					}
				}
			} else {
				// radio
				if ($cart->updateItem($quantity, $this->_validateItemUpdate(SQ_CART_PRESET_QUANTITY, 1, $assetid))) {
					$this->_tmp['cart_updated'] = TRUE;
				}
			}
		}

		// delete
		if (isset($_REQUEST[SQ_CART_PRESET_REMOVE])) {
			$remove = $_REQUEST[SQ_CART_PRESET_REMOVE];
			if (is_array($remove)) {
				// checkbox, textbox, or drop-down
				foreach ($remove as $assetid => $amount) {
					if ($cart->removeItem($assetid)) {
						$this->_tmp['cart_updated'] = TRUE;
					}
				}
			} else {
				// radio
				if ($cart->removeItem($remove)) {
					$this->_tmp['cart_updated'] = TRUE;
				}
			}
		}

		return TRUE;

	}//end processAssetSelections()


	/**
	* Checks that the new value for the selection is valid, and matches
	* the conditions set on the Asset Selection screen for that selection,
	* and modifies the quanitity to match these conditions if necessary
	*
	* Returns the new quantity
	*
	* @param string	$selection_name	the id of the asset selection
	* @param int	$quantity		the quantity to be assigned to the selection
	* @param int	$assetid		the id of the asset we are validating
	*
	* @return int
	* @access private
	*/
	function _validateItemUpdate($selection_name, $quantity, $assetid)
	{
		$asset = $GLOBALS['SQ_SYSTEM']->am->getAsset($assetid);
		if ($asset instanceof Donation) {
			return $quantity;
		}

		$selections     = $this->attr('asset_selections');
		$final_quantity = $quantity;

		// ensure we have enough items to add to the cart
		$checkout_id = $this->attr('ecommerce_checkout_id');
		if (!empty($checkout_id)) {
			if (!$GLOBALS['SQ_SYSTEM']->am->assetExists($checkout_id)) {
				trigger_localised_error('ECOM0003', E_USER_WARNING, $checkout_id);
			} else {
				$checkout_asset = $GLOBALS['SQ_SYSTEM']->am->getAsset($checkout_id);
				if ($checkout_asset->attr('enforce_quantities')) {
					// determine which to use: metadata field or attribute name
					$source = $checkout_asset->attr('quantity_selector');
					switch ($source) {
						case 'metadata':
						$quantity_field = $checkout_asset->attr('quantity_field');
						if (!empty($quantity_field)) {
							$mm =& $GLOBALS['SQ_SYSTEM']->getMetadataManager();
							$available_quantity = $mm->getMetadataValueByAssetid($assetid, $quantity_field);
							if ($quantity > $available_quantity) {
								$this->insufficient_quantities = TRUE;
								// add name of asset to error list for displaying on frontend
								$this->insufficient_quantity_assets[] = $asset->name;
								$final_quantity = $available_quantity;
							} else {
								$this->insufficient_quantities = FALSE;
							}
						}
						break;

						case 'attribute':
						$quantity_attribute = $checkout_asset->attr('quantity_attribute');
						if (!empty($quantity_attribute)) {
							$available_quantity = $asset->attr($quantity_attribute);
							if ($quantity > $available_quantity) {
								$this->insufficient_quantities = TRUE;
								// add name of asset to error list for displaying on frontend
								$this->insufficient_quantity_assets[] = $asset->name;
								$final_quantity = $available_quantity;
							} else {
								$this->insufficient_quantities = FALSE;
							}
						}
						break;
					}
					if (is_array($this->insufficient_quantity_assets)) {
						$this->insufficient_quantity_assets = array_unique($this->insufficient_quantity_assets);
					}
				}
			}//end else
		}//end if

		if (empty($selections[$selection_name]['options'])) {
			return $final_quantity;
		}
		$options = $selections[$selection_name]['options'];
		$restrict_range = FALSE;

		// allow_negative?
		if (isset($options['allow_negative']) && !$options['allow_negative']) {
			$restrict_range = TRUE;
			$min            = 0;

			// will always succeed
			$max            = NULL;
		}

		// if restrict_range is turned on
		if (isset($options['restrict_range']) && !empty($options['restrict_range']['enable'])) {
			$restrict_range = TRUE;

			$defaults = $this->getAssetSelectionDefaults();
			$min      = array_get_index($options['restrict_range'], 'min', $defaults['options']['restrict_range']['min']);
			$max      = array_get_index($options['restrict_range'], 'max', $defaults['options']['restrict_range']['max']);
		}

		// if this is a radio button or checkbox
		if ($selections[$selection_name]['type'] == 'radio') {
			$restrict_range = TRUE;
			$min = 0;
			$max = 1;
		}

		if ($restrict_range) {
			if ($quantity < $min) {
				$final_quantity = $min;
			} else if (!is_null($max) && $quantity > $max) {
				$final_quantity = $max;
			}
		}

		return $final_quantity;

	}//end _validateItemUpdate()


	/**
	* Return the available keywords
	*
	* The return value is in the form:
	* <pre>
	* Array(
	*     'keyword' => 'name',
	* )
	* </pre>
	*
	* @return array
	* @access public
	*/
	function getAvailableKeywords()
	{
		$keywords = parent::getAvailableKeywords();

		$keywords['product_count'] = translate('Cart Item Count');

		$keywords['total_value'] = translate('Cart Total Value');


		return $keywords;

	}//end getAvailableKeywords()


	/**
	* Add valid keywords for this asset to an array of keywords when asked
	*
	* @param object	&$broadcaster	the asset that triggered the event
	* @param array	$vars			the vars that get submitted by the broadcaster we add keywords to the $vars['keywords'] array
	*
	* @return boolean
	* @access private
	*/
	function onRequestKeywords(&$broadcaster, $vars=Array())
	{
		if (!isset($vars['keywords'])) return;
		if (!($broadcaster instanceof Content_Type)) return;
		$parents = $GLOBALS['SQ_SYSTEM']->am->getParents($broadcaster->id, 'bodycopy', TRUE);

		// the broadcaster could be our page contents bodycopy, in which case
		// we want to supply our own keyword replacements
		$page_contents_link = $GLOBALS['SQ_SYSTEM']->am->getLink($this->id, SQ_LINK_TYPE_2, 'bodycopy', TRUE, 'page_contents');
		if (!empty($page_contents_link)) {
			if (isset($parents[$page_contents_link['minorid']])) {
				$vars['keywords'] += $this->getContentsKeywords();
				return;
			}
		}

		// type folder links
		$folder =& $this->getFolder();
		$type_formats	= Array();
		if (!is_null($folder)) {
			$type_links = $GLOBALS['SQ_SYSTEM']->am->getLinks($folder->id, SQ_LINK_TYPE_2, 'bodycopy');
			foreach ($type_links as $link_info) {
				if (isset($parents[$link_info['minorid']])) {
					$type_formats[] = $link_info['value'];
				}
			}
		}

		$keywords	= Array();
		if (count($type_formats) > 0) {
			$keywords['product_quantity'] = translate('Product Quantity');

			$keywords['product_total'] = translate('Product Total');

		}

		$vars['keywords'] = array_merge($vars['keywords'], $keywords);

		// if it's not the page contents bodycopy, it's something that we've
		// inherited from the listing engine, so pass it on
		parent::onRequestKeywords($broadcaster, $vars);

	}//end onRequestKeywords()


	/**
	* Return the available keywords in the Page Contents Bodycopy for this asset
	*
	* The return value is in the form:
	* <pre>
	* Array(
	*     'keyword' => 'name',
	* )
	* </pre>
	*
	* @return array
	* @access public
	*/
	function getContentsKeywords()
	{
		$keywords = parent::_getContentsKeywords();
		$GLOBALS['SQ_SYSTEM']->lm->includeAssetStrings('page_asset_listing');

		$keywords += Array(
						'cart_updated'					=> translate('Cart Updated Message'),

						'product_count'					=> translate('Cart Item Count'),

						'total_value'					=> translate('Cart Total Value'),

						'cart_update_button'			=> translate('Cart Update Button'),

						'checkout_cart_button'			=> translate('Cart Checkout Button'),

						'checkout_cart_href'			=> translate('Cart Checkout Href'),

						'insufficient_quantity_message'	=> translate('Insufficient Product Quantity Message'),

						'asset_listing'					=> translate('Asset Listing'),

						'previous_page'					=> translate('Previous Page Link'),

						'next_page'						=> translate('Next Page Link'),

						'page_list'						=> translate('Page List'),

						'page_number'					=> translate('Page Number'),

						'asset_count'					=> translate('Total Assets Listed'),

						'page_asset_count'				=> translate('Assets Listed On This Page'),

						'first_asset_position'			=> translate('First Asset on Page\'s Position Number'),

						'last_asset_position'			=> translate('Last Asset on Page\'s Position Number'),

					 );

		return $keywords;

	}//end getContentsKeywords()


	/**
	* Get insufficient_quantity_message keyword replacement
	*
	* @return string
	* @access protected
	*/
	function getInsufficientQuantityMessageKeywordReplacement()
	{
		if ($this->insufficient_quantities) {
			$checkout_id = $this->attr('ecommerce_checkout_id');
			if (!empty($checkout_id)) {
				if (!$GLOBALS['SQ_SYSTEM']->am->assetExists($checkout_id)) {
					trigger_localised_error('ECOM0003', E_USER_WARNING, $checkout_id);
				} else {
					$checkout_asset = $GLOBALS['SQ_SYSTEM']->am->getAsset($checkout_id);
					ob_start();
					echo $checkout_asset->attr('quantity_message_cart');
					echo '<ul>';
					foreach ($this->insufficient_quantity_assets as $asset_name) {
						echo '<li>';
						echo $asset_name;
						echo '</li>';
					}
					echo '</ul>';
					return ob_get_clean();
				}
			}
		}

	}//end getInsufficientQuantityMessageKeywordReplacement()


	/**
	* Product Count keyword replacement
	*
	* @return string
	* @access private
	*/
	function getProductCountKeywordReplacement()
	{
		$cart =& $this->_getCart();

		return (string)$cart->getCount();

	}//end getProductCountKeywordReplacement()


	/**
	* Total Value keyword replacement
	*
	* @return string
	* @access private
	*/
	function getTotalValueKeywordReplacement()
	{
		$cart =& $this->_getCart();

		return sprintf("%.{$this->attr('float_precision')}f", $cart->getTotal());

	}//end getTotalValueKeywordReplacement()


	/**
	* Get the current form submit method (always POST)
	*
	* @return string
	* @access protected
	*/
	function _getFormSubmitMethod()
	{
		return 'post';

	}//end _getFormSubmitMethod()


}//end class

?>
