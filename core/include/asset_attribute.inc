<?php

/**
* Asset Attribute
*
* Purpose
*
*
* @author  Blair Robertson <blair@squiz.net>
* @version $Version$ - 1.0
* @package Resolve
*/
class Asset_Attribute extends Resolve_Object
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
	* Parameters for processing/calculating this attribute
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
		$this->Resolve_Object();

		if ($attributeid) {
			$db = &$GLOBALS['SQ_SYSTEM']->db;

			$sql = 'SELECT attributeid, name, type, parameters, default_value
					FROM sq_asset_attribute
					WHERE attributeid = '.$db->quote($attributeid);

			$result = $db->getRow($sql, null, DB_FETCHMODE_ORDERED);
			if (DB::isError($result)) trigger_error($result->getMessage().'<br/>'.$result->getUserInfo(), E_USER_ERROR);

			if (is_null($result)) return;

			list($this->id, $this->name, $type, $parameters, $this->_default_value) = $result;

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

	}// end contructor

	/*
	* Returns the type of this attribute
	*
	* @access public
	*/
	function type() 
	{
		// strip the 'Asset_Attribute_' off the front
		return substr(get_class($this), 16);
	}//end type();

	/*
	* Prints the interface for filling in a value
	*
	* @param string	$prefix	prefix for the form element
	*
	* @access public
	*/
	function paint($prefix)
	{
		echo 'Interface for '.ucwords(str_replace('_', ' ', $this->type())).' Not Done';
	}

	/*
	* Prints the interface for filling in a value
	*
	* @param string	prefix	prefix for the form element
	*
	* @access public
	*/
	function process($prefix)
	{
		$this->processed = false;
	}

	/*
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
			trigger_error('XML Tree Node type "'.$node->name.'" not compatible with attribute type "'.$this->type().'<br/>Full Node Contents :<br/><pre>'.$node->get().'</pre>', E_USER_WARNING);
			return false;
		}
		return true;
	}

}//end class
?>
