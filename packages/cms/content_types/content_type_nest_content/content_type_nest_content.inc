<?php
/**
* +--------------------------------------------------------------------+
* | Squiz.net Open Source Licence                                      |
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
* $Id: content_type_nest_content.inc,v 1.15 2003/12/08 05:47:50 gsherwood Exp $
* $Name: not supported by cvs2svn $
*/


require_once SQ_CORE_PACKAGE_PATH.'/content_type/content_type.inc';

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
	* Returns an array of all the permitted link type, the type asset and the cardinality
	*
	* @see Asset::_getAllowLinks()
	* @access private
	*/
	function _getAllowedLinks()
	{
		return Array(SQ_LINK_TYPE_3 => Array('asset' => Array('card' => 1, 'exclusive' => false)),
					);

	}//end _getAllowedLinks()


	/**
	* Returns true if there is no nested assetid associated with this content type
	*
	* @access public
	* @return boolean
	*/
	function isEmpty()
	{
		$id = $this->attr('nested_assetid');
		return empty($id);

	}//end isEmpty()

}//end class

?>