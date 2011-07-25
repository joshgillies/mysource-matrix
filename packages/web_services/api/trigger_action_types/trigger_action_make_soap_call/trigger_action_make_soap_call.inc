<?php
/**
* +--------------------------------------------------------------------+
* | This MySource Matrix CMS file is Copyright (c) Squiz Pty Ltd       |
* | ACN 084 670 600                                                    |
* +--------------------------------------------------------------------+
* | IMPORTANT: Your use of this Software is subject to the terms of    |
* | the Licence provided in the file licence.txt. If you cannot find   |
* | this file please contact Squiz (www.squiz.net) so we may provide   |
* | you a copy.                                                        |
* +--------------------------------------------------------------------+
*
* $Id: trigger_action_make_soap_call.inc,v 1.5 2011/07/25 00:25:18 cupreti Exp $
*
*/

require_once SQ_CORE_PACKAGE_PATH.'/system/triggers/trigger_action/trigger_action.inc';
require_once SQ_FUDGE_PATH.'/general/text.inc';
require_once SQ_INCLUDE_PATH.'/general.inc';


// WSDL sometime defines a complex type in a loop,
// we need to get out of this loop if it repeats too many times while parsing
define('SQ_MAX_BREAK_OUT_AFTER', 3);

/**
* Trigger_Action_Make_SOAP_Call
*
*
*
* @author  Ash Karelia <akarelia@squiz.com.au>
* @version $Revision: 1.5 $
* @package MySource_Matrix_Packages
* @subpackage __core__
*/
class Trigger_Action_Make_SOAP_Call extends Trigger_Action
{


	private static $recent_parsed_type = Array();
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
		$options = self::_getSoapOptions($settings);

		// Work out the parameters of that soap function
		for($i = 0, $param = Array(); $i < count($settings['actions'][$settings['action']]); $i++) {
			if(isset($settings[$settings['action'].'parameters'.$i])) {
				$soap_message = self::getKeywordReplacementsFromAsset($settings[$settings['action'].'parameters'.$i], $state['asset']);
				$object = @simplexml_load_string($soap_message);

				// Convert XML to array, replace file content keywords, xml is not friendly for binary data
				$object = self::_toArray($object);

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
			$client = new SoapClient($wsdl, $options);
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
				$data = simplexml_load_string($match[2]);
            }
		}

		// Store results in session
		self::_store_results($data, 'soap_', $soap_response_keywords);

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
		ob_start();
		?>

		<p class="sq-backend-section-subheading">Connection Details</p>

			<table class="sq-backend-table" >
				<tr>
					<td width="120"><p>WSDL</p></td>
					<td><p><?php text_box($prefix.'[wsdl]', array_get_index($settings, 'wsdl', ''), 120); ?></p></td>
				</tr>
				<tr>
					<td><?php echo translate('login'); ?></td>
					<td><p><?php text_box($prefix.'[login]', array_get_index($settings, 'login', ''), 35); ?><br />
					</p></td>
				</tr>
				<tr>
					<td><?php echo translate('password'); ?></td>
					<td><p><?php password_box($prefix.'[password]', array_get_index($settings, 'password', ''), 35); ?></p></td>
				</tr>
				<tr>
					<td colspan="2"><?php echo translate('use_http_authentication'); ?></td>
				</tr>
				<tr>
					<td><?php echo translate('mute_exception'); ?></td>
					<td><p><?php check_box($prefix.'[mute_exceptions]', 1, array_get_index($settings, 'mute_exceptions', 0)); ?></p></td>
				</tr>
			</table>

			<table class="sq-backend-table" >
				<tr>
					<td><p><?php echo translate('local_cert'); ?></p></td>
					<td><p><?php asset_finder($prefix.'[local_cert]', array_get_index($settings, 'local_cert', 0), Array('file'=>'D')); ?><br/>
				</tr>
				<tr>
					<td><p><?php echo translate('passphrase'); ?></p></td>
					<td><p><?php password_box($prefix.'[passphrase]', array_get_index($settings, 'passphrase', ''), 35); ?></p></td>
				</tr>
				<tr>
					<td colspan="2"><?php echo translate('use_https'); ?></td>
				</tr>
			</table>
	
		<p class="sq-backend-section-subheading">Test Response</p>
		<div id="<?php echo $prefix ?>_test_request_response" style="padding: 0.5em 0px 2em 30px;">
			<table class="sq-backend-table" >
				<tr>
					<td width="120"><?php echo translate('run_test'); ?></td>
					<td><p>
					<?php
						$run_test = array_get_index($settings, 'run_test', 0);
						check_box($prefix.'[run_test]', 1, $run_test);
					?>
					</p></td>
				</tr>
				<tr>
					<td colspan="2"><?php echo translate('run_test_note'); ?></td>
				</tr>
			</table>
		</div>

		<div width="120">
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


		<?php if (isset($settings['actions']) && !empty($settings['actions'])) { ?>
			<p class="sq-backend-section-subheading"><?php echo translate('request_message'); ?></p>

				<table class="sq-backend-table" >
					<tr>
						<td><strong><?php echo translate('action');?></strong><br /><br /></td>
						<td>
							<?php
								foreach ($settings['actions'] as $name => $detail) {
									$action_list[$name] = $name;
								}
								combo_box($prefix.'[action]', $action_list, FALSE, array_get_index($settings, 'action', ''));
							?>
							<br /><br />
						</td>
					</tr>
					<tr>
						<td><span style="padding-top:10px;"><strong><?php echo translate('params');?></strong></span></td>
						<td>
							<?php
								$i = 0;
								if(isset($settings['actions'][$settings['action']])) {
									foreach ($settings['actions'][$settings['action']] as $parameter_xml) {
										text_area($prefix.'['.$settings['action'].'parameters'.$i.']', array_get_index($settings,$settings['action'].'parameters'.$i, $parameter_xml), 80, 15);
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

				<table class="sq-backend-table" >
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

				<table class="sq-backend-table" >
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

			<div id="<?php echo $prefix ?>_response_error" style="padding: 0.5em 0px 2em 30px;">
				<table class="sq-backend-table" >
					<tr width="120">
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
										<td class="sq-backend-table-header">Field Name</td>
										<td class="sq-backend-table-header">Error Message</td>
										<td class="sq-backend-table-header">Delete ?</td>
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
												<?php text_box($prefix.'['.$settings['action'].'_error_msg_'.$selected_field.']', $error_msg, 110); ?>
											</td>

											<td class="sq-backend-table-cell">
												<?php check_box($prefix.'['.$settings['action'].'_delete_field_'.$selected_field.']'); ?>
											</td>
										</tr>
										<?php
									}//end foreach
									?>
								</table>
								<?php
							}//end else

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
							?>
						</td>
					</tr>
					<tr>
						<td>
							<?php check_box($prefix.'['.$settings['action'].'_treat_as_fatal]', '1', $settings[$settings['action'].'_treat_as_fatal']); 
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
					<tr>
					</tr>
				</table>
			</div>

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
		$settings['login']				= array_get_index($request_data, 'login', '');
		$settings['password']			= array_get_index($request_data, 'password', '');
		$settings['mute_exceptions']	= array_get_index($request_data, 'mute_exceptions', FALSE);
		$settings['local_cert']			= array_get_index($request_data['local_cert'], 'assetid','0');
		$settings['passphrase']			= array_get_index($request_data, 'passphrase', '');
		$settings['run_test']			= array_get_index($request_data, 'run_test', 0);
		$settings['action']				= array_get_index($request_data, 'action', NULL);

		if(isset ($settings['wsdl']) && !empty($settings['wsdl'])) {

			$options = self::_getSoapOptions($settings);

			// if we are asked to use certificate but its not valid, complain !
			if (!$options) {
				trigger_error(taranslate('no_cert'), E_USER_WARNING);
				return TRUE;
			}

			try {
				$client = new SoapClient($settings['wsdl'], $options);
				$functions = $client->__getFunctions();
				$types = $client->__getTypes();
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
						$xml_param = self::_parse_type ($param, $types);
						$parsed_params[$param] = self::_formatXmlString(empty($xml_param) ? $param : $xml_param->asXML());
					}
					$function_list[$function_name] = $parsed_params;

					// parse response
					$xml_response = self::_parse_type ($function_response, $types);
					if ($xml_response === TRUE || $xml_response === FALSE) {
						$response_list[$function_name] = self::_formatXmlString('<?xml version=\"1.0\"?'.'>'."<$function_name>??</$function_name>");						
						$keywords = 'globals_session_soap'.($xml_response ? '0' : '');
					} else {
						$response_list[$function_name] = self::_formatXmlString(empty($xml_response) ? '' : $xml_response->asXML());
						$keywords = self::_parse_response_keywords ($xml_response, 'globals_session_soap_');
					}
					// parse response keywords
					$keywords_list[$function_name] = str_replace('[ARRAY_ELEMENT]', '0', $keywords);
				}

				$settings['actions'] = $function_list;
				$settings['responses'] = $response_list;
				$settings['response_keywords'] = $keywords_list;
			}
		}

		$settings[$settings['action'].'_treat_as_fatal'] = array_get_index($request_data, $request_data['action'].'_treat_as_fatal',0);

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

		// Run test request
		$test_results = Array();
		if ($settings['run_test']) {
			// run test only for test selected
			foreach($functions as $function) {
				$function_name = preg_replace('/^\w+ (\w+)\(.*\)/', '$1', $function);
				if($function_name != $settings['action']) continue;
				$test_results[$function_name] = self::testSoapRequest($client, $settings, $function_name);
			}
		}//end if
		$settings['test_results'] = $test_results;


		return FALSE;

	}//end processInterface()


	/**
	* Get soap options from setting, e.g http auth, https cert.
	*
	* @return array
	* @access public
	*/
	public static function _getSoapOptions($settings)
	{
		// Work out HTTP auth
		if (isset($settings['login']) && isset($settings['password'])) {
			$options['login'] = $settings['login'];
			$options['password'] = $settings['password'];
		}

		// Work out HTTPS certificate and passphrase
		if(!empty($settings['local_cert'])) {
			$file = $GLOBALS['SQ_SYSTEM']->am->getAsset($settings['local_cert']);
			if(empty($file) || !method_exists($file, 'getExistingFile')) {
				return FALSE;
			}
			$file_info = $file->getExistingFile();
			$options['local_cert']  = $file_info['path'];
		}

		if(isset($settings['passphrase'])) {
			$options['passphrase'] = $settings['passphrase'];
		}

		if ($settings['run_test']) {
			$options['trace'] = 1;
		}

		return $options;
		
	}//end _getSoapOptions()


	/**
	* Converts xml object to array
	* Replace file content keyword
	*
	* @return array
	* @access public
	*/
	public static function _toArray($data)
	{
		if (is_object($data)) $data = get_object_vars($data);

		// Don't play with empty object
		if(empty($data)) return '';

		if(is_array($data)) {
			$result = Array();
			foreach ($data as $index => $element) {
				$result[$index] = self::_toArray($element);
			}
			return $result;
		} else {
			return $data;
		}

	}//end _toArray


	/**
	* Get sample SOAP request/response
	*
	* @return array
	* @access public
	*/
	public static function testSoapRequest($client, $settings, $function)
	{
		// Work out the parameters of that soap function
		for($i = 0, $param = Array(); $i < count($settings['actions'][$settings['action']]); $i++) {
			if(isset($settings[$settings['action'].'parameters'.$i])) {

				$soap_message = $settings[$settings['action'].'parameters'.$i];
				$object = @simplexml_load_string($soap_message);

				$param[] = $object;
			}
		}
		
		// Get an array of parameters
		$eval_string = '';
		foreach ($param as $index => $object) {
			$eval_string .= $index == 0 ? '' : ', ';
			$eval_string .= '$param['.$index.']';
		}

		// Make SOAP call
		try {
			eval('$data = $client->$function('.$eval_string.');');
			$soap_request = $client->__getLastRequest();
			$soap_response = $client->__getLastResponse();

		}
		catch (Exception $e) {
			trigger_error($e->getMessage());
			return FALSE;
		}

		return Array('request' => $soap_request, 'response' => $soap_response);

	}//end testSoapRequest()


	/**
	* This function recursively checks an object properties and store them in session
	*
	* @return void
	* @access publis
	*/
	public static function _store_results($object, $prefix = 'soap_', &$soap_response_keywords=Array())
	{
		if (empty($object)) return;

		$vars = is_object($object) ? get_object_vars($object) : $object;
		if (empty($vars)) return;

		foreach ($vars as $index => $value) {
			if (is_object($value) || is_array($value)) {
				self::_store_results ($value, $prefix.$index, $soap_response_keywords);
			}
			else {
				$_SESSION[SQ_SESSION_SANDBOX_INDEX][$prefix.$index] = $value;
				$soap_response_keywords[$prefix.$index] = $value;
			}
		}

	}//end _store_results()


	/**
	* This function returns XML representation of param struct
	*
	* @return object | void
	* @access public
	*/
	public static function _parse_type($type, $types)
	{
		$type_name = preg_replace('/ .*/', '', $type);
		//$param_name = preg_replace('/.* (.*);/', '$1', $param);
		$sub_types = self::_is_complex_type($type_name, $types);

		// If simple type arguments
		if (!$sub_types) {
			return self::_is_array_type($type_name, $types);
		}//end if

		// reset infinity loop check counter
		self::$recent_parsed_type = Array();

		// It's a complext type, let's parse it recursively
		$node = new SimpleXMLElement('<'.$type_name.'/>');
		self::_parse_type_recursive($type, $types, $node);

		return $node;

	}//end _parse_type()


	/**
	* This function recursively parse the complex param type
	*
	* @return null
	* @access public
	*/
	public static function _parse_type_recursive($param, $types, &$node)
	{
		// Keep recently parsed type, to do an infinity check
		foreach (self::$recent_parsed_type as $parsed) {
			if ($parsed == $param) return;
		}
		array_push(self::$recent_parsed_type, $param);
		if(count(self::$recent_parsed_type) > SQ_MAX_BREAK_OUT_AFTER) {
			array_shift(self::$recent_parsed_type);
		}

		$param_type = preg_replace('/ .*/', '', $param);
		$param_name = preg_replace('/.* (.*);/', '$1', $param);

		$sub_types = self::_is_complex_type($param_type, $types);
		if(!$sub_types) return;

		// It's a complext type, let's parse its children
		foreach ($sub_types as $type) {
			$is_type_array = FALSE;
			$sub_type_type = preg_replace('/ .*/', '', $type);
			$sub_type_name = preg_replace('/.* (.*);/', '$1', $type);
			$sub_sub_types = self::_is_complex_type($sub_type_type, $types, $is_type_array);

			if (!$sub_sub_types || ($sub_type_type == $param_type)) {
				// it's a simple type
				$node->addChild($sub_type_name, '??');
			}
			else {
				// it's complex type, so step into its children
				$new_node = $node->addChild($sub_type_name.($is_type_array ? '[ARRAY_ELEMENT]' : ''));
				self::_parse_type_recursive($type, $types, $new_node);

			}
		}

	}//end _parse_type_recursive()


	/**
	* This function checks if a param is of complex type from types list, if so, return the sub types
	*
	* @return false | array
	* @access public
	*/
	public static function _is_complex_type($param, $types, &$is_array=FALSE)
	{
		// Get array types
		$array_types = Array();
		foreach($types as $type) {
			if (substr($type, -2) == '[]') {
				$array_type = explode(' ', $type);
				if (count($array_type) > 1) {
					$array_types[rtrim($array_type[1], '[]')] = $array_type[0];
				}
			}
		}
		if (isset($array_types[$param])) {
			$param = $array_types[$param];
			$is_array = TRUE;
		}

		foreach ($types as $type) {
			if (preg_match ('/struct '.$param.'/', $type)) {
				// find and return sub types
				preg_match_all ('/\w+ \w+;/', $type, $matches);
				return $matches[0];
			}
		}

		return FALSE;

	}//end _parse_param()


	/** 
	* Checks if the given param is array
	*
	* @return boolean
	* @access private
	*/
	private static function _is_array_type($param, $types)
	{
		// Get array types
		$array_types = Array();
		foreach($types as $type) {
			if (substr($type, -2) == '[]') {
				$array_type = explode(' ', $type);
				if (count($array_type) > 1) {
					$array_types[rtrim($array_type[1], '[]')] = $array_type[0];
				}
			}
		}

		return isset($array_types[$param]);

	}//end _is_array_type()

	
	/**
	* This function formats a xml, inserts indentation, newlines for better display
	*
	* @return string
	* @access public
	*/
	public  static function _formatXmlString($xml)
	{
	  // add marker linefeeds to aid the pretty-tokeniser (adds a linefeed between all tag-end boundaries)
	  $xml = preg_replace('/(>)(<)(\/*)/', "$1\n$2$3", $xml);

	  // now indent the tags
	  $token      = strtok($xml, "\n");
	  $result     = ''; // holds formatted version as it is built
	  $pad        = 0; // initial indent
	  $matches    = array(); // returns from preg_matches()

	  // scan each line and adjust indent based on opening/closing tags
	  while ($token !== false) :

	    // test for the various tag states

	    // 1. open and closing tags on same line - no change
	    if (preg_match('/.+<\/\w[^>]*>$/', $token, $matches)) :
	      $indent=0;
	    // 2. closing tag - outdent now
	    elseif (preg_match('/^<\/\w/', $token, $matches)) :
	      $pad--;
	    // 3. opening tag - don't pad this one, only subsequent tags
	    elseif (preg_match('/^<\w[^>]*[^\/]>.*$/', $token, $matches)) :
	      $indent=1;
	    // 4. no indentation needed
	    else :
	      $indent = 0;
	    endif;

	    // pad the line with the required number of leading spaces
	    $line    = str_pad($token, strlen($token)+$pad, ' ', STR_PAD_LEFT);
	    $result .= $line . "\n"; // add to the cumulative result, with linefeed
	    $token   = strtok("\n"); // get the next token
	    $pad    += $indent; // update the pad size for subsequent lines
	  endwhile;

	  return $result;

	}// end _formatXmlString()


	/**
	* This function recursively extarcts available session keywords from response message
	*
	* @return string
	* @access public
	*/
	public static function _parse_response_keywords (&$response_xml, $prefix = '')
	{
		if (!is_object($response_xml)) return '';
		$data = get_object_vars($response_xml);
		$keywords_list = '';
		foreach ($data as $index => $sub_object) {
			if(is_object($sub_object) && !empty($sub_object)) {
				$keywords_list .= self::_parse_response_keywords($sub_object, $prefix.$index);
			}
			else {
				$keywords_list .= '%'.$prefix.$index.'%<br/>';
			}
		}

		return $keywords_list;

	}//end _parse_response_keywords


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
				apply_keyword_modifiers($replacement[$keyword], $modifiers);
			}

		}
		if (!empty($replacement)) replace_keywords($content, $replacement);

		return $content;

	}//end getKeywordReplacementsFromAsset()


}//end class

?>
