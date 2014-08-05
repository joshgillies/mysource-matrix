<?php
/**
* +--------------------------------------------------------------------+
* | This MySource Matrix CMS file is Copyright (c) Squiz Pty Ltd	   |
* | ABN 77 084 670 600												   |
* +--------------------------------------------------------------------+
* | IMPORTANT: Your use of this Software is subject to the terms of    |
* | the Licence provided in the file licence.txt. If you cannot find   |
* | this file please contact Squiz (www.squiz.com.au) so we may provide|
* | you a copy.														   |
* +--------------------------------------------------------------------+
*
* $Id: form_action_call_rest_resource.inc,v 1.8 2013/04/16 01:29:51 ewang Exp $
*
*/


require_once SQ_PACKAGES_PATH.'/cms/form/form_action/form_action.inc';
require_once SQ_ATTRIBUTES_PATH.'/http_request/http_request.inc';
require_once SQ_LIB_PATH.'/html_form/html_form.inc';
require_once SQ_PACKAGES_PATH.'/web_services/rest/lib/rest_common.inc';

/**
* Form_Action_Call_Rest_Resource
*
* Purpose
*
*
* @author  Maki Maruyama <mmaruyama@squiz.co.uk>
* @version $Revision: 1.8 $
* @package MySource_Matrix_Packages
* @subpackage rest
*/
class Form_Action_Call_Rest_Resource extends Form_Action
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
	public static function paintInlineInterface(Form $form, $settings, Backend_Outputter $o, $prefix)
	{
		if (!isset($settings['http_request'])) {
			$req_settings = serialize(Array());
		} else {
			$req_settings = $settings['http_request'];
		}

		// Set up HTTP request attribute and validation rule
		?>
		<div style="padding: 5px">
			<p class="sq-backend-section-subheading"><?php echo translate('form_action_call_rest_resource_http_request') ?></p>
			<?php
		$http_request = new Asset_Attribute_HTTP_Request(0, $req_settings);

		$http_request->paint($prefix);

			$options = self::_getOptions();
			?>
		</div>
		<div style="padding: 5px">
			<p class="sq-backend-section-subheading"><?php echo translate('form_action_call_rest_resource_js_processing') ?></p>
			<table class="sq-backend-table">
				<col width="20%" />
				<col width="80%" />
				<tr>
					<th><?php echo translate('form_action_call_rest_resource_js_processing_session_var'); ?></th>
					<td><?php
						text_box($prefix.'[js_session_var]', array_get_index($settings, 'js_session_var', ''), 20);
						echo '<div class="sq-backend-smallprint">'.translate('form_action_call_rest_resource_js_processing_session_var_note').'</div>';
						?>
					</td>
				</tr>

				<tr>
					<th><?php echo translate('form_action_call_rest_resource_js_processing_replace_keywords'); ?></th>
					<td><?php
						combo_box($prefix.'[js_replace_keywords]', Array(translate('no'), translate('yes')), FALSE, array_get_index($settings, 'js_replace_keywords', 0));
						echo '<div class="sq-backend-smallprint">'.translate('form_action_call_rest_resource_js_processing_replace_keywords_note').'</div>';
						?>
					</td>
				</tr>
				<tr>
					<th><?php echo translate('form_action_call_rest_resource_js_processing_engine'); ?></th>
					<td><?php
						$supported_js_engines = Rest_Common::$supported_js_engines;
						$default_js_engine = Rest_Common::$default_js_engine;
						combo_box($prefix.'[js_engine]', $supported_js_engines, FALSE, array_get_index($settings, 'js_engine', $default_js_engine));
						echo '<div class="sq-backend-smallprint">'.translate('form_action_call_rest_resource_js_processing_engine_note').'</div>';
						?>
					</td>
				</tr>
				<tr>
					<th><?php echo translate('form_action_call_rest_resource_js_processing_include_files'); ?></th>
					<td><?php
						$include_files = array_get_index($settings, 'js_include_files', Array());
						multiple_asset_finder($prefix.'[js_include_files]', $include_files, Array('file' => 'D'));
						echo '<div class="sq-backend-smallprint">'.translate('form_action_call_rest_resource_js_processing_include_files_note').'</div>';
						?>
					</td>
				</tr>
				<tr>
					<th><?php echo translate('form_action_call_rest_resource_js_processing_javascript'); ?></th>
					<td><?php
						$js_code = array_get_index($settings, 'js_code', '');
						text_area($prefix.'[js_code]', $js_code, 71, 10, 0, 'style="font-family: monospace;');
						echo '<div class="sq-backend-smallprint">'.translate('form_action_call_rest_resource_js_processing_javascript_note').'</div>';
						?>
					</td>
				</tr>
			</table>
		</div>
		<div style="padding: 5px" id="xxx">
			<p class="sq-backend-section-subheading"><?php echo translate('form_action_call_rest_resource_details') ?></p>
			<table class="sq-backend-table">
				<col width="20%" />
				<col width="80%" />
				<tr>
					<th><?php label(translate('form_action_call_rest_resource_validation'), $prefix.'[validation_enabled]'); ?></th>
					<td><?php
						$validation_enabled = isset($settings['validation_enabled']) && $settings['validation_enabled'] ? TRUE : FALSE;
						check_box($prefix.'[validation_enabled]', 1, $validation_enabled);
						?>
					</td>
				</tr>
				<tr>
					<th><?php echo translate('form_action_call_rest_resource_details'); ?></th>
					<td><?php
						echo '<span id="'.$prefix.'_validation_scope">';
						combo_box($prefix.'[validation_scope]', $options['scope'], false, array_get_index($settings, 'validation_scope', 0));
						echo '</span>';
						combo_box($prefix.'[validation_target]', $options['targets'], false, array_get_index($settings, 'validation_target', 0), 0, 'onClick="toggleKeywordsReplacementInput()"');
						combo_box($prefix.'[validation_operator]', $options['operators'], false, array_get_index($settings, 'validation_operator', 0));
						text_box($prefix.'[validation_value]', array_get_index($settings, 'validation_value', ''), 40);
						?>
					</td>
				</tr>
				<tr id="<?php echo $prefix; ?>_validation_kw_replacement">
					<th><?php echo "Keyword Replacement"; ?></th>
					<td><?php text_box($prefix.'[validation_kw_replacement]', array_get_index($settings, 'validation_kw_replacement', ''), 80); ?></td>
				</tr>
					<script>
						// Execute initally when backend form loads
						toggleKeywordsReplacementInput();

						function toggleKeywordsReplacementInput()
						{
							if (document.getElementById('<?php echo $prefix; ?>[validation_target]').value == 2) {
								document.getElementById('<?php echo $prefix; ?>_validation_kw_replacement').style.display = "table-row";
								document.getElementById('<?php echo $prefix; ?>_validation_scope').style.display = "none";
								document.getElementById('<?php echo $prefix; ?>[validation_scope]').disabled = true;
							} else {
								document.getElementById('<?php echo $prefix; ?>_validation_kw_replacement').style.display = "none";
								document.getElementById('<?php echo $prefix; ?>_validation_scope').style.display = "table-row";
								document.getElementById('<?php echo $prefix; ?>[validation_scope]').disabled = false;
							}
						}
					</script>
				<tr>
					<th><?php echo translate('form_action_call_rest_resource_custom_error_text'); ?></th>
					<td><?php text_box($prefix.'[validation_error_text]', array_get_index($settings, 'validation_error_text', ''), 80); ?></td>
				</tr>
			</table>
		</div><?php

	}//end paintInlineInterface()


	/**
	* Process inline interface
	*
	* @access public
	* @return boolean
	*/
	public static function processInlineInterface(Form $form, &$settings, Backend_Outputter $o, $prefix)
	{
		if (!isset($settings['http_request'])) {
			$req_settings = serialize(Array());
		} else {
			$req_settings = $settings['http_request'];
		}

		// Set up HTTP request attribute
		$http_request = new Asset_Attribute_HTTP_Request(0, $req_settings);

		$http_request->process($prefix);

		$settings['http_request'] = $http_request->value;

		// and process validation rule settings
		$request_data = $_REQUEST[$prefix];

		$settings['validation_enabled']    = array_get_index($request_data, 'validation_enabled', 0);
		$settings['validation_scope']      = array_get_index($request_data, 'validation_scope', 0);
		$settings['validation_target']     = array_get_index($request_data, 'validation_target', 0);
		$settings['validation_operator']   = array_get_index($request_data, 'validation_operator', 0);
		$settings['validation_value']      = array_get_index($request_data, 'validation_value');
		$settings['validation_error_text'] = array_get_index($request_data, 'validation_error_text', '');
		$settings['validation_kw_replacement']      = array_get_index($request_data, 'validation_kw_replacement', 0);

		$settings['js_session_var'] = preg_replace('/[^a-z0-9_\-]/', '' , strtolower(array_get_index($request_data, 'js_session_var', 0)));
		$settings['js_replace_keywords'] = array_get_index($request_data, 'js_replace_keywords', 0);
		$settings['js_engine'] = array_get_index($request_data, 'js_engine', Rest_Common::$default_js_engine);
		$settings['js_code'] = array_get_index($request_data, 'js_code', '');

		if (isset($request_data['js_include_files'])) {
			$include_files = Array();
			foreach($request_data['js_include_files'] as $index => $asset_info) {
				if (!empty($asset_info['assetid'])) {
					$include_files[] = $asset_info['assetid'];
				}//end if
			}//end foreach
			$settings['js_include_files'] = $include_files;
		}//end if

		// if validation is enabled, we need to run this submission action before submitting the form so that
		// a validation failure can be picked up and prevent the form from being submitted.
		// if the validation is not enabled, then we just need to run this submission action at the normal timing
		// after submitting the form along with other submission actions.
		if ($settings['validation_enabled']) {
			$settings['before_submit'] = TRUE;
		} else {
			// whether the submission action should be run or not is checked by the existence of
			// the 'before_submit' key, so, we'll remove the key from settings if we don't have to validate
			// so that this action will run after submitting the form
			unset($settings['before_submit']);
		}

		return TRUE;

	}//end processInlineInterface()


	/**
	* Paint summary description
	*
	* @access public
	* @return void
	*/
	public static function paintSummary(Form $form, $settings, Backend_Outputter $o, $prefix)
	{
		if (!isset($settings['http_request'])) {
			$req_settings = Array();
		} else {
			$req_settings = unserialize($settings['http_request']);
		}
		$warning_class = self::isValid($form, $settings) ? '' : 'sq-backend-warning';

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
						?><span class="<?php echo $warning_class; ?>"><?php echo translate('form_action_call_rest_resource_no_method'); ?></span><?php
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
						?><span class="<?php echo $warning_class; ?>"><?php echo translate('form_action_call_rest_resource_no_urls'); ?></span><?php
					}
				?></p></td></tr>
			</tbody>
		</table>
		<?php

	}//end paintSummary()


	/**
	* Execute form action
	*
	* @access public
	* @return boolean
	*/
	public static function execute(Form $form, $settings)
	{
		$req_settings = $settings['http_request'];

		// Set up http request object
		$http_request = new Asset_Attribute_HTTP_Request(0, $req_settings);
		$current_value = @unserialize($http_request->value);

		// Get list of replacements
		$keywords_list = Array();
		if (isset($current_value['urls'])) {
			$keywords_list = array_merge(
										$keywords_list,
										retrieve_keywords_replacements(implode(', ', $current_value['urls']))
									);
		}

		if (isset($current_value['request_headers'])) {
			$keywords_list = array_merge(
										$keywords_list,
										retrieve_keywords_replacements(implode(', ', $current_value['request_headers']))
									);
		}

		if (isset($current_value['request_body'])) {
			$keywords_list = array_merge(
										$keywords_list,
										retrieve_keywords_replacements($current_value['request_body'])
									);
		}

		if (isset($current_value['http_auth_options'])) {
			$keywords_list = array_merge(
										$keywords_list,
										retrieve_keywords_replacements(implode(', ', $current_value['http_auth_options']))
									);
		}

		$keywords_list = array_unique($keywords_list);

		// ...and find their replacements
		$replacements = Array();
		foreach ($keywords_list as $keyword) {
			$keyword_value = $form->_getThankYouKeywordReplacement($keyword);
			if (strlen($keyword_value) > 0) {
				$replacements[$keyword] = $keyword_value;
			} else {
				$replacements[$keyword] = $form->getKeywordReplacement($keyword);
				//Fix #4336
				if ($replacements[$keyword] == "%$keyword%" && 0 !== strpos($keyword, 'globals_')) {
					$replacements[$keyword] = '';
				}
			}
		}

		// set them to the http request object
		$http_request->setKeywordReplacements($replacements);

		// send the request
		$res =& $http_request->run();

		// If response needs to be stored in the session var
		if (!empty($settings['js_session_var'])) {
			$js = '';
			$js_file_ids = $settings['js_include_files'];
			foreach($js_file_ids as $js_file_id) {
				$file = $GLOBALS['SQ_SYSTEM']->am->getAsset($js_file_id);
				$existing = $file->getExistingFile();
				if (!empty($existing)) {
					$js .= file_get_contents($existing['path']);
				}
				$GLOBALS['SQ_SYSTEM']->am->forgetAsset($file);
			}
			$js .= $settings['js_code'];

			// Process JS if required
			if (!empty($js)) {

				// Add keywords in the js code to the keyword replacements list
				if ($settings['js_replace_keywords']) {
					self::_addFormKeywordReplacements($form, $js, $replacements);
				}//end if

				// Append the JSON object "_REST" in the beginning of the JS code
				$js = Rest_Common::createJsonStringFromRestResponse($res).$js;

				// Replace global keywords and form submission keywords
				if ($settings['js_replace_keywords']) {
					replace_global_keywords($js);
					replace_keywords($js, $replacements);
				}
				$GLOBALS['SQ_SYSTEM']->pm->startTimer($form, 'processJavascript');
				$ret = Rest_Common::processJavascript($js, $settings['js_engine']);
				$GLOBALS['SQ_SYSTEM']->pm->stopTimer($form, 'processJavascript');
				if (!empty($ret['error_message'])) {
					trigger_localised_error('REST0001', E_USER_WARNING, $form->id, $ret['error_message']);
				} else {
					$_SESSION[SQ_SESSION_SANDBOX_INDEX][$settings['js_session_var']] =  $ret['output'];
				}
			} else {
				// Store the response in the session as an array
				$_SESSION[SQ_SESSION_SANDBOX_INDEX][$settings['js_session_var']] = $res;
			}
		}//end if process js

		// if the validation is enabled, validate the response according to the rule specified
		if (isset($settings['validation_enabled']) && $settings['validation_enabled']) {
			$failed_URLs = self::_validate($settings, $res['responses'], $replacements, $form);

			// if the validation failed, set an appropriate error message to be printed on the frontend
			if (!empty($failed_URLs)) {
				// use custom error text if it's set, otherwise use a default error message
				$error_text = array_get_index($settings, 'validation_error_text', '');
				$failed_URLs_text = implode(', ', $failed_URLs);
				if (!strlen(trim($error_text))) {
					$error_text = translate('form_action_call_rest_resource_false', $failed_URLs_text);
				}
				$form->submission_errors[] = $error_text;

				return FALSE;
			}
		}

		return TRUE;

	}//end execute()


	/**
	 * Validates the form action.
	 *
	 * @access public
	 * @return boolean
	 */
	public static function isValid(Form $form, $settings)
	{
		if (!isset($settings['http_request'])) {
			$req_settings = Array();
		} else {
			$req_settings = unserialize($settings['http_request']);
		}

		// Valid if we have "method" and "url" to make http request
		if (!empty($req_settings['method']) && !empty($req_settings['urls'])) {
			// Need to have both "method" and "url" to make http request
			return TRUE;
		}

		// Valid if we have any js code and session var name set
		return !empty($settings['js_session_var']) && !self::_emptyJsCode($settings);

	}//end isValid


	/**
	* Return TRUE if JS code is empty
	*
	* @param array $settings
	*
	* @return boolean
	* @access private
	*/
	private static function _emptyJsCode($settings)
	{
		$js = isset($settings['js_code']) ? $settings['js_code'] : '';
		if (empty($js)) {
			$js_file_ids = $settings['js_include_files'];
			foreach($js_file_ids as $js_file_id) {
				$file = $GLOBALS['SQ_SYSTEM']->am->getAsset($js_file_id);
				$existing = $file->getExistingFile();
				if (!empty($existing)) {
					$js = file_get_contents($existing['path']);
				}
				$GLOBALS['SQ_SYSTEM']->am->forgetAsset($file);
				if (!empty($js)) {
					// Just need to know if there is any js code
					break;
				}
			}
		}//end if

		return empty($js);

	}//end emptyJsCode()


	/**
	* Returns available options
	*
	* @access protected
	* @return array
	*/
	protected function _getOptions() {
		$options['scope'] = Array(
							translate('form_action_call_rest_resource_all'),
							translate('form_action_call_rest_resource_one'),
						);
		$options['targets'] = Array(
							translate('form_action_call_rest_resource_http_code'),
							translate('form_action_call_rest_resource_response_bodies'),
							translate('form_action_call_rest_resource_keywords_replacement'),
						);
		$options['operators'] = Array(
							translate('form_action_call_rest_resource_match'),
							translate('form_action_call_rest_resource_not_match'),
							translate('form_action_call_rest_resource_greater'),
							translate('form_action_call_rest_resource_less'),
						);

		return $options;
	}


	/**
	* Validates responses from REST Calls
	*
	* This examins the responses based on the validation rule specified on the form submission action
	* and returns a list of URLs of which response failed the validation.
	*
	* @param array	$settings	form submission action settings including the validation rule
	* @param array	$responses	responses from REST Calls
	* @param array	$replacements	Form specific keyword replacements
	* @param object $form			The form asset
	*
	* @access protected
	* @return array
	*/
	protected function _validate($settings, $responses, $replacements, $form) {
		// if validation is not enabled, nothing needs to be validated
		if (isset($settings['validation_enabled']) && !$settings['validation_enabled']) return TRUE;

		// get the validation rule
		$scope    = array_get_index($settings, 'validation_scope', 0);
		$target   = array_get_index($settings, 'validation_target', 0);
		$operator = array_get_index($settings, 'validation_operator', 0);
		$value    = array_get_index($settings, 'validation_value', '');

		if (empty($responses) && $target == '2') {
			// If we are checking keyword string, we dont need http response
			// Create an empty response structure for such scnerio
			$responses = Array(Array('info' => Array('url' => '')));
		}
		// response validation
		// loop through the responses back from the REST call to a web service and
		// validate the part of a response specified by this rule
		$failed_URLs = Array();
		foreach ($responses as $response) {
			// find the part of the response we need to validate
			switch ($target) {
				case 0: // HTTP Code
					$target_value = isset($response['info']['http_code']) ? $response['info']['http_code'] : NULL;
					break;
				case 1: // Response Body
					$target_value = isset($response['body']) ? $response['body'] : NULL;
					break;
				case 2: // Specific string with keyword(s)
					$target_value = array_get_index($settings, 'validation_kw_replacement', '');
					self::_addFormKeywordReplacements($form, $target_value, $replacements);
					replace_keywords($target_value, $replacements);
					replace_global_keywords($target_value);
					log_dump($target_value);
					break;
				default:
					$target_value = NULL;
			}

			// find if the target value satisfies the validation rule
			if (!is_null($target_value) && strlen(trim($operator))) {
				switch ($operator) {
					case 0: // match
						// Escape the regex delimiter char
						$value = str_replace('/', '\/', $value);
						$pass = preg_match('/'.$value.'/', $target_value);
						break;
					case 1: // not match
						// Escape the regex delimiter char
						$value = str_replace('/', '\/', $value);
						$pass = !preg_match('/'.$value.'/', $target_value);
						break;
					case 2: // greater than
						$pass = $target_value > $value;
						break;
					case 3: // less than
						$pass = $target_value < $value;
						break;
					default:
						$pass = FALSE;
				}
			} else {
				$pass = FALSE;
			}

			if (!$pass) $failed_URLs[] = $response['info']['url'];

			// if the scope is all targets ($scope == 0) every response will need to be checked but
			// if the scope is at least one target ($scope == 1) and found this response passed the validation
			// then there is no need to iterate the rest as we already know the validation result
			if (($scope == 1 && $pass)) {
				$failed_URLs = Array();
				break;
			}

			// For "string with keyword" we dont need to go through each response
			// as the keyword replacement value will be same for all the response(s)
			if ($target == 2 && $pass) {
				$failed_URLs = Array();
				break;
			}
		}

		return $failed_URLs;
	}


	/**
	* Extract the keywords and get the form specific keyword replacements
	*
	* @param object $form			The form asset
	* @param string $text			Keywords to extract from
	* @param array  &$replcements	Already evaluted keyword replacements
	*								The new keywords evaluted will be appended to this array
	*
	* @return void
	* @access private
	*/
	private static function _addFormKeywordReplacements($form, $text, &$replacements)
	{

		$keywords = retrieve_keywords_replacements($text);
		if (!empty($keywords)) {
			foreach($keywords as $keyword) {
				if (!isset($replacements[$keyword])) {
					$replacement = $form->_getThankYouKeywordReplacement($keyword);
					if ($replacement == '%'.$keyword.'%' && strpos($keyword, 'globals_') !== 0) {
						$replacement = '';
					}
					$replacements[$keyword] = $replacement;
				}//end if
			}//end foreach
		}//end if

	}//end _addFormKeywordReplacements()


}//end class

?>
