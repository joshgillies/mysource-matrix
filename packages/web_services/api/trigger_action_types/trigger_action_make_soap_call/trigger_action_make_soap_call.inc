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
* $Id: trigger_action_make_soap_call.inc,v 1.15 2013/09/10 03:15:04 ewang Exp $
*
*/

require_once SQ_CORE_PACKAGE_PATH.'/system/triggers/trigger_action/trigger_action.inc';
require_once SQ_FUDGE_PATH.'/general/text.inc';
require_once SQ_INCLUDE_PATH.'/general.inc';
require_once dirname(dirname(dirname(__FILE__))).'/lib/soap_common.inc';


/**
* Trigger_Action_Make_SOAP_Call
*
*
* @author  Ash Karelia <akarelia@squiz.com.au>
* @version $Revision: 1.15 $
* @package MySource_Matrix_Packages
* @subpackage __core__
*/
class Trigger_Action_Make_SOAP_Call extends Trigger_Action
{


	/**
	* Execute this action and stores the processed call's outvcome in global session vars
	*
	* @param array	$settings	the stored settings for this action
	* @param array	&$state		the state that describes the asset the action is executing on
	*
	* @return mixed boolean
	* @access public
	*/
	public static function execute($settings, &$state)
	{
		$success = TRUE;

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

		if(!isset($settings['actions'][$settings['action']])) {
			return FALSE;
		}

		if (empty($state['asset'])) {
			// grab the asset if assetid is given, but not the asset.
			if (empty($state['assetid'])) {
				return FALSE;
			} else {
				$state['asset'] = &$GLOBALS['SQ_SYSTEM']->am->getAsset($state['assetid']);
			}
		}

		$function = $settings['action'];
		$wsdl = $settings['wsdl'];
		$options = SOAP_Common::_getSoapOptions($settings);

		// Work out the parameters of that soap function
		for($i = 0, $param = Array(); $i < count($settings['actions'][$settings['action']]); $i++) {
			if(isset($settings[$settings['action'].'parameters'.$i])) {
				$soap_message = self::getKeywordReplacementsFromAsset($settings[$settings['action'].'parameters'.$i], $state['asset']);
				$dom = new DOMDocument();
				// use DOMDocument::loadXML() to see if there is any entity  reference loop
				// which may exhaust us of the memory limit, if not then continue as normal
				if ($dom->loadXML($soap_message, LIBXML_NOENT)) {
					$object = @simplexml_import_dom($dom);
				} else {
					// don't worry about any warning, if we are here then PHP has already raised the warning for us
					return;
				}

				// Convert XML to array, replace file content keywords, xml is not friendly for binary data
				$object = SOAP_Common::toArray($object);

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
			$client = SOAP_Common::getSoapClient($settings, $options);
			eval('$data = $client->$function('.$eval_string.');');
		}
		catch (Exception $e) {
			// SOAP error should be treated as normal error
			trigger_error($e->getMessage());
			// also store the error in our global_session_vars
			$_SESSION[SQ_SESSION_SANDBOX_INDEX]['soap_trigger_action_error'] = $e->getMessage();
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
				if ($dom->loadXML($match[2], LIBXML_NOENT)) {
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
			$soap_response_keywords[substr_replace($key, '', 0,  5)] = $val;
			unset($soap_response_keywords[$key]);
		}

		// If the SOAP response has one of the non-empty "Check Response" fields, return error
		if (!empty($settings[$settings['action'].'_response_error_fields'])) {
			foreach($settings[$settings['action'].'_response_error_fields'] as $check_field => $error_msg) {
				if (!empty($soap_response_keywords[$check_field])) {
					$warning_level = $settings[$settings['action'].'_treat_as_fatal'] ? E_USER_ERROR : E_USER_WARNING;
					$_SESSION[SQ_SESSION_SANDBOX_INDEX]['soap_trigger_action_error'] = $error_msg;
					trigger_error($error_msg, $warning_level);

					$success = FALSE;
				}
			}//end foreach
		}//end if

		return $success;

	}//end execute()


	/**
	* Get the HTML editing interface for this action. Returns the string instead of printing it
	*
	* @param array		$settings		settings that this condition saves in processInterface()
	* @param string		$prefix			unique prefix
	* @param boolean	$write_access	determines whether the interface is editable
	*
	* @return boolean
	* @access public
	*/
	public static function getInterface($settings, $prefix, $write_access=FALSE)
	{
		$auth_types = Soap_Common::$auth_types;
		
		$selected_caching_type  = array_get_index($settings, 'cache_wsdl', WSDL_CACHE_BOTH);
		$selected_service_address  = array_get_index($settings, 'service_address', NULL);
		$disabled = $write_access ? '' : 'disabled';
		$prefix_str = md5($prefix);

		ob_start();
		?>

		<p class="sq-backend-section-subheading">Connection Details</p>

			<table class="sq-backend-table with-bottom-margin">
				<tr>
					<th style="width: 130px;">WSDL</th>
					<td>
					<?php
						$wsdl = array_get_index($settings, 'wsdl', '');
						if ($write_access) {
							text_box($prefix.'[wsdl]', $wsdl, 120);
						} else {
							echo $wsdl;
						}
					?></td>
				</tr>
				<tr>
					<th>Auth Type</th>
					<td>
					<?php
						$selected_auth_type = array_get_index($settings, 'auth_type', 'HTTP_BASIC');
						combo_box($prefix.'[auth_type]', $auth_types, FALSE, $selected_auth_type, 0, $write_access ? '' : 'disabled');
					?></td>
				</tr>
				<?php
					// Paint function authentication fields
					$o = new Backend_Outputter();
					Soap_Common::paintSoapAuthenticationFields($prefix, $settings, $o, $write_access, FALSE, TRUE);					
				?>
				<tr>
					<th><?php echo translate('mute_exception'); ?></th>
					<td>
					<?php
						$mute_exp = array_get_index($settings, 'mute_exceptions', 0);
						if ($write_access) {
							check_box($prefix.'[mute_exceptions]', 1, $mute_exp);
						} else {
							$mute_exp_status = ($mute_exp == '1') ? 'tick' : 'cross';
							echo '<img class="sq-icon" width="16" height="16" src="'.sq_web_path('lib').'/web/images/'.$mute_exp_status.'.gif" alt="" style="border: 0;" />';
						}
					?>
					</td>
				</tr>
				<tr>
					<th><?php echo translate('service_address'); ?></th>
					<td><?php combo_box($prefix.'[service_address]', array_get_index($settings, 'available_service_address', ''), FALSE, $selected_service_address, 0, $write_access ? '' : 'disabled'); ?></td>
				</tr>
				<tr>
					<th>Cache WSDL</th>
					<td><?php combo_box($prefix.'[cache_wsdl]', SOAP_Common::$wsdl_caching_opts, FALSE, $selected_caching_type, 0, $write_access ? '' : 'disabled'); ?></td>
				</tr>
				<tr>
					<th><?php echo translate('local_cert'); ?></th>
					<td>
					<?php
						$local_cert = array_get_index($settings, 'local_cert', 0);
						if ($write_access) {
							asset_finder($prefix.'[local_cert]', $local_cert, Array('file'=>'D'));
						} else {
							if ($local_cert) echo  get_asset_tag_line($local_cert);
						}
					?>
					</td>
				</tr>
				<tr>
					<th><?php echo translate('passphrase'); ?></th>
					<td>
					<?php
						if ($write_access) password_box($prefix.'[passphrase]', array_get_index($settings, 'passphrase', ''), 35);
					?>
					</td>
				</tr>
				<tr>
					<td colspan="2"><?php echo translate('use_https'); ?></td>
				</tr>
			</table>

		<p class="sq-backend-section-subheading">Test Response</p>
		<div id="<?php echo $prefix ?>_test_request_response">
			<table class="sq-backend-table" >
				<tr>
					<th style="width: 130px;"><?php echo translate('run_test'); ?></th>
					<td>
					<?php
						$run_test = array_get_index($settings, 'run_test', 0);
						if ($write_access) {
							check_box($prefix.'[run_test]', 1, $run_test);
						} else {
							$run_test_status = ($run_test == '1') ? 'tick' : 'cross';
							echo '<img class="sq-icon" width="16" height="16" src="'.sq_web_path('lib').'/web/images/'.$run_test_status.'.gif" alt="" />';
						}
					?>
					</td>
				</tr>
				<tr>
					<td colspan="2"><?php echo translate('run_test_note'); ?>.</td>
				</tr>
			</table>
		</div>

		<div>
			<?php
				if ($write_access && $run_test && !empty($settings['test_results'])) {
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


		<?php if (isset($settings['actions']) && !empty($settings['actions'])) { ?>
			<p class="sq-backend-section-subheading"><?php echo translate('request_message'); ?></p>

				<table class="sq-backend-table with-bottom-margin" >
					<tr>
						<th><?php echo translate('action');?></strong></th>
						<td>
							<?php
								$action_selected = array_get_index($settings, 'action', '');
								foreach ($settings['actions'] as $name => $detail) {
									$action_list[$name] = $name;
								}
								if ($write_access) {
									combo_box($prefix.'[action]', $action_list, FALSE, $action_selected);
								} else {
									echo $action_selected;
								}
							?>
							<br /><br />
						</td>
					</tr>
					<tr>
						<th><?php echo translate('params');?></th>
						<td>
							<?php
								$i = 0;
								if(isset($settings['actions'][$settings['action']])) {
									foreach ($settings['actions'][$settings['action']] as $parameter_xml) {
										if ($write_access) text_area($prefix.'['.$settings['action'].'parameters'.$i.']', array_get_index($settings,$settings['action'].'parameters'.$i, $parameter_xml), 80, 15);
										echo '<br/>';
										$i++;
									}
								}
							?>
							<br />
						</td>
					</tr>
					<tr>
						<td colspan="2">
							<?php
								echo translate('keyword_usable_note');
							?>
						</td>
					</tr>
				</table>

			<p class="sq-backend-section-subheading"><?php echo translate('response_message'); ?></p>

				<table class="sq-backend-table with-bottom-margin" >
					<tr>
						<td>
							<?php
								$i = 0;
								if(isset($settings['responses'][$settings['action']])) {
									echo (nl2br(str_replace(' ', '&nbsp;&nbsp;', htmlentities($settings['responses'][$settings['action']]))));
								}
							?>
							<br />
						</td>
					</tr>
					<tr>
						<td>
							<?php echo translate('response_message_note'); ?>
						</td>
					</tr>
					<tr>
					</tr>
				</table>

				<p class="sq-backend-section-subheading"><?php echo translate('response_keywords'); ?></p>

				<table class="sq-backend-table with-bottom-margin" >
					<tr>
						<td>
							<?php
								$i = 0;
								if(isset($settings['response_keywords'][$settings['action']])) {
									echo ($settings['response_keywords'][$settings['action']]);
								}
							?>
							<br />
						</td>
					</tr>
					<tr>
					</tr>
				</table>

			<p class="sq-backend-section-subheading"><?php echo translate('check_response')?></p>

			<table class="sq-backend-table with-bottom-margin" >
				<tr>
					<td>
						<?php
						$selected_fields = isset($settings[$settings['action'].'_response_error_fields']) ? $settings[$settings['action'].'_response_error_fields'] : Array();
						$selected_field_names = Array();
						if (empty($selected_fields)) {
							echo translate('no_field_selected');
						} else {
							?>
							<table class="sq-backend-table">
								<tr>
									<td class="sq-backend-table-header"><?php echo translate('field_name'); ?></td>
									<td class="sq-backend-table-header"><?php echo translate('error_text'); ?></td>
									<td class="sq-backend-table-header"><?php echo translate('delete_question'); ?></td>
								</tr>
								<?php
								foreach ($selected_fields as $selected_field => $error_msg) {
									?>
									<tr>
										<td class="sq-backend-table-cell">
											<?php
												echo $selected_field;
												$selected_field_names[] = $selected_field;
											?>
										</td>
										<td class="sq-backend-table-cell">
											<?php
												if ($write_access) {
													text_box($prefix.'['.$settings['action'].'_error_msg_'.$selected_field.']', $error_msg, 110);
												} else {
													echo $error_msg;
												}
											?>
										</td>

										<td class="sq-backend-table-cell">
											<?php if ($write_access) check_box($prefix.'['.$settings['action'].'_delete_field_'.$selected_field.']'); ?>
										</td>
									</tr>
									<?php
								}//end foreach
								?>
							</table>
							<?php
						}//end else

						if ($write_access) {
							echo translate('add_new_field');
							$response_fields = Array('' => '-- Select Field --');
							if (isset($settings['response_keywords'][$settings['action']])) {
								$response_keywords = explode('<br/>', $settings['response_keywords'][$settings['action']]);
								foreach($response_keywords as $val) {
									$val = trim($val, '%');
									$val = substr_replace($val, '', 0,  strlen('globals_session_soap_'));

									if ($val) {
										$response_fields[$val] = $val;
									}
								}//end foreach
							}
							combo_box($prefix.'['.$settings['action'].'_response_error_fields'.']', $response_fields, TRUE, $selected_field_names);
						}
						?>
						</td>
					</tr>
					<tr>
						<td>
						<?php
							$treating_fatal = array_get_index($settings, $settings['action'].'_treat_as_fatal', 0);
							if ($write_access) {
								check_box($prefix.'['.$settings['action'].'_treat_as_fatal]', '1', $treating_fatal);
							} else {
								$treat_fatal_status = ($treating_fatal == '1') ? 'tick' : 'cross';
								echo '<img class="sq-icon" width="16" height="16" src="'.sq_web_path('lib').'/web/images/'.$treat_fatal_status.'.gif" alt="" style="border: 0;" />';
							}
							echo translate('treat_as_fatal_note');
						?>
						</td>
					</tr>
					<tr>
						<td>
						<?php
							echo translate('check_response_note');
						?>
						</td>
					</tr>
			</table>

			<p class="sq-backend-section-subheading"><?php echo translate('response_to_download_as_file'); ?></p>
			<table class="sq-backend-table with-bottom-margin">
				<tr>
					<td><?php echo translate('field_keyword_tags'); ?></td>
				</tr>
				<?php
					$response_to_file = array_get_index($settings, $settings['action'].'_file_response_to_save', Array());
					$currently_configured = count($response_to_file);

					foreach ($response_to_file as $i => $response) { ?>
				<tr>
					<td>
					<?php
						if ($write_access) {
							text_box($prefix.'['.$settings['action'].'_file_response_to_save]['.$i.']', $response, 50);

							if (in_array($response, $settings[$settings['action'].'_invalid_keywords_for_file'])) {
								// let the user know if one of the keywords entered was invalid
								echo "<span style=\"color:#FF0000\"><b> *invalid keyword</b></span>";
							}
						} else {
							echo $response;
						}

					?>
					</td>
				</tr>
				<?php } ?>
				<tr>
					<td>
					<?php
						$count = $currently_configured++;
						if ($write_access) text_box($prefix.'['.$settings['action'].'_file_response_to_save]['.$count.']', '', 50);
					?>
					</td>
				</tr>
				<tr>
					<td><?php echo translate('response_to_download_as_file_note'); ?></td>
				</tr>
			</table>
		<?php
		}

		return ob_get_clean();

	}//end getInterface()


	/**
	* Function that handles the conversion of interface to settings
	* together with settings it is expected to populate the hash object
	*
	* @param array	&$settings		a container for any data the action might want to save
	* @param string	$request_data	array of data corresponding to this action as
	*								specified by the $prefix in getInterface.
	*
	* @return boolean
	* @access public
	*/
	public static function processInterface(&$settings, $request_data)
	{
		$settings['wsdl']				= array_get_index($request_data, 'wsdl', '');
		$settings['mute_exceptions']	= array_get_index($request_data, 'mute_exceptions', FALSE);
		$settings['local_cert']			= array_get_index($request_data['local_cert'], 'assetid','0');
		$settings['passphrase']			= array_get_index($request_data, 'passphrase', '');
		$settings['run_test']			= array_get_index($request_data, 'run_test', 0);
		$settings['action']				= array_get_index($request_data, 'action', NULL);
		$settings['auth_type']			= array_get_index($request_data, 'auth_type', NULL);
		$settings['cache_wsdl']			= array_get_index($request_data, 'cache_wsdl', NULL);
		$settings['service_address']	= array_get_index($request_data, 'service_address', NULL);
		
		// Process function authentication fields
		$o = new Backend_Outputter();
		Soap_Common::processSoapAuthenticationFields($request_data, $settings, $o);

		if(isset ($settings['wsdl']) && !empty($settings['wsdl'])) {

			$options = SOAP_Common::_getSoapOptions($settings);

			// if we are asked to use certificate but its not valid, complain !
			if (!$options) {
				trigger_error(taranslate('no_cert'), E_USER_WARNING);
				return TRUE;
			}

			try {
				$client = SOAP_Common::getSoapClient($settings, $options);
				if (!is_null($client)) {
					$functions = $client->__getFunctions();
					$types = $client->__getTypes();
				}
			}
			catch (Exception $e) {
				trigger_error($e->getMessage());
				return TRUE;
			}

			$function_list = Array();
			if(!empty($functions)) {
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

					if ($xml_response === TRUE || $xml_response === FALSE) {
						$response_list[$function_name] = SOAP_Common::_formatXmlString('<?xml version=\"1.0\"?'.'>'."<$function_name>??</$function_name>");
						$keywords = 'globals_session_soap'.($xml_response ? '0' : '');
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
		if(isset($settings['action']))
		    $settings[$settings['action'].'_treat_as_fatal'] = array_get_index($request_data, $request_data['action'].'_treat_as_fatal',0);

		
		// grab the service address from WSDL
		$settings['available_service_address'] = SOAP_Common::getAvailableServiceAddress($settings['wsdl'], $settings['login'], $settings['password']);
		
		
		// Currently selected "Check response" fields
		$current_selected_fields = (isset($request_data[$settings['action'].'_response_error_fields']) && $request_data[$settings['action'].'_response_error_fields'] != '')? $request_data[$settings['action'].'_response_error_fields'] : Array();

		$field_error_msgs = Array();

		// String containg all the SOAP response keywords
		$keywords_string = '';
		if (isset($settings['response_keywords'])) {
			$keywords_string = implode(';',$settings['response_keywords']);
		}

		foreach ($current_selected_fields as $index => $selected_field) {

			// If the field is deleted or dosen't exists in SOAP response anymore
			if ((!isset($request_data[$settings['action'].'_delete_field_'.$selected_field]) || !$request_data[$settings['action'].'_delete_field_'.$selected_field]) && strpos($keywords_string, '%globals_session_soap_'.$selected_field.'%') !== FALSE) {
				if (isset($request_data[$settings['action'].'_error_msg_'.$selected_field])) {
					$field_error_msgs[$selected_field] = trim($request_data[$settings['action'].'_error_msg_'.$selected_field]);
				} else {
					$field_error_msgs[$selected_field] = '';
				}//end else
			}//end if
		}//end foreach

		// Set up current params for current action
		if(isset($settings['actions'][$settings['action']])) {
			$settings[$settings['action'].'_response_error_fields'] = $field_error_msgs;
			for($i = 0; $i < count($settings['actions'][$settings['action']]); $i++) {
				if(isset($request_data[$settings['action'].'parameters'.$i])) {
					$parameters_passed = $request_data[$settings['action'].'parameters'.$i];
					$settings[$settings['action'].'parameters'.$i] = $request_data[$settings['action'].'parameters'.$i];
				}
			}
		}

		$settings[$settings['action'].'_invalid_keywords_for_file'] = Array();
		if (isset($settings['actions'][$settings['action']])) {
			$save_file_response  = array_get_index($request_data, $settings['action'].'_file_response_to_save', Array());
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
		if ($settings['run_test'] && isset($functions)) {
			// run test only for test selected
			foreach($functions as $function) {
				$function_name = preg_replace('/^\w+ (\w+)\(.*\)/', '$1', $function);
				if($function_name != $settings['action']) continue;
				$test_results[$function_name] = SOAP_Common::testSoapRequest($client, $settings, $function_name);
			}
		}//end if
		$settings['test_results'] = $test_results;

		unset($settings['crm_auth_success']);
		if ($settings['auth_type'] == 'JSESSIONID' && $settings['test_crm_auth']) {
			$crm_auth_test = FALSE;
			if (SOAP_Common::getJSessionId($settings['login'], $settings['password'], $settings['login_url'])) {
				$crm_auth_test = TRUE;
			}
			$settings['crm_auth_success'] = $crm_auth_test;
		}

		return FALSE;

	}//end processInterface()


	/**
	* This function replace all the keywords for each of the field
	*
	* @return string
	* @access public
	*/
	public static function getKeywordReplacementsFromAsset($content, &$asset)
	{
		$replacements = Array();

		// replace any global keywords before we go any further
		replace_global_keywords($content);

		$keyword_list = extract_keywords($content);

		if (empty($keyword_list)) return $content;

		foreach ($keyword_list as $keyword) {
			$modified_keyword = parse_keyword($keyword, $modifiers);
			$replacement[$keyword] = $asset->getKeywordReplacement($modified_keyword);
			if (!empty($modifiers)) {
				apply_keyword_modifiers($replacement[$keyword], $modifiers, Array('assetid' => $asset->id));
			}

		}
		if (!empty($replacement)) replace_keywords($content, $replacement);

		return $content;

	}//end getKeywordReplacementsFromAsset()


}//end class

?>
