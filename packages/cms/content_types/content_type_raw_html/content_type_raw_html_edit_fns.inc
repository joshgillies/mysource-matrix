<?php
/**
* +--------------------------------------------------------------------+
* | Squiz.net Commercial Module Licence                                |
* +--------------------------------------------------------------------+
* | Copyright (c) Squiz Pty Ltd (ACN 084 670 600).                     |
* +--------------------------------------------------------------------+
* | This source file is not open source or freely usable and may be    |
* | used subject to, and only in accordance with, the Squiz Commercial |
* | Module Licence.                                                    |
* | Please refer to http://www.squiz.net/licence for more information. |
* +--------------------------------------------------------------------+
*
* $Id: content_type_raw_html_edit_fns.inc,v 1.3.2.2 2005/05/02 02:44:56 gsherwood Exp $
* $Name: not supported by cvs2svn $
*/


require_once SQ_CORE_PACKAGE_PATH.'/content_type/content_type_edit_fns.inc';

/**
* Content_Type_Raw_HTML_Edit_Fns
*
* Purpose
*
*
* @author  Greg Sherwood <greg@squiz.net>
* @version $Version$ - 1.0
* @package MySource_Matrix_Packages
* @subpackage cms
*/
class Content_Type_Raw_HTML_Edit_Fns extends Content_Type_Edit_Fns
{


	/**
	* Constructor
	*
	*/
	function Content_Type_Raw_HTML_Edit_Fns()
	{
		$this->Content_Type_Edit_Fns();

	}//end constructor


	/**
	* Prints the backend interface for this WYSIWYG editor
	*
	* @param object Content_Type_Raw_HTML	&$asset	the nest content content type object
	* @param string								$prefix	prefix for form elements
	*
	* @return boolean
	* @access public
	*/
	function paintBackend(&$asset, $prefix)
	{
		if ($asset->writeAccess('attributes')) {
			$size = $GLOBALS['SQ_SYSTEM']->getUserPrefs($asset->type(), 'SQ_RAW_HTML_SIZE');
			$html = $asset->attr('html');
			$html = str_replace('&lt;?php', '<?php', $html);
			text_area($prefix.'_html', $html, $size['width'], $size['height'], 0, 'style="font-family: monospace"');		
		}
		return true;

	}//end paintBackend()


	/**
	* Processes the backend interface for this Raw HTML editor
	*
	* @param array(string)					$link	information used to create the initial link
	* @param object Content_Type_Raw_HTML	&$asset	the wysiwyg content type object
	* @param string							$prefix	prefix for form elements
	*
	* @return boolean
	* @access public
	*/
	function processBackend($link, &$asset, $prefix)
	{
		if ($asset->writeAccess('attributes')) {
			$html = $_POST[$prefix.'_html'];

			// encode PHP so it doesnt get executed
			$html = str_replace('<?php', '&lt;?php', $html);
			
			$current_html = $asset->attr('html');
			if ($html != $current_html) {
				if ($asset->setAttrValue('html', $html)) return true;
			}
		}
		return false;

	}//end processBackend()


}//end class

?>