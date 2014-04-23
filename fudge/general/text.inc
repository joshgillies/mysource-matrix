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
* $Id: text.inc,v 1.86.2.3 2013/10/21 23:49:15 ewang Exp $
*
*/


/**
* General Text Functions
*
* Purpose
*     Generic functions that are useful, but don't fall into any other category
*
* @author  Greg Sherwood <greg@squiz.net>
* @version $Revision: 1.86.2.3 $
* @package Fudge
* @subpackage Text
*/


/**
* Replaces keywords on a string or array using the keywords supplied in the $replacements array
* $text can either be an array or string containing keywords to be replaced
*
* (replace_keywords() is a wrapper function for _replace_keywords_recursive() - if
* the arguments pass validation, it passes them off to the latter function to do
* the actual replacement)
*
* @param mixed	&$text			the text or array to replace in
* @param array	&$replacements	an array of replacements (keyword => replacement)
*
* @return mixed string|array
* @access public
*/
function replace_keywords(&$text, &$replacements)
{
	// type verification
	$is_replacements_array = is_array($replacements);
	if (is_string($text)) {
		if (!$is_replacements_array) return '';
	} else if (is_array($text)) {
		if (!$is_replacements_array) return Array();
	} else {
		return '';
	}

	return _replace_keywords_recursive($text, $replacements);

}//end replace_keywords()


/**
* Replaces any occurance of "%keyword%" with the replacements text
*
* @param mixed	&$text			the text or array to replace in
* @param array	&$replacements	an array of replacements (keyword => replacement)
*
* @return mixed string|array
* @access private
*/
function _replace_keywords_recursive(&$text, &$replacements)
{
	if (is_array($text)) {
		$r = Array();
		foreach ($text as $key => $value) {
			$key = _replace_keywords_recursive($key, $replacements);
			$r[$key] = _replace_keywords_recursive($value, $replacements);
		}
		$text = $r;
		return $text;
	}
	foreach ($replacements as $keyword => $replacement) {
		if ($keyword == '' || is_array($replacement)) continue;
		$text = str_replace("%$keyword%", $replacement, $text);
	}
	return $text;

}//end _replace_keywords_recursive()


/**
 * Parses a keyword which may contain modifier syntax.
 *
 * e.g. asset_contents~maxwords:10
 *
 * ..would return the plain keyword asset_contents, and modifiers
 * would contain one entry for maxwords.
 *
 * @return string
 * @access public
 */
function parse_keyword($keyword, &$modifiers = NULL)
{
	$modifiers = Array();
	$parts = explode('^', $keyword);
	$keyword = array_shift($parts);

	foreach ($parts as &$p) {
		$args = preg_split('/:{1}/', $p, -1);
		$modifier = array_shift($args);
		if (!empty($modifier)) {
			array_push($modifiers, Array('modifier' => $modifier, 'args' => $args));
		}
	}

	return $keyword;

}//end parse_keyword()


/**
 * Applies a set of modifiers against given text. $text is updated in place.
 *
 * @param string	$text				the text to modify
 * @param array		$modifiers			modifiers to apply to $text
 * @param array		$replace_keywords	info used for 'replace_keywords' modifier,
 * 										needed to determine replacement source for nested keywords
 * <pre>
 *					Array (
 *						'assetid'		=> the asset id the keyword replacement ($text) belongs to
 *										   supply only if the caller scope includes replacements from all the 'call_fns',
 *						'call_fns'		=> a list of asset keyword replacement functions to call for 'assetid'
 *										   if not specified will default to include 'getKeywordReplacement',
 *						'replacements'	=> supply if any keyword replacements available from the caller scope,
 *					)
 * </pre>
 *
 * @return void
 * @access public
 */
function apply_keyword_modifiers(&$text, $modifiers, $replace_keywords=Array(), $actual_keyword='')
{
	foreach ($modifiers as $m) {
		$text = apply_keyword_modifier($text, $m, $replace_keywords, $actual_keyword);
	}

	// If there are no modifiers to apply, for array data we will still need to apply "default" modifier
	if (is_array($text) && empty($modifiers)) {
		$text = apply_keyword_modifier($text, Array('modifier' => '','args' => Array()), $replace_keywords, $actual_keyword);
	}

}//end apply_keyword_modifiers()


/**
 * Returns a version of $text piped through a predefined modifier.
 *
 * Most modifiers should be self explanatory.
 *
 * @param string	$text				the text to modify
 * @param array		$modifier			the modifier type and its arguments
 * @param array		$replace_keywords	info used for 'replace_keywords' modifier,
 * 										needed to determine replacement source for nested keywords
 * <pre>
 *					Array (
 *						'assetid'		=> the asset id the keyword replacement ($text) belongs to
 *										   supply only if the caller scope includes replacements from all the 'call_fns',
 *						'call_fns'		=> a list of asset keyword replacement functions to call for 'assetid'
 *										   if not specified will default to include 'getKeywordReplacement',
 *						'replacements'	=> supply if any keyword replacements available from the caller scope,
 *					)
 * </pre>
 *
 * @return string
 * @access public
 */
function apply_keyword_modifier($text, $modifier, $replace_keywords=Array(), $actual_keyword='')
{
	if (is_scalar($text) && !empty($actual_keyword) && (trim($text, '%') == trim($actual_keyword, '%'))) {
		return $text;
	}

	// These modifiers are applicable to arrays only
	if (is_array($text)) {
		switch ($modifier['modifier']) {

		case "count":
			return count($text);

		case "index":
			$index = array_get_index($modifier['args'], 0, 0);
			return array_get_index($text, $index, '');

		case "as_xml":
			$root_element = empty($actual_keyword) ? 'as_xml' : $actual_keyword;
			return trim(array_to_xml(Array($root_element => $text)));

		case "as_csv":
			return array_to_csv($text);

		case "as_csv_noheader":
			return array_to_csv($text, FALSE);

		case "as_url_string":
			return http_build_query($text);

		// Array value without any modifiers will be returned as a JSON string by default
		case "as_json":
		default:
			return json_format($text);
		}

	}//end if is_array()

	switch ($modifier['modifier']) {

	case "replace_keywords":

		/* This modifier takes another modifier as its arguments.
		 * Will attempt to replace any keywords in this nested modifiers arguments.
		 * Info from $replace_keywords will be used to determine source for keyword replacements.
		 */
		$nested_modifier = array_get_index($modifier['args'], 0, NULL);
		$nested_modifier_args = Array();

		$kw_replacements = array_get_index($replace_keywords, 'replacements', Array());
		$assetid = array_get_index($replace_keywords, 'assetid', 0);
		$call_fns = array_get_index($replace_keywords, 'call_fns', Array('getKeywordReplacement'));
		// find the asset to replace keywords for
		if(empty($assetid)) {
		    $asset = NULL;
		    if ($GLOBALS['SQ_SYSTEM']->isGlobalDefineSet('CURRENT_ASSET')) {
			    $asset = $GLOBALS['SQ_SYSTEM']->getGlobalDefine('CURRENT_ASSET', NULL);
		    } else {
			    if (SQ_IN_LIMBO) {
				    if (isset($_REQUEST['limbo_assetid'])) {
					    $asset = $GLOBALS['SQ_SYSTEM']->am->getAsset((int)$_REQUEST['limbo_assetid']);
				    } else {
					    $asset = $GLOBALS['SQ_SYSTEM']->am->getAssetFromURL();
				    }
			    } else {
				    $asset = $GLOBALS['SQ_SYSTEM']->frontend_asset;
			    }
		    }
		} else {
		    $asset = $GLOBALS['SQ_SYSTEM']->am->getAsset($assetid);
		}
		
		for ($i = 1; $i < count($modifier['args']); $i++){
			$value = $modifier['args'][$i];
			preg_match_all('/\{([a-zA-Z_]{2,}[a-zA-Z_\-0-9\.:]+)\}/', $value, $matches, PREG_PATTERN_ORDER);
			$keywords = array_unique($matches[1]);

			$replacements = Array();
			foreach ($keywords as $keyword){
				if (isset($kw_replacements[$keyword])){
					$replacements[$keyword] = $kw_replacements[$keyword];
				} else if (strpos($keyword, 'globals_') === 0) {
					$global_keyword = '%'.$keyword.'%';
					replace_global_keywords($global_keyword);
					if ($global_keyword != '%'.$keyword.'%') {
						$replacements[$keyword] = $global_keyword;
					}
				} else if (!empty($assetid) || !is_null($asset)){
					if (!isset($asset)) $asset = $GLOBALS['SQ_SYSTEM']->am->getAsset($assetid);
					if ($asset){
						foreach ($call_fns as $fn){
							if (method_exists($asset, $fn)){
								$asset_replacement = $asset->$fn($keyword);
								if (!is_null($asset_replacement) && $asset_replacement != '%'.$keyword.'%'){
									$replacements[$keyword] = $asset_replacement;
									break;
								}
							}
						}

					}
				}
			}

			foreach ($replacements as $keyword => $replacement) {
				if ($keyword == '') continue;
				$value = str_replace("{".$keyword."}", $replacement, $value);
			}
			$nested_modifier_args[] = $value;
		}

		return apply_keyword_modifier($text, Array('modifier' => $nested_modifier, 'args' => $nested_modifier_args));

	case "urlencode":
		return urlencode($text);

	case "urldecode":
		return urldecode($text);

	case "rawurlencode":
		return rawurlencode($text);

	case "rawurldecode":
		return rawurldecode($text);
	    
	case "base64encode":
		return base64_encode($text);

	case "base64decode":
		return base64_decode($text);

	case "lowercase":
		return strtolower($text);

	case "uppercase":
		return strtoupper($text);

	case "nl2br":
		return nl2br($text);

	case "escapequotes":
		return addslashes($text);

	case "escapehtml":
		return htmlspecialchars($text, ENT_QUOTES, SQ_CONF_DEFAULT_CHARACTER_SET);

	case "striphtml":
		$allowable_tags = array_get_index($modifier['args'], 0, '');

		if (!empty($allowable_tags)) {
			return strip_tags($text, html_entity_decode($allowable_tags, ENT_NOQUOTES, SQ_CONF_DEFAULT_CHARACTER_SET));
		} else {
			return strip_tags($text);
		}

	case "maxchars":
		// One argument; the max characters as an integer.
		$max = array_get_index($modifier['args'], 0, -1);

		if (SQ_CONF_DEFAULT_CHARACTER_SET == 'utf-8' && extension_loaded('mbstring')) 
			return mb_substr($text, 0, $max, SQ_CONF_DEFAULT_CHARACTER_SET);
		else
			return substr($text, 0, $max);

	case "maxwords":
		// One argument; the max words as an integer.
		$max = array_get_index($modifier['args'], 0, NULL);

		if (is_numeric($max)) {
			$text  = str_replace('&nbsp;', ' ', trim($text));
			$words = preg_split("/[\s]+/", $text);

			if ($max >= count($words)) {
				return $text;
			} else {
				$words = array_splice($words, 0, $max);
				return implode(' ', $words);
			}
		} else {
			return $text;
		}

	case "wordcount":
		return str_word_count($text, 0, '0123456789');

	case "charcount":
		return strlen($text);

	case "capitalize":
		return ucfirst($text);

	case "titleize":
		return ucwords($text);

	case "trim":
		return trim($text);

	case "underscore":
		return preg_replace('/\s+/', '_', $text);

	case "xpath":
		// Args are delimeter, xpath
		//
		// Example (ordered list of RSS item titles, using tagif modifier to ol list):
		//
		//    %asset_contents~xpath:</li><li>://item/title~tagif:li~tagif:ol%
		//
		// The path lists all titles in the feed, delimited by </li><li>, and then conditionally wrapped in <li>,
		// which in turn is conditionally wrapped in <ol>.
		//
		if (empty($text)) return $text;

		$delim = array_get_index($modifier['args'], 0, '');
		$path  = array_get_index($modifier['args'], 1, NULL);

		// If there are more args, this means there were colons in the path. Fixup.
		for ($i = 2; $i < count($modifier['args']); $i++) {
			$path .= ':' . $modifier['args'][$i];
		}

		if ($path) {
			try {
				$xml = new SimpleXMLElement($text);

				// if namespaces are used, register them before using them in the xpath query
				$namespaces = explode('xmlns:', $path);
				$path = array_shift($namespaces);
				foreach ($namespaces as $namespace) {
					$ns = explode('=', $namespace);
					if (count($ns) == 2) {
						$xml->registerXPathNamespace($ns[0], trim($ns[1]));
					}
				}

				$res = $xml->xpath($path);
				if ($res) {
					return implode($delim, $xml->xpath($path));
				}
			} catch (Exception $e) {
				// Badly formed xml, most likely.
				trigger_error($e->getMessage());
			}

		}
		return '';

	case "xpathe":
		// Arg is a xpath query
		//
		// PHP state that XPath version 1.0 is supported,
		// see: http://au.php.net/manual/en/class.domxpath.php
		//
		// This should mean that all the functions documented by W3C are supported,
		// see: http://www.w3.org/TR/xpath/
		//
		// Example:
		//
		//    %asset_contents~xpathe:sum(//item/value)%
		//
		// The path takes the mathematical sum() of item/value tags in the XML.
		//
		// Example:
		//
		//    %asset_contents~xpathe:count(//h1)%
		//
		// Counts the number of heading ones in the supplied markup.
		//
		if (empty($text)) return $text;

		// There may have been a colon in the path, which would have split the argument
		// out across elements - if so, fix it up.
		$path = implode(':', $modifier['args']);

		$dom = new DOMDocument();
		$dom->loadXML($text);
		$xpath = new DOMXPath($dom);

		$result = $xpath->evaluate($path);

		// free up memory
		unset($xpath);
		unset($dom);

		return is_scalar($result) ? $result : '';

	case "empty":
		// One argument; text to display if the keyword result is empty.
		if (empty($text)) {
			// We only want one arg, which might have : in. So implode.
			$text = implode(':', $modifier['args']);
		}
		return $text;

	case "notempty":
		// One argument; text to display if the keyword result is not empty.
		if (!empty($text)) {
			// We only want one arg, which might have : in. So implode.
			$text = implode(':', $modifier['args']);
		}
		return $text;

	case "tag":
		// One argument; the tag name to wrap around the text.
		$tag = array_get_index($modifier['args'], 0, NULL);
		if ($tag) {
			if (empty($text)) {
				return "<$tag />";
			} else {
				return "<$tag>$text</$tag>";
			}
		}
		return $text;

	case "tagif":
		// Same as above, except do nothing if the text is empty.
		$tag = array_get_index($modifier['args'], 0, NULL);
		if ($tag) {
			if (empty($text)) {
				return "";
			} else {
				return "<$tag>$text</$tag>";
			}
		}
		return $text;

	case "cdata":
		// Escape CDATA closing markup.
		$text = str_replace(']]>', ']]]]><![CDATA[>', $text);
		return "<![CDATA[$text]]>";

	case "stripdecl":
		// Strip XML declaration.
		$text = preg_replace('/^<\?xml\s.*\?>/', '', $text);
		return $text;

	case "increment":
		// Increment
		return ++$text;

	case "decrement":
		// Decrement
		return --$text;

	case "negate":
		// Negate
		return -$text;

	case "abs":
		// Returns the absolute number.
		return abs($text);

	case "sign":
		// Returns the sign of a number, +, -, or 0. Overriden in args 2, 3, and 4.
		$plus  = array_get_index($modifier['args'], 0, '+');
		$minus = array_get_index($modifier['args'], 1, '-');
		$zero  = array_get_index($modifier['args'], 2, '0');

		if ($text > 0) {
			return $plus;
		} else if ($text < 0) {
			return $minus;
		} else {
			return $zero;
		}

	case "ceil":
		// Rounds up to integer.
		return ceil($text);

	case "floor":
		// Rounds down to integer.
		return floor($text);

	case "round":
		// Round to specified precision (0 by default).
		$precision = array_get_index($modifier['args'], 0, 0);
		return round($text, $precision);

	case "number_format":
		// Format a number with grouped thousands and round to decimal point (0 by default).
		$decimals = array_get_index($modifier['args'], 0, 0);
		return number_format($text, $decimals);

	// Arithmetic modifiers.
	case "add":
	case "subtract":
	case "modulo":
	case "divide":
	case "multiply":
	case "squareroot":
		return _run_arithmetic_keyword_modifier($modifier, $text);

	// Comparison operator modifiers. By default these return '1' if true and '' if false.
	// Return values can be overriden with the subsequent args, e.g:
	//
	//  %asset_contents~eq:Hello:MATCH:NOMATCH%
	case "eq":
	case "neq":
	case "gt":
	case "lt":
	case "gte":
	case "lte":
		return _run_comparison_keyword_modifier($modifier, $text);

	// Hash generation modifiers.
	case "md5":
		return md5($text);

	case "sha1":
		return sha1($text);

	case "date_format":
		// Format a date string and only prints the field required
		$format = array_get_index($modifier['args'], 0, '');
		// remove rubbish string from calendar date event
		$text = preg_replace('/--(:)?/', '', $text);
		return date($format, strtotime($text));

	// Comparison date modifiers. By default these return '1' if true and '' if false.
	// Return values can be overriden with the subsequent args, e.g:
	//
	//  %asset_contents~eq:Hello:MATCH:NOMATCH%
	case "eq_date":
	case "neq_date":
	case "lt_date":
	case "lte_date":
	case "gt_date":
	case "gte_date":
		return _run_date_comparison_keyword_modifier($modifier, $text);

	case "replace":
		$toReplace   = array_get_index($modifier['args'], 0, NULL);
		$replaceWith = array_get_index($modifier['args'], 1, NULL);

		if (!is_null($toReplace) && !is_null($replaceWith)) {
			$toReplace = str_replace(chr(0), '' , $toReplace);
			return preg_replace('/'.$toReplace.'/', $replaceWith, $text);
		}
		return $text;

	case "preg_replace":
		$assetid = array_get_index($modifier['args'], 0, '');
		if ($assetid != '' && assert_valid_assetid($assetid, '', TRUE, FALSE)) {
			if ($GLOBALS['SQ_SYSTEM']->am->getAssetInfo($assetid, 'regular_expression')) {
				$regex_asset = $GLOBALS['SQ_SYSTEM']->am->getAsset($assetid);
				$regex_data = $regex_asset->attr('regex_data');

				$replace_from = Array();
				$replace_to = Array();
				foreach($regex_data as $data) {
					if ($data['replace_keywords']) {
						replace_global_keywords($data['regex']);
					}
					if (preg_match($data['regex'], '') === FALSE) {
						continue;
					}
					replace_global_keywords($data['replacement']);

					// Get rid of the 'e' modifier if any
					$regex_delimiter = substr($data['regex'], 0, 1);
					$ending_delimiter_index = strrpos($data['regex'], $regex_delimiter);
					if ($ending_delimiter_index > 0) {
						$regex_modifier_str = substr($data['regex'], $ending_delimiter_index);
						$regex_modifier_str_fixed = str_replace('e', '', $regex_modifier_str);
						if ($regex_modifier_str != $regex_modifier_str_fixed) {
							 $data['regex'] = substr($data['regex'], 0, $ending_delimiter_index).$regex_modifier_str_fixed;
						}
					}

					$replace_from[] = $data['regex'];
					$replace_to[] = $data['replacement'];
				}
				
				$limit = $regex_asset->attr('limit');
				if (empty($limit)) {
					$limit = -1;
				}

				if (!empty($replace_from)) {
					 return preg_replace($replace_from, $replace_to, $text, $limit);
				}
			}
		}

		return $text;

	case "preg_match":
		$assetid = array_get_index($modifier['args'], 0, '');
		$success = array_get_index($modifier['args'], 1, NULL);
		$failure = array_get_index($modifier['args'], 2, NULL);
		if ($assetid != '' && assert_valid_assetid($assetid, '', TRUE, FALSE)) {
			if ($GLOBALS['SQ_SYSTEM']->am->getAssetInfo($assetid, 'regular_expression')) {
				$regex_asset = $GLOBALS['SQ_SYSTEM']->am->getAsset($assetid);
				$regex_data = $regex_asset->attr('regex_data');

				// The very first regex in the regex/replacement pair is used for this modifier
				if (!empty($regex_data[0]) && !empty($regex_data[0]['regex'])) {
					$matches = Array();
					if ($regex_data[0]['replace_keywords']) {
						replace_global_keywords($regex_data[0]['regex']);
					}
					if (@preg_match_all($regex_data[0]['regex'], $text, $matches)) {
						if (isset($matches[0]) && count($matches[0]) > 0) {
							return $success ? $success : count($matches[0]);
						}
					}
				}
			}
		}

		return $failure ? $failure : 0;

	case "preg_match_result":
		$assetid = array_get_index($modifier['args'], 0, '');
		if ($assetid != '' && assert_valid_assetid($assetid, '', TRUE, FALSE)) {
			if ($GLOBALS['SQ_SYSTEM']->am->getAssetInfo($assetid, 'regular_expression')) {
				$regex_asset = $GLOBALS['SQ_SYSTEM']->am->getAsset($assetid);
				$regex_data = $regex_asset->attr('regex_data');

				// The very first regex in the regex/replacement pair is used for this modifier
				if (!empty($regex_data[0]) && !empty($regex_data[0]['regex'])) {
					$matches = Array();
					if ($regex_data[0]['replace_keywords']) {
						replace_global_keywords($regex_data[0]['regex']);
					}
					if (@preg_match_all($regex_data[0]['regex'], $text, $matches)) {
						if (isset($matches[0])) {
							return $matches;
						}
					}
				}
			}
		}

		return Array();

	case "contains":
		$toMatch = array_get_index($modifier['args'], 0, NULL);
		$success = array_get_index($modifier['args'], 1, NULL);
		$failure = array_get_index($modifier['args'], 2, NULL);

		if (!is_null($toMatch) && strpos($toMatch, '/') !== FALSE) $toMatch = str_replace('/', '\/', $toMatch);

		if (!is_null($toMatch) && preg_match_all('/'.$toMatch.'/', $text, $matches)) {
			if (isset($matches[0]) && count($matches[0]) > 0) {
				return $success ? $success : count($matches[0]);
			}
		}
		return $failure ? $failure : 0;

	 case "as_asset":
		$asset_keyword = array_get_index($modifier['args'], 0, '');
		if ($asset_keyword != '' && assert_valid_assetid($text, '', TRUE, FALSE)) {
			if ($GLOBALS['SQ_SYSTEM']->am->assetExists($text)) {
				$asset = $GLOBALS['SQ_SYSTEM']->am->getAsset($text);
				if ($asset->readAccess()) {
					if (strpos($asset_keyword, 'asset_contents') === 0) {
						if ($asset_keyword == 'asset_contents') {
							// get the front end of the asset
							$this_paint_layout = $asset->getCurrentPaintLayoutName();
							$this_url = preg_replace('/^.+:\/\//', '', $asset->getURL());
							$layout_id = $GLOBALS['SQ_SYSTEM']->am->getValueFromURL($this_url, $this_paint_layout);
							ob_start();
								if (!$layout_id) {
									$asset->printBodyWithPaintLayout();
								} else {
									$asset->printBodyWithPaintLayout($layout_id);
								}
								$content = ob_get_contents();
							ob_end_clean();

						} else if ($asset_keyword == 'asset_contents_raw') {
							ob_start();
								$asset->printBody();
								$content = ob_get_contents();
							ob_end_clean();

						} else if (strpos($asset_keyword, 'asset_contents_paint_layout_id') === 0) {
							$layout_id = str_replace('asset_contents_paint_layout_id_', '', $asset_keyword);
							ob_start();
								if (!$layout_id) {
									$asset->printBodyWithPaintLayout();
								} else {
									$asset->printBodyWithPaintLayout($layout_id);
								}
								$content = ob_get_contents();
							ob_end_clean();

						} else if (0 === strpos ($asset_keyword, 'asset_contents_paint_layout_name')) {
							$layout_name = str_replace('asset_contents_paint_layout_name_', '', $asset_keyword);
							$this_url = preg_replace('/^.*:\/\//', '', $asset->getURL());

							// if user has not mentioned if it is a user defined or default frontend
							// assume it is user defined
							if (strpos($layout_name, 'paint_layout::user::') === FALSE && strpos($layout_name, 'paint_layout::system::frontend') === FALSE) {
								$layout_name = 'paint_layout::user::'.$layout_name;
							}
							$layout_id = $GLOBALS['SQ_SYSTEM']->am->getValueFromURL($this_url, $layout_name);
							ob_start();
								if (!$layout_id) {
									$asset->printBodyWithPaintLayout();
								} else {
									$asset->printBodyWithPaintLayout($layout_id);
								}
								$content = ob_get_contents();
							ob_end_clean();
						} else {
							$content = '';
						}

						return $content;

					} else {
						return $asset->getKeywordReplacement($asset_keyword);
					}
				}
			}
		}

		return '';

	case "xslt":
		$new_text = '';
		try {
			//create an XSL object from the file asset id
			$xsl_assetid = array_get_index($modifier['args'], 0, NULL);
			if (empty($xsl_assetid)) return $new_text;
			$xsl_asset = $GLOBALS['SQ_SYSTEM']->am->getAsset($xsl_assetid);
			if (empty($xsl_asset)) return $new_text;

			$xsl = new DOMDocument();
			if ($xsl_asset instanceof File) {
				$xsl->load($xsl_asset->data_path.'/'.$xsl_asset->name);
			} else {
				trigger_error("XSLT keyword modifier: The asset #$xsl_assetid is not a File asset.");
				return $new_text;
			}

			//create the xslt object and import the style sheet
			$xslt = new XSLTProcessor();
			$xslt->importStylesheet($xsl);

			//load the XML from the keyword replacement text
			$xml = new DOMDocument();
			$xml->loadXML($text);
			$new_text = $xslt->transformToXml($xml);

		} catch (Exception $e) {
			trigger_error($e->getMessage());
		}

		return $new_text;

	case "json_encode":
		return json_format($text);

	case "css_safe":
		$prefix = array_get_index($modifier['args'], 0, NULL);
		$separator = array_get_index($modifier['args'], 1, NULL);

		// ensure separator character is valid
		if( $separator !== '-' && $separator !== '' ) $separator = '_';

		// ensure prefix character is valid
		if( !preg_match( '/^[a-z]+$/i' , $prefix ) ) $prefix = 'A';

		// Find all the bad characters
		$find = array(
		'/(?:^[^a-zA-Z0-9]+|[^_a-zA-Z0-9-]+$)/' // find bad characters from begining and end of string
		// (Note: in this version, strings cannot start with a
		// hyphen or underscores ('-', '_') as these are generally
		// reserved for browser specific selectors)
		,'/^(?=[0-9])/' // find numbers at the start of the string
		,'/[^_a-zA-Z0-9-]+/' // find all bad characters within string
		,'/[_-]{3}/' // cleaning up
		);

		// Replace all the bad characters
		$replace = array(
		'' // strip bad characters from begining and end of string
		,$prefix.$separator.'\1' // add underscore to strings starting with a number
		,$separator // replace multiple bad chars with a single underscore (or separator char)
		,$separator // cleaned up
		);

		return preg_replace( $find , $replace , $text );
	
	default:
		return $text;

	}

}//end _run_keyword_modifier()


/**
 * Returns a version of $text piped through a predefined arithmetic modifier.
 *
 * Mofifiers should be self explanatory.
 *
 * @return string
 * @access public
 */
function _run_arithmetic_keyword_modifier($modifier, $text)
{
	// Divide
	$n = str_replace(',', '', array_get_index($modifier['args'], 0, 0));

	$unformatted_text = str_replace(',', '', $text);

	switch ($modifier['modifier']) {
		case "add":
			return $unformatted_text + $n;
		case "subtract":
			return $unformatted_text - $n;
		case "multiply":
			return $unformatted_text * $n;
		case "divide":
			if ($n != 0) {
				return $unformatted_text / $n;
			} else {
				return '';
			}
		case "modulo":
			if ($n != 0) {
				return $unformatted_text % $n;
			} else {
				return '';
			}
		case "squareroot":
			if ($text > 0) {
				return sqrt($unformatted_text);
			} else if ($unformatted_text == '-1') {
				return 'i';
			} else {
				return '';
			}//end if
		default:
			return $text;
	}

}//end _run_arithmetic_keyword_modifier()


/**
 * Returns a version of $text piped through a predefined comparison operator modifier.
 *
 * Mofifiers should be self explanatory.
 *
 * @return string
 * @access public
 */
function _run_comparison_keyword_modifier($modifier, $text)
{
	// Greater than. Returns '1' or ''.
	$n = array_get_index($modifier['args'], 0, 0);
	$t = array_get_index($modifier['args'], 1, '1');
	$f = array_get_index($modifier['args'], 2, '');

	$ret = FALSE;
	switch ($modifier['modifier']) {
	case "eq":
		$ret = $text == $n;
		break;
	case "neq":
		$ret = $text != $n;
		break;
	case "gt":
		$ret = $text > $n;
		break;
	case "gte":
		$ret = $text >= $n;
		break;
	case "lt":
		$ret = $text < $n;
		break;
	case "lte":
		$ret = $text <= $n;
		break;
	}

	if ($ret) {
		return $t;
	} else {
		return $f;
	}

}//end _run_comparison_keyword_modifier()


/**
 * Returns a version of $text piped through a predefined comparison operator modifier.
 *
 * Mofifiers should be self explanatory.
 *
 * @return string
 * @access public
 */
function _run_date_comparison_keyword_modifier($modifier, $text)
{
	// If "compare time" is not supplied, use the current timestamp
	$c = strtotime(array_get_index($modifier['args'], 0, ''));
	if (!$c) {
		$c = time();
	}
	// Greater than. Returns '1' or ''.
	$t = array_get_index($modifier['args'], 1, '1');
	$f = array_get_index($modifier['args'], 2, '');

	// Invalid date to compare to
	$date = strtotime($text);
	if (!$date) {
		return $f;
	}

	$ret = FALSE;
	switch ($modifier['modifier']) {
		case "eq_date":
			$ret = $date == $c;
		break;
		case "neq_date":
			$ret = $date != $c;
		break;
		case "lt_date":
			$ret = $date < $c;
		break;
		case "lte_date":
			$ret = $date <= $c;
		break;
		case "gt_date":
			$ret = $date > $c;
		break;
		case "gte_date":
			$ret = $date >= $c;
		break;
	}
	
	if ($ret) {
		return $t;
	} else {
		return $f;
	}

}//end  _run_date_comparison_keyword_modifier()


/**
* An alias for extract keywords
*
* This function name is confusing because id does not deal with replacements, only keywords
*
* @param string	$text		the text or array to search
* @param string	$additional	additional valid keyword charaters for this match
* @param bool	$strict		extract strict format keywords	
*
* @return array
* @access public
* @see extract_keywords
*/
function retrieve_keywords_replacements($text, $additional='', $strict=FALSE)
{
	return extract_keywords($text, $additional, $strict);

}//end retrieve_keywords_replacements()


/**
* Extract keywords from a given text container
*
* @param string	$text		the text or array to search
* @param string	$additional	additional valid keyword charaters for this match
* @param bool	$strict		extract strict format keywords	
*
* @return array
* @access public
*/
function extract_keywords($text, $additional='', $strict=FALSE)
{
	$matches = Array();
	$additional = preg_replace('|(.)|', '\\\\\1', $additional);

	if($strict) {
	    // Keywords should have '_', do not confuse with url encoding characters such as %AAtesturl%BB
	    preg_match_all ('/%([a-zA-Z]{3,}_[a-zA-Z_\-0-9\.:'.$additional.']+(\^{1}[^%]+)*)%/', $text, $matches, PREG_PATTERN_ORDER);
	}
	else {
	    // Match any keyword including the filter portion. Loose format is normally used in asset custom level, such as %score%
	    preg_match_all ('/%([a-zA-Z_]{2,}[a-zA-Z_\-0-9\.:'.$additional.']+(\^{1}[^%]+)*)%/', $text, $matches, PREG_PATTERN_ORDER);
	}
	return array_unique($matches[1]);

}//end extract_keywords()


/**
* Extracts word counts for a given string.
*
* Takes a text string, splits it into words (according to $split_regexp), and extracts words
* from each split element (according to $word_match_regexp).
* Words must be larger than or equal to $min_length to be output.
* It return the array, Keys: words; values: the word counts
*
* @param string	$text				text
* @param string	$split_regexp		Regular expression delineating words
* @param string	$word_match_regexp	Regular expression for matching valid words
* @param int	$min_length			The minimum length for words matched
*
* @return array
* @access public
*/
function get_word_counts($text, $split_regexp="/\s+/", $word_match_regexp="/\S+/", $min_length=1)
{
	// type verification
	if (!is_string($text) || !is_string($split_regexp) || !is_string($word_match_regexp) || !is_int($min_length)) {
		return Array();
	} else if (!is_int($min_length) || ($min_length < 0)) {
		return Array();
	}

	// Remove silent characters
	$text = remove_silent_chars($text);

	$words = preg_split($split_regexp, $text, -1, PREG_SPLIT_NO_EMPTY);

	// Get rid of the leading and trailing white space
	// Dont have good enough reason to save them
	foreach ($words as $key => $word) {
		$words[$key] = trim($word);
	}

	$word_counts = array_count_values($words);
	foreach ($word_counts as $word => $count) {
		if (strlen($word) < $min_length || !preg_match($word_match_regexp, $word, $matches)) {
			unset($word_counts[$word]);
		}
	}

	return $word_counts;

}//end get_word_counts()


/**
* Remove a set of special characters from the given text.
*
* @param string		$text			text
* @param boolean	$not_indexing	default is FALSE, when TRUE this function does not remove a number of certain characters.
* @param boolean	$oracle_keyword when TRUE, apostrophes and email address pattern are not altered in given string. This is relevent when
									search plugin for Oracle db calls this function to filter the silent chars in search keyword

*
* @return string
* @access public
*/
function remove_silent_chars($text, $not_indexing=FALSE, $oracle_keyword=FALSE)
{
	if (!$oracle_keyword) {
		// Replace period (.) and comma (,) in number, e.g. 1,234.5 => 12345
		$number_pattern = '/(\d{1})[,.](\d{1})/'; // use that pattern instead of /(\d+)[,.](\d+)/ because this pattern can not replace both period and comma in one number. For example, 1,234.5 => 1234.5
		$text = preg_replace($number_pattern, '$1$2', $text);

		// Remove at (@) and period (.) in email addresses, e.g. everyone@squiz.net => everyonesquiznet
		// The following pattern matches most email addresses, but it will need to be review to match all email addresses
		$email_pattern = '/([^@\s]+)@(([-a-z0-9]+\.)+[a-z]{2,})/';
		$text = preg_replace_callback($email_pattern, create_function('$m',
			'return str_replace(\'.\',\'\',$m[1]).str_replace(\'.\',\'\',$m[2]);'
		),
		$text);

		// Remove apostrophe ('), e.g. you've => youve
		$text = str_replace('\'', '', $text);
	}

	$silent_chars = Array('.', ':', ';', ',', '!', '?', '`', '!', '@', '#', '$', '^', '*', '-', '=', '+', '{', '}', '[', ']', '\\', '<', '>', '/');
	if ($not_indexing == FALSE) {
		$oracle_search_operators = Array('(',')','"','~','&','|','%');
		$silent_chars = array_merge($silent_chars, $oracle_search_operators);
		//Remove the underscore ('_') that is usually used in metadata select field, e.g. select_1 => select1
		$text = str_replace('_', '', $text); // just move underscore ('_') from the above line to this line so that it won't be removed when search in oracle
	}

	// Replace the rest of silent characters with space (' '). This will create more indexed words but it is safer, e.g. Hello,world => Hello world
	$text = str_replace($silent_chars, ' ', $text);

	return $text;

}//end remove_silent_chars()


/**
* Escape double quotes by reference
*
* @param string	&$subject	subject
*
* @return void
* @access public
*/
function escape_double_quotes(&$subject)
{
	$subject = str_replace('"', '\"', $subject);

}//end escape_double_quotes()


/**
* Returns the json encoded string
*
* @param string|array	$data
*
* @return string
* @access public
*/
function json_format($data)
{
	if (!function_exists('json_encode'))  {
		require_once 'Services/JSON.php';
		$json = new Services_JSON();
		$output = $json->encodeUnsafe($data);
		if (Services_JSON::isError($output)) {
			trigger_error($output, E_USER_NOTICE);
		}
	} else {
		$output = json_encode($data);
		if (function_exists('json_last_error') && (json_last_error() != JSON_ERROR_NONE)) {
			switch (json_last_error()) {
				case JSON_ERROR_DEPTH:
					$error = 'Maximum stack depth exceeded';
					break;
				case JSON_ERROR_STATE_MISMATCH:
					$error = 'Underflow or the modes mismatch';
					break;
				case JSON_ERROR_CTRL_CHAR:
					$error = 'Unexpected control character found';
					break;
				case JSON_ERROR_SYNTAX:
					$error = 'Syntax error, malformed JSON';
					break;
				case JSON_ERROR_UTF8:
					$error = 'Malformed UTF-8 characters, possibly incorrectly encoded';
					break;
				default:
					$error = 'Unknown error';
					break;
			}
			trigger_error($error, E_USER_NOTICE);
		}
	}

	// our encodeing has added extra double quotes at the start
	// and end of our string. Remove them before returning text
	return preg_replace('|^"?(.*?)"?$|ms', '$1', $output);

}//end json_format()


/**
* Converts an array structure of any depth into an xml string
*
* @parm array|string $str
*
* @return string
* @access public
*/
function array_to_xml($str)
{
	if (!is_array($str)) {
		return '<![CDATA['.$str.']]>';
	}

	$xml = '';
	foreach($str as $key => $value) {
		if (is_int($key)) {
			$xml .= array_to_xml($value);
		} else {
		// apply xml element naming rules
			$key = preg_replace('/\s+/', '', $key);
			if (empty($key)) continue;
			$xml .= "\n\t".'<'.$key.'>'.array_to_xml($value).'</'.preg_replace('|\s.*$|', '', $key).'>';
        }
	}//end foreach

	return $xml;

}//end array_to_xml()


/**
* Converts the array data to a CSV string
*
* Array data above the 2 level deep or more will be return simply as string "*array*"
* Example 1:
* 	Array(
*		'name' => Array('max', 'min'),
*		'address' => Array('au', Array('city' => 'syd', country => 'au'),)
*	)
* will be returned as:
* 	name,address
* 	max,au
* 	min,*array*
*
* Example 2:
*	Array(
*		'name' => 'max',
*		'address' => Array('city' => 'syd', country => 'au')
*	)
* will be returned as
*	name,address
*	max,*array*
*
* @param array $array
* @param boolean $include_header
*
* @return string
* @access public
*/
function array_to_csv($array, $include_header=TRUE)
{
	if (!is_array($array)) {
		return '';
	}
	$data = Array();

	// First line is the CSV header
	$data = Array();
	if ($include_header) {
		$data[] = '"'.implode('","', array_keys($array)).'",';
	}
	$values = array_values($array);

	// Maximum number of items in a single data row
	$max_column_count = 0;

	// If atleast one value is scalar, then whole array values will be handled as scalar
	$scalar_values = FALSE;
	foreach($values as $index => $value) {
		if (is_scalar($value)) {
			$scalar_values = TRUE;
		}
		if (count($value) > $max_column_count) {
			$max_column_count = count($value);
		}
	}
	if (!$max_column_count) {
		return '';
	}

	foreach($values as $index => $value) {
		$count = 1;
		if (!$scalar_values) {
			for($i = 0; $i < $max_column_count; $i++) {
				if (!isset($data[$count])) {
					$data[$count] = '';
				}
				if (isset($value[$i])) {
					$data[$count] .= '"'.(is_scalar($value[$i]) ? str_replace('"', '""', $value[$i]) : '*array*').'",';
				} else {
					$data[$count] .= ',';
				}
				$count++;
			}//end for
		} else {
			if (!isset($data[$count])) {
				$data[$count] = '';
			}
			$data[$count] .= '"'.(is_scalar($value) ? str_replace('"', '""', $value) : '*array*').'",';
		}
	}//end foreach

	// Strip out a single appending ','
	foreach($data as $key => $val) {
		$data[$key] = substr($val, 0, -1);
	}

	return implode("\r\n", $data);

}//end array_to_csv()


/**
* Escape dangerous content (eg. <?php, <?, <script language="php">) by reference
* Intended to be used on cache files that are eval()'d or include()'d
*
* @param string	&$subject	subject
*
* @return string
* @access public
*/
function escape_php(&$subject)
{
	// not necessary to escape the php end tag when outside of a PHP block
	$subject = str_replace('<?', "<?php echo '<'.'?'; ?>", $subject);
	$subject = str_replace('<%', "<?php echo '<'.'%'; ?>", $subject);
	$subject = preg_replace(
				'/
					<script[\s]*
					language[\s]*=[\s]*
					["\']?php["\']?
					[^>]*>
				/xi',
				"<?php echo '<'.'script language=\"php\">'; ?>",
				$subject
			   );
	return $subject;

}//end escape_php()


/**
* Removes spaces and capitalises first letters of words
*
* Given a string 'my best_string' will return 'MyBestString'
*
* @param string	$string			string to modify
* @param string	$word_separator	character representing a word split (in addition to space)
*
* @return string
* @access public
*/
function ucwords_no_space($string, $word_separator='_')
{
	$tmp_word = strtr($string, $word_separator, ' ');
	$tmp_word = preg_replace('/\s+/', '', ucwords($tmp_word));

	return $tmp_word;

}//end ucwords_no_space()


/**
* Makes a human-readable list of words
*
* Given an array of strings like A,B,C create a human readable list like A, B and C
* where and is an ending delimeter that can be supplied
*
* @param array	$string_array	array of words to format
* @param string	$end_delimiter	delimiter used to join last word to the list
*
* @return string
* @access public
*/
function make_readable_list($string_array, $end_delimiter='and')
{
	if (empty($string_array) || !is_array($string_array)) {
		return '';
	}

	$last = array_pop($string_array);

	if (empty($string_array)) return $last;

	$retval = implode(', ', $string_array);
	$retval .= ' '.$end_delimiter.' '.$last;

	return $retval;

}//end make_readable_list()


/**
* Escapes special regular expression chars
*
* @param string	$text
*
* @return void
* @access public
*/
function escape_regex_chars($text)
{
	return preg_replace('|([\|\{\}\(\)\^\$\.\*\?\+\-\[\]\\\])|', '\\\$1', $text);

}//end escape_regex_chars()

?>
