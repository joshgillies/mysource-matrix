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
* $Id: content_type_wysiwyg_edit_fns.inc,v 1.9 2004/08/23 04:07:03 gsherwood Exp $
* $Name: not supported by cvs2svn $
*/


require_once SQ_FUDGE_PATH.'/wysiwyg/wysiwyg.inc';
require_once SQ_CORE_PACKAGE_PATH.'/content_type/content_type_edit_fns.inc';

/**
* Content_Type_WYSIWYG_Edit_Fns
*
* Purpose
*
*
* @author  Greg Sherwood <greg@squiz.net>
* @version $Version$ - 1.0
* @package MySource_Matrix_Packages
* @subpackage __core__
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
	function &_createEditor(&$asset, $prefix)
	{
		// allowed plugins (shortened to $ap for easier use below)
		$ap = explode('|', $GLOBALS['SQ_SYSTEM']->getUserPrefs($asset->type(), 'SQ_WYSIWYG_PLUGINS'));
		// ignore allowed plugins (shortened to $iap for easier use);
		$iap = in_array('all', $ap);

		if (empty($this->_tmp['wysiwg_object'])) {

			$html = $asset->attr('html');

			$wysiwyg = new wysiwyg($prefix, sq_web_path('fudge').'/wysiwyg/');

			$wysiwyg_style = $GLOBALS['SQ_SYSTEM']->getUserPrefs($asset->type(), 'SQ_WYSIWYG_STYLE');
			$wysiwyg->set_body_type($wysiwyg_style['body_type']);
			$wysiwyg->set_init_onload(false);
			$wysiwyg->set_show_status_bar(false);
			$wysiwyg->set_width($wysiwyg_style['width']);
			$wysiwyg->set_height($wysiwyg_style['height']);

			$base_php_self = str_replace('/'.SQ_CONF_BACKEND_SUFFIX, '', $_SERVER['PHP_SELF']);
			$base_php_self = str_replace('/'.SQ_CONF_LIMBO_SUFFIX, '', $base_php_self);
			$base_php_self = rtrim($base_php_self, '/ ');

			$wysiwyg->add_relative_href_check('(http[s]?://'.$_SERVER['HTTP_HOST'].')'.$_SERVER['PHP_SELF'], '$1'.$base_php_self);
			$wysiwyg->add_relative_href_check('http[s]?://'.$_SERVER['HTTP_HOST'].$base_php_self.'/?(\?a=[0-9]+)', './$1');
			$wysiwyg->add_relative_href_check('([^.][^/])(\?a=[0-9]+)', '$1./$2');
			$wysiwyg->add_relative_href_check('http[s]?://'.$_SERVER['HTTP_HOST'].$base_php_self.'/?%([a-zA-Z_\-0-9]+)%', '%$1%');
			$wysiwyg->add_absolute_url_check('\./\?(a=[0-9]+)', 'http://'.$_SERVER['HTTP_HOST'].$base_php_self.'/?$1');

			// get the keyword replacements
			$em = &$GLOBALS['SQ_SYSTEM']->getEventManager();
			$keywords = Array();
			$keyword_vars = Array('keywords' => &$keywords);
			$em->broadcastEvent($asset, 'requestKeywords', $keyword_vars);

			$wysiwyg->open_group();
				if ($iap || in_array('format_text', $ap)) $wysiwyg->add_plugin('format_text');
				if ($iap || in_array('format_font', $ap)) $wysiwyg->add_plugin('format_font');
				if ($iap || in_array('general', $ap))     $wysiwyg->add_plugin('general');
				if ($iap || in_array('s_script', $ap))    $wysiwyg->add_plugin('s_script');
			$wysiwyg->close_group();
			
			$wysiwyg->open_group();
				if ($iap || in_array('matrix_apply_style', $ap)) $wysiwyg->add_plugin('matrix_apply_style');
			$wysiwyg->close_group();
			
			if ($iap || in_array('justify_text', $ap)) $wysiwyg->add_plugin('justify_text');
			if (($iap || in_array('keyword_replace', $ap)) && !empty($keywords)) {
				$wysiwyg->add_plugin('keyword_replace');
				$keyword_replace = &$wysiwyg->get_plugin('keyword_replace');
				$keyword_replace->keyword_tag = '%';
				$keyword_replace->keywords = $keywords;
			}
			if ($iap || in_array('history', $ap))     $wysiwyg->add_plugin('history');
			if ($iap || in_array('indenting', $ap))     $wysiwyg->add_plugin('indenting');
			if ($iap || in_array('listing', $ap))       $wysiwyg->add_plugin('listing');
			if ($iap || in_array('table_editing', $ap)) $wysiwyg->add_plugin('table_editing');
			if ($iap || in_array('select_color', $ap))  $wysiwyg->add_plugin('select_color');
			
			$wysiwyg->open_group();
				if ($iap || in_array('matrix_insert_link', $ap) || in_array('insert_link', $ap))  $wysiwyg->add_plugin('matrix_insert_link');
				if ($iap || in_array('matrix_insert_anchor', $ap) || in_array('insert_anchor', $ap))  $wysiwyg->add_plugin('matrix_insert_anchor');
				if ($iap || in_array('matrix_insert_image', $ap) || in_array('insert_image', $ap)) $wysiwyg->add_plugin('matrix_insert_image');
				if ($iap || in_array('matrix_embed_movie', $ap))  $wysiwyg->add_plugin('matrix_embed_movie');
			$wysiwyg->close_group();
			
			$wysiwyg->open_group();
				if ($iap || in_array('spell_checker', $ap))  $wysiwyg->add_plugin('spell_checker');
				if ($iap || in_array('text_indicator', $ap)) $wysiwyg->add_plugin('text_indicator');
				if ($iap || in_array('view_source', $ap))    $wysiwyg->add_plugin('view_source');
				if ($iap || in_array('popup_editor', $ap))   $wysiwyg->add_plugin('popup_editor');
				if ($iap || in_array('help', $ap))           $wysiwyg->add_plugin('help');
			$wysiwyg->close_group();

			// html_tidy needs to be last so other plugins can set options
			if ($iap || in_array('html_tidy', $ap)) $wysiwyg->add_plugin('html_tidy');

			// contents must be set AFTER all plugins are loaded
			$wysiwyg->set_contents($html);

			$this->_tmp['wysiwg_object'] = $wysiwyg;

		}

		return $this->_tmp['wysiwg_object'];

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
		if ($asset->writeAccess('attributes')) {
			$wysiwyg = &$this->_createEditor($asset, $prefix);
			?>
			<div style="display:none;" id="<?php echo $prefix?>_wysiwyg_div"><?php echo $wysiwyg->paint(); ?></div>
			<div id="<?php echo $prefix?>_contents_div" onclick="editDiv=document.getElementById('<?php echo $prefix ?>_wysiwyg_div'); if (editDiv.style.display == 'none') {switchEditingMode('<?php echo $prefix ?>_contents_div','<?php echo $prefix ?>_wysiwyg_div', editor_<?php echo $prefix ?>);}"><?php echo $wysiwyg->contents ?></div>
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
		print_bodycopy_icon("Javascript: switchEditingMode('".$prefix."_contents_div','".$prefix."_wysiwyg_div', editor_".$prefix.");", 'Edit Contents', 'Switch between preview and edit mode', 'edit_mode', '[Edit Contents]');

	}//end paintBackendIcons()


	/**
	* Paints raw HTML
	*
	* @param object Content_Type_WYSIWYG	$asset		the wysiwyg object
	* @param string							$prefix		prefix for form elements
	* @param boolean						$generating	are we genenerating the cache file
	*
	* @return boolean
	* @access public
	*/
	function paint(&$asset, $prefix, $generating=false)
	{
		$html = $asset->attr('html');
		$html = preg_replace('|(<htmlarea_special[^>]*?>)(.*?)(</htmlarea_special>)|si', '$2', $html);
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
		if ($asset->writeAccess('attributes')) {
			$wysiwyg = &$this->_createEditor($asset, $prefix);
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