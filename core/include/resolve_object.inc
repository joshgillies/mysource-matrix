<? 

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
*    $array   = $xml->getArrayFromFile($file);
*    print_r($array);
*
* @author  Blair Robertson <blair@squiz.net>
* @package XML
* @version $Version$ - 1.0
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
	}#end __sleep()


}#end class


?>