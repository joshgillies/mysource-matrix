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
* $Id: html_tidy.inc,v 1.79 2013/08/15 07:15:51 ewang Exp $
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
* @version $Revision: 1.79 $
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

	// HTML versions and their respective doctype declaration
	var $html_standards = Array(
							'HTML_4.01_Transitional' => '<!DOCTYPE HTML PUBLIC "-//W3C//DTD HTML 4.01 Transitional//EN" "http://www.w3.org/TR/html4/loose.dtd">',
							'HTML_4.01_Strict' => '<!DOCTYPE HTML PUBLIC "-//W3C//DTD HTML 4.01//EN" "http://www.w3.org/TR/html4/strict.dtd"> ',
							'HTML_4.01_Frameset' => '<!DOCTYPE HTML PUBLIC "-//W3C//DTD HTML 4.01 Frameset//EN" "http://www.w3.org/TR/html4/frameset.dtd">',
							'XHTML_1.0_Strict' => '<!DOCTYPE html PUBLIC "-//W3C//DTD XHTML 1.0 Strict//EN" "http://www.w3.org/TR/xhtml1/DTD/xhtml1-strict.dtd">',
							'XHTML_1.0_Transitional' => '<!DOCTYPE html PUBLIC "-//W3C//DTD XHTML 1.0 Transitional//EN" "http://www.w3.org/TR/xhtml1/DTD/xhtml1-transitional.dtd">',
							'XHTML_1.0_Frameset' => '<!DOCTYPE html PUBLIC "-//W3C//DTD XHTML 1.0 Frameset//EN" "http://www.w3.org/TR/xhtml1/DTD/xhtml1-frameset.dtd">',
							'XHTML_1.1' => '<!DOCTYPE html PUBLIC "-//W3C//DTD XHTML 1.1//EN" "http://www.w3.org/TR/xhtml11/DTD/xhtml11.dtd">',
						);



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

		if (!file_exists(SQ_TOOL_HTML_TIDY_PATH)) {
			// Disable HTML Tidy if path doesn't exist (of course!)
			$this->htmltidy_status = 'disabled';
		} else if (empty($html)) {
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

			// If the page is on a character set other than UTF-8,
			// change these MS Word common characters, otherwise HTML Tidy throws errors
			// because the UTF8 characters do not exist on other character sets like Western Euro ISO
			if (SQ_CONF_DEFAULT_CHARACTER_SET != 'utf-8') {
				$html = str_replace(chr(145), '&lsquo;', $html);	// left single quote
				$html = str_replace(chr(146), '&rsquo;', $html);	// right single quote
				$html = str_replace(chr(147), '&ldquo;', $html);	// left double quote
				$html = str_replace(chr(148), '&rdquo;', $html);	// right double quote
				$html = str_replace(chr(149), '&ndash;', $html);	// bullet
				$html = str_replace(chr(150), '&ndash;', $html);	// endash
				$html = str_replace(chr(151), '&mdash;', $html);	// emdash
			}//end if

			// replace unicode's URL '%u' character entities with their HTML entity equivs
			$html = preg_replace('/%u([0-9A-Fa-f]{4})/', '&#x$1;', $html);

			////////////////////////
			//  MS WORD CLEANUP  //
			//////////////////////

			// Remove weird tags MS Word leaves in its output
			$html = preg_replace('|<([\w]+)([^>]+?)class="?mso[^">]+"?([^>]+)?>|is', '<\\1>', $html);
			$html = preg_replace('|<([\w]+)([^>]+?)style="([^"]+)?mso([^"]+)?"([^>]+)?>|is', '<\\1>', $html);
			$html = preg_replace('|<\/?\w+:[^>]*>|is', '', $html);
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
				$path_to_tidy = SQ_TOOL_HTML_TIDY_PATH;

				$config = Array (
							'--preserve-entities'	=> TRUE,
							'--show-body-only'		=> TRUE,
							'--show-errors'			=> 1,
							'--show-warnings'		=> TRUE,
							'--wrap'				=> FALSE,
							'--word-2000'			=> TRUE,
							'--force-output'		=> TRUE,
							'--logical-emphasis'	=> TRUE,
							'--char-encoding'		=> $tidy_char_set,
							'-access'				=> (int) SQ_TOOL_HTML_TIDY_ACCESSIBILITY_LEVEL,
						  );

				// add any inline tags that other plugins may use (possibly for encapsulating plugin specific data)
				// so HTMLTidy doesnt strip them out
				if (!empty($this->_new_inline_tags)) {
					$config['--new-inline-tags']	= implode('","',$this->_new_inline_tags);
				}

				// Read the config variables into a string of arguments
				$args = ' -iq -asxhtml';
				foreach ($config as $param => $option) {
					$args .= ' '.$param.' '.((is_bool($option)) ? (int) $option : $option);
				}

				$doctype_header= $this->html_standards[SQ_TOOL_HTML_TIDY_HTML_STANDARD ? SQ_TOOL_HTML_TIDY_HTML_STANDARD : 'HTML_4.01_Transitional'];
				$complete_html = $doctype_header.'
						<html lang="en"><head><title></title>
						<meta name="description" content="test content" />
						</head><body>
						'.$html.'
						</body></html>';

				$input_file = tempnam(SQ_TEMP_PATH, 'tidy_in_');
				$output_file = tempnam(SQ_TEMP_PATH, 'tidy_out_');
				$error_file = tempnam(SQ_TEMP_PATH, 'tidy_err_');

				file_put_contents($input_file, $complete_html);
				touch($output_file);
				touch($error_file);

				$descriptorspec = array(
					0 => Array("file", $input_file, "r"),  // file that the child will read from
					1 => Array("file", $output_file, "w"),  // file that the child will write to
					2 => Array("file", $error_file, "w"),  // stderr is a file to write to
				);

				$process = proc_open("$path_to_tidy $args ", $descriptorspec, $pipes);

				if (is_resource($process)) {
					$return = proc_close($process);

					$tidy = file_get_contents($output_file);
					$error = file_get_contents($error_file);
				}

				unlink($input_file);
				unlink($output_file);
				unlink($error_file);

				// First ignore anything we don't want to show for whatever reason
				$ignore_errors_list = $this->getIgnoredErrors();
				foreach ($ignore_errors_list as $err) {
					$error = str_replace($err, '', $error);
				}

				// Filter out errors caused by inline tags used by WYSIWYG
				foreach ($this->_new_inline_tags as $tag) {
					$error = preg_replace('/line \d+ column \d+\ - Warning\: \<'.$tag.'\> is not approved by W3C/', '', $error);
				}

				$this->htmltidy_errors = trim($error);

				// A return of 2 from tidy indicates there were fatal errors.
				// (0 = no messages; 1 = only warnings, which we allow)
				if (($return >= 2) || empty($tidy) || empty($tidy_char_set)) {
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
			} else {
				$this->htmltidy_status = 'wait';
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
		if (SQ_TOOL_HTML_TIDY_ENABLED && file_exists(SQ_TOOL_HTML_TIDY_PATH)) {
			return TRUE;
		}
		return translate('HTML Tidy is disabled');

	}//end check_usable()


	/**
	* Ignore these errors
	*
	* @return array
	* @access public
	*/
	function getIgnoredErrors()
	{
		$ignore_list = Array();
		$ignore_list[] = 'line 1 column 1 - Access: [3.3.1.1]: use style sheets to control presentation.';
		$ignore_list[] = '
Accessibility Checks: Version 0.1
';

		return $ignore_list;

	}//end getIgnoredErrors()


}//end class

?>
