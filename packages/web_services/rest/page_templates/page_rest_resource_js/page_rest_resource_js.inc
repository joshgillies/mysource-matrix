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
* $Id: page_rest_resource_js.inc,v 1.12 2012/08/31 03:48:28 ewang Exp $
*
*/


require_once SQ_PACKAGES_PATH.'/web_services/rest/page_templates/page_rest_resource/page_rest_resource.inc';


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
		// let the user decide if they want Matrix to do keyword replacements on the response. See  bug
		// #5701 Minor Enhancement : Allow REST resources assets to stop replacing keywords in response
		$GLOBALS['SQ_SYSTEM']->setGlobalDefine('SQ_REPLACE_MYSOURCE_LEVEL_KEYWORDS', $this->attr('allow_global_replace'));

		$this->_processJavascript();

	}//end printBody()


    /**    
	* Creates JSON structures and runs the JS.
	*  
	* @return void
	* @access protected
	*/    
	protected function _processJavascript()
	{
		$js = 'var _REST = Array();';

		// Encode the data to JSON.
		if (!empty($this->_res)) {
			// Mostly the body doesn't want an XML declaration, as this chokes E4X in JS.
			// response.bodyx is available with any XML declaration removed.
			foreach ($this->_res['responses'] as &$resp) {
				$resp['bodyx'] = preg_replace("/^<\?xml\s.*\?>/", "", $resp['body']); 
			}
			if (!function_exists('json_encode')) {
				require_once 'Services/JSON.php';
				$json = new Services_JSON();
				$js .= '_REST = ' . $json->encodeUnsafe($this->_res) . ";\n";
			} else {
				$js .= '_REST = ' .json_encode($this->_res) . ";\n";
			}
		}

		// Run any include files.
		$file_ids = $this->attr('javascript_files');
		foreach ($file_ids as $fid) {
			$file =& $GLOBALS['SQ_SYSTEM']->am->getAsset($fid);

			$existing = $file->getExistingFile();
			if (!empty($existing)) {
				$js .= file_get_contents($existing['path']);
			}

			$GLOBALS['SQ_SYSTEM']->am->forgetAsset($file);
		}

		// Run any inline JS.
		$js .= $this->attr('javascript');

		// Go!
		//$rm =& $GLOBALS['SQ_SYSTEM']->am->getSystemAsset('rest_manager');
		$engine = $this->attr('js_engine');
		if($engine == 'spidermonkey')
		    echo $this->_runJavascript($js);
		else if($engine == 'v8')
		    echo $this->_runJavascriptv8($js);
		
	}//end _processJavascript()


	/**    
	* Runs JavaScript code through js on the command line using pipes. Anything on STDERR triggers an error (most likely a JS parse error).
	*  
	* @return string
	* @access protected
	*/    
	public function _runJavascript($js)
	{
		$js_output = '';

		// Open the process
		$descriptorspec = Array(
			0 => Array('pipe', 'r'),
			1 => Array('pipe', 'w'),
			2 => Array('pipe', 'w')
		);
		require_once SQ_DATA_PATH.'/private/conf/tools.inc';

		$process = proc_open(SQ_TOOL_JS_PATH . ' - ', $descriptorspec, $pipes);

		if (is_resource($process)) {
			$STDIN  = $pipes[0];
			$STDOUT = $pipes[1];
			$STDERR = $pipes[2];

			// Send data through js.
			fwrite($STDIN, $js);
			fclose($STDIN);
			$js_output = stream_get_contents($STDOUT);
			fclose($STDOUT);

			// Read STDERR
			// We must check for changes on the stream before reading, to avoid hanging for 
			// data that will never arrive.
			$read = Array($STDERR);
			$write = NULL;
			$except = NULL;
			$num_changed_streams = stream_select($read, $write, $except, 0);

			// For each readable pipe
			foreach ($read as $r) {
				switch ($r) {
				case $STDERR:
					$js_errors = stream_get_contents($STDERR);
					break;
				}
			}

			// Cleanup remaining resources
			fclose($STDERR);
			proc_close($process);

			// Handle errors
			if ((isset($js_errors)) && (strlen($js_errors) > 0 )) {
				trigger_error(__CLASS__ . " (#$this->id) " . $js_errors);
				$errors = TRUE;
			}
		} 

		return $js_output;

	}//end _runJavascript()

	/**
	* Runs JavaScript code through the Google V8 library.
	* http://uk.php.net/manual/en/book.v8js.php
	*
	* @return string
	* @access protected
	*/
	public function _runJavascriptv8($js)
	{
		if(!extension_loaded('v8js')){
			trigger_error('PECL v8js extension is required');
			return;
		}
		$js_output = '';

		$v8 = new V8Js();
		try {
			ob_start();
			$v8->executeString($js);
			$js_output = ob_get_clean();
		} catch (V8JsException $e) {
			$js_errors = $e->getMessage() .' - \''.$e->getJsSourceLine().'\' Line: '.$e->getJsLineNumber();
			trigger_error(__CLASS__ . " (#$this->id) " . $js_errors);
			$errors = TRUE;
		}

		return $js_output;

	}//end _runJavascript()

}//end class

?>
