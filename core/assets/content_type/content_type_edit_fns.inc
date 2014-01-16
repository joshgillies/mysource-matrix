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
* $Id: content_type_edit_fns.inc,v 1.19 2013/04/16 01:41:58 akarelia Exp $
*
*/


require_once SQ_INCLUDE_PATH.'/asset_edit/asset_edit_fns.inc';

/**
* Content_Type_Edit_Fns
*
* Purpose
*
*
* @author  Greg Sherwood <greg@squiz.net>
* @version $Revision: 1.19 $
* @package MySource_Matrix_Packages
* @subpackage __core__
*/
class Content_Type_Edit_Fns extends Asset_Edit_Fns
{


	/**
	* Constructor
	*
	*/
	function __construct()
	{
		parent::__construct();

		unset($this->static_screens['permissions']);
		unset($this->static_screens['workflow']);
		unset($this->static_screens['metadataSchemas']);
		unset($this->static_screens['metadata']);
		unset($this->static_screens['linking']);
		unset($this->static_screens['settings']);
		unset($this->static_screens['history']);
		unset($this->static_screens['tagging']);

	}//end constructor


	/**
	* Paints raw HTML to be printed on the frontend
	*
	* @param Content_Type	$asset		the content type we are painting
	* @param boolean		$editing	are we printing an editing interface
	* @param boolean		$generating	are we genenerating the cache file
	*
	* @return void
	* @access public
	*/
	function paint(Content_Type $asset, $editing=FALSE, $generating=FALSE)
	{
		$html = $asset->attr('html');
		
		// remove any sq_wysiwyg_* tags used as placeholders for plugins
		// such as Embed Movie/Youtube
		if (!SQ_IN_BACKEND && !SQ_IN_LIMBO) {
			$html = preg_replace('|<sq_wysiwyg_([^ >]*)([^>]*)>(.*)</sq_wysiwyg_\1>|si', '$3', $html);
		}

		if ($generating) {
			// escape dangerous content
			require_once SQ_FUDGE_PATH.'/general/text.inc';
			escape_php($html);
			echo $html;
			return TRUE;
		}

		if ($editing) {
			// We are in the edit interface, so print the interface and content
			$this->_paintInterface($html, $asset);
		} else {
			// Just print content
			echo $html;
		}//end if

	}//end paint()


	/**
	* Paints raw HTML interface
	*
	* @param string			$html		the html source we are painting
	* @param Content_Type	$asset		the content type we are painting
	*
	* @return void
	* @access public
	*/
	function _paintInterface($html, $asset)
	{
		$source = htmlspecialchars($html);
		$source = preg_replace('/&lt;(div|br|p|table|tr|td|th|style|script|blockquote)/i', '<br />&lt;$1', $source);
		$source = ltrim($source, '<br />');
		$prefix = $asset->getPrefix();
		$content_prefix = $prefix.'_'.get_class_lower($asset);
		$path = sq_web_path('data').'/asset_types/bodycopy/images/icons/source_mode.png';
		echo '<table class="sq-bodycopy-raw-html-table bodytext" style="width: 100%;">';
		echo '<tr><td align="left" id="'.$prefix.'_div_'.$asset->id.'" >';
		echo '<img align="absmiddle" src="'.$path.'" alt="View Source" title="View Source" width="16" height="16" style="cursor:pointer;" onclick="sq_toggle_double_div(\''.$content_prefix.'_html\', \''.$content_prefix.'_source\', \''.$content_prefix.'_control\', \'View Source\', \'View Preview\');if (this.alt == \'View Source\') { this.alt=\'View Preview\'; this.title=\'View Preview\'; } else { this.alt=\'View Source\'; this.title=\'View Source\' };" />';
		echo '<span id="'.$content_prefix.'_control" style="display:none;">View Source</span>';
		echo '</td></tr>';
		echo '<tr><td id="'.$content_prefix.'_cell">';
		echo '<div id="'.$content_prefix.'_html" class="sq-preview-view sq-wysiwyg-preview-wrapper" style="display: block;">';
		echo $html;
		echo '</div>';
		echo '<div id="'.$content_prefix.'_source" class="sq-source-view" style="display: none;">';
		echo $source;
		echo '</div>';
		echo '</td></tr>';
		echo '</table>';

	}//end _paintInterface()


	/**
	* Prints any icons the content type needs next to the cell properties
	*
	* @param Content_Type	$asset	the content type object
	* @param string			$prefix	prefix for form elements
	*
	* @return void
	* @access public
	*/
	function paintBackendIcons(Content_Type $asset, $prefix)
	{

	}//end paintBackendIcons()


	/**
	* Prints extra backend icons at the right hand side of the menu
	*
	* @param Content_Type	$asset	the WYSIWYG Content Type object
	* @param string			$prefix	prefix for form elements
	*
	* @return void
	* @access public
	*/
	function paintBackendStatusIcons(Content_Type $asset, $prefix)
	{

	}//end paintBackendStatusIcons()


}//end class

?>
