<?php 
/**
* +--------------------------------------------------------------------+
* | Squiz.net Open Source Licence                                      |
* +--------------------------------------------------------------------+
* | Copyright (c), 2003 Squiz Pty Ltd (ABN 77 084 670 600).            |
* +--------------------------------------------------------------------+
* | This source file may be used subject to, and only in accordance    |
* | with, the Squiz Open Source Licence Agreement found at             |
* | http://www.squiz.net/licence.                                      |
* | Make sure you have read and accept the terms of that licence,      |
* | including its limitations of liability and disclaimers, before     |
* | using this software in any way. Your use of this software is       |
* | deemed to constitute agreement to be bound by that licence. If you |
* | modify, adapt or enhance this software, you agree to assign your   |
* | intellectual property rights in the modification, adaptation and   |
* | enhancement to Squiz Pty Ltd for use and distribution under that   |
* | licence.                                                           |
* +--------------------------------------------------------------------+
*
* $Id: page_rest_resource_js.inc,v 1.1 2009/09/23 07:26:30 akarelia Exp $
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
			$js .= '_REST = ' . json_encode($this->_res) . ";\n";
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
		echo $this->_runJavascript($js);

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
				trigger_error($js_errors);
				$errors = TRUE;
			}
		} 

		return $js_output;

	}//end _runJavascript()

}//end class

?>
