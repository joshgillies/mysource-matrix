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
* $Id: data_source_soap.inc,v 1.11 2013/09/10 03:15:04 ewang Exp $
*
*/


require_once SQ_CORE_PACKAGE_PATH.'/../include/asset.inc';
require_once SQ_CORE_PACKAGE_PATH.'/data_source/data_source/data_source.inc';
require_once SQ_PACKAGES_PATH.'/web_services/api/lib/soap_common.inc';


/**
* Data_Source_Soap
*
* @author  Chiranjivi Upreti <cupreti@squiz.com.au>
* @version $Revision: 1.11 $
* @package MySource_Matrix_Packages
* @subpackage data
*/
class Data_Source_Soap extends Data_Source
{

	/**
	* Soap client object
	*/
	protected $_soap_client = null;


	/**
	* Current tree level of XML doc while parsing
	*
	* @var int
	*/
	protected $_tree_level = 0;


	/**
	* XML tree level where the record(s) is/are to be picked lies
	*
	* @var int
	*/
	protected $_record_tree_level = 0;


	/**
	* Flag set when relevent tag is encountered and unset when current xml recordset is done parsing with
	*
	* @var boolean
	*/
	protected $_non_empty_record = FALSE;


	/**
	* Parsed XML is stored in this array
	*
	* @var array
	*/
	protected $_result = Array();


	/**
	* Flag set when we are in a relevent tag
	* Set to false once parser's end of the element function handler is called for the tag
	*
	* @var boolean
	*/
	protected $_inside_relevant_tag = FALSE;


	/**
	* Flag set when we are in the specified root tag
	*
	* @var boolean
	*/
	protected $_inside_root_tag = FALSE;


	/**
	* Name of current tag
	*
	* @var string
	*/
	protected $_current_tag_name = '';


	/**
	* Current number of data record sets
	*
	* @var int
	*/
	protected $_record_set_index = 0;


	/**
	* Flag set when we are in a relevent tag
	* Set to false once the parser's character handler function is called for the tag
	*
	* @var boolean
	*/
	protected $_new_teg = FALSE;


	/**
	* Array to store info about if the element nests other non-text elements
	*
	*/
	protected $_complex_elements = Array();


	/**
	* The current tags hierarchy string
	* For example:
	*
	* <info>
	* 	<name>
	*		<first>Maxx</first>
	*		<other>Smith</other>
	*	</name>
	*	.
	*	.
	*	.
	* </info>
	*
	* If the current tag for above is "first", then the
	* tags hierarchy array will be Array(0 => 'info', 1 => 'name', 2=> 'first')
	*
	* @var array
	*/
	protected $_current_tag_hierarchy = Array();


	/**
	* Constructor
	*
	* @param int    $assetid    the asset id to be loaded
	*
	*/
	function __construct($assetid=0)
	{
		parent::__construct($assetid);

	}//end constructor


	/**
	* Returns an array of the result of SOAP request
	*
	* @return array
	* @access public
	*/
	function getResultSet()
	{
		$hash = $this->_getSoapRequestHash();

		$GLOBALS['SQ_SYSTEM']->pm->startTimer($this);
		// Check the local cache
		if (!isset($this->_tmp[$hash])) {

			// Check the system cache
			$result = $this->getCachedResultSet($hash);
			if ($result !== FALSE) {
				$this->_tmp[$hash] = $result;
			} else {
				// Not cached at all, do the SOAP request
				$this->_tmp[$hash] = $this->getSoapRequestResult();
				if (is_array($this->_tmp[$hash])) {
					$this->cacheResultSet($this->_tmp[$hash], $hash);
				}
			}
		}
		$GLOBALS['SQ_SYSTEM']->pm->stopTimer($this);

		return $this->_tmp[$hash];

	}//end getResultSet()


	/**
	* Get the unique hash for the SOAP request
	*
	* @return string
	* @access private
	*/
	function _getSoapRequestHash()
	{
		return md5(
				$this->attr('wsdl').
				$this->attr('username').
				$this->attr('password').
				$this->attr('auth_type').
				$this->attr('local_cert').
				$this->attr('passphrase').
				($this->attr('mute_exception') ? 't' : 'f').
				$this->attr('request_fn').
				serialize($this->getSoapRequestMessage()).
				$this->attr('root_tag').
				$this->attr('selected_tags').
				$this->attr('data_delimiter')
			);

	}//end _getSoapRequestHash()


	/**
	* Get the unique hash for the SOAP client
	*
	* @return string
	* @access private
	*/
	function _getSoapClientHash()
	{
		return md5(
				$this->attr('wsdl').
				$this->attr('username').
				$this->attr('password').
				$this->attr('auth_type').
				$this->attr('local_cert').
				$this->attr('passphrase').
				($this->attr('mute_exception') ? 't' : 'f')
			);

	}//end _getSoapClientHash()


	/**
	* Process the SOAP request and return the result
	*
	* @param object $response	Response object to get the result from instead of SOAP response
	*							If not null, the result will be obtained based on the given object, bypassing the SOAP call
	*
	* @return array
	* @access public
	*/
	function getSoapRequestResult($response=NULL)
	{
		if (is_null($response)) {
			$soap_messages = $this->getSoapRequestMessage();

			if (empty($soap_messages)) {
				return Array();
			}

			$params = Array();
			foreach($soap_messages as $soap_message) {
				// use DOMDocument::loadXML() to see if there is any entity  reference loop
				// which may exhaust us of the memory limit, if not then continue as normal
				$dom = new DOMDocument();
				if ($dom->loadXML($soap_message, LIBXML_NOENT)) {
					$xml_object = simplexml_import_dom($dom);
					if ($xml_object === FALSE) {
						continue;
					}
					$params[] = SOAP_Common::toArray($xml_object);
				}
			}//end foreach

			// Construct the eval string for SOAP function paramters
			$eval_string = '';
			foreach ($params as $index => $object) {
				$eval_string .= $index == 0 ? '' : ', ';
				$eval_string .= '$params['.$index.']';
			}

			// Make SOAP call
			$soap_function = $this->attr('request_fn');
			try {
				$soap_client = $this->getSoapClient();
				if (is_object($soap_client)) {
					eval('$response = $soap_client->$soap_function('.$eval_string.');');
				} else {
					throw new Exception("Invalid SOAP response object");
				}
			} catch (Exception $e) {
				// Save the last request
				$data = Array();
				$data['request'] = is_object($soap_client) ? $soap_client->__getLastRequest() : '';
				$data['response'] = Array(
										'last_response' => '',
										'response_xml' => '',
										'response_object' => '',
								);
				// Save the response/request data in the data dir
				$this->saveLastRequestResponse($data);
				trigger_error($e->getMessage());
				return Array();
			}
		}
		// Get the SOAP response xml and save the last request and response
		$data = Array();
		$data['request'] = !empty($soap_client) ? $soap_client->__getLastRequest() : '';
		$data['response'] = Array(
								'last_response' => !empty($soap_client) ? $soap_client->__getLastResponse() : '',
								'response_xml' => '',
								'response_obj' => !is_null($response) ? serialize($response) : '',
							);

		// Build the xml from the SOAP response
		$result_attr = $this->attr('request_fn').'Response';
		$response_xml = '';
		if (!is_null($response)) {
			// Convert response object to associative array
			$response = json_decode(json_encode($response), TRUE);
			// If response has any xml data, convert it to array first
			array_walk_recursive($response, Array($this, '_xml_to_array'));
			$this->_adjust_soap_array_values($response);

			$response_xml = '<'.$result_attr.'>'.SOAP_Common::arrayToXml($response)."\n".'</'.$result_attr.'>';
		}

		$data['response']['response_xml'] = $response_xml;
		$this->saveLastRequestResponse($data);

		// use DOMDocument::loadXML() to see if there is any entity  reference loop
		// which may exhaust us of the memory limit, if not then continue as normal
		$dom = new DOMDocument();
		if (empty($response_xml) || (!$dom->loadXML($response_xml, LIBXML_NOENT) && !@simplexml_import_dom($dom))) {
			trigger_error("Expected xml content not found in the SOAP response");
			return Array();
		}

		$result = $this->_createParser($response_xml);

		return $result;

	}//end getSoapRequestResult()


	/**
	* Get the SOAP request message
	*
	* @return array
	* @access public
	*/
	function getSoapRequestMessage()
	{
		$soap_messages = Array();

		$selected_fn = $this->attr('request_fn');
		$soap_requests = $this->attr('soap_requests');

		if (!empty($soap_requests[$selected_fn])) {
			$keyword_wrapper = '%%';
			$keyword_pattern = '('.$keyword_wrapper.'([a-zA-Z_\-0-9\.]+)'.$keyword_wrapper.')';
			foreach($soap_requests[$selected_fn] as $index => $soap_message) {
				// Replace the dynamic parameters in the SOAP message
				preg_match_all ('/'.$keyword_pattern.'/', $soap_message, $matches, PREG_PATTERN_ORDER);
				if (!empty($matches[2])) {
					$replacements = Array();
					$patterns = Array();
					foreach ($matches[2] as $keyword) {
						$value = $this->getDynamicVariableValue($keyword);
						$replacements[] = str_replace('$', '\$', $value);
						$patterns[] = '/('.$keyword_wrapper.$keyword.$keyword_wrapper.')/';
					}//end foreach
					$soap_message = preg_replace($patterns, $replacements, $soap_message);
				}//end if
				$soap_messages[] = $soap_message;
			}//end foreach
		}//end if

		return $soap_messages;

	}//end getSoapRequestMessage()


	/**
	* Makes the SOAP client based on the given settings
	*
	* @return null|object
	* @access public
	*/
	function getSoapClient()
	{
		if (!is_null($this->_soap_client)) {
			return $this->_soap_client;
		} else {
			$wsdl = trim($this->attr('wsdl'));
			if (!empty($wsdl)) {
				$settings = $this->attr('auth_details');
				$settings['wsdl'] = $wsdl;
				$settings['auth_type'] = $this->attr('auth_type');
				
				if ($this->attr('local_cert')) {
					$settings['local_cert'] = $this->attr('local_cert');
				}
				if ($this->attr('passphrase')) {
					$settings['passphrase'] = $this->attr('passphrase');
				}
				if ($this->attr('mute_exception')) {
					$settings['mute_exception'] = 1;
				}
				if ($this->attr('service_address')) {
					$settings['service_address'] = $this->attr('service_address');
				}
				$settings['run_test'] = 1;
				$soap_options = SOAP_Common::_getSoapOptions($settings);
				$client  = SOAP_Common::getSoapClient($settings, $soap_options);
				
				$this->_soap_client = $client;

			}//end if
		}

		return $this->_soap_client;

	}//end  getSoapClient()


	/**
	* Get SOAP service info
	*
	* @return array
	* @access public
	*/
	function getSoapServiceInfo()
	{
		$hash = $this->_getSoapClientHash().'_info';
		if (!isset($this->_tmp[$hash])) {
			$info = $this->getCachedResultSet($hash);
			if ($info !== FALSE) {
				$this->_tmp[$hash] = $info;
			} else {
				// Get the info
				$client = $this->getSoapClient();
				if (!is_null($client)) {
					try {
						$functions = $client->__getFunctions();
						$types = $client->__getTypes();
					}
					catch (Exception $e) {
						trigger_error($e->getMessage());
						return TRUE;
					}
					if(!empty($functions)) {
						$info = Array();
						$function_list = Array();
						$response_list = Array();

						foreach ($functions as $function) {
							$function_name = preg_replace('/^\w+ (\w+)\(.*\)/', '$1', $function);
							$function_params = explode(', ', preg_replace('/^\w+ \w+\((.*)\)/', '$1', $function));
							$function_response = preg_replace('/^(\w+) \w+\(.*\)/', '$1', $function);

							// Fixed duplicate function definition issue of PHP SoapClient
							if(isset($function_list[$function_name])) continue;

							// parse parameters
							$parsed_params = Array();
							foreach ($function_params as $param) {
								if (!trim($param)) continue;

								// Get XML representation of params of the functions
								$xml_param = SOAP_Common::_parse_type($param, $types);
								$parsed_params[$param] = SOAP_Common::_formatXmlString(empty($xml_param) ? $param : $xml_param->asXML());
							}
							$function_list[$function_name] = $parsed_params;

							// parse response
							$xml_response = SOAP_Common::_parse_type ($function_response, $types);
							if ($xml_response === TRUE || $xml_response === FALSE) {
								$response_list[$function_name] = SOAP_Common::_formatXmlString('<?xml version=\"1.0\"?'.'>'."<$function_name>??</$function_name>");
							} else {
								$response_list[$function_name] = SOAP_Common::_formatXmlString(empty($xml_response) ? '' : $xml_response->asXML());
							}
						}//end foreach
						$info['actions'] = $function_list;
						$info['responses'] = $response_list;

						$this->_tmp[$hash] = $info;
						$this->cacheResultSet($this->_tmp[$hash], $hash);
					}//end if
				}//end if
			}
		}//end if

		return isset($this->_tmp[$hash]) ? $this->_tmp[$hash] : Array();

	}//end getSoapServiceInfo()


	/**
	* Save the given last request/response data
	* in the asset's data dir
	*
	* @param array $data
	*
	* @return boolean
	* @access public
	*/
	function saveLastRequestResponse($data)
	{
		if (!is_array($data) || !isset($data['request']) || !isset($data['response'])) {
			return FALSE;
		}

		$string_data = @serialize($data);

		require_once SQ_FUDGE_PATH.'/general/file_system.inc';
		if ($string_data === FALSE || !create_directory($this->data_path)) {
			trigger_error('Could not write the SOAP request/response data to the file', E_USER_WARNING);
			return FALSE;
		}

		$file_path = $this->data_path.'/response_data';
		file_put_contents($file_path, $string_data);

	}//end saveLastRequestResponse()


	/**
	* Prints out the Frontend for this asset
	*
	* @return void
	* @access public
	*/
	public function printFrontend()
	{
		// Get the XSL file, if set
		$link = $GLOBALS['SQ_SYSTEM']->am->getLink($this->id, SQ_LINK_NOTICE, 'xsl_file', FALSE, 'xslt');
		$assetid = array_get_index($link, 'minorid', 0);
		$xsl_file = $assetid ? $GLOBALS['SQ_SYSTEM']->am->getAsset($assetid) : NULL;

		if (!is_null($xsl_file) && class_exists('XSLTProcessor')) {
			Asset::printFrontend();
		} else {
			parent::printFrontend();
		}

	}//end printFrontend()


	/**
	* Called by the design to print the body of this asset
	*
	* @return void
	* @access public
	*/
	public function printBody()
	{
		// Get the XSL Stylesheet
		$link = $GLOBALS['SQ_SYSTEM']->am->getLink($this->id, SQ_LINK_NOTICE, 'xsl_file', FALSE, 'xslt');
		$assetid = array_get_index($link, 'minorid', 0);
		$xsl_content = '';
		if ($assetid) {
			$xsl_file = $GLOBALS['SQ_SYSTEM']->am->getAsset($assetid);
			$xsl_content = $xsl_file->getContent();
		}

		if (!empty($xsl_content)) {
			// Refresh the SOAP response, if required
			$this->getResultSet();

			// Get the last SOAP response xml
			$path = $this->data_path.'/response_data';
			$data = @unserialize(file_get_contents($path));
			$response_xml = isset($data['response']['response_xml']) ? $data['response']['response_xml'] : '';

		}

		if (empty($xsl_content) || empty($response_xml)) {
			parent::printBody();
		} else {
			// Do transformation
			$xml = new DOMDocument();
			$xml->loadXML($response_xml);
			$xsl = new DOMDocument();
			$xsl->loadXML($xsl_content);

			$parser = new XSLTProcessor();
			$parser->importStylesheet($xsl);

			$content = $parser->transformToXML($xml);
			echo $content;

		}//end if

	}//end printBody()


	// XML Parser functions ///

	/**
	* Creates the xml parser which will then go off and parse our feed
	*
	* @param string	$content	The XML content to parse
	*
	* @return mixed array|boolean
	* @access public
	*/
	function _createParser($content)
	{
		$this->_result = Array();

		if (!empty($content)) {

			// hack time : #4588 Empty tags are not being parsed in XML Data Source
			//		1 - if we have <abc /> tag we change it to <abc>\n</abc>
			//		2 - if we have <abc></abc> tag we change it to <abc>\n</abc>
			$required_tags = $this->_getRequiredTags();
			if(!empty($required_tags)) {
				foreach($required_tags as $index => $required_tag) {
					$required_tag_quoted = preg_quote($required_tag);
					$pattern = '|(<'.$required_tag_quoted.')([\s]*[^>]*)/>|i';
					$replace = "<".$required_tag."$2>\n</".$required_tag.">";
					$content = preg_replace($pattern, $replace, $content);

					$pattern = '|(<'.$required_tag_quoted.')([\s]*[^>]*)></'.$required_tag_quoted.'>|i';
					$replace = "<".$required_tag."$2>\n</".$required_tag.">";

					$content = preg_replace($pattern, $replace, $content);

				}
			}

			$xml_parser = xml_parser_create();
			// stop the parser upper-casing everything
			xml_parser_set_option ($xml_parser, XML_OPTION_CASE_FOLDING, 0);
			xml_set_character_data_handler ($xml_parser, Array(&$this,'characterData') );
			xml_set_element_handler($xml_parser, Array(&$this,'startElement'), Array(&$this,'endElement'));

			if (!xml_parse($xml_parser, $content, TRUE)) {
				trigger_localised_error('CORE0258', E_USER_WARNING, xml_error_string(xml_get_error_code($xml_parser)).', line '.xml_get_current_line_number($xml_parser).', column '.xml_get_current_column_number($xml_parser).' (byte '.xml_get_current_byte_index($xml_parser).')', htmlentities($content, ENT_COMPAT, SQ_CONF_DEFAULT_CHARACTER_SET));
				return FALSE;
			}
			xml_parser_free($xml_parser);
			return $this->_result;
		}

		return Array();

	}//end _createParser()


	/**
	* Determines whether the parser is inside the specified root tag or not
	*
	* @param string	$name	Name of the element
	*
	* @return void
	* @access private
	*/
	function _checkRootTag($name)
	{
		$root_tag = trim($this->attr('root_tag'));

		if (!$this->_inside_root_tag) {
			if (empty($root_tag)) {
				$this->_inside_root_tag = TRUE;
			} else if ($name === $root_tag) {
				$this->_inside_root_tag = TRUE;
			} else {
				$this->_inside_root_tag = FALSE;
			}
		} else {
			// already inside the root tag
			if (empty($root_tag)) {
				// handle empty root tag
				$this->_inside_root_tag = TRUE;
			} else if ($name === $root_tag) {
				// handle the closing root tag
				$this->_inside_root_tag = FALSE;
			}
		}

	}//end _checkRootTag()


	/**
	* Handles the start of an element
	*
	* @param object	$parser		the xml parser
	* @param string	$name		name of the element
	* @param array	$attributes	the attributes
	*
	* @return void
	* @access public
	*/
	function startElement($parser, $name, $attributes=FALSE)
	{
		// Increment the XML tree level tracker one level up
		$this->_tree_level++;

		// Count the number of non-text elements the parsed element nests
		foreach($this->_complex_elements as $element => $val) {
			if ($this->_complex_elements[$element] >= 0) $this->_complex_elements[$element]++;
		}
		if (!isset($this->_complex_elements[$name])) {
			$this->_complex_elements[$name] = 0;
		}

		if (!empty($name)) {
			// check that we are inside the root tag
			$this->_checkRootTag($name);

			if ($this->_inside_root_tag) {

				// Add the tag in the hierarchy stack
				$this->_current_tag_hierarchy[] = $name;

				$required_tags = $this->_getRequiredTags();
				for ($i=0; $i<count($required_tags); $i++) {
					if ($name === $required_tags[$i]) {
						$this->_inside_relevant_tag = TRUE;
						$this->_current_tag_name = $name;
						$this->_new_tag = TRUE;

						// Set record tree level to where required elemets are
						$this->_record_tree_level = $this->_tree_level;

						// As far as required tags are concerned current record is non empty for sure
						$this->_non_empty_record = TRUE;

						if ($attributes) {
							$this->_current_attributes = $attributes;
						} else {
							$this->_current_attributes = FALSE;
						}
					}
				}

				// If the current tree level is less than tree level where records are then we are finished with the current record
				// Or even if the current tree level is on same level where the relevent record is,
				// if the current element complex then it will have to goto new record
				if ($this->_non_empty_record && ($this->_tree_level < $this->_record_tree_level || ($this->_tree_level == $this->_record_tree_level && $this->_complex_elements[$this->_current_tag_name] == -1))) {
					$this->_record_set_index++;

					// Don't know if required tags are going to be there in next recordset
					$this->_non_empty_record = FALSE;
				}

			}//end if inside root node
		}

	}//end startElement()


	/**
	* Handles the data
	*
	* @param object	$parser	the xml parser
	* @param string	$data	data inside the element
	*
	* @return void
	* @access public
	*/
	function characterData($parser, $data)
	{
		if ($this->_inside_relevant_tag) {
			$record_name = implode('.', $this->_current_tag_hierarchy);
			if (!isset($this->_result[$this->_record_set_index][$record_name])) {
				$this->_result[$this->_record_set_index][$record_name] = $data;
			}
			// If value for this tag is already set even when it is new tag then it means there are multiple tags with same name in the record
			// Merge the values using the given delimiter
			else if ($this->_new_tag) {
				$this->_result[$this->_record_set_index][$record_name] .=  $this->attr('data_delimiter') . $data;
			}
			// Being here implies that this function handler has been called more than once for data in the same tag.
			// Merge the data to get the whole thing.
			// See bug #4113 - When parser encounters special content in data like &amp;, &apos;, etc., it stops scanning and calls the data handler,
			// and then calls the handler again for the remaining data in the tag.
			else {
				$this->_result[$this->_record_set_index][$record_name] .=  $data;
			}
		}

		$this->_new_tag = FALSE;

	}//end characterData()


	/**
	* Handles the end of an element
	*
	* @param object	$parser	the xml parser
	* @param string	$name	name of the element
	*
	* @return void
	* @access public
	*/
	function endElement($parser, $name)
	{
		// check that we are outside of the root tag
		$this->_checkRootTag($name);

		// If the element nests 1 or more non-text elements then it is complex
		// -1 means complex and -2 means otherwise
		if ($this->_complex_elements[$name] >= 0) {
			$this->_complex_elements[$name] = $this->_complex_elements[$name] > 0 ? -1 : -2;

		}
		// Move the XML tree level tracker one level down on the closing of the element
		$this->_tree_level--;

		// Remove the tag in the hierarchy stack
		array_pop($this->_current_tag_hierarchy);

		$this->_inside_relevant_tag = FALSE;

	}//end endElement()


	/**
	* Returns all tags requested from the XML feed
	*
	* @return array
	* @access private
	*/
	function _getRequiredTags()
	{
		$required_tags = explode(',', $this->attr('selected_tags'));
		foreach($required_tags as $index => $required_tag) {
			$required_tags[$index] = trim($required_tag);
		}//end foreach

		return $required_tags;

	}//end _getRequiredTags()


	/**
	* Returns an array of all the permitted links type, the type asset and the cardinality
	* In the form
	*
	*   Array('[link_type]' => Array('[type_code]' => Array('card' => [cardinality], 'exclusive' => [exclusive])));
	* Where:
	*   link_type   = SQ_LINK_TYPE_1|SQ_LINK_TYPE_2|SQ_LINK_TYPE_3|SQ_LINK_NOTICE
	*   cardinality = [max number of links]|'M'
	*   exclusive   = TRUE|FALSE
	*
	* @return array
	* @access private
	*/
	function _getAllowedLinks()
	{
		// any link is allowed
		$allowed_link['asset']['card'] = 'M';
		$allowed_link['asset']['exclusive'] = FALSE;

		$links[SQ_LINK_TYPE_1] = $allowed_link;
		$links[SQ_LINK_TYPE_2] = $allowed_link;
		$links[SQ_LINK_TYPE_3] = $allowed_link;
		$links[SQ_LINK_NOTICE] = $allowed_link;

		return $links;

	}//end _getAllowedLinks()


	/**
	* Converts the valid XML string to array data
	* Used as callback function for array_walk_recursive() in getSoapRequestResult()
	*
	* @param array|string $val
	* @param $key
	*
	* @return void
	* @access private
	*/
	private function _xml_to_array(&$val, $key)
	{
		if (is_string($val)) {
			$dom = new DOMDocument();
			// use DOMDocument::loadXML() to see if there is any entity  reference loop
			// which may exhaust us of the memory limit, if not then continue as normal
			if ($dom->loadXML($val, LIBXML_NOENT)) {
				$xml_obj = @simplexml_import_dom($dom);
			}

			if ($xml_obj) {
				$val = json_decode(json_encode($xml_obj), TRUE);
			}
		}

	}//end _xml_to_array()


	/**
	* Adjust array data so tha "str_key" => "str_value" to
	* "str_key" => Array(0 => "str_value")
	* to make the SOAP response array consistant
	*
	* SOAP response will return multiple results as:
	* Array(
	*	'result' => Array(
	*				0 => [result1],
	*				1 => [result2],
	*					.
	*					.
	*					.
	*				),
	*			)
	* However if there is only one result then it will return result as:
	* Array(
	*	'result' => [result1],
	* 	)
	*
	* This is done to for the respone array 
	*
	* @return void
	* @access private
	*/
	private function _adjust_soap_array_values(&$data)
	{
		if (!is_array($data)) {
			return;
		}
		foreach($data as $key => $val) {
			if (!is_int($key) && (!is_array($val) || ((is_array($val) && !preg_match('/^[0-9]+$/', implode('', array_keys($val))))))) {
				$data[$key] = Array($val);
			}
			$this->_adjust_soap_array_values($data[$key]);
		}

	}//end _adjust_soap_array_values


}//end class

?>
