<?php

/**
* Resolve_Object
*
* @author  Blair Robertson <blair@squiz.net>
* @version $Version$ - 1.0
* @package Resolve
*/
class Resolve_Object 
{
		 
	/**
	* Array to be used for temporary storage of any kind, 
	* guaranteed not to be stored if object is serialised
	*
	* @var Array
	* @see __sleep()
	*/
	var $_tmp;
	 
	/**
	* Constructor
	*
	* @access  public
	*/
	function Resolve_Object() 
	{
		$this->_tmp = Array();
	}

	/**
	* Returns an array of var names in the object that are to saved if this object get's serialised
	* Removes the _tmp array as promised above
	*
	* @returns array string
	*/
	function __sleep() 
	{
		$object_vars = get_object_vars($this);
		unset($object_vars['_tmp']);
		return array_keys($object_vars);
	}//end __sleep()


}//end class


?>
