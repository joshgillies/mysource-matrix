<?php

require_once dirname(__FILE__).'/../../content_type/content_type.inc';

/**
* Content_Type_WYSIWYG
*
* Purpose
*
*
* @author  Greg Sherwood <greg@squiz.net>
* @version $Version$ - 1.0
* @package MySource_Matrix_Packages
* @subpackage cms
*/
class Content_Type_WYSIWYG extends Content_Type
{

	/**
	* Constructor
	*
	* @param int	$assetid	the asset id to be loaded
	*
	*/
	function Content_Type_WYSIWYG($assetid=0)
	{
		$this->Content_Type($assetid);

	}//end constructor

	/**
	* returns true if there is no content in the wysiwyg
	*
	* @access public
	* @return boolean
	*/
	function isEmpty() 
	{
		$html = trim($this->attr('html'));
		return empty($html);
	} // isEmpty()

}//end class

?>