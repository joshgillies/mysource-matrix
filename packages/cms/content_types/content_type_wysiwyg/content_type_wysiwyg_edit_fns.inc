<?php

require_once dirname(__FILE__).'/../../content_type/content_type_edit_fns.inc';

/**
* Content_Type_WYSIWYG_Edit_Fns
*
* Purpose
*
*
* @author  Greg Sherwood <greg@squiz.net>
* @version $Version$ - 1.0
* @package Resolve::packages::cms
*/
class Content_Type_WYSIWYG_Edit_Fns extends Content_Type_Edit_Fns
{

	/**
	* Constructor
	*
	*/
	function Content_Type_WYSIWYG_Edit_Fns()
	{
		$this->Asset_Edit_Fns();
	}

	
	/**
	* Prints the backend interface for this table cell
	*
	* @param object Content_Type_WYSIWYG	&$asset	the wysiwyg content type object
	* @param string							$prefix	prefix for form elements
	*
	* @return boolean
	* @access public
	*/
	function paintBackend(&$asset, $prefix)
	{
		echo 'WYSIWYG: I am '.$prefix;
		return true;
	}

}//end class
?>
