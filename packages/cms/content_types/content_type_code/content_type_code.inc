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
* $Id: content_type_code.inc,v 1.3 2006/01/03 02:53:54 dmckee Exp $
*
*/


require_once SQ_CORE_PACKAGE_PATH.'/content_type/content_type.inc';

/**
* Content_Type_Code
*
* Purpose
*
*
* @author  Darren McKee <dmckee@squiz.net>
* @version $Revision: 1.3 $
* @package MySource_Matrix_Packages
* @subpackage cms
*/
class Content_Type_Code extends Content_Type
{


	/**
	* Constructor
	*
	* @param int	$assetid	the asset id to be loaded
	*
	*/
	function Content_Type_Code($assetid=0)
	{
		$this->_ser_attrs = true;
		$this->Content_Type($assetid);

	}//end constructor


	/**
	* Highlight Other:
	* Highlights a string of code that isnt php
	*
	* @param string	$str	The string to Highlight
	*
	* @return string
	* @access public
	*/
	function highlightOther($str)
	{
		$highlighted = $str;

		$classes = $GLOBALS['SQ_SYSTEM']->getUserPrefs($this->type(), 'SQ_CODE_EDITOR_HTML_CLASSES');

		//Match Comments
		$highlighted = str_replace('<!--', '&lt!--', $highlighted);
		$highlighted = str_replace('-->', '--&gt;', $highlighted);

		//$highlighted = preg_replace('/COMMENT_START(.*|\n)COMMENT_END/i', 'SPANSTART class="'.$classes['html_comment'].'>&lt;--\\1--&gt;ENDSPAN',$highlighted);


		//Below Matches Single html tags(those without attributes specifified, as well as singleton tags);
		preg_match_all('/\<([A-Za-z]+[A-Za-z0-9]*)(\s*\/)\>/i', $highlighted, $matches=Array());

		for ($i = 0; $i < count($matches[0]); $i++) {
			$needle = $matches[0][$i];
			$replace = 'SPANSTART class="'.$classes['tag_brackets'].'">&lt;ENDSPAN';
			$replace .= 'SPANSTART class="'.$classes['tag_name'].'">'.$matches[1][$i].'ENDSPAN';
			$replace .= 'SPANSTART class="'.$classes['tag_brackets'].'">'.$matches[2][$i].'&gt;ENDSPAN';
			$highlighted = str_replace($needle, $replace, $highlighted);
		}

		//End tags ( </aaa> )
		preg_match_all('/\<\/\s*([A-Za-z][A-Za-z0-9]*)\>/i', $highlighted, $matches=Array());

		for ($i = 0; $i < count($matches[0]); $i++) {
			$needle = $matches[0][$i];
			$replace = 'SPANSTART class="'.$classes['tag_brackets'].'">&lt;/ENDSPAN';
			$replace .= 'SPANSTART class="'.$classes['tag_name'].'">'.$matches[1][$i].'ENDSPAN';
			$replace .= 'SPANSTART class="'.$classes['tag_brackets'].'">&gt;ENDSPAN';
			$highlighted = str_replace($needle, $replace, $highlighted);
		}

		$simple_matches = Array();

		//Tags like <h1> etc;
		preg_match_all('/\<\s*([A-Za-z]+[A-Za-z0-9]*)\>/i', $highlighted, $matches=Array());

		for ($i = 0; $i < count($matches[0]); $i++) {
			$needle = $matches[0][$i];
			$replace = 'SPANSTART class="'.$classes['tag_brackets'].'">&lt;ENDSPAN';
			$replace .= 'SPANSTART class="'.$classes['tag_name'].'">'.$matches[1][$i].'ENDSPAN';
			$replace .= 'SPANSTART class="'.$classes['tag_brackets'].'">&gt;ENDSPAN';
			$highlighted = str_replace($needle, $replace, $highlighted);
		}

		foreach ($simple_matches as $needle) {
			$replace = 'SPANSTART style="color: blue;">'.htmlspecialchars($needle).'ENDSPAN';
			$highlighted = str_replace($needle, $replace, $highlighted);
		}

		//Match Param Names
		preg_match_all('/<\\s*([A-Za-z][A-Za-z0-9]*)\\s+([^>]*)\\s*>/i', $highlighted, $matches=Array());

		for ($i = 0; $i < count($matches[0]); $i++) {
			$original = $matches[0][$i];
			$replace_text = $original;
			$replace_text = str_replace('<', '&lt;', $replace_text);
			$replace_text = str_replace('/>', 'ORIG_END_TAG', $replace_text);
			$replace_text = str_replace('>', 'TAG_END_BRACE', $replace_text);
			$replace_text = preg_replace('/([a-z]+-?_?[a-z]*)\\s*=\\s*\"([^\"]*)\"/i', 'SPANSTART class="'.$classes['attribute_names'].'">$1ENDSPAN=SPANSTART class="'.$classes['attribute_value'].'">&quot;$2&quot;ENDSPAN', $replace_text);
			$replace_text = preg_replace('/&lt;('.trim($matches[1][$i]).')/i', 'SPANSTART class="'.$classes['tag_brackets'].'">&lt;ENDSPANSPANSTART class="'.$classes['tag_name'].'">'.$matches[1][$i].'ENDSPAN', $replace_text);

			$highlighted = str_replace($original, $replace_text, $highlighted);
		}

		$highlighted = str_replace('<', '&lt;', $highlighted);

		$highlighted = str_replace('SPANSTART', '<span', $highlighted);
		$highlighted = str_replace('ENDSPAN', '</span>', $highlighted);
		$highlighted = str_replace('ORIG_END_TAG', '<span class="'.$classes['tag_brackets'].'">/&gt;</span>', $highlighted);
		$highlighted = str_replace('TAG_END_BRACE', '<span class="'.$classes['tag_brackets'].'">&gt;</span>', $highlighted);

		return $highlighted;

	}//end highlightOther()


	/**
	* Highlight PHP:
	* Highlights a string of PHP Code
	*
	* @param string	$str	The string to Highlight
	*
	* @return string
	* @access public
	*/
	function highlightPHP($str)
	{
		$highlighted = $str;

		//Grab the preferences
		$classes = $GLOBALS['SQ_SYSTEM']->getUserPrefs($this->type(), 'SQ_CODE_EDITOR_PHP_CLASSES');

		$system_colours = Array();
		$system_colours[strtolower(strval(ini_get('highlight.bg')))] = $classes['background'];
		$system_colours[strtolower(strval(ini_get('highlight.default')))] = $classes['default'];
		$system_colours[strtolower(strval(ini_get('highlight.html')))] = $classes['html'];
		$system_colours[strtolower(strval(ini_get('highlight.keyword')))] = $classes['keyword'];
		$system_colours[strtolower(strval(ini_get('highlight.comment')))] = $classes['comment'];
		$system_colours[strtolower(strval(ini_get('highlight.string')))] = $classes['string'];

		$one_liner = false;
		//Check if its only one line
		if (substr_count($highlighted, ';') <= 1) {
			$one_liner = true;
		}

		$highlighted = highlight_string($highlighted, true);

		$highlighted = str_replace('<code>', '', $highlighted);
		$highlighted = str_replace('</code>', '', $highlighted);
		$highlighted = str_replace('&nbsp;', ' ', $highlighted);

		$highlighted = str_replace(Array('<font ', '</font>'), Array('<span ', '</span>'), $highlighted);
		foreach ($classes as $colour => $class) {
			$highlighted = str_replace('color="'.strtoupper($colour).'"', 'class="'.$class.'"', $highlighted);
		}
		preg_match_all('#color="(.*?)"#', $highlighted, $matches=Array());
		for ($i = 0; $i < count($matches[0]); $i++) {
			$needle = $matches[0][$i];
			$replace_class = $system_colours[strtolower($matches[1][$i])];
			$replace = 'class="'.$replace_class.'"';
			$highlighted = str_replace($needle, $replace, $highlighted);
		}

		//If there is only one expression, we want it to appear in the current context, so we have to strip out any new lines
		if ($one_liner == true) {
			$highlighted = str_replace("\n", '', $highlighted);
		}
		//PHP Seems to pop this span over multiple lines when it has similar elements in a row
		//eg. if there is a brace ({) and a var on the next line, the highlighter sees them as the same
		//grouping and the span puts both inside. This will split those up into 2 spans
		//preg_match_all('/<span class\=".*">\{<br \/>.*<\/span>/i', $highlighted, $matches=Array());

		$highlighted = str_replace('<br />', "\n", $highlighted);

		return $highlighted;

	}//end highlightPHP()


	/**
	* Highlight Script:
	* Highlights a string of Javascript Code
	*
	* @param string	$str	The string to Highlight
	*
	* @return string
	* @access public
	*/
	function highlightScript($str)
	{
		$reserved = Array(
						'abstract', 'as', 'boolean', 'break', 'byte', 'case', 'catch', 'char', 'class', 'continue', 'const',
						'debugger', 'default', 'delete', 'do', 'double', 'else', 'enum', 'export', 'extends', 'false', 'final',
						'finally', 'float', 'for', 'function', 'goto', 'if', 'implements', 'import', 'in', 'instanceof', 'int', 'interface',
						'is', 'long', 'namespace', 'native', 'new', 'null', 'package', 'private', 'protected', 'public', 'return', 'short', 'static',
						'super', 'switch', 'synchronized', 'this', 'throw', 'throws', 'transient', 'true', 'try', 'typeof', 'use', 'var', 'void', 'volatile',
						'while', 'with',
					);
		$highlighted = str_replace(Array('jsender', 'jsstarter'), '' , $str);

		//Get our user preferences
		$classes = $GLOBALS['SQ_SYSTEM']->getUserPrefs($this->type(), 'SQ_CODE_EDITOR_JS_CLASSES');

		//Highlight any reserved words
		foreach ($reserved as $word) {
			$highlighted = str_replace($word, '<span class="'.$classes['reserved'].'">'.$word.'</span>', $highlighted);
		}
		$highlighted = preg_replace('/(\".*\")/i', '<span class="'.$classes['string'].'">\\1</span>', $highlighted);
		$highlighted = preg_replace('/(\'.*\')/i', '<span class="'.$classes['string'].'">\\1</span>', $highlighted);
		$highlighted = preg_replace('/([a-zA-Z]*)\((.*)\)/i', '<span class="'.$classes['function'].'">\\1</span><span class="'.$classes['brackets'].'">(</span><span class="'.$classes['default'].'">\\2</span><span class="'.$classes['brackets'].'">)</span>', $highlighted);

		return $highlighted;

	}//end highlightScript()


	/**
	* syntaxHighlight
	* Highlights a string of code
	*
	* @param string	$str	The string to Highlight
	*
	* @return string
	* @access public
	*/
	function syntaxHighlight($str)
	{
		$highlighted = '';

		$php_start_token = 'PHPSTARTTAG';
		$php_end_token = 'PHPENDTAG';

		$str = str_replace('<?php', $php_start_token, $str);
		$str = str_replace('<?PHP', $php_start_token, $str);
		$str = str_replace('<? ', $php_start_token.' ', $str);
		$str = str_replace(' ?>', ' '.$php_end_token, $str);
		$str = str_replace('<?', $php_start_token.' ', $str);
		$str = str_replace('?>', ' '.$php_end_token, $str);

		//Replace any php code that appears between quotes (ie in an html attribute)
		//Replaced with an alphabetical string, so that attributes see this as a valid attribute value
		$str = preg_replace('/\"(.*)'.$php_start_token.'(.*)'.$php_end_token.'(.*)\"/i', '"\\1inlinephpstart\\2inlinephpend\\3"',$str);

		//Get some javascript gear in here now. The rest of the text will be parsed as usual(like inline PHP, quotes etc.
		//But this may highlight some reserved words etc.
		$str = preg_replace('/(script .*javascript.*>)/i', '\\1jsstarter', $str);
		$str = preg_replace('/(\<\/script>)/i', 'jsender\\1', $str);

		//Keep shortening the original string, by cutting it at the php tags
		while (false !== ($start_pos = strpos($str, $php_start_token))) {
			$other_string = substr($str, 0, $start_pos);

			$highlighted .= $this->highlightOther($other_string);
			$str = substr($str, $start_pos);
			$length = strlen($str);
			if (strpos($str, $php_end_token) === false) {
				$str .= $php_end_token;
			}

			$end_pos = strpos($str, $php_end_token);
			$end_pos += strlen($php_end_token);
			$php_string = substr($str, 0, $end_pos);

			$php_string = str_replace($php_start_token, '<?php', $php_string);
			$php_string = str_replace($php_end_token, '?>', $php_string);

			$highlighted .= $this->highlightPHP($php_string);

			$str = substr($str, strpos($str, $php_end_token) + strlen($php_end_token));

		}//End While PHP Tags exist

		//Get the bit after the last PHP tag
		$highlighted .= $this->highlightOther($str);

		$start_pos = strpos($highlighted, 'jsstarter');
		$end_pos = strpos($highlighted, 'jsender');
		$script = substr($highlighted, $start_pos, $end_pos - $start_pos + 7);
		if ($end_pos !== FALSE || $start_pos !== FALSE) {
			$highlighted = str_replace($script, $this->highlightScript($script), $highlighted);
		}

		$highlighted = str_replace(Array('inlinephpstart', 'inlinephpend'), Array('&lt;?php', '?&gt;'), $highlighted);

		$line_num_prefs = $GLOBALS['SQ_SYSTEM']->getUserPrefs($this->type(), 'SQ_CODE_EDITOR_LINE_NUMBERS');
		if ($line_num_prefs['use_line_numbers'] == true) {
			$highlighted = $this->addLineNumbers($highlighted, $line_num_prefs['line_number_left'], $line_num_prefs['line_number_style'], $line_num_prefs['line_number_class']);
		}
		return '<pre>'.$highlighted.'</pre>';

	}//end syntaxHighlight()


	/**
	* addLineNumbers
	* Adds line numbers to a string of code.
	*
	* @param string	$str	The string to Highlight
	* @param string	$left	Display at left?
	* @param string	$style	Style to display. span, or list so far
	* @param string	$class	CSS class to apply to line numbers
	*
	* @return string
	* @access public
	*/
	function addLineNumbers($str, $left=true, $style='span', $class='line_numbers')
	{
		$lines = explode("\n", $str);

		if ($left == false) $style='span';
		switch (strtolower($style)) {
			case 'span':
				if ($left) {
					// Setup the replace value.
					// We leave -margin_value- and -line_number- in there, as they will be determined later
					$replace = '<span class="'.$class.'" style="-margin_value-">-line_number-</span>';
					$replace_right = '<span class="'.$class.'" style="right: 0px;">-line_number-</span>';
					$replace = ($left==true) ? "\n$replace" : "$replace\n";

					//Replace all the new lines with our gear
					if ($left) {
						$str = preg_replace('/\n/i', $replace, $str);
						//$str = preg_replace('/\r/i', '', $str);
						//Add one for the first line
						$str = $replace.$str;
					}

					//Now we've gotta get through all the line numbers, and replace them, but also keep a track
					//of how many there are
					$line_number = 1;
					while (false !== strpos($str, '-line_number-')) {
						$str = preg_replace('/-line_number-/i', $line_number.'.', $str, 1);
						$line_number++;
					}

					//Work out the margin based on the number of digits in our line numbers
					$margin = strlen(strval($line_number)) * 5 + 5;

					$margin_replace = ($left == true) ? 'margin-right' : 'margin-left';
					$margin_replace .= ': '.$margin.'px;';

					$str = str_replace('-margin_value-', $margin_replace, $str);
				} else {
					//Line Numbers on the right
					$new_str = '<table style="width: 100%;" ><tr><td><pre>'.$str.'</pre></td><td style="width: 100px;" >';
					$line_numbers = range(1, count($lines));
					$new_str .= '<pre>'.implode("\n", $line_numbers).'</pre>';
					$new_str .= '</td></tr></table>';
					$str = $new_str;
				}
			break;
			case 'list':
				$str = implode('</pre></li><li class="'.$class.'"><pre>', $lines);
				$str = '<ol><li class="'.$class.'">'.$str.'</li></ol>';
			break;
		}
		return $str;

	}//end addLineNumbers()


}//end class

?>