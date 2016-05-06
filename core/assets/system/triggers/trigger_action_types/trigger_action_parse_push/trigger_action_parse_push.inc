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
*
*/

require_once SQ_INCLUDE_PATH.'/general_occasional.inc';
require_once SQ_CORE_PACKAGE_PATH.'/system/triggers/trigger_action/trigger_action.inc';
require_once SQ_SYSTEM_ROOT.'/core/attributes/parameter_map/parameter_map.inc';
require_once SQ_SYSTEM_ROOT.'/core/attributes/option_list/option_list.inc';

/**
* Trigger_Action_Parse_Push
*
* A trigger action that sends a Push Notification using Parse
*
* @author  Nic Hubbard <nic@zedsaid.com>
* @package MySource_Matrix_Packages
*/
class Trigger_Action_Parse_Push extends Trigger_Action
{


	/**
	* Execute this action
	*
	* Returns an array of data about what it did, or false on error
	* 
	* @param array	$settings	the stored settings for this action
	* @param array	&$state		the state that describes the asset the action is executing on
	*
	* @return mixed array|boolean
	* @access public
	*/
	public static function execute($settings, &$state)
	{
		
		// Static values
		$static_app_id = $settings['parse_app_id'];
		$static_app_token = $settings['parse_device_token'];
		$static_app_rest_api_key = $settings['parse_app_rest_api_key'];
		$static_push_message = $settings['parse_message'];
		$static_push_sound = $settings['parse_sound'];
		$static_push_channel = $settings['parse_channel'];
		$static_push_badge = $settings['parse_badge'];
		$static_push_key1 = $settings['parse_key1'];
		$static_push_key2 = $settings['parse_key2'];
		$static_push_key3 = $settings['parse_key3'];
		$static_push_key4 = $settings['parse_key4'];
		$static_push_key5 = $settings['parse_key5'];
		$static_push_value1 = $settings['parse_value1'];
		$static_push_value2 = $settings['parse_value2'];
		$static_push_value3 = $settings['parse_value3'];
		$static_push_value4 = $settings['parse_value4'];
		$static_push_value5 = $settings['parse_value5'];
		$static_push_type = $settings['parse_app_type'];
		$static_android_title = $settings['android_title'];
		$static_android_uri = $settings['android_uri'];
		
		// check required settings
		if (!isset($static_app_id) || empty($static_app_id)) {
			trigger_error('You are missing the Application ID', E_USER_WARNING);
			return FALSE;
		}
		if (!isset($static_app_rest_api_key) || empty($static_app_rest_api_key)) {
			trigger_error('You are missing the REST API Key', E_USER_WARNING);
			return FALSE;
		}
		if (!isset($static_push_message) || empty($static_push_message)) {
			trigger_error('You are missing the push notification message', E_USER_WARNING);
			return FALSE;
		}
		
		// Make sure we either have a device token or we are broadcasting or have a channel
		$should_broadcast = array_get_index($settings, 'parse_broadcast', 0) ? TRUE : FALSE;
		if (empty($static_push_channel)) {
			if ((!isset($static_app_token) || empty($static_app_token)) && !$should_broadcast) {
				trigger_error('You must use either a device token, channel or broadcast.', E_USER_WARNING);
				return FALSE;
			}
		}
		
		if (empty($state['asset'])) {
			// grab the asset if assetid is given, but not the asset.
			if (empty($state['assetid'])) {
				return FALSE;
			} else {
				$state['asset'] = $GLOBALS['SQ_SYSTEM']->am->getAsset($state['assetid']);
			}
		}
		if (is_null($state['asset'])) return FALSE;
						
		// Param Map
		$parameter_map_value = array_get_index($settings, 'value_map', serialize(Array()));
		$atr_parameter_map = new Asset_Attribute_Parameter_Map(0, $parameter_map_value);
		$param_map_token = $atr_parameter_map->getParameterValue('value_map_token');
		$param_map_badge = $atr_parameter_map->getParameterValue('value_map_badge');
		$param_map_sound = $atr_parameter_map->getParameterValue('value_map_sound');
		$param_map_message = $atr_parameter_map->getParameterValue('value_map_message');
		$param_map_channel = $atr_parameter_map->getParameterValue('value_map_channel');
		$param_map_title = $atr_parameter_map->getParameterValue('value_map_title');
		$param_map_uri = $atr_parameter_map->getParameterValue('value_map_uri');
		
		// Dynamic Replacements
		if (!empty($param_map_token)) {
			$static_app_token = $param_map_token;
		}
		if (!empty($param_map_badge)) {
			$static_push_badge = $param_map_badge;
		}
		if (!empty($param_map_sound)) {
			$static_push_sound = $param_map_sound;
		}
		if (!empty($param_map_message)) {
			$static_push_message = $param_map_message;
		}
		if (!empty($param_map_channel)) {
			$static_push_channel = $param_map_channel;
		}
		if (!empty($param_map_title)) {
			$static_android_title = $param_map_title;
		}
		if (!empty($param_map_uri)) {
			$static_android_uri = $param_map_uri;
		}

		// replace keywords
		$static_push_badge_replaced = self::_replaceKeywordsInValue($state['asset'], $static_push_badge);
		$static_push_sound_replaced = self::_replaceKeywordsInValue($state['asset'], $static_push_sound);
		$static_push_message_replaced = self::_replaceKeywordsInValue($state['asset'], $static_push_message);
		$static_android_title_replaced = self::_replaceKeywordsInValue($state['asset'], $static_android_title);
		$static_android_uri_replaced = self::_replaceKeywordsInValue($state['asset'], $static_android_uri);
		
		// Only values should be replaced, not keys
		$static_push_value1_replaced = self::_replaceKeywordsInValue($state['asset'], $static_push_value1);
		$static_push_value2_replaced = self::_replaceKeywordsInValue($state['asset'], $static_push_value2);
		$static_push_value3_replaced = self::_replaceKeywordsInValue($state['asset'], $static_push_value3);
		$static_push_value4_replaced = self::_replaceKeywordsInValue($state['asset'], $static_push_value4);
		$static_push_value5_replaced = self::_replaceKeywordsInValue($state['asset'], $static_push_value5);
				

		if (!empty($static_push_message_replaced)) {
			
			// Message
			$message['alert'] = $static_push_message_replaced;
			
			// Sound
			if (!empty($static_push_sound_replaced)) {
				$message['sound'] = $static_push_sound_replaced;
			}
			
			// Badge
			if (!empty($static_push_badge_replaced)) {
				$message['badge'] = intval($static_push_badge_replaced);
			}

			
			// Custom payload
			$custom_payload = array();
			if (!empty($static_push_key1) && !empty($static_push_value1_replaced)) {
				$custom_payload[$static_push_key1] = $static_push_value1_replaced;
			}
			if (!empty($static_push_key2) && !empty($static_push_value2_replaced)) {
				$custom_payload[$static_push_key2] = $static_push_value2_replaced;
			}
			if (!empty($static_push_key3) && !empty($static_push_value3_replaced)) {
				$custom_payload[$static_push_key3] = $static_push_value3_replaced;
			}
			if (!empty($static_push_key4) && !empty($static_push_value4_replaced)) {
				$custom_payload[$static_push_key4] = $static_push_value4_replaced;
			}
			if (!empty($static_push_key5) && !empty($static_push_value5_replaced)) {
				$custom_payload[$static_push_key5] = $static_push_value5_replaced;
			}
			if (count($custom_payload) > 0) {
				$message['extra'] = $custom_payload;
			}
						
		}
		
		// Device Tokens
		if (!empty($static_app_token)) {
			if (strpos($static_app_token, ',')) {
				$static_app_token = explode(',', $static_app_token);
			} else {
				$static_app_token = array($static_app_token);
			}
		}
		
		// App Type
		$app_type = array_get_index($settings, 'parse_app_type', '');
		if (isset($options[$app_type])) {
			$app_type_value = $options[$app_type];
		}
		$app_type = strtolower($app_type);
		
		// Build the Parse request
		$APPLICATION_ID = $static_app_id;
		$REST_API_KEY = $static_app_rest_api_key;
		
		// Push Data
		$push_data = array();
		$push_data['alert'] = $message['alert'];
		
		// iOS
		if ($app_type == 'ios' || $app_type == 'all') {
			$push_data['badge'] = $message['badge'];
			$push_data['sound'] = $message['sound'];	
		}
		
		// Android
		if ($app_type == 'android' || $app_type == 'all') {
			if (!empty($static_android_title_replaced)) {
				$push_data['title'] = $static_android_title_replaced;
			}
			if (!empty($static_android_uri_replaced)) {
				$push_data['uri'] = $static_android_uri_replaced;
			}
		}
		
		// Custom Payload
		if (count($custom_payload) > 0) {
			$push_data['extra'] = $message['extra'];
		}
		
		$url = 'https://api.parse.com/1/push';
		$data = array();
		$data['data'] = $push_data;
		if ($should_broadcast) {
			
			$data['where'] = "{}";
			
		} else {
			
			$where = array();
			if (!empty($static_push_channel)) {
				$where['channels'] = $static_push_channel;
			}
			
			if (!empty($static_app_token)) {
				$where['deviceToken'] = array(
					'$in' => $static_app_token,
				);
			}
			
			if (!empty($app_type) && $app_type != 'all') {
				$where['deviceType'] = $app_type;
			}
			
			$data['where'] = $where;
			
		}
		
		$_data = json_encode($data);
		$headers = array(
			'X-Parse-Application-Id: ' . $APPLICATION_ID,
			'X-Parse-REST-API-Key: ' . $REST_API_KEY,
			'Content-Type: application/json',
			'Content-Length: ' . strlen($_data),
		);
		
		$curl = curl_init($url);
		curl_setopt($curl, CURLOPT_POST, 1);
		curl_setopt($curl, CURLOPT_POSTFIELDS, $_data);
		curl_setopt($curl, CURLOPT_HTTPHEADER, $headers);
		$content = curl_exec($curl);

		// Check if any error occured
		$response = curl_getinfo($curl);
		curl_close($curl);
		if ($response['http_code'] != 200) {
			 error_log("Parse Push Error: ".$content." Error Code: ".$response['http_code']." JSON: ".$_data, 0);
			 return FALSE;
		}
		
		return Array(
			'assetid'	=> $state['asset']->id,
		);

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
                
		<p class="sq-backend-section-subheading"><?php echo translate('Parse API Details').' '; ?></p>

			<table class="sq-backend-table with-bottom-margin" >
				<tr>
					<th width="100"><?php echo translate('Device Type').' '; ?></th>
					<td>
					<?php 
						$app_type = array_get_index($settings, 'parse_app_type', 'ios');
						$options = Array(
												'ios'			=> 'iOS',
												'android'		=> 'Android',
												'winrt'			=> 'Windows',
												'winphone'		=> 'Windows Phone',
												'all'			=> 'All',
											 );
						if ($write_access) {
							combo_box($prefix.'[parse_app_type]', $options, FALSE, $app_type);
						} else {
							echo '<b>'.$options[$app_type].'</b>';
						}
					?></td>
				</tr>
				<tr>
					<th><?php echo translate('Application ID').' '; ?></th>
					<td>
					<?php 
						$app_key = array_get_index($settings, 'parse_app_id', '');
						if ($write_access) {
							text_box($prefix.'[parse_app_id]', $app_key, 45);
						} else {
							echo str_repeat('&#9679;', strlen($app_key));
						}
					?></td>
				</tr>
				<tr>
					<th><?php echo translate('REST API Key').' '; ?></th>
					<td>
					<?php 
						$app_master_secret = array_get_index($settings, 'parse_app_rest_api_key', '');
						if ($write_access) {
							text_box($prefix.'[parse_app_rest_api_key]', $app_master_secret, 45);
						} else {
							echo str_repeat('&#9679;', strlen($app_master_secret));
						}
					?></td>
				</tr>
				<tr>
					<th><?php echo translate('Device Tokens').' '; ?></th>
					<td>
					<?php 
						$device_token = array_get_index($settings, 'parse_device_token', '');
						if ($write_access) {
							echo '<p>';
							$option_list = new Asset_Attribute_Option_List(0, $device_token, ',');
							$option_list->setEditParam('width', 85);
							$option_list->paint('parse_device_token');
							echo '</p>';
							echo '<p>'.translate('The token of the devices as generated by Parse').'</p>';
						} else {
							echo '<p>'.str_replace(',', '<br />', $device_token).'</p>';
						}
					?></td>
				<tr>
					<th><?php echo translate('Broadcast').' '; ?></th>
					<td>
					<?php 
						$parse_broadcast = array_get_index($settings, 'parse_broadcast', 0);
						if ($write_access) {
							echo '<p>';
							check_box($prefix.'[parse_broadcast]', 1, $parse_broadcast);
							echo '</p>';
							echo '<p class="sq-backend-warning">'.translate('This option will broadcast the push notification to ALL users associated with the Application ID. If this option is used device type, device tokens and channel will be ignored.').'</p>';
						} else {
							$tick_type = ($parse_broadcast == '1') ? 'tick' : 'cross';
							echo '<img class="sq-icon" width="16" height="16" src="'.sq_web_path('lib').'/web/images/'.$tick_type.'.png" alt="" style="border: 0;" />';
						}
					?></td>
				</tr>
                
			</table>            
			<p class="sq-backend-section-subheading"><?php echo translate('Push Notification Details').' '; ?></p>
			<table class="sq-backend-table with-bottom-margin" >
				<tr>
					<th width="100"><?php echo translate('Channel').' '; ?></th>
					<td>
					<?php 
						$parse_channel = array_get_index($settings, 'parse_channel', '');
						if ($write_access) {
							echo '<p>';
							text_box($prefix.'[parse_channel]', $parse_channel,  25);
							echo '</p>';
							echo '<p>'.translate('The name of the channel that the device is subscribed to.').'</p>';
						} else {
							echo '<p>'.$parse_channel.'</p>';
						}
					?></td>
				</tr>
                <?php
				
				// Set a default
				if (!isset($options[$app_type])) {
					$options[$app_type] = 'iOS';
				}
				$type = strtolower($options[$app_type]);
				if ($type == 'ios' || $type == 'all') {
				?>
				<tr>
					<th><?php echo translate('Badge').' '; ?></th>
					<td>
					<?php 
						$parse_badge = array_get_index($settings, 'parse_badge', '');
						if ($write_access) {
							echo '<p>';
							text_box($prefix.'[parse_badge]', $parse_badge,  5);
							echo '</p>';
							echo '<p>'.translate('A string to set as the app icon badge, normally a number is used. (iOS only)').'</p>';
						} else {
							echo '<p>'.$parse_badge.'</p>';
						}
					?></td>
				</tr>
				<tr>
					<th><?php echo translate('Sound').' '; ?></th>
					<td>
					<?php 
						$parse_sound = array_get_index($settings, 'parse_sound', '');
						if ($write_access) {
							echo '<p>';
							text_box($prefix.'[parse_sound]', $parse_sound, 25);
							echo '</p>';
							echo '<p>'.translate('The name of the sound to be played when the notification is recieved. Use the word &lt;strong&gt;default&lt;/strong&gt; for the default message sound. You can specify a custom sound that iOS plays when it presents the remote notification for an application. The sound files must be in the main bundle of the client application. (iOS only)').'</p>';
						} else {
							echo '<p>'.$parse_sound.'</p>';
						}
					?></td>
				</tr>
                <?php
				}
				?>
				<tr>
					<th><?php echo translate('Alert').' '; ?></th>
					<td>
					<?php 
						$parse_message = array_get_index($settings, 'parse_message', '');
						if ($write_access) {
							echo '<p>';
							text_box($prefix.'[parse_message]', $parse_message, 95);
							echo '</p>';
							echo '<p>'.translate('The maximum size allowed for a notification payload is 256 bytes. If you use too much text for the message it will be rejected.').'</p>';
						} else {
							echo '<p>'.$parse_message.'</p>';
						}
					?></td>
                </tr>
                <?php
				$type = strtolower($options[$app_type]);
				if ($type == 'android' || $type == 'all') {
				?>
				<tr>
					<th><?php echo translate('Title').' '; ?></th>
					<td>
					<?php 
						$parse_android_title = array_get_index($settings, 'android_title', '');
						if ($write_access) {
							echo '<p>';
							text_box($prefix.'[android_title]', $parse_android_title, 95);
							echo '</p>';
							echo '<p>'.translate('The value displayed in the Android system tray notification. (Android only)').'</p>';
						} else {
							echo '<p>'.$parse_android_title.'</p>';
						}
					?></td>
                </tr>
				<tr>
					<th><?php echo translate('URI').' '; ?></th>
					<td>
					<?php 
						$parse_android_uri = array_get_index($settings, 'android_uri', '');
						if ($write_access) {
							echo '<p>';
							text_box($prefix.'[android_uri]', $parse_android_uri, 95);
							echo '</p>';
							echo '<p>'.translate('An optional field that contains a URI. When the notification is opened, an Activity associated with opening the URI is launched. (Android only)').'</p>';
						} else {
							echo '<p>'.$parse_android_uri.'</p>';
						}
					?></td>
                </tr>
                <?php
				}
				?>
				<tr>
					<th><?php echo translate('Custom Key/Value Pairs'); ?></th>
					<td>
					<?php 
						$parse_key1 = array_get_index($settings, 'parse_key1', '');
						$parse_key2 = array_get_index($settings, 'parse_key2', '');
						$parse_key3 = array_get_index($settings, 'parse_key3', '');
						$parse_key4 = array_get_index($settings, 'parse_key4', '');
						$parse_key5 = array_get_index($settings, 'parse_key5', '');
						$parse_value1 = array_get_index($settings, 'parse_value1', '');
						$parse_value2 = array_get_index($settings, 'parse_value2', '');
						$parse_value3 = array_get_index($settings, 'parse_value3', '');
						$parse_value4 = array_get_index($settings, 'parse_value4', '');
						$parse_value5 = array_get_index($settings, 'parse_value5', '');
						if ($write_access) {
							echo '<p>';
							text_box($prefix.'[parse_key1]', $parse_key1, 15);
							echo '&nbsp; &nbsp;';
							text_box($prefix.'[parse_value1]', $parse_value1, 65);
							echo '</p>';
							echo '<p>';
							text_box($prefix.'[parse_key2]', $parse_key2, 15);
							echo '&nbsp;&nbsp;';
							text_box($prefix.'[parse_value2]', $parse_value2, 65);
							echo '</p>';
							echo '<p>';
							text_box($prefix.'[parse_key3]', $parse_key3, 15);
							echo '&nbsp;&nbsp;';
							text_box($prefix.'[parse_value3]', $parse_value3, 65);
							echo '</p>';
							echo '<p>';
							text_box($prefix.'[parse_key4]', $parse_key4, 15);
							echo '&nbsp;&nbsp;';
							text_box($prefix.'[parse_value4]', $parse_value4, 65);
							echo '</p>';
							echo '<p>';
							text_box($prefix.'[parse_key5]', $parse_key5, 15);
							echo '&nbsp;&nbsp;';
							text_box($prefix.'[parse_value5]', $parse_value5, 65);
							echo '</p>';
							echo '<p>'.translate('Additional key value pairs that will be sent as part of the payload within the "extra" JSON key. Keywords are supported for values.').'</p>';
						} else {
							echo '<p><ul>';
							echo '<li>Key: '.$parse_key1.' Value: '.$parse_value1.'</li>';
							echo '<li>Key: '.$parse_key2.' Value: '.$parse_value2.'</li>';
							echo '<li>Key: '.$parse_key3.' Value: '.$parse_value3.'</li>';
							echo '<li>Key: '.$parse_key4.' Value: '.$parse_value4.'</li>';
							echo '<li>Key: '.$parse_key5.' Value: '.$parse_value5.'</li>';
							echo '</ul></p>';
						}
					?></td>
				</tr>
			</table>
            
         <?php
		 
		// Print the parameter map
		if ($write_access) {
			?>
			<p class="sq-backend-section-subheading"><?php echo translate('Dynamic Parameters').' '; ?></p>
			<?php
			echo '<div>';
			$new_prefix = str_replace(array('[',']'), '_', $prefix);
			hidden_field($prefix.'[new_prefix]', $new_prefix);
			
			$parameter_map_value = array_get_index($settings, 'value_map', serialize(Array()));
			$atr_parameter_map = new Asset_Attribute_Parameter_Map(0, $parameter_map_value);
			
			// Param Map
			$atr_parameter_map->setParameter('value_map_token', 'Device Token');
			$atr_parameter_map->setParameter('value_map_badge', 'Badge');
			$atr_parameter_map->setParameter('value_map_sound', 'Sound');
			$atr_parameter_map->setParameter('value_map_message', 'Message');
			$atr_parameter_map->setParameter('value_map_channel', 'Channel');
			$atr_parameter_map->setParameter('value_map_title', 'Title');
			$atr_parameter_map->setParameter('value_map_uri', 'URI');
			echo $atr_parameter_map->paint($new_prefix.'_parameter_map');
			echo '</div>';
		}//end
		
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
		$settings['android_title'] = trim(array_get_index($request_data, 'android_title', ''));
		$settings['android_uri'] = trim(array_get_index($request_data, 'android_uri', ''));
		$settings['parse_app_id'] = trim(array_get_index($request_data, 'parse_app_id', ''));
		$settings['parse_channel'] = trim(array_get_index($request_data, 'parse_channel', ''));
		$settings['parse_app_rest_api_key'] = trim(array_get_index($request_data, 'parse_app_rest_api_key', ''));
		$settings['parse_message'] = trim(array_get_index($request_data, 'parse_message', ''));
		$settings['parse_sound'] = trim(array_get_index($request_data, 'parse_sound', ''));
		$settings['parse_broadcast'] = trim(array_get_index($request_data, 'parse_broadcast', 0));
		$settings['parse_badge'] = trim(array_get_index($request_data, 'parse_badge', 0));
		$settings['parse_key1'] = trim(array_get_index($request_data, 'parse_key1', 0));
		$settings['parse_key2'] = trim(array_get_index($request_data, 'parse_key2', 0));
		$settings['parse_key3'] = trim(array_get_index($request_data, 'parse_key3', 0));
		$settings['parse_key4'] = trim(array_get_index($request_data, 'parse_key4', 0));
		$settings['parse_key5'] = trim(array_get_index($request_data, 'parse_key5', 0));
		$settings['parse_value1'] = trim(array_get_index($request_data, 'parse_value1', 0));
		$settings['parse_value2'] = trim(array_get_index($request_data, 'parse_value2', 0));
		$settings['parse_value3'] = trim(array_get_index($request_data, 'parse_value3', 0));
		$settings['parse_value4'] = trim(array_get_index($request_data, 'parse_value4', 0));
		$settings['parse_value5'] = trim(array_get_index($request_data, 'parse_value5', 0));
		$settings['parse_app_type'] = trim(array_get_index($request_data, 'parse_app_type', 0));
		
		$settings['value_map'] = '';
		$new_prefix = str_replace(array('[',']'), '_', array_get_index($request_data, 'new_prefix', ''));
		if ($new_prefix) {
			$atr_parameter_map = new Asset_Attribute_Parameter_Map();
			$atr_parameter_map->process($new_prefix.'_parameter_map');
			$settings['value_map'] = $atr_parameter_map->value;
		}
		
		$option_list = new Asset_Attribute_Option_List(0, NULL, ',');
		$option_list->process('parse_device_token');
		$settings['parse_device_token'] = $option_list->value;
		
		return FALSE;

	}//end processInterface()


	/**
	* Get the list of locks that the action needs acquire before executing
	*
	* @param array	$settings	the stored settings for this action
	* @param array	&$state		the state that describes the asset the action is executing on
	*
	* @return array
	* @access public
	*/
	public static function getLocks($settings, &$state)
	{
		return Array();

	}//end getLocks()

	/**
	* Replace asset specific and global keywords in the value
	*
	* @param array	$asset	the asset to replace keywords for
	*
	* @return null
	* @access private
	*/
	private static function _replaceKeywordsInValue($asset, $value)
	{
		// replace global keywords
		$old_current_asset = $GLOBALS['SQ_SYSTEM']->getGlobalDefine('CURRENT_ASSET', NULL);
		$GLOBALS['SQ_SYSTEM']->setGlobalDefine('CURRENT_ASSET', $asset);

		replace_global_keywords($value);

		if (is_null($old_current_asset)) {
		        $GLOBALS['SQ_SYSTEM']->unsetGlobalDefine('CURRENT_ASSET');
		} else {
		        $GLOBALS['SQ_SYSTEM']->setGlobalDefine('CURRENT_ASSET', $old_current_asset);
		}

		$value = $asset->replaceKeywordsInString($value);

		return $value;
	}


}//end class

?>
