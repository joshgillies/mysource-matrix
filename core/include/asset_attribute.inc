<?php

/**
* Asset Attribute
*
* Purpose
*
*
* @author  Blair Robertson <blair@squiz.net>
* @version $Version$ - 1.0
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
	* Parameters for displaying this attribute
	* @var Array()
	*/
	var $_params = Array();

	/**
	* The default value for this attribute
	* @var mixed
	*/
	var $_default_value = null;


	/**
	* The current value for this instance of this attribute
	* @var mixed
	*/
	var $value = null;

	/**
	* Constructor
	*
	* @param int	$attributeid	the attribute id to be loaded
	* @param mixed	$value			the current value for the element
	*
	*/
	function Asset_Attribute($attributeid, $value) 
	{
		$this->Resolve_Object();

		$db = &$GLOBALS['SQ_RESOLVE']->db;

		$sql = 'SELECT attributeid, name, type, parameters, default_value
				FROM sq_asset_attribute
				WHERE attributeid = '.$db->quote($attributeid);

		pre_echo($sql);

		$result = $db->getRow($sql, null, DB_FETCHMODE_ORDERED);
		if (DB::isError($result)) trigger_error($result->getMessage().'<br>'.$result->getUserInfo(), E_USER_ERROR);

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

		$this->value = $value;

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
	* Returns a value that can be saved in the 
	*
	* @access public
	*/
	function valueToSave() 
	{
		return $this->value;
	}//end valueToSave();

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
		return false;
	}

	/*
	* Returns a string representation of the attribute
	*
	* @param string	prefix	prefix for the form element
	*
	* @access public
	*/
	function toString()
	{
		return (string) $this->value;
	}


}//end class
?>
