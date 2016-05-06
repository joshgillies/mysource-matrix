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
*
*/


require_once SQ_CORE_PACKAGE_PATH.'/system/triggers/trigger_action/trigger_action.inc';
require_once SQ_ATTRIBUTES_PATH.'/http_request/http_request.inc';
require_once SQ_PACKAGES_PATH.'/web_services/rest/lib/rest_common.inc';

/**
* Trigger_Action_Sync_User_Sugar_Record
*
* A trigger action that syncs a Matrix user to Sugar Record
*
*
* @package MySource_Matrix_Packages
* @subpackage sugar
*/
class Trigger_Action_Sync_User_Sugar_Record extends Trigger_Action
{
	/**
	* Execute this action
	*
	* @param array	$settings	the stored settings for this action
	* @param array	&$state		the state that describes the asset the action is executing on
	*
	* @return mixed array|boolean
	* @access public
	*/
	public static function execute($settings, &$state)
	{
		if (empty($state['asset'])) {
			// grab the asset if assetid is given, but not the asset.
			if (empty($state['assetid'])) {
				return FALSE;
			} else {
				$state['asset'] = $GLOBALS['SQ_SYSTEM']->am->getAsset($state['assetid']);
			}
		}

		if (is_null($state['asset'])) return FALSE;

		$assetid = $state['asset']->id;

		$existing_record_id = $settings['record_id'];
		$attributes_to_sync = $settings['attributes_to_sync'];
		$attributes_for_create = $settings['attributes_for_create'];
		$module = $settings['module'];
		$token_id = $settings['token'];
		$token = $GLOBALS['SQ_SYSTEM']->am->getAsset($token_id);
		$create_new_record = $settings['create_new_record'];
		$link_record = $settings['link_record'];
		$link_record_id = $settings['link_record_id'];
		$link_record_module = $settings['link_record_module'];
		$link_record_id_attribute_name = $settings['link_record_id_attribute_name'];

		if(empty($token)){
			trigger_error(translate('Token is not specified.'));
			return FALSE;
		}
		if(empty($module)) {
			trigger_error(translate('Module is not specified.'));
			return FALSE;
		}

		// Set the current asset temporaily to one that fired this trigger action
		$old_current_asset = $GLOBALS['SQ_SYSTEM']->getGlobalDefine('CURRENT_ASSET', NULL);
		$GLOBALS['SQ_SYSTEM']->setGlobalDefine('CURRENT_ASSET', $state['asset']);

		$current_userid = $state['asset']->id;
		$current_user = $state['asset'];
		if($current_user instanceof Public_User) {
			trigger_error(translate('Public User can not be synced with a Sugar Record.'));
			return FALSE;
		}

		// replace keywords of some fields
		$existing_record_id = $state['asset']->replaceKeywordsInString($existing_record_id);
		replace_global_keywords($existing_record_id);
		$attributes_to_sync = $state['asset']->replaceKeywordsInString($attributes_to_sync);
		replace_global_keywords($attributes_to_sync);
		$attributes_for_create = $state['asset']->replaceKeywordsInString($attributes_for_create);
		replace_global_keywords($attributes_for_create);
		$link_record_id = $state['asset']->replaceKeywordsInString($link_record_id);
		replace_global_keywords($link_record_id);

		if (is_null($old_current_asset)) {
			$GLOBALS['SQ_SYSTEM']->unsetGlobalDefine('CURRENT_ASSET');
		} else {
			$GLOBALS['SQ_SYSTEM']->setGlobalDefine('CURRENT_ASSET', $old_current_asset);
		}

		$access_token_array = $token->getAccessToken();
		if(!isset($access_token_array['access_token'])) return FALSE;

		$access_token = $access_token_array['access_token'];
		$api_version = $token->attr('api_version');
		$base_url = $token->attr('token_endpoint');
		if(substr($base_url, -1) !== '/') {
			$base_url .= '/';
		}
		$api_url = $base_url.'rest/'.$api_version.'/';


		// do we have existing link?
		$bind_vars = Array(
						'matrix_userid'      => $current_userid,
						'module'      => $module,
					    );
		$result = MatrixDAL::executeAll('sugar_package', 'getLinks', $bind_vars);

		// if no link, lets search in Sugar
		if(empty($result) || (isset($result[0]['sugar_record_id']) && empty($result[0]['sugar_record_id']))) {
			if(!empty($existing_record_id)) {
				$existing_record = self::getSugarRecord($existing_record_id, $module, $api_url, $access_token);
				if(!empty($existing_record) && isset($existing_record['id'])) {
					// if there is a predefined record id and there is a match
					self::linkCurrentUserToSugar($current_userid, $existing_record['id'], $module);
					self::updateSugarRecord($existing_record['id'], $attributes_to_sync, $module, $api_url, $access_token);
					
					// associalte this record to an account
					if($link_record) {
						$result = self::associateToAccount($existing_record['id'], $link_record_id, $module, $link_record_module, $api_url, $access_token);
						// get refresh record again
						$existing_record = self::getSugarRecord($existing_record['id'], $module, $api_url, $access_token);
					}

					self::saveProfileToSession($existing_record, $module);
					return TRUE;
				}
			}
			if($create_new_record) {
				// create new record
				$new_record = self::createNewSugarRecord($current_userid, $attributes_for_create, $module, $api_url, $access_token);
				if(isset($new_record['id'])) {
					self::linkCurrentUserToSugar($current_userid, $new_record['id'], $module);
				
					// associalte this record to an account
					if($link_record) {
						$result = self::associateToAccount($new_record['id'], $link_record_id, $module, $link_record_module, $api_url, $access_token);
						// get refresh record again
						$new_record = self::getSugarRecord($new_record['id'], $module, $api_url, $access_token);
					}
					self::saveProfileToSession($new_record, $module);
				}
				else {
					trigger_error(translate('Failed to create new Sugar Record.'));
					return TRUE;
				}
			}
		}
		else {
			// sync user
			if(isset($result[0]['sugar_record_id']) && !empty($result[0]['sugar_record_id'])) {
				$record = self::updateSugarRecord($result[0]['sugar_record_id'], $attributes_to_sync, $module, $api_url, $access_token);
				// link record
				if($link_record) {
					// if it hasn't been linked before
					if(isset($record[$link_record_id_attribute_name]) && empty($record[$link_record_id_attribute_name])) {
						self::associateToAccount($result[0]['sugar_record_id'], $link_record_id, $module, $link_record_module, $api_url, $access_token);
						// have to get refresh record data again
						$record = self::getSugarRecord($result[0]['sugar_record_id'], $module, $api_url, $access_token);
					}
				}
				self::saveProfileToSession($record, $module);
			}
		}


		return TRUE;

	}//end execute()


	/**
	* Create a new Sugar record
	*
	* @param string	$current_userid			asset id of current matrix user
	* @param string	$attributes_to_sync		JSON format attributes to pass on
	* @param string	$module					sugar module
	* @param string	$api_url				API base url
	* @param string	$access_token			Sugar OAuth access token
	*
	* @return array
	* @access public
	*/
	public static function createNewSugarRecord($current_userid, $attributes_to_sync, $module, $api_url, $access_token)
	{
		if(empty($current_userid) || empty($api_url) || empty($module) || empty($access_token)) {
			trigger_error(translate('Missing information to create new Sugar Record'));
			return FALSE;
		}

		$new_record = self::executeRequest('POST', $api_url.$module, $access_token, $attributes_to_sync);

		return $new_record;
	}//end createNewSugarRecord()

	/**
	* Update a Sugar record
	*
	* @param string	$sugar_record_id		sugar record id
	* @param string	$attributes_to_sync		JSON format attributes to pass on
	* @param string	$module					sugar module
	* @param string	$api_url				API base url
	* @param string	$access_token			Sugar OAuth access token
	*
	* @return array
	* @access public
	*/
	public static function updateSugarRecord($sugar_record_id, $attributes_to_sync, $module, $api_url, $access_token)
	{
		if(empty($sugar_record_id) || empty($api_url) || empty($module) || empty($access_token)) {
			trigger_error(translate('Missing information to update Sugar Record'));
			return FALSE;
		}

		$new_record = self::executeRequest('PUT', $api_url.$module.'/'.$sugar_record_id, $access_token, $attributes_to_sync);

		return $new_record;
	}//end updateSugarRecord()


	/**
	* Retrieve Sugar record
	*
	* @param string	$sugar_record_id		sugar record id
	* @param string	$module					sugar module
	* @param string	$api_url				API base url
	* @param string	$access_token			Sugar OAuth access token
	*
	* @return array
	* @access public
	*/
	public static function getSugarRecord($sugar_record_id, $module, $api_url, $access_token)
	{
		if(empty($sugar_record_id) || empty($api_url) || empty($module) || empty($access_token)) {
			trigger_error(translate('Missing information to update Sugar Record'));
			return FALSE;
		}

		$record = self::executeRequest('GET', $api_url.$module.'/'.$sugar_record_id, $access_token);

		return $record;
	}//end getSugarRecord()



	/**
	* Relate a Sugar record to a Sugar account
	*
	* @param string	$record_id				sugar record id
	* @param string	$account_id				account id to relate to
	* @param string	$module					sugar module
	* @param string	$link_record_module		record module to link to
	* @param string	$api_url				API base url
	* @param string	$access_token			Sugar OAuth access token
	*
	* @return array
	* @access public
	*/
	public static function associateToAccount($record_id, $account_id, $module, $link_record_module, $api_url, $access_token)
	{
		if(empty($record_id) || empty($account_id) || empty($module) || empty($link_record_module)  || empty($access_token)) {
			trigger_error(translate('Missing information to associate record to an account'));
			return FALSE;
		}
		$post_data = '{ "link_name": "'.strtolower($module).'", "ids": ["'.$record_id.'"] }';
		$new_record = self::executeRequest('POST', $api_url.$link_record_module.'/'.$account_id.'/link', $access_token, $post_data);

		return $new_record;
	}//end associateToAccount()




	/**
	* store sugar record array to session
	*
	* @param string	$record					sugar record array
	* @param string	$module					sugar module
	*
	* @return array
	* @access public
	*/
	public static function saveProfileToSession($record, $module)
	{
		if(empty($record) || empty($module)) {
			return FALSE;
		}
		$_SESSION[SQ_SESSION_SANDBOX_INDEX]['sugar_'.strtolower($module)] = json_encode_array($record);
		self::_store_results($record, 'sugar_'.strtolower($module).'_');
		return $record;
	}//end saveProfileToSession()



	/**
	* This function recursively checks an object properties and store them in session
	* The stored data can be printed via keywords %globals_session_sugar_[module]_xx%
	*
	* @return void
	* @access private
	*/
	protected static function _store_results($object, $prefix = 'sugar_record_')
	{
		if (empty($object)) return;

		$vars = is_object($object) ? get_object_vars($object) : $object;
		if (empty($vars)) return;

		foreach ($vars as $index => $value) {
			if (is_object($value) || is_array($value)) {
				$_SESSION[SQ_SESSION_SANDBOX_INDEX][$prefix.$index] = json_encode_array($value);
				self::_store_results ($value, $prefix.$index.'_');
			} else {
				$_SESSION[SQ_SESSION_SANDBOX_INDEX][$prefix.$index] = $value;
			}
		}

	}//end _store_results()


	/**
	* Executing the CURL request
	*
	* @param string	$current_userid		asset id of current matrix user
	* @param string	$record_id			sugar record id
	* @param string	$module				sugar module
	*
	* @return array
	* @access public
	*/
	public static function linkCurrentUserToSugar($current_userid, $record_id, $module)
	{
		if(empty($current_userid) || empty($record_id) || empty($module)) {
			trigger_error(translate('Missing information to link current user to Sugar Record'));
			return FALSE;
		}


		// delete any existing entry
		$bind_vars = Array(
						'matrix_userid'	=>  $current_userid,
						'module'	=> $module
					    );
		$result = MatrixDAL::executeQuery('sugar_package', 'deleteLinkedUser', $bind_vars);


		// insert new entry
		$bind_vars = Array(
						'matrix_userid'	=>  $current_userid,
						'sugar_record_id'	=>  $record_id,
						'module'	=> $module,
						'created'	=>  ts_iso8601(time()),
					    );
		$result = MatrixDAL::executeQuery('sugar_package', 'addLink', $bind_vars);

		return $result;
	}//end linkCurrentUserToSugar()


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

		$new_prefix = str_replace(array('[',']'), '_', $prefix);

		$http_request_value = array_get_index($settings, 'http_request', Array());

		?>
			<p class="sq-backend-section-subheading"><?php echo translate('Sync Settings') ?></p>

			<table border="0" class="sq-backend-table with-bottom-margin">
				<tr>
					<th width="20%"><?php echo translate('SugarCRM Token'); ?></th>

					<td><?php
						$token = array_get_index($settings, 'token', '');
						if ($write_access) {
							asset_finder($new_prefix.'_token', $token, Array('sugarcrm_token' => 'D'));
						} else {
							echo get_asset_tag_line($token);
						}
						?>
					</td>
				</tr>
				<tr>
					<th><?php echo translate('SugarCRM Module'); ?></th>

					<td><?php
						$module = array_get_index($settings, 'module', '');


						if ($write_access) {
							text_box($new_prefix.'_module', $module, 120, '', FALSE, 'class="sq-full-cell-width"');
						} else {
							echo array_get_index($settings, 'module', '');
						}
						echo '<div class="sq-backend-smallprint">'.translate('The name of the module in SugarCRM to create this record under and to search under for an existing record to sync to.').'</div>';

						?>
					</td>
				</tr>
				<tr>
					<th><?php echo translate('Existing SugarCRM Record ID'); ?></th>

					<td><?php
						$module = array_get_index($settings, 'record_id', '');


						if ($write_access) {
							text_box($new_prefix.'_record_id', $module, 120, '', FALSE, 'class="sq-full-cell-width"');
						} else {
							echo array_get_index($settings, 'record_id', '');
						}
						echo '<div class="sq-backend-smallprint">'.translate('Optional. If you would like to sync current user to an existing Sugar Record, specify the Sugar Record ID here. Matrix will try to find and sync to the existing Record. If not found, Matrix will then create new Record using information specified below if "Create New Record If Record ID Not Found" is set to "Yes".').'</div>';

						?>
					</td>
				</tr>
				<tr>
					<th><?php echo translate('Create New Record If Record ID Not Found'); ?></th>

					<td><?php
						if ($write_access) {
							combo_box($new_prefix.'_create_new_record', Array('1' => translate('Yes'), '0' => translate('No')), FALSE, array_get_index($settings, 'create_new_record', '1'));
						} else {
							echo array_get_index($settings, 'create_new_record', '1') ? translate('Yes') : translate('No');
						}
						echo '<div class="sq-backend-smallprint">'.translate('If set to "Yes", Matrix will try and create a new Record in SugarCRM if it can not find the specified existing SugarCRM Record ID in the specified Module.').'</div>';

						?>
					</td>
				</tr>
				<tr>
					<th><?php echo translate('Attributes For Creating New Record'); ?></th>

					<td><?php
						$code = array_get_index($settings, 'attributes_for_create', '');
						if ($write_access) {
							text_area($new_prefix.'_attributes_for_create', $code, 101, 10, 0, 'style="font-family: monospace;" class="sq-full-cell-width"');
						} else {
							echo "<pre>".htmlspecialchars($code)."</pre>";
						}
						echo '<div class="sq-backend-smallprint">'.translate('Matrix will pass the above attributes when trying to create a record in SugarCRM. Attributes data must be in JSON format.').'</div>';

						?>
					</td>
				</tr>
				<tr>
					<th><?php echo translate('Attributes For Syncing To Existing Record'); ?></th>

					<td><?php
						$code = array_get_index($settings, 'attributes_to_sync', '');
						if ($write_access) {
							text_area($new_prefix.'_attributes_to_sync', $code, 101, 10, 0, 'style="font-family: monospace;" class="sq-full-cell-width"');
						} else {
							echo "<pre>".htmlspecialchars($code)."</pre>";
						}
						echo '<div class="sq-backend-smallprint">'.translate('Matrix will pass the above attributes when trying to do a sync to SugarCRM. Attributes data must be in JSON format.').'</div>';

						?>
					</td>
				</tr>
			</table>

			<p class="sq-backend-section-subheading"><?php echo translate('Record Linking Settings') ?></p>

			<table border="0" class="sq-backend-table">
				<tr>
					<th width="20%"><?php echo translate('Link Record To Another Record'); ?></th>

					<td><?php
						if ($write_access) {
							combo_box($new_prefix.'_link_record', Array('1' => translate('Yes'), '0' => translate('No')), FALSE, array_get_index($settings, 'link_record', '0'));
						} else {
							echo array_get_index($settings, 'link_record', '0') ? translate('Yes') : translate('No');
						}
						echo '<div class="sq-backend-smallprint">'.translate('If set to "Yes", Matrix will try to link the synced Record to another Record within the SugarCRM system using the Module and Record ID values specified below. Example: When relating a Contact Record to an Account Record.').'</div>';

						?>
					</td>
				</tr>
				<tr>
					<th><?php echo translate('Target Module'); ?></th>

					<td><?php
						$module = array_get_index($settings, 'link_record_module', '');


						if ($write_access) {
							text_box($new_prefix.'_link_record_module', $module, 120, '', FALSE, 'class="sq-full-cell-width"');
						} else {
							echo array_get_index($settings, 'link_record_module', '');
						}
						echo '<div class="sq-backend-smallprint">'.translate('The name of the Module in SugarCRM to search under for the Record ID specified below.').'</div>';

						?>
					</td>
				</tr>
				<tr>
					<th><?php echo translate('Target Record ID'); ?></th>

					<td><?php
						$module = array_get_index($settings, 'link_record_id', '');


						if ($write_access) {
							text_box($new_prefix.'_link_record_id', $module, 120, '', FALSE, 'class="sq-full-cell-width"');
						} else {
							echo array_get_index($settings, 'link_record_id', '');
						}
						echo '<div class="sq-backend-smallprint">'.translate('The Record ID to link the synced Matrix User Record to.').'</div>';

						?>
					</td>
				</tr>
				<tr>
					<th><?php echo translate('Target Record ID Attribute Name'); ?></th>

					<td><?php
						$module = array_get_index($settings, 'link_record_id_attribute_name', '');


						if ($write_access) {
							text_box($new_prefix.'_link_record_id_attribute_name', $module, 120, '', FALSE, 'class="sq-full-cell-width"');
						} else {
							echo array_get_index($settings, 'link_record_id_attribute_name', '');
						}
						echo '<div class="sq-backend-smallprint">'.translate('Specify the name of the attribute in the current Matrix User Record profile that holds the target Record ID. Matrix will only fire the linking API call to the SugarCRM system if this attribute value is empty (if the Record is not yet linked). Example: If we are linking a Contact Record to an Account Record, we should specify the attribute name as "account_id".').'</div>';

						?>
					</td>
				</tr>
			</table>
		<?php
		hidden_field($prefix.'[new_prefix]', $new_prefix);
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

		$new_prefix = str_replace(array('[',']'), '_', array_get_index($request_data, 'new_prefix', ''));

		$settings['module'] = array_get_index($_POST, $new_prefix.'_module', '');
		$settings['record_id'] = array_get_index($_POST, $new_prefix.'_record_id', '');
		$settings['create_new_record'] = array_get_index($_POST, $new_prefix.'_create_new_record', 1);
		$settings['attributes_to_sync'] = array_get_index($_POST, $new_prefix.'_attributes_to_sync', '');
		$settings['attributes_for_create'] = array_get_index($_POST, $new_prefix.'_attributes_for_create', '');
		$settings['link_record'] = array_get_index($_POST, $new_prefix.'_link_record', 0);
		$settings['link_record_id'] = array_get_index($_POST, $new_prefix.'_link_record_id', '');
		$settings['link_record_module'] = array_get_index($_POST, $new_prefix.'_link_record_module', '');
		$settings['link_record_id_attribute_name'] = array_get_index($_POST, $new_prefix.'_link_record_id_attribute_name', '');

		if(!empty($settings['attributes_to_sync'])) {
			$json_array = json_decode_array($settings['attributes_to_sync']);
			if(empty($json_array)) {
				trigger_error(translate('The attributes to sync is not in JSON format'));
			}
		}

		$settings['token'] = '';
		$token_array = array_get_index($_POST, $new_prefix.'_token', array());
		$settings['token'] = array_get_index($token_array, 'assetid', '');

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
		return Array($state['assetid'] => Array('lookups'));

	}//end getLocks()


	/**
	* Executing the CURL request
	*
	* @param string	$type			type of request: GET, PUT, POST etc
	* @param string	$url			request URL
	* @param string	$access_token	Sugar OAuth access token
	* @param array	$data			POST data
	*
	* @return array
	* @access public
	*/
	public static function executeRequest($type='GET', $url, $access_token, $data='')
	{
		$curl = curl_init($url);
		curl_setopt($curl, CURLOPT_HEADER, false);
		curl_setopt($curl, CURLOPT_RETURNTRANSFER, true);
		curl_setopt($curl, CURLOPT_HTTPHEADER, array('Content-Type: application/json',"OAuth-Token: $access_token"));
		curl_setopt($curl, CURLOPT_CUSTOMREQUEST, $type);
		if(!empty($data)) {
			curl_setopt($curl, CURLOPT_POSTFIELDS, $data);
		}

		// Make the REST call, returning the result
		$response = curl_exec($curl);
		$response_array = json_decode_array($response);
		if(empty($response_array) || isset($response_array['error'])) {
			if(isset($response_array['error_message'])) {
				trigger_error(translate('Sugar API error: ').$response_array['error_message']);
			}
			else {
				trigger_error(translate('Sugar API error: ').$response);
			}
		}
		return $response_array;
	}//end executeRequest()



}//end class

?>
