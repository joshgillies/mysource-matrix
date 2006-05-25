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
* $Id: content_type_raw_html_edit_fns.inc,v 1.9.2.2 2006/05/25 23:29:51 rhoward Exp $
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
* @version $Revision: 1.9.2.2 $
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
	* @param object Content_Type_Raw_HTML	&$asset	the raw html content type object
	* @param string							$prefix	prefix for form elements
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
			$this->_paintKeywordInserter($asset, $prefix);
			text_area($prefix.'_html', $html, $size['width'], $size['height'], 0, 'style="font-family: monospace; display: block"');
		}
		return true;

	}//end paintBackend()


	/**
	* Paint the keyword-insert dropdown box
	*
	* @access private
	* @return void
	*/
	function _paintKeywordInserter(&$asset, $prefix)
	{
		$em = &$GLOBALS['SQ_SYSTEM']->getEventManager();
		$keywords = Array();
		$keyword_vars = Array('keywords' => &$keywords);
		$em->broadcastEvent($asset, 'requestKeywords', $keyword_vars);
		if (empty($keywords)) return;
		?>
		<script type="text/javascript" src="<?php echo sq_web_path('data'); ?>/asset_types/content_type_raw_html/js/raw_html_insert_keyword.js"></script>
		<select id="<?php echo $prefix; ?>_keyword_inserter" onchange="insertKeyword('<?php echo $prefix; ?>')">
			<option value="">-- <?php echo translate('insert_keyword') ?> --</option>
		<?php
		foreach ($keywords as $keyword => $name) {
			?>
			<option value="%<?php echo $keyword; ?>%"><?php echo $name; ?></option>
			<?php
		}
		?>
		</select>
		<?php

	}//end _paintKeywordInserter()


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
			if (isset($_POST[$prefix.'_html'])) {
				$html = $_POST[$prefix.'_html'];
			
				// encode PHP so it doesnt get executed
				$html = str_replace('<?php', '&lt;?php', $html);

				$current_html = $asset->attr('html');
				if ($html != $current_html) {
					if ($asset->setAttrValue('html', $html)) return true;
				}
			}
		}
		return false;

	}//end processBackend()


}//end class

?>
