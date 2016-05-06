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
* $Id: form_action_soap_call.inc,v 1.31 2013/09/10 03:15:04 ewang Exp $
*
*/


require_once SQ_PACKAGES_PATH.'/cms/form/form_action/form_action.inc';
require_once SQ_FUDGE_PATH.'/general/text.inc';
require_once dirname(dirname(dirname(__FILE__))).'/lib/soap_common.inc';
require_once dirname(dirname(dirname(__FILE__))).'/lib/soap_form_based_common.inc';


/**
* Form_Action_SOAP_Call
*
* Purpose
* Make a SOAP call before submitting form
*
* @author  Edison Wang <ewang@squiz.com.au>
* @version $Revision: 1.31 $
* @package MySource_Matrix_Packages
* @subpackage web_service
*/


class Form_Action_SOAP_Call extends Form_Action
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
	*
	*/
	public static function paintInlineInterface(Form $form, $settings, Backend_Outputter $o, $prefix)
	{
		$auth_types = Soap_Common::$auth_types;

		$selected_auth_type = array_get_index($settings, 'auth_type', 'HTTP_BASIC');
		$selected_caching_type  = array_get_index($settings, 'cache_wsdl', WSDL_CACHE_BOTH);
		$selected_service_address  = array_get_index($settings, 'service_address', NULL);

		$auth_paint_fn = 'paint'.str_replace(' ', '', ucwords(str_replace('_', ' ', strtolower($selected_auth_type)))).'Fields';
		?>
		
		<p class="sq-backend-section-subheading">Connection Details</p>
		<div id="<?php echo $prefix ?>_connection_div">
			<table class="sq-backend-table" >
				<colgroup>
					<col width="20%" />
					<col width="80%" />
				</colgroup>
				<tr>
					<th><div style="min-width:200px"><?php echo translate('WSDL'); ?></div></th>
					<td><?php text_box($prefix.'_wsdl', array_get_index($settings, 'wsdl', ''), 120,'',FALSE, ' style="width: 100%;box-sizing: border-box;height: 24px;"'); ?> </td>
				</tr>
				<tr>
					<th><?php echo translate('Auth Type'); ?></p></th>
					<td><?php combo_box($prefix.'_auth_type', $auth_types, FALSE, $selected_auth_type); ?></td>
				</tr>
				<?php
					// Paint function authentication fields
					Soap_Common::paintSoapAuthenticationFields($prefix, $settings, $o, TRUE);
				?>
			</table>
		</div>

		<div id="<?php echo $prefix ?>_https_div">
			<table class="sq-backend-table" >
				<colgroup>
					<col width="20%" />
					<col width="80%" />
				</colgroup>
				<tr>
					<th style='min-width:200'><?php echo translate('Local Certificate'); ?></th>

					<td><?php asset_finder($prefix.'_local_cert', array_get_index($settings, 'local_cert', ''), Array('file'=>'D')); ?>
				</tr>
				<tr>
					<th><p><?php echo translate('Passphrase'); ?></p></th>

					<td><?php password_box($prefix.'_passphrase', '', 35); ?></td>
				</tr>
				<tr>
					<td colspan="2"><?php echo translate('For HTTPS client certificate authentication use local certificate and passphrase options.'); ?></td>

				</tr>
			</table>
		</div>

		<p class="sq-backend-section-subheading"><?php echo translate('Additional SOAP Headers'); ?></p>
		<div id="<?php echo $prefix ?>_additional_soap_header">
			<?php Soap_Common::paintAdditionalSOAPHeader($prefix.'_additional_soap_headers', $settings,true, $o, true)?>
		</div>

		<p class="sq-backend-section-subheading"><?php echo translate('SOAP Options'); ?></p>
		<div id="<?php echo $prefix ?>_https_div" >
			<table class="sq-backend-table">
			<colgroup>
					<col width="20%" />
					<col width="80%" />
			</colgroup>
				<tr>
					<th style='min-width=200px'><?php echo translate('Cache WSDL'); ?></div></th>
					<td><?php combo_box($prefix.'_cache_wsdl', SOAP_Common::$wsdl_caching_opts, FALSE, $selected_caching_type); ?></td>
				</tr>
				<tr>
					<th><?php echo translate('Mute SOAP Exceptions'); ?></th>

					<td><?php check_box($prefix.'_mute_exceptions', 1, array_get_index($settings, 'mute_exceptions', 0)); ?></td>
				</tr>
				<tr>
					<th><?php echo translate('Service Address'); ?></th>

					<td><?php combo_box($prefix.'_service_address', array_get_index($settings, 'available_service_address', ''), FALSE, $selected_service_address); ?></td>
				</tr>
			</table>
		</div>


		<p class="sq-backend-section-subheading">Test Response</p>
		<div id="<?php echo $prefix ?>_test_request_response" >
			<table class="sq-backend-table" >
			<colgroup>
					<col width="20%" />
					<col width="80%" />
			</colgroup>
				<tr>
					<th style='min-width:200px'><?php echo translate('Run Test'); ?></th>
					<td>
					<?php
						$run_test = array_get_index($settings, 'run_test', 0);
						check_box($prefix.'_run_test', 1, $run_test);
					?>
					</td>
				</tr>
				<tr>
					<td colspan="2">
					<?php echo translate('The test will be based on the request content in the "SOAP Request Message" below'); ?>
					</td>
				</tr>
			</table>
		</div>

		<div>
			<?php
				if ($run_test && !empty($settings['test_results'])) {
					foreach($settings['test_results'] as $soap_func => $info) {
						echo "<b><u>$soap_func</u></b><br /><br />";
						echo "<b>Request:</b><br />";
						echo (nl2br(str_replace(' ', '&nbsp;&nbsp;', htmlentities($info['request']))));
						echo "<br />";
						echo "<b>Response:</b><br />";
						echo (nl2br(str_replace(' ', '&nbsp;&nbsp;', htmlentities($info['response']))));
						echo "<br />";

					}
					echo "<br />";
				}
			?>
		</div>

	<?php if(!isset($settings['actions']) || empty($settings['actions'])) return;?>

		<p class="sq-backend-section-subheading"><?php echo translate('SOAP Request Message'); ?></p>


		<div id="<?php echo $prefix ?>_action_selection" >
			<table class="sq-backend-table" >
				<tr>
					<td><strong><?php echo translate('Action');?></strong><br /><br /></td>

					<td>
						<?php
							ksort($settings['actions']);
							foreach ($settings['actions'] as $name => $detail) {
								$action_list[$name] = $name;
							}
							combo_box($prefix.'_action', $action_list, FALSE, array_get_index($settings, 'action', ''));
						?>
						<br /><br />
					</td>
				</tr>
				<tr>
					<td><span style="padding-top:10px;"><strong><?php echo translate('Parameters');?></strong></span></td>

					<td>
						<?php
							$i = 0;
							if(isset($settings['actions'][$settings['action']])) {
								foreach ($settings['actions'][$settings['action']] as $parameter_xml) {
									text_area($prefix.'_'.$settings['action'].'parameters'.$i, array_get_index($settings,$settings['action'].'parameters'.$i, $parameter_xml), 80, 15);
									echo '<br/>';
									$i++;
								}
							}
						?>
						<br />
					</td>
				</tr>
				<tr>
					<td colspan="2"><?php echo translate('Question keywords can be used in the message, for example, &#37;response_100_q1&#37;, &#37;question_name_100_q1&#37;, &#37;file_upload_100_q2&#37;'); ?></td>

				</tr>
			</table>
		</div>

		<p class="sq-backend-section-subheading"><?php echo translate('SOAP Response Message'); ?></p>


		<div id="<?php echo $prefix ?>_response" >
			<table class="sq-backend-table" >
				<tr>
					<td>
						<?php
							$i = 0;
							if(isset($settings['responses'][$settings['action']])) {
								// echo the response xml, with style
								echo (nl2br(str_replace(' ', '&nbsp;&nbsp;', htmlentities(str_replace('[ARRAY_ELEMENT]', '', $settings['responses'][$settings['action']])))));
							}
						?>
						<br />
					</td>
				</tr>
				<tr>
					<td>
						<?php echo translate('Response will be stored in global session vars.They can be printed using keywords as below.'); ?>

					</td>
				</tr>
				<tr>
				</tr>
			</table>
		</div>

		<p class="sq-backend-section-subheading">Check Response</p>

		<div id="<?php echo $prefix ?>_response_error" >
			<table class="sq-backend-table" >
				<tr width="120">
					<td>
						<?php
						$selected_fields = isset($settings[$settings['action'].'_response_error_fields']) ? $settings[$settings['action'].'_response_error_fields'] : Array();
						if (empty($selected_fields)) {
							echo "No SOAP response fields selected";
						} else {
							?>
							<table class="sq-backend-table">
								<tr>
									<td class="sq-backend-table-header"><?php echo translate('Field Name'); ?></td>

									<td class="sq-backend-table-header"><?php echo translate('Error Text'); ?></td>

									<td class="sq-backend-table-header"><?php echo translate('Delete?'); ?></td>

								</tr>
								<?php
								$rules = SOAP_Form_Based_Common::validationRules();
								foreach ($selected_fields as $selected_field => $rule) {
									$error_msg =  array_get_index($rule, 'error_msg', '');
									$rule_value = array_get_index($rule, 'value', '');
									$rule_type = array_get_index($rule, 'type', 'must_contain');;

									$sf_prefix = $prefix.'_'.$settings['action'].'_'.$selected_field;
									?>
									<tr>
										<td class="sq-backend-table-cell"><?php echo $selected_field; ?></td>
										<td class="sq-backend-table-cell">
											<table>
												<tr>
													<td>Rule</td>
													<td><?php
														combo_box($sf_prefix.'_type', $rules, FALSE, $rule_type);
														?>
													</td>
												</tr>
												<tr>
													<td>Value</td><td><?php text_box($sf_prefix.'_value', $rule_value, 40); ?></td>
												</tr>
												<tr>
													<td>Error Message</td><td><?php text_box($sf_prefix.'_error_msg', $error_msg, 110); ?></td>
												</tr>
											</table>
										</td>

										<td class="sq-backend-table-cell">
											<?php check_box($sf_prefix.'_delete_field['.$selected_field.']'); ?>
										</td>
									</tr>
									<?php
								}//end foreach
								?>
							</table>
							<?php
						}//end else
						?>
						<br />
						<?php
							echo translate('New field to check?');

							$response_fields = Array('' => '-- Select Field --');
							if (isset($settings['response_keywords'][$settings['action']])) {
								$response_keywords = explode('<br/>', $settings['response_keywords'][$settings['action']]);
								foreach($response_keywords as $val) {
									$val = trim($val, '%');
									$val = substr_replace($val, '', 0,  strlen('globals_session_soap_'));
									if(empty($val)) $val = 'Response';
									if ($val) {
										$response_fields[$val] = $val;
									}
								}//end foreach
							}
							combo_box($prefix.'_'.$settings['action'].'_response_error_field', $response_fields, FALSE);
						?>
					</td>
				</tr>

				<tr>
					<td><?php translate('If the specified field fails the validation, the step action will treat it as an regular form error.'); ?></td>

				</tr>
			</table>
		</div>

		<p class="sq-backend-section-subheading"><?php echo translate('Keywords Available'); ?></p>


		<div id="<?php echo $prefix ?>_response_keywords" >
			<table class="sq-backend-table" >
				<tr>
					<td>
						<?php
							if(isset($settings['response_keywords'][$settings['action']])) {
								// echo the response xml, with style
								echo ($settings['response_keywords'][$settings['action']]);
							}
						?>
						<br />
					</td>
				</tr>
				<tr>
					<td><?php echo translate('Note: This list only shows the keywords for the first set of data for the array elements in the soap response, if any'); ?></td>

				</tr>
			</table>
			<p class="sq-backend-section-subheading"><?php echo translate('Response to download as file'); ?></p>

			<table class="sq-backend-table">
				<tr>
					<td class="sq-backend-table-header"><?php echo translate('Field Keyword (Tag)'); ?></td>

				</tr>
				<?php
					$response_to_file = array_get_index($settings, $settings['action'].'_file_response_to_save', Array());
					$currently_configured = count($response_to_file);

					foreach ($response_to_file as $i => $response) { ?>
				<tr>
					<td>
					<?php
						text_box($prefix.'_'.$settings['action'].'_file_response_to_save['.$i.']', $response, 50);

						if (in_array($response, $settings[$settings['action'].'_invalid_keywords_for_file'])) {
							// let the user know if one of the keywords entered was invalid
							echo "<span style=\"color:#FF0000\"><b> *invalid keyword</b></span>";
						}
					?>
					</td>
				</tr>
				<?php } ?>
				<tr>
					<td>
					<?php
						$count = $currently_configured++;
						text_box($prefix.'_'.$settings['action'].'_file_response_to_save['.$count.']', '', 50);
					?>
					</td>
				</tr>
				<tr>
					<td><?php echo translate('If a file is expected in the SOAP Response, you can configure the keywords above. This will ensure the file is temporary downloaded on the server and a relative link is provided to view that file in the replacement for above keyword.'); ?></td>

				</tr>
			</table>
		</div>

		<p class="sq-backend-section-subheading">Log</p>
		<div id="<?php echo $prefix ?>_log" >
			<table class="sq-backend-table" >
				<tr>
					<td width="120"><?php echo translate('Enable Log');?></td>
					<td><p>
							<?php
							$run_test = array_get_index($settings, 'log_required', 0);
							check_box($prefix.'_log_required', 1, $run_test);
							?>
						</p></td>
				</tr>
				<tr>
					<td><?php echo translate('Location');?></td>
					<td>
						<?php
							$location = array_get_index($settings, 'log_location', '');
							text_box($prefix.'_log_location', $location, 120);

							if(strpos($location, '/') !== 0 && !empty($location)) {
								$location = SQ_SYSTEM_ROOT.'/'.$location;
							}
							if ($run_test)
							{
								if (empty($location)) {
									?><br />
									<span class="sq-backend-warning"><?php echo translate('Location is required'); ?></span>

									<?php
								} else if (!is_dir($location)) {
									?><br />
									<span class="sq-backend-warning"><?php echo translate('Location does not exist or is not a valid directory.'); ?></span>

									<?php
								} else if (!authorised_file_write_path($location)) {
									?><br />
									<span class="sq-backend-warning"><?php echo translate('Location is not allowed. Please check Authorised File Write Paths setting in External Tools Configuration (i.e SQ_TOOL_AUTHORISED_FILE_WRITE_PATHS).'); ?></span>

									<?php

								} else if (!is_writable($location)) {
									?><br />
									<span class="sq-backend-warning"><?php echo translate('Location is not writable'); ?></span>

									<?php
								}
							}//end if
							?>
						<div class="sq-backend-smallprint"><?php echo translate('Path is relative to Matrix installation location unless prefixed with \'/\'. Path must be allowed in Authorised File Write Paths setting in External Tools Configuration(i.e SQ_TOOL_AUTHORISED_FILE_WRITE_PATHS)');?></div>

					</td>

				</tr>
				<tr>
					<td><?php echo translate('Log Name');?></td>
					<td>

						<?php
							$log_name = array_get_index($settings, 'log_name', '');
							text_box($prefix.'_log_name', $log_name, 120);

							if ($run_test && empty($log_name)) {
								?><br/>
								<span class="sq-backend-warning"><?php echo translate('Log Name is required'); ?></span>
								<?php
							}
						?>

						<div class="sq-backend-smallprint"><?php echo translate('Global Keywords are supported.');?></div>


					</td>
				</tr>
				<tr>
					<td><?php echo translate('Log Format');?></td>
					<td>
						<?php text_box($prefix.'_log_format', array_get_index($settings, 'log_format', ''), 120); ?>
						<div class="sq-backend-smallprint"><?php echo translate('Global Keywords and Question Keywords are supported, for example, &#37;response_100_q1&#37;, &#37;question_name_100_q1&#37;.');?></div>
					</td>
				</tr>
			</table>
		</div>

		<?php

	}//end paintInlineInterface()


	/**
	* Process inline interface
	*
	* @return boolean
	* @access public
	*/
	public static function processInlineInterface(Form $form, &$settings, Backend_Outputter $o, $prefix)
	{
		$settings['local_cert']				= isset($_POST[$prefix.'_local_cert']['assetid']) ? $_POST[$prefix.'_local_cert']['assetid']: NULL;
		$settings['wsdl']					= array_get_index($_POST, $prefix.'_wsdl', NULL);
		$settings['mute_exceptions']		= array_get_index($_POST, $prefix.'_mute_exceptions', NULL);
		$settings['run_test'] 				= array_get_index($_POST, $prefix.'_run_test', NULL);
		if(isset($_POST[$prefix.'_passphrase']) && !empty($_POST[$prefix.'_passphrase'])) {
			$settings['passphrase']				= array_get_index($_POST, $prefix.'_passphrase', NULL);
		}
		$settings['action']					= array_get_index($_POST, $prefix.'_action', NULL);
		$settings['auth_type']				= array_get_index($_POST, $prefix.'_auth_type', NULL);
		$settings['cache_wsdl']				= array_get_index($_POST, $prefix.'_cache_wsdl', NULL);
		$settings['service_address']		= array_get_index($_POST, $prefix.'_service_address', NULL);
		$settings['log_required']			= array_get_index($_POST, $prefix.'_log_required', 0);
		$settings['log_location']			= trim(array_get_index($_POST, $prefix.'_log_location', NULL));
		$settings['log_name']				= trim(array_get_index($_POST, $prefix.'_log_name', NULL));
		$settings['log_format']				= trim(array_get_index($_POST, $prefix.'_log_format', NULL));


		// Process function authentication fields
		Soap_Common::processSoapAuthenticationFields($prefix, $settings, $o);

		SOAP_Common::processAdditionalSOAPHeader($prefix, $settings, $o);

		// This submission needs to be executed before form get submitted
		$settings['before_create']	= TRUE;

		if(isset ($settings['wsdl']) && !empty($settings['wsdl'])) {

			$options = SOAP_Common::_getSoapOptions($settings, $form);
			try {
				$client = SOAP_Common::getSoapClient($settings, $options, $form);
				if (!is_null($client)) {
					$functions = $client->__getFunctions();
					$types = $client->__getTypes();
				}
			}
			catch (Exception $e) {
				trigger_error($e->getMessage());
				return TRUE;
			}
			if(!empty($functions) && !isset($settings['run_test'])) {
				foreach ($functions as $function) {
					$function_name = preg_replace('/^\w+ (\w+)\(.*\)/', '$1', $function);
					$function_params = explode(', ', preg_replace('/^\w+ \w+\((.*)\)/', '$1', $function));
					$function_response = preg_replace('/^(\w+) \w+\(.*\)/', '$1', $function);

					// Fixed duplicate function definition issue of PHP SoapClient
					if(isset($function_list[$function_name])) continue;

					// parse parameters
					$parsed_params = Array();
					foreach ($function_params as $param) {
						if (!trim($param)) continue;

						// Get XML representation of params of the functions
						$xml_param = SOAP_Common::_parse_type ($param, $types);
						$parsed_params[$param] = SOAP_Common::_formatXmlString(empty($xml_param) ? $param : $xml_param->asXML());
					}
					$function_list[$function_name] = $parsed_params;

					// parse response
					$xml_response = SOAP_Common::_parse_type ($function_response, $types);
					// Issue in parse type
					if ($xml_response === TRUE || $xml_response === FALSE) {
						$response_list[$function_name] = SOAP_Common::_formatXmlString('<?xml version=\"1.0\"?'.'>'."<$function_name>??</$function_name>");
						$keywords = '%globals_session_soap'.($xml_response ? '_0' : '').'%';
					} else {
						$response_list[$function_name] = SOAP_Common::_formatXmlString(empty($xml_response) ? '' : $xml_response->asXML());
						$keywords = SOAP_Common::_parse_response_keywords ($xml_response, 'globals_session_soap_');
					}
					// parse response keywords
					$keywords_list[$function_name] = str_replace('[ARRAY_ELEMENT]', '0', $keywords);
				}
				$settings['actions'] = $function_list;
				$settings['responses'] = $response_list;
				$settings['response_keywords'] = $keywords_list;
			}
		}

		// grab the specified address from WSDL
		$settings['available_service_address'] = SOAP_Common::getAvailableServiceAddress($settings['wsdl'], $settings['login'], empty($settings['password']) ? '' : $settings['password']);


		// Currently selected "Check response" fields
		$current_selected_fields = isset($settings[$settings['action'].'_response_error_fields']) ? $settings[$settings['action'].'_response_error_fields'] : Array();
		$field_rule = Array();

		// String containg all the SOAP response keywords
		$keywords_string = '';
		if (isset($settings['response_keywords'])) {
			$keywords_string = implode(';',$settings['response_keywords']);
		}

		foreach ($current_selected_fields as $selected_field => $error_msg) {
			$sf_prefix = $prefix.'_'.$settings['action'].'_'.$selected_field;

			// If the field is deleted or dosen't exists in SOAP response anymore
			if ((!isset($_POST[$sf_prefix.'_delete_field'][$selected_field]) || !$_POST[$sf_prefix.'_delete_field'][$selected_field])) {
				if (isset($_POST[$sf_prefix.'_error_msg'])) {
					$field_rule[$selected_field] = Array(
													'type'	=> $_POST[$sf_prefix.'_type'],
													'value'	=> $_POST[$sf_prefix.'_value'],
													'error_msg' => trim($_POST[$sf_prefix.'_error_msg']),
												);
				} else {
					$field_rule[$selected_field] = $rule;
				}//end else
			}//end if
		}//end foreach

		// Add new "Check response" field
		if (isset($_POST[$prefix.'_'.$settings['action'].'_response_error_field']) && !empty($_POST[$prefix.'_'.$settings['action'].'_response_error_field'])) {
			$new_field = $_POST[$prefix.'_'.$settings['action'].'_response_error_field'];
			$field_rule[$new_field] = Array();
		}

		// Set up current params for current action
		if(isset($settings['actions'][$settings['action']])) {
			$settings[$settings['action'].'_response_error_fields'] = $field_rule;
			for($i = 0; $i < count($settings['actions'][$settings['action']]); $i++) {
				if(isset($_POST[$prefix.'_'.$settings['action'].'parameters'.$i]))
					$settings[$settings['action'].'parameters'.$i] = $_POST[$prefix.'_'.$settings['action'].'parameters'.$i];
			}
		}

		$settings[$settings['action'].'_invalid_keywords_for_file'] = Array();
		if (isset($settings['actions'][$settings['action']])) {
			$save_file_response  = array_get_index($_POST, $prefix.'_'.$settings['action'].'_file_response_to_save', Array());
			foreach ($save_file_response as $index => $response_kwd) {
				// see if the keyword mentioned here is blnk or not from current response
				// if so remove it and also let user know of any invalid keywords put in
				if (trim($response_kwd) == '') {
					unset($save_file_response[$index]);
					continue;
				}

				if (strpos($settings['response_keywords'][$settings['action']], $response_kwd) === FALSE) {
					$settings[$settings['action'].'_invalid_keywords_for_file'][] = $response_kwd;
				}
			}
			$settings[$settings['action'].'_file_response_to_save'] = $save_file_response;
		}

		// Run test request
		$test_results = Array();
		if ($settings['run_test']) {
			foreach($functions as $function) {
				$function_name = preg_replace('/^\w+ (\w+)\(.*\)/', '$1', $function);
				if($function_name != $settings['action']) continue;
				$test_results[$function_name] = SOAP_Common::testSoapRequest($client, $settings, $function_name, $form);
			}
		}//end if
		$settings['test_results'] = $test_results;

		return TRUE;

	}//end processInlineInterface()


	/**
	* Paint summary description
	*
	* @return void
	* @access public
	*/
	public static function paintSummary(Form $form, $settings, Backend_Outputter $o, $prefix)
	{

		?><table class="no-borders">
			<colgroup>
				<col width="80" />
				<col/>
			</colgroup>
			<tbody>
				<tr>
					<td class="sq-backend-table-cell" style="vertical-align: top"><p><strong><?php echo translate('Action');?></strong></p></td>

					<td class="sq-backend-table-cell" style="vertical-align: top"><p><?php
					if (isset ($settings['action'])) {
						echo $settings['action'];
					} else {
						?><span class="sq-backend-warning"><?php echo translate('No SOAP action specified.');?></span><?php

					}

					if (!isset ($settings[$settings['action'].'parameters0'])) {
						?><br/><span class="sq-backend-warning"><?php echo translate('Request parameters not set');?></span><?php

					}
					?></p></td>
				</tr>
			</tbody>
		</table>
		<?php

	}//end paintSummary()


	/**
	* Execute form action
	*
	* @param
	* @param
	*
	* @return boolean
	* @access public
	*/
	public static function execute(Form $form, $settings)
	{
		$success = TRUE;

		if(!isset($form->submission_errors)) {
			$form->submission_errors = Array();
		}

		if(!isset($settings['actions'][$settings['action']])) {
			$form->submission_errors = array_merge($form->submission_errors, Array(translate('No SOAP action specified.')));

			return FALSE;
		}
		if(!isset($settings['wsdl'])) {
			$form->submission_errors = array_merge($form->submission_errors, Array(translate('SOAP WSDL not configured.')));

			return FALSE;
		}

		$function = $settings['action'];
		$wsdl = $settings['wsdl'];
		$options = SOAP_Common::_getSoapOptions($settings, $form);

		// Work out the parameters of that soap function
		for($i = 0, $param = Array(); $i < count($settings['actions'][$settings['action']]); $i++) {
			if(isset($settings[$settings['action'].'parameters'.$i])) {

				$soap_message = SOAP_Form_Based_Common::getKeywordReplacementsFromForm($settings[$settings['action'].'parameters'.$i], $form);
				$dom = new DOMDocument();
				// use DOMDocument::loadXML() to see if there is any entity  reference loop
				// which may exhaust us of the memory limit, if not then continue as normal
				if ($dom->loadXML($soap_message, LIBXML_NOENT|LIBXML_PARSEHUGE)) {
					$object = @simplexml_import_dom($dom);
				} else {
					// don't worry about any warning, if we are here then PHP has already raised the warning for us
					return;
				}

				if($object === FALSE) {
					// Just string argument, replace file content keywords
					$object = SOAP_Form_Based_Common::getFileContentReplacementsFromForm($soap_message, $form);
				} else {
					// Convert XML to array, replace file content keywords, xml is not friendly for binary data
					$object = SOAP_Form_Based_Common::toArray($object, $form);
				}

				$param[] = $object;
			}
		}

		// Get an array of parameters
		$eval_string = '';
		foreach ($param as $index => $object) {
			$eval_string .= $index == 0 ? '' : ', ';
			$eval_string .= '$param['.$index.']';
		}

		// Enable backtrace option
		$options['trace'] = 1;

		// If SOAP exceptions are to be silenced
		if ($settings['mute_exceptions']) {
			$options['exceptions'] = 0;
		}

		// Make SOAP call
		try {
			$client = SOAP_Common::getSoapClient($settings, $options, $form);
			eval('$data = $client->$function('.$eval_string.');');
		}
		catch (Exception $e) {
			// SOAP error should be treated as normal form error
			$form->submission_errors = array_merge($form->submission_errors, Array($e->getMessage()));
			return FALSE;
		}

		if ($settings['mute_exceptions'] && get_class($data) == 'SoapFault') {
			$soap_response = $client->__getLastResponse();
			$data = NULL;
			// Create the SOAP call return object from last SOAP request
			if (preg_match('|<([^:]+:)?Body>(.*)</([^:]+:)?Body>|msi', $soap_response, $match)) {
				$dom = new DOMDocument();
				// use DOMDocument::loadXML() to see if there is any entity  reference loop
				// which may exhaust us of the memory limit, if not then continue as normal
				if ($dom->loadXML($match[2], LIBXML_NOENT|LIBXML_PARSEHUGE)) {
					$data = simplexml_import_dom($dom);
				} else {
					// don't worry about any warning, if we are here then PHP has already raised the warning for us
					return;
				}
            }
		}

		// Store results in session
		$soap_response_keywords = Array();
		$file_saving_keywords = array_get_index($settings, $settings['action'].'_file_response_to_save', Array());
		SOAP_Common::_store_results($data, 'soap_', $soap_response_keywords, $file_saving_keywords);

		// Remove prefix from response keywords
		foreach($soap_response_keywords as $key => $val) {
			$new_key = substr_replace($key, '', 0,  5);
			if(empty($new_key)) {
				$new_key = 'Response';
			}
			$soap_response_keywords[$new_key] = $val;
			unset($soap_response_keywords[$key]);
		}
		// If the SOAP response has one of the non-empty "Check Response" fields, return error
		if (!empty($settings[$settings['action'].'_response_error_fields'])) {
			foreach($settings[$settings['action'].'_response_error_fields'] as $check_field => $rule) {
				if (!isset($soap_response_keywords[$check_field]) || !SOAP_Form_Based_Common::validateValue($soap_response_keywords[$check_field], $rule)) {
					$form->submission_errors = array_merge($form->submission_errors, Array($rule['error_msg']));
					$success = FALSE;
				}
			}//end foreach
		}//end if


		if (isset($settings['log_required']) && $settings['log_required'])
		{
			$location = array_get_index($settings, 'log_location', '');
			if(strpos($location, '/') !== 0 && !empty($location)) {
				$location = SQ_SYSTEM_ROOT.'/'.$location;
			}//end if

			if (authorised_file_write_path($location) && is_writable($location))
				if (!empty($settings['log_name']))
				{
					$logFile = rtrim($location,'/') .'/'.basename(replace_global_keywords($settings['log_name']));

					if (!empty($settings['log_format']))
					{
						$handle = fopen($logFile, 'a');
						$message = replace_global_keywords(SOAP_Form_Based_Common::getKeywordReplacementsFromForm($settings['log_format'], $form))."\r\n";

						if (!fwrite($handle, $message))
							trigger_localised_error('SYS0051', sprintf(translate('Unable to Write to log file %s with message %s'), $logFile, $message), E_USER_WARNING);

						fclose($handle);
					}//end if
				}//end if
		}//end if

		return $success;

	}//end execute()


	/**
	* Check if the form action is configured correctly
	*
	* @return void
	* @access public
	*/
	public static function isValid(Form $form, $settings)
	{
		// Must have a wsdl
		if (empty($settings['wsdl'])) {
			return FALSE;
		}

		// The SOAP service must have actions
		if (empty($settings['actions'])) {
			return FALSE;
		}//end if

		// The action we try to use must be specified
		if (!isset($settings['action'])) {
			return FALSE;
		}//end if

		// Expect at least one parameter
		if (!isset ($settings[$settings['action'].'parameters0'])) {
			return FALSE;
		}//end if

		return TRUE;

	}//end isValid()


}//end class

?>
