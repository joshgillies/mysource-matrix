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
* $Id: html_tidy.inc,v 1.40 2006/09/27 23:14:53 gsherwood Exp $
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
* @version $Revision: 1.40 $
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
		if (in_array($tag_name, $this->_new_inline_tags)) return true;
		$this->_new_inline_tags[] = $tag_name;
		return true;

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
			$search  = Array("%u2018",  "%u2019",  "%u2013",  "%u2014",  "%u2026",   "%u201C",  "%u201D");
			$replace = Array("&lsquo;", "&rsquo;", "&ndash;", "&mdash;", "&hellip;", "&ldquo;", "&rdquo;");
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
			if (SQ_TOOL_HTML_TIDY_ENABLED && SQ_TOOL_HTML_TIDY_PATH != '') {
				// tidy the HTML produced using the HTMLTidy program
				$path_to_tidy = SQ_TOOL_HTML_TIDY_PATH;

				// is_executable doesn't exist on windows pre 5.0.0
				if (function_exists('is_executable')) {
					if (is_executable($path_to_tidy)) {
						 $command = "/bin/echo ".escapeshellarg($html)." | $path_to_tidy -iq --show-body-only y --show-errors 0 --show-warnings 0 --wrap 0 -asxml --quote-marks 1 --word-2000 1 --force-output 1";
						// add any inline tags that other plugins may use (possibly for encapsulating plugin specific data)
						// so HTMLTidy doesnt strip them out
						if (!empty($this->_new_inline_tags)) $command .= ' --new-inline-tags "'.implode('","',$this->_new_inline_tags).'"';

						$tidy = Array();
						exec($command, $tidy);
						if (empty($tidy)) {
							$this->htmltidy_status = 'fail';
						} else {
							$html = implode("\n", $tidy);

							//Change old style HTML Tags to XHTML compliant self closing tags (singletons)
							$html = preg_replace('|<br>|is', '<br />', $html);
							$html = preg_replace('|<hr>|is', '<hr />', $html);
						}
						unset($tidy);
					}
				}
			}
		}

	}//end process


	/**
	* Open the HTML tidy dialog
	*
	* @returns void
	* @access  public
	*/
	function print_plugin_button_click()
	{
		?>
		case "replacetext":
				this._popupDialog("ReplaceText", "<?php echo $this->get_popup_href('html_tidy.php')?>", 440, 520, true, function(types){
					if (types == null) return false;
					var reg = null;
					var rep = null;
					var selected_html = types[0];
					if (types[0] && editor.getSelectedHTML().length != 0) {
						selected_html = true;
						var curHTML = editor.getSelectedHTML();
					} else if (!types[0]) {
						selected_html = false;
						var curHTML = editor.getHTML();
					} else {
						alert("Selected text is not defined. Replace Text action aborted.");
						return false;
					}
					var HTMLtags = "!--|a|abbr|above|acronym|address|applet|array|area|b|base|basefont|bdo|bgsound|big|blink|blockquote|body|box|br|blink|button|caption|center|cite|code|col|colgroup|comment|dd|del|dfn|dir|div|dl|dt|em|embed|fieldset|fig|font|form|frame|frameset|h1|h2|h3|h4|h5|h6|head|hr|html|i|id|iframe|ilayer|img|input|ins|isindex|kbd|label|layer|legend|li|link|listing|map|marquee|menu|meta|multicol|nextid|nobr|noframes|nolayer|note|noscript|object|ol|option|keygen|optgroup|p|param|pre|q|quote|range|root|s|samp|script|select|small|sound|spacer|span|sqrt|strike|strong|style|sub|sup|table|tbody|td|text|textarea|tfoot|th|thead|title|tr|tt|u|ul|var|wbr|xmp";
					var bullet = unescape("%B7");
					for (var i=1;i<types.length;i++)
						if (types[i])
							{
							switch (i) {
								case 1	: {
									reg = new RegExp("<\/?font ?[^>]*>", "gi");
									rep = "";
									break;
								}
								case 2	: {
									reg = new RegExp("(&nbsp;| |\t|\n){2,}", "gi");
									rep = "$1";
									break;
								}
								case 3	: {
									reg = new RegExp("<(?!(\/?(" + HTMLtags + ")[> ]))([^>]*)>", "gi");
									rep = "";
									break;
								}
								case 4	: {
									reg = new RegExp("<p[^>]*>"+bullet+"(.*?)<\/p>", "gi");
									rep = "<li>$1";
									break;
								}
								case 5	: {
									reg = new RegExp(" style=\"?[^\">]*\"?", "gi");
									rep = "";
									break;
								}
								case 6	: {
									reg = new RegExp(" class=\"?[^\"> ]*[\"]?", "gi");
									rep = "";
									break;
								}
								case 7	: {
									reg = new RegExp("<(table|/table|tr|tbody|/tbody|td|th) ?[^>]*>", "gi");
									rep = "";
									curHTML = curHTML.replace(reg, rep);
									reg = new RegExp("<(/tr|/td|/th)>", "gi");
									rep = "<br />";
									break;
								}
								case 8	: {
									reg = new RegExp("<\/?span( [^>]*>|>)", "gi");
									rep = "";
									break;
								}
								case 9	: {
									reg = new RegExp("<([A-Z][A-Z0-9]*)( [^>]*)?>(&nbsp;| |\n|\t)*<\/\\1>", "gi");
									rep = "";
									break;
								}
								case 10	: {
									reg = new RegExp('<([^\/ >]+)([^>]*?( (src|href)="?[^>"]*"?)*)*>', "gi");
									rep = "<$1$3>";
									break;
								}
								default	: return false;
							}
							// BUG#928 - special condition to allow empty anchor tag
							if (i == 9) {
									reg2 = new RegExp("(<A NAME[^>]*?>)(&nbsp;| |\n|\t)*(</A>)", "gi");
									rep2 = "$1matrix_anchor_tmp$3"
									curHTML = curHTML.replace(reg2, rep2);
									reg2 = new RegExp("(<A ID[^>]*?>)(&nbsp;| |\n|\t)*(</A>)", "gi");
									rep2 = "$1matrix_anchor_tmp$3"
									curHTML = curHTML.replace(reg2, rep2);
							}
							curHTML = curHTML.replace(reg, rep);
							if (i == 9) {
									reg3 = new RegExp("(<A NAME[^>]*?>)matrix_anchor_tmp(</A>)", "gi");
									rep3 = "$1$2"
									curHTML = curHTML.replace(reg3, rep3);
									reg3 = new RegExp("(<A ID[^>]*?>)matrix_anchor_tmp(</A>)", "gi");
									rep3 = "$1$2"
									curHTML = curHTML.replace(reg3, rep3);
							}
						}
					if (selected_html) {
						if (types.length>0) editor.insertHTML(curHTML);
					} else {
						if (types.length>0) editor.setHTML(curHTML);
					}
				}, null);
		break;
		<?php

	}//end print_plugin_button_click()


}//end class

?>
