<?php

require_once SQ_INCLUDE_PATH.'/asset_management.inc';

/**
* Content_Type_WYSIWYG_Management
*
* @see Asset_Management
*
* @author  Greg Sherwood <greg@squiz.net>
* @version $Version$ - 1.0
* @package MySource_Matrix_Packages
* @subpackage cms
*/
class Content_Type_WYSIWYG_Management extends Asset_Management
{

	/**
	* Constructor
	*
	*/
	function Content_Type_WYSIWYG_Management(&$pm)
	{
		$this->Asset_Management($pm);

		$this->vars = Array();

	}//end constructor


}//end class
?>
