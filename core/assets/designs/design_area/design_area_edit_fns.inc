<?php

require_once SQ_INCLUDE_PATH.'/asset_edit/asset_edit_fns.inc';

/**
* Design_Area_Edit_Fns
*
* Purpose
*
*
* @author  Blair Robertson <blair@squiz.net>
* @version $Version$ - 1.0
* @package Resolve::packages::__core__
*/
class Design_Area_Edit_Fns extends Asset_Edit_Fns
{

	/**
	* Echo's the Design Areas id name
	*
	* @param object Asset				$asset	the asset whose interface we are painting
	* @param object	Backend_Outputter	$o		the outputter class
	* @param string						$prefix	prefix for the form element
	*
	* @access public
	*/
	function paintIdName(&$asset, &$o, $prefix)
	{
		echo $asset->attr('id_name');
	}// end paintIdName()

	/**
	* Parses the string into an array recursively cut up into html and tag pieces
	*
	* @param string	$file_contents		the contents to parse
	*
	* @return Array
	* @access private
	*/
	function _parseString($file_contents)
	{

		// lets just make sure that they are all UNIX new lines
		$file_contents = str_replace(Array("\r\n", "\r"), "\n", $file_contents);

		 /////////////////////////////////////////////////////////////////////
		// Before we send this contents off to be parsed lets do some cleanup
		// and adjustment of it so that it works.
		// Using the supplied image root go through all the
		// images, scripts and stylesheets and replace their image root with a
		// tag to print the FILE_PREFIX

		//// DEAL WITH SCRIPTS AND IMAGES ////

		// let's make sure that all the src are in the form src="[what ever]"
		$e = '/((src)|(background))=([^"\'>[:space:]]+)/i';
		$file_contents = preg_replace($e, '\\1="\\4"', $file_contents);

		$e = '/((src)|(background))=\'([^\']*)\'/i';
		$file_contents = preg_replace($e, '\\1="\\4"', $file_contents);

		// OK, so now every src tag has is contained in double quotes

		// replace any './[folder/filename]' with '[folder/filename]'
		$e = '/(((src)|(background))=")\\.\\//i';
		$file_contents = preg_replace($e, '\\1', $file_contents);

		// replace any image roots with a print operation for the image prefix
		$e = '/(((src)|(background))=")files\\/([^"]*)(")/i';
		$file_contents = preg_replace($e, '\\1<'.SQ_SYSTEM_SHORT_NAME.'_PRINT id_name=\'__global__\' var=\'file_path\' filename=\'\\5\' />\\6', $file_contents);

		//// DEAL WITH STYLESHEETS ////

		// let's make sure that all the src are in the form <link ... href="[what ever]" ...>
		$e = '/(<link[^>]*href)=([^"\'>[:space:]]+)([^>]*)/i';
		$file_contents = preg_replace($e, '\\1="\\2"\\3', $file_contents);

		$e = '/(<link[^>]*href)=\'([^\']*)\'([^>]*)/i';
		$file_contents = preg_replace($e, '\\1="\\2"\\3', $file_contents);

		// OK, so now every link tag has is contained in double quotes

		// replace any './[folder/filename]' with '[folder/filename]'
		$e = '/(<link[^>]*href=")\\.\\//i';
		$file_contents = preg_replace($e, '\\1', $file_contents);

		// replace any image roots with a print operation for the image prefix
		$e = '/(<link[^>]*href=")files\\/([^"]*)(")/i';
		$file_contents = preg_replace($e, '\\1<'.SQ_SYSTEM_SHORT_NAME.'_PRINT id_name=\'__global__\' var=\'file_path\' filename=\'\\2\' />\\3', $file_contents);



		// lets just make sure they aren't trying to be tricky and do some processing we don't know about
		// if there are issues with this, then maybe some regular expressions could be used, but for now
		// LEAVE THIS ALONE, DO NOT TOUCH - BCR
		$file_contents = str_replace('<'.'?', '&lt;?', $file_contents);
		$file_contents = str_replace('?'.'>', '?&gt;', $file_contents);

		// Lets make all occurances of the open cnd close tags the
		// same case as the constant just to make it easier to parse
		$file_contents = preg_replace('/<'.SQ_SYSTEM_SHORT_NAME.'/i', '<'.SQ_SYSTEM_SHORT_NAME, $file_contents);
		$file_contents = preg_replace('/<\\/'.SQ_SYSTEM_SHORT_NAME.'/i', '</'.SQ_SYSTEM_SHORT_NAME, $file_contents);

		// create the super array from the tags
		$cur_pos    = 0;
		$nest_level = 0;
		$contents = $this->_parseStringContents($file_contents, $cur_pos, $nest_level);
		if (!is_null($contents)) {
			$this->_parserCleanupContents($contents);
		}

		return $contents;

	}//end _parseString()

	/**
	* Take the raw contents passed in and create individual elements for each
	* section of the contents (HTML or SQ_SYSTEM_SHORT_NAME tag)
	* if code is nested between open and terminate tags this function is called
	* recursivley
	*
	* @param string	$file_contents	the raw contents to parse file
	* @param int	$cur_pos		the current position in $file_contents that we are
	* @param int	$cur_pos		the current position in $file_contents that we are
	*
	* @returns Array()
	* @access private
	*/
	function _parseStringContents(&$file_contents, &$cur_pos, &$nest_level)
	{
		$start_pos = $cur_pos;
		$parsed = Array();
		$tag    = Array();		// the currently processed tag

		$is_nested = ($nest_level > 0);

		while($next_tag = $this->_tagToParse($file_contents, $cur_pos)) {
			// append the string before this current pos to the array
			$parsed[]  =  array('_type' => 'HTML',
								'contents' => substr($file_contents, $start_pos, $cur_pos - $start_pos)
								);

			if ($next_tag == 'terminate') {
				// if this instance is nested then we have found our parents end tag
				if ($is_nested) {
					// hey we found a terminate tag so decrement the nest counter
					$nest_level--;
					return $parsed;
				// else are in the root level and we have found a stray ternminate tag
				// so remove it
				} else {
					$this->_parseTerminateTag($file_contents, $cur_pos);
				}

			// its an open tag
			} else {

				$tag = $this->_parseOpenTag($file_contents, $cur_pos);

				// if this tag doesn't terminate the we need to allow for its contents
				if (!$tag['self_terminating']) {
					$this_nest_level = $nest_level;
					// because we are going to the next level increment
					$nest_level++;
					$tag['contents'] = $this->_parseStringContents($file_contents, $cur_pos, $nest_level);
					// if something went wrong, bail out
					if (is_null($tag['contents'])) return null;

					// if this instance of the functions nest level is the same as the global one
					// then the recursive call found e terminate tag, so remove it
					if ($this_nest_level == $nest_level) {
						$this->_parseTerminateTag($file_contents, $cur_pos);

					// else we're missing a terminate tag, so die screaming
					} else {

						$die_str  = 'There is a missing &lt;/'.SQ_SYSTEM_SHORT_NAME.'&gt; tag for this tag :';
						$die_str .= "<br/>&lt;".SQ_SYSTEM_SHORT_NAME;

						$die_str .= (empty($tag['operation'])) ? '' : '_'.$tag['operation'];

						foreach($tag['attributes'] as $name => $value) $die_str .= ' '.$name.'="'.$value.'"';

						$die_str .= "&gt;\n";

						trigger_error($die_str, E_USER_WARNING);
						return null;

					}// end if
				}// end if

				$parsed[]  = $tag;

			} //end if if next tag

			// end parsing the tag

			$start_pos = $cur_pos;

		}// end while

		// if this instance opf the funciton is not nested
		// then it must be the root level
		if (!$is_nested) {
			// so append the rest of the html code to the array
			$parsed[] = array('_type' => 'HTML',
									'contents' => substr ($file_contents, $cur_pos)
									);

		}//end if

		return $parsed;

	}//end _parseStringContents()

	/**
	* Figure out which type of tag (open or terminate) come next in the text
	* returns the type and sets the current pos to the start of the tag
	*
	* @param string	$file_contents	the raw contents to parse file
	* @param int	$cur_pos		the current position in $file_contents that we are
	*
	* @return string
	* @access private
	*/
	function _tagToParse(&$file_contents, &$cur_pos)
	{
		// get the next terminate and open tag locations
		$next_open_pos      = strpos($file_contents, '<'.SQ_SYSTEM_SHORT_NAME,  $cur_pos);
		$next_open_tag      = ($next_open_pos !== false);
		$next_terminate_pos = strpos($file_contents, '</'.SQ_SYSTEM_SHORT_NAME, $cur_pos);

		// if there is a terminate tag
		if ($next_terminate_pos !== false) {
			// if there is no open tag
			// or if there is a open tag and its located after the terminate tag
			// then parse the terminate tag
			if (!$next_open_tag || ($next_open_tag && ($next_terminate_pos < $next_open_pos))) {
				$cur_pos = $next_terminate_pos;
				return 'terminate';
			}

		}

		// if there is a open tag then use it
		if ($next_open_tag) {
			$cur_pos = $next_open_pos;
			return 'open';
		}

		// if they got this far there isn't anything left
		return '';

	}//end _tagToParse()

	/**
	* parse all the info from the start of the open tag until its end
	* assigning any attributes that it may have contained
	*
	* @param string	$file_contents	the raw contents to parse file
	* @param int	$cur_pos		the current position in $file_contents that we are
	*
	* @return Array
	* @access private
	*/
	function _parseOpenTag(&$file_contents, &$cur_pos)
	{
		$cur_pos += strlen('<'.SQ_SYSTEM_SHORT_NAME);

		$in_quote     = '';
		$escape_slash = false;

		$currently_getting = '';  // 'name' or 'value'
		$current_name  = '';      // attribute name
		$current_value = '';      // attribute value

		// used to reverse any HTML encoding that has taking place
		$trans = array_flip(get_html_translation_table(HTML_ENTITIES));

		$tag = Array('_type' => 'TAG', 'attributes' => Array());

		// if the next character is an underscore then we have a special operation tag
		if ($file_contents{$cur_pos} == '_') {
			$cur_pos++;
			$tag['operation'] = '';
			// while this isn't the end of the tag and it's not some white space,
			// append to operation
			while ($file_contents{$cur_pos} != '>' && !$this->_isWhitespace($file_contents{$cur_pos})) {
				$tag['operation'] .= $file_contents{$cur_pos};
				$cur_pos++;

			}//end while

			$tag['operation'] = strtolower($tag['operation']);


		}//end if specific operation

		while ($file_contents{$cur_pos} != '>' || $in_quote) {

			$escape_slash = (!$in_quote && $file_contents{$cur_pos} == '/');
			if (!$escape_slash) {
				// if we are not currenly getting any thing and we happen to fall upon
				// a non-whitespace charater then let's assume its an attribute name
				// and start getting it
				if (!$currently_getting && !$this->_isWhitespace($file_contents{$cur_pos})) {
					$currently_getting = 'name';
				}


				if ($currently_getting == 'name') {
					// if this is the equals sign then time to change to getting the value
					if ($file_contents{$cur_pos} == '=') {
						$currently_getting = 'value';
						$current_value = '';

					// if its whitespace then they fucked up so discard this name
					} elseif ($this->_isWhitespace($file_contents{$cur_pos})) {
						$currently_getting = '';
						$current_name = '';
					// must be a character, append to name
					} else {
						$current_name .= $file_contents{$cur_pos};
					}

				} else if ($currently_getting == 'value') {

					// if the current value is blank, we are not in a quote and this char is a quote
					// then we must be starting a new value
					$is_quote = ($file_contents{$cur_pos} == '\'' || $file_contents{$cur_pos} == '"');
					if (!$current_value && !$in_quote && $is_quote) {
						$in_quote = $file_contents{$cur_pos};

					// if we're not in a quote, then this value doesn't have quotes around it
					// so just keep going until we hit some whitespace
					} else if (!$in_quote && !$this->_isWhitespace($file_contents{$cur_pos})) {
						$current_value .= $file_contents{$cur_pos};

					// if we are in a quote and this char is not that quote, append
					} else if ($in_quote && $file_contents{$cur_pos} != $in_quote) {
						$current_value .= $file_contents{$cur_pos};

					// else we are either not in a quote and have hit some white space
					// or we are in a quote and have found a another quote of the same char
					// SO, we have finished this value, assign to tag and start over
					} else {
 						$tag['attributes'][strtolower($current_name)] = strtr($current_value, $trans);
						$currently_getting = '';
						$current_name = '';
						$current_value = '';
						$in_quote = '';

					}// end if

				}// end if

			}//end if not escape slash


			$cur_pos++;

		}//end while
		$cur_pos++;

		// if the last char before the close bracket is an foreslash
		// then this tag does not have a matching termination tag
		$tag['self_terminating'] = $escape_slash;

		return $tag;

	}//end _parseOpenTag()

	/**
	* keep counting the characters, until we hit a close bracket
	*
	* @param string	$file_contents	the raw contents to parse file
	* @param int	$cur_pos		the current position in $file_contents that we are
	*
	* @access private
	*/
	function _parseTerminateTag(&$file_contents, &$cur_pos)
	{


		$cur_pos += strlen('</'.SQ_SYSTEM_SHORT_NAME);
		// go to the next close bracket
		$cur_pos = strpos($file_contents, '>', $cur_pos);
		// go to char after it
		$cur_pos++;

	}//end _parseTerminateTag()

	/**
	* returns true if the whole string is whitespace of somekind
	*
	* @param string	$str
	*
	* @return boolean
	* @access private
	*/
	function _isWhitespace($str)
	{
		return preg_match('/^[[:space:]]*$/', $str);
	}

	/**
	* Recurses through the contents array and removes any HTML tags contain only whitespace
	* and are set between two tags
	*
	* @param array	$contents	the contents array we are currently checking
	*
	* @access private
	*/
	function _parserCleanupContents(&$contents)
	{
		for($i = 0; $i < count($contents); $i++) {
			if ($contents[$i]['_type'] == 'HTML' && $this->_isWhitespace($contents[$i]['contents'])) {
				array_splice($contents, $i, 1);
				$i--;
			} else if ($contents[$i]['_type'] == 'TAG' && !empty($contents[$i]['contents'])) {
				$this->_parserCleanupContents($contents[$i]['contents']);
			}// end if
		}// end for

	}// end _parserCleanupContents()

	/**
	* Loops through the contents array and creates the necessary objects
	*
	* @param object Asset	$asset		the asset to which we belong
	* @param array			$contents	the contents array we generated with _parseString()
	*
	* @return boolean
	* @access private
	*/
	function _processContents(&$asset, $contents)
	{
		if ($this->_setVariables($asset, $contents)) {
			return $asset->setAttrValue('contents', $contents);
		} else {
			return false;
		}

	}// end _processContents()


	/**
	* Takes the reference to the tag array passed into created by _parseStringContents()
	* and sets both the attributes and sets operation tags into values
	* the set tags are then removed from the array
	*
	* Also anything with 'image' contained in it will have the file root removed
	* from its value
	*
	* @param object Asset	$asset		the asset to which we belong
	* @param array			$contents	the contents array from _parseString
	*
	* @access private
	*/
	function _setVariables(&$asset, &$contents)
	{
		// process the set operations of the element
		$remove_indexes = Array();
		$var_references = Array();
		foreach($contents as $index => $element) {

			if ($element['_type'] != 'TAG' || $element['operation'] != 'set') continue;

			// Is this a reference to another variable ?
			if (!empty($element['attributes']['id_name']) && !empty($element['attributes']['var'])) {

				// just set this so we remember what is happening when it comes to painting
				$var_references[$element['attributes']['name']] = Array(
																		'id_name' => $element['attributes']['id_name'],
																		'var'     => $element['attributes']['var']
																		);

			// Do we want a whole design areas value ?
			} elseif (!empty($element['attributes']['id_name'])) {

				// just set this so we remember what is happening when it comes to painting
				$var_references[$element['attributes']['name']] = Array(
																		'id_name' => $element['attributes']['id_name'],
																		'var'     => ''
																		);

			// we must be a normal value, so set it
			} elseif (isset($element['attributes']['value'])) {
				// if this holds an image, then remove any image roots before saving
				if (stristr($element['attributes']['name'], 'image')) {
					$element['attributes']['value'] = $this->_removeFileRoot($element['attributes']['value']);
				}

				if (!$asset->setAttrValue($element['attributes']['name'], $element['attributes']['value'])) return false;

			// else something is amiss and we don't know what we can do, so barf in their direction
			} else {
				trigger_error('Unable to set "'.$element['attributes']['name'].'" in "'.$asset->attr('id_name').'". Either a value must be set or both the id_name and name attributes must be set', E_USER_WARNING);


			}// endif

			$remove_indexes[] = $index;

		}//end foreach

		if (!$asset->setAttrValue('var_references', $var_references)) return false;

		// Now remove the elems
		// reverse the order so we don't throw the numbers out of whack by removing one from the start
		rsort($remove_indexes, SORT_NUMERIC);
		foreach($remove_indexes as $i) {
			array_splice($contents, $i, 1);
		}

		return true;

	}//end _setVariables()

	/**
	* used to remove any image roots from the front of the string
	* replace any './[folder/filename]' with '[folder/filename]'
	*
	* @param string	$str	the string to remove any image root from
	*
	* @return string
	* @access private
	*/
	function _removeFileRoot($str) {
		return preg_replace('/^(\\.\\/)?files\/(.*)/i', '\\2', $str);
	}

	/**
	* Outputs the PHP/HTML code that will hopefully form part of the creating the design file
	*
	* @param Array			$contents	the parse contents to use in the output
	* @param object Asset	$asset		the asset to which we belong
	* @param object Design	$design		the design object that is painting us
	*
	* @return boolean
	* @access private
	*/
	function _createDesignFileOutput($contents, &$asset, &$design)
	{
		return $this->_outputContents($contents, $asset, $design);
	}// end _createDesignFileOutput()

	/**
	* Outputs the PHP/HTML code that will hopefully form part of the creating the design file
	*
	* @param Array			$contents	the parse contents to use in the output
	* @param object Asset	$asset		the asset to which we belong
	* @param object Design	$design		the design object that is painting us
	*
	* @return boolean
	* @access private
	*/
	function _outputContents($contents, &$asset, &$design)
	{
		// right, just loop over the contents and output each of the elements
		foreach($contents as $index => $element) {

			if ($element['_type'] == 'HTML') {
				echo $element['contents'];
			} else {
				if (!$this->_outputElement($element, $asset, $design)) return false;
			}// end if

		}// end foreach

		return true;

	}// end _outputContents()


	/**
	* Used by _outputContents() to deal with a print tag
	*
	* @param array			$element	the print element tag
	* @param object Asset	$asset		the asset to which we belong
	* @param object Design	$design		the design object that is painting us
	*
	* @return boolean
	* @access private
	* @see _outputContents()
	*/
	function _outputElement($element, &$asset, &$design)
	{
		switch($element['operation']) {
			case 'print' :
				// if there isn't a id_name we are using the current asset
				if (empty($element['attributes']['id_name']) && empty($element['attributes']['var'])) {
					trigger_error('Print tag found without id_name or var attributes, ignoring print operation', E_USER_WARNING);
					return false;
				}

				// if there isn't a id_name we are using the current asset
				if (empty($element['attributes']['id_name'])) {
					$id_obj = &$asset;

				// this is the global design var ?
				} elseif ($element['attributes']['id_name'] == '__global__') {
					$id_obj = &$design;

				// otherwise we have to find the design area
				} else {
					$id_obj = &$design->getDesignArea($element['attributes']['id_name']);
					if (is_null($id_obj)) {
						trigger_error('Design Area "'.$element['attributes']['id_name'].'" not found, ignoring print operation', E_USER_WARNING);
						return false;
					}
				}

				// if there is no var then print the asset
				if (empty($element['attributes']['var'])) {
					// we don't want recursion, so only print if the id_obj is not the asset
					if ($id_obj !== $asset) {
						$id_obj->printArea($design);
					}
				} else {
					$args = $element['attributes'];
					unset($args['id_name']);
					unset($args['var']);
					$id_obj->printVar($element['attributes']['var'], $args);
				}// endif

				return true;

			default :
				trigger_error('Tag Operation "'.$element['operation'].'" unknown', E_USER_WARNING);
				return false;

		}// end switch

	}// end _outputElement()

	/**
	* Creates a string that would be able to be parsed to recreate this design area in its current form
	*
	* @param object Asset	$asset	the asset whose interface we are painting
	*
	* @return boolean
	* @access public
	*/
	function createParseString(&$asset)
	{
		$text = '';
		$vars = $this->_getSetableVars($asset);
		foreach($vars as $name => $data) {
			$text .= '<'.SQ_SYSTEM_SHORT_NAME.'_SET name="'.$name.'"';

			// if this is this a var reference ?
			if (!empty($asset->vars['var_references']['value'][$name])) {
				$text .= ' id_name="'.htmlentities($asset->vars['var_references']['value'][$name]['id_name']).'" var="'.htmlentities($asset->vars['var_references']['value'][$name]['var']).'"';
			} else {
				$text .= ' value="'.htmlentities($data['value']).'"';
			}
			$text .= " />\n";
		}

		foreach($asset->vars['contents']['value'] as $data) {
			switch($data['_type']) {
				case 'HTML' : 
					$text .= $data['contents'];
					break;
				case 'TAG' : 
					$text .= '<'.SQ_SYSTEM_SHORT_NAME.'_'.strtoupper($data['operation']);
					foreach($data['attributes'] as $k => $v) $text .= ' '.$k.'="'.htmlentities($v).'"';
					$text .= ' />';
					break;
			}// end switch
		}// end foreach

		return $text;
	}// end createParseString()

	/**
	* Returns an array of vars that are setable, used to create the parseString
	*
	* @param object Asset	$asset	the asset whose interface we are painting
	*
	* @return Array
	* @access public
	* @see createParseString()
	*/
	function _getSetableVars(&$asset)
	{
		$vars = $asset->vars;
		unset($vars['id_name']);
		unset($vars['contents']);
		unset($vars['var_references']);
		unset($vars['specialised']);
		return $vars;

	}// end _getSetableVars()

	/**
	* Paint the interface for the specialising this design area
	* 
	* @param object Asset				$asset	the asset whose interface we are painting
	* @param object	Backend_Outputter	$o		the outputter class
	* @param string						$prefix	prefix for the form element
	*
	* @return boolean
	* @access public
	*/
	function paintSpecialisation(&$asset, &$o, $prefix)
	{
		$text = $this->createParseString($asset);
		hidden_field($prefix.'_specialisation_orig', $text);
		text_area($prefix.'_specialisation', $text, 75, 20, 0, 'wrap="off" style="font-family: courier new, courier, monospace; font-size: 9pt;"');
	}// end paintSpecialisation()

	/**
	* Process the specialisation, ie call _parseString() and set the specialised flag to true
	*
	* @param object Asset				$asset	the asset to which we belong
	* @param object	Backend_Outputter	$o		the outputter class
	* @param string						$prefix	prefix for the form element
	*
	* @return boolean
	* @access public
	*/
	function processSpecialisation(&$asset, &$o, $prefix)
	{
		// if nothing changed then do nothing
		if ($_POST[$prefix.'_specialisation_orig'] == $_POST[$prefix.'_specialisation']) return false;

		pre_echo($_POST[$prefix.'_specialisation']);
		$contents = $this->_parseString($_POST[$prefix.'_specialisation']);
		pre_echo($contents);
		if (!is_null($contents) && $this->_processContents($asset, $contents)) {
			pre_echo("PROCESS OK");
			$asset->_tmp['regenerate_design'] = true;
			return $asset->setAttrValue('specialised', 1);
		}

		return false;

	}// end processSpecialisation()

}//end class
?>