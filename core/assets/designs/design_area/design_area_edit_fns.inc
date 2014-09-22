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
* $Id: design_area_edit_fns.inc,v 1.63 2012/08/30 01:09:05 ewang Exp $
*
*/


require_once SQ_INCLUDE_PATH.'/asset_edit/asset_edit_fns.inc';

/**
* Design_Area_Edit_Fns
*
* Purpose
*
*
* @author  Blair Robertson <blair@squiz.net>
* @version $Revision: 1.63 $
* @package MySource_Matrix_Packages
* @subpackage __core__
*/
class Design_Area_Edit_Fns extends Asset_Edit_Fns
{

	/**
	* @var string
	* The prefix for tags in parse files
	*/
	public $tag_name = 'MySource';


	/**
	* Contructor
	*
	*/
	function __construct()
	{
		$this->static_screens['details']['lock_type'] = 'parsing';
		parent::__construct();

	}//end constructor


	/**
	* Echo the Design Area's ID name
	*
	* @param Design_Area		$asset	the asset whose interface we are painting
	* @param Backend_Outputter	$o		the outputter class
	* @param string				$prefix	prefix for the form element
	*
	* @return void
	* @access public
	*/
	public function paintIdName(Design_Area $asset, Backend_Outputter $o, $prefix)
	{
		echo $asset->attr('id_name');
		return TRUE;

	}//end paintIdName()


	/**
	* Parse the string into an array recursively cut up into html and tag pieces
	*
	* @param string	$file_contents	the contents to parse
	*
	* @return array
	* @access public
	*/
	public function _parseString($file_contents)
	{
		// lets just make sure that they are all UNIX new lines
		$file_contents = str_replace(Array("\r\n", "\r"), "\n", $file_contents);

		// replace any image roots with a print operation for the image prefix
		$e = '|mysource_files\/([a-zA-Z0-9\._\-@]+)|i';
		$file_contents = preg_replace($e, '<'.$this->tag_name.'_PRINT id_name=\'__global__\' var=\'file_path\' filename=\'\\1\' />', $file_contents);

		// replace any css roots with a print operation for the css prefix
		$e = '|mysource_css\/([a-zA-Z0-9\._\-@]+)|i';
		$file_contents = preg_replace($e, '<'.$this->tag_name.'_PRINT id_name=\'__global__\' var=\'css_path\' filename=\'\\1\' />', $file_contents);

		// replace any './[folder/filename]' with '[folder/filename]'
		// except for './?a=' Matrix assetid links
		$e = '/(<link[^>]*href=")\\.\\/(?!\\?a=)/i';
		$file_contents = preg_replace($e, '\\1', $file_contents);

		// Lets just make sure they aren't trying to be tricky and do some processing we don't know about
		// Escape all the opening '<?' tags that are not xml header tag '<?xml'
		$file_contents = preg_replace('/\<\?(?!xml)/i', '&lt;?', $file_contents);
		$file_contents = preg_replace('|\<\?(?=xml)(.+?)(?=\?\>)\?\>|', '<'.'?php echo "<"."?"; ?'.'>\\1<?php echo "?".">"; ?'.'>', $file_contents);

		// Lets make all occurances of the open cnd close tags the
		// same case as the constant just to make it easier to parse
		$file_contents = preg_replace('/<'.$this->tag_name.'/i', '<'.$this->tag_name, $file_contents);
		$file_contents = preg_replace('/<\\/'.$this->tag_name.'/i', '</'.$this->tag_name, $file_contents);

		// create the super array from the tags
		$cur_pos    = 0;
		$nest_level = 0;
		$contents = $this->_parseStringContents($file_contents, $cur_pos, $nest_level);

		return $contents;

	}//end _parseString()


	/**
	* Take the raw contents passed in and create individual elements for each
	* section of the contents (HTML or this->tag_name tag)
	* if code is nested between open and terminate tags this function is called
	* recursivley
	*
	* @param string	&$file_contents	the raw contents to parse file
	* @param int	&$cur_pos		the current position in $file_contents that we are
	* @param int	&$nest_level	the current position in $file_contents that we are
	*
	* @return array
	* @access private
	*/
	protected function _parseStringContents(&$file_contents, &$cur_pos, &$nest_level)
	{
		$start_pos = $cur_pos;
		$parsed = Array();
		$tag    = Array(); // the currently processed tag

		$is_nested = ($nest_level > 0);

		while ($next_tag = $this->_tagToParse($file_contents, $cur_pos)) {
			// append the string before this current pos to the array
			$parsed[]  =  Array(
							'_type'		=> 'HTML',
							'contents'	=> substr($file_contents, $start_pos, $cur_pos - $start_pos),
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

				// if this tag terminates then blank array for it's contents
				if ($tag['self_terminating']) {
					$tag['contents'] = Array();

				// else this tag doesn't terminate the we need to allow for its contents
				} else {
					$this_nest_level = $nest_level;
					// because we are going to the next level increment
					$nest_level++;
					$tag['contents'] = $this->_parseStringContents($file_contents, $cur_pos, $nest_level);
					// if something went wrong, bail out
					if (is_null($tag['contents'])) return NULL;

					// if this instance of the functions nest level is the same as the global one
					// then the recursive call found e terminate tag, so remove it
					if ($this_nest_level == $nest_level) {
						$this->_parseTerminateTag($file_contents, $cur_pos);

					// else we're missing a terminate tag, so die screaming
					} else {
						$die_str = $this->tag_name;
						$die_str .= (empty($tag['operation'])) ? '' : '_'.$tag['operation'];

						foreach ($tag['attributes'] as $name => $value) {
							$die_str .= ' '.$name.'="'.$value.'"';
						}

						trigger_localised_error('CORE0213', sprintf(translate('There is a missing &lt;/%1$s&gt; tag for this tag:<br/>&lt;%2$s&gt;'), $this->tag_name, $die_str), E_USER_WARNING)
						return NULL;

					}// end if
				}// end if

				$parsed[]  = $tag;

			}//end else if next tag

			// end parsing the tag

			$start_pos = $cur_pos;

		}//end while

		// if this instance opf the funciton is not nested
		// then it must be the root level
		if (!$is_nested) {
			// so append the rest of the html code to the array
			$parsed[] = Array(
							'_type'		=> 'HTML',
							'contents'	=> substr ($file_contents, $cur_pos),
						);

		}//end if

		return $parsed;

	}//end _parseStringContents()


	/**
	* Figure out which type of tag (open or terminate) comes next in the text
	*
	* Returns the type and sets the current position to the start of the tag
	*
	* @param string	&$file_contents	the raw contents to parse file
	* @param int	&$cur_pos		the current position in $file_contents that we are
	*
	* @return string
	* @access private
	*/
	protected function _tagToParse(&$file_contents, &$cur_pos)
	{
		// get the next terminate and open tag locations
		$next_open_pos      = strpos($file_contents, '<'.$this->tag_name,  $cur_pos);
		$next_open_tag      = ($next_open_pos !== FALSE);
		$next_terminate_pos = strpos($file_contents, '</'.$this->tag_name, $cur_pos);

		// if there is a terminate tag
		if ($next_terminate_pos !== FALSE) {
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
	* Parse all the info from the start of the open tag until its end
	*
	* Also assigns any attributes that the tag may have contained
	*
	* @param string	&$file_contents	the raw contents to parse file
	* @param int	&$cur_pos		the current position in $file_contents that we are
	*
	* @return array
	* @access private
	*/
	protected function _parseOpenTag(&$file_contents, &$cur_pos)
	{
		$cur_pos += strlen('<'.$this->tag_name);

		$in_quote     = '';
		$escape_slash = FALSE;

		$currently_getting = '';  // name or value
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

					// if its whitespace, discard this name
					} else if ($this->_isWhitespace($file_contents{$cur_pos})) {
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
	* Keep counting the characters until we hit a close bracket
	*
	* @param string	&$file_contents	the raw contents to parse file
	* @param int	&$cur_pos		the current position in $file_contents that we are
	*
	* @return void
	* @access private
	*/
	protected function _parseTerminateTag(&$file_contents, &$cur_pos)
	{
		$cur_pos += strlen('</'.$this->tag_name);
		// go to the next close bracket
		$cur_pos = strpos($file_contents, '>', $cur_pos);
		// go to char after it
		$cur_pos++;

	}//end _parseTerminateTag()


	/**
	* Return TRUE if the whole string is whitespace of some kind
	*
	* @param string	$str	The string to check
	*
	* @return boolean
	* @access private
	*/
	protected function _isWhitespace($str)
	{
		return preg_match('/^[[:space:]]*$/', $str);

	}//end _isWhitespace()


	/**
	* Loop through the contents array and create the necessary objects
	*
	* @param Design_Area	$asset		the asset to which we belong
	* @param array			$contents	the contents array we generated with _parseString()
	*
	* @return boolean
	* @access private
	*/
	public function _processContents(Design_Area $asset, Array $contents)
	{
		if ($this->_setVariables($asset, $contents)) {
			$GLOBALS['SQ_PROCESSED_DESIGN_AREAS'][$asset->attr('id_name')] = TRUE;
			return $asset->setAttrValue('contents', $contents);
		} else {
			return FALSE;
		}

	}//end _processContents()


	/**
	* Takes the reference to the tag array passed into created by _parseStringContents()
	* and sets both the attributes and sets operation tags into values
	* the set tags are then removed from the array
	*
	* Also anything with 'image' contained in it will have the file root removed
	* from its value
	*
	* @param Design_Area	$asset		the asset to which we belong
	* @param array			&$contents	the contents array from _parseString
	*
	* @return boolean
	* @access private
	*/
	protected function _setVariables(Design_Area &$asset, Array &$contents)
	{
		// process the set operations of the element
		$remove_indexes = Array();
		// Holds all attributes that have been set in the parse file for this design area
		$set_attrs = Array();
		$var_references = Array();
		foreach ($contents as $index => $element) {

			if ($element['_type'] != 'TAG' || $element['operation'] != 'set') {
				continue;
			}

			// Is this a reference to another variable ?
			if (!empty($element['attributes']['id_name']) && !empty($element['attributes']['var'])) {

				// just set this so we remember what is happening when it comes to painting
				$var_references[$element['attributes']['name']] = Array(
																	'id_name'	=> $element['attributes']['id_name'],
																	'var'		=> $element['attributes']['var'],
																  );

			// Do we want a whole design areas value ?
			} else if (!empty($element['attributes']['id_name'])) {

				// just set this so we remember what is happening when it comes to painting
				$var_references[$element['attributes']['name']] = Array(
																	'id_name'	=> $element['attributes']['id_name'],
																	'var'		=> '',
																  );

			// we must be a normal value, so set it
			} else if (isset($element['attributes']['value'])) {
				// if this holds an image, then remove any image roots before saving
				if (stristr($element['attributes']['name'], 'image')) {
					$element['attributes']['value'] = $this->_removeFileRoot($element['attributes']['value']);
				}

				if (!$asset->setAttrValue($element['attributes']['name'], $element['attributes']['value'])) {
					trigger_localised_error('CORE0189', sprintf(translate('Unable to set the value of "%1$s" in "%2$s"'), $element['attributes']['name'], $asset->attr('id_name')), E_USER_WARNING)
					return FALSE;
				}
				$set_attrs[] = $element['attributes']['name'];

			// else something is amiss and we don't know what we can do
			} else {
				trigger_localised_error('CORE0188', sprintf(translate('Unable to set "%s". Either a value must be set or both the id_name and name attributes must be set'), $element['attributes']['name'], $asset->attr('id_name')), E_USER_WARNING)


			}// endif

			$remove_indexes[] = $index;
		}//end foreach

		if (!$asset->setAttrValue('var_references', $var_references)) {
			return FALSE;
		}
		// Now remove the elems
		// reverse the order so we don't throw the numbers out of whack by removing one from the start
		rsort($remove_indexes, SORT_NUMERIC);
		foreach ($remove_indexes as $i) {
			array_splice($contents, $i, 1);
		}

		// Now we need to ensure that any attributes that weren't set in the parse file
		// are returned to their default values
		// Design Area attributes that shouldn't be reset

		$not_setable_attrs = $asset->getProtectedAttrs();
		$current_asset_attrs = $GLOBALS['SQ_SYSTEM']->am->getAssetTypeAttributes($asset->type(), Array('name'));

		// Now compute the remaining attributes, by subtracting all that shouldn't be deleted
		// and all that are set in the parse file
		$set_to_default = array_diff($current_asset_attrs, $not_setable_attrs, $set_attrs);

		// Now iterate through each variable that is setable and doesn't exist in the parse file,
		// find and retrieve its attribute and set its current value to the default
		foreach ($set_to_default as $i => $attr_name) {
			$attr_to_set = $GLOBALS['SQ_SYSTEM']->am->getAttribute($asset->vars[$attr_name]['attrid']);
			$default_val = $attr_to_set->_default_value;
			if ($attr_to_set instanceof Asset_Attribute_Serialise) {
				$default_val = unserialize($default_val);
			}
			$asset->setAttrValue($attr_name, $default_val);
			$attr_to_set = NULL;
		}
		return TRUE;

	}//end _setVariables()


	/**
	* Remove any image roots from the front of the string
	*
	* Replace any './[folder/filename]' with '[folder/filename]'
	*
	* @param string	$str	the string to remove any image root from
	*
	* @return string
	* @access private
	*/
	protected function _removeFileRoot($str)
	{
		return preg_replace('/^(\\.\\/)?mysource_files\/(.*)/i', '\\2', $str);

	}//end _removeFileRoot()


	/**
	* Returns a default contents for the passed type
	*
	* Can be used when there is no contents supplied in parse file
	*
	* @param Design_Area	$asset			the asset to which we belong
	* @param string			$content_type	the name that forms the filename 'default_'.$content_type.'_contents.html' in the asset types directory
	*
	* @return array
	* @access private
	* @see _parseString()
	* @see _processContents()
	*/
	protected function _getDefaultContents(Design_Area $asset, $content_type)
	{
		$parent_types = $asset->getTypeAncestors(FALSE);
		array_unshift($parent_types, $asset->type());
		for ($i = 0, $total = count($parent_types); $i < $total; $i++) {

			$file = SQ_SYSTEM_ROOT.'/'.$GLOBALS['SQ_SYSTEM']->am->getTypeInfo($parent_types[$i], 'dir').'/default_'.$content_type.'_contents.html';
			if (file_exists($file)) {
				require_once SQ_FUDGE_PATH.'/general/file_system.inc';
				$contents = $this->_parseString(file_to_string($file));
				return $contents;
			}//end if

			// we don't need to go any higher than this class
			if ($parent_types[$i] == 'design_area') break;

		}//end for

		return Array();

	}//end _getDefaultContents()


}//end class
?>
