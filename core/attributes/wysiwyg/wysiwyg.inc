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
* $Id: wysiwyg.inc,v 1.2 2003/11/26 00:51:08 gsherwood Exp $
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

		$this->_edit_params['style']  = isset($node->attributes['style'])  ? $node->attributes['style']  : 'div';
		$this->_edit_params['width']  = isset($node->attributes['width'])  ? $node->attributes['width']  : '100%';
		$this->_edit_params['height'] = isset($node->attributes['height']) ? $node->attributes['height'] : '300';

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
		$wysiwyg = new wysiwyg($prefix, sq_web_path('fudge').'/wysiwyg/');
		$wysiwyg->set_body_type($this->_edit_params['style']);
		$wysiwyg->set_init_onload(false);
		$wysiwyg->set_show_status_bar(false);
		$wysiwyg->set_width($this->_edit_params['width']);
		$wysiwyg->set_height($this->_edit_params['height']);
		$wysiwyg->add_relative_href_check('http[s]?://'.$_SERVER['HTTP_HOST'].$_SERVER['PHP_SELF'].'(\?a=[0-9]+)', './$1');
		$wysiwyg->add_absolute_url_check('\./\?(a=[0-9]+)', 'http://'.$_SERVER['HTTP_HOST'].$_SERVER['PHP_SELF'].'?$1');

		$wysiwyg->open_group();
			$wysiwyg->add_plugin('format_text');
			$wysiwyg->add_plugin('general');
		$wysiwyg->close_group();
		$wysiwyg->open_group();
			$wysiwyg->add_plugin('matrix_apply_style');
		$wysiwyg->close_group();
		$wysiwyg->add_plugin('justify_text');
		$wysiwyg->add_plugin('indenting');
		$wysiwyg->add_plugin('listing');
		$wysiwyg->add_plugin('table_editing');
		$wysiwyg->add_plugin('select_color');
		$wysiwyg->open_group();
			$wysiwyg->add_plugin('matrix_insert_link');
			$wysiwyg->add_plugin('matrix_insert_image');
			$wysiwyg->add_plugin('matrix_embed_movie');
		$wysiwyg->close_group();
		$wysiwyg->open_group();
			$wysiwyg->add_plugin('spell_checker');
			$wysiwyg->add_plugin('view_source');
			$wysiwyg->add_plugin('popup_editor');
		$wysiwyg->close_group();

		// html_tidy needs to be last so other plugins can set options
		$wysiwyg->add_plugin('html_tidy');
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