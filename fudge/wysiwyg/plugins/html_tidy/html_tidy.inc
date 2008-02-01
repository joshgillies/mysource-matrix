<?php
/**
* +--------------------------------------------------------------------+
* | This MySource Matrix CMS file is Copyright (c) Squiz Pty Ltd       |
* | ACN 084 670 600                                                    |
* +--------------------------------------------------------------------+
* | IMPORTANT: Your use of this Software is subject to the terms of    |
* | the Licence provided in the file licence.txt. If you cannot find   |
* | this file please contact Squiz (www.squiz.net) so we may provide   |
* | you a copy.                                                        |
* +--------------------------------------------------------------------+
*
* $Id: html_tidy.inc,v 1.58 2008/02/01 01:19:59 hnguyen Exp $
*
*/


global $ROOT_PATH;
include_once($ROOT_PATH.'wysiwyg_plugin.inc');
require_once SQ_DATA_PATH.'/private/conf/tools.inc';


/**
* WYSIWYG Plugin - HTML Tidy
*
* Purpose
*     A WYSIWYG plugin to clean the HTML produced by
*     the editor with HTML Tidy
*
* @author	Greg Sherwood <greg@squiz.net>
* @author	Dmitry Baranovskiy <dbaranovskiy@squiz.net>
* @author	Scott Kim <skim@squiz.net>
* @version $Revision: 1.58 $
* @package Fudge
* @subpackage wysiwyg
*/
class HTML_Tidy extends Wysiwyg_Plugin
{

	/**
	* Speical inline tags that HTMLTidy needs to know about
	*
	* @var array
	* @see new_inline_tag()
	*/
	var $_new_inline_tags = Array();

	/**
	* Show this plugin in the toolbar or not
	*
	* @var array
	*/
	var $_show_in_toolbar = FALSE;


	/**
	* Flag to to the status of the last HTMLTidy action
	*
	* @var string
	*/
	var $htmltidy_status = 'pass';

	var $htmltidy_errors = NULL;


	/**
	* Constructor
	*
	* @return object HTML_Tidy
	* @access public
	*/
	function HTML_Tidy()
	{

	}//end constructor


	/**
	* Let HTMLTidy known about a special inline tag during cleaning
	*
	* @param string	$tag_name	the name of the new tag
	*
	* @return boolean
	* @access public
	*/
	function new_inline_tag($tag_name)
	{
		if (in_array($tag_name, $this->_new_inline_tags)) {
			return TRUE;
		}
		$this->_new_inline_tags[] = $tag_name;
		return TRUE;

	}//end new_inline_tag()


	/**
	* Cleans the submitted HTML using HTML Tidy
	*
	* @param string	&$html	the HTML that was submitted by the editor
	*
	* @return string
	* @access public
	*/
	function process(&$html)
	{
		$this->htmltidy_status = 'pass';

		if (empty($html)) {
			// skip as we cannot do tidy on empty content
			return;

		} else {
			//////////////////////////////
			//  UNSUPPORTED CHARACTERS  //
			//////////////////////////////

			// Remove the following unsupported character codes that do not get replaced
			// back to their original characters through the encoding process (mostly MS Word chars)
			$search  = Array('%u2018',  '%u2019',  '%u2013',  '%u2014',  '%u2026',   '%u201C',  '%u201D');
			$replace = Array('&lsquo;', '&rsquo;', '&ndash;', '&mdash;', '&hellip;', '&ldquo;', '&rdquo;');
			$html = str_replace($search, $replace, $html);

			// replace unicode's URL '%u' character entities with their HTML entity equivs
			$html = preg_replace('/%u([0-9A-Fa-f]{4})/', '&#x$1;', $html);

			////////////////////////
			//  MS WORD CLEANUP  //
			//////////////////////

			// Remove weird tags MS Word leaves in its output
			$html = preg_replace('|<([\w]+)([^>]+?)class="mso[^"]+"([^>]+)?>|is', '<\\1>', $html);
			$html = preg_replace('|<([\w]+)([^>]+?)style="([^"]+)?mso([^"]+)?"([^>]+)?>|is', '<\\1>', $html);
			$html = preg_replace('|<\/?\w+:[^>]*>|is', '', $html);
			$html = preg_replace('|<font[^>]+>(.+?)</font>|is', '\\1', $html);
			$html = preg_replace('|<span([^>]+?)lang=[^\s]+([^>]+?)xml:lang="[^\s]+">(.+?)</span>|is', '\\3', $html);
			$html = preg_replace('|<\?xml[^>]+microsoft[^>]+\?>|is', '', $html);
			$html = preg_replace('|<\/?\w+:[^>]*>|is', '', $html);
			$html = preg_replace('|<\\?\??xml[^>]>|is', '', $html);

			// Remove javascript tags
			$html = preg_replace('|<script\b[^>]*>(.*?)</script>|is', '', $html);

			/////////////////
			//  HTML TIDY  //
			/////////////////

			// If HTML Tidy is enabled, let's rock'n'roll
			if ($this->check_usable() === TRUE) {
                require SQ_FUDGE_PATH.'/standards_lists/character_sets.inc';
                $tidy_char_set = array_get_index($standards_lists_tidy_char_sets, SQ_CONF_DEFAULT_CHARACTER_SET);

				// tidy the HTML produced using the PHP5 Tidy
				$tidy = new tidy;

				$config = Array (
							'output-xhtml'			=> TRUE,
							'preserve-entities'		=> TRUE,
							'show-body-only'		=> TRUE,
							'wrap'					=> FALSE,
							'word-2000'				=> TRUE,
							'show-warnings'			=> FALSE,
							'show-errors'			=> 1,
							'char-encoding'			=> $tidy_char_set,
							'force-output'			=> TRUE,
							'quote-marks'			=> TRUE,
							'accessibility-check'	=> SQ_TOOL_HTML_TIDY_ACCESSIBILITY_LEVEL,
						  );

				// add any inline tags that other plugins may use (possibly for encapsulating plugin specific data)
				// so HTMLTidy doesnt strip them out
				if (!empty($this->_new_inline_tags)) {
					$config['new-inline-tags']	= implode('","',$this->_new_inline_tags);
				}

				$tidy->parseString($html, $config);
				$tidy->cleanRepair();

				$this->htmltidy_errors = $tidy->errorBuffer;

				if (empty($tidy) || empty($tidy_char_set)) {
					$this->htmltidy_status = 'fail';
				} else {
					// Only now we update the HTML
					$html = $tidy;

					// HTML Tidy adds a new line character after <pre> and beofre </pre> tag
					// which causes problem when the content is put back to WYSIWYG, replaced with a space
					$html = preg_replace('|<pre>\n|is', '<pre>', $html);
					$html = preg_replace('|\n</pre>|is', '</pre>', $html);
				}
				unset($tidy);
			}//end if
		}//end else

	}//end process()


	/**
	* Check whether this plugin is ready to be used.
	*
	* Return a TRUE if ok or a error message.
	*
	* @return mixed boolean|string
	* @access public
	* @static
	*/
	public static function check_usable()
	{
		if (SQ_TOOL_HTML_TIDY_ENABLED && class_exists('tidy')) {
			return TRUE;
		}
		return translate('wysiwyg_htmltidy_status_disabled');

	}//end check_usable()


}//end class

?>
