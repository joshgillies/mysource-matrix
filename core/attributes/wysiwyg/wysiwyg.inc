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
* $Id: wysiwyg.inc,v 1.2.2.2 2004/04/21 10:55:16 brobertson Exp $
* $Name: not supported by cvs2svn $
*/


require_once SQ_ATTRIBUTES_PATH.'/text/text.inc';
require_once SQ_FUDGE_PATH.'/wysiwyg/wysiwyg.inc';

/**
* WYSIWYG Asset Attribute
*
*
*
*
* <!-- Sample Edit Interface XML Node -->
*    <wysiwyg style="div" width="100%" height="300" />
*
* @author  Greg Sherwood <greg@squiz.net>
* @version $Version$ - 1.0
* @package MySource_Matrix
* @subpackage attributes
*/
class Asset_Attribute_WYSIWYG extends Asset_Attribute_Text
{

	/**
	* Constructor
	*
	* @param int	$attributeid	the attribute id to be loaded
	* @param mixed	$value			the current value for the element
	*
	*/
	function Asset_Attribute_WYSIWYG($attributeid=0, $value=null)
	{
		$this->Asset_Attribute_Text($attributeid, $value);

		// set default edit parameters
		$this->_edit_params['style']  = 'div';
		$this->_edit_params['width']  = '100%';
		$this->_edit_params['height'] = '300';

	}//end constructor


	/**
	* Takes an XML_Tree_Node from an edit interface XML file and creates the params array from it
	*
	* @param object XML_Tree_Node	$node	the XML_Tree_Node from the edit interface XML file
	*
	* @return boolean
	* @access public
	*/
	function setEditParams(&$node)
	{
		if (!parent::setEditParams($node)) return false;

		if (isset($node->attributes['style']))  $this->_edit_params['style']  = $node->attributes['style'];
		if (isset($node->attributes['width']))  $this->_edit_params['width']  = $node->attributes['width'];
		if (isset($node->attributes['height'])) $this->_edit_params['height'] = $node->attributes['height'];

		return true;

	}//end setEditParams()


	/**
	* Sets the various params for this WYSIWYG editor
	*
	* @param string	$prefix	prefix for form elements
	*
	* @return object WYSIWYG
	* @access private
	*/
	function &_createEditor($prefix)
	{
		// allowed plugins (shortened to $ap for easier use below)
		$ap = explode('|', $GLOBALS['SQ_SYSTEM']->getUserPrefs('content_type_wysiwyg', 'SQ_WYSIWYG_PLUGINS'));
		// ignore allowed plugins (shortened to $iap for easier use);
		$iap = in_array('all', $ap);

		$wysiwyg = new wysiwyg($prefix, sq_web_path('fudge').'/wysiwyg/');

		$wysiwyg->set_body_type($this->_edit_params['style']);
		$wysiwyg->set_init_onload(false);
		$wysiwyg->set_show_status_bar(false);
		$wysiwyg->set_width($this->_edit_params['width']);
		$wysiwyg->set_height($this->_edit_params['height']);

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
		if ($iap || in_array('history', $ap))      $wysiwyg->add_plugin('history');
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
		$wysiwyg->set_contents($this->value);

		return $wysiwyg;

	}//end _createEditor()


	/**
	* Print the interface for filling in a value
	*
	* @param string		$prefix		prefix for the form elements
	* @param boolean	$read_only	are we just printing the value
	*
	* @access public
	*/
	function paint($prefix, $read_only=false)
	{
		if ($read_only) {
			echo $this->value;
			return;
		}

		$prefix = $prefix.'_'.$this->name;
		$wysiwyg = $this->_createEditor($prefix);

		?>
		<table style="width:100%;">
			<tr>
				<td align="left" width="4%" style="border: 1px dashed #C0C0C0;">
					<a href="Javascript: switchEditingMode('<?php echo $prefix; ?>_contents_div','<?php echo $prefix; ?>_wysiwyg_div', editor_<?php echo $prefix; ?>);">
					<script language="JavaScript" type="text/javascript">sq_print_icon("<?php echo sq_web_path('lib')?>/web/images/icons/edit_mode.png", "16", "16", "");</script>
					</a>
				</td>
			</tr>
			<tr>
				<td style="border-left: 1px dashed #C0C0C0; border-right: 1px dashed #C0C0C0; border-bottom: 1px dashed #C0C0C0;">
					<div style="display:none;" id="<?php echo $prefix?>_wysiwyg_div"><?php echo $wysiwyg->paint()?></div>
					<div id="<?php echo $prefix?>_contents_div"><?php echo $this->value?></div>
				</td>
			</tr>
		</table>
		<?php

	}//end paint()


	/**
	* Process the interface for filling in a value
	*
	* @param string	prefix	prefix for the form elements
	*
	* @access public
	*/
	function process($prefix)
	{
		$prefix = $prefix.'_'.$this->name;
		$wysiwyg = &$this->_createEditor($prefix);
		$html = $wysiwyg->process();
		if ($html != $this->value && $this->setValue($html)) $this->processed = true;
		else $this->processed = false;

	}//end process()


}//end class

?>