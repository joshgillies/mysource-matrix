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
* @package Resolve_Packages
* @subpackage cms
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
		if ($asset->writeAccess()) {
			$wysiwyg = new wysiwyg($prefix, sq_web_path('fudge').'/wysiwyg/');
			$wysiwyg->set_body_type('div');
			$wysiwyg->set_init_onload('yes');
			$wysiwyg->set_contents($asset->attr('html'));
			//$wysiwyg->set_stylesheet('http://www.nswrl.com.au/__data/site/design/1/images/styles.css');
			$wysiwyg->set_width('100%');
			$wysiwyg->add_plugin('format_text');
			$wysiwyg->add_plugin('justify_text');
			$wysiwyg->add_plugin('indenting');
			$wysiwyg->add_plugin('listing');
			$wysiwyg->add_plugin('general');
			$wysiwyg->add_plugin('history');
			$wysiwyg->add_plugin('select_color');
			$wysiwyg->add_plugin('matrix_insert_link');
			$wysiwyg->add_plugin('matrix_insert_image');
			$wysiwyg->add_plugin('view_source');
			$wysiwyg->add_plugin('popup_editor');
			$wysiwyg->paint();
		}
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
	function paint(&$asset, $generating=false) {
		echo $asset->attr('html');
	}


	/**
	* Processes the backend interface for this WYSIWYG editor
	*
	* @param array(string)					$link	information used to create the initial link
	* @param object Content_Type_WYSIWYG	&$asset	the wysiwyg content type object
	* @param string							$prefix	prefix for form elements
	*
	* @return boolean
	* @access public
	*/
	function processBackend($link, &$asset, $prefix)
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
