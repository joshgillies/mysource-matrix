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
* $Id: content_type_markdown_edit_fns.inc,v 1.3 2012/08/30 00:58:07 ewang Exp $
*
*/


require_once SQ_CORE_PACKAGE_PATH.'/content_type/content_type_edit_fns.inc';

/**
* Content_Type_Markdown_Edit_Fns
*
* Purpose
*
*
* @author  Anh Ta <ata@squiz.co.uk>
* @version $Revision: 1.3 $
* @package MySource_Matrix_Packages
* @subpackage cms
*/
class Content_Type_Markdown_Edit_Fns extends Content_Type_Edit_Fns
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
	* Prints the backend interface for this Markdown editor
	*
	* @param object	&$asset	the markdown content type object
	* @param string	$prefix	prefix for form elements
	*
	* @return boolean
	* @access public
	*/
	function paintBackend(&$asset, $prefix)
	{
		if ($asset->writeAccess('attributes')) {
			$size = $GLOBALS['SQ_SYSTEM']->getUserPrefs($asset->type(), 'SQ_MARKDOWN_SIZE');
			$markdown = $asset->attr('markdown');
			$markdownify_not_configured_message = '';
			if ($markdown == '') {
				$html = $asset->attr('html');
				if (trim($html) != '') {
					$is_markdownified = FALSE;
					require_once SQ_DATA_PATH.'/private/conf/tools.inc';
					require_once SQ_FUDGE_PATH.'/general/text.inc';
					if (SQ_TOOL_MARKDOWNIFY_ENABLED && is_file(SQ_TOOL_MARKDOWNIFY_PATH)) {
						require_once SQ_TOOL_MARKDOWNIFY_PATH;
						$class_name = basename(SQ_TOOL_MARKDOWNIFY_PATH, '.php');
						if (method_exists($class_name, 'parseString')) {
							$markdownify = new $class_name;

							// Keep the html 'pure' only make changes on safe_html
							$safe_html = $html;

							// Find the keywords and make them 'markdown-proof'
							$keywords = retrieve_keywords_replacements($html);
							foreach ($keywords as $keyword) {
								$safe_keyword = '%'.str_replace('_', '[MY3SEP]', $keyword).'%';
								$safe_html = str_replace('%'.$keyword.'%', $safe_keyword, $safe_html);
							}//end foreach

							// Convert HTML to Markdown
							$markdown = $markdownify->parseString($safe_html);

							// Change the safe keywords back to normal keywords
							foreach ($keywords as $keyword) {
								$safe_keyword = '%'.str_replace('_', '[MY3SEP]', $keyword).'%';
								$markdown = str_replace($safe_keyword, '%'.$keyword.'%', $markdown);
							}//end foreach
							$is_markdownified = TRUE;
						}
					}

					//if can not markdownify, keep the HTML value
					if (!$is_markdownified) {
						$markdown = $html;
						$markdownify_not_configured_message = translate('HTML content could not be converted to markdown content because markdownify tool has not been configured properly.');

					}

				}//end if (trim($html) != '')
			}//end if ($markdown == '')


			$em = $GLOBALS['SQ_SYSTEM']->getEventManager();
			$keywords = Array();
			$prepared_keywords = Array();
			$keyword_vars = Array('keywords' => &$keywords);
			$em->broadcastEvent($asset, 'requestKeywords', $keyword_vars);
			if (!empty($keywords)) {
				foreach ($keywords as $key => $name) {
					$prepared_keywords['%'.$key.'%'] = $name;
				}
				keyword_selector($prepared_keywords, $prefix.'_markdown');
			}

			//print a notice if markdownify is not configured when it is needed
			if ($markdownify_not_configured_message != '') {
				echo '<span class="sq-backend-warning">'.$markdownify_not_configured_message.'</span><br />';
			}

			//print a notice if markdown is not configured
			$markdown_configured = FALSE;
			if (SQ_TOOL_MARKDOWN_ENABLED && is_file(SQ_TOOL_MARKDOWN_PATH)) {
				require_once SQ_TOOL_MARKDOWN_PATH;
				if (function_exists('Markdown')) {
					$markdown_configured = TRUE;
				}
			}
			if (!$markdown_configured) {
				echo '<span class="sq-backend-warning">'.translate('This markdown content type can not be saved because markdown tool has not been configured properly.').'</span><br />';

			}

			//print editor
			text_area($prefix.'_markdown', $markdown, '', '', 0, 'style="width: '.$size['width'].'; height: '.$size['height'].'; font-family: \'Courier New\', Courier, monospace; display: block"');
		}
		return TRUE;

	}//end paintBackend()


	/**
	* Processes the backend interface for this Markdown editor
	*
	* @param array	$link	information used to create the initial link
	* @param object	&$asset	the markdown content type object
	* @param string	$prefix	prefix for form elements
	*
	* @return boolean
	* @access public
	*/
	function processBackend($link, &$asset, $prefix)
	{
		if ($asset->writeAccess('attributes')) {
			if (isset($_POST[$prefix.'_markdown'])) {
				$markdown = $_POST[$prefix.'_markdown'];
				$current_markdown = $asset->attr('markdown');
				if ($markdown != $current_markdown) {
					require_once SQ_DATA_PATH.'/private/conf/tools.inc';
					require_once SQ_FUDGE_PATH.'/general/text.inc';
					if (SQ_TOOL_MARKDOWN_ENABLED && is_file(SQ_TOOL_MARKDOWN_PATH)) {
						require_once SQ_TOOL_MARKDOWN_PATH;
						if (function_exists('Markdown')) {
							// Keep the markdown 'pure' only make changes on safe_markdown
							$safe_markdown = $markdown;

							// Find the keywords and make them 'markdown-proof'
							$keywords = retrieve_keywords_replacements($markdown);
							foreach ($keywords as $keyword) {
								$safe_keyword = '%'.str_replace('_', '[MY3SEP]', $keyword).'%';
								$safe_markdown = str_replace('%'.$keyword.'%', $safe_keyword, $safe_markdown);
							}//end foreach

							// Convert Markdown to HTML
							$html = Markdown($safe_markdown);

							// Change the safe keywords back to normal keywords
							foreach ($keywords as $keyword) {
								$safe_keyword = '%'.str_replace('_', '[MY3SEP]', $keyword).'%';
								$html = str_replace($safe_keyword, '%'.$keyword.'%', $html);
							}//end foreach

							// Save the changes
							if ($asset->setAttrValue('markdown', $markdown) && $asset->setAttrValue('html', $html)) {
								return TRUE;
							}
						}
					} else {
						trigger_localised_error('CMS0107', E_USER_WARNING);
					}

				}//end if
			}
		}
		return FALSE;

	}//end processBackend()


}//end class

?>
