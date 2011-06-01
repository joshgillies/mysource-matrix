<?php
/**
* +--------------------------------------------------------------------+
* | This MySource Matrix CMS file is Copyright (c) Squiz Pty Ltd       |
* | ACN 084 670 600                                                    |
* +--------------------------------------------------------------------+
* | IMPORTANT: Your use of this Software is subject to the terms of    |
* | the Licence provided in the file licence.txt. If you cannot find   |
* | this file please contact Squiz (www.squiz.net) so we may provide   |
* | you a copy.                                                        |
* +--------------------------------------------------------------------+
*
* $Id: mysource_object.inc,v 1.7 2006/12/05 05:36:14 bcaldwell Exp $
*
*/


/**
* MySource_Object
*
* @author  Blair Robertson <blair@squiz.net>
* @version $Revision: 1.7 $
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
	*
	* Removes the _tmp array as promised above
	*
	* @return array
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
