<?php
/**
* +--------------------------------------------------------------------+
* | This MySource Matrix CMS file is Copyright (c) Squiz Pty Ltd       |
* | ACN 084 670 600                                                    |
* +--------------------------------------------------------------------+
* | IMPORTANT: Your use of this Software is subject to the terms of    |
* | the Licence provided in the file licence.txt. If you cannot find   |
* | this file please contact Squiz (www.squiz.net) so we may provide   |
* | you a copy.                                                        |
* +--------------------------------------------------------------------+
*
* $Id: xml_converter.inc,v 1.6 2006/12/06 05:14:24 bcaldwell Exp $
*
*/

require_once 'XML/Parser.php';

/**
* XML_Converter
*
* Purpose
* Converts an xml document from a string to an array or from an array to a
* string
*
* <root>
*  		<b attr="val">vb1</b>
* 		<b>vb2</b>
* 		<c> <d/> </c>
* </root>
*
* is equivalent to:
*
*		Array (
*		    'root' => Array (
*		        Array (
*		            'b' => Array (
*		                        Array('@attr' => val, '-content' => vb1),
*		                        Array('-content' => vb2)
*		                	),
*		            'c' => Array (
*		                    	Array ('d' => Array (Array ()))
*		                	)
*		       	)
*		    )
*		)
*
*
* @author  Geoffroy Noel <gnoel@squiz.co.uk>
* @version $Revision: 1.6 $
* @package Fudge
* @subpackage General
*/
class XML_Converter extends XML_Parser
{

	/**
	* Holds the last converted xml array
	*
	* @var Array
	*/
	var $array_xml;


	/**
	* Tells whether the conversion to a string includes indentation
	*
	* @var boolean
	*/
	var $indent_xml = false;


	/**
	* Generates compact arrays (Not implemented yet)
	*
	* @var boolean
	*/
	var $compact_array = false;


	/**
	* Level of recursivity during conversion
	* @see _getStringFromArrayInternal
	*
	* @var int
	*/
	var $tag_level = 0;


	/**
	* Converts a xml-array into a xml-string
	*
	* @param array()	$xml		xml-array to convert
	* @param boolean	$no_header	if true, add a xml header
	*
	* @return string
	* @access public
	*/
	function getStringFromArray($xml=NULL, $no_header=false)
	{
		if (!isset($xml)) $xml = &$this->array_xml;

		$this->tag_level = 0;

		$ret = $this->_getStringFromArrayInternal($xml);

		if (!$no_header) {
			$ret = '<?xml version="1.0" encoding="utf-8"?>'.$ret;
		}

		return $ret;

	}//end getStringFromArray()


	/**
	* Converts a file containing a xml-string into a xml-array
	*
	* @param string	$xml_file	file name
	*
	* @return array()
	* @access public
	*/
	function getArrayFromFile($xml_file)
	{
		$ret = NULL;

		$xml = file_get_contents($xml_file);

		if ($xml !== false) {
			$ret = $this->getArrayFromString($xml);
		}

		return $ret;

	}//end getArrayFromFile()


	/**
	* Converts a xml-string into a xml-array
	*
	* @param string	$xml	a well formed xml-string to convert
	*
	* @return array()
	* @access public
	*/
	function getArrayFromString($xml)
	{
		$this->_reset();

		$this->parseString($xml);

		return $this->array_xml[0];

	}//end getArrayFromString()


	/**
	* Returns the content of an element
	*
	* @param array()	&$parent		the parent element as a xml-array
	* @param string		$element_name	element name
	* @param boolean	$default_value	default value if the element doesn't
	*									exists
	*
	* @return string
	* @access public
	*/
	function getElementContent(&$parent, $element_name, $default_value=NULL)
	{
		$ret = $default_value;

		if (isset($parent[$element_name][0]['-content'])) {
			$ret = $parent[$element_name][0]['-content'];
		}

		return $ret;

	}//end getElementContent()


	/**
	* Internal function to converts an xml-array into a string
	*
	* @param mixed	$xml		part of a xml-array
	* @param string	$parent_tag	name of the parent element
	*
	* @return array()
	* @access private
	*/
	function _getStringFromArrayInternal($xml=NULL, $parent_tag=0)
	{
		$is_tag_open = false;
		$attributes = '';
		$rc = "\n".str_repeat("\t", $this->tag_level ? $this->tag_level - 1 : 0);
		$optional_rc = '';
		$ret = '';

		if (is_array($xml)) {
			foreach ($xml as $xml_key => $xml_val) {

				$is_tag_open |= !is_integer($xml_key);

				if (is_array($xml_val)) {
					// empty element (no attr or subelement), we force to print the tag name
					if (count($xml_val)) {
						$optional_rc = $rc;
						$tag_name = $xml_key;

						if (is_integer($xml_key)) {
							$tag_name = $parent_tag;
						}
						else {
							$this->tag_level++;
						}

						$ret .= $this->_getStringFromArrayInternal($xml_val, $tag_name);

						if (!is_integer($xml_key)) $this->tag_level--;
					}
					else {
						$is_tag_open = true;
					}
				}
				else {
					if ($xml_key == '-content') {
						//$ret .= htmlentities($xml_val);
						$xml_val = str_replace('&', '&amp;', $xml_val);
						$ret .= $xml_val;
					}
					else if ($xml_key{0} == '@') {
						$xml_key = substr($xml_key, 1);
						$attributes .= " $xml_key=\"$xml_val\"";
					}
					else {
						$optional_rc = $rc;
						$ret .= "<$xml_key>$xml_val</$xml_key>";
					}
				}
			}
		}

		if ($is_tag_open && $parent_tag) {
			$ret = "$rc<$parent_tag$attributes>$ret$optional_rc</$parent_tag>";
		}

		return $ret;

	}//end _getStringFromArrayInternal()


	/**
	* clean up object
	*
	* @return void
	* @access public
	*/
	function close()
	{
		if (!empty($this->parser) && is_resource($this->parser)) {
			xml_parser_free($this->parser);
		}

	}//end close()


	/**
	* reset object, prepare it to be used
	*
	* @return void
	* @access private
	*/
	function _reset()
	{
		$this->close();

		$this->folding = false;
		$this->XML_Parser(null, 'event');

		$this->array_xml = Array(Array());

	}//end _reset()


	/**
	* Start element event handler for the XML parser
	*
	* @return void
	* @access private
	*/
	function startHandler($xp, $elem, &$attribs)
	{
		$new_tag = Array();

		foreach ($attribs as $attrib_name => $attrib_value) {
			$new_tag['@'.$attrib_name] = $attrib_value;
		}

		$this->array_xml[] = &$new_tag;

	}//end startHandler()


	/**
	* End element event handler for the XML parser
	*
	* @return void
	* @access private
	*/
	function endHandler($xp, $elem)
	{
		$element_index = count($this->array_xml) - 1;
		$element = &$this->array_xml[$element_index];
		if (isset($element['-content'])) {
			$element['-content'] = trim($element['-content']);
			if ($element['-content'] == '') {
				unset($this->array_xml[$element_index]['-content']);
			}
		}

		$this->array_xml[$element_index - 1][$elem][] = $element;

		array_pop($this->array_xml);

	}//end endHandler()


	/**
	* Element Content event handler for the XML parser
	* No need to decode entities, the default php parser already does it.
	*
	* @return void
	* @access private
	*/
	function cdataHandler($xp, $data)
	{
		if (isset($data)) {
			$element = &$this->array_xml[count($this->array_xml) - 1];
			if (!isset($element['-content'])) {
				$element['-content'] = '';
			}
			$element['-content'] .= $data;
		}

	}//end cdataHandler()


}//end class


?>
