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
* $Id: asset_attribute.inc,v 1.23.2.1 2004/02/18 11:39:06 brobertson Exp $
* $Name: not supported by cvs2svn $
*/


/**
* Asset Attribute
*
* Purpose
*
*
* @author  Blair Robertson <blair@squiz.net>
* @version $Version$ - 1.0
* @package MySource_Matrix
*/
class Asset_Attribute extends MySource_Object
{

	/**
	* The unique ID for this attribute
	* @var int
	*/
	var $id = 0;

	/**
	* The name of this attribute
	* @var string
	*/
	var $name = '';

	/**
	* Whether this attribute value is unique
	* @var boolean
	*/
	var $uniq = false;

	/**
	* The description of this attribute
	* @var string
	*/
	var $description = '';

	/**
	* Parameters for processing/calculating/validating this attribute
	* @var Array()
	*/
	var $_params = Array();

	/**
	* The default value for this attribute
	* @var mixed
	*/
	var $_default_value = null;

	/**
	* Parameters for displaying this attribute
	* @var Array()
	*/
	var $_edit_params = Array();

	/**
	* The current value for this instance of this attribute
	* @var mixed
	*/
	var $value = null;

	/**
	* Whether or not this attribute has processed the results of a form post successfully
	* @var mixed
	*/
	var $processed = false;

	/**
	* Constructor
	*
	* @param int	$attributeid	the attribute id to be loaded
	* @param mixed	$value			the current value for the element
	* @param Array	$default_params	the default values for the params
	*
	*/
	function Asset_Attribute($attributeid=0, $value=null, $default_params=Array())
	{
		$this->MySource_Object();

		if ($attributeid) {
			$db = &$GLOBALS['SQ_SYSTEM']->db;

			$sql = 'SELECT attributeid, name, type, uniq, description, parameters_value, default_value
					FROM '.SQ_TABLE_PREFIX.'asset_attribute
					WHERE attributeid = '.$db->quote($attributeid);

			$result = $db->getRow($sql, null, DB_FETCHMODE_ORDERED);
			if (DB::isError($result)) trigger_error($result->getMessage().'<br/>'.$result->getUserInfo(), E_USER_ERROR);

			if (is_null($result)) return;

			list($this->id, $this->name, $type, $this->uniq, $this->description, $parameters, $this->_default_value) = $result;

			// make sure the asset we are loading is of the same type as our class
			if ($type != $this->type()) {
				trigger_error('Attribute #'.$attributeid.' is not a '.$this->type(), E_USER_WARNING);
				$this->id = null;
				return;
			}

			$this->_params = ($parameters) ? unserialize($parameters) : Array();
			unset($result);

		}

		$this->value = $value;

		// now just make sure some default values are set
		foreach($default_params as $name => $default) {
			if (!isset($this->_params[$name])) $this->_params[$name] = $default;
		}

	}//end contructor


	/**
	*
	* Retrieves subclasses of Asset_Attribute
	*
	* @static
	* @access public
	*/
	function loadAttribute($attributeid=0, $value=null, $default_params=Array())
	{
		if ($attributeid == 0) {
			return new Asset_Attribute(0, $value, $default_params);;
		}

		$db = &$GLOBALS['SQ_SYSTEM']->db;

		$sql = 'SELECT type FROM '.SQ_TABLE_PREFIX.'asset_attribute
				WHERE attributeid = '.$db->quote($attributeid);

		$type = $db->getOne($sql);
		if (DB::isError($type)) {
			trigger_error('Could not retrieve type for Attribute #'.$attributeid, E_USER_WARNING);
			return null;
		}

		$class = 'Asset_Attribute_'.$type;
		require_once SQ_ATTRIBUTES_PATH.'/'.$type.'/'.$type.'.inc';
		return new $class($attributeid, $value, $default_params);

	}//end loadAttribute()


	/**
	* Returns the type of this attribute
	*
	* @access public
	*/
	function type()
	{
		// strip the 'Asset_Attribute_' off the front
		return substr(get_class($this), 16);

	}//end type();


	/**
	* Prints the interface for filling in a value
	*
	* @param string		$prefix		prefix for the form element
	* @param boolean	$read_only	are we just printing the value
	*
	* @access public
	*/
	function paint($prefix, $read_only=false)
	{
		echo 'Interface for '.ucwords(str_replace('_', ' ', $this->type())).' Not Done';

	}//end paint()


	/**
	* Prints the interface for filling in a value
	*
	* @param string	$prefix	prefix for the form element
	*
	* @access public
	*/
	function process($prefix)
	{
		$this->processed = false;

	}//end process()


	/**
	* Validate the value of this attribute - children should overwrite
	*
	* @param mixed	&$value	value to be validated
	*
	* @return boolean
	* @access public
	*/
	function validateValue(&$value)
	{
		return true;

	}//end validateValue()


	/**
	* Sets the value of this attribute after validating
	*
	* @param mixed	&$value	value to be validated
	*
	* @return boolean
	* @access public
	*/
	function setValue(&$value)
	{
		if (!$this->validateValue($value)) return false;
		$this->value = $value;
		return true;

	}//end setValue()


	/**
	* Takes an XML_Tree_Node from an edit interface XML file and creates
	* the params array from it
	*
	* @param object XML_Tree_Node	$node
	*
	* @return boolean
	* @access public
	*/
	function setEditParams(&$node)
	{
		if (strtolower($node->name) != strtolower($this->type())) {
			trigger_error('XML Tree Node type "'.$node->name.'" not compatible with attribute type "'.$this->type().'<br/>Full Node Contents :<br/><pre>'.htmlspecialchars($node->get()).'</pre>', E_USER_WARNING);
			return false;
		}
		return true;

	}//end setEditParams()


	/**
	* Retrieve a list of indexable values from this attribute
	*
	* The return value not only contains the indexable values, but also the number of
	* times they occur in this attributes value
	*
	* @return array(string => int)
	* @access public
	*/
	function getKeyData()
	{
		return Array();

	}//end getKeyData()


}//end class
?>
