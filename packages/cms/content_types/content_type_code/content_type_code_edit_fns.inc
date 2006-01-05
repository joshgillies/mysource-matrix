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
* $Id: content_type_code_edit_fns.inc,v 1.2 2006/01/05 04:48:48 dmckee Exp $
*
*/


require_once SQ_CORE_PACKAGE_PATH.'/content_type/content_type_edit_fns.inc';

/**
* Content_Type_Code_Edit_Fns
*
* Purpose
*
*
* @author  Darren McKee <dmckee@squiz.net>
* @version $Revision: 1.2 $
* @package MySource_Matrix_Packages
* @subpackage cms
*/
class Content_Type_Code_Edit_Fns extends Content_Type_Edit_Fns
{


	/**
	* Constructor
	*
	*/
	function Content_Type_Code_Edit_Fns()
	{
		$this->Content_Type_Edit_Fns();

	}//end constructor


	/**
	* Prints the backend interface for this WYSIWYG editor
	*
	* @param object	&$asset	The Code Content Type Object
	* @param string	$prefix	prefix for form elements
	*
	* @return boolean
	* @access public
	*/
	function paintBackend(&$asset, $prefix)
	{
		if ($asset->writeAccess('attributes')) {
			$size = $GLOBALS['SQ_SYSTEM']->getUserPrefs($asset->type(), 'SQ_CODE_EDITOR_SIZE');
			$html = $asset->attr('edit_content');
			$html = str_replace('&lt;?php', '<?php', $html);
			$html = str_replace('&lt;script', '<script', $html);
			text_area($prefix.'_html', $html, '', '', 0, 'style="width: '.$size['width'].'; height: '.$size['height'].'; font-family: monospace; display: block"');
		}
		return TRUE;

	}//end paintBackend()


	/**
	* Processes the backend interface for this Raw HTML editor
	*
	* @param array	$link	information used to create the initial link
	* @param object	&$asset	the wysiwyg content type object
	* @param string	$prefix	prefix for form elements
	*
	* @return boolean
	* @access public
	*/
	function processBackend($link, &$asset, $prefix)
	{
		if ($asset->writeAccess('attributes')) {
			if (isset($_POST[$prefix.'_html'])) {
				$html = $_POST[$prefix.'_html'];

				$asset->setAttrValue('edit_content', str_replace(Array('<script','<?php'), Array('&lt;script', '&lt;?php'), $html));

				$html = $asset->syntaxHighlight($html);

				$current_html = $asset->attr('html');
				if ($html != $current_html) {
					if ($asset->setAttrValue('html', $html)) return TRUE;
				}
			}
		}
		return FALSE;

	}//end processBackend()


	/**
	* Paints raw HTML to be printed on the frontend
	*
	* @param object		&$asset		the content type we are painting
	* @param string		$prefix		prefix for form elements
	* @param boolean	$editing	are we printing an editing interface
	* @param boolean	$generating	are we genenerating the cache file
	*
	* @return void
	* @access public
	*/
	function paint(&$asset, $prefix, $editing=FALSE, $generating=FALSE)
	{
		// This may be changed to use the styles of the design, to show
		// what it will look like. If $editing == TRUE, then we do this
		echo $asset->attr('html');

	}//end paint()


}//end class

?>
