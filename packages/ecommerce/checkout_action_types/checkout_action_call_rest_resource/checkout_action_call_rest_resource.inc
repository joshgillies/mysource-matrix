<?php
/**
* +--------------------------------------------------------------------+
* | This MySource Matrix CMS file is Copyright (c) Squiz Pty Ltd	   |
* | ABN 084 670 600													   |
* +--------------------------------------------------------------------+
* | IMPORTANT: Your use of this Software is subject to the terms of    |
* | the Licence provided in the file licence.txt. If you cannot find   |
* | this file please contact Squiz (www.squiz.com.au) so we may provide|
* | you a copy.														   |
* +--------------------------------------------------------------------+
*
* $Id: checkout_action_call_rest_resource.inc,v 1.8 2013/09/10 06:05:32 cupreti Exp $
*
*/

require_once SQ_PACKAGES_PATH.'/ecommerce/checkout_action/checkout_action.inc';
require_once SQ_ATTRIBUTES_PATH.'/http_request/http_request.inc';
require_once SQ_ATTRIBUTES_PATH.'/email_format/email_format.inc';

define('ECOM_CHECKOUT_REST_ACTION_LOG_FILE', 'ecom_rest_checkout_action');

/**
* Checkout_Action_Call_Rest_Resource
*
* Purpose
*
*
* @author  Chiranjivi Upreti <cupreti@squiz.com.au>
* @version $Revision: 1.8 $
* @package MySource_Matrix_Packages
* @subpackage ecommerce
*/
class Checkout_Action_Call_Rest_Resource extends Checkout_Action
{

	/**
	* Constructor
	*
	*/
	function __construct()
	{
		parent::__construct();

	}//end constructor


	/**
	* Paint inline interface
	*
	* @access public
	* @return void
	*/
	public static function paintInlineInterface(Ecommerce_Checkout $checkout, $settings, Backend_Outputter $o, $prefix)
	{	
		if (!isset($settings['http_request'])) {
			$req_settings = serialize(Array());
		} else {
			$req_settings = $settings['http_request'];
		}
		
		if (!isset($settings['email_format'])) {
			$email_settings = serialize(Array());
		} else {
			$email_settings = $settings['email_format'];
		}

		// Set up HTTP request attribute
		$http_request = new Asset_Attribute_HTTP_Request(0, $req_settings);
		
		// Set up Error Response Email attribute
		$email_format = new Asset_Attribute_Email_Format(0, $email_settings);
		
		echo '<p class="sq-backend-section-subheading">REST Request</p>';
		echo '<div id="'.$prefix.'"_http_request_div" style="padding: 0.5em 0px 2em 30px;">';
			$http_request->paint($prefix.'_hr');
		echo '</div>';
		
		// Paint "order array" setup screen
		echo '<p class="sq-backend-section-subheading">Order Array Configuration</p>';
		echo '<div id="'.$prefix.'"_order_array_div" style="padding: 0.5em 0px 2em 30px;">';		
			self::_paintOrderArraySetup($checkout, $settings, $o, $prefix);
		echo '</div>';
		
		// Paint error email response configuration screen
		echo '<p class="sq-backend-section-subheading"> Error Response Notification Email</p>';
		echo '<div id="'.$prefix.'"_email_format_div" style="padding: 0.5em 0px 2em 30px;">';		
			$email_format->paint($prefix.'_email');
		echo '</div>';		
		
		// Paint "order array" setup screen
		echo '<p class="sq-backend-section-subheading">Available Keywords</p>';
		echo '<div id="'.$prefix.'"_keywords_list_div" style="padding: 0.5em 0px 2em 30px;">';		
			self::_paintAvailableKeywords($prefix, $o, $settings);
		echo '</div>';

	}//end paintInlineInterface()
	
	
	/**
	* Paint inline interface for order array setup
	*
	* @access private
	* @return void
	*/
	private static function _paintOrderArraySetup(Ecommerce_Checkout $checkout, $settings, Backend_Outputter $o, $prefix)
	{	
		$order_array = array_get_index($settings, 'order_array', Array());
		
		$current_customisation = self::_getCurrentCustomisation($prefix, $settings);
		
		// Customise asset types drop down name
		$dropdown_field = $prefix.'_asset_type_customise';
		
		?>
		<table style="width:100%">
			<tr><td><b>Customise Order Asset Types</b></td></tr>
			<?php
			// Configured Product asset types
			$configured_assets = array_keys($order_array);		
			if (!empty($configured_assets)) { ?>				
					<tr><td>
						<table class="sq-backend-table" style="width:50%">
							<tr>								
								<td class="sq-backend-table-header">Customised Order Asset Types</td>
								<td class="sq-backend-table-header">Remove</td>
							</tr>
							<?php								
								
								foreach ($configured_assets as $key) {									
									
									$type_name = ucwords(str_replace('_', ' ', $key));

									// when we click on a customisation, set a hidden field to the
									// key that we are customising and submit the screen to save any options
									// that have been changed
									
									$src  = 'Javascript: document.getElementById(\''.$dropdown_field.'\').value = \''.$key.'\';';
									$src .= 'main_form.submit()';
									?>
									<tr>
										<td>
											<a href="<?php echo $src?>"><?php echo $type_name; ?></a>
										</td>
										<td style="width:10%">
											<?php check_box($dropdown_field.'_remove['.$key.']', 1, FALSE); ?>
										</td>											
									</tr>
									<?php
								}//end foreach								
							?>
						</table>
					</td></tr>
			<?php 
			}
			hidden_field($dropdown_field, '');
			echo '<tr><td>';	
			// Dropdown to select the Project asset type
			$current_customisation = self::_getCurrentCustomisation($prefix, $settings);					
			self::_paintAssetTypeChooser($prefix.'_asset_type_selected', $current_customisation);
			echo '</td></tr>';
			
			// Interface to setup the paramters for the "Order Array"
			if (!empty($current_customisation)) {
			?>
				<tr><td>&nbsp;</td></tr>
				<tr><td><b>Parameter Setup for "<?php echo ucwords(str_replace('_', ' ', $current_customisation)); ?>"</b></td></tr>
				<?php
				
				$customised_values = array_get_index($order_array, $current_customisation, Array());				
				if (!empty($customised_values)) {
					?>					
					<tr><td>
						<table class="sq-backend-table" style="width:60%">
							<tr>								
								<td class="sq-backend-table-header">Array Item Name</td>
								<td class="sq-backend-table-header">Value</td>
								<td class="sq-backend-table-header">Remove</td>
							</tr>				
							<?php
							foreach($customised_values as $param => $value) { 
								echo '<tr>';
									echo '<td>'.$param.'</td>';
									echo '<td>';
										text_box($prefix.'_params_value['.$param.']', $value, 50);
									echo '</td>';
									echo '<td style="width:10%">';
										check_box($prefix.'_params_remove['.$param.']', 1, FALSE);
									echo '</td>';
								echo '</tr>';
							}
							?>
						</table>
					</td></tr>
					<?php
				}
				echo '<tr><td>';
				echo "Add new parameter: ";
				text_box($prefix.'_new_param', '', 21);
				hidden_field($prefix.'_asset_type_submitted', $current_customisation);
				echo '&nbsp;';
				submit_button($prefix.'_new_param_button', translate('go'), 'document.getElementById(\''.$dropdown_field.'\').value = \''.$current_customisation.'\'; main_form.submit()');
				echo '</td></tr>';				
			}
			?>			
			</table><?php	
	
	}// _paintOrderArraySetup()
	
	
	/**
	* Display the keywords available for use in the rest request body
	* and the "order array"
	*
	* @return void
	* @accesss private
	*/
	private static function _paintAvailableKeywords($prefix, Backend_Outputter $o, $settings)
	{
		$asset_type = self::_getCurrentCustomisation($prefix, $settings);		
		?>
		<table style="width:100%">
		<?php
		if (!empty($asset_type)) {
			$asset_type_name = ucwords(str_replace('_', ' ', $asset_type));
			$order_array_keywords = self::_getOrderArrayKeywords($asset_type);
		?>
			<tr><td><b>In Order Array for "<?php echo $asset_type_name ?>" Type Asset (Order Items Keywords)</b></td></tr>
			<tr><td>
				<table class="sq-backend-table" style="width:80%">
					<tr>								
						<td class="sq-backend-table-header" style="width:30%">Keywords</td>
						<td class="sq-backend-table-header">Description</td>
					</tr>
					<?php					
					foreach($order_array_keywords as $keyword => $desc) {
						echo '<tr>';
						echo '<td>%'.$keyword.'%</td>';
						echo '<td>'.$desc.'</td>';
						echo '</tr>';							
					}//end foreach						
					
					?>
					<tr><td colspan="2">NOTE: Generic asset keywords (%asset_assetid%, %asset_name%, etc) and metadata keywords (%asset_metadata_<MFIELD_NAME>%) for the product item assets are also available</td></tr>
				</table>
			</td></tr>
			<tr><td>&nbsp;</td></tr>
		<?php		
		}		
		
		$rest_keywords = self::_getRestRequestKeywords();
		?>
			<tr><td><b>In REST Request (Order Keywords)</b></td></tr>
			<tr><td>
				<table class="sq-backend-table" style="width:80%">
					<tr>								
						<td class="sq-backend-table-header" style="width:30%">Keywords</td>
						<td class="sq-backend-table-header">Description</td>
					</tr>
					<?php					
					foreach($rest_keywords as $keyword => $desc) {
						echo '<tr>';
						echo '<td>%'.$keyword.'%</td>';
						echo '<td>'.$desc.'</td>';
						echo '</tr>';							
					}//end foreach						
					
					?>					
				</table>
			</td></tr>
			<tr><td>&nbsp;</td></tr>
		<?php
		
		$email_keywords = self::_getEmailFormatKeywords();
		?>
			<tr><td><b>In Error Response Notification Email</b></td></tr>
			<tr><td>
				<table class="sq-backend-table" style="width:80%">
					<tr>								
						<td class="sq-backend-table-header" style="width:30%">Keywords</td>
						<td class="sq-backend-table-header">Description</td>
					</tr>
					<?php					
					foreach($email_keywords as $keyword => $desc) {
						echo '<tr>';
						echo '<td>%'.$keyword.'%</td>';
						echo '<td>'.$desc.'</td>';
						echo '</tr>';							
					}//end foreach						
					
					?>
					<tr><td colspan="2">NOTE: All keywords in the "Order Keywords" list are also available</td></tr>
				</table>
			</td></tr>		
			
		</table>		
		<?php

	}
	
	
	/**
	* Paints the interface for choosing an asset type.
	*
	* Only non-system assets and instantiable assets and their ancestors are printed
	*
	* @param string	$field_name			the name to use for the combo box
	* @param string	$selected_type_code	the type_code that should be currently selected
	*
	* @return void
	* @access private
	*/
	private static function _paintAssetTypeChooser($field_name, $selected_type_code)
	{
		// All "product"/"ecommerce product" type assets in the system
		$asset_types = $GLOBALS['SQ_SYSTEM']->am->getTypeDescendants('product', TRUE);
		$asset_types_info = Array();		
		foreach ($asset_types as $type_code) {			
			$asset_types_info[$type_code] = $GLOBALS['SQ_SYSTEM']->am->getTypeInfo($type_code, 'name');
		}
		$asset_types_info = array_unique($asset_types_info);
		arsort($asset_types_info);		
		$asset_types_info[''] = '';
		$asset_types_info = array_reverse($asset_types_info);

		echo "Add Product asset type to customise: ";
		combo_box($field_name, $asset_types_info, FALSE, $selected_type_code);		
		echo '&nbsp;';
		submit_button($field_name.'_go', translate('go'), 'main_form.submit()');

	}//end _paintAssetTypeChooser()
	
	
	/**
	* Returns the key of the current asset type be configured
	*
	* @param string	$prefix	the field prefix	
	* @param array	$settings
	*
	* @return string
	* @access private
	*/
	private static function _getCurrentCustomisation($prefix, $settings)
	{			
		$key = array_get_index($_POST, $prefix.'_asset_type_selected', '');

		// if there was nothing selected to customise, check for a current customisation in process
		if (empty($key)) {
			$key = array_get_index($_POST, $prefix.'_asset_type_current', '');
		}

		// check to see if a current customisation was selected
		if (isset($_POST[$prefix.'_asset_type_customise']) && !empty($_POST[$prefix.'_asset_type_customise'])) {
			$key = $_POST[$prefix.'_asset_type_customise'];
		}
		
		// if still nothing, get the first one from the list of customised assets
		if (empty($key)) {
			$customised_assets = array_keys(array_get_index($settings, 'order_array', Array()));
			$key = isset($customised_assets[0]) ? $customised_assets[0] : '';			
		}
		
		return $key;

	}//end _getCurrentCustomisation()


	/**
	* Process inline interface
	*
	* @access public
	* @return boolean
	*/
	public static function processInlineInterface(Ecommerce_Checkout $checkout, &$settings, Backend_Outputter $o, $prefix)
	{
		// Process "http request" settings
		if (!isset($settings['http_request'])) {
			$req_settings = serialize(Array());
		} else {
			$req_settings = $settings['http_request'];
		}
		$http_request = new Asset_Attribute_HTTP_Request(0, $req_settings);		
		$http_request->process($prefix.'_hr');
		$settings['http_request'] = $http_request->value;
		
		// Process "email format" settings
		if (!isset($settings['email_format'])) {
			$email_settings = serialize(Array());
		} else {
			$email_settings = $settings['email_format'];
		}		
		$email_format = new Asset_Attribute_Email_Format(0, $email_settings);
		$email_format->process($prefix.'_email');
		$settings['email_format'] = $email_format->value;
		
		// Process "order array" data
		self::_processOrderArraySetup($checkout, $settings, $o, $prefix);

		return TRUE;

	}//end processInlineInterface()
	
	
	/**
	* Process inline interface
	*
	* @access public
	* @return boolean
	*/
	public static function _processOrderArraySetup(Ecommerce_Checkout $checkout, &$settings, Backend_Outputter $o, $prefix)
	{			
		$order_array = isset($settings['order_array']) ? $settings['order_array'] : Array();
		$selected_asset_type = isset($_POST[$prefix.'_asset_type_submitted']) ? $_POST[$prefix.'_asset_type_submitted'] : '';
		
		
		if (!empty($selected_asset_type)) {
		
			if (!isset($order_array[$selected_asset_type])) {
				$order_array[$selected_asset_type] = Array();
			}
			
			// Add new parameter for the selected asset type
			if (!empty($_POST[$prefix.'_new_param'])) {
				$order_array[$selected_asset_type][$_POST[$prefix.'_new_param']] = '';
			}
			
			// Update parameter data
			$update_params = isset($_POST[$prefix.'_params_value']) ? $_POST[$prefix.'_params_value'] : Array();
			foreach($update_params as $param_name => $param_value) {
				$order_array[$selected_asset_type][$param_name] = $param_value;
			}
			
			// Remove the selected params
			$remove_params = isset($_POST[$prefix.'_params_remove']) ? $_POST[$prefix.'_params_remove'] : Array();
			foreach($remove_params as $param_name => $param_value) {
				if (isset($order_array[$selected_asset_type][$param_name])) {
					unset($order_array[$selected_asset_type][$param_name]);
				}
			}//end foreach		
		}
		// Remove the selected customised asset types
		$remove_types = isset($_POST[$prefix.'_asset_type_customise_remove']) ? $_POST[$prefix.'_asset_type_customise_remove'] : Array();		
		foreach($remove_types as $asset_type => $val) {
			if (isset($order_array[$asset_type])) {
				unset($order_array[$asset_type]);
			}
		}//end foreach

		// Update the "order array" value
		$settings['order_array'] = $order_array;		
		
		return TRUE;
	
	}//end _processOrderArraySetup()
	

	/**
	* Paint summary description
	*
	* @access public
	* @return void
	*/
	public static function paintSummary(Ecommerce_Checkout $checkout, $settings, Backend_Outputter $o, $prefix)
	{
		if (!isset($settings['http_request'])) {
			$req_settings = Array();
		} else {
			$req_settings = unserialize($settings['http_request']);
		}

		?><table class="no-borders">
			<colgroup>
				<col width="80" />
				<col/>
			</colgroup>
			<tbody>
				<tr>
				<td class="sq-backend-table-cell" style="vertical-align: top"><p><strong><?php echo translate('http_request_method'); ?></strong></p></td>
					<td class="sq-backend-table-cell" style="vertical-align: top"><p><?php

					// Get manually-entered "method" settings.
					if (!empty($req_settings['method'])) {
						echo $req_settings['method'];
					} else {
						?><span class="sq-backend-warning">No method specified.</span><?php
					}
					?></p></td>
				</tr>
				<tr>
				<td class="sq-backend-table-cell" style="vertical-align: top"><p><strong><?php echo translate('http_request_urls'); ?></strong></p></td>
					<td class="sq-backend-table-cell" style="vertical-align: top"><p><?php
					if (!empty($req_settings['urls'])) {
						foreach ($req_settings['urls'] as $url) {
							echo ellipsisize($url, 512).'<br />';
						}
					} else {
						?><span class="sq-backend-warning">No URLs specified.</span><?php
					}
				?></p></td></tr>
			</tbody>
		</table>
		<?php

	}//end paintSummary()


	/**
	* Execute checkout action
	*
	* @access public
	* @return boolean
	*/
	public static function execute(Ecommerce_Checkout $checkout, Ecommerce_Order $order, $settings)
	{
		$req_settings = $settings['http_request'];

		// Set up http request object
		$http_request = new Asset_Attribute_HTTP_Request(0, $req_settings);
		$current_value = @unserialize($http_request->value);		

		// Get list of keywords in http request
		$keywords_list = Array();
		if (isset($current_value['urls'])) {
			$keywords_list = array_merge(
										$keywords_list,
										extract_keywords(implode(', ', $current_value['urls']))
									);
		}

		if (isset($current_value['request_headers'])) {
			$keywords_list = array_merge(
										$keywords_list,
										extract_keywords(implode(', ', $current_value['request_headers']))
									);
		}

		if (isset($current_value['request_body'])) {
			$keywords_list = array_merge(
										$keywords_list,
										extract_keywords($current_value['request_body'])
									);
		}
		
		if (isset($current_value['http_auth_options'])) {
			$keywords_list = array_merge(
										$keywords_list,
										extract_keywords(implode(', ', $current_value['http_auth_options']))
									);
		}
		
		// Setup the email format attribute
		$email_settings = $settings['email_format'];		
		$email_format = new Asset_Attribute_Email_Format(0, $email_settings);
		$email_val = @unserialize($email_format->value);
		
		// Get list of keywords in email attribute
		$keywords_list = array_merge($keywords_list, extract_keywords($email_val['html_format'].', '.$email_val['text_format'].', '.$email_val['subject'].', '.$email_val['from'].', '.$email_val['reply_to'].', '.implode(', ', $email_val['to'])));		
		
		// Get order data
		$order_xml = $order->attr('order_xml');
		// If "strip HTML tags" global preference is enabled, 
		// then we will need to decode the encoded xml data to get proper xml
		if (strpos($order_xml, '<') === FALSE) {
			$order_xml = htmlspecialchars_decode($order_xml, ENT_COMPAT);
		}
		$order_data = xml_to_array($order_xml);

		// If the order was tax exempted (i.e. for non-taxable country)
		if (isset($order_data['cart'][0]['total_tax'][0]) && ($order_data['cart'][0]['total_tax'][0] > 0)) {
			$order_data['taxable'] = TRUE;
		} else {
			$order_data['cart'][0]['total_tax'][0] = '0';
			$order_data['taxable'] = FALSE;
		}
		
		// Get all the keyword replacements		
		$keywords_list = array_unique($keywords_list);
		$replacements = Array();
		foreach($keywords_list as $full_keyword) {			
			// Get the root
			$keyword = parse_keyword($full_keyword, $modifiers);
			
			$replacement = '%'.$keyword.'%';
			switch ($keyword) {
				case 'order_subtotal':
					$replacement = @$order_data['cart'][0]['sub_total'][0];
				break;
				
				case 'order_total_tax':
					$replacement = @$order_data['cart'][0]['total_tax'][0];
				break;
				
				case 'order_total_delivery':
					$replacement = @$order_data['cart'][0]['delivery_total'][0];
				break;
				
				case 'order_grand_total_excl_delivery':
					$replacement = @$order_data['cart'][0]['grand_total_ex_delivery'][0];
				break;
				
				case 'order_grand_total':
					$replacement = @$order_data['cart'][0]['grand_total'][0];
				break;
				
				case 'order_total_discount':
					$replacement = @$order_data['cart'][0]['total_discount'][0];
				break;
				
				case 'order_grand_total_ex_discount':
					$replacement = @$order_data['cart'][0]['grand_total_ex_discount'][0];
				break;
				
				case 'order_promotional_code':
					$replacement = @$order_data['cart'][0]['promotional_code'][0];
				break;
				
				case 'order_currency_symbol':
					$replacement = @$order_data['cart'][0]['currency_symbol'][0];					
				break;
				
				case 'order_item_count':
					$items = @$order_data['cart'][0]['products'][0]['product'];					
					$item_count = 0;
					if (!is_null($items)) {
						foreach($items as $item) {
							if (isset($item['quantity'])) {
								$item_count += $item['quantity'][0];
							}//end if
						}//end foreach
					}//end if
					$replacement = $item_count;
				break;

				case 'order_item_type_count':
					$items = @$order_data['cart'][0]['products'][0]['product'];
					$replacement = count($items);
				break;

				case 'order_date':
					$timezone_offset = substr(date('O', $order->created), 0, 1).str_pad(date('Z', $order->created)/60, 3, '0', STR_PAD_LEFT);
					$replacement = date('YmdHis000', $order->created).$timezone_offset;
				break;
				
				case 'order_date_reg':
					$replacement = date('Y-m-d H:i:s', $order->created);
				break;
				
				case 'order_array':
					$replacement = self::_evaluateOrderArrayKeywordReplacement($order_data, $settings);
				break;
				
			}//end switch
			
			// Delivery form response keywords
			$matches = Array();
			if (preg_match('|order_response_q(\d+)|i', $keyword, $matches) && isset($matches[1])) {
				$delivery_form_values = $order->attr('order_form_value');
				if (isset($delivery_form_values['q'.$matches[1]])) {
					$replacement = $delivery_form_values['q'.$matches[1]];
				}
			}			
			
			// Get remaining replacements for the Ecomm order asset
			if ($replacement == '%'.$keyword.'%') {
				$replacement = $order->getKeywordReplacement($keyword);
			}
			
			// If keyword been replaced, apply the modifiers
			if (!is_null($replacement) && $replacement != '%'.$keyword.'%') {
				apply_keyword_modifiers($replacement, $modifiers);				
				$replacements[$full_keyword] = $replacement;
			}		
		}//end foreach

		// Set them to the http request object
		$http_request->setKeywordReplacements($replacements);

		// Send the request
		$res = $http_request->run();
		$rest_request = @$res['request'];
		$rest_response = @$res['response'];
		
		// Write the info in the log file
		self::_log('-- Order Assetid --');
		self::_log($order->id);
		self::_log('-- REST Request --');
		self::_log($rest_request);
		self::_log('-- REST Response --');
		self::_log($rest_response);
		
		// Send the notification email if REST resource returns non-2xx response
		$http_response_code = @$res['response']['info']['http_code'];		
		if (strpos($http_response_code, '2') !== 0) {
			foreach($replacements as $key => $val) {
				$replacements[$key] = urldecode($val);
			}
			// Further add these additionals keyword replacements specified to the email			
			$request_headers = '';
			foreach($rest_request['headers'] as $key => $val) {
				$request_headers .= $key.': '.$val.'<br />';
			}			
			$replacements['rest_request_headers'] = $request_headers;
			$replacements['rest_request_body'] = isset($rest_request['body']) ? $rest_request['body'] : '';			
			
			$response_headers = '';
			foreach($rest_response['headers'] as $key => $val) {
				$response_headers .= $key.': '.$val.'<br />';
			}			
			$replacements['rest_response_headers'] = $response_headers;
			$replacements['rest_response_body'] = isset($rest_response['body']) ? $rest_response['body'] : '';

			$email_format->sendMail($replacements);
		}		

		return TRUE;

	}//end execute()
	
	
	/**
	* Evaluates the special "order_array" keyword
	*
	* @access private
	* @return string
	*/	
	private static function _evaluateOrderArrayKeywordReplacement($order_info, $settings)
	{	
		$order_array = '';
		$ordered_items_raw = @$order_info['cart'][0]['products'][0]['product'];
		$params = @$settings['order_array'];
		
		if (empty($params) || empty($ordered_items_raw)) {
		
			return $order_array;
		}		
		
		foreach($ordered_items_raw as $item) {
			if (isset($item['id'][0])) {				
				$ordered_items[$item['id'][0]] = $item;			
			}			
		}
		
		$taxable = $order_info['taxable'];
		$items_asset_info = $GLOBALS['SQ_SYSTEM']->am->getAssetInfo(array_keys($ordered_items));
		$array_index = 0;
		foreach($items_asset_info as $item_id => $item_asset_info) {
			$item_data = $ordered_items[$item_id];
			$item_type = $item_asset_info['type_code'];
			
			// Parameters configured for this Product type
			$item_params =  isset($params[$item_type]) ? $params[$item_type] : FALSE;
			if (!$item_params) {
				continue;				
			}
			$item_asset = $GLOBALS['SQ_SYSTEM']->am->getAsset($item_id);
			foreach($item_params as $param_name => $value) {
				// Extract and replace the keywords in the parameter value, if any
				$keywords = extract_keywords($value);				
				if (empty($keywords)) {
					$parm_value = $value;
				} else {
					$replacements = Array();
					foreach($keywords as $full_keyword) {
						$keyword = parse_keyword($full_keyword, $modifiers);			
						$replacement = '%'.$keyword.'%';
						if (strpos($keyword, 'product_') !== FALSE) {
							$item_index_name = substr($keyword, 8);							
							if (isset($item_data[$item_index_name])) {
								switch ($item_index_name) {
									case 'item_price_incl_tax':
										// The product pricing data in the order will include tax value,
										// so for tax exempted order, fix the pricings for the keyword replcements here
										if (!$taxable) {
											$replacement = @$item_data['item_price'][0];
										} else {
											$replacement = $item_data[$item_index_name][0];
										}
									break;

									case 'total_item_price_incl_tax':
										if (!$taxable) {
											$replacement = @$item_data['total_item_price'][0];
										} else {
											$replacement = $item_data[$item_index_name][0];
										}
									break;

									case 'item_tax':
										if (!$taxable) {
											$replacement = '0';
										} else {
											$replacement = $item_data[$item_index_name][0];
										}
									break;

									case 'total_item_tax':
										if (!$taxable) {
											$replacement = '0';
										} else {
											$replacement = $item_data[$item_index_name][0];
										}
									break;

									default:
										$replacement = $item_data[$item_index_name][0];
									break;
								}//end switch
							}//end if
						} else if ($keyword == 'item_array_index') {
							$replacement = str_pad($array_index, 4, '0', STR_PAD_LEFT);
						}
						if ($replacement == '%'.$keyword.'%') {
							$replacement = $item_asset->getKeywordReplacement($keyword);
						}
						
						// If keyword was replaced apply the modifiers
						if ($replacement != '%'.$keyword.'%') {
							apply_keyword_modifiers($replacement, $modifiers);
						} else {
							// Blank out the keywords not replaced
							$replacement = '';
						}
						$replacements[$full_keyword] = $replacement;
					}//end foreach					
					replace_keywords($value, $replacements);
				}//end else
				
				// Add the parameter to the array
				$order_array .= $param_name.'['.$array_index.']='.urlencode($value).'&';				
			}//end foreach
			
			$array_index++;
			
		}//end foreach
		
		$order_array = rtrim($order_array, '&');		
		
		return $order_array;
		
	}//end _evaluateOrderArrayKeywordReplacement()
	
	
	/**
	 * Validates the checkout action.
	 *
	 * @access public
	 * @return boolean
	 */
	public static function isValid(Ecommerce_Checkout $checkout, $settings)
	{		
		if (!isset($settings['http_request'])) {
			$req_settings = Array();
		} else {
			$req_settings = unserialize($settings['http_request']);
		}

		// if either no method or urls are set, won't be able to send the request... 
		if (empty($req_settings['method'])) return FALSE;
		if (empty($req_settings['urls'])) return FALSE;		

		return TRUE;

	}//end isValid
	
	
	/**
	* Get the available keywords for the given asset type
	*
	* @return array
	* @access private
	*/
	private static function _getAvailableKeywords($asset_type, $ommit_generic_keywords=TRUE)
	{
		$GLOBALS['SQ_SYSTEM']->am->includeAsset($asset_type);
		$temp_asset = new $asset_type();
		$keywords = $temp_asset->getAvailableKeywords();
		
		if ($ommit_generic_keywords) {
			// Get rid of generic default keywords from the list
			foreach($keywords as $keyword => $desc) {
				if (strpos($keyword, 'asset_attribute_') === FALSE) {
					unset($keywords[$keyword]);
				} else {
					// We have reached asset_attribute_* keyword, which means
					// generic keywords are no more in the list
					break;
				}
			}//end foreach
		}		
		
		return $keywords;
		
	}//end _getAvailableKeywords()
	

	/**
	* Get the available keywords in the "order array"
	*
	* @return array
	* @access private
	*/
	private static function _getOrderArrayKeywords($asset_type)
	{
		$keywords = Array();
		$keywords['item_array_index'] = 'Four digit item\'s position value in the "order array"';
		$keywords['asset_attribute_name'] = "The full name of the product item";
		$keywords['product_quantity'] = "Total number of the items of the given type in the order";
		$keywords['product_item_price'] = "Product item unit price excluding tax";
		$keywords['product_item_price_incl_tax'] = "Product item unit price including tax";
		$keywords['product_item_tax'] = "Product item unit tax amount";
		$keywords['product_total_item_price'] = "Total price of the items of the given type in the order excluding tax";
		$keywords['product_total_item_price_incl_tax'] = "Total price of the items of the given type in the order including tax";
		$keywords['product_total_item_tax'] = "Total tax for the given item type in the order";
		$keywords['product_item_discount'] = "Product item's unit discount amount";
		$keywords['product_total_item_price_incl_discount'] = "Total price of the items of the given type in the order with the discount applied";
		
		$keywords += self::_getAvailableKeywords($asset_type);
		
		return $keywords;
		
	}//end _getOrderArrayKeywords()

	
	/**
	* Get the available keywords in REST request body
	*
	* @return array
	* @access private
	*/
	private static function _getRestRequestKeywords()
	{	
		// Keywords added for this "Checkout action"
		$keywords = Array(
						'order_array' => 'Array of data for the each item in the order (as configured in the "Order Array" section)',
						'order_response_qX' => 'Response submitted for Delivery Method\'s question "X"',
						'order_item_count' => 'Total number of quantity in the order',
						'order_item_type_count' => 'Total number of item types in the order',						
						'order_subtotal' => 'Total transaction amount excluding tax and delivery',
						'order_total_tax' => 'Total transaction amount including tax',
						'order_total_delivery' => 'Total delivery amount',
						'order_grand_total_excl_delivery' => 'Grand total transaction amount excluding delivery',
						'order_total_discount' => 'Total discount amount',						
						'order_grand_total_ex_discount' => 'Grand total transaction amount before discount',						
						'order_grand_total' => 'Grand total transaction amount',						
						'order_promotional_code' => 'Promotional code used',						
						'order_date' => 'Order date timestamp in YYYYMMDDHHNNSSKKKs000 format',
						'order_date_reg' => 'Order date timestamp in YYYY:MM:DD HH:MM:SS format',
						'order_currency_symbol' => 'Currency symbol for the transaction amount',
					);
					
		// Available "Ecommerce Order" keywords		
		$keywords += self::_getAvailableKeywords('ecommerce_order');
		
		return $keywords;
		
	}//end _getRestRequestKeywords
	
	
	/**
	* Get the available keywords in REST request body
	*
	* @return array
	* @access private
	*/
	private static function _getEmailFormatKeywords()
	{	
		// Keywords added for this "Checkout action"
		$keywords = Array(
						'rest_request_headers' => 'REST request headers',
						'rest_request_body' => 'REST request body content',
						'rest_response_headers' => 'Response headers returned by the REST resource',
						'rest_response_body' => 'Response body returned by the REST resource',
					);		
		
		return $keywords;	
		
	}//end _getEmailFormatKeywords();
	
	
	
	/**
	* Log a message to the rest action log file
	*
	* @param string	$message	the message to log
	* @param int	$level		the error level of the message
	* @param bolean	$encode		the flag indicates if the message needs to be encoded
	*
	* @return void
	* @access private
	*/
	private static function _log($message, $level = E_USER_NOTICE, $encode=FALSE)
	{
		if (is_array($message)) {
			$message = print_r($message, TRUE);		
		}
		
		log_write($message, ECOM_CHECKOUT_REST_ACTION_LOG_FILE, $level, $encode);
		
	}//end _log()


}//end class

?>
