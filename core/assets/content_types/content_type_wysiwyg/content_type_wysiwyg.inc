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
* $Id: content_type_wysiwyg.inc,v 1.9 2006/01/17 21:57:01 lwright Exp $
*
*/


require_once SQ_CORE_PACKAGE_PATH.'/content_type/content_type.inc';

/**
* Content_Type_WYSIWYG
*
* Purpose
*	Stores html content to be edited using the WYSIWYG editor
*
* @author  Greg Sherwood <greg@squiz.net>
* @version $Revision: 1.9 $
* @package MySource_Matrix_Packages
* @subpackage __core__
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
	* Morph the asset to either one of its parent types or one of its children types
	*
	* In this case we are probably morphing to raw-html and want to remove mozilla's <br> tag
	* if the content is otherwise blank
	*
	* @param string	$new_type_code	the new type_code to attempt to cast this asset to
	*
	* @return boolean
	* @access public
	*/
	function morph($new_type_code)
	{
		if (!parent::morph($new_type_code))	return FALSE;

		if (str_replace(Array("\r","\n"), Array('',''), $this->attr('html')) == '<br>') {
			// work around mozilla wysiwyg bug
			$this->setAttrValue('html', '');
			$this->saveAttributes();
		}
		return TRUE;

	}//end morph()


}//end class

?>