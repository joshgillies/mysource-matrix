<?php

# From PEAR
require_once 'XML/Parser.php';

/**
* XML_Array
*
* Purpose
*
*    Allows the creation of PHP array's from XML source's
*
*    ** Thanks goes to the authors of PEAR::XML_Tree for **
*    ** their great example on how to use the XML_Parser **
*
* Example
*
*    $array_paths = Array('root/elem_two_list/elem_two_item'); # for more info on this see $array_paths var below
*    $xml_arr = new XML_Array($array_paths);
*    $array   = $xml_arr->getArrayFromFile($file);
*    print_r($array);
*
* @author  Blair Robertson <blair@squiz.net>
* @package XML
* @version $Version$ - 1.0
*/
class XML_Array extends XML_Parser
{

	/**
	* The array of paths into the XML, each of which has the potential to occur multiple times
	* eg.
	*   <root>
	*       <elem_one>Blah</elem_one>
	*       <elem_two_list>
	*           <elem_two_item>Example Text</elem_two_item>
	*           <elem_two_item>MoreExample Text</elem_two_item>
	*       </elem_two_list>
	*   </root>
	* here 'root/elem_two_list/elem_two_item' would one value in $array_paths
	*
	*
	* @var  Array
	*/
	var $_array_paths = Array();

	/**
	* Results array
	*
	* @var  Array
	*/
	var $_results = Array();

	/**
	* Stack used in parsing
	*
	* @var Array(string)
	*/
	var $_path = Array();

	/**
	* pointers to the current path elements in the array
	*
	* @var Array(&Array())
	*/
	var $_path_ptrs = Array();

	/**
	* Stack used in parsing to determine type (simple/complex) of current element
	*
	* @var Array(char)
	*/
	var $_path_ptrs = Array();


	/**
	* Storage unit used in parsing
	*
	* @var string
	*/
	var $_cdata = '';

	/**
	* Constructor
	*
	* @param array $defs
	*/
	function XML_Array($arr_paths=Array()) {
		$this->setArrayPaths($arr_paths);
	}


	/*
	* Sets the array paths
	*
	* @access public
	*/
	function setArrayPaths ($arr_paths)
	{
		$this->_array_paths = $arr_paths;
	}

	/*
	* Maps a xml file to a objects tree
	*
	* @return mixed The objects tree (XML_tree or an Pear error)
	* @access public
	*/
	function &getArrayFromFile ($filename, $arr_paths=Array())
	{
		if (count($arr_paths)) $this->setArrayPaths($arr_paths);

		$this->_initParse();

		$err = $this->setInputFile($filename);
		if (PEAR::isError($err)) {
			return $err;
		}

		$err = $this->parse();
		if (PEAR::isError($err)) {
			return $err;
		}


		return $this->_results;
	}

	function getArrayFromString($str, $defs=Array())
	{
		if (count($arr_paths)) $this->setArrayPaths($arr_paths);

		$this->_initParse();

		$err = $this->parseString($str);
		if (PEAR::isError($err)) {
			return $err;
		}
		return $this->_results;
	}

	/*
	* Do some initialisation before any parsing
	*
	* @access private
	*/
	function _initParse()
	{
		$this->close();

		$this->folding = false;
		$this->XML_Parser(null, 'event');

		$this->_results     = '';
		$this->_cdata      = '';
		$this->_path       = Array();
		$this->_path_ptrs  = Array();
		$this->_path_types = Array();

	}

	/**
	* Handler for the xml-data
	*
	* @param mixed  $xp         ignored
	* @param string $elem       name of the element
	* @param array  $attribs    attributes for the generated node
	*
	* @access private
	*/
	function startHandler($xp, $elem, &$attribs)
	{

		# root element ?
		if (!count($this->_path)) {
			$this->_path[]       = $elem;
			$this->_path_ptrs[]  = &$this->_results;
			$this->_path_types[] = 's';

		# normal element
		} else {

			# get a ref to the parent element
			$parent = &$this->_path_ptrs[count($this->_path_ptrs) - 1];

			# we aren't going to deal with  mixed contents for the moment
			//if (!empty($this->cdata)) {
			//  $parent_id = 'obj' . ($this->i - 1);
			//  $parent    =& $this->$parent_id;
			//  $parent->children[] = &new $this->_node_class(null, $this->cdata);
			//}

			# create this elements entry, need to check 
			# if it exists here so that we don't overright multiple entry
			# elements contents
			if (!isset($parent[$elem]) || !is_array($parent[$elem])) {
				$parent[$elem] = Array();
			}

			# OK, now because we have a parent, then that parent is a compound element, comprenday ?
			$this->_path_types[count($this->_path_types) - 1] = 'c';

			$this->_path[]       = $elem;
			$this->_path_types[] = 's'; # every element start's out simple :)
			$this->_path_ptrs[]  = &$parent[$elem];
			
			# OK, this element in one of possibly many with the same name
			# so what we have to do is create an indexed array with each element under it
			$path_str = implode('/', $this->_path);
			if (in_array($path_str, $this->_array_paths)) {
				$i = count($parent[$elem]);
				$parent[$elem][$i]  = Array();

				$this->_path_ptrs[] = &$parent[$elem][$i];

			}

		}

		# Now add the attributes to the element, which is the one
		# on the top of the ptrs stack
		$i = count($this->_path_ptrs) - 1;
		for(reset($attribs); null !== ($k = key($attribs)); next($attribs)) {
			$this->_path_ptrs[$i]['@'.$k] = $attribs[$k];
		}

		$this->_cdata = '';

		return null;
	}

	/**
	* Handler for the xml-data
	*
	* @param mixed  $xp         ignored
	* @param string $elem       name of the element
	*
	* @access private
	*/
	function endHandler($xp, $elem)
	{

		# if we aren't the root element, then add any data necessary
		if (count($this->_path) > 1) {


			# if this element is a simple element (is just data), add the value
			if ($this->_path_types[count($this->_path_types) - 1] == 's' && $this->_cdata != '') {
				$this->_path_ptrs[count($this->_path_ptrs) - 1]['value'] = $this->_cdata;
			}

			# mixed contents
			//if (count($node->children) > 0)   {
			//  if (trim($this->cdata)) {
			//      $node->children[] = &new $this->_node_class(null, $this->cdata);
			//  }
			//} else {
			//  $node->set_content($this->cdata);
			//}

		}#end if

		# OK, this element in one of possibly many with the same name
		# so what we need to pop 2 elements of the ptr's array 
		# because we put 2 on in startHandler
		$path_str = implode('/', $this->_path);
		if (in_array($path_str, $this->_array_paths)) {
			array_pop($this->_path_ptrs);
		}
		array_pop($this->_path_ptrs);
		array_pop($this->_path_types);
		array_pop($this->_path);


		$this->_cdata = '';
		return null;

	}

	/*
	* The xml character data handler
	*
	* @param mixed  $xp         ignored
	* @param string $data       PCDATA between tags
	*
	* @access private
	*/
	function cdataHandler($xp, $data)
	{
		if (trim($data) != '') {
			$this->_cdata .= $data;
		}
	}

	/*
	* Close the parser and do any cleanup that is necessary
	*
	* @access public
	*/
	function close()
	{
		if (is_resource($this->parser)) xml_parser_free($this->parser);
	}


}
?>
