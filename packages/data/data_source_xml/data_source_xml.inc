<?php
/**
* +--------------------------------------------------------------------+
* | This MySource Matrix Module file is Copyright (c) Squiz Pty Ltd    |
* | ACN 084 670 600                                                    |
* +--------------------------------------------------------------------+
* | IMPORTANT: This Module is not available under an open source       |
* | license and consequently distribution of this and any other files  |
* | that comprise this Module is prohibited. You may only use this     |
* | Module if you have the written consent of Squiz.                   |
* +--------------------------------------------------------------------+
*
* $Id: data_source_xml.inc,v 1.5 2008/12/10 05:47:18 bshkara Exp $
*
*/

check_ssv_licence();
require_once SQ_CORE_PACKAGE_PATH.'/../include/asset.inc';
require_once SQ_CORE_PACKAGE_PATH.'/data_source/data_source/data_source.inc';
require_once SQ_LIB_PATH.'/html_form/html_form.inc';
require_once 'HTTP/Client.php';

/**
* Data_Source_Xml
*
* Purpose
*
*
* @author  Basil Shkara <bshkara@squiz.net>
* @version $Revision: 1.5 $
* @package MySource_Matrix_Packages
* @subpackage data
*/
class Data_Source_Xml extends Data_Source
{

	/**
	* Parsed XML is stored in this array
	*
	* @var array
	*/
	protected $_result = Array();


	/**
	* Flag set when we are in a tag of which we want its data
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
	* Current attributes if any
	*
	* @var array
	*/
	protected $_current_attributes = Array();


	/**
	* Number of tags parsed
	*
	* @var int
	*/
	protected $_passes = 0;


	/**
	* Result of: passes / number of required tags
	*
	* @var int
	*/
	protected $_quotient = 0;


	/**
	* Current number of data record sets
	*
	* @var int
	*/
	protected $_record_set_index = 0;


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
	* Returns an array of the result of XML parsing either from cache or by parsing the file again
	*
	* @return array
	* @access public
	*/
	function getResultSet()
	{
		// use feed url as the cache key
		$url = $this->attr('url');

		$result = Array();
		$result = parent::getResultSet($url);
		if (empty($result)) {
			if (!empty($url)) {
				if (isset($this->_tmp[$url])) {
					$result = $this->_tmp[$url];
				}
				if (empty($result)) $result = $this->getItems();
				if (!empty($result)) {
					parent::setResultSet($result, $url);
					$this->_tmp[$url] = $result;
				}
			}
		}
		return $result;

	}//end getResultSet()


	/**
	* Returns an array of the result by parsing the XML feed
	*
	* @return array
	* @access public
	*/
	function getItems()
	{
		$result = $this->parseXML();
		return $result;

	}//end getItems()


	/**
	* Uses HTTP_Client to send a GET request to the URL provided
	*
	* @param string	$url	The url where the feed is located
	*
	* @return mixed array|boolean
	* @access public
	*/
	function openFeed($url)
	{
		// Using HTTP_Client PEAR package
		// check whether or not the HTTP_Client object has already been
		// cached in $_SESSION[SQ_REMOTE_HTTP_CLIENT][AssetID]
		$HTTP_Client = array_get_index(array_get_index($_SESSION, 'SQ_REMOTE_HTTP_CLIENT'), $this->id);
		if (!empty($HTTP_Client)) {
			$HTTP_Client = unserialize($HTTP_Client);
		}

		if (is_null($HTTP_Client)) {
			// parameters for HTTP_Request. See Package for description.
			$request_parameters['useBrackets'] = FALSE;

			$default_headers['User-Agent'] = SQ_SYSTEM_LONG_NAME;

			// Add the Proxy authentication details if enabled
			addProxyInfo($request_parameters, $default_headers, $url);

			$HTTP_Client =& new HTTP_Client($request_parameters, $default_headers);

			$HTTP_Client->enableHistory(FALSE);
		}

		$HTTP_Client->setRequestParameter('useBrackets', FALSE);

		$result = $HTTP_Client->get($url);

		// exit on error
		if (PEAR::isError($result)) {
			trigger_localised_error('CMS0063', E_USER_WARNING, $url, $result->getMessage());
			unset($HTTP_Client);
			return FALSE;
		}
		if (preg_match('/^[45].*?/', $result)) {
			trigger_localised_error('CMS0064', E_USER_WARNING, $result, $url);
			unset($HTTP_Client);
			return FALSE;
		}

		$response = $HTTP_Client->currentResponse();

		// wipe the response because we won't use it again and it could be too big to just ignore
		$HTTP_Client->_responses = NULL;
		$_SESSION['SQ_REMOTE_HTTP_CLIENT'][$this->id] = serialize($HTTP_Client);
		unset($HTTP_Client);

		return $response['body'];

	}//end openFeed()


	/**
	* Sends GET request and calls the parsing process
	*
	* @return mixed array|boolean
	* @access public
	*/
	function parseXML()
	{
		$url = html_entity_decode($this->attr('url'));
		$content = $this->openFeed($url);

		return $this->_createParser($content);

	}//end parseXML()


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
		if (!empty($content)) {
			$xml_parser = xml_parser_create();
			// stop the parser upper-casing everything
			xml_parser_set_option ($xml_parser, XML_OPTION_CASE_FOLDING, 0);
			xml_set_character_data_handler ($xml_parser, Array(&$this,'characterData') );
			xml_set_element_handler($xml_parser, Array(&$this,'startElement'), Array(&$this,'endElement'));

			if (!xml_parse($xml_parser, $content, TRUE)) {
				trigger_localised_error('CORE0258', E_USER_WARNING, xml_error_string(xml_get_error_code($xml_parser)), htmlentities($content));
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
		$root_tag = $this->attr('root_tag');
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
		if (!empty($name)) {
			// check that we are inside the root tag
			$this->_checkRootTag($name);

			if ($this->_inside_root_tag) {
				$required_tags = $this->_getRequiredTags();
				for ($i=0; $i<count($required_tags); $i++) {
					if ($name === $required_tags[$i]) {
						$this->_inside_relevant_tag = TRUE;
						$this->_current_tag_name = $name;
						$this->_passes++;
						// used to calculate when next record set begins
						$this->_quotient = $this->_passes / count($required_tags);
						if ($attributes) {
							$this->_current_attributes = $attributes;
						} else {
							$this->_current_attributes = FALSE;
						}
					}
				}
			}
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
		$required_attributes = $this->_getRequiredAttributes();
		if ($this->_inside_relevant_tag && trim($data) !== '') {
			$this->_result[$this->_record_set_index][$this->_current_tag_name] = $data;
			// add attribute values if required
			if (!empty($required_attributes) && $this->_current_attributes) {
				foreach ($required_attributes as $tag_name => $attr_name) {
					if ($this->_current_tag_name === $tag_name) {
						foreach ($this->_current_attributes as $name => $value) {
							for ($i=0; $i<count($attr_name); $i++) {
								if ($attr_name[$i] === $name) {
									$this->_result[$this->_record_set_index][$this->_current_tag_name.':'.$attr_name[$i]] = $value;
								}
							}
						}
					}
				}
			}
			// integer quotient means we've iterated required tags
			if (is_int($this->_quotient)) {
				$this->_record_set_index++;
			}
		}

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
		return unserialize(html_entity_decode($this->attr('tags')));

	}//end _getRequiredTags()


	/**
	* Returns all requested attributes and their corresponding tag names from the XML feed
	*
	* @return array
	* @access private
	*/
	function _getRequiredAttributes()
	{
		return unserialize(html_entity_decode($this->attr('tag_attributes')));

	}//end _getRequiredAttributes()


}//end class
?>
