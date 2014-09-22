<?php
/**
* +--------------------------------------------------------------------+
* | This MySource Matrix CMS file is Copyright (c) Squiz Pty Ltd   |
* | ABN 77 084 670 600                                                 |
* +--------------------------------------------------------------------+
* | IMPORTANT: Your use of this Software is subject to the terms of    |
* | the Licence provided in the file licence.txt. If you cannot find   |
* | this file please contact Squiz (www.squiz.com.au) so we may provide|
* | you a copy.                                                        |
* +--------------------------------------------------------------------+
*
* $Id: data_source_xml.inc,v 1.35 2013/08/20 07:51:18 ewang Exp $
*
*/


require_once SQ_CORE_PACKAGE_PATH.'/../include/asset.inc';
require_once SQ_CORE_PACKAGE_PATH.'/data_source/data_source/data_source.inc';
require_once SQ_LIB_PATH.'/html_form/html_form.inc';

/**
* Data_Source_Xml
*
* Purpose
*
*
* @author  Basil Shkara <bshkara@squiz.net>
* @version $Revision: 1.35 $
* @package MySource_Matrix_Packages
* @subpackage data
*/
class Data_Source_Xml extends Data_Source
{
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
	* Current attributes if any
	*
	* @var array
	*/
	protected $_current_attributes = Array();


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
	* Initialises the XML data source variables
	*
	* @return void
	* @access public
	*/
	function initialiseVars()
	{
		$this->_tree_level = 0;
		$this->_record_tree_level = 0;
		$this->_non_empty_record = FALSE;
		$this->_result = Array();
		$this->_inside_relevant_tag = FALSE;
		$this->_inside_root_tag = FALSE;
		$this->_current_tag_name = '';
		$this->_current_attributes = Array();
		$this->_record_set_index = 0;
		$this->_new_teg = FALSE;
		$this->_complex_elements = Array();

	}//end initialiseVars()


	/**
	* Returns an array of all the permitted link type, the type asset and the cardinality
	*
	* @return array
	* @access private
	* @see Asset::_getAllowLinks()
	*/
	public function _getAllowedLinks()
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
	* Returns an array of the result of XML parsing either from cache or by parsing the file again
	*
	* @return array
	* @access public
	*/
	function getResultSet()
	{
		$GLOBALS['SQ_SYSTEM']->pm->startTimer($this);
		// use feed url as the cache key
		$url = $this->_getFeedURL();

		if (!empty($url)) {
			$hash = md5($url);

			// Check the local cache
			if (!isset($this->_tmp[$hash])) {
				// Try from the system cache
				$result = parent::getResultSet($hash);

				if ($result !== FALSE) {
					$this->_tmp[$hash] = $result;
				} else {
					$this->_tmp[$hash] = $this->getItems();
					if ($this->_tmp[$hash] !== FALSE) {
						parent::setResultSet($this->_tmp[$hash], $hash);
					}
				}
			}
		}
		$GLOBALS['SQ_SYSTEM']->pm->stopTimer($this);

		return (!empty($url)) ? $this->_tmp[$hash] : Array();

	}//end getResultSet()


	/**
	* Returns an array of the result by parsing the XML feed
	*
	* @return array
	* @access public
	*/
	function getItems()
	{
		// Initialise the data source variables for the XML parsing so that
		// they does not confict with the previous XML parsing
		$this->initialiseVars();

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

		if (isset($this->_tmp['urlcontents'][$url]) === TRUE) {
			return $this->_tmp['urlcontents'][$url];
		}

		$options = array(
					'RETURNTRANSFER' => 1,
					'USERAGENT'      => SQ_SYSTEM_LONG_NAME,
				   );

		$responseDetails = fetch_url($url, $options);


		if ($responseDetails['errornumber'] != 0) {
			trigger_localised_error('CMS0063', sprintf(translate('Cannot connect to server while attempting to access "%1$s" - %2$s'), $url, $responseDetails['errorstring']), E_USER_WARNING)
			return FALSE;
		}
		/**
		if (preg_match('/^[45].*?/', $result)) {
			trigger_localised_error('CMS0064', sprintf(translate('Error response %1$s from server while attempting to access "%2$s"'), $result, $url), E_USER_WARNING)
			unset($HTTP_Client);
			return FALSE;
		}
		*/

		$response = $responseDetails['response'];

		$this->_tmp['urlcontents'][$url] = $response;

		return $response;

	}//end openFeed()


	/**
	* Sends GET request and calls the parsing process
	*
	* @return mixed array|boolean
	* @access public
	*/
	function parseXML()
	{
		$url = html_entity_decode($this->_tmp['url_string']);
		$content = $this->openFeed($url);

		$content = $this->_addCDATA($content);
		return $this->_createParser($content);

	}//end parseXML()


	/**
	* Returns the processed content of XML feed
	*
	* @return string
	* @access public
	*/
	function getXMLContent()
	{
		$url = html_entity_decode(isset($this->_tmp['url_string']) ? $this->_tmp['url_string'] : $this->attr('url'));
		$content = $this->openFeed($url);

		return $this->_addCDATA($content);

	}//end getXMLContent()


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

			$dom = new DOMDocument();
			// use DOMDocument::loadXML() to see if there is any entity  reference loop
			// which may exhaust us of the memory limit, if not then continue as normal
			if ($dom->loadXML($content, LIBXML_NOENT)) {

				$xml_parser = xml_parser_create();
				// stop the parser upper-casing everything
				xml_parser_set_option ($xml_parser, XML_OPTION_CASE_FOLDING, 0);
				xml_set_character_data_handler ($xml_parser, Array(&$this,'characterData') );
				xml_set_element_handler($xml_parser, Array(&$this,'startElement'), Array(&$this,'endElement'));

				if (!xml_parse($xml_parser, $content, TRUE)) {
					trigger_localised_error('CORE0258', sprintf(translate('XML error: "%1$s" for chunk "%2$s"', xml_error_string(xml_get_error_code($xml_parser)).', line '.xml_get_current_line_number($xml_parser).', column '.xml_get_current_column_number($xml_parser).' (byte '.xml_get_current_byte_index($xml_parser).')'), htmlentities($content, ENT_COMPAT, SQ_CONF_DEFAULT_CHARACTER_SET)), E_USER_WARNING)
					return FALSE;
				}
				xml_parser_free($xml_parser);

				return $this->_result;
			} else {
				return Array();
			}
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
			if (!isset($this->_result[$this->_record_set_index][$this->_current_tag_name])) {
				$this->_result[$this->_record_set_index][$this->_current_tag_name] = $data;
			}
			// If value for this tag is already set even when it is new tag then it means there are multiple tags with same name in the record
			// Merge the values using the given delimiter
			else if ($this->_new_tag) {
				$this->_result[$this->_record_set_index][$this->_current_tag_name] .=  $this->attr('data_delimiter') . $data;
			}
			// Being here implies that this function handler has been called more than once for data in the same tag.
			// Merge the data to get the whole thing.
			// See bug #4113 - When parser encounters special content in data like &amp;, &apos;, etc., it stops scanning and calls the data handler,
			// and then calls the handler again for the remaining data in the tag.
			else {
				$this->_result[$this->_record_set_index][$this->_current_tag_name] .=  $data;
			}

			$required_attributes = $this->_getRequiredAttributes();
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


	/**
	* Processes and returns the XML Feed URL for the XML datasource
	*
	* @return string
	* @access private
	*/
	function _getFeedURL()
	{
		$url = $this->attr('url');
		$keyword_wrapper = '%%';
		$keyword_pattern = '('.$keyword_wrapper.'([a-zA-Z_\-0-9\.]+)'.$keyword_wrapper.')';

		// insert the dynamic parameters into the url
		preg_match_all ('/'.$keyword_pattern.'/', $url, $matches, PREG_PATTERN_ORDER);
		if (empty($matches[1])) {
			$this->_tmp['url_string'] = $url;
			return $this->_tmp['url_string'];
		}

		$raw_keywords =& $matches[1];
		$keywords =& $matches[2];

		foreach ($keywords as $keyword) {
			$value = $this->getDynamicVariableValue($keyword);
			$replacements[] = str_replace('$', '\$', $value);
			$keyword = preg_quote($keyword);
			$patterns[] = '/('.$keyword_wrapper.$keyword.$keyword_wrapper.')/';
		}

		$url = preg_replace($patterns, $replacements, $url);

		$this->_tmp['url_string'] = $url;
		return $this->_tmp['url_string'];

	}//end _getFeedURL()


	/**
	* Returns the XML Feed URL for the XML datasource
	*
	* @return string
	* @access public
	*/
	function getURLString()
	{
		return isset($this->_tmp['url_string']) ? $this->_tmp['url_string'] : '';

	}//end getURLString()


	/**
	* Get a value of a dynamic variable
	*
	* @param string	$variable_name	Name of variable
	*
	* @return string
	* @access public
	*/
	function getDynamicVariableValue($variable_name)
	{
		$dynamic_vars = $this->attr('dynamic_vars');
		if (!isset($dynamic_vars[$variable_name])) return '';

		$parameter_map = $this->getAttribute('parameter_map');
		$value = $parameter_map->getParameterValue($variable_name);

		if (empty($value)) {
			// Default values are already urlencoded
			return array_get_index($dynamic_vars, $variable_name, '');
		}

		return urlencode($value);

	}//end getDynamicVariableValue()


	/**
	* Encloses the selected XML elements with CDATA term
	*
	* @param string $content 	XML content
	*
	* @return string
	* @access private
	*/
	function _addCDATA($content)
	{
		$cdata_tags = unserialize(html_entity_decode($this->attr('cdata_tags')));
		if (empty($cdata_tags)) return $content;
		$cdata_tags = array_unique($cdata_tags);

		foreach($cdata_tags as $cdata_tag) {
			if (!$cdata_tag) continue;
			$cdata_tag = preg_quote($cdata_tag);
			// Replace only if the element data is not already wrapped with CDATA
			$replace_pattern = "/(\<$cdata_tag(\s[^>]*?)*?(?<!\/)>)(?!\<\!\[CDATA\[)(.*?)(?<!\]\]>)(\<\/$cdata_tag\s*>)/ms";
			$replacement = "$1<![CDATA[$3]]>$4";
			$content = preg_replace($replace_pattern, $replacement, $content);
		}

		return $content;
	}


	/**
	* Prints out the Frontend for this asset
	*
	* @return void
	* @access public
	*/
	public function printFrontend()
	{
		$xsl_file = NULL;
		$xsl_file_id = $this->attr('xslt_file');

		if ($xsl_file_id) {
			$xsl_file = $GLOBALS['SQ_SYSTEM']->am->getAsset($xsl_file_id);

			if (!is_null($xsl_file) && class_exists('XSLTProcessor')) {
				Asset::printFrontend();
			}
		}//end if


	}//end printFrontend()


	/**
	* Called by the design to print the body of this asset
	*
	* @return void
	* @access public
	*/
	public function printBody()
	{
		// Find the XSL Stylesheet
		$xsl_file_id = $this->attr('xslt_file');
		$feed_url = $this->_getFeedURL();

		if (!empty($feed_url) && $xsl_file_id) {
			$xsl_file = $GLOBALS['SQ_SYSTEM']->am->getAsset($xsl_file_id);

			if (!is_null($xsl_file)) {
				// Do transformation
				$xml = new DOMDocument();
				// use DOMDocument::loadXML() to see if there is any entity  reference loop
				// which may exhaust us of the memory limit, if not then continue as normal
				if ($xml->loadXML($this->getXMLContent(), LIBXML_NOENT)) {
					$xsl = new DOMDocument();

					$xsl_content = $xsl_file->getContent();
					if ($this->attr('xslt_file_replace_keywords')) {
						replace_global_keywords($xsl_content);
					}
					$xsl->loadXML($xsl_content);

					$parser = new XSLTProcessor();
					$parser->importStylesheet($xsl);

					$content = $parser->transformToXML($xml);
					echo $content;
				}
			}
		}//end if

	}//end printBody()

}//end class

?>
