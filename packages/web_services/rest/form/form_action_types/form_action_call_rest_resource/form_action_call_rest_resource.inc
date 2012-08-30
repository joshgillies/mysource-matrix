<?php
/**
* +--------------------------------------------------------------------+
* | This MySource Matrix CMS file is Copyright (c) Squiz Pty Ltd	   |
* | ACN 084 670 600													   |
* +--------------------------------------------------------------------+
* | IMPORTANT: Your use of this Software is subject to the terms of    |
* | the Licence provided in the file licence.txt. If you cannot find   |
* | this file please contact Squiz (www.squiz.com.au) so we may provide|
* | you a copy.														   |
* +--------------------------------------------------------------------+
*
* $Id: form_action_call_rest_resource.inc,v 1.7 2012/08/30 00:59:46 ewang Exp $
*
*/


require_once SQ_PACKAGES_PATH.'/cms/form/form_action/form_action.inc';
require_once SQ_ATTRIBUTES_PATH.'/http_request/http_request.inc';

/**
* Form_Action_Call_Rest_Resource
*
* Purpose
*
*
* @author  Maki Maruyama <mmaruyama@squiz.co.uk>
* @version $Revision: 1.7 $
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

		// Set up HTTP request attribute
		$http_request = new Asset_Attribute_HTTP_Request(0, $req_settings);

		$http_request->paint($prefix);

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

		// if either no method or urls are set, won't be able to send the request... 
		if (empty($req_settings['method'])) return FALSE;
		if (empty($req_settings['urls'])) return FALSE;
		

		return TRUE;

	}//end isValid

}//end class

?>
