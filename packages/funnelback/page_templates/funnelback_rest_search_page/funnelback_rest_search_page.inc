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
* $Id: funnelback_rest_search_page.inc,v 1.3 2012/08/30 00:58:43 ewang Exp $
*
*/

require_once SQ_PACKAGES_PATH.'/web_services/rest/page_templates/page_rest_resource/page_rest_resource.inc';

/**
* Funnelback_REST_Search_Page 
*
*
* @author  Ash Karelia <akarelia@squiz.com.au>
* @version $Revision: 1.3 $
* @package MySource_Matrix_Packages
* @subpackage funnelback
*/
class Funnelback_REST_Search_Page extends Page_REST_Resource
{

	// These are the headers to be sent if send_headers is true. 
	// Only those with values found in the first response will be forwarded.
	private $_response_headers = Array(
									'Content-Type',
									'Content-Length',
									'Last-Modified',
									'Cache-Control',
									'Expires',
								 );


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
	* Called by the design to print extra material in
	* the head of the HTML document for this asset
	* 
	* @return void
	* @access public
	*/
	public function printHead()
	{
		// Run the request(s).
		$this->_process();

		if (!empty($this->_res['responses'])) {
			// Send the response. If there are multiple responses, send them all. This might
			// seem odd, but occasionally you might be building a document from multiple fragments.
			foreach ($this->_res['responses'] as &$resp) {
				if ($resp['info']['http_code'] < 400) {
					$body = $resp['body'];
					if (strpos($body, '<!--rest-asset-head-body-separator-->') !== FALSE) {
						$body_parts = explode('<!--rest-asset-head-body-separator-->', $body);
						echo $body_parts[0];
					}
				}
			}
		}

	}//end printHead()


	public function printBody()
	{
		// Run the request(s).
		$this->_process();

		if (!empty($this->_res['responses'])) {
			if ($this->attr('send_headers')) {
				foreach ($this->_response_headers as $h) {
					if (isset($this->_res['response']['headers'][$h])) {
						header("$h: " . $this->_res['response']['headers'][$h]);
					}
				}
			}

			// Send the response. If there are multiple responses, send them all. This might
			// seem odd, but occasionally you might be building a document from multiple fragments.
			foreach ($this->_res['responses'] as &$resp) {
				if ($resp['info']['http_code'] >= 400) {
					echo $this->attr('error_response');
				} else {
					$body = $resp['body'];
					if (strpos($body, '<!--rest-asset-head-body-separator-->') !== FALSE) {
						$body_parts = explode('<!--rest-asset-head-body-separator-->', $body);
						echo $body_parts[1];
					} else {
						echo $resp['body'];
					}
				}
			}
		}

	}//end printBody()


}//end class

?>
