<?php

require_once SQ_FUDGE_PATH.'/wysiwyg/wysiwyg.inc';
require_once dirname(__FILE__).'/../../content_type/content_type_edit_fns.inc';

/**
* Content_Type_WYSIWYG_Edit_Fns
*
* Purpose
*
*
* @author  Greg Sherwood <greg@squiz.net>
* @version $Version$ - 1.0
* @package Resolve::packages::cms
*/
class Content_Type_WYSIWYG_Edit_Fns extends Content_Type_Edit_Fns
{

	/**
	* Constructor
	*
	*/
	function Content_Type_WYSIWYG_Edit_Fns()
	{
		$this->Asset_Edit_Fns();
	}

	
	/**
	* Prints the backend interface for this WYSIWYG editor
	*
	* @param object Content_Type_WYSIWYG	&$asset	the wysiwyg content type object
	* @param string							$prefix	prefix for form elements
	*
	* @return boolean
	* @access public
	*/
	function paintBackend(&$asset, $prefix)
	{
		$wysiwyg = new wysiwyg($prefix, sq_web_path('fudge').'/wysiwyg/');
		$wysiwyg->set_body_type('iframe');
		$wysiwyg->set_init_onload('yes');
		$wysiwyg->set_contents($asset->attr('html'));
		//$wysiwyg->set_stylesheet('/nswrl/__data/site/design/1/images/styles.css');
		$wysiwyg->set_width('100%');
		$wysiwyg->set_height('100%');
		$wysiwyg->add_plugin('format_text');
		$wysiwyg->add_plugin('general');
		$wysiwyg->add_plugin('history');
		$wysiwyg->add_plugin('select_color');
		$wysiwyg->add_plugin('view_source');
		$wysiwyg->add_plugin('popup_editor');
		$wysiwyg->paint();
		return true;
	}


	/**
	* Paints raw HTML
	*
	* @param object Content_Type_WYSIWYG	$asset	the wysiwyg object
	*
	* @return boolean
	* @access public
	*/
	function paint(&$asset) {
		echo $asset->attr('html');
	}


	/**
	* Processes the backend interface for this WYSIWYG editor
	*
	* @param object Content_Type_WYSIWYG	&$asset	the wysiwyg content type object
	* @param string							$prefix	prefix for form elements
	*
	* @return boolean
	* @access public
	*/
	function processBackend(&$asset, $prefix)
	{
		$current_html = $asset->attr('html');
		$html = (isset($_POST[$prefix])) ? $_POST[$prefix] : '';
		$html = trim($html);
		if ($html != $current_html) {
			if ($asset->setAttrValue('html', $html)) return true;
		}
		return false;
	}

}//end class
?>
