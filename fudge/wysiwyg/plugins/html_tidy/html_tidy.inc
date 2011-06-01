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
* $Id: html_tidy.inc,v 1.67.2.1 2009/05/01 02:04:04 bpearson Exp $
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
* @version $Revision: 1.67.2.1 $
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
				$path_to_tidy = SQ_TOOL_HTML_TIDY_PATH;

				$config = Array (
							'--preserve-entities'	=> TRUE,
							'--show-body-only'		=> TRUE,
							'--show-errors'			=> 1,
							'--show-warnings'		=> TRUE,
							'--wrap'				=> FALSE,
							'--quote-marks'			=> TRUE,
							'--word-2000'			=> TRUE,
							'--force-output'		=> TRUE,
							'--char-encoding'		=> $tidy_char_set,
							'-access'				=> SQ_TOOL_HTML_TIDY_ACCESSIBILITY_LEVEL,
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

				// replaces &middot; with a <li>
				$html = str_replace('&middot;','<li>',$html);

				$complete_html = '<!DOCTYPE HTML PUBLIC "-//W3C//DTD HTML 4.01 Transitional//EN" "http://www.w3.org/TR/html4/loose.dtd">
						<html lang="en"><head><title></title>
						<meta name="description" content="test content" />
						</head><body>
						'.$html.'
						</body></html>';

				$descriptorspec = array(
					0 => Array("pipe", "r"),  // stdin is a pipe that the child will read fromi
					1 => Array("pipe", "w"),  // stdout is a pipe that the child will write to
					2 => Array("pipe", "w"),  // stderr is a file to write to
				);

				$process = proc_open("$path_to_tidy $args ", $descriptorspec, $pipes);

				if (is_resource($process)) {
					$STDIN = $pipes[0];
					$STDOUT = $pipes[1];
					$STDERR = $pipes[2];

					// Send html data and receive tidied html data.
					fwrite($STDIN, $complete_html);
					fclose($STDIN);
					$tidy = stream_get_contents($STDOUT);
					fclose($STDOUT);
					
					// Find the errors
					$errors = Array($STDERR);
					$error = '';
					foreach ($errors as $key => $err) {
						switch ($err) {
							case $STDERR:
								$error = stream_get_contents($STDERR)."\n";
								break;
						}
					}
					fclose($STDERR);
					$return = proc_close($process);
				}

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
		return translate('wysiwyg_htmltidy_status_disabled');

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
