<?php
/**
* +--------------------------------------------------------------------+
* | This MySource Matrix CMS file is Copyright (c) Squiz Pty Ltd       |
* | ABN 77 084 670 600                                                 |
* +--------------------------------------------------------------------+
* | IMPORTANT: Your use of this Software is subject to the terms of    |
* | the Licence provided in the file licence.txt. If you cannot find   |
* | this file please contact Squiz (www.squiz.com.au) so we may provide|
* | you a copy.                                                        |
* +--------------------------------------------------------------------+
*
*/


/**
* Rest_Common
* This static class will have all the common functions used for REST Page, Trigger Action and Form Action
*
* @author  James Hurst <jhurst@squiz.co.uk>
* @author  Chiranjivi Upreti <cupreti@squiz.com.au>
* @version $Revision: $
* @package MySource_Matrix_Packages
* @subpackage web_service
*/
class Rest_Common
{

	/**
	* Supported JS processors list
	*
	* @array
	*/
	public static $supported_js_engines = Array(
											'spidermonkey'  => 'SpiderMonkey',
											'v8'    => 'V8',
										);
	/**
	* Default selected JS processer
	* Must be one of the keys in self::$supportedJsEngines
	*
	* @string
	*/
	public static $default_js_engine = 'v8';


	/**
	* Creates JSON structure "_REST" from given REST response
	*
	* @param array $rest_response
	*
	* @return string
	* @access public
	*/
	public static function createJsonStringFromRestResponse($rest_response)
	{
		$js = "var _REST = Array();\n";

		// Encode the data to JSON.
		if (!empty($rest_response['responses'])) {
			// Mostly the body doesn't want an XML declaration, as this chokes E4X in JS.
			// response.bodyx is available with any XML declaration removed.
			foreach ($rest_response['responses'] as &$resp) {
				$resp['bodyx'] = preg_replace("/^<\?xml\s.*\?>/", "", $resp['body']);
			}
			if (!function_exists('json_encode')) {
				require_once 'Services/JSON.php';
				$json = new Services_JSON();
				$js .= '_REST = ' . $json->encodeUnsafe($rest_response) . ";\n";
			} else {
				$js .= '_REST = ' .json_encode($rest_response) . ";\n";
			}
		}

		return $js;

	}//end createJsonStringFromRestResponse()


    /**
	* Runs the given JavaScript code
	*
	* @param string $js_code	JavaScript code to process
	* @param string $js_engine	JavaScript engine to use to process the js code
	*
	* @return array
	* @access public
	*/
	public static function processJavascript($js_code, $js_engine)
	{
		switch($js_engine) {
			case 'spidermonkey':
				$output = self::_runJavascript($js_code);
			break;
			case 'v8':
				$output = self::_runJavascriptv8($js_code);
			break;
			default:
				$output = Array(
						'output' => '',
						'error_message' => 'Unknown js engine',
					);
		}

		return $output;

	}//end processJavascript()


	/**
	* Runs JavaScript code through js on the command line using pipes. Anything on STDERR triggers an error (most likely a JS parse error).
	*
	* @return string
	* @access protected
	*/
	private static function _runJavascript($js)
	{
		$js_output = '';
		$js_errors = '';

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
		}

		return Array(
					'output' => $js_output,
					'error_message' => $js_errors,
				);

	}//end _runJavascript()


	/**
	* Runs JavaScript code through the Google V8 library.
	* http://uk.php.net/manual/en/book.v8js.php
	*
	* @return string
	* @access protected
	*/
	private static function _runJavascriptv8($js)
	{
		$js_output = '';
		$js_errors = '';

		if(!extension_loaded('v8js')){
			$js_errors = 'PECL v8js extension is required';
		} else {
			$v8 = new V8Js();
			try {
				ob_start();
				$v8->executeString($js);
				$js_output = ob_get_clean();
			} catch (V8JsException $e) {
				$js_errors = $e->getMessage() .' - \''.$e->getJsSourceLine().'\' Line: '.$e->getJsLineNumber();
			}
		}

		return Array(
					'output' => $js_output,
					'error_message' => $js_errors,
				);

	}//end _runJavascript()


	/**
	* Check if the given js engine type is installed on the system
	*
	* @param string $js_engine	JavaScript engine to use to process the js code
	*
	* @return array
	* @access public
	*/
	public static function jsEngineInstalled($js_engine)
	{
		$installed = FALSE;

		$js_engine = strtolower($js_engine);
		if ($js_engine == 'v8') {
			$installed = extension_loaded('v8js');
		} else if ($js_engine == 'spidermonkey') {
			require_once SQ_DATA_PATH.'/private/conf/tools.inc';
			$command = SQ_TOOL_JS_PATH.' -e \'print("ok");\'';
			exec($command, $output, $status);
			$installed = $status === 0;
		}

		return $installed;

	}//end jsEngineInstalled()

}//end class

?>
