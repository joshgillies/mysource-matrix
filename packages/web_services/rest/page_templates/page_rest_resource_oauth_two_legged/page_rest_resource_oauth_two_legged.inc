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
* $Id: page_rest_resource_oauth_two_legged.inc,v 1.3.4.1 2013/09/04 03:37:02 akarelia Exp $
*
*/


require_once SQ_PACKAGES_PATH.'/web_services/rest/page_templates/page_rest_resource/page_rest_resource.inc';


/**
* Page_REST_Resource_OAuth_Two_Legged
*
* Calls 2 legged OAuth process
*
* @author  Maki Maruyama <mmaruyama@squiz.co.uk>
* @version $ $
* @package web_services
* @subpackage rest
*/
class Page_REST_Resource_OAuth_Two_Legged extends Page_REST_Resource
{
	private $_errors = Array();

	/**
	* Constructor
	*
	* @param string	$assetid	the asset id to be loaded
	*
	*/
	function __construct($assetid=0)
	{
		parent::__construct($assetid);

	}//end constructor


	/**
	 * Print this asset.
	 *
	 * @return void
	 * @access public
	 */
	public function printBody()
	{

	}//end printBody()

	/**
	 * Process the requests.
	 *
	 * @return void
	 * @access protected
	 */
	protected function _process()
	{

	}//end _process()


	/**
	 * Get Authorization Headers for user data access
	 *
	 * @param	Array	urls	urls that needs Authrization Headers for user data acess
	 * @param	Array	args	http request config values such as http method and request body
	 *
	 * @return	Array	key: urls passed  value: an Authorization Header for OAuth	
	 * @access	public
	 */
	public function getUserDataRequestAuthHeaders($urls=Array(), $args=Array())
	{
		$auth_headers = Array();
		if (empty($urls) || !isset($args['method'])) return Array();

		// two legged oauth needs the calling asset to tell it some config values such as method and request body
		$oauth = $this->getAttribute('oauth');
		foreach ($args as $config_name => $config_value) {
			$oauth->setConfig($config_name, $config_value);
		}
		
		// 2 legged oauth doesn't require neither access token nor access secret, so, just pass the url
		foreach ($urls as $url) {
			$auth_headers[$url] = $oauth->getUserDataAuthHeader($url);
		}

		return $auth_headers;

	}// end getUserDataRequestAuthHeaders()


}
?>
