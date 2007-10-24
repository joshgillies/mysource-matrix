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
* $Id: data_source_xml.inc,v 1.1.2.2 2007/10/24 06:03:13 bshkara Exp $
*
*/

check_ssv_licence();
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
* @version $Revision: 1.1.2.2 $
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
	var $_result = Array();


	/**
	* Flag set when we are in a tag of which we want its data
	*
	* @var boolean
	*/
	var $_inside_relevant_tag = FALSE;


	/**
	* Name of current tag
	*
	* @var string
	*/
	var $_current_tag_name = '';


	/**
	* Current attributes if any
	*
	* @var array
	*/
	var $_current_attributes = Array();


	/**
	* Number of tags parsed
	*
	* @var int
	*/
	var $_passes = 0;


	/**
	* Result of: passes / number of required tags
	*
	* @var int
	*/
	var $_quotient = 0;


	/**
	* Current number of data record sets
	*
	* @var int
	*/
	var $_record_set_index = 0;


	/**
	* Constructor
	*
	* @param int    $assetid    the asset id to be loaded
	*
	*/
	function Data_Source_Xml($assetid=0)
	{
		$this->Data_Source($assetid);

	}//end constructor


	/**
	* Returns an array of the result of xml parsing either from cache or by parsing the file again
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
	* Used for parsing the XML file
	*
	* @return void
	* @access public
	*/
	function parseXML()
	{
		$url = html_entity_decode($this->attr('url'));

        $xml_parser = xml_parser_create();
        // stop the parser upper-casing everything
		xml_parser_set_option ($xml_parser, XML_OPTION_CASE_FOLDING, 0);
		xml_set_character_data_handler ($xml_parser, Array(&$this,'characterData') );
        xml_set_element_handler($xml_parser, Array(&$this,'startElement'), Array(&$this,'endElement'));

		// initialise file handler
		if (!($fp = fopen($url, 'r'))) {
			trigger_localised_error('CORE0257', E_USER_WARNING, $import_path);
			return FALSE;
			die();
		}

		// read the XML file 4KB at a time
		$block_size = 4096;
		while ($data = fread($fp, $block_size)) {
            if (!xml_parse($xml_parser, $data, feof($fp))) {
                trigger_localised_error('CORE0258', E_USER_WARNING, xml_error_string(xml_get_error_code($xml_parser)), htmlentities($data));
                return FALSE;
            }
		}

		// clean up
		fclose($fp);
		xml_parser_free($xml_parser);

		return $this->_result;

	}//end parseXML()


	/**
	* Handles the end of an element
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
