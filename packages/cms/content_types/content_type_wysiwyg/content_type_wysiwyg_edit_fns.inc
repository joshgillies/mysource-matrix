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
* @package MySource_Matrix_Packages
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
		$this->Content_Type_Edit_Fns();

	}//end constructor


	/**
	* Sets the various params for this WYSIWYG editor
	*
	* @param object Content_Type_WYSIWYG	&$asset	the wysiwyg content type object
	* @param string							$prefix	prefix for form elements
	*
	* @return object WYSIWYG
	* @access private
	*/
	function _createEditor(&$asset, $prefix)
	{
		$html = $asset->attr('html');

		$wysiwyg = new wysiwyg($prefix, sq_web_path('fudge').'/wysiwyg/');
		$wysiwyg->set_body_type('div');
		$wysiwyg->set_init_onload(false);
		$wysiwyg->set_show_status_bar(true);
		//$wysiwyg->set_stylesheet('http://www.nswrl.com.au/__data/site/design/1/images/styles.css');
		$wysiwyg->set_width('100%');
		$wysiwyg->set_height('100%');
		$wysiwyg->add_plugin('format_text');
		$wysiwyg->add_plugin('justify_text');
		$wysiwyg->add_plugin('indenting');
		$wysiwyg->add_plugin('listing');
		$wysiwyg->add_plugin('general');
		$wysiwyg->add_plugin('history');
		$wysiwyg->add_plugin('table_editing');
		$wysiwyg->add_plugin('select_color');
		$wysiwyg->add_plugin('matrix_embed_movie');
		$wysiwyg->add_plugin('matrix_insert_link');
		$wysiwyg->add_plugin('matrix_insert_image');
		$wysiwyg->add_plugin('view_source');
		$wysiwyg->set_contents($html);
		//$wysiwyg->add_plugin('popup_editor');
		return $wysiwyg;

	}//end _createEditor()


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
			$wysiwyg = $this->_createEditor($asset, $prefix);
			?>
			<div style="display:none;" id="<?php echo $prefix?>_wysiwyg_div"><?php echo $wysiwyg->paint()?></div>
			<div id="<?php echo $prefix?>_contents_div"><?php echo $this->paint($asset)?></div>
			<?php
		}
		return true;

	}//end paintBackend()


	/**
	* Prints any icons the content type needs next to the cell properties
	*
	* @param object Content_Type_WYSIWYG	&$asset	the wysiwyg content type object
	* @param string							$prefix	prefix for form elements
	*
	* @return void
	* @access public
	*/
	function paintBackendIcons(&$asset, $prefix)
	{
		print_bodycopy_icon("Javascript: switchEditingMode('".$prefix."_contents_div','".$prefix."_wysiwyg_div', editor_".$prefix.");", 'Edit Cell Contents', 'Switch between preview and edit mode', 'edit_mode');

	}//end paintBackendIcons()


	/**
	* Paints raw HTML
	*
	* @param object Content_Type_WYSIWYG	$asset	the wysiwyg object
	*
	* @return boolean
	* @access public
	*/
	function paint(&$asset, $generating=false)
	{
		$html = $asset->attr('html');
		$html = preg_replace('|(<SQ_WYSIWYG_EDITOR[^>]*?>)(.*?)(</SQ_WYSIWYG_EDITOR>)|si', '$2', $html);
		echo $html;

	}//end paint()


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
		if ($asset->writeAccess()) {
			$wysiwyg = $this->_createEditor($asset, $prefix);
			$html = $wysiwyg->process();

			$current_html = $asset->attr('html');
			if ($html != $current_html) {
				if ($asset->setAttrValue('html', $html)) return true;
			}
		}
		return false;

	}//end processBackend()

}//end class

?>