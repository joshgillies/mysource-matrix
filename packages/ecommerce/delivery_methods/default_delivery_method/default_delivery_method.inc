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
*/

require_once SQ_PACKAGES_PATH.'/ecommerce/delivery_methods/ecommerce_delivery_method/ecommerce_delivery_method.inc';
require_once SQ_LIB_PATH.'/html_form/html_form.inc';
require_once SQ_LIB_PATH.'/xml_array/xml_array.inc';
require_once SQ_FUDGE_PATH.'/general/www.inc';
require_once SQ_PACKAGES_PATH.'/ecommerce/lib/ecommerce_cart_processor.inc';


/**
* Default_Delivery_Method
*
* Purpose
*
*
* @version $Revision: 1.30.6.1 $
* @package MySource_Matrix_Packages
* @subpackage ecommerce
*/
class Default_Delivery_Method extends Ecommerce_Delivery_Method
{


	function Default_Delivery_Method($assetid=0)
	{
		$this->_ser_attrs = TRUE;
		$this->Ecommerce_Delivery_Method($assetid);

	}//end constructor


	/**
	* create the additional assets
	*
	* @param object	&$link	link
	*
	* @return boolean
	* @access public
	*/
	function _createAdditional(&$link)
	{
		if (!parent::_createAdditional($link)) return FALSE;

		$sub_assets = Array(
						'form'			=> Array(
											'type_code'	=> 'simple_form',
											'name'		=> 'Form Contents',
											'link_type'	=> SQ_LINK_TYPE_2,
											'exclusive'	=> FALSE,
										   ),
						'bodycopies'	=> Array(
											'type_code'	=> 'folder',
											'name'		=> 'Bodycopies',
											'link_type'	=> SQ_LINK_TYPE_2,
											'exclusive'	=> TRUE,
										   ),
					  );

		$success = TRUE;
		$bodycopy_folder = NULL;
		foreach ($sub_assets as $link_value => $sub_asset) {
			$asset_type = $sub_asset['type_code'];
			$GLOBALS['SQ_SYSTEM']->am->includeAsset($asset_type);

			$asset = new $asset_type();
			$copy_link = Array(
							'asset'			=> &$this,
							'value'			=> $link_value,
							'link_type'		=> $sub_asset['link_type'],
							'is_dependant'	=> 1,
							'is_exclusive'	=> $sub_asset['exclusive'],
						 );

			$asset->setAttrValue('name', $sub_asset['name']);
			$success = $asset->create($copy_link);

			if ($link_value == 'bodycopies') {
				$bodycopy_folder = $asset;
			}

			$GLOBALS['SQ_SYSTEM']->am->forgetAsset($asset);
			unset($asset);

			if (!$success) break;
		}

		// now create bodycopies
		if ($success) {
			$GLOBALS['SQ_SYSTEM']->am->includeAsset('bodycopy');
			foreach ($this->getAvailableBodycopies() as $link_value => $bodycopy_name) {
				$asset = new Bodycopy();

				$copy_link = Array(
								'asset'			=> &$bodycopy_folder,
								'value'			=> $link_value,
								'link_type'		=> SQ_LINK_TYPE_2,
								'is_dependant'	=> 1,
								'is_exclusive'	=> 1,
							 );

				$asset->setAttrValue('name', $bodycopy_name);

				$success = $asset->create($copy_link);
				unset($asset);
				if (!$success) break;
			}
		}

		return TRUE;

	}//end _createAdditional()


	/**
	* get confirmation summary
	*
	* @return boolean
	* @access public
	*/
	function getConfirmationSummary()
	{
		$bodycopy =& $this->getBodycopy('confirmation');

		// get an array with all the values entered by the user
		$form_values = unserialize($_SESSION['order_extra_information']);

		// create the replacement keyword array
		$keyword_remplacements = Array();

		// get all the keywords from the bodycopy
		$confirmation_keywords = $bodycopy->getKeywords();

		// create the keyword replacements with values entered by the user
		foreach ($confirmation_keywords as $keyword) {
			$key_parts = explode('_', $keyword);
			if ((count($key_parts) == 2 ) && ($key_parts[0] == 'form')) {

				$keyword_remplacements[$keyword] = $form_values[$key_parts[1]];
			}
		}
		$bodycopy->setKeywordReplacements($keyword_remplacements);

		ob_start();
		$bodycopy->printBody();
		$contents = ob_get_contents();
		ob_end_clean();

		return $contents;

	}//end getConfirmationSummary()


	/**
	* get the keywords on request
	*
	* @param object	&$broadcaster	broadcaster
	* @param array	$vars			vars
	*
	* @return boolean
	* @access public
	*/
	function onRequestKeywords(&$broadcaster, $vars=Array())
	{
		$keywords = Array();

		if (!is_array($vars['keywords'])) {
			$vars['keywords'] = Array();
		}

		$bodycopy_folder_link  = $GLOBALS['SQ_SYSTEM']->am->getLink($this->id, SQ_LINK_TYPE_2, 'folder', TRUE, 'bodycopies');
		// not our bodycopy, just bail out
		if (empty($bodycopy_folder_link)) {
			return;
		} else {
			$bodycopy_folder_id = $bodycopy_folder_link['minorid'];
		}

		$broadcaster_parents = $GLOBALS['SQ_SYSTEM']->am->getParents($broadcaster->id, 'bodycopy', TRUE);
		$content_bodycopy_value = NULL;
		foreach (array_keys($broadcaster_parents) as $bodycopy_id) {
			$content_bodycopy_link = $GLOBALS['SQ_SYSTEM']->am->getLinkByAsset($bodycopy_folder_id, $bodycopy_id);
			if (!empty($content_bodycopy_link) && !empty($content_bodycopy_link['value'])) {
				$bodycopy_context = $content_bodycopy_link['value'];
				break;
			}
		}

		// check if the found bodycopy context is recognised
		$contexts = $this->getAvailableBodycopies();
		if (empty($bodycopy_context) || !isset($contexts[$bodycopy_context])) {
			return;
		}

		require_once SQ_FUDGE_PATH.'/general/text.inc';

		$context_kw_fn = '_get'.ucwords_no_space($bodycopy_context).'BodycopyKeywords';
		if (method_exists($this, $context_kw_fn)) {
			$vars['keywords'] = array_merge($vars['keywords'], $this->$context_kw_fn());
		}

	}//end onRequestKeywords()


	/**
	* Get the Confirmation keywords
	*
	* @return array
	* @access public
	*/
	function _getConfirmationBodycopyKeywords()
	{
		$keywords = Array();
		$form =& $this->getForm();

		$question_values = $form->getQuestions();
		foreach ($question_values as $key => $value) {
			$key_parts = explode( ':', $key);
			$keywords['form_'.$key_parts[1]] = $value['name'].' ('.$key_parts[1].')';
		}
		return $keywords;

	}//end _getConfirmationBodycopyKeywords()


	/**
	* Get the Success keywords
	*
	* @return array
	* @access public
	*/
	function _getSuccessBodycopyKeywords()
	{
		$keywords = Array();
		$form =& $this->getForm();

		$question_values = $form->getQuestions();
		foreach ($question_values as $key => $value) {
			$key_parts = explode( ':', $key);
			$keywords['form_'.$key_parts[1]] = $value['name'].' ('.$key_parts[1].')';
		}
		return $keywords;

	}//end _getSuccessBodycopyKeywords()


	/**
	* Get the Failure keywords
	*
	* @return array
	* @access public
	*/
	function _getFailureBodycopyKeywords()
	{
		$keywords = Array();
		$form =& $this->getForm();

		$question_values = $form->getQuestions();
		foreach ($question_values as $key => $value) {
			$key_parts = explode( ':', $key);
			$keywords['form_'.$key_parts[1]] = $value['name'].' ('.$key_parts[1].')';
		}
		return $keywords;

	}//end _getFailureBodycopyKeywords()


	/**
	* get allowed links
	*
	* @return array
	* @access public
	*/
	function _getAllowedLinks()
	{
		$links = parent::_getAllowedLinks();
		$links[SQ_LINK_TYPE_2] = Array(
									'simple_form'	=> Array(
														'card'		=> '1',
														'exclusive'	=> FALSE,
													   ),
									'folder'		=> Array(
														'card'		=> '1',
														'exclusive'	=> TRUE,
													   ),
								 );

		return $links;

	}//end _getAllowedLinks()

	
	/**
	* Check if question type can be used for pass through variable
	*
	* @param string	$type	question type code
	*
	* @return boolean
	* @access public
	*/
	function _isAllowedPassThroughQuestionType($type='')
	{
		switch ($type) {
			case('form_question_type_text'):
			case('form_question_type_numeric'):
			case('form_question_type_email_address'):
			case('form_question_type_select'):
			case('form_question_type_option_list'):
			case('form_question_type_tickbox_list'):
			case('form_question_type_tickbox_table'):
			case('form_question_type_country'):
				return TRUE;

			default:
				return FALSE;
		}

	}//end _isAllowedPassThroughQuestionType()
	

	/**
	* Get the XML representation of this delivery method to store in the order_xml attribute of Ecommerce Order asset
	*
	* @return string
	* @access public
	*/
	function getDeliveryMethodXML()
	{
		$xml_str = '';
		
		$form =& $this->getForm();
		$questions = $form->getQuestions();
		$answers = unserialize($_SESSION['order_extra_information']);
		
		foreach ($questions as $q_id => $question_data) {
			$id_parts = explode(':', $q_id);
			$answer = array_get_index($answers, $id_parts[1], '');
			
			$xml_str .= <<<HEREDOC

			<question>
				<id><![CDATA[$q_id]]></id>
				<name><![CDATA[{$question_data['name']}]]></name>
				<answer><![CDATA[$answer]]></answer>
			</question>
HEREDOC;
		}

		$xml_str = <<<HEREDOC
	<delivery_method>
		<questions>$xml_str
		</questions>
	</delivery_method>
HEREDOC;
		
		return $xml_str;
		
	}//end getDeliveryMethodXML()
	
	
	/**
	* Get the Receipt
	*
	* @param boolean	$html	if true means we are returning a html format
	*
	* @return string
	* @access public
	*/
	function getReceipt($html=FALSE)
	{
		ob_start();
		if ($html) echo '<pre><ul>';

		$form =& $this->getForm();
		$question_values = $form->getQuestions();
		$answers = unserialize($_SESSION['order_extra_information']);
		foreach ($question_values as $key => $value) {
			$key_parts = explode(':', $key);
			if ($html) {
				echo '<li>'.$value['name'].' ('.$key_parts[1].') : '.$answers[$key_parts[1]].'</li>';
			} else {
				echo $value['name'].' ('.$key_parts[1].') : '.$answers[$key_parts[1]]."\n";
			}
		}

		if ($html) echo '</ul></pre>';

		$receipt = ob_get_contents();
		ob_end_clean();

		return $receipt;

	}//end getReceipt()


	/**
	* get the receipt summary
	*
	* @return string
	* @access public
	*/
	function getReceiptSummary()
	{
		$transaction_id = 0;
		$bodycopy =& $this->getBodycopy('success');

		$bodycopy->setKeywordReplacements($this->_getReceiptKeywordReplacements());

		// if the order id is not empty get the asset and get the form values entered from the user
		if (!empty($this->state['transaction_id'])) {
			$tmp_order = $GLOBALS['SQ_SYSTEM']->am->getAsset($this->state['transaction_id']);
			$transaction_id = $this->state['transaction_id'];
			// get the keywords from the bodycopy and replace them with the values entered by the user
			$keywords = $bodycopy->getKeywords();
			$form_values = $tmp_order->attr('order_form_value');
			$keyword_replacements = Array();
			foreach ($keywords as $keyword) {
				$key_parts = explode('_', $keyword);
				if ((count($key_parts) == 2) && ($key_parts[0] == 'form')) {
					$keyword_replacements[$keyword] = $form_values[$key_parts[1]];
				}
			}
			$bodycopy->setKeywordReplacements($keyword_replacements);

		}


		ob_start();
		if (!empty($transaction_id)) {
			echo '<p id="order_id"><span id="order_id_title">Order ID:</span> '.$transaction_id.'</p>';
		}
		$bodycopy->printBody();
		$contents = ob_get_contents();
		ob_end_clean();

		return $contents;

	}//end getReceiptSummary()


	/**
	* get the receipt keyword replacement
	*
	* @return array
	* @access public
	*/
	function _getReceiptKeywordReplacements()
	{
		$keywords = Array();
		return $keywords;

	}//end _getReceiptKeywordReplacements()


	/**
	* return the available bodycopies
	*
	* @return array
	* @access public
	*/
	function getAvailableBodycopies()
	{
		return Array(
				'success'		=> 'Success',
				'failure'		=> 'Failure',
				'confirmation'	=> 'Confirmation',
			   );

	}//end getAvailableBodycopies()


	/**
	* get the input interface
	*
	* @return boolean
	* @access public
	*/
	function getInputInterface()
	{
		$interface = parent::getInputInterface();

		$form =& $this->getForm();

		ob_start();
		$form->printBody(FALSE, FALSE);
		$contents = ob_get_contents();
		ob_end_clean();

		return $contents;

	}//end getInputInterface()


	/**
	* process the input interface
	*
	* @return mixed boolean|array	Array containing form questions and answers
	* @access public
	*/
	function processInputInterface()
	{
		$form =& $this->getForm();
		$state = $this->getState();

		$ok =  $form->process();

		// add the form values entered by the user in a session variable
		$question_values = $form->getQuestions();
		$answers = $form->getAnswers();
		$order_extra_info = Array();
		foreach ($question_values as $key => $value) {
			$tmp_value = '';
			if (!empty($answers[$key]['summary'])) {
				$tmp_value = $answers[$key]['summary'];
			} else if (!empty($answers[$key]['value'])) {
				$tmp_value = $answers[$key]['value'];
			}
			$key_parts = explode(':', $key);
			$order_extra_info[$key_parts[1]] = $tmp_value;
			//if the question is a country question, stores the value for pass-through variables later
			if ($answers[$key]['type_code'] == 'form_question_type_country') {
				$order_extra_info[$key] = $answers[$key]['value'];
			}
		}
		
		$_SESSION['order_extra_information'] =  serialize($order_extra_info);

		if (!$ok) {
			return FALSE;
		} else {
			return $answers;
		}

	}//end processInputInterface()


	/**
	* get the errors
	*
	* @return array
	* @access public
	*/
	function getErrors()
	{
		$form =& $this->getForm();
		return $form->getErrors();

	}//end getErrors()


	/**
	* get the form of the delivery method
	*
	* @return object
	* @access public
	*/
	function &getForm()
	{
		if (empty($this->_tmp['form'])) {
			$this->_tmp['form'] = NULL;

			$form_link = $GLOBALS['SQ_SYSTEM']->am->getLink($this->id, SQ_LINK_TYPE_2, 'simple_form', TRUE, 'form');
			if ($form_link) {
				$this->_tmp['form'] = $GLOBALS['SQ_SYSTEM']->am->getAsset($form_link['minorid'], 'simple_form');
			}
		}

		return $this->_tmp['form'];

	}//end getForm()


	/**
	* get the state
	*
	* @return string
	* @access public
	*/
	function getStatusMessage()
	{
		switch ($status) {
			case 0:
				return 'Processing';
			break;

			case 1:
				return 'Sent to mail server';
			break;

			case 2:
			default:
				return 'Failed';
			break;
		}

	}//end getStatusMessage()


	/**
	* get the state
	*
	* @return string
	* @access public
	*/
	function validateState()
	{
		$state = $this->getState();
		return TRUE;

	}//end validateState()


	/**
	* get the failure summary
	*
	* @return string
	* @access public
	*/
	function getFailureSummary()
	{
		$bodycopy =& $this->getBodycopy('failure');

		ob_start();
		$bodycopy->printBody();
		$contents = ob_get_contents();
		ob_end_clean();

		return $contents;

	}//end getFailureSummary()


	/**
	* get the bodycopy
	*
	* @param string	$name	name of the bodycopy
	*
	* @return object
	* @access public
	*/
	function &getBodycopy($name='')
	{
		$null = NULL;
		if (!$name) return $null;
		$am =& $GLOBALS['SQ_SYSTEM']->am;

		$folder_link = $GLOBALS['SQ_SYSTEM']->am->getLink($this->id, SQ_LINK_TYPE_2, 'folder', TRUE, 'bodycopies');
		if (empty($folder_link)) return $null;

		$folder = $am->getAsset($folder_link['minorid'], $folder_link['minor_type_code']);
		if (is_null($folder)) return $null;

		$bodycopy_link = $GLOBALS['SQ_SYSTEM']->am->getLink($folder->id, SQ_LINK_TYPE_2, 'bodycopy', TRUE, $name);
		if (empty($bodycopy_link)) return $null;

		$bodycopy = $am->getAsset($bodycopy_link['minorid'], $bodycopy_link['minor_type_code']);

		if (!empty($bodycopy)) return $bodycopy;

		return NULL;

	}//end getBodycopy()


	/**
	* This function process the delivery
	*
	* @param object		&$order					Ecommerce_Order asset
	* @param object		&$contents_bodycopy		Receipt contents bodycopy
	* @param object		&$format_bodycopy		Receipt format bodycopy
	* @param string		$currency_symbol		Symbol to use as the currency for all prices
	* @param int		$float_precision		Precision of the floating numbers used for prices
	* @param string		$taxation_name			Name of the tax
	* @param float		$taxation_rate			Tax value
	* @param array		$flat_charges			Array('Charge Name' => 'Price')
	* @param string		$neg_quantity_list		List specifying which products have negative quantities
	* @param string		$neg_quantity_message	Message explaining negative quantities list
	*
	* @return boolean
	* @access public
	*/
	function processDelivery(&$order, &$contents_bodycopy, &$format_bodycopy, $currency_symbol='$', $float_precision=2, $taxation_name='GST', $taxation_rate=0.0, $flat_charges=Array(), $neg_quantity_list='', $neg_quantity_message='')
	{
		$this->state['transaction_id']  = $order->id;

		$order->setOrderStatus(SQ_ECOM_ORDER_STATUS_PROCESSING, 'Pending');
		
		if ($this->attr('send_emails')) {
			$email_receipt_attr = $this->getAttribute('receipt_emails_format');
			$email_recipient_attr = $this->getAttribute('recipient_emails_format');

			$receipt_value = @unserialize($email_receipt_attr->value);
			$recipient_value = @unserialize($email_recipient_attr->value);

			// get all the keyword replacements
			$keywords_replacements = Array();
			$keywords_to_replace = Array();

			// get order id keyword replacements
			$keywords_to_replace[] = 'order_id';
			$keywords_replacements[] =  $this->state['transaction_id'];
			
			// get transaction reference id keyword replacements
			$keywords_to_replace[] = 'transaction_id';
			$keywords_replacements[] =  $order->attr('transaction_id');

			// get merchant transaction reference id keyword replacements
			$keywords_to_replace[] = 'merchant_transaction_id';
			$keywords_replacements[] =  $order->attr('ecom_ref_no');
			
			// get keyword replacements for order receipt
			$keywords_to_replace[] = 'order_receipt';

			// get the order summary from the cart
			$cart = new Ecommerce_Cart_Processor();
			$cart->setCartContainer($_SESSION['sq_local_cart_contents']['cart_contents']);
			$tmp_order_receipt_cart = '<pre>';
			$tmp_order_receipt_cart_text = '';
			ob_start();
			echo 'Order ID: '.$this->state['transaction_id']."\n\n";

			$remove_tax = isset($_SESSION['sq_region_specific']['remove_tax']) ? $_SESSION['sq_region_specific']['remove_tax'] : FALSE;
			$cart->printReceipt($taxation_name, $taxation_rate, $flat_charges, $remove_tax, $currency_symbol, $float_precision);

			$tmp_order_receipt_cart_text .= ob_get_contents();
			ob_end_clean();
			$tmp_order_receipt_cart .= $tmp_order_receipt_cart_text.'</pre>';

			// get the form summary
			$tmp_order_receipt_form = '<pre>'.$this->getReceipt(TRUE).'</pre>';
			$keywords_replacements[] =  $tmp_order_receipt_cart.$tmp_order_receipt_form;

			// get keyword replacements for cart summary
			$keywords_to_replace[] = 'cart_summary';
			$keywords_replacements[] = $tmp_order_receipt_cart;

			$keywords_to_replace[] = 'cart_summary_text';
			$keywords_replacements[] = $tmp_order_receipt_cart_text;

			// get keyword replacements for customised receipt
			if (!is_null($contents_bodycopy) && !is_null($format_bodycopy)) {
				$keywords_to_replace[] = 'customised_receipt';
				$keywords_replacements[] = $cart->printCustomisedReceipt($contents_bodycopy, $format_bodycopy, $taxation_name, $taxation_rate, $flat_charges, $remove_tax, $currency_symbol, $float_precision);
			}

			// get keyword replacements for form summary
			$keywords_to_replace[] = 'form_summary';
			$keywords_replacements[] = $tmp_order_receipt_form;

			$keywords_to_replace[] = 'form_summary_text';
			$keywords_replacements[] = $this->getReceipt(FALSE);

			// get negative quantities replacements
			// used only if payment gateway is processed and there is not enough product quantity before payment goes through
			$keywords_to_replace[] = 'negative_quantity_product_list';
			$keywords_replacements[] = $neg_quantity_list;
			$keywords_to_replace[] = 'negative_quantity_message';
			$keywords_replacements[] = $neg_quantity_message;

			// add replacements for form questions
			$form_values = unserialize($_SESSION['order_extra_information']);
			$tmp_form =& $this->getForm();
			$tmp_questions = $tmp_form->getQuestions();
			foreach ($tmp_questions as $key => $value) {
				$key_parts = explode(':', $key);
				$keywords_to_replace[] = 'response_'.$key_parts[1].'';
				if (!empty($form_values[$key_parts[1]])) {
					$keywords_replacements[] = $form_values[$key_parts[1]];
				} else {
					$keywords_replacements[] = '';
				}
			}

			// create the keyword replacement array
			$send_mail_replacements = Array();
			for ($i = 0; $i < count($keywords_to_replace); $i++) {
				$send_mail_replacements = array_merge($send_mail_replacements, Array($keywords_to_replace[$i] => $keywords_replacements[$i]));
			}

			// send emails if we need to
			$email_recipient_attr->sendMail($send_mail_replacements);
			$email_receipt_attr->sendMail($send_mail_replacements);
		}//end if

		$success = TRUE;

		if ($success) {
			$order->setOrderStatus(SQ_ECOM_ORDER_STATUS_SUCCESS);
		} else {
			$order->setOrderStatus(SQ_ECOM_ORDER_STATUS_FAILED);
		}
		$order->setAttrValue('order_form_value', unserialize($_SESSION['order_extra_information']));

		return TRUE;

	}//end processDelivery()


	/**
	* Sets up SESSION vars for the payment gateway and sends user to it
	* Called by the checkout asset
	*
	* @param float	$total_cost 		The total cost of all the items in the cart
	* @param int	$gateway_assetid	Gateway to use to process the payment
	*
	* @return void
	* @access public
	*/
	function processPaymentGateway($total_cost, $gateway_assetid='')
	{
		// If no gateway is supplied, use the first one from the list
		if (empty($gateway_assetid)) {
			$gateways = $this->attr('payment_gateways');
			$gateway_assetid = $gateways[0];
		}

		$gateway_asset =& $GLOBALS['SQ_SYSTEM']->am->getAsset($gateway_assetid);
		$gateway_asset_url = $gateway_asset->getURL();
		// set up expected session vars
		$_SESSION['SQ_ECOM_AMOUNT'] = $total_cost;
		// get checkout asset URL
		$checkout_link = $GLOBALS['SQ_SYSTEM']->am->getLink($this->id, SQ_LINK_NOTICE, 'ecommerce_checkout', TRUE, 'delivery_method', 'minor');
		$checkout_url = $GLOBALS['SQ_SYSTEM']->am->getAssetURL($checkout_link['majorid']);
		$checkout =& $GLOBALS['SQ_SYSTEM']->am->getAsset($checkout_link['majorid']);

		$_SESSION['SQ_ECOM_SUCCESS_URL'] = $checkout_url.'?'.$checkout->getPrefix().'_action=payment_gateway_successful';
		$_SESSION['SQ_ECOM_CANCEL_URL'] = $checkout_url.'?'.$checkout->getPrefix().'_action=return_to_checkout';
		$_SESSION['SQ_ECOM_REF_NO'] = md5(session_id().time());
		
		//Add the pass through variables to SESSION variables
		$pass_through_vars = $this->_getPassThroughVars();
		if (!empty($pass_through_vars)) {
			$_SESSION['SQ_ECOM_PASS_THROUGH_VARS'] = $pass_through_vars;
		}

		$pending_order_id = array_get_index($_SESSION['sq_local_cart_contents'], 'pending_order_id', NULL);
		$pending_order = $GLOBALS['SQ_SYSTEM']->am->getAsset($_SESSION['sq_local_cart_contents']['pending_order_id']);

		$pending_order->setAttrValue('ecom_ref_no', $_SESSION['SQ_ECOM_REF_NO']);
		$pending_order->appendAuditLog(translate('ecom_ref_num_recorded_before_processing'));

		header('Location: '.$gateway_asset_url);

	}//end processPaymentGateway()

	
	/**
	* Get the pass through variables under key - value pair array
	*
	* @return array
	* @access private
	*/
	function _getPassThroughVars()
	{
		$key_val_arr = Array();
		$var_rules = $this->attr('key_val_rules');
		if (empty($var_rules)) {
			return $key_val_arr;
		}
		
		//get Asset Manager
		$am =& $GLOBALS['SQ_SYSTEM']->am;
		
		//get the current answers entered by the user
		$current_answers = unserialize($_SESSION['order_extra_information']);
		
		$form =& $this->getForm();
		$questions = $form->getQuestions();
		foreach ($questions as $q_id => $question_data) {
			if (key_exists($q_id, $var_rules)) {
				$var_rule = $var_rules[$q_id];
				$key = array_get_index($var_rule, 'key', '');
				if (trim($key) == '') {
					continue;
				}
				
				$val = '';
				$question = $am->getAsset($q_id);
				if ($question instanceof Form_Question_Type_Country) {
					$val = $current_answers[$q_id];
				} else if ($question->isSelection()) {
					$options = array_get_index($var_rule, 'value', Array());
					foreach ($options as $option_id => $option_val) {
						//if option is selcted and option is not --leave empty--
						if ($question->isSelected($option_id) && (!($question instanceof Form_question_type_select) || !$question->isEmptyOption($option_id))) {
							$val = $option_val;
							break;
						}
					}
				} else {
					$key_parts = explode(':', $q_id);
					if (!empty($current_answers[$key_parts[1]])) {
						$val = $current_answers[$key_parts[1]];
					}
				}
				
				$key_val_arr[$key] = $val;
			}//end if
		}//end for
		
		return $key_val_arr;

	}//end _getPassThroughVars()
	

}//end class

?>
