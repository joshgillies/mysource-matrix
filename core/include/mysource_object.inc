<?php
/**
* Copyright (c) 2003 - Squiz Pty Ltd
*
* $Id: mysource_object.inc,v 1.2 2003/09/26 05:26:31 brobertson Exp $
* $Name: not supported by cvs2svn $
*/


/**
* MySource_Object
*
* @author  Blair Robertson <blair@squiz.net>
* @version $Version$ - 1.0
* @package MySource_Matrix
*/
class MySource_Object 
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
	* @access public
	*/
	function MySource_Object()
	{
		$this->_tmp = Array();
	}//end constructor


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


	/**
	* Perform some actions after the object is unserialised
	*
	* @return void
	* @access private
	*/
	function __wakeup()
	{
		$this->_tmp = Array();
	}//end __wakeup()


}//end class


?>
