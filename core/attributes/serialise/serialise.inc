<?php

require_once SQ_INCLUDE_PATH.'/asset_attribute.inc';

/**
* Serialise Asset Attribute
*
*
*
*
* @author  Greg Sherwood <greg@squiz.net>
* @version $Version$ - 1.0
* @package Resolve
* @subpackage attributes
*/
class Asset_Attribute_Serialise extends Asset_Attribute
{

	/**
	* Constructor
	*
	* @param int	$attributeid	the attribute id to be loaded
	* @param mixed	$value			the current value for the element
	*
	*/
	function Asset_Attribute_Serialise($attributeid=0, $value=null) 
	{
		$this->Asset_Attribute($attributeid, $value);
	}// end constructor


	/**
	* Ensure the attribute being set is serilisable
	*
	* @param mixed	&$value	value to be validated
	*
	* @return boolean
	* @access public
	*/
	function validateValue(&$value)
	{
		// cant serilise scalar variables
		if (is_scalar($value)) {
			// see if it is already serilised
			$unser = @unserialize(trim((string) $value));
			if (is_scalar($unser)) return false;
			else $value = $unser;
		}
		$value = serialize($value);
		return true;
	}//end validateValue()


}//end class

?>