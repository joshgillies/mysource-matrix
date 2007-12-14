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
* $Id: content_type_wysiwyg.inc,v 1.13 2007/12/14 05:36:27 rong Exp $
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
* @version $Revision: 1.13 $
* @package MySource_Matrix_Packages
* @subpackage __core__
*/
class Content_Type_WYSIWYG extends Content_Type
{


	/**
	* Constructor
	*
	* @param int	$assetid	the asset id to be loaded
	*/
	function __construct($assetid=0)
	{
		parent::__construct($assetid);

	}//end __construct()


	/**
	* Morph the asset to either one of its parent types or one of its children types
	*
	* In this case we are probably morphing to raw-html and want to remove mozilla's <br> tag
	* if the content is otherwise blank
	* see asset.inc::morph() for more info on changes on 3.18
	*
	* @param string	$new_type_code	the new type_code to attempt to cast this asset to
	*
	* @return boolean|object
	* @access public
	*/
	public function morph($new_type_code)
	{
		if (!$morphed = parent::morph($new_type_code)) {
			return FALSE;
		}

		if (str_replace(Array("\r","\n"), Array('',''), $morphed->attr('html')) == '<br>') {
			// work around mozilla wysiwyg bug
			$morphed->setAttrValue('html', '');
			$morphed->saveAttributes();
		}
		return $morphed;

	}//end morph()


}//end class

?>