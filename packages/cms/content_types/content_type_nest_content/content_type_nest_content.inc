<?php
/**
* Copyright (c) 2003 - Squiz Pty Ltd
*
* $Id: content_type_nest_content.inc,v 1.10 2003/09/26 05:26:40 brobertson Exp $
* $Name: not supported by cvs2svn $
*/


require_once dirname(__FILE__).'/../../content_type/content_type.inc';

/**
* Content_Type_Nest_Content
*
* Purpose
*
*
* @author  Greg Sherwoood <greg@squiz.net>
* @version $Version$ - 1.0
* @package MySource_Matrix_Packages
* @subpackage cms
*/
class Content_Type_Nest_Content extends Content_Type
{

	/**
	* Constructor
	*
	* @param int	$assetid	the asset id to be loaded
	*
	*/
	function Content_Type_Nest_Content($assetid=0)
	{
		$this->Content_Type($assetid);

	}//end constructor

	/**
	* returns true if there is no nested assetid associated with this content type
	*
	* @access public
	* @return boolean
	*/
	function isEmpty()
	{
		$id = $this->attr('nested_assetid');
		return empty($id);
	} // isEmpty()

}//end class

?>