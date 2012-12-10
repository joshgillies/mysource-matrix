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
* $Id: matrix_diff_highlighter.inc,v 1.15 2012/12/10 06:47:52 cupreti Exp $
*
*/

// Text/Diff.php is throwing php5 warnings
// instead of overwritting the pear package, we are making the error mute
@include_once 'Text/Diff/Renderer.php';
include_once 'Text/Diff/Renderer/inline.php';

/**
* Matrix_Diff_Highlighter
*
* This class extends Pear::Text_Diff's Text_Diff_Renderer_Inline class,
* overriding certain functions to present the HTML output the way we
* want (showing text changes while preserving (valid) HTML).
*
* It also has a shortcut function process() which does some pre-processing
* of the input strings, calls Text_Dif and then does some post-processing
* on the result.
*
*
* @author  Tom Barrett <tbarrett@squiz.net>
* @version $Revision: 1.15 $
* @package MySource_Matrix_Packages
* @subpackage __core__
*/
class Matrix_Diff_Highlighter extends Text_Diff_Renderer_Inline
{


	/**
	* Mark the supplied content as having changed, using the supplied prefix and sufix
	*
	* @param array	$lines	The input lines or words
	* @param string	$prefix	Thing to put before changed content
	* @param string	$suffix	Thing to put after changed content
	*
	* @return string
	* @access private
	*/
	function _markChanges($lines, $prefix, $suffix)
	{
		if (empty($lines)) return '';
		$lines = $this->_splitOnWords(implode(' ', $lines));
		if (count($lines) == 1) {
			if (!$this->_isOpenTag($lines[0]) && !$this->_isCloseTag($lines[0])) {
				return $prefix.$lines[0].$suffix;
			} else {
				return $lines[0];
			}
		} else if (count($lines) >= 2) {
			$num_lines = 0;
			foreach ($lines as $i => $line) {
				if ($this->_isOpenTag($lines[$i]) ||  $this->_isCloseTag($lines[$i])){
					$num_lines++;
				}
			}
			if ($num_lines == count($lines)) return $this->_lines($lines, ' ', FALSE); 
		}

		if ($this->_isOpenBlockTag($lines[0]) && $this->_isCloseBlockTag($lines[count($lines)-1])) {
			// simple case - we have one or more block elements
			return $prefix.$this->_lines($lines, ' ', FALSE).$suffix;
		}

		$table_opened = FALSE;
		$list_opened = FALSE;
		$need_traversal = FALSE;
		foreach ($lines as $i => $line) {
			if (preg_match('/<table([^>]*)>/i', $line)) {
				$table_opened = TRUE;
			}
			if (preg_match('/<(ul|ol|dl)([^>]*)>/i', $line)) {
				$list_opened = TRUE;
			}
			if (preg_match('/<li([^>]*)>/i', $line) && !$list_opened) {
				// list item found
				$need_traversal = TRUE;
				break;
			}
			if (preg_match('/<td([^>]*)>/i', $line) && !$table_opened) {
				// list item found
				$need_traversal = TRUE;
				break;
			}
		}

		if ($need_traversal) {

			$opened = FALSE;
			foreach ($lines as $i => $line) {
				if (preg_match('/<(td|th|li)([^>]*)>/i', $line)) {
					//lets find the first closing tag (assuming valid html)
					$end_index = $i + 1;
					$pattern = '/<\/td>/i';
					if (preg_match('/<th([^>]*)>/i', $line)) {
						$pattern = '/<\/th>/i';
					} else if (preg_match('/<li([^>]*)>/i', $line)) {
						$pattern = '/<\/li>/i';
					}
					while ($end_index < count($lines)){
						if (preg_match($pattern, $lines[$end_index])) {
							$found = 1;
							break;
						}
					$end_index++;
					}
					//if we find the closing (td,th,li) tag lets make sure tags such as <p> are nested  
					//inside the <ins> and <del> tags which is what is causing tables to look distorted as per Bug #4384
					if ($found){
						$begin_index = $i + 1;
						while ($this->_isOpenTag($lines[$begin_index]) && ($begin_index < $end_index)) {
							$begin_index++;
						}
						$end_index--;
						while ($this->_isCloseTag($lines[$end_index]) && ($end_index > 0)) {
							$end_index--;
						}
					
						$lines[$begin_index] = $prefix.$lines[$begin_index];
						$lines[$end_index] .= $suffix;
						$found = 0;
					} else {
						$lines[$i] .= $prefix;
					}
				
					$opened = TRUE; //tracking if the last tag is an open tag
					if (!isset($first)){ //tracking whether first tag is open or close
						$first = "open";
					}
				} else if (preg_match('/<\/(td|th|li)([^>]*)>/i', $line)) {
					$opened = FALSE;
					if (!isset($first)){
						$first = "closed";
						//takes care of situations where the FIRST tag in the lines is an open and close tag
						//in this case we want <ins> or <del> tags AFTER.
						if ($this->_isOpenTag($lines[0]) || $this->_isCloseTag($lines[0])){
							$lines[0] = $lines[0].$prefix;
						} else {
							$lines[0] = $prefix.$lines[0];
						}		
						$end_index = $i - 1;
						while ($this->_isCloseTag($lines[$end_index]) && ($end_index > 0)) {
							$end_index--;
						}
						$lines[$end_index] .= $suffix;
					}
				}
			}
			
			if ($opened) {
				//takes care of situations where the LAST tag in the lines is an open and close tag
				//in this case we don't want any <ins> or <del> tags.
				if ($this->_isOpenTag($lines[count($lines)-1]) || $this->_isCloseTag($lines[count($lines)-1])){
					$lines[count($lines)-2] = str_replace($prefix, '',$lines[count($lines)-2]);
				} else {
					$lines[count($lines)-1] .= $suffix;
				}
			}
			return $this->_lines($lines, ' ', FALSE);

		} else {
			// First get rid of any invalid non matching block tags (bug #5932)
			$this->_removeInvalidBlockTags($lines);

			// simple case, just make sure we put the mod tags in the right place
			$begin_index = 0;
			while ($this->_isCloseTag($lines[$begin_index]) && ($begin_index < count($lines))) {
				$begin_index++;
			}
			$end_index = count($lines) - 1;
			while ($this->_isOpenTag($lines[$end_index]) && ($end_index > 0)) {
				$end_index--;
			}

			$lines[$begin_index] = $prefix.$lines[$begin_index];
			$lines[$end_index] .= $suffix;

			return $this->_lines($lines, ' ', FALSE);
		}

	}//end _markChanges()


	/**
	* Remove any non-matching /incorrectly nested block tags in the given lines block
	*
	* @param array $lines
	*
	* @return void
	* @access private
	*/
	private function _removeInvalidBlockTags(&$lines)
	{
		$opened_tags = Array();
		foreach($lines as $index => $tag) {
			if ($this->_isOpenTag($tag)) {
				$opened_tags[] = Array('line' => $index, 'tag' => trim(preg_replace('|<([^\s]+).*?>|', '$1', $tag)));
			} else if ($this->_isCloseTag($tag)) {
				if (!empty($opened_tags)) {
					$previous_tag = $opened_tags[count($opened_tags)-1]['tag'];
					$current_tag = trim(preg_replace('|</([^>]+)>|', '$1', $tag));
					if ($previous_tag != $current_tag) {
						// It does not has a matching opened tag, get rid of it
						$lines[$index] = '';
					} else {
						array_pop($opened_tags);
					}
				} else {
					$lines[$index] = '';
				}
			}//end else if
		}//end foreach

		// Also get rid of remaing non-closed opened tags
		foreach($opened_tags as $opened_tag) {
			$lines[$opened_tag['line']] = '';
		}//end foreach

	}//end _removeInvalidBlockTags()


	/**
	* Mark up the supplied content as having been added to the document
	*
	* @param array	$lines	The content
	*
	* @return string
	* @access private
	*/
	function _added($lines)
	{
		return $this->_markChanges($lines, $this->_ins_prefix, $this->_ins_suffix);

	}//end _added()


	/**
	* Mark up the supplied content as having been deleted from the document
	*
	* @param array	$lines	The content
	*
	* @return string
	* @access private
	*/
	function _deleted($lines)
	{
		return $this->_markChanges($lines, $this->_del_prefix, $this->_del_suffix);

	}//end _deleted()


	/**
	* Mark the supplied content as having changed in the document
	*
	* @param string	$orig	Original content
	* @param string	$final	Final content
	*
	* @return string
	* @access private
	*/
	function _changed($orig, $final)
	{
		/* If we've already split on words, don't try to do so again - just
		 * display. */
		if ($this->_split_level == 'words') {
			$prefix = '';
			while ($orig[0] !== FALSE && $final[0] !== FALSE &&
				   substr($orig[0], 0, 1) == ' ' &&
				   substr($final[0], 0, 1) == ' ') {
				$prefix .= substr($orig[0], 0, 1);
				$orig[0] = substr($orig[0], 1);
				$final[0] = substr($final[0], 1);
			}
			return $prefix.$this->_deleted($orig).$this->_added($final);
		}

		$text1 = implode("\n", $orig);
		$text2 = implode("\n", $final);

		/* Non-printing newline marker. */
		$nl = "\0";

		$text1_words = $this->_splitOnWords($text1);
		$text2_words = $this->_splitOnWords($text2);

		/* We want to split on word boundaries, but we need to
		 * preserve whitespace as well. Therefore we split on words,
		 * but include all blocks of whitespace in the wordlist. */
		$diff = new Text_Diff($text1_words,
							   $text2_words);

		/* Get the diff in inline format. */
		$renderer = new Matrix_Diff_Highlighter(array_merge($this->getParams(),
															   Array('split_level' => 'words')));

		/* Run the diff and get the output. */
		return str_replace($nl, "\n", $renderer->render($diff))."\n";

	}//end _changed()


	/**
	* Return true if the input is an HTML closing tag
	*
	* @param string	$x	The tag to test
	*
	* @return boolean
	* @access private
	*/
	function _isCloseTag($x)
	{
		return preg_match('/<\/[^>]+>/i', $x);

	}//end _isCloseTag()


	/**
	* Return true if the input is an HTML opening tag
	*
	* @param string	$x	The tag to test
	*
	* @return boolean
	* @access private
	*/
	function _isOpenTag($x)
	{
		if (preg_match('/<(br|hr|img)( ([^>]*))?>/i', $x)) {
			return FALSE; // no empty tags thanks
		}
		
		if (preg_match('/<a[^>]+href=[^>]+>/i', $x)) {
			return true; // anchor tags are open tags
		}
		
		return preg_match('/<[^\/>]+>/i', $x);

	}//end _isOpenTag()


	/**
	* Return true if the input is the opening tag of a block-level element
	*
	* @param string	$x	The tag to test
	*
	* @return boolean
	* @access private
	*/
	function _isOpenBlockTag($x)
	{
		return preg_match('/<(address|blockcode|blockquote|div|h|h1|h2|h3|h4|h5|h6|hr|p|pre|section)>/i', $x);

	}//end _isOpenBlockTag()


	/**
	* Return true if the input is the closing tag of a block-level element
	*
	* @param string	$x	The tag to test
	*
	* @return boolean
	* @access private
	*/
	function _isCloseBlockTag($x)
	{
		return preg_match('/<\/(address|blockcode|blockquote|div|h|h1|h2|h3|h4|h5|h6|hr|p|pre|section)>/i', $x);

	}//end _isCloseBlockTag()


	/**
	* Split the suplied string into words
	*
	* This function treats any html tag as a single word
	*
	* @param string	$string	String to split up
	*
	* @return array
	* @access private
	*/
	function _splitOnWords($string)
	{
		// Ignore \0; otherwise the while loop will never finish.
		$string = str_replace("\0", '', $string);
		$string = str_replace('>', '> ', $string);
		$string = str_replace('<', ' <', $string);

		$bits = preg_split('/[\s]+/', $string);
		$i = 0;
		$total = count($bits); // need to copy this now coz it will change
		while ($i < $total) {
			if (isset($bits[$i])) {
				if ($bits[$i] === '') {
					unset($bits[$i]);
				} else if ($bits[$i]{0} == '<') {
					// looks like the start of a tag
					if (strpos($bits[$i], '>') === FALSE) {
						// the tag doesn't end here so look for its endings
						$j = $i+1;
						do {
							$bits[$i] .= ' '.$bits[$j];
							unset($bits[$j]);
							$j++;
						} while (strpos($bits[$i], '>') === FALSE);
					}
				}
			}
			$i++;
		}

		foreach ($bits as $i => $v) {
			$bits[$i] = $v.' ';
		}

		return array_values($bits);

	}//end _splitOnWords()


	/**
	* Render the differences represented by the supplied diff object
	*
	* @param object	$diff	The differences to render
	*
	* @return string
	* @access public
	*/
	function render($diff)
	{
		$out = '
			<style type="text/css">
				ins, ins *, ins * * {
					background: #aaffaa;
					text-decoration: none;
				}
				del, del *, del * * {
					background: #ffaaaa;
					text-decoration: line-through;
				}
			</style>
		';

		// Text/Diff.php is throwing php5 warnings
		// instead of overwritting the pear package, we are making the error mute
		$out .= @parent::render($diff);

		// make sure we have some whitespace around ins/del tags,
		// but none immediately inside
		$out = preg_replace('/<ins>(\s*)/', ' <ins>', $out);
		$out = preg_replace('/<del>(\s*)/', ' <del>', $out);
		$out = preg_replace('/(\s*)<\/ins>/', '</ins> ', $out);
		$out = preg_replace('/(\s*)<\/del>/', '</del> ', $out);

		// clean out empty ins/del elements
		$out = preg_replace('/<del>(\s*)<\/del>/', '', $out);
		$out = preg_replace('/<ins>(\s*)<\/ins>/', '', $out);

		return html_entity_decode($out, ENT_COMPAT, SQ_CONF_DEFAULT_CHARACTER_SET);

	}//end render()


	/**
	* Perform the whole difference-calculation and rendering process
	*
	* @param string	$first	The first version of the content
	* @param string	$second	The second version of the content
	*
	* @return string
	* @access public
	*/
	function process($first, $second)
	{
		if ($first == $second) return $second;

		// Make sure tags are separated from words
		$first = trim(preg_replace('/<([^>]+)>/', ' <$1> ', $first), ' ');
		$second = trim(preg_replace('/<([^>]+)>/', ' <$1> ', $second), ' ');

		// Collapse inline whitespace
		$first = preg_replace('/([ \t]+)/', ' ', $first);
		$second = preg_replace('/([ \t]+)/', ' ', $second);

		/* Create the Diff object. */
		// Text/Diff.php is throwing php5 warnings
		// instead of overwritting the pear package we are making the error mute
		@include_once 'Text/Diff.php';
		@$diff = new Text_Diff(explode("\n", $first), explode("\n", $second));

		return $this->render($diff);

	}//end process()


}//end class
?>
