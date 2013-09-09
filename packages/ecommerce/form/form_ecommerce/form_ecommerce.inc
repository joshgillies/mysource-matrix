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
* $Id: form_ecommerce.inc,v 1.62 2013/09/09 07:54:18 cupreti Exp $
*
*/

require_once SQ_PACKAGES_PATH.'/cms/form/form_email/form_email.inc';
require_once SQ_FUDGE_PATH.'/general/www.inc';
require_once SQ_FUDGE_PATH.'/general/file_system.inc';
require_once SQ_PACKAGES_PATH.'/ecommerce/orders/order/order.inc';
require_once SQ_PACKAGES_PATH.'/ecommerce/lib/ecommerce_common.inc';

/**
* Form_Ecommerce
*
* Form that can have e-commerce dollar value rules assigned to it and later
* processed using an attached payment gateway. Uses Form-email's emailing
* mechanisms. Relies on session variables for communication with the
* payment gateway asset
*
*
* @author	Andrei Railean		<arailean@squiz.net>
* @author	Dmitry Baranovskiy	<dbaranovskiy@squiz.net>
* @version $Revision: 1.62 $
* @package MySource_Matrix_Packages
* @subpackage ecommerce
*/
class Form_Ecommerce extends Form_Email
{

	/**
	* confirm view request index ($_REQUEST['view_name'])
	* @var string
	*/
	var $confirm_view = 'confirmation';

	/**
	* process view index
	* @var string
	*/
	var $process_view = 'process';

	/**
	* receipt view index
	* @var string
	*/
	var $receipt_view = 'receipt';

	/**
	* list of bodycopies
	* @var array
	*/
	var $_bodycopies = Array('Form', 'Confirmation', 'Receipt');


	/**
	* Constructor
	*
	* @param int	$assetid	the asset id to be loaded
	*
	*/
	function Form_Ecommerce($assetid=0)
	{
		$this->Form_Email($assetid);

	}//end constructor


	/**
	* Do extra processing when creating
	*
	* @param array	&$link	link
	*
	* @return boolean
	* @access private
	*/
	function _createAdditional(&$link)
	{
		return $this->_createBodycopies();

	}//end _createAdditional()


	/**
	* Creates the necessary bodycopies
	*
	* @return boolean
	* @access private
	*/
	function _createBodycopies()
	{
		$result = TRUE;
		foreach ($this->_bodycopies as $bodycopy) {
			$result = $result && $this->createBodycopy($bodycopy);
		}

		return $result;

	}//end _createBodycopies()


	/**
	* Returns an array of all the permitted link type, the type asset and
	* the cardinality
	*
	* @return array
	* @access private
	* @see Asset::_getAllowLinks()
	*/
	function _getAllowedLinks()
	{
		$order_links = parent::_getAllowedLinks();
		$order_links[SQ_LINK_TYPE_2]['folder'] = Array('card' => 'M');
		return $order_links;

	}//end _getAllowedLinks()


	/**
	* Check if question type can be used for e-commerce
	*
	* @param string	$type	question type code
	*
	* @return boolean
	* @access public
	*/
	function _isAllowedQuestionType($type='')
	{
		switch ($type) {
			case('form_question_type_numeric'):
			case('form_question_type_select'):
			case('form_question_type_option_list'):
			case('form_question_type_tickbox_list'):
			case('form_question_type_tickbox_table'):
				return TRUE;

			default:
				return FALSE;
		}

	}//end _isAllowedQuestionType()


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
	* Return question rule for e-commerce form
	*
	* @param string	$name	question type code
	*
	* @return boolean
	* @access public
	*/
	function _getEcommerceRule($name='')
	{
		// get e-commerce rules
		$e_questions = $this->attr('rules');
		$result = array_get_index($e_questions, $name, FALSE);
		return $result;

	}//end _getEcommerceRule()


	/**
	* Save one rule
	*
	* @param string	$name	rule name - question id
	* @param mixed	$value	rule contents
	*
	* @return boolean
	* @access private
	*/
	function _setEcommerceRule($name, $value)
	{
		$rules = $this->attr('rules');

		if (!isset($rules[$name])) return FALSE;

		$rules[$name] = $value;
		$this->setAttrValue('rules', $rules);

		return TRUE;

	}//end _setEcommerceRule()


	/**
	* Main controller for this asset
	*
	* This function determines what action to take next, based on the various
	* request parameters and statuses of various actions
	*
	* @return boolean
	* @access public
	*/
	function printStandard()
	{
		// an order in this case is a state of this form which was saved in a file
		if (isset($_SESSION[$this->getPrefix()]['order'])) {
			$order =& $_SESSION[$this->getPrefix()]['order'];
		} else {
			$order = $this->_readOrder();
		}

		$am =& $GLOBALS['SQ_SYSTEM']->am;

		// check to see if the form has been submitted or not
		$action = array_get_index($_REQUEST, $this->getPrefix().'_action');
		if (isset($_REQUEST[$this->getPrefix().'_submit'])) {
			if ($this->processPage(1) && empty($action)) {
				// Save Values for File Uploads and the create File asset if create location is specified
				$questions =& $this->getAllQuestionAssets();
				for (reset($questions); $question =& $questions[key($questions)]; next($questions)) {
					if($question instanceof Form_Question_Type_File_Upload) {
						$question->saveValue($this);
						$this->current_answers[$question->id]      = $question->getValue();
						$this->extra_data[$question->id]      = $question->getExtraData();

						if (method_exists($question, 'onSubmitForm')) {
							$question->onSubmitForm($this);
							$this->extra_data[$question->id]      = $question->getExtraData();
							if (!empty($question->failed_rules)) {
								if (!is_array($this->submission_errors)) $this->submission_errors = Array();
								$this->submission_errors = array_merge($this->submission_errors, $question->failed_rules);
							}
						}		
					}

				}
				if(empty($this->submission_errors))
					$action = $this->confirm_view;
			}
		} else if (!empty($order)) {
			//only get the current answers from session variables if we do not process after the submit button is clicked
			//if we put this block of codes before processPage() method is called like in the previous version, the result
			//of the answers will be wrong with tickbox list question type (it remembers the previous checked answer not the current unchecked one)
			$this->current_answers = $order['submission'];
			$this->extra_data = $order['submission_extra_data'];
			$this->repopulate();
		}
		
		$request_var_name = $this->attr('item_req_var');
		if (empty($action) && isset($_REQUEST[$request_var_name])) {
			$action = 'add_to_cart';
		}

		// determine which view to display
		switch ($action) {
			case ('add_to_cart'):
				// MAYBE: Convert to parameter map
				if ($request_var_name == '') break;

				$assetid = @$_REQUEST[$request_var_name];
				if (is_null($assetid)) break;

				$quantity = @$_REQUEST[$this->attr('item_quantity_req_var')];

				if (!$this->_addAssetItem($assetid, $quantity)) {
					trigger_error('This item cannot be purchased', E_USER_NOTICE);
					break;
				}
				
				$promo_code_var_name = $this->attr('item_promo_code_req_var');
				if (($promo_code_var_name != '') && isset($_REQUEST[$promo_code_var_name])) {
					$this->_addAssetItemPromotionalCode($assetid, $_REQUEST[$promo_code_var_name]);
				}

				$this->_makeOrder();

				header('Location: '.$this->getUrl());
				return TRUE;

			break;

			case ('empty_cart'):
				$this->_clearAssetItems();
				$this->_clearAssetItemPromotionalCodes();
				$this->_makeOrder();

				header('Location: '.$this->getUrl());
				return TRUE;

			break;


			// confirm view: after the form has been filled out it needs
			// to be processed and confirmation displayed
			case ($this->confirm_view):
				// update order with ecom options

				$this->_makeOrder();
				$this->_saveOrder();
				// we prepare payment processing stuff here instead of in 'process' view
				// because we might want to nest the payment gateway on this screen
				// so we need all data to be available to it on this view
				// side effect of this is that it makes the 'process' view unnecessary if the gateway is nested
				$order = $_SESSION[$this->getPrefix()]['order'];
				$action_name = $this->getPrefix().'_action';

				$_SESSION['SQ_ECOM_SUCCESS_URL'] = $this->getURL().'?'.$action_name.'='.$this->receipt_view;
				$_SESSION['SQ_ECOM_CANCEL_URL'] = $this->getURL().'?'.$action_name.'='.$this->confirm_view;
				$_SESSION['SQ_ECOM_REF_NO'] = md5(session_id().time());
				
				//Add the pass through variables to SESSION variables
				$pass_through_vars = $this->_getPassThroughVars();
				if (!empty($pass_through_vars)) {
					$_SESSION['SQ_ECOM_PASS_THROUGH_VARS'] = $pass_through_vars;
				}

				// if the custom reference id attribtue is not empty it means
				// that we need a customised reference id
				$new_reference_id = $this->attr('custom_reference_id');
				if (!empty($new_reference_id)) {
					$new_reference_replacements = Array();
					$new_reference_keywords = extract_keywords($new_reference_id);
					foreach ($new_reference_keywords as $keyword) {
						$new_reference_replacements[$keyword] = $this->_getThankYouKeywordReplacement($keyword);
					}

					$new_reference_replacements['session_id'] = session_id();
					$new_reference_replacements['current_time'] = time();

					replace_keywords($new_reference_id, $new_reference_replacements);
					// replace_global_keywords($new_reference_id);
					$_SESSION['SQ_ECOM_REF_NO'] = $new_reference_id;
				}

				$_SESSION['SQ_ECOM_AMOUNT'] = $order['total'];

				$this->printConfirmationView();
				return TRUE;

			break;


			case ('process'):
				// Array of configured gateway assetids for this ecomm form
				$gateways = $this->attr('gateway_assetids');
				if (empty($gateways)) {
					trigger_error('Payment Gateway not configured. Cannot proceed.', E_USER_NOTICE);
					$this->printConfirmationView();
					return TRUE;
				}

				$supplied_gateway = trim(array_get_index($_REQUEST, 'selected_gw', ''));
				if (!empty($supplied_gateway) && in_array($supplied_gateway, $gateways)) {
					$selected_gateway = $supplied_gateway;
				} else {
					// No valid gateway selected. Get the first gateway from configured list
					$selected_gateway = $gateways[0];
				}
				$gateway_asset = $GLOBALS['SQ_SYSTEM']->am->getAsset($selected_gateway);
				$gateway_asset_url = $gateway_asset->getURL();

				header('Location: '.$gateway_asset_url);
				return TRUE;

			break;


			// receipt view: after the gateway has processed payment and redirected back to us
			// here we need to display the receipt and do the logging, etc
			case ($this->receipt_view):
				// check for response from the payment gateway
				$response = array_get_index($_SESSION, 'SQ_ECOM_RESPONSE', FALSE);
				if (!$response) {
					// no response: we've either been here already - user has seen the receipt
					// or someone has used an invalid url
					// we redirect because we don't want users being able to refresh the browser
					header('Location: '.$this->getUrl());
					return TRUE;
				}
				


				unset($_SESSION['SQ_ECOM_RESPONSE']);
				$this->_tmp['GATEWAY_RESPONSE'] = $response;

				// this function is reponsible for doing business logic related to finalizing the
				// e-commerce transaction: e-mailing, logging and display
				$order = $this->_readOrder();
				$this->_deleteOrderFile();

				$this->current_answers = $order['submission'];
				$this->extra_data = $order['submission_extra_data'];
				// re-populate the active submissions
				$this->repopulate();

			
				
				// CREATE SUBMISSION
				$this->_saveOrderAsset();

				// Does two things, depending on whether a create location is set:
				// Create Location: create a NOTICE link between the File asset and the submission.
				// No Create Location: Move file upload to the order asset's data directory.
				$order_asset = $this->order;
				$file_uploads     = $this->getAllQuestionAssets('form_question_type_file_upload');

				foreach ($file_uploads as $assetid => $file_upload) {
					$extra_data = $file_upload->getExtraData();
		
					$create_location = $file_upload->attr('create_location');
					if (empty($create_location)) {
						if (!empty($extra_data['temp_filesystem_path'])) {
							$filename   = $file_upload->getValue();
							$perm_path  = $order_asset->data_path.'/attachments/'.str_replace(':', '_', $file_upload->id);
		
							// Move it using Fudge file_system function
							create_directory($perm_path);
							move_file($extra_data['temp_filesystem_path'], $perm_path.'/'.$filename);
							$extra_data = Array(
											'filesystem_path'	=> $perm_path.'/'.$filename,
										  );
		
							$file_upload->setExtraData($extra_data);
							// update order asset with uploaded file path
							$this->_saveOrderAsset();
						}
					} else {
						// create a notice link so we can keep track of uploaded file						
						$uploaded_assetid = array_get_index($extra_data, 'existing_file_assetid', array_get_index($extra_data, 'new_file_assetid', NULL));
						if (!empty($uploaded_assetid)) {
							$uploaded_file = $GLOBALS['SQ_SYSTEM']->am->getAsset($uploaded_assetid);
		
							$GLOBALS['SQ_SYSTEM']->setRunLevel(SQ_RUN_LEVEL_FORCED);
							$order_asset->createLink($uploaded_file, SQ_LINK_NOTICE, 'file_submission');
							$GLOBALS['SQ_SYSTEM']->restoreRunLevel();
						}
					}
					// clean up tmp file path since it has been either imported to order asset dir or created as a file asset
					$file_upload->cleanUp($this);
				}
		
				
		
				// log order to file
				if ($this->attr('log_to_file')) {
					$this->_logOrderToFile();
				}

				// send emails
				$this->mailReceipt();
				$this->mailRecipients();

				// perform submission actions before showing the receipt
				$this->performSubmissionActions(TRUE);
				
				$this->printReceiptView();

				// clear the cart
				$_SESSION[$this->getPrefix()]['items'] = Array();
				$_SESSION[$this->getPrefix()]['order'] = Array();
				$this->_clearAssetItems();
				$this->_clearAssetItemPromotionalCodes();

				// perform submission actions
				$this->performSubmissionActions();
				
				return TRUE;

		}//end switch


		// print the initial form
		$this->printFormView();
		return TRUE;

	}//end printStandard()


	/**
	* Log Order To File
	*
	* @return void
	* @access private
	*/
	function _logOrderToFile()
	{
		$am =& $GLOBALS['SQ_SYSTEM']->am;

		$asset_item_format = $this->attr('log_asset_item_format');
		$question_item_format = $this->attr('log_question_item_format');
		$item_block_format = $this->attr('log_item_block_format');
		$log_entry_format = $this->attr('log_entry_format');

		ob_start();

		$asset_list = @$_SESSION[$this->getPrefix()]['order']['ecom_assets'];
		if (empty($asset_list)) $asset_list = Array();
		foreach ($asset_list as $id => $item) {
			$asset = $am->getAsset($id);
			$item_string = $asset->replaceKeywordsInString($asset_item_format);
			$replacements['item_name'] = $item['name'];
			$replacements['item_price'] = $item['price'];
			$replacements['item_id'] = $item['id'];
			replace_keywords($item_string, $replacements);
			echo $item_string;
		}
		$asset_item_list = ob_get_clean();
		$item_block_replacements['asset_item_list'] = $asset_item_list;

		ob_start();
		$question_list = @$_SESSION[$this->getPrefix()]['order']['ecom_questions'];
		if (empty($question_list)) $question_list = Array();
		foreach ($question_list as $id => $item) {
			$item_string = $question_item_format;
			$replacements['item_name'] = $item['name'];
			$replacements['item_price'] = $item['price'];
			$replacements['item_id'] = $item['id'];
			replace_keywords($item_string, $replacements);
			echo $item_string;
		}
		$question_item_list = ob_get_clean();
		$item_block_replacements['question_item_list'] = $question_item_list;

		$item_block = '';
		if (!empty($question_item_list) || !empty($asset_item_list)) {
			$item_block = $item_block_format;
			replace_keywords($item_block, $item_block_replacements);
		}

		$log_entry_string = $log_entry_format;
		$log_entry_keywords = extract_keywords($log_entry_format);
		foreach ($log_entry_keywords as $keyword) {
			if ($keyword == 'form_summary') {
				$keyword2 = $keyword.'_text';
				$log_entry_replacements[$keyword] = $this->_getThankYouKeywordReplacement($keyword2);
			} else {
				$log_entry_replacements[$keyword] = $this->_getThankYouKeywordReplacement($keyword);
			}
		}

		$log_entry_replacements['item_block'] = $item_block;
		$log_entry_replacements['order_id'] = $this->getOrderNumberKeywordReplacement();
		replace_keywords($log_entry_string, $log_entry_replacements);

		replace_global_keywords($log_entry_string);

		// write to log
		$this->_log($log_entry_string);

	}//end _logOrderToFile()


	/**
	* Called by the design to print the body of this asset
	*
	* @return void
	* @access public
	*/
	function printBody()
	{
		$this->printStandard();

	}//end printBody()


	/**
	* Prints current form
	*
	* @return void
	* @access public
	*/
	function printFormView()
	{
		$prefix = $this->getPrefix();

		$clientside = ($this->attr('use_client_side')) ? 'onsubmit="return beforeSubmit_'.$prefix.'(this);"' : '';

		$form_action = current_url();

		// Add the query string as well if it exists, so asset references with ?a=###
		// submit back to themselves.
		if (!empty($_SERVER['QUERY_STRING'])) {
			$form_action .= '?'.htmlentities($_SERVER['QUERY_STRING']);
		}

		ob_start();
		if ($this->attr('use_client_side')) {
			?><script type="text/javascript">
			function beforeSubmit_<?php echo $prefix; ?>(form) {
				var submission_errors = new Array();
				i = 0;
				<?php echo $this->generateJSCode(); ?>
					if (submission_errors.length > 0) {
					var errors_list = "<?php echo translate('cms_form_submission_errors'); ?>\n";
					for(x in submission_errors) {
						errors_list += submission_errors[x] + "\n";
					}
					alert(errors_list);
					return false;
				}
				return true;
			}
			</script>
		<?php
		}
		$javascript = ob_get_clean();

		$bodycopy =& $this->getBodycopy('Form');

		if (is_null($bodycopy)) return '';

		$replacements = Array();
		$keyword_list = $bodycopy->getKeywords();		
		if (!empty($keyword_list)) {
			foreach ($keyword_list as $keyword) {
				if (isset($replacements[$keyword])) continue;
				$replacements[$keyword] = $this->getKeywordReplacement($keyword);
			}
		}

		$bodycopy->setKeywordReplacements($replacements);

		// all the output is here
		echo '<form id="'.$prefix.'" enctype="multipart/form-data" action="'.$form_action.'" method="post" '.$clientside.'>';
		echo $javascript;
		echo hidden_field('SQ_FORM_'.$this->id.'_PAGE', 1);; 
		$bodycopy->printBody();
		echo '</form>';

	}//end printFormView()


	/**
	* Keyword replacement processor
	*
	* @param string	$keyword	keyword to find a replacement for
	*
	* @return string
	* @access public
	*/
	function getKeywordReplacement($keyword)
	{
		$value = NULL;
		$unreplaced_value = '%'.$keyword.'%';

		$keyword_parts = explode('_', $keyword);
		if (empty($keyword_parts)) return $unreplaced_value;

		$keyword_category = $keyword_parts[0];

		if ($keyword_category == 'question') {

			// keywords look like this:
			// question_TYPE_BRIDGEID_qQUESTIONID
			// where TYPE = id, label, field, note

			if (count($keyword_parts) != 4) {
				return $unreplaced_value;
			}

			$question_id = $keyword_parts[2].':'.$keyword_parts[3];

			if (!$this->isAllowedQuestionId($question_id)) {
				return $unreplaced_value;
			}

			$q_asset = $GLOBALS['SQ_SYSTEM']->am->getAsset($question_id);

			switch ($keyword_parts[1]) {
				case 'id':
					$value = 'q'.$keyword_parts[2].'_'.$keyword_parts[3];
				break;

				case 'label':
					$value = $q_asset->getHtmlLabel();
				break;

				case 'field':
					$value = $q_asset->getHtmlField();
				break;

				case 'note':
					$value = $q_asset->attr('note');
				break;

				case 'response':
					$value = $q_asset->getSummary();
				break;
				
				case 'name':
					$value = $q_asset->attr('name');
				break;

				default:
					$value = $unreplaced_value;
			}

			$GLOBALS['SQ_SYSTEM']->am->forgetAsset($q_asset);

			return $value;

		} else if ($keyword_category == 'response') {
			if (count($keyword_parts) != 3) {
				return $unreplaced_value;
			}

			$question_id = $keyword_parts[1].':'.$keyword_parts[2];
			if (!$this->isAllowedQuestionId($question_id)) {
				return $unreplaced_value;
			}

			$q_asset =& $GLOBALS['SQ_SYSTEM']->am->getAsset($question_id);
			$value = $q_asset->getSummary();
			return $value;

		} else if ($keyword_category == 'section') {
			// section keyword looks like this:
			// section_title_SECTIONID

			if (count($keyword_parts) != 3) {
				return $unreplaced_value;
			}

			if ($keyword_parts[1] == 'title') {
				$section_id = $keyword_parts[2];
				$section_asset =& $GLOBALS['SQ_SYSTEM']->am->getAsset($section_id);
				$value = $section_asset->attr('name');
				$GLOBALS['SQ_SYSTEM']->am->forgetAsset($section_asset);

				return $value;
			}
		}

		return parent::getKeywordReplacement($keyword);

	}//end getKeywordReplacement()


	/**
	* Checks if a passed question ID belongs to this form
	*
	* Requires a full asset id
	*
	* @param string	$question_id	question id to check
	*
	* @return boolean
	* @access public
	*/
	function isAllowedQuestionId($question_id)
	{
		$all_questions = $this->getAllQuestionsCached();
		return isset($all_questions[$question_id]);

	}//end isAllowedQuestionId()


	/**
	* Gets a list of all questions in the form (cached)
	*
	* Caches the output of getAllQuestions in tmp vars to avoid memory problems with arrays
	*
	* @return array
	* @access public
	*/
	function &getAllQuestionsCached()
	{
		if (is_null(@$this->_tmp['all_questions'])) {
			$this->_tmp['all_questions'] = $this->getAllQuestions();
		}

		$questions =& $this->_tmp['all_questions'];

		return $questions;

	}//end getAllQuestionsCached()


	/**
	* Gets replacements for question keywords
	*
	* TODO: Move this into the local getKeywordReplacement function
	*
	* @return void
	* @access public
	*/
	function _getCommonFormKeywordReplacements()
	{
		// check to see if the bodycopy attached to this form has any content
		$questions = $this->getQuestions();
		$sections =& $this->getSections();

		foreach ($questions as $q_id => $question) {
			$q_asset =& $GLOBALS['SQ_SYSTEM']->am->getAsset($this->id.':q'.$q_id);
			$q_name = $q_asset->attr('name');
			$replacements['question_field_'.$this->id.'_q'.$q_id] = $q_asset->getHtmlField();
			$replacements['question_id_'.$this->id.'_q'.$q_id] = 'q'.$this->id.'_q'.$q_id;
			$replacements['question_note_'.$this->id.'_q'.$q_id] = $q_asset->attr('note');
			$replacements['question_label_'.$this->id.'_q'.$q_id] = $q_asset->getHtmlLabel();
		}

		foreach ($sections as $section) {
			$replacements['section_title_'.$section->id] = $section->attr('name');
			$questions = $section->getQuestions();
			foreach ($questions as $q_id => $question) {
				$q_asset =& $GLOBALS['SQ_SYSTEM']->am->getAsset($section->id.':q'.$q_id);
				$q_name = $section->attr('name').': '.$q_asset->attr('name');
				$replacements['question_field_'.$section->id.'_q'.$q_id] = $q_asset->getHtmlField();
				$replacements['question_id_'.$section->id.'_q'.$q_id] = 'q'.$section->id.'_q'.$q_id;
				$replacements['question_note_'.$section->id.'_q'.$q_id] = $q_asset->attr('note');
				$replacements['question_label_'.$section->id.'_q'.$q_id] = $q_asset->getHtmlLabel();
			}
		}

		return $replacements;

	}//end _getCommonFormKeywordReplacements()


	/**
	* Get bodycopy by name
	*
	* @param string	$bodycopy	code of bodycopy to print
	*
	* @return array
	* @access private
	*/
	function &getBodycopy($bodycopy)
	{
		if ($bodycopy == 'Page Contents') {
			$bodycopy = 'Form';
		} else if ($bodycopy == 'Thank You') {
			$bodycopy = 'Confirmation';
		}

		$bodycopy_asset =& parent::getBodycopy($bodycopy);

		return $bodycopy_asset;

	}//end getBodycopy()


	/**
	* Get keywords for Form bodycopy
	*
	* @return array
	* @access private
	*/
	function _getFormBodycopyKeywords()
	{
		return $this->_getPageContentsBodycopyKeywords();

	}//end _getFormBodycopyKeywords()


	/**
	* Get keywords for Confirmation bodycopy
	*
	* @return array
	* @access private
	*/
	function _getConfirmationBodycopyKeywords()
	{
		return $this->_getThankYouBodycopyKeywords();

	}//end _getConfirmationBodycopyKeywords()


	/**
	* Get keywords for Receipt bodycopy
	*
	* @return array
	* @access private
	*/
	function _getReceiptBodycopyKeywords()
	{
		return $this->_getThankYouBodycopyKeywords();

	}//end _getReceiptBodycopyKeywords()


	/**
	* creates a new submission object
	*
	* @return object
	* @access public
	*/
	function &createSubmission()
	{
		return ($order =& $this->createOrder());

	}//end createSubmission()


	/**
	* Makes Receipt Header
	*
	* @param boolean	$as_html	whether to get HTML copy or note
	*
	* @return string
	* @access public
	*/
	function _getReceiptHeader($as_html=TRUE)
	{
		$header = '';

		// get response from the gateway
		$response = array_get_index($this->_tmp, 'GATEWAY_RESPONSE', FALSE);
		if (!$response) return $header;

		// today
		$date = date('F j, Y, g:i a');

		if ($as_html) {
			$header ='
				<table class="ecom_header_table">
					<tr>
						<td colspan="2"><h2>Receipt</h2></td>
					</tr>
					<tr>
						<td>'.translate('ecom_form_transaction_number:').'</td>
						<td>'.$response['TRANSACTION'].'</td>
					</tr>
					<tr>
						<td>'.translate('ecom_form_transaction_status:').'</td>
						<td>'.$response['STATUS'].'</td>
					</tr>
					<tr>
						<td>'.translate('ecom_form_credit_card:').'</td>
						<td>'.$response['CARDNO'].'</td>
					</tr>
					<tr>
						<td>'.translate('ecom_form_date:').'</td>
						<td>'.$date.'</td>
					</tr>
				</table>
			';

		} else {
			$header .= translate('ecom_form_transaction_number:').' '.$response['TRANSACTION']."\n";
			$header .= translate('ecom_form_transaction_status:').' '.$response['STATUS']."\n";
			$header .= translate('ecom_form_date:').' '.$date."\n";
		}

		return $header;

	}//end _getReceiptHeader()


	/**
	* Print Summary of the submission
	*
	* Overridden parent function to include the receipt information
	*
	* @param boolean	$as_html	flag whether to print as HTML
	*
	* @return void
	* @access public
	*/
	function printSummary($as_html=FALSE)
	{
		echo $this->_getReceiptHeader($as_html);
		echo $this->_getReceipt($as_html);
		if ($as_html) {
			echo '<br /><br />';
		} else {
			echo "\n\n";
		}
		parent::printSummary($as_html);

	}//end printSummary()


	/**
	* processes the form and returns TRUE or FALSE depending on whether form processed successfully
	*
	* @return boolean
	* @access public
	*/
	function processForm()
	{
		return parent::processPage(1);

	}//end processForm()


	/**
	* Brings the submission asset up-to-date with the current state
	*
	* @return void
	* @access public
	*/
	function _saveOrderAsset()
	{
		$questions =& $this->getAllQuestionAssets();

		$order =& $this->getCurrentOrderAsset();
		if (is_null($order)) $order =& $this->createOrder();


		$this->order =& $order;
		$this->submission_asset =& $order;


		$GLOBALS['SQ_SYSTEM']->changeDatabaseConnection('db2');
		// save question values
		for (reset($questions); $question =& $questions[key($questions)]; next($questions)) {
			$question->saveValue($this);

			// If the question is sticky, stick the value then
			if ($question->attr('sticky')) {
				$question->stickValue($question->getValue());
			}
		}

		// create the submission asset
		for (reset($questions); $question =& $questions[key($questions)]; next($questions)) {
			$order->setAnswer(key($questions), $question->getValue());
			$order->setSummary(key($questions), $question->attr('name'), $question->getSummary());
			$order->setExtraData(key($questions), $question->getExtraData());
		}

		$order->setIP($_SERVER['REMOTE_ADDR']);
		$order->setAttrValue('complete', TRUE);
		$order->setAttrValue('xml', $this->getXML());
		//set ecommerce order's details
		$order->setAttrValue('transaction_id', array_get_index($this->_tmp['GATEWAY_RESPONSE'], 'TRANSACTION', ''));
		$order->setAttrValue('billing_name', array_get_index($this->_tmp['GATEWAY_RESPONSE'], 'BILLING_NAME', ''));
		$order->setAttrValue('billing_addr', array_get_index($this->_tmp['GATEWAY_RESPONSE'], 'BILLING_ADDR', ''));
		$order->setAttrValue('delivery_name', array_get_index($this->_tmp['GATEWAY_RESPONSE'], 'DELIVERY_NAME', ''));
		$order->setAttrValue('delivery_addr', array_get_index($this->_tmp['GATEWAY_RESPONSE'], 'DELIVERY_ADDR', ''));
		
		$order->saveAttributes();

		// Link the applied promotions to the asset
		$this->_linkPromotionAssets($order);		

		$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();

	}//end _saveOrderAsset()
	
	
	/**
	* Link the applied promotions to the order asset
	*
	* @return boolean
	* @access private
	*/	
	function _linkPromotionAssets($order)
	{
		// Get the applied promotions
		$promo_codes_in_cart = $this->_getAssetItemPromotionalCodes();
		$order_promo_assetids = Array();
		foreach($promo_codes_in_cart as $product_id => $promotion_assetids) {
			$order_promo_assetids = array_merge($order_promo_assetids, $promotion_assetids);
		}
		$order_promo_assetids = array_unique($order_promo_assetids);
		
		// Get the existing promotion links on the order
		$links = $GLOBALS['SQ_SYSTEM']->am->getLinks($order->id, SQ_LINK_NOTICE, 'promotional_code', TRUE, 'minor', 'order');
		$existing_promotions = Array();
		foreach($links as $link) {
			$existing_promotions[$link['majorid']] = $link['linkid'];
		}

		// Link the promotions applied to the order asset		
		$GLOBALS['SQ_SYSTEM']->setRunLevel(SQ_RUN_LEVEL_OPEN);
		$success = TRUE;
		$new_promotions = Array();
		foreach($order_promo_assetids as $promo_assetid) {
			if (isset($existing_promotions[$promo_assetid])) {
				$new_promotions[$promo_assetid] = $existing_promotions[$promo_assetid];
				continue;
			}
			$promo_asset = $GLOBALS['SQ_SYSTEM']->am->getAsset($promo_assetid);
			$linkid = $GLOBALS['SQ_SYSTEM']->am->createAssetLink($promo_asset, $order, SQ_LINK_NOTICE, 'order');
			if (!$linkid) {
				$success = FALSE;
			}

			$GLOBALS['SQ_SYSTEM']->am->forgetAsset($promo_asset);
			if (!$success) {
				break;
			}

			$new_promotions[$promo_assetid] = $linkid;
		}//end foreach

		// Delete the promotion links that are not applied anymore
		if ($success) {
			foreach($existing_promotions as $promo_assetid => $linkid) {
				if (!isset($new_promotions[$promo_assetid])) {
					$success = $GLOBALS['SQ_SYSTEM']->am->deleteAssetLink($linkid);
					if (!$success) {
						break;
					}
				}//end if
			}// end foreach
		}//end if

		$GLOBALS['SQ_SYSTEM']->restoreRunLevel();
	
	}//end _linkPromotionAssets()


	/**
	* creates a new Order object
	*
	* @return object
	* @access public
	*/
	function &createOrder()
	{
		$am =& $GLOBALS['SQ_SYSTEM']->am;

		$GLOBALS['SQ_SYSTEM']->changeDatabaseConnection('db2');
		$GLOBALS['SQ_SYSTEM']->doTransaction('BEGIN');

		// search for a bodycopies folder and create one if there isn't one there
		$folder_link = $GLOBALS['SQ_SYSTEM']->am->getLink($this->id, SQ_LINK_TYPE_2, 'folder', TRUE, 'orders_folder');
		if (empty($folder_link)) {

			// create a folder for the bodycopies
			$GLOBALS['SQ_SYSTEM']->am->includeAsset('folder');
			$folder = new Folder();
			$folder->setAttrValue('name', 'Orders');

			$folder_link = Array('asset' => &$this, 'link_type' => SQ_LINK_TYPE_2, 'value' => 'orders_folder', 'is_dependant' => 0, 'is_exclusive' => 1);

		 	// Create order folder
            $GLOBALS['SQ_SYSTEM']->setRunLevel(SQ_RUN_LEVEL_FORCED);
            $success = $folder->create($folder_link);
            $GLOBALS['SQ_SYSTEM']->restoreRunLevel();
            if (!$success) {
				$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
				$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();
				return FALSE;
            }
			
			// Deny public read access to the order folder by default
			$GLOBALS['SQ_SYSTEM']->setRunLevel(SQ_RUN_LEVEL_FORCED);
			$success = $GLOBALS['SQ_SYSTEM']->am->setPermission($folder->id, $GLOBALS['SQ_SYSTEM']->am->getSystemAssetid('public_user'), SQ_PERMISSION_READ, 0, TRUE, TRUE);
			$GLOBALS['SQ_SYSTEM']->restoreRunLevel();
			if (!$success) {
				$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
				$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();
				return FALSE;
			}
	
		} else {
			// folder already exists, get the reference
			$folder = $GLOBALS['SQ_SYSTEM']->am->getAsset($folder_link['minorid'], $folder_link['minor_type_code']);
		}


		if (is_null($folder)) {
			$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
			$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();
			return FALSE;
		}

		// create the Order asset
		$order = new Order();
		$copy_link  = Array('asset' => &$folder, 'link_type' => SQ_LINK_TYPE_2, 'is_dependant' => 1, 'is_exclusive' => 1);

		$GLOBALS['SQ_SYSTEM']->setRunLevel(SQ_RUN_LEVEL_FORCED);
		$success = $order->create($copy_link);
		$GLOBALS['SQ_SYSTEM']->restoreRunLevel();

		// attempt to create the link to this asset
		if (!$success) {
			$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
			$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();
			return FALSE;
		}
		$GLOBALS['SQ_SYSTEM']->doTransaction('COMMIT');
		$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();

		return $order;

	}//end createOrder()


	/**
	* Format reciept values into table for nice output
	*
	* @param boolean	$as_html	whether to get HTML copy or note
	*
	* @return string
	* @access private
	*/
	function _getReceipt($as_html=TRUE)
	{
		$order = $_SESSION[$this->getPrefix()]['order'];
		if (empty($order)) return FALSE;
		
		$curr_sign = $this->attr('currency_sign');
		if ($as_html) {
			$receipt_fmt   = '<hr /><table class="sq_ecomTable">%s%s%s</table><hr />';
			$order_row_fmt = '<tr class="sq_ecomTableItemRow"><td class="sq_ecomTableItemName">%s</td><td class="sq_ecomTableItemValue" align="right">%01.2f</td></tr>';
			$divider_row = '<tr class="sq_ecomTableItemRow"><td class="sq_ecomTableItemName">&nbsp;</td><td class="sq_ecomTableItemValue" align="right"><hr /></td></tr>';
			$total_discount_row_fmt = '<tr class="sq_ecomTableTotalDiscountRow"><td class="sq_ecomTableTotalDiscountName" align="right">%s</td><td class="sq_ecomTableTotalDiscountValue" align="right">'.$curr_sign.'%01.2f</td></tr>';
			$total_row_fmt = '<tr class="sq_ecomTableTotalRow"><td class="sq_ecomTableTotalName" align="right">%s</td><td class="sq_ecomTableTotalValue" align="right">'.$curr_sign.'%01.2f</td></tr>';
		} else {
			$receipt_fmt   = strtoupper(translate('ecom_form_receipt:'))."\n############################\n%s\n%s\n%s\n############################";
			$order_row_fmt = "%s: %01.2f\n";
			$divider_row = "----------------------------\n";
			$total_discount_row_fmt = "*** %s $curr_sign%01.2f ***\n";
			$total_row_fmt = "*** %s $curr_sign%01.2f ***\n";
		}

		$rows = '';
		foreach ($order['items'] as $item) {
			$rows .= sprintf($order_row_fmt, $item['name'], $item['price']);
		}

		$rows .= $divider_row;
		
		$total_discount = '';
		if ($order['total_discount'] > 0) {
			$total_discount = sprintf($total_discount_row_fmt, translate('ecom_form_total_discount:'), $order['total_discount']);
			$total_discount .= $divider_row;
		}

		$total   = sprintf($total_row_fmt, translate('ecom_form_total:'), $order['total']);
		$receipt = sprintf($receipt_fmt, $rows, $total_discount, $total);

		return $receipt;

	}//end _getReceipt()


	/**
	* Generates order array, which contains all necessary information for payment processing
	*
	* @return array
	* @access private
	*/
	function _makeOrder()
	{
		$total = 0;
		$taxable_total = 0;
		$total_discount = 0;

		$questions =& $this->getAllQuestionAssets();
		$ecom_items = Array();
		$taxable_items = Array();
		$ecom_assets = Array();
		$ecom_questions = Array();

		// process the assets being purchased
		$am =& $GLOBALS['SQ_SYSTEM']->am;
		$assets_in_cart = $this->_getAssetItems();
		$promo_codes_in_cart = $this->_getAssetItemPromotionalCodes();
		foreach ($assets_in_cart as $asset_id => $count) {
			$ecom_item['taxable'] = FALSE;
			$asset = $am->getAsset($asset_id);
			$name = $this->_getPurchasableAssetName($asset);
			$price = $this->_getPurchasableAssetPrice($asset);
			
			// process promotional codes
			$discount = 0;
			if (isset($promo_codes_in_cart[$asset_id])) {
				$discount = Ecommerce_Common::getPromotionalDiscount($promo_codes_in_cart[$asset_id], $price, $count);
			}
			
			if ($count > 1) {
				$name = $count.' x '.$name.' @'.$price;
				$price = $price * $count;
			}

			$ecom_item['name'] = $name;
			$ecom_item['quantity'] = $count;
			$ecom_item['price'] = $price;
			$ecom_item['discount'] = $discount;
			$ecom_item['id'] = $asset_id;

			$is_taxable = $this->_isPurchasableAssetTaxable($asset);

			if ($is_taxable) {
				$taxable_items[$asset_id] = $ecom_item;
				$taxable_total += $ecom_item['price'];
				$ecom_item['taxable'] = TRUE;
			}
			$ecom_assets[$asset_id] = $ecom_item;
			$ecom_items[$asset_id] = $ecom_item;
			$total += $ecom_item['price'];
			$total_discount += $ecom_item['discount'];
			$am->forgetAsset($asset);
		}

		// now process the ecommerce questions
		$taxable_rules = $this->attr('taxable_rules');

		foreach ($questions as $q_id => $question) {
			if (($rule = $this->_getEcommerceRule($q_id)) !== FALSE) {
				$ecom_item['taxable'] = FALSE;
				$question_title = $question->attr('name');
				// if it is a selection question, we ask the question to
				// tell us if an option was selected

				// Reset the var here because we are going to overwrite them anyway
				$ecom_item['name']  = '';
				$ecom_item['price'] = '';
				if ($question->isSelection()) {
					foreach ($rule as $option_id => $price) {
						//if option is selcted and option is not --leave empty--
						if ($question->isSelected($option_id) && (!($question instanceof Form_question_type_select) || !$question->isEmptyOption($option_id))) {
							if (isset($ecom_item['name']) && !empty($ecom_item['name'])) {
								$ecom_item['name']	.= ', '.$question->getOptionName($option_id);
							} else {
								$ecom_item['name']	= $question_title.': '.$question->getOptionName($option_id);
							}//end if else
							if (!isset($ecom_item['price']) && !empty($ecom_item['price'])) {
								$ecom_item['price'] = $price;
							} else {
								$ecom_item['price'] += $price;
							}//end if

							$ecom_item['id'] = $q_id;

							if (array_get_index($taxable_rules, $q_id, FALSE)) {
								$taxable_items[$q_id] = $ecom_item;
								$taxable_total += $price;
								$ecom_item['taxable'] = TRUE;
							}
							$ecom_items[$q_id] = $ecom_item;
							$ecom_questions[$q_id] = $ecom_item;
							$total += $price;
						}
					}

				} else {
					// if it is not a question with options, we get the submission directly
					// we assume that it is a numeric question
					$ecom_item['name']  = $question_title;
					$ecom_item['price'] = $this->current_answers[$q_id];
					$ecom_item['id'] = $q_id;

					if (array_get_index($taxable_rules, $q_id, FALSE)) {
						$taxable_items[$q_id] = $ecom_item;
						$taxable_total += $ecom_item['price'];
						$ecom_item['taxable'] = TRUE;
					}
					$ecom_items[$q_id] = $ecom_item;
					$ecom_questions[$q_id] = $ecom_item;
					$total += $ecom_item['price'];
				}//end if

			}//end if

		}//end foreach

		$order['items'] = $ecom_items;
		$order['ecom_assets'] = $ecom_assets;
		$order['ecom_questions'] = $ecom_questions;
		$order['taxable_items'] = $taxable_items;
		$order['taxable_total'] = $taxable_total;
		$order['total'] = $total - $total_discount;
		$order['total_discount'] = $total_discount;
		$order['submission'] = $this->current_answers;
		$order['submission_extra_data'] = $this->extra_data;

		$_SESSION[$this->getPrefix()]['order'] = $order;

		return TRUE;

	}//end _makeOrder()


	/**
	* Save order into file, to prevent losing information in case of session dying
	*
	* @return boolean
	* @access private
	*/
	function _saveOrder()
	{
		$filename = $this->_getOrderFileName();
		$order = $_SESSION[$this->getPrefix()]['order'];
		$order_contents = serialize($order);
		return string_to_file($order_contents, $filename);

	}//end _saveOrder()


	/**
	* Read order from the file
	*
	* @return array
	* @access private
	*/
	function _readOrder()
	{
		$filename = $this->_getOrderFileName();
		$order = file_to_string($filename);
		if (!$order) return FALSE;
		$_SESSION[$this->getPrefix()]['order'] = unserialize($order);
		return $_SESSION[$this->getPrefix()]['order'];

	}//end _readOrder()


	/**
	* Delete file with order
	*
	* @return boolean
	* @access private
	* @see _readOrder()
	*/
	function _deleteOrderFile()
	{
		$filename = $this->_getOrderFileName();
		unlink($filename);
		return TRUE;

	}//end _deleteOrderFile()


	/**
	* Return file name for order file
	*
	* @param string	$order_no	id of the saved file
	*
	* @return string
	* @access private
	* @see _saveOrder()
	* @see _readOrder()
	* @see _deleteOrderFile()
	*/
	function _getOrderFileName($order_no=FALSE)
	{
		$order_path = $this->data_path.'/tmp_orders';

		if (!is_dir($order_path)) {
			mkdir($order_path);
			chmod($order_path, 0700);
		}

		if (!$order_no) $order_no = session_id();

		return $order_path.'/'.$order_no.'.tmp';

	}//end _getOrderFileName()


	/**
	* Print Confirmation View
	*
	* @return void
	* @access public
	*/
	function printConfirmationView()
	{
		$body =& $this->getBodycopy('Confirmation');

		$keyword_list = $body->getKeywords();
		$replacements = Array();
		foreach ($keyword_list as $keyword) {
			$replacements[$keyword] = $this->_getThankYouKeywordReplacement($keyword == 'form_summary' ? 'form_summary_text' : $keyword);
		}

		$body->setKeywordReplacements($replacements);
		$body->printBody();

	}//end printConfirmationView()


	/**
	* Print receipt view
	*
	* @return void
	* @access private
	*/
	function printReceiptView()
	{
		$body = $this->getBodycopy('Receipt');
		$replacements = Array();

		$keyword_list = $body->getKeywords();
		foreach ($keyword_list as $keyword) {
			$replacements[$keyword] = $this->_getThankYouKeywordReplacement($keyword == 'form_summary' ? 'form_summary_text' : $keyword);
		}

		$body->setKeywordReplacements($replacements);
		$body->printBody();

	}//end printReceiptView()


	/**
	* Get parent page asset id
	*
	* @return string
	* @access private
	*/
	function _getParentPageAssetId()
	{
		$parent_page_assetid = array_get_index($this->_tmp, 'parent_form_page_assetid');

		if (is_null($parent_page_assetid)) {
			$parents = array_keys($GLOBALS['SQ_SYSTEM']->am->getParents($this->id, 'page_custom_form_ecommerce', FALSE));
			$parent_page_assetid = array_pop($parents);
			$this->_tmp['parent_form_page_assetid'] =& $parent_page_assetid;
		}

		return $parent_page_assetid;

	}//end _getParentPageAssetId()


	/**
	* Get parent page asset
	*
	* @return object
	* @access private
	*/
	function &_getParentPageAsset()
	{
		if (!isset($this->_tmp['parent_form_page_asset'])) {
			$parent_page = $GLOBALS['SQ_SYSTEM']->am->getAsset($this->_getParentPageAssetId());
			$this->_tmp['parent_form_page_asset'] =& $parent_page;
		} else {
			$parent_page =& $this->_tmp['parent_form_page_asset'];
		}

		return $parent_page;

	}//end _getParentPageAsset()


	/**
	* Get URL
	*
	* Returns url to the parent page asset
	*
	* @return string
	* @access private
	*/
	function getUrl()
	{
		$url = array_get_index($this->_tmp, 'current_url');

		if (is_null($url)) {
			$parent =& $this->_getParentPageAsset();
			$url = $parent->getURL();

			$this->_tmp['current_url'] = $url;
		}

		return $url;

	}//end getUrl()


//--        KEYWORDS AVAILABLE        --//


	/**
	* Get keywords for thank you bodycopy
	*
	* @return array
	* @access private
	*/
	function _getThankYouBodycopyKeywords()
	{
		$keywords = parent::_getThankYouBodycopyKeywords();

		$keywords['confirm_text'] = 'Confirm and pay button text';
		$keywords['cancel_text'] = 'Text Used on "Cancel" button';

		$keywords['confirm_url'] = 'URL to Confirmation Screen';
		$keywords['cancel_url'] = 'URL to Cancel Screen';

		$keywords['order_item_count'] = 'Count of items being purchased';
		$keywords['order_total'] = 'Total';
		$keywords['order_total_excluding_tax'] = 'Total minus tax';
		$keywords['order_total_tax'] = 'Total tax';
		$keywords['order_taxable_total'] = 'Taxable total';
		$keywords['order_taxable_item_count'] = 'Count of taxable items being purchased';
		$keywords['order_item_list'] = 'List of items being purchased';
		$keywords['order_total_discount'] = 'Total discount';
		$keywords['order_total_before_discount'] = 'Total before discount';

		$keywords['tax_rate'] = 'Tax Rate';
		$keywords['tax_name'] = 'Tax Name';

		$keywords['taxable_item_string'] = 'String appended to the taxable items';

		return $keywords;

	}//end _getThankYouBodycopyKeywords()


//--        KEYWORD REPLACEMENT FUNCTIONS        --//


	/**
	* Confirm Text keyword replacement
	*
	* @return string
	* @access private
	*/
	function getConfirmTextKeywordReplacement()
	{
		return $this->attr('confirm_button_name');

	}//end getConfirmTextKeywordReplacement()


	/**
	* Cancel Text keyword replacement
	*
	* @return string
	* @access private
	*/
	function getCancelTextKeywordReplacement()
	{
		return $this->attr('cancel_button_name');

	}//end getCancelTextKeywordReplacement()


	/**
	* Confirm URL keyword replacement
	*
	* @return string
	* @access private
	*/
	function getConfirmUrlKeywordReplacement()
	{
		$confrim_url = $this->getUrl();
		$confrim_url .= '?'.$this->getPrefix().'_action=process';

		return $confrim_url;

	}//end getConfirmUrlKeywordReplacement()


	/**
	* Cancel URL keyword replacement
	*
	* @return string
	* @access private
	*/
	function getCancelUrlKeywordReplacement()
	{
		$cancel_url = $this->getUrl();

		return $cancel_url;

	}//end getCancelUrlKeywordReplacement()


	/**
	* Order Item Count keyword replacement
	*
	* @return string
	* @access private
	*/
	function getOrderItemCountKeywordReplacement()
	{
		$item_count = 0;

		if (isset($_SESSION[$this->getPrefix()]['order']['items'])) {
			$item_count = count($_SESSION[$this->getPrefix()]['order']['items']);
		}

		return $item_count;

	}//end getOrderItemCountKeywordReplacement()


	/**
	* Order Total keyword replacement
	*
	* @return string
	* @access private
	*/
	function getOrderTotalKeywordReplacement()
	{
		$total = $this->_getTotal();
		$total = $this->_formatPriceAmount($total);

		return $total;

	}//end getOrderTotalKeywordReplacement()
	
	
	/**
	* Order Total Discount keyword replacement
	*
	* @return string
	* @access private
	*/
	function getOrderTotalDiscountKeywordReplacement()
	{
		$total_discount = $this->_getTotalDiscount();
		$total_discount = $this->_formatPriceAmount($total_discount);

		return $total_discount;

	}//end getOrderTotalDiscountKeywordReplacement()
	
	
	/**
	* Order Total Before Discount keyword replacement
	*
	* @return string
	* @access private
	*/
	function getOrderTotalBeforeDiscountKeywordReplacement()
	{
		$total = $this->_getTotal();
		$total_discount = $this->_getTotalDiscount();
		$total_before_discount = $total + $total_discount;
		$total_before_discount = $this->_formatPriceAmount($total_before_discount);

		return $total_before_discount;

	}//end getOrderTotalBeforeDiscountKeywordReplacement()
	


	/**
	* Order Total Excluding Tax keyword replacement
	*
	* @return string
	* @access private
	*/
	function getOrderTotalExcludingTaxKeywordReplacement()
	{
		$total = $this->_getTotal();

		$tax = $this->_getTaxTotal();
		$total_no_tax = $total - $tax;
		$total_no_tax = $this->_formatPriceAmount($total_no_tax);

		return $total_no_tax;

	}//end getOrderTotalExcludingTaxKeywordReplacement()


	/**
	* Order Total Tax keyword replacement
	*
	* @return string
	* @access private
	*/
	function getOrderTotalTaxKeywordReplacement()
	{
		$total_tax = $this->_formatPriceAmount($this->_getTaxTotal());

		return $total_tax;

	}//end getOrderTotalTaxKeywordReplacement()


	/**
	* Order Taxable Total keyword replacement
	*
	* @return string
	* @access private
	*/
	function getOrderTaxableTotalKeywordReplacement()
	{
		$taxable_total = $this->_formatPriceAmount($this->_getTaxableTotal());

		return $taxable_total;

	}//end getOrderTaxableTotalKeywordReplacement()


	/**
	* Order Taxable Item Count keyword replacement
	*
	* @return string
	* @access private
	*/
	function getOrderTaxableItemCountKeywordReplacement()
	{
		$taxable_count = 0;

		if (isset($_SESSION[$this->getPrefix()]['order']['taxable_items'])) {
			$taxable_count = count($_SESSION[$this->getPrefix()]['order']['taxable_items']);
		}
		return $taxable_count;

	}//end getOrderTaxableItemCountKeywordReplacement()


	/**
	* Tax Rate keyword replacement
	*
	* @return string
	* @access private
	*/
	function getTaxRateKeywordReplacement()
	{
		return $this->_getTaxRate();

	}//end getTaxRateKeywordReplacement()


	/**
	* Tax Name keyword replacement
	*
	* @return string
	* @access private
	*/
	function getTaxNameKeywordReplacement()
	{
		return $this->attr('tax_name');

	}//end getTaxNameKeywordReplacement()


	/**
	* Order Item List keyword replacement
	*
	* @return string
	* @access private
	*/
	function getOrderItemListKeywordReplacement()
	{
		$item_format = $this->attr('item_format');
		$keywords = extract_keywords($item_format);

		ob_start();
		foreach ($this->_getOrderItems() as $item) {
			$item_line = $item_format;

			$item_name = array_get_index($item, 'name', '--UNKNOWN ITEM NAME--');
			$item_price = array_get_index($item, 'price', 0);
			$item_id = array_get_index($item, 'id');
			$is_taxable = array_get_index($item, 'taxable', FALSE);

			$replacements['item_name'] = $item_name;
			$replacements['item_price'] = $this->_formatPriceAmount($item_price);
			$replacements['currency_sign'] = $this->attr('currency_sign');
			$replacements['item_taxable_string'] = '';
			if ($is_taxable) {
				$replacements['taxable_item_string'] = $this->attr('item_taxable_string');
			}
			replace_keywords($item_line, $replacements);

			echo $item_line;
		}

		return ob_get_clean();

	}//end getOrderItemListKeywordReplacement()


	/**
	* Order Purchasable Asset List keyword replacement
	*
	* @return string
	* @access private
	*/
	function getOrderPurchasableAssetListKeywordReplacement()
	{
		$am =& $GLOBALS['SQ_SYSTEM']->am;
		$assets_in_cart = $this->_getAssetItems();

		$item_format = $this->attr('form_asset_format');

		ob_start();
		foreach ($assets_in_cart as $asset_id => $count) {
			$asset =& $am->getAsset($asset_id);

			$price = $this->_getPurchasableAssetPrice($asset);
			$name = $this->_getPurchasableAssetName($asset);
			$is_taxable = $this->_isPurchasableAssetTaxable($asset);

			if ($count > 1) {
				$name = $count.' x '.$name.' @'.$price;
				$price = $price * $count;
			}

			$replacements['item_name'] = $name;
			$replacements['item_price'] = $this->_formatPriceAmount($price);
			$replacements['currency_sign'] = $this->attr('currency_sign');
			$replacements['item_taxable_string'] = '';
			if ($is_taxable) {
				$replacements['taxable_item_string'] = $this->attr('item_taxable_string');
			}

			// ask asset to replace any known keywords
			$item_line = $asset->replaceKeywordsInString($item_format);
			// replace those new keywords the asset doesn't know about
			replace_keywords($item_line, $replacements);

			echo $item_line;
		}

		return ob_get_clean();

	}//end getOrderPurchasableAssetListKeywordReplacement()


	/**
	* Item List keyword replacement
	*
	* @return string
	* @access private
	*/
	function getItemListingKeywordReplacement()
	{
		$item_printout  = $this->getOrderPurchasableAssetListKeywordReplacement();
		if (empty($item_printout)) {
			return $this->attr('form_assets_empty_str');
		}

		$form_assets_format = $this->attr('form_asset_list_format');
		$keywords['item_format'] = $item_printout;
		replace_keywords($form_assets_format, $keywords);

		return $form_assets_format;

	}//end getItemListingKeywordReplacement()


	/**
	* Taxable Item String keyword replacement
	*
	* @return string
	* @access private
	*/
	function getTaxableItemStringKeywordReplacement()
	{
		return $this->attr('item_taxable_string');

	}//end getTaxableItemStringKeywordReplacement()
	
	
	/**
	* Currency Sign keyword replacement
	*
	* @return string
	* @access private
	*/
	function getCurrencySignKeywordReplacement()
	{
		return $this->attr('currency_sign');

	}//end getTaxableItemStringKeywordReplacement()


	/**
	* Transaction Number keyword replacement
	*
	* @return string
	* @access private
	*/
	function getTransactionNumberKeywordReplacement()
	{
		return array_get_index($this->_tmp['GATEWAY_RESPONSE'], 'TRANSACTION', '');

	}//end getTransactionNumberKeywordReplacement()


	/**
	* Transaction Status keyword replacement
	*
	* @return string
	* @access private
	*/
	function getTransactionStatusKeywordReplacement()
	{
		return array_get_index($this->_tmp['GATEWAY_RESPONSE'], 'STATUS', '');

	}//end getTransactionStatusKeywordReplacement()


	/**
	* Transaction Type keyword replacement
	*
	* @return string
	* @access private
	*/
	function getTransactionTypeKeywordReplacement()
	{
		return array_get_index($this->_tmp['GATEWAY_RESPONSE'], 'TYPE', '');

	}//end getTransactionTypeKeywordReplacement()


	/**
	* Transaction Time keyword replacement
	*
	* @return string
	* @access private
	*/
	function getTransactionTimeKeywordReplacement()
	{
		return array_get_index($this->_tmp['GATEWAY_RESPONSE'], 'TIME', '');

	}//end getTransactionTimeKeywordReplacement()


	/**
	* Transaction Amount keyword replacement
	*
	* @return string
	* @access private
	*/
	function getTransactionAmountKeywordReplacement()
	{
		$amount = array_get_index($this->_tmp['GATEWAY_RESPONSE'], 'AMOUNT', '');
		return $this->_formatPriceAmount($amount);

	}//end getTransactionAmountKeywordReplacement()


	/**
	* Transaction Card Number keyword replacement
	*
	* @return string
	* @access private
	*/
	function getTransactionCardNumberKeywordReplacement()
	{
		return array_get_index($this->_tmp['GATEWAY_RESPONSE'], 'CARDNO', '');

	}//end getTransactionCardNumberKeywordReplacement()
	
	
	/**
	* Transaction Billing Name keyword replacement
	*
	* @return string
	* @access private
	*/
	function getTransactionBillingNameKeywordReplacement()
	{
		return array_get_index($this->_tmp['GATEWAY_RESPONSE'], 'BILLING_NAME', '');

	}//end getTransactionBillingNameKeywordReplacement()
	
	
	/**
	* Transaction Billing Address keyword replacement
	*
	* @return string
	* @access private
	*/
	function getTransactionBillingAddressKeywordReplacement()
	{
		return array_get_index($this->_tmp['GATEWAY_RESPONSE'], 'BILLING_ADDR', '');

	}//end getTransactionBillingAddressKeywordReplacement()
	
	
	/**
	* Transaction Delivery Name keyword replacement
	*
	* @return string
	* @access private
	*/
	function getTransactionDeliveryNameKeywordReplacement()
	{
		return array_get_index($this->_tmp['GATEWAY_RESPONSE'], 'DELIVERY_NAME', '');

	}//end getTransactionDeliveryNameKeywordReplacement()
	
	
	/**
	* Transaction Delivery Address keyword replacement
	*
	* @return string
	* @access private
	*/
	function getTransactionDeliveryAddressKeywordReplacement()
	{
		return array_get_index($this->_tmp['GATEWAY_RESPONSE'], 'DELIVERY_ADDR', '');

	}//end getTransactionDeliveryAddressKeywordReplacement()


	/**
	* Order Number keyword replacement
	*
	* @return string
	* @access private
	*/
	function getOrderNumberKeywordReplacement()
	{
		$order = $this->getCurrentOrderAsset();
		if (is_null($order)) return;

		return $order->id;

	}//end getOrderNumberKeywordReplacement()


	/**
	* Get the order asset
	*
	* @return object
	* @access private
	*/
	function &getCurrentOrderAsset()
	{
		if (!empty($this->order)) {
			$order =& $this->order;
			return $order;
		}

		$order_id = @$_SESSION[$this->getPrefix()]['order_id'];
		if (empty($order_id)) {
			$order = NULL;
			return $order;
		}

		$this->order =& $GLOBALS['SQ_SYSTEM']->am->getAsset($order_id);
		$order =& $this->order;
		return $order;

	}//end getCurrentOrderAsset()


	/**
	* Get the list of assets on order
	*
	* @return string
	* @access private
	*/
	function _getAssetItems()
	{
		$assets_in_cart = @$_SESSION[$this->getPrefix()]['assets'];
		if (is_null($assets_in_cart)) {
			$assets_in_cart = Array();
		}
		return $assets_in_cart;

	}//end _getAssetItems()


	/**
	* Clear the list of assets on order
	*
	* @return string
	* @access private
	*/
	function _clearAssetItems()
	{
		$_SESSION[$this->getPrefix()]['assets'] = Array();

	}//end _clearAssetItems()


	/**
	* Adds an asset item to order
	*
	* @param string	$assetid	Asset ID to add
	* @param int	$quantity	Quantity of asset to add
	*
	* @return string
	* @access private
	*/
	function _addAssetItem($assetid, $quantity=1)
	{
		$asset = $GLOBALS['SQ_SYSTEM']->am->getAsset($assetid);
		if (is_null($asset)) return FALSE;

		if (!$this->_isPurchasableAsset($asset)) {
			$GLOBALS['SQ_SYSTEM']->am->forgetAsset($asset);
			return FALSE;
		}

		// can only buy one of each item
		if ($this->_isPurchasableAssetQuantityAllowed($asset)) {
			if (!is_numeric($quantity) || $quantity <= 0) {
				$quantity = 1;
			}
		} else {
			$quantity = 1;
		}

		$_SESSION[$this->getPrefix()]['assets'] = Array();
		$_SESSION[$this->getPrefix()]['assets'][$assetid] = $quantity;

		$GLOBALS['SQ_SYSTEM']->am->forgetAsset($asset);

		return TRUE;

	}//end _addAssetItem()
	
	
	/**
	* Get the list of promotional code assets on order
	*
	* @return array
	* @access private
	*/
	function _getAssetItemPromotionalCodes()
	{
		$promo_codes_in_cart = @$_SESSION[$this->getPrefix()]['promo_code_assets'];
		if (is_null($promo_codes_in_cart)) {
			$promo_codes_in_cart = Array();
		}
		
		return $promo_codes_in_cart;
		
	}//end _getAssetItemPromotionalCodes()


	/**
	* Clear the list of promotional code assets on order
	*
	* @return void
	* @access private
	*/
	function _clearAssetItemPromotionalCodes()
	{
		$_SESSION[$this->getPrefix()]['promo_code_assets'] = Array();
		
	}//end _clearAssetItemPromotionalCodes()
	
	
	/**
	* Adds a promotional code asset ID to a purchasable asset
	*
	* @param string	$assetid	Asset ID of the purchasable asset to add
	* @param string	$promo_code	The code of the promotional code asset
	*
	* @return void
	* @access private
	*/
	function _addAssetItemPromotionalCode($assetid, $promo_code)
	{
		if (trim($promo_code) == '') return;
		
		$asset = $GLOBALS['SQ_SYSTEM']->am->getAsset($assetid);
		if (is_null($asset)) return;

		if (!$this->_isPurchasableAssetPromotable($asset)) {
			$GLOBALS['SQ_SYSTEM']->am->forgetAsset($asset);
			return;
		}
		
		$promo_code_assets = Ecommerce_Common::getPromotionalCodeAssetIds($asset->id, $promo_code);
		
		if (!empty($promo_code_assets)) {
			// Reset the promo code asset list because _addAssetItem() method also reset the asset list
			// A new array 'promo_code_assets' is created instead of using the 'assets' array
			// because we might want to add the promotional code for the whole ecommerce form later (the whole cart, not only individual items)
			$_SESSION[$this->getPrefix()]['promo_code_assets'] = Array();
			$_SESSION[$this->getPrefix()]['promo_code_assets'][$assetid] = $promo_code_assets;
		}
		
		$GLOBALS['SQ_SYSTEM']->am->forgetAsset($asset);
		
	}//end _addAssetItemPromotionalCode()
	
	
	/**
	* Get the total in current order
	*
	* @return string
	* @access private
	*/
	function _getTotal()
	{
		$total = 0;

		if (isset($_SESSION[$this->getPrefix()]['order']['total'])) {
			$total = $_SESSION[$this->getPrefix()]['order']['total'];
		}
		return $total;

	}//end _getTotal()
	
	
	/**
	* Get the total discount in current order
	*
	* @return string
	* @access private
	*/
	function _getTotalDiscount()
	{
		$total_discount = 0;
		
		if (isset($_SESSION[$this->getPrefix()]['order']['total_discount'])) {
			$total_discount = $_SESSION[$this->getPrefix()]['order']['total_discount'];
		}
		
		return $total_discount;

	}//end _getTotalDiscount()


	/**
	* Get taxable total
	*
	* Part of total that inicludes tax
	*
	* @return string
	* @access private
	*/
	function _getTaxableTotal()
	{
		$taxable_total = 0;

		if (isset($_SESSION[$this->getPrefix()]['order']['taxable_total'])) {
			$taxable_total = $_SESSION[$this->getPrefix()]['order']['taxable_total'];
		}
		return $taxable_total;

	}//end _getTaxableTotal()


	/**
	* Get the current tax rate
	*
	* @return string
	* @access private
	*/
	function _getTaxRate()
	{
		return $this->attr('tax_value');

	}//end _getTaxRate()


	/**
	* Get the total tax amount in current state
	*
	* @return string
	* @access private
	*/
	function _getTaxTotal()
	{
		$taxable_total = $this->_getTaxableTotal();

		$tax_rate = $this->_getTaxRate();
		$total_tax = $taxable_total * $tax_rate/(100 + $tax_rate);

		return $total_tax;

	}//end _getTaxTotal()


	/**
	* Get the itesm being ordered
	*
	* @return array
	* @access private
	*/
	function _getOrderItems()
	{

		$items = @$_SESSION[$this->getPrefix()]['order']['items'];
		if (is_null($items) || !is_array($items)) {
			$items = Array();
		}

		return $items;

	}//end _getOrderItems()


	/**
	* Get the price of purchasable asset
	*
	* @param object	&$asset	asset to check
	*
	* @return string
	* @access private
	*/
	function _getPurchasableAssetPrice(&$asset)
	{
		$price_string = $this->attr('asset_price_source');
		$price = $asset->replaceKeywordsInString($price_string);

		if (!is_numeric($price)) $price = 0;

		return $price;

	}//end _getPurchasableAssetPrice()


	/**
	* Get the name of purchasable asset
	*
	* @param object	&$asset	asset to check
	*
	* @return string
	* @access private
	*/
	function _getPurchasableAssetName(&$asset)
	{
		$name_string = $this->attr('asset_name_source');
		$name = $asset->replaceKeywordsInString($name_string);

		if ($name == '') {
			$name = $asset->replaceKeywordsInString('%asset_name%');
		}

		return $name;

	}//end _getPurchasableAssetName()


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
		
		$questions =& $this->getAllQuestionAssets();
		foreach ($questions as $q_id => $question) {
			if (key_exists($q_id, $var_rules)) {
				$var_rule = $var_rules[$q_id];
				$key = array_get_index($var_rule, 'key', '');
				if (trim($key) == '') {
					continue;
				}
				
				$val = '';
				if (!($question instanceof Form_Question_Type_Country) && $question->isSelection()) {
					$options = array_get_index($var_rule, 'value', Array());
					foreach ($options as $option_id => $option_val) {
						//if option is selcted and option is not --leave empty--
						if ($question->isSelected($option_id) && (!($question instanceof Form_question_type_select) || !$question->isEmptyOption($option_id))) {
							$val = $option_val;
							break;
						}
					}
				} else {
					$val = $this->current_answers[$q_id];
				}
				
				$key_val_arr[$key] = $val;
			}//end if
		}//end for
		
		return $key_val_arr;

	}//end _getPassThroughVars()
	
	
	/**
	* True if passed asset is purchasable
	*
	* @param object	&$asset	asset to check
	*
	* @return boolean
	* @access private
	*/
	function _isPurchasableAsset(&$asset)
	{
		$price_string = $this->attr('asset_price_source');
		$price = $asset->replaceKeywordsInString($price_string);

		return is_numeric($price);

	}//end _isPurchasableAsset()


	/**
	* True if passed asset is taxable
	*
	* @param object	&$asset	asset to check
	*
	* @return boolean
	* @access private
	*/
	function _isPurchasableAssetTaxable(&$asset)
	{
		$tax_source_string = $this->attr('asset_taxable_source');
		$tax_check_string = $asset->replaceKeywordsInString($tax_source_string);

		return $tax_check_string !== '';

	}//end _isPurchasableAssetTaxable()


	/**
	* True if multiples of passed asset can be purchased
	*
	* @param object	&$asset	asset to check
	*
	* @return boolean
	* @access private
	*/
	function _isPurchasableAssetQuantityAllowed(&$asset)
	{
		$multiple_source_string = $this->attr('asset_multiple_source');
		$check_string = $asset->replaceKeywordsInString($multiple_source_string);

		return $check_string !== '';

	}//end _isPurchasableAssetQuantityAllowed()
	
	
	/**
	* True if the passed asset is promotable
	*
	* @param object	&$asset	asset to check
	*
	* @return boolean
	* @access private
	*/
	function _isPurchasableAssetPromotable(&$asset)
	{
		$promotable_source_string = $this->attr('asset_promotable_source');
		$check_string = $asset->replaceKeywordsInString($promotable_source_string);

		return (boolean) $check_string;

	}//end _isPurchasableAssetPromotable()
	
	
	/**
	* Format price amount with 2 decimals
	*
	* @param string	$amount	amount to format
	*
	* @return string
	* @access private
	*/
	function _formatPriceAmount($amount=0)
	{
		return sprintf('%01.2f', $amount);

	}//end _formatPriceAmount()


	/**
	* True if passed item id is taxable
	*
	* @param string	$item_id	id of item to test
	*
	* @return boolean
	* @access private
	*/
	function _isItemTaxable($item_id)
	{

		$taxable = @$_SESSION[$this->getPrefix()]['order']['taxable_items'][$item_id];
		return !is_null($taxable);

	}//end _isItemTaxable()


	/**
	* Get the filename of log file
	*
	* @return string
	* @access private
	*/
	function _getLogFileName()
	{
		return 'order.log';

	}//end _getLogFileName()


	/**
	* Get the path to log file
	* @param string	$string	string to log
	*
	* @return string
	* @access private
	*/
	function _getLogFilePath()
	{
		return $this->data_path.'/'.$this->_getLogFileName();

	}//end _getLogFilePath()


	/**
	* Writes a string to log file
	*
	* @param string	$string	string to log
	*
	* @return boolean
	* @access private
	*/
	function _log($string='')
	{
		$file = $this->_getLogFilePath();
		if (!$file_pointer = fopen($file, 'a')) return FALSE;

		$sucess = fputs($file_pointer, $string);
		fclose($file_pointer);
		if ($sucess === FALSE) {
			trigger_error('Unable to write '.$file, E_USER_WARNING);
			return FALSE;
		}

		$om = umask(0000);
		chmod($file, 0664);
		umask($om);

		return TRUE;

	}//end _log()


}//end class

?>
