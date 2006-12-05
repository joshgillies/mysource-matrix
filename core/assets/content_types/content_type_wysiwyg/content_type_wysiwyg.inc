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
* $Id: content_type_wysiwyg.inc,v 1.10 2006/12/05 05:48:39 emcdonald Exp $
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
* @version $Revision: 1.10 $
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