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
* $Id: html_tidy.inc,v 1.48 2007/03/05 22:03:44 rong Exp $
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
* @version $Revision: 1.48 $
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
				// tidy the HTML produced using the HTMLTidy program
				$path_to_tidy = SQ_TOOL_HTML_TIDY_PATH;

				// is_executable doesn't exist on windows pre 5.0.0
				if (function_exists('is_executable')) {
					if (is_executable($path_to_tidy)) {

						require SQ_FUDGE_PATH.'/standards_lists/character_sets.inc';
						$tidy_char_set = array_get_index($standards_lists_tidy_char_sets, SQ_CONF_DEFAULT_CHARACTER_SET);

						$command = "/bin/echo ".escapeshellarg($html)." | $path_to_tidy -iq --show-body-only y --show-errors 0 --show-warnings 0 --wrap 0 -asxml --quote-marks 1 --word-2000 1 --force-output 1 --char-encoding ".$tidy_char_set;
						// add any inline tags that other plugins may use (possibly for encapsulating plugin specific data)
						// so HTMLTidy doesnt strip them out
						if (!empty($this->_new_inline_tags)) {
							$command .= ' --new-inline-tags "'.implode('","',$this->_new_inline_tags).'"';
						}
						$tidy = Array();
						exec($command, $tidy);
						if (empty($tidy) || empty($tidy_char_set)) {
							$this->htmltidy_status = 'fail';
						} else {
							$html = implode("\n", $tidy);

							// Change old style HTML Tags to XHTML compliant self closing tags (singletons)
							$html = preg_replace('|<br>|is', '<br />', $html);
							$html = preg_replace('|<hr>|is', '<hr />', $html);
						}
						unset($tidy);
					}
				}
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
	function check_usable()
	{
		if (SQ_TOOL_HTML_TIDY_ENABLED && SQ_TOOL_HTML_TIDY_PATH != '') {
			return TRUE;
		}
		return translate('wysiwyg_htmltidy_status_disabled');

	}//end check_usable()


}//end class

?>
