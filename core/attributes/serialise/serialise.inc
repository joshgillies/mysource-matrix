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
* $Id: serialise.inc,v 1.6 2003/11/18 15:37:31 brobertson Exp $
* $Name: not supported by cvs2svn $
*/


require_once SQ_INCLUDE_PATH.'/asset_attribute.inc';

/**
* Serialise Asset Attribute
*
*
*
*
* @author  Greg Sherwood <greg@squiz.net>
* @version $Version$ - 1.0
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