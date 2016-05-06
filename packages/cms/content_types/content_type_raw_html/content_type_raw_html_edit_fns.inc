<?php
/**
* +--------------------------------------------------------------------+
* | This MySource Matrix CMS file is Copyright (c) Squiz Pty Ltd       |
* | ABN 77 084 670 600                                                 |
* +--------------------------------------------------------------------+
* | IMPORTANT: Your use of this Software is subject to the terms of    |
* | the Licence provided in the file licence.txt. If you cannot find   |
* | this file please contact Squiz (www.squiz.com.au) so we may provide|
* | you a copy.                                                        |
* +--------------------------------------------------------------------+
*
* $Id: content_type_raw_html_edit_fns.inc,v 1.20 2012/08/30 00:58:07 ewang Exp $
*
*/


require_once SQ_CORE_PACKAGE_PATH.'/content_type/content_type_edit_fns.inc';

/**
* Content_Type_Raw_HTML_Edit_Fns
*
* Purpose
*
*
* @author  Greg Sherwood <greg@squiz.net>
* @version $Revision: 1.20 $
* @package MySource_Matrix_Packages
* @subpackage cms
*/
class Content_Type_Raw_HTML_Edit_Fns extends Content_Type_Edit_Fns
{


	/**
	* Constructor
	*
	*/
	function __construct()
	{
		parent::__construct();

	}//end constructor


	/**
	* Prints the backend interface for this WYSIWYG editor
	*
	* @param object	&$asset	the raw html content type object
	* @param string	$prefix	prefix for form elements
	*
	* @return boolean
	* @access public
	*/
	function paintBackend(&$asset, $prefix, $o)
	{
		if ($asset->writeAccess('attributes')) {
			$size = $GLOBALS['SQ_SYSTEM']->getUserPrefs($asset->type(), 'SQ_RAW_HTML_SIZE');
			$html = $asset->attr('html');

			$em = $GLOBALS['SQ_SYSTEM']->getEventManager();
			$keywords = Array();
			$prepared_keywords = Array();
			$keyword_vars = Array('keywords' => &$keywords);
			$em->broadcastEvent($asset, 'requestKeywords', $keyword_vars);
			if (!empty($keywords)) {
				foreach ($keywords as $key => $name) {
					$prepared_keywords['%'.$key.'%'] = $name;
				}
				keyword_selector($prepared_keywords, $prefix.'_html');
			}
			// enable the Ace editor to edit backend code (if available)
			if($o->isViperAceAvailable()) {
				$o->enableViperAceEditor();
			}
			text_area($prefix.'_html', $html, '', '', 0, 'style="width: '.$size['width'].'; height: '.$size['height'].';" class="sq-backend-code-editing sq-viper-ace-editor-textarea sq-viper-ace-editor-textarea-mode-html"');
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
				$current_html = $asset->attr('html');
				if ($html != $current_html) {
					if ($asset->setAttrValue('html', $html)) return TRUE;
				}
			}
		}
		return FALSE;

	}//end processBackend()


}//end class

?>
