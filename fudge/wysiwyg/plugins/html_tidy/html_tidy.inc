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
* $Id: html_tidy.inc,v 1.23 2004/09/23 22:16:21 amiller Exp $
* $Name: not supported by cvs2svn $
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
* @author  Greg Sherwood <greg@squiz.net>
* @version $Version$ - 1.0
* @package Fudge
* @subpackage wysiwyg
*/
class HTML_Tidy extends Wysiwyg_Plugin {

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
	var $_show_in_toolbar = false;


	/**
	* Constructor
	*
	* @return object HTML_Tidy
	* @access public
	*/
	function HTML_Tidy()
	{
		$this->_add_button('replacetext','ReplaceText','Replace Text','true','11');

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
		if (empty($html)) return;


		  //////////////////////////////
		 //  UNSUPPORTED CHARACTERS  //
		//////////////////////////////

		// Remove the following unsupported character codes that do not get replaced
		// back to their original characters through the encoding process (mostly MS Word chars)
		$search  = Array("%u2018","%u2019","%u2013","%u2014","%u2026","%u201C","%u201D");
		$replace = Array("'"     , "'"    ,"-"     ,"-",     "...",   '"',     '"'     );
		$html = str_replace($search, $replace, $html);

		// replace unicode's URL '%u' character entities with their HTML entity equivs
		$html = preg_replace('/%u([0-9A-Fa-f]{4})/', '&#x$1;', $html);

		  /////////////////
		 //  HTML TIDY  //
		/////////////////

		// If HTML Tidy is enabled, let's rock'n'roll
		if (SQ_TOOL_HTML_TIDY_ENABLED) {

			// tidy the HTML produced using the HTMLTidy program
			$path_to_tidy = SQ_TOOL_HTML_TIDY_PATH;
			
			// is_executable doesn't exist on windows pre 5.0.0
			if (function_exists('is_executable')) {
				if (is_executable($path_to_tidy)) {
					$command = "/bin/echo ".escapeshellarg($html)." | $path_to_tidy -iq --show-body-only y --show-errors 0 --show-warnings 0 --wrap 0 -asxml --word-2000 1 --force-output 1";

					// add any inline tags that other plugins may use (possibly for encapsulating plugin specific data)
					// so HTMLTidy doesnt strip them out
					if (!empty($this->_new_inline_tags)) $command .= ' --new-inline-tags "'.implode('","',$this->_new_inline_tags).'"';
					
					$tidy = Array();
					exec($command, $tidy);
					if (empty($tidy)) return;
					$html = implode("\n", $tidy);
					unset($tidy);
				}
			}
		}

		  ////////////////////
		 //  MS WORD CRAP  //
		////////////////////

		// Remove weird tags MS Word leaves in its output
		$html = preg_replace('|<([\w]+)([^>]+?)class="mso[^"]+"([^>]+)?>|is', '<\\1>', $html);
		$html = preg_replace('|<([\w]+)([^>]+?)style="([^"]+)?mso([^"]+)?"([^>]+)?>|is', '<\\1>', $html);
		$html = preg_replace('|<\/?\w+:[^>]*>|is', '', $html);
		$html = preg_replace('|<font[^>]+>(.+?)</font>|is', '\\1', $html);
		$html = preg_replace('|<span([^>]+?)lang=[^\s]+([^>]+?)xml:lang="[^\s]+">(.+?)</span>|is', '\\3', $html);
		$html = preg_replace('|<\?xml[^>]+microsoft[^>]+\?>|is', '', $html);
		$html = preg_replace('|<\/?\w+:[^>]*>|is', '', $html);
		$html = preg_replace('|<\\?\??xml[^>]>|is', '', $html);


	}//end process


	/**
	* Open the popup colour selector to get the colour from the user
	*
	* @returns void
	* @access  public
	*/
	function print_plugin_button_click()
	{
		?>
		case "replacetext":
		this._popupDialog("replacetext", "<?php echo $this->get_popup_href('html_tidy.html')?>", 380, 400, true, function(types){
			if (types == null) return false;
			var reg = null;
			var rep = null;
			var curHTML = editor.getHTML();
			var HTMLtags = "!--|a|abbr|above|acronym|address|applet|array|area|b|base|basefont|bdo|bgsound|big|blink|blockquote|body|box|br|blink|button|caption|center|cite|code|col|colgroup|comment|dd|del|dfn|dir|div|dl|dt|em|embed|fieldset|fig|font|form|frame|frameset|h1|h2|h3|h4|h5|h6|head|hr|html|i|id|iframe|ilayer|img|input|ins|isindex|kbd|label|layer|legend|li|link|listing|map|marquee|menu|meta|multicol|nextid|nobr|noframes|nolayer|note|noscript|object|ol|option|keygen|optgroup|p|param|pre|q|quote|range|root|s|samp|script|select|small|sound|spacer|span|sqrt|strike|strong|style|sub|sup|table|tbody|td|text|textarea|tfoot|th|thead|title|tr|tt|u|ul|var|wbr|xmp";
			var bullet = unescape("%B7");
			for (var i=0;i<types.length;i++)
				if (types[i])
					{
					switch (i) {
						case 0	: {
							reg = new RegExp("<font[^>]*>(.*?)<\/font>", "gi");
							rep = "$1";
							break;
						}
						case 1	: {
							reg = new RegExp(" style=\"?[^\">]*\"?", "gi");
							rep = "";
							break;
						}
						case 2	: {
							reg = new RegExp(" class=\"?[^\"> ]*[\" ]?", "gi");
							rep = "";
							break;
						}
						case 3	: {
							reg = new RegExp("<(table|/table|tr|tbody|/tbody|td|th) ?[^>]*>", "gi");
							rep = "";
							curHTML = curHTML.replace(reg, rep);

							reg = new RegExp("<(/tr|/td|/th)>", "gi");
							rep = "<br />";
							break;
						}
						case 4	: {
							reg = new RegExp("<\/?span( [^>]*>|>)", "gi");
							rep = "";
							break;
						}
						case 5	: {
							reg = new RegExp("<(?!(\/?(" + HTMLtags + ")[> ]))([^>]*)>", "gi");
							rep = "";
							break;
						}
						case 6	: {
							reg = new RegExp("(&nbsp;| |\t|\n){2,}", "gi");
							rep = "$1";
							break;
						}
						case 7	: {
							reg = new RegExp("<p[^>]*>(&nbsp;| |\n|\t|<(.*?)>)?<\/p>", "gi");
							rep = "";
							break;
						}
						case 8	: {
							reg = new RegExp('<([^\/ >]+)([^>]*?( (src|href)="?[^>"]*"?)*)*>', "gi");
							rep = "<$1$3>";
							break;
						}
						case 9	: {
							reg = new RegExp("<p[^>]*>"+bullet+"(.*?)<\/p>", "gi");
							rep = "<li>$1";
							break;
						}
						default	: return false;
					}

					curHTML = curHTML.replace(reg, rep);
					}
			if (types.length>0) editor.setHTML(curHTML);
		}, null);
		break;
		<?php

	}//end print_plugin_button_click()


}//end class

?>
