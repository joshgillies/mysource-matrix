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
* $Id: trigger_action_call_rest_resource.inc,v 1.9 2013/04/23 06:33:38 ewang Exp $
*
*/


require_once SQ_CORE_PACKAGE_PATH.'/system/triggers/trigger_action/trigger_action.inc';
require_once SQ_ATTRIBUTES_PATH.'/http_request/http_request.inc';
require_once SQ_PACKAGES_PATH.'/web_services/rest/lib/rest_common.inc';

/**
* Trigger_Action_Call_Rest_Resource
*
* A trigger action that calls REST resource(s)
*
*
* @author  Maki Maruyama <mmaruyama@squiz.co.uk>
* @version $Revision: 1.9 $
* @package MySource_Matrix_Packages
* @subpackage rest
*/
class Trigger_Action_Call_Rest_Resource extends Trigger_Action
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

		$http_request_value = $settings['http_request'];

		// create a new http request object
		$http_request_attr = new Asset_Attribute_HTTP_Request(0, $http_request_value);
		if (is_null($http_request_attr)) return FALSE;

		$current_value = unserialize($http_request_attr->value);

		// return false if there is no method or url specified
		if (empty($current_value['method']) && empty($current_value['urls'])) {
			return FALSE;
		}

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

		// ...and find their replacements and set them to the http request object
		$replacements = Array();
		foreach ($keywords_list as $keyword) {
			$replacements[$keyword] = $state['asset']->getKeywordReplacement($keyword);
		}

		$http_request_attr->setKeywordReplacements($replacements);

		// Set the current asset temporaily to one that fired this trigger action
		$old_current_asset = $GLOBALS['SQ_SYSTEM']->getGlobalDefine('CURRENT_ASSET', NULL);
		$GLOBALS['SQ_SYSTEM']->setGlobalDefine('CURRENT_ASSET', $state['asset']);

		// send the request
		$res =& $http_request_attr->run();

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
				// Append the JSON object "_REST" in the beginning of the JS code
				$js = Rest_Common::createJsonStringFromRestResponse($res).$js;

				// Replace global keywords and asset keywords
				if ($settings['js_replace_keywords']) {
					replace_global_keywords($js);
					replace_keywords($js, $replacements);
				}
				$GLOBALS['SQ_SYSTEM']->pm->startTimer($state['asset'], 'processJavascript');
				$ret = Rest_Common::processJavascript($js, $settings['js_engine']);
				$GLOBALS['SQ_SYSTEM']->pm->stopTimer($state['asset'], 'processJavascript');
				if (!empty($ret['output'])) {
					$_SESSION[SQ_SESSION_SANDBOX_INDEX][$settings['js_session_var']] =  $ret['output'];
				} else {
					trigger_localised_error('REST0001', sprintf(translate('%1$s: Keyword %2$s not found in REST response.'), $state['asset']->id, $ret['error_message']), E_USER_WARNING)
				}
			} else {
				// Store the response in the session as an array
				$_SESSION[SQ_SESSION_SANDBOX_INDEX][$settings['js_session_var']] = $res;
			}
		}//end if process js

		if (is_null($old_current_asset)) {
			$GLOBALS['SQ_SYSTEM']->unsetGlobalDefine('CURRENT_ASSET');
		} else {
			$GLOBALS['SQ_SYSTEM']->setGlobalDefine('CURRENT_ASSET', $old_current_asset);
		}

		// pass the triggering asset and REST responses received
		$GLOBALS['SQ_SYSTEM']->broadcastTriggerEvent('trigger_event_rest_response_received', $state['asset'], $res);
		$GLOBALS['SQ_SYSTEM']->am->forgetAsset($state['asset']);

		return Array(
				'assetid'	=> $assetid,
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

		$new_prefix = str_replace(array('[',']'), '_', $prefix);

		$http_request_value = array_get_index($settings, 'http_request', Array());
		$http_request_attr = new Asset_Attribute_HTTP_Request(0, $http_request_value);

		?>
		<div style="padding: 5px">
			 <p class="sq-backend-section-subheading"><?php echo translate('HTTP Request') ?></p>

			 <?php $http_request_attr->paint($new_prefix.'_http_request', !$write_access); ?>
		</div>
		<div style="padding: 5px">
			<p class="sq-backend-section-subheading"><?php echo translate('JavaScript Processing') ?></p>

			<table border="0" class="sq-backend-table">
				<tr>
					<th width="20%"><?php echo translate('Session Var Name'); ?></th>

					<td><?php
						if ($write_access) {
							text_box($new_prefix.'_js_session_var', array_get_index($settings, 'js_session_var', ''), 20);
						} else {
							echo array_get_index($settings, 'js_session_var', '');
						}
						echo '<div class="sq-backend-smallprint">'.translate('The response of HTTP Request will be stored as an array in the specified var in the Matrix session sandbox. NOTE: If you add any JavaScript code below to be executed, it will overwrite the HTTP response array that gets stored into this session variable by default.').'</div>';

						?>
					</td>
				</tr>
				<tr>
					<th><?php echo translate('Pre-Process Matrix Global Keywords'); ?></th>

					<td><?php
						$options = Array(translate('No'), translate('Yes'));


						if ($write_access) {
							combo_box($new_prefix.'_js_replace_keywords', $options, FALSE, array_get_index($settings, 'js_replace_keywords', 0));
						} else {
							echo array_get_index($options, array_get_index($settings, 'js_replace_keywords', 0), '');
						}
						echo '<div class="sq-backend-smallprint">'.translate('Enabling this option will replace the global keywords in the JS code prior running the JS engine.').'</div>';

						?>
					</td>
				</tr>
				<tr>
					<th><?php echo translate('JS Engine'); ?></th>

					<td><?php
						$supported_js_engines = Rest_Common::$supported_js_engines;
						$default_js_engine = Rest_Common::$default_js_engine;
						if ($write_access) {
							combo_box($new_prefix.'_js_engine', $supported_js_engines, FALSE, array_get_index($settings, 'js_engine', $default_js_engine));
						} else {
							echo array_get_index($supported_js_engines, array_get_index($settings, 'js_engine', $default_js_engine), '');
						}
						echo '<div class="sq-backend-smallprint">'.translate('JavaScript enigne to process the JavaScript code.').'</div>';

						?>
					</td>
				</tr>
				<tr>
					<th><?php echo translate('Include Files'); ?></th>

					<td><?php
						$include_files = array_get_index($settings, 'js_include_files', Array());
						if ($write_access) {
							multiple_asset_finder($new_prefix.'_js_include_files', $include_files, Array('file' => 'D'));
						} else {
							echo '<ul>';
								foreach ($include_files as $index => $assetid) {
									echo '<li>'.get_asset_tag_line($assetid).'</li>';
								}
							echo '</ul>';
						}
						echo '<div class="sq-backend-smallprint">'.translate('Selected JS files will be run sequentially before the JS code entered below.').'</div>';

						?>
					</td>
				</tr>
				<tr>
					<th><?php echo translate('JavaScript Code'); ?></th>

					<td><?php
						$js_code = array_get_index($settings, 'js_code', '');
						if ($write_access) {
							text_area($new_prefix.'_js_code', $js_code, 92, 10, 0, 'style="font-family: monospace;');
						} else {
							echo "<pre>".htmlspecialchars($js_code)."</pre>";
						}
						echo '<div class="sq-backend-smallprint">'.translate('The HTTP Request data is available as JSON data in the pre-defined variable "_REST". Use the "Run Test" feature to view this structure. For example "print(_REST.response.body);" will print the body of the first response. If you are using E4X to process an XML document, you will likely need to use "_REST.response.bodyx" to avoid the XML declaration choking the parser.<br/>NOTE: Any data printed via the JS "print();" function. will be stored into the Session Var configured above.').'</div>';

						?>
					</td>
				</tr>
			</table>
		</div>
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
		if (!isset($settings['http_request'])) {
			$http_request_value = serialize(Array());
		} else {
			$http_request_value = $settings['http_request'];
		}

		$http_request_attr = new Asset_Attribute_HTTP_Request(0, $http_request_value);
		$new_prefix = str_replace(array('[',']'), '_', array_get_index($request_data, 'new_prefix', ''));

		$http_request_attr->process($new_prefix.'_http_request');
		$settings['http_request'] = $http_request_attr->value;

		$settings['js_session_var'] = array_get_index($_POST, $new_prefix.'_js_session_var', '');
		$settings['js_replace_keywords'] = array_get_index($_POST, $new_prefix.'_js_replace_keywords', 0);
		$settings['js_engine'] = array_get_index($_POST, $new_prefix.'_js_engine', Rest_Common::$default_js_engine);
		$settings['js_code'] = array_get_index($_POST, $new_prefix.'_js_code', '');

		$settings['js_include_files'] = Array();
		$include_js_files = array_get_index($_POST, $new_prefix.'_js_include_files', Array());
		foreach($include_js_files as $index => $asset_info) {
			if (!empty($asset_info['assetid'])) {
				$settings['js_include_files'][] = $asset_info['assetid'];
			}//end if
		}//end foreach

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


}//end class

?>
