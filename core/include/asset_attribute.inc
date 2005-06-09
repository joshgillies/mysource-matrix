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
* $Id: asset_attribute.inc,v 1.37 2005/06/09 00:55:37 tbarrett Exp $
*
*/


/**
* Asset Attribute
*
* Purpose
*
*
* @author  Blair Robertson <blair@squiz.net>
* @version $Revision: 1.37 $
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

			$sql = 'SELECT attrid, name, type, uniq, description, parameters_val, default_val
					FROM sq_ast_attr
					WHERE attrid = '.$db->quote($attributeid);

			$result = $db->getRow($sql, null, DB_FETCHMODE_ORDERED);
			assert_valid_db_result($result);

			if (is_null($result)) return;

			list($this->id, $this->name, $type, $this->uniq, $this->description, $parameters, $this->_default_value) = $result;

			// make sure the asset we are loading is of the same type as our class
			if ($type != $this->type()) {
				trigger_localised_error('SYS0095', E_USER_WARNING, $attributeid, $this->type());
				$this->id = null;
				return;
			}

			$this->_params = ($parameters) ? unserialize($parameters) : Array();
			unset($result);

		}

		$this->value = $value;

		// now just make sure some default values are set
		foreach ($default_params as $name => $default) {
			if (!isset($this->_params[$name])) {
				$this->_params[$name] = $default;
			}
		}

	}//end constructor


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

		$sql = 'SELECT type FROM sq_ast_attr
				WHERE attrid = '.$db->quote($attributeid);

		$type = $db->getOne($sql);
		assert_valid_db_result($type);

		$class = 'Asset_Attribute_'.$type;
		require_once SQ_ATTRIBUTES_PATH.'/'.$type.'/'.$type.'.inc';
		return new $class($attributeid, $value, $default_params);

	}//end loadAttribute()


	/**
	* Returns the type of this attribute
	*
	* @return string
	* @access public
	*/
	function type()
	{
		// strip the 'Asset_Attribute_' off the front
		return substr(get_class($this), 16);

	}//end type()


	/**
	* Prints the interface for filling in a value
	*
	* @param string		$prefix		prefix for the form element
	* @param boolean	$read_only	are we just printing the value
	*
	* @return void
	* @access public
	*/
	function paint($prefix, $read_only=false)
	{
		echo translate('interface_not_created', ucwords(str_replace('_', ' ', $this->type())));

	}//end paint()


	/**
	* Print the interface for changing some params
	*
	* Attributes must override this function
	*
	* @param string		$prefix		prefix for the form element
	*
	* @return boolean
	* @access public
	*/
	function paintEditParams($prefix, $write_access=false)
	{
		return true;

	}//end paintEditParams()


	/**
	* Process the interface for changing some params
	*
	* Attributes must override this function
	*
	* @param string		$prefix		prefix for the form element
	*
	* @return boolean
	* @access public
	*/
	function processEditParams($prefix)
	{
		return true;

	}//end processEditParams()


	/**
	* Prints the interface for filling in a value
	*
	* @param string	$prefix	prefix for the form element
	*
	* @return void
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
			trigger_localised_error('SYS0300', E_USER_WARNING, $node->name, $this->type(), '<pre>'.htmlspecialchars($node->get()).'</pre>');
			return false;
		}
		return true;

	}//end setEditParams()


	/**
	* Manually set the value of an edit parameter for this attribute
	*
	* The parameter must already contain a default value to be set with this function
	*
	* @field	The parameter field to set
	* @value	The value to set it to
	*
	* @access public
	* @return boolean	True if parameter set successfully
	*/
	function setEditParam($field, $value)
	{
		if (isset($this->_edit_params[$field])) {
			$this->_edit_params[$field] = $value;
			return true;
		} else {
			trigger_localised_error('SYS0258', E_USER_NOTICE, $field, $this->type());
			return false;
		}

	}//end setEditParam()


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
