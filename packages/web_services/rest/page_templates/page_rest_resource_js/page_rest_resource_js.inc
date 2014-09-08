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
* $Id: page_rest_resource_js.inc,v 1.15 2013/10/04 00:41:11 ewang Exp $
*
*/


require_once SQ_PACKAGES_PATH.'/web_services/rest/page_templates/page_rest_resource/page_rest_resource.inc';
require_once SQ_PACKAGES_PATH.'/web_services/rest/lib/rest_common.inc';


/**
* Page_REST_Resource_JS
*
* A simple Rest client, for fetching a remote resource and processing it with ECMAScript.
*
* Example:
*
* @author  James Hurst <jhurst@squiz.co.uk>
* @version $ $
* @package Squiz_UK_Clients
* @subpackage rest
*/
class Page_REST_Resource_JS extends Page_REST_Resource
{

	/**
	* Constructor
	*
	* @param string	$assetid	the asset id to be loaded
	*
	*/
	function __construct($assetid=0)
	{
		$this->_ser_attrs = TRUE;
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
		// Run the request(s).
		$this->_process();

		// String representing the REST response in the JSON object "_REST"
		$js = Rest_Common::createJsonStringFromRestResponse($this->_res);

		// Get any include js files
		$file_ids = $this->attr('javascript_files');
		foreach ($file_ids as $fid) {
			$file = $GLOBALS['SQ_SYSTEM']->am->getAsset($fid);
			$existing = $file->getExistingFile();
			if (!empty($existing)) {
				$js .= file_get_contents($existing['path']);
			}
			$GLOBALS['SQ_SYSTEM']->am->forgetAsset($file);
		}

		// Get any inline JS
		$js .= $this->attr('javascript');

		// Replace the matrix keywords on the resulting JS
		if ($this->attr('preprocess_keywords')) {
			replace_global_keywords($js);
		}//end if

		$GLOBALS['SQ_SYSTEM']->pm->startTimer($this, 'processJavascript');
		$ret = Rest_Common::processJavascript($js, $this->attr('js_engine'));
		$GLOBALS['SQ_SYSTEM']->pm->stopTimer($this, 'processJavascript');

		if (!empty($ret['error_message'])) {
			trigger_localised_error('REST0002', E_USER_WARNING, $this->id, $ret['error_message']);
		} else {
			if (!$this->attr('allow_global_replace')) {
				require_once SQ_FUDGE_PATH.'/general/text.inc';
				escape_matrix_keywords($ret['output']);
			}
			echo $ret['output'];
		}

	}//end printBody()

}//end class

?>
