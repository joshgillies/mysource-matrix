<?php
/**
* +--------------------------------------------------------------------+
* | MySource 3 - MySource Matrix                                       |
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
* $Id: mysource_object.inc,v 1.3 2003/11/18 15:37:34 brobertson Exp $
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
