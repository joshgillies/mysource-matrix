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
* $Id: rss_feed_reader.inc,v 1.33 2013/10/21 04:53:06 ewang Exp $
*
*/


/**
* RSS_Feed_Reader
*
* Purpose
* Converts an xml document to different arrays of relative arrays
*
* @author  Pratibha Gannavarapu <pgannavarapu@squiz.net>
* @version $Revision: 1.33 $
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
	public $open_tags = Array();

	/**
	* The tags that have sub tags are pushed into this array to keep track of index
	*
	* @var array
	*/
	public $parent_tags = Array();

	/**
	* The items are stored in this array which is an associative array
	*
	* @var array
	*/
	public $item = Array();

	/**
	* The image info is stored in here
	*
	* @var array
	*/
	public $image = Array();

	/**
	* The Channel info of the RSS is saved in this array
	*
	* @var array
	*/
	public $channel = Array();

	/**
	* The text inpur of the RSS is saved in here
	*
	* @var array
	*/
	public $textinput = Array();

	/**
	* The attributes of the tag
	*
	* @var array
	*/
	public $attributes = Array();

	/**
	* Any of the unrecognised tags are pushed in to this array
	*
	* @var array
	*/
	public $unrecognised = Array();

	/**
	* The element that is currently being processed eg. items, channel, link, description etc.
	*
	* @var string
	*/
	public $element = '';

	/**
	* The file pointer to read the file
	*
	* @var array
	*/
	public $fp = NULL;

	/**
	* The parser
	*
	* @var array
	*/
	public $parser;

	/**
	* The type of RSS (RSS 1.0, RSS 2.0 or Atom 1.0)
	*
	* @var array
	*/
	public $type = '';

	/**
	* A flag to see if the file or the XML string has been parsed or not
	*
	* @var boolean
	*/
	public $parsed = FALSE;
	
	/**
	* delimeter
	*
	* @var string
	*/
	public $delimiter = ',';
	
	
	/**
	* if we are working on a new tag
	*
	* @var bool
	*/
	private $_new_tag;	


	/**
	* Constructor
	*
	*/
	function __construct()
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
	public function setInputFile($file='')
	{
		$check = FALSE;
		if (!empty($file)) {
			// Make sure it's compatible with new schemes
			//  feed:[/]
			//  feed:http://[/]
			//  feed://[/]
			//  feed://http://[/]
			$file = preg_replace('/^feed:(\/\/)?(http:\/\/)?/','http://',$file);

			if (preg_match('%^(http[s]*|ftp)://%', substr($file, 0, 10))) {
				if (!ini_get('allow_url_fopen')) {
					trigger_error('Remote Files Cannot Be Parsed', E_USER_WARNING);
					return FALSE;
				}
				require_once(SQ_INCLUDE_PATH.'/general_occasional.inc');
				$this->fp = open_socket_connection($file, $errno, $errstr);
			} else {
				$this->fp = fopen($file, 'r');
			}


			if (!$this->fp) {
				trigger_error('Error occured during opening the file '.$file, E_USER_WARNING);
				return FALSE;
			}
		} else {
			trigger_error('No File Path Or URL Entered', E_USER_ERROR);
			return FALSE;
		}
		return TRUE;

	}//end setInputFile()


	/**
	* Set the file pointer
	*
	* @param int	$xml	the pointer to the xml file
	*
	* @return boolean
	* @access public
	*/
	public function setInputString($xml)
	{
		if (empty($xml)) {
			trigger_error('Empty Strings Cannot Be Parsed', E_USER_ERROR);
			return FALSE;
		}
		if (!is_string($xml)) {
			trigger_error('Only XML Strings Can Be Parsed', E_USER_ERROR);
			return FALSE;
		}
		$this->fp = $xml;
		return TRUE;

	}//end setInputString()


	/**
	* Parsers the file
	*
	* Returns error message on error, TRUE on success.
	*
	* @return boolean|string
	* @access public
	*/
	public function parse()
	{
		$fp = $this->fp;
		if (!$fp) {
			trigger_error('Invalid file pointer', E_USER_WARNING);
			return 'Invalid file pointer';
		} else {
			$this->parsed = TRUE;
			if (!is_string($this->fp)) {
				$found_xml = FALSE;
				// Read the XML file 4KB at a time
				while (!feof($fp)) {
					$data = fread($fp, 4096);
					// find the XML so we can strip out possible http headers (when proxy is used)
					if (!$found_xml) {
						$data = explode('<?xml', $data);
						if (count($data) > 1) {
							array_shift($data);
							$data = '<?xml'.implode('<?xml', $data);
							$found_xml = TRUE;
						} else {
							// try find a valid XML start tag (in this order)
							$start_tags = Array ('rss', 'rdf', 'feed');
							foreach ($start_tags as $tag){
								$test_data = str_ireplace("<$tag", "<$tag", $data[0]);
								$test_data = explode("<$tag", $test_data);
								if (count($test_data) > 1) {
									array_shift($test_data);
									$data = "<$tag".implode("<$tag", $test_data);
									$found_xml = TRUE;
									break;
								}
							}
						}
					}

					if ($found_xml && !xml_parse($this->parser, $data)) {
						return 'Error in the feed';
					}
				}

				// still did not find the XML
				if (!$found_xml){
					return 'Error in the feed';
				}

			} else {
				if (!xml_parse($this->parser, $this->fp)) {
					return 'Error in the feed';
				}
			}
		}

		return TRUE;

	}//end parse()


	/**
	* Returns a string of the type of the RSS as e.g. rss_1.0/ rss_2.0
	*
	* @return array
	* @access public
	*/
	public function getRssType()
	{
		if ($this->_issetFile()) return $this->type;

	}//end getRssType()


	/**
	* Returns the array of all of the items
	*
	* @return array
	* @access public
	*/
	public function getItems()
	{
		if ($this->_issetFile()) return $this->item;

	}//end getItems()


	/**
	* Returns the array of textinput elements of the RSS
	*
	* @return array
	* @access public
	*/
	public function getTextInput()
	{
		if ($this->_issetFile()) return $this->textinput;

	}//end getTextInput()


	/**
	* returns the array ofchannel info
	*
	* @return array
	* @access public
	*/
	public function getChannelInfo()
	{
		if ($this->_issetFile()) return $this->channel;

	}//end getChannelInfo()


	/**
	* Returns the array of image info
	*
	* @return array
	* @access public
	*/
	public function getImageInfo()
	{
		if ($this->_issetFile()) return $this->image;

	}//end getImageInfo()


	/**
	* Returns the array of all of the unrecognised elements
	*
	* @return array
	* @access public
	*/
	public function getUnrecognisedElements()
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
	public function startElementHandler($xp, $name, $attributes)
	{
		if (!empty($name)) {
			// for atom the tags are feed and entry for channel and item respectivel. This has to be renamed as channel and
			// item so that we can use the proper class variables
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
					$this->type .= isset($attributes['VERSION']) ? $attributes['VERSION'] : '';
				break;
				case 'RDF':
					$this->type = 'rss_1.0';
				break;
			}
			$this->open_tags[] = $name;
			// if there is a tag that is previously opened but not closed till now then the current tag is its child
			// so push the parent into the parent_tags array
			$this->parent_tags[] = $name;

			$this->element = $name;
			// if teh current tag has any attributes then pass the attributes to the character handler so that it can be pushed
			// into the respective array as attribute
			if (!empty($attributes)) {
				$this->attributes = $attributes;
				$this->characterHandler($xp, $this->attributes, TRUE);
			}
			// we started a new tag
			$this->_new_tag = TRUE;
		}//end if

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
	public function endElementHandler($xp, $name)
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
			// unset($this->open_tags[$key]);
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
	public function characterHandler($xp, $data, $attributes=FALSE)
	{
		$index = NULL;
		if ((is_array($data) && !empty($data)) || (is_string($data) && trim($data) != '')) {
			$element = $this->element;
			$var = NULL;
			// get the key for the current element in the open tags and get its parents from teh open tags
			// for e.g. open tags could be array('CHANNEL', 'ITEM', 'LINK')
			// if the current element is link then in the above example it has to be pplaced in to the ITEM array
			// as its first set parent which is also a class variable is Item
			// Hence $var is item
			// the parent is the if the current tag has any parent that is not var. For example.. if the current tags is name
			// open tags is Array('CHANNEL', 'AUTHOR', 'NAME'). in this example $parent is author as name is a subtag
			// and var is CHANNEL
			// if teh tag is an unrecognised tag then the var is unrecognised
			
			$key = array_search($element, $this->open_tags);
			if($key === FALSE)
			    $key = -1;
			$class_vars = get_class_vars(get_class($this));
			for ( ; $key != -1; $key--) {
				if (isset($class_vars[strtolower($this->open_tags[$key])]) && ($this->open_tags[$key] != 'TYPE')) {
					break;
				}
			}
			$parent = NULL;
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
			
			// if the var is unrecognised then push the element intot he unrecognised array
			if ($var == 'unrecognised') {
				// get the index where the data has to be inserted in the unrecognised array
				// group the child tags together basing upon how many times the parent tags has repeated
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
				// if attributes is not true then this is a value else we are dealing with the value of attributes
				if (!$attributes) {
					$this->{$var}[$parent][$index][$element]['value'] = $data;
				} else {
					$this->{$var}[$parent][$index][$element]['_attributes'] = $data;
				}
			} else {
				// RSS feeds should only have one LINK. But Atom may have multiple, with different REL attributes ('enclosure', 'alternate').
				// Since multiple links are not part of the design here, ensure we only get 'alternate' links if attributes are provided.
				// This is essentially a hack, but provides better Atom support than without it.
				// @see http://www.atomenabled.org/developers/syndication/atom-format-spec.php#element.link
				// @see http://bugs.matrix.squiz.net/view_bug.php?bug_id=4700
				if ($element == 'LINK') {
					if ((is_array ($data)) && (array_key_exists('REL', $data)) && ($data['REL'] != 'alternate')) return;
				}

				// if the array exists and it is not empty
				// then we get the index in which this data has to be inserted
				if (!empty($this->{$var})) {
					// if this var is in parents array then check how many times it is in array
					// depending upon that we could get theo index wherte this element has to be inserted
					// if it is not in parents tag then this is the first time we are pushing the element in to the repsective array
					// so the index is 0
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
				// if this element is not in parent tags and if this is not empty then we can use the data
				if (!empty($element)) {
					$element = strtolower($element);
					// if we are in new tag, it means it's duplicate tags, use delimeter to merge them
					// otheriwise it could be in tag, this func just called multiple times
					$delimeter = $this->_new_tag ? $this->delimiter : '';
					// if the current element is already in the array then we have to append the current data to the already existing data
					// this happens when the data has tags in it.
					if (empty($parent)) {
						if (isset($this->{$var}[$index][$element]['value']) && (!$attributes)) {
							$value = $this->{$var}[$index][$element]['value'];
							$data = $value.$delimeter.$data;
						}
					} else {
						if (isset($this->{$var}[$index][$parent][$element]['value']) && (!$attributes)) {
							$value = $this->{$var}[$index][$parent][$element]['value'];
							$data = $value.$delimeter.$data;
						}
					}
					// if it is value that we are dealing with then store it in the array with the 'value' key
					// otherwise then it is an attribute we are trying to store. So we store it in the right place
					// with the '_attributes' key
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
				}//end if element not empty
			}//end else where var is unrecognised or not
		}//end if data not empty

		// already processed this tag
		$this->_new_tag = FALSE;
	}//end characterHandler()


	/**
	* Checks if the file pointer is set or not
	*
	* @return boolean
	* @access public
	*/
	public function _issetFile()
	{
		if (!isset($this->fp)) {
			trigger_error('The XML That Has To Be Parsed Is To Be Set First Using setInputFile or setInputString');
			return FALSE;
		}
		if (!$this->parsed) {
			trigger_error('The XML That Has To Be Parsed First Using parse() Function');
			return FALSE;
		}
		return TRUE;

	}//end _issetFile()


//--        END OF INTERNAL FUNCTIONS        --//


}//end class


?>
