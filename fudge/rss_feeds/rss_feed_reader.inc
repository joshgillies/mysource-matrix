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
* $Id: rss_feed_reader.inc,v 1.8 2006/03/29 00:54:36 pgannavarapu Exp $
*
*/


/**
* RSS_Feed_Reader
*
* Purpose
* Converts an xml document to different arrays of relative arrays
*
* @author  Pratibha Gannavarapu <pgannavarapu@squiz.net>
* @version $Revision: 1.8 $
* @package Fudge
* @subpackage RSS_Feeds
*/
class RSS_Feed_Reader
{

	/**
	* The currently open tags that are still awaiting to be closed
	*
	* @var array
	*/
	var $open_tags = Array();

	/**
	* The tags that have sub tags are pushed into this array to keep track of index
	*
	* @var array
	*/
	var $parent_tags = Array();

	/**
	* The items are stored in this array which is an associative array
	*
	* @var array
	*/
	var $item = Array();

	/**
	* The image info is stored in here
	*
	* @var array
	*/
	var $image = Array();

	/**
	* The Channel info of the RSS is saved in this array
	*
	* @var array
	*/
	var $channel = Array();

	/**
	* The text inpur of the RSS is saved in here
	*
	* @var array
	*/
	var $textinput = Array();

	/**
	* The attributes of the tag
	*
	* @var array
	*/
	var $attributes = Array();

	/**
	* Any of the unrecognised tags are pushed in to this array
	*
	* @var array
	*/
	var $unrecognised = Array();

	/**
	* The element that is currently being processed eg. items, channel, link, description etc.
	*
	* @var string
	*/
	var $element = '';

	/**
	* The file pointer to read the file
	*
	* @var array
	*/
	var $fp = null;

	/**
	* The parser
	*
	* @var array
	*/
	var $parser;

	/**
	* The type of RSS (RSS 1.0, RSS 2.0 or Atom 1.0)
	*
	* @var array
	*/
	var $type = '';

	/**
	* A flag to see if the file or the XML string has been parsed or not
	*
	* @var boolean
	*/
	var $parsed = false;


	/**
	* Constructor
	*
	*/
	function RSS_Feed_Reader()
	{
		$this->parser = xml_parser_create();

		xml_set_object($this->parser, $this);
		xml_set_element_handler($this->parser, 'startElementHandler', 'endElementHandler');
		xml_set_character_data_handler($this->parser, 'characterHandler');

	}//end constructor


	/**
	* Accepts the path of the file to be parsed and sets the file pointer
	*
	* @param string	$file	the path of the file to be parsed
	*
	* @return boolean
	* @access public
	*/
	function setInputFile($file='')
	{
		$check = false;
		if (!empty($file)) {
			if (eregi('^(http|ftp)://', substr($file, 0, 10))) {
				if (!ini_get('allow_url_fopen')) {
					trigger_error('Remote Files Cannot Be Parsed', E_USER_WARNING);
					return false;
				}
			}
			$this->fp = fopen($file, 'r');
			if (!$this->fp) {
				trigger_error('Error occured during opening the file', E_USER_WARNING);
				return false;
			}
		} else {
			trigger_error('No File Path Or URL Entered', E_USER_ERROR);
			return false;
		}
		return true;

	}//end setInputFile()


	/**
	* Set the file pointer
	*
	* @param int	$xml	the pointer to the xml file
	*
	* @return boolean
	* @access public
	*/
	function setInputString($xml)
	{
		if (empty($xml)) {
			trigger_error('Empty Strings Cannot Be Parsed', E_USER_ERROR);
			return false;
		}
		if (!is_string($xml)) {
			trigger_error('Only XML Strings Can Be Parsed', E_USER_ERROR);
			return false;
		}
		$this->fp = $xml;
		return true;

	}//end setInputString()


	/**
	* Parsers the file
	*
	* @return string
	* @access public
	*/
	function parse()
	{
		$fp = $this->fp;
		if (!$fp) {
			trigger_error('Invalid File Pointer', E_USER_WARNING);
			return;
		} else {
			$this->parsed = true;
			if (!is_string($this->fp)) {
				while ($data = fread($fp, 4096)) {
					if (!xml_parse($this->parser, $data)) {
						return 'Error in the feed';
					}
				}
			} else {
				if (!xml_parse($this->parser, $this->fp)) {
					return 'Error in the feed';
				}
			}
		}

	}//end parse()


	/**
	* Returns a string of the type of the RSS as e.g. rss_1.0/ rss_2.0
	*
	* @return array
	* @access public
	*/
	function getRssType()
	{
		if ($this->_issetFile()) return $this->type;

	}//end getRssType()


	/**
	* Returns the array of all of the items
	*
	* @return array
	* @access public
	*/
	function getItems()
	{
		if ($this->_issetFile()) return $this->item;

	}//end getItems()


	/**
	* Returns the array of textinput elements of the RSS
	*
	* @return array
	* @access public
	*/
	function getTextInput()
	{
		if ($this->_issetFile()) return $this->textinput;

	}//end getTextInput()


	/**
	* returns the array ofchannel info
	*
	* @return array
	* @access public
	*/
	function getChannelInfo()
	{
		if ($this->_issetFile()) return $this->channel;

	}//end getChannelInfo()


	/**
	* Returns the array of image info
	*
	* @return array
	* @access public
	*/
	function getImageInfo()
	{
		if ($this->_issetFile()) return $this->image;

	}//end getImageInfo()


	/**
	* Returns the array of all of the unrecognised elements
	*
	* @return array
	* @access public
	*/
	function getUnrecognisedElements()
	{
		if ($this->_issetFile()) return $this->unrecognised;

	}//end getUnrecognisedElements()


//--        INTERNAL FUNCTIONS FOR HANDLING TAGS AND DATA        --//


	//--	THIS HANDLES OPENING TAGS	--//


	/**
	* Handles the start if an element
	*
	* @param object	$xp			the xml parser
	* @param string	$name		name of the element
	* @param array	$attributes	the attributes
	*
	* @return void
	* @access public
	*/
	function startElementHandler($xp, $name, $attributes)
	{
		if (!empty($name)) {
			//for atom the tags are feed and entry for channel and item respectivel. This has to be renamed as channel and
			//item so that we can use the proper class variables
			switch ($name) {
				case 'ENTRY':
					$name = 'ITEM';
				break;
				case 'FEED':
					$this->type = 'atom_1.0';
					$name = 'CHANNEL';
				break;
				case 'RSS':
					$this->type = 'rss_';
					$this->type .= isset($attributes['VERSION'])?$attributes['VERSION']:'';
				break;
				case 'RDF':
					$this->type = 'rss_1.0';
				break;
			}
			$this->open_tags[] = $name;
			//if there is a tag that is previously opened but not closed till now then the current tag is its child
			//so push the parent into the parent_tags array
			$this->parent_tags[] = $name;

			$this->element = $name;
			//if teh current tag has any attributes then pass the attributes to the character handler so that it can be pushed
			//into the respective array as attribute
			if (!empty($attributes)) {
				$this->attributes = $attributes;
				$this->characterHandler($xp, $this->attributes, true);
			}
		}

	}//end startElementHandler()


	//--	THIS HANDLES CLOSING TAGS	--//


	/**
	* Handles the end of an element
	*
	* @param object	$xp		the xml parser
	* @param string	$name	name of the element
	*
	* @return void
	* @access public
	*/
	function endElementHandler($xp, $name)
	{
		switch ($name) {
			case 'ENTRY':
				$name = 'ITEM';
			break;
			case 'FEED':
				$name = 'CHANNEL';
			break;
		}
		if ($key = array_search($name, $this->open_tags)) {
			if ($key == count($this->open_tags)-1) {
				$element = array_pop($this->open_tags);
			}
			//unset($this->open_tags[$key]);
			$count = count($this->parent_tags);

			if ($this->element == $this->parent_tags[$count-1]) {
				$element = array_pop($this->parent_tags);
			}
		}

	}//end endElementHandler()


	//--	THIS HANDLES DATA	--//


	/**
	* Handles the data
	*
	* @param object		$xp			the xml parser
	* @param string		$data		data inside the element
	* @param boolean	$attributes	default of false but becomes true when we are calling this to assign attributes
	*
	* @return void
	* @access public
	*/
	function characterHandler($xp, $data, $attributes=false)
	{
		$index = null;
		//if the data is a string and has any white spaces then trim them off
		if (is_string($data)) $data = trim($data);
		if (!empty($data)) {
			$element = $this->element;
			$var = null;
			//get the key for the current element in the open tags and get its parents from teh open tags
			//for e.g. open tags could be array('CHANNEL', 'ITEM', 'LINK')
			//if the current element is link then in the above example it has to be pplaced in to the ITEM array
			//as its first set parent which is also a class variable is Item
			//Hence $var is item
			//the parent is the if the current tag has any parent that is not var. For example.. if the current tags is name
			//open tags is Array('CHANNEL', 'AUTHOR', 'NAME'). in this example $parent is author as name is a subtag
			//and var is CHANNEL
			//if teh tag is an unrecognised tag then the var is unrecognised
			$key = array_search($element, $this->open_tags);

			$class_vars = get_class_vars(get_class($this));
			for ( ; $key != -1; $key--) {
				if (isset($class_vars[strtolower($this->open_tags[$key])])) {
					break;
				}
			}
			$parent = null;
			if ($key != -1) {
				$var = strtolower($this->open_tags[$key]);
				$key = array_search($element, $this->open_tags) - 1;
				if (isset($this->open_tags[$key])) {
					if (strcasecmp($this->open_tags[$key], $var) != 0) {
						$parent = strtolower($this->open_tags[$key]);
					}
				}
			} else {
				$var = 'unrecognised';
				$key = array_search($element, $this->open_tags) - 1;
				if (isset($this->open_tags[$key])) {
					$parent = strtolower($this->open_tags[$key]);
				} else {
					$parent = strtolower($element);
				}
			}
			//if the var is unrecognised then push the element intot he unrecognised array
			if ($var == 'unrecognised') {
				//get the index where the data has to be inserted in the unrecognised array
				//group the child tags together basing upon how many times the parent tags has repeated
				if (!empty($this->{$var})) {
					$required = $this->{$var};
					if (array_search(strtoupper($parent), $this->parent_tags)) {
						$keys = Array();
						foreach ($this->parent_tags as $key => $name) {
							if (strcasecmp($name, $parent) == 0) $keys[] = $key;
						}
						$index = count($keys) - 1;
					} else {
						$index = 0;
					}
				} else {
					$index = 0;
				}
				//if attributes is not true then this is a value else we are dealing with the value of attributes
				if (!$attributes) {
					$this->{$var}[$parent][$index][$element]['value'] = $data;
				} else {
					$this->{$var}[$parent][$index][$element]['_attributes'] = $data;
				}
			} else {
				//if the array exists and it is not empty
				//then we get the index in which this data has to be inserted
				if (!empty($this->{$var})) {
					//if this var is in parents array then check how many times it is in array
					//depending upon that we could get theo index wherte this element has to be inserted
					//if it is not in parents tag then this is the first time we are pushing the element in to the repsective array
					//so the index is 0
					if (array_search(strtoupper($var), $this->parent_tags)) {
						$count = 0;
						foreach ($this->parent_tags as $key => $value) {
							if (strcasecmp($value, $var) == 0) $count++;
						}

						$index = $count - 1;

					} else {
						$index = 0;
					}
				} else {
					$index = 0;
				}
				//if this element is not in parent tags and if this is not empty then we can use the data
				if (!empty($element)) {
					$element = strtolower($element);
					//if the current element is already in the array then we have to append the current data to the already existing data
					//this happens when the data has tags in it.
					if (empty($parent)) {
						if (isset($this->{$var}[$index][$element]['value']) && (!$attributes)) {
							$value = $this->{$var}[$index][$element]['value'];
							$data = $value.$data;
						}
					} else {
						if (isset($this->{$var}[$index][$parent][$element]['value']) && (!$attributes)) {
							$value = $this->{$var}[$index][$parent][$element]['value'];
							$data = $value.$data;
						}
					}
					//if it is value that we are dealing with then store it in the array with the 'value' key
					//otherwise then it is an attribute we are trying to store. So we store it in the right place
					//with the '_attributes' key
					if (!$attributes) {
						if (empty($parent)) {
							$this->{$var}[$index][$element]['value'] = $data;
						} else {
							$this->{$var}[$index][$parent][$element]['value'] = $data;
						}
					} else {

						if ($var != $element) {
							if (empty($parent)) {
								$this->{$var}[$index][$element]['_attributes'] = $data;
							} else {
								$this->{$var}[$index][$parent][$element]['_attributes'] = $data;
							}
						} else {
							$this->{$var}[$index]['_attributes'] = $data;
						}
					}
				} //end if element not empty
			}//end if where var is unrecognised or not
		}//end if data not empty

	}//end characterHandler()


	/**
	* Checks if the file pointer is set or not
	*
	* @return boolean
	* @access public
	*/
	function _issetFile()
	{
		if(!isset($this->fp)) {
			trigger_error('The XML That Has To Be Parsed Is To Be Set First Using setInputFile or setInputString');
			return false;
		}
		if (!$this->parsed) {
			trigger_error('The XML That Has To Be Parsed First Using parse() Function');
			return false;
		}
		return true;

	}//end _issetFile()


//--        END OF INTERNAL FUNCTIONS        --//


}//end class


?>
