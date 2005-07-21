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
* $Id: ecommerce_checkout.inc,v 1.7 2005/07/21 06:12:55 arailean Exp $
*
*/


require_once SQ_CORE_PACKAGE_PATH.'/page/page.inc';
require_once SQ_PACKAGES_PATH.'/ecommerce/lib/ecommerce_common.inc';

/**
* Ecommerce_Checkout
*
* Purpose
*
*
* @author  Andrei Railean <arailean@squiz.net>
* @version $Revision: 1.7 $
* @package MySource_Matrix_Packages
* @subpackage ecommerce
*/
class Ecommerce_Checkout extends Page
{

	/**
	* Screen Context
	*
	* Variable holding context to be printed by 'printContext' function
	* Context can change based on the logic of this asset.
	*
	*/
	var $print_context;



	/**
	* Constructor
	*
	* @param int	assetid	Id of this asset
	*
	* @return void
	* @access public
	*/
	function Ecommerce_Checkout($assetid=0)
	{
		// initialise the temporary order container so as to avoid extra checking
		if (!isset($_SESSION['SQ_TEMP_ORDER'])) {
			$_SESSION['SQ_TEMP_ORDER'] = Array();
		}
		$this->Page($assetid);

	}//end constructor


	/**
	* Returns a list of available contexts
	*
	* @return array		available contexts
	* @access private
	*/
	function _getAvailableContexts()
	{
		return Array(
				'content',
				'confirmation',
				'receipt',
			   );

	}//end _getAvailableContexts()


	/**
	* Returns an array of all the permitted links type, the type asset and the cardinality
	* In the form
	*
	*   Array('[link_type]' => Array('[type_code]' => Array('card' => [cardinality], 'exclusive' => [exclusive])));
	* Where:
	*   link_type   = SQ_LINK_TYPE_1|SQ_LINK_TYPE_2|SQ_LINK_TYPE_3|SQ_LINK_NOTICE
	*   cardinality = [max number of links]|'M'
	*   exclusive   = true|false
	*
	* @return array
	* @access private
	*/
	function _getAllowedLinks()
	{
		$allowed_links = parent::_getAllowedLinks();
		$allowed_links[SQ_LINK_NOTICE]['ecommerce_delivery_method'] = Array('card' => 'M', 'exclusive' => false);

		return $allowed_links;

	}//end _getAllowedLinks()


	/**
	* Returns name of the asset
	*
	* @param boolean	$short_name	whether or not we are after the short name or the full name
	*
	* @see Asset::_getName()
	* @access private
	*/
	function _getName($short_name=false)
	{
		return $this->attr('name');

	}//end _getName()


	/**
	* Called by the design to print the body of this asset
	*
	* @access public
	*/
	function printBody()
	{
		# this is a main print controller of the checkout form that determines which
		# screen we are on and what action to take next
		// process contents
		$this->_processInterface();

		// determine context

		bam($_REQUEST);
		$this->_printContext();

	}//end printBody()


	/**
	* TODO: function comment: _processInterface
	*
	*
	*
	* @param
	*
	* @return void
	* @access public
	*/
	function _processInterface()
	{
		$request_action = array_get_index($_REQUEST, $this->getPrefix().'_action');
		if (empty($request_action)){
			return;
		}

		$processing_data = array_get_index($_SESSION['SQ_TEMP_ORDER'], 'processing_data', Array());

		$action = key($request_action);
		$success = true;

		switch ($action) {
			case 'go_to_confirmation':

				$delivery_method_id = array_get_index($_REQUEST, 'sq_delivery_method_id');
				if (!is_null($delivery_method_id)) {
					$this->_tmp['delivery_method_id'] = $delivery_method_id;

					$delivery_asset =& $GLOBALS['SQ_SYSTEM']->am->getAsset($delivery_method_id);
					if (!empty($delivery_asset)) {
						$delivery_status = $delivery_asset->processInputInterface();
						$delivery_state = $delivery_asset->getState();
						$GLOBALS['SQ_SYSTEM']->am->forgetAsset($delivery_asset);
					} else {
						$this->print_context = 'content';
						return;
					}

					// create a temporary order
					$processing_data['delivery']['assetid']	= $delivery_method_id;
					$processing_data['delivery']['state']	= $delivery_state;

					if (!$delivery_status && false){
						// TODO: localise error
						trigger_error('Failed processing delivery information');
						$success = false;
					}
				}

				if (empty($_SESSION['SQ_CART']) && false) {
					// TODO: localise error
					trigger_error('The ecommerce cart is empty, cannot checkout');
					$success = false;
				}

				if ($success) {
					// move cart contents into the order
					$_SESSION['SQ_TEMP_ORDER']['cart_contents'] = get_cart();
					clear_cart();

					$this->print_context = 'confirmation';
				} else {
					$this->print_context = 'content';
				}

				$_SESSION['SQ_TEMP_ORDER']['processing_data'] = $processing_data;

			break;

			case 'return_to_checkout':
				$this->print_context = 'content';
			break;

			case 'confirm_order':
				// create order asset
				# $this->_createOrder();

				// unset temporary order
				unset($_SESSION['SQ_TEMP_ORDER']);

				$this->print_context = 'receipt';
			break;

		}

		return;

	}//end _processInterface()


	/**
	* TODO: function comment: _printContext
	*
	*
	*
	* @return void
	* @access public
	*/
	function _printContext()
	{
		$context = $this->print_context;

		// if no context is set, display the first one in the list
		if (empty($context)) {
			$context = array_shift($this->_getAvailableContexts());
		}

		$bodycopy_folder_link  = $GLOBALS['SQ_SYSTEM']->am->getLink($this->id, SQ_LINK_TYPE_2, 'folder', true, 'bodycopies');
		if (empty($bodycopy_folder_link)) {
			// TODO: Localise trigger_localised_error();
			trigger_error('Bodycopy Folder is missing');
			return;
		}
		$content_bodycopy_link = $GLOBALS['SQ_SYSTEM']->am->getLink($bodycopy_folder_link['minorid'], SQ_LINK_TYPE_2, 'bodycopy', true, $context);
		$content_bodycopy = &$GLOBALS['SQ_SYSTEM']->am->getAsset($content_bodycopy_link['minorid'], $content_bodycopy_link['minor_type_code']);
		if (is_null($content_bodycopy)) {
			// TODO: Localise trigger_localised_error();
			trigger_error('Cannot find appropriate bodycopy');
			return;
		}

		require_once SQ_FUDGE_PATH.'/general/text.inc';
		$keywords = $content_bodycopy->getKeywords();
		$replacements = Array();
		foreach($keywords as $keyword) {
			$replacements[$keyword] = $this->getKeywordReplacement($keyword);
		}

		$form_req = $this->_isFormRequired();

		if ($form_req) {
			echo '<form action="'.$_SERVER['PHP_SELF'].'" method="post">';
		}

		$content_bodycopy->setKeywordReplacements($replacements);
		$content_bodycopy->printBody();

		if ($form_req) {
			echo '</form>';
		}

	}//end printContext()


	/**
	* Perform any additional processing required during the creation of this asset
	*
	* Create bopycopy and folder assets when they are created
	*
	* @param array	&$link	information used to create the initial link
	*
	* @access private
	* @return boolean
	*/
	function _createAdditional(&$link)
	{
		if (!parent::_createAdditional($link)) return false;

		$success = true;

		$GLOBALS['SQ_SYSTEM']->am->includeAsset('folder');

		$sub_assets = Array(
						'bodycopies'	=> 'folder',
						'orders'		=> 'folder',
					  );

		$bodycopy_folder = null;
		foreach ($sub_assets as $link_value => $type) {
			$asset = new $type();
			$copy_link = Array(
							'asset'			=> &$this,
							'value'			=> $link_value,
							'link_type'		=> SQ_LINK_TYPE_2,
							'is_dependant'	=> 1,
							'is_exclusive'	=> 1,
						  );

			$asset->setAttrValue('name', ucwords(str_replace('_',' ', $link_value)));
			if (!$asset->create($copy_link)) {
				$success = false;
			}
			if ($link_value == 'bodycopies') {
				$bodycopy_folder = $asset;
			}

			$GLOBALS['SQ_SYSTEM']->am->forgetAsset($asset);
			unset($asset);

			if (!$success) break;
		}

		if ($success) {
			// now create context bodycopies
			$GLOBALS['SQ_SYSTEM']->am->includeAsset('bodycopy');
			foreach ($this->_getAvailableContexts() as $context_name) {
				$asset = new Bodycopy();

				$copy_link = Array(
								'asset'			=> &$bodycopy_folder,
								'value'			=> $context_name,
								'link_type'		=> SQ_LINK_TYPE_2,
								'is_dependant'	=> 1,
								'is_exclusive'	=> 1,
							  );

				$asset->setAttrValue('name', ucwords(str_replace('_',' ', $context_name)));

				$success = $asset->create($copy_link);
				unset($asset);
				if (!$success) break;
			}
		}

		return $success;

	}//end _createAdditional()


	/**
	* TODO: function _getDeliveryAssetIds
	*
	*
	*/
	function _getDeliveryAssetIds()
	{
		if (!isset($this->_tmp['delivery_ids'])) {
			$this->_tmp['delivery_ids'] = Array();

			$delivery_links = $GLOBALS['SQ_SYSTEM']->am->getLinks($this->id, SQ_LINK_NOTICE, '', false, 'major', 'delivery_method');
			foreach ($delivery_links as $one_link) {
				$this->_tmp['delivery_ids'][] = $one_link['minorid'];
			}
		}

		return $this->_tmp['delivery_ids'];

	}//end _getDeliveryAssetIds()


	/**
	* Register a form field name
	*
	* This helps determine whether the output should be wrapped in a form
	*
	* @param string	$field_name	Name of the form field
	*
	* @return void
	* @access private
	*/
	function _registerFormField($field_name)
	{
		if (!empty($field_name)) {
			$this->_tmp['registered_form_fields'][] = $field_name;
			$this->_registerFormRequirement();
		}

	}//end _registerFormField()



	/**
	* Register Form Requirement
	*
	* If an element of the asset's interface requires a form, this will register that requirement
	*
	* @return void
	* @access private
	*/
	function _registerFormRequirement()
	{
		$this->_tmp['form_required'] = true;

	}//end _registerFormField()


	/**
	* Is Form Required?
	*
	* This function can reply to the question whether the form tags are required in asset's interface
	* Form is required when _registerFormRequirement() was called
	*
	* @return boolean
	* @access private
	*/
	function _isFormRequired()
	{
		return array_get_index($this->_tmp, 'form_required', false);

	}//end _registerFormField()



	//// -- Keywords


	/**
	* Add valid keywords for this asset to an array of keywords when asked
	*
	* @param &object Asset	$asset	the asset that triggered the event
	* @param Array			$vars	the vars that get submitted by the broadcaster
	*								we add keywords to the $vars['keywords'] array
	*
	* @return boolean
	* @access private
	*/
	function onRequestKeywords(&$broadcaster, $vars=Array())
	{
		$keywords = Array();

		if (!is_array($vars['keywords'])) {
			$vars['keywords'] = Array();
		}

		$bodycopy_folder_link  = $GLOBALS['SQ_SYSTEM']->am->getLink($this->id, SQ_LINK_TYPE_2, 'folder', true, 'bodycopies');
		// not our bodycopy, just bail out
		if (empty($bodycopy_folder_link)) {
			return;
		} else {
			$bodycopy_folder_id = $bodycopy_folder_link['minorid'];
		}

		$broadcaster_parents = $GLOBALS['SQ_SYSTEM']->am->getParents($broadcaster->id, 'bodycopy', true);
		$content_bodycopy_value = null;
		foreach (array_keys($broadcaster_parents) as $bodycopy_id) {
			$content_bodycopy_link = $GLOBALS['SQ_SYSTEM']->am->getLinkByAsset($bodycopy_folder_id, $bodycopy_id);
			if (!empty($content_bodycopy_link) && !empty($content_bodycopy_link['value'])){
				$bodycopy_context = $content_bodycopy_link['value'];
				break;
			}
		}

		// no special bodycopy found
		if (empty($bodycopy_context) || !in_array($bodycopy_context, $this->_getAvailableContexts())) {
			return;
		}

		require_once SQ_FUDGE_PATH.'/general/text.inc';

		$context_kw_fn = '_get'.ucwords_no_space($bodycopy_context).'ContextKeywords';
		if (method_exists($this, $context_kw_fn)) {
			$vars['keywords'] = array_merge($vars['keywords'], $this->$context_kw_fn());
		}

	}//end onRequestKeywords()


	/**
	* Get keywords for 'Content' context
	*
	* @access private
	* @return array
	*/
	function _getContentContextKeywords()
	{
		// TODO: translate
		$keywords['delivery_selector']			= 'Delivery Method Selector';
		$keywords['delivery_form'] 				= 'Delivery Method Form Contents';
		$keywords['go_to_confirmation_button']	= 'Go To Confirmation Button';

		return $keywords;

	}//end _getContentKeywords()


	/**
	* Get keywords for 'Confirmation' context
	*
	* @access private
	* @return array
	*/
	function _getConfirmationContextKeywords()
	{
		// TODO: translate
		$keywords['checkout_summary']			= 'Checkout Summary';
		$keywords['confirm_order_button']		= 'Confirm Order Button';
		$keywords['return_to_checkout_button']	= 'Return to Checkout Button';

		return $keywords;

	}//end _getConfirmationKeywords()


	/**
	* Get keywords for 'Receipt' context
	*
	* @access private
	* @return array
	*/
	function _getReceiptContextKeywords()
	{
		// TODO: translate
		$keywords['receipt_summary']	= 'Receipt Summary';

		return $keywords;

	}//end _getReceiptKeywords()


	/**
	* TODO: function comment
	*
	*
	*
	* @return
	* @access public
	*/
	function getAvailableKeywords() {
		$keywords = parent::getAvailableKeywords();

		return $keywords;

	}


	//// -- Replacements


	/**
	* TODO: function comment
	*
	*
	*
	* @return
	* @access public
	*/
	function getDeliverySelectorKeywordReplacement() {
		$multiple_asset_info = $GLOBALS['SQ_SYSTEM']->am->getAssetInfo($this->_getDeliveryAssetIds());

		$selection_contents = Array();
		foreach($multiple_asset_info as $assetid => $asset_info) {
			$selection_contents[$assetid] = $asset_info['name'];
		}

		reset($selection_contents);
		$active_delivery_method = array_get_index($_SESSION['SQ_TEMP_ORDER'], 'delivery_assetid', key($selection_contents));

		$this->_registerFormRequirement();
		ob_start();
			?>
			<script type="text/javascript">
				//<![CDATA[
				var current_method_id = <?php echo is_null($active_delivery_method)?'null':$active_delivery_method; ?> ;

				function handleDeliverySelectChange(field) {
					selected_id = field.options[field.selectedIndex].value;

					if ( current_method_id != null ) {
						document.getElementById('sq_delivery_method_' + current_method_id).style.display = 'none';
					}

					current_method_id = selected_id;
					document.getElementById('sq_delivery_method_' + current_method_id).style.display = 'block';

				}
				//]]>
			</script>

			<?php
			combo_box('sq_delivery_method_id', $selection_contents, false, $active_delivery_method, 0, 'onchange="handleDeliverySelectChange(this)"');

		return ob_get_clean();

	}


	/**
	* TODO: function comment
	*
	*
	*
	* @return
	* @access public
	*/
	function getDeliveryFormKeywordReplacement()
	{

		$active_delivery_method = array_get_index($_SESSION['SQ_TEMP_ORDER'], 'delivery_assetid');

		$this->_registerFormRequirement();
		ob_start();
		foreach ($this->_getDeliveryAssetIds() as $one_assetid) {
			$delivery_asset =& $GLOBALS['SQ_SYSTEM']->am->getAsset($one_assetid);

			if (!is_null($active_delivery_method) && ($active_delivery_method == $one_assetid)) {
				$display = 'block';
				$delivery_asset->setState(array_get_index($this->_tmp, 'delivery_state'));
			} else {
				$display = 'none';
			}

			echo "\n".'<div id="sq_delivery_method_'.$one_assetid.'" style="display:'.$display.'" >'."\n".$delivery_asset->getInputInterface()."\n</div>\n";

			$GLOBALS['SQ_SYSTEM']->am->forgetAsset($delivery_asset);
		}

		return ob_get_clean();

	}


	/**
	* TODO: function getConfirmOrderButtonKeywordReplacement
	*
	*
	*/
	function getConfirmOrderButtonKeywordReplacement()
	{
		$field_value = $this->getPrefix().'_action[confirm_order]';

		$this->_registerFormField($field_value);
		ob_start();
			submit_button($field_value, $this->attr('confirm_order_button_text'));
		return ob_get_clean();

	}//end getConfirmOrderButtonKeywordReplacement()


	/**
	* TODO: function getConfirmOrderButtonKeywordReplacement
	*
	*
	*/
	function getReturnToCheckoutButtonKeywordReplacement()
	{
		$field_value = $this->getPrefix().'_action[return_to_checkout]';
		$this->_registerFormField($field_value);
		ob_start();
			submit_button($field_value, $this->attr('return_to_checkout_button_text'));
		return ob_get_clean();

	}//end getConfirmOrderButtonKeywordReplacement()


	/**
	* TODO: function getGoToConfirmationButtonKeywordReplacement
	*
	*
	*/
	function getGoToConfirmationButtonKeywordReplacement()
	{
		$field_value = $this->getPrefix().'_action[go_to_confirmation]';
		$this->_registerFormField($field_value);
		ob_start();
			submit_button($field_value, $this->attr('go_to_confirmation_button_text'));
		return ob_get_clean();

	}//end getGoToConfirmationButtonKeywordReplacement()


	/**
	* TODO: function getCheckoutSummaryKeywordReplacement
	*
	*
	*/
	function getCheckoutSummaryKeywordReplacement()
	{
		echo "Checkout Summary Replacement";

		$active_delivery_method = array_get_index($_SESSION['SQ_TEMP_ORDER'], 'delivery_assetid');
		$delivery_asset_state = array_get_index($_SESSION['SQ_TEMP_ORDER'], 'delivery_state');
		$delivery_asset =& $GLOBALS['SQ_SYSTEM']->am->getAsset($active_delivery_method);

		echo "Delivery Method Summary\n";
		echo $delivery_asset->getReceipt();

	}//end getCheckoutSummaryKeywordReplacement()


	/**
	* TODO: function getCheckoutSummaryKeywordReplacement
	*
	*
	*/
	function getReceiptSummaryKeywordReplacement()
	{
		echo "Receipt Summary Replacement\n";

		$active_delivery_method = array_get_index($_SESSION['SQ_TEMP_ORDER'], 'delivery_assetid');
		$delivery_asset_state = array_get_index($_SESSION['SQ_TEMP_ORDER'], 'delivery_state');
		$delivery_asset =& $GLOBALS['SQ_SYSTEM']->am->getAsset($active_delivery_method);

		echo "Delivery Method Receipt\n";
		echo $delivery_asset->getReceipt();

	}//end getCheckoutSummaryKeywordReplacement()


}//end class
?>
