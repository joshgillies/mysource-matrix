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
* $Id: serialise.inc,v 1.14 2006/12/05 05:40:37 bcaldwell Exp $
*
*/


require_once SQ_INCLUDE_PATH.'/asset_attribute.inc';

/**
* Serialise Asset Attribute
*
*
*
*
* @author  Greg Sherwood <greg@squiz.net>
* @version $Revision: 1.14 $
* @package MySource_Matrix
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

	}//end constructor


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
			if (is_scalar($unser)) {
				return false;
			} else {
				$value = $unser;
			}
		}
		$value = serialize($value);
		return true;

	}//end validateValue()


	/**
	* Returns the content of the attribute
	*
	* @return string
	* @access public
	*/
	function getContent()
	{
		return var_export($this->value, true).';';

	}//end getContent()


}//end class

?>
