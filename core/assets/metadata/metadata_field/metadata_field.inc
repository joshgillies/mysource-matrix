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
* $Id: metadata_field.inc,v 1.9 2004/12/21 17:01:56 brobertson Exp $
*
*/


require_once SQ_INCLUDE_PATH.'/asset.inc';

/**
* Metadata_Field
*
* Purpose
*
*
* @author  Greg Sherwood <greg@squiz.net>
* @version $Revision: 1.9 $
* @package MySource_Matrix_Packages
* @subpackage __core__
*/
class Metadata_Field extends Asset
{

	/**
	* Constructor
	*
	* @param int	$assetid	the asset id to be loaded
	*
	*/
	function Metadata_Field($assetid=0)
	{
		$this->_ser_attrs = true;
		$this->Asset($assetid);

	}//end constructor


	/**
	* Create this asset
	*
	* @param array	&$link	information used to create the initial link
	*
	* @see Asset::create()
	* @return mixed int or false
	* @access public
	*/
	function create(&$link)
	{
		$name = trim($this->attr('name'));
		if ($name == '') {
			trigger_error('Unable to create a '.$GLOBALS['SQ_SYSTEM']->am->getTypeInfo($this->type(), 'name').' with a blank name', E_USER_WARNING);
			return false;
		}

		return parent::create($link);

	}//end create()


	/**
	* Returns name of the asset
	*
	* @param bool	$short_name	whether or not we are after the shortname or the full name
	*
	* @see Asset::_getName()
	* @access private
	*/
	function _getName($short_name=false)
	{
		return $this->attr('name');

	}//end _getName()


	/**
	* Returns an array of all the permitted links type, the type asset and the cardinality
	*
	* @see Asset::_getAllowLinks()
	* @access private
	*/
	function _getAllowedLinks()
	{
		return Array(SQ_LINK_TYPE_1 => Array(),
					 SQ_LINK_TYPE_2 => Array(),
					 SQ_LINK_TYPE_3 => Array(),
					 SQ_LINK_NOTICE => Array()
					);

	}//end _getAllowedLinks()


	/**
	* Get the default values for this field
	*
	* @param object Metadata_Field	&$asset	the asset whose values we are getting
	*
	* @return array
	* @access public
	*/
	function getDefaultValue()
	{
		return Metadata_Field::encodeValueString($this->attr('default'), $this->attr('value_components'));

	}//end getDefaultValue()


	/**
	* Generate an array of current values for the passed metadata field names
	*
	* @param object Asset			&$asset		the asset whose field values we are getting
	* @param string					$value_str	the current value for this field in the asset (NOT $asset) 
	*											NULL means that there is no current value set
	*
	* @access public
	* @return string	the metadata value for the passed asset 
	*/
	function getMetadataValue(&$asset, $value_str)
	{
		require_once SQ_FUDGE_PATH.'/general/text.inc';
		$mm = &$GLOBALS['SQ_SYSTEM']->getMetadataManager();

		if (is_null($value_str)) {
			$value_str = Metadata_Field::encodeValueString($this->attr('default'), $this->attr('value_components'));
		}

		$used_keywords = retrieve_keywords_replacements($value_str, '.');

		$used_keywords = $mm->generateKeywordReplacements($asset, $used_keywords, false);

		return replace_keywords($value_str, $used_keywords);

	}//end getMetadataValue()


	/**
	* Takes a metadata value and value-components and returns a properly escaped string
	*  example string :
	* <code>
	*	VALUE : This is a test desc
	*	COMPONENTS : Array
	*				(
	*					[corporateName] => squiz UK
	*					[jurisdiction] => uk
	*					[email] => brobertson@squiz.co.uk
	*					[address] => 140 Tabernacle St Old Street
	*				)
	*
	*   // this will return 
	*
	*   This is a test desc; corporateName=squiz UK; jurisdiction=uk; email=brobertson@squiz.co.uk; address=140 Tabernacle St Old Street;
	* </code>
	*
	* Derived from information found here: From http://dublincore.org/documents/dcmi-dcsv/index.shtml
	*
	* @param string		&$value			the variable to place the value into
	* @param Array()	&$components	the associative array to put the components into
	*
	* @return string	the encoded metadata value
	* @access public
	* @static
	* @final
	* @see decodeValueString()
	*/
	function encodeValueString($value, $components)
	{
		// NOTE: need to have the 4 slashes == 2 slashes in regex == 1 slash literal being matched/printed
		$regex   = '/([;=\\\\])/';
		$replace = '\\\\$1';

		$str = preg_replace($regex, $replace, $value);
		foreach($components as $k => $v) {
			$str .= '; '.preg_replace($regex, $replace, $k).'='.preg_replace($regex, $replace, $v);
		}// end foreach

		if (!empty($components)) $str .= ';';

		return $str;

	}//end encodeValueString()


	/**
	* Takes a full metadata string (i.e. value and value components) and returns the value and value components
	*  example string :
	* <code>
	*   This is a test desc; corporateName=squiz UK; jurisdiction=uk; email=brobertson@squiz.co.uk; address=140 Tabernacle St Old Street;
	*   // this will return 
	*
	*	VALUE : This is a test desc
	*	COMPONENTS : Array
	*				(
	*					[corporateName] => squiz UK
	*					[jurisdiction] => uk
	*					[email] => brobertson@squiz.co.uk
	*					[address] => 140 Tabernacle St Old Street
	*				)
	*
	* </code>
	*
	* Derived from perl-code example located here: From http://dublincore.org/documents/dcmi-dcsv/index.shtml
	*
	* @param string		$str				the string to decode
	* @param string		&$value				the variable to place the value into
	*										the value is defined as the concatentation of all strings that don't 
	*										have a component name (ie a 'name=' before them)
	* @param Array()	&$components		the associative array to put the components into
	*										If the array is NOT empty then only components in $str that are elements
	*										in $components will be set
	*
	* @return void
	* @access public
	* @static
	* @final
	* @see encodeValueString()
	*/
	function decodeValueString($str, &$value, &$components)
	{

		// First escape % characters (note: we use % instead of % so that keyword replacements don't have any issues)
		$str = str_replace('%', '%'.ord('%').'%', $str);
		// Next change \ escaped characters to ^d^ where d is the character's ascii code
		$str = preg_replace('/\\\\(.)/e', '"%".ord("\\1")."%"', $str);

		$value = '';
		$check_component = !empty($components);
		$parts = explode(';', $str);
		foreach($parts as $part) {
			if ($part == '') continue;
			$pair = explode('=', $part, 2);

			// strip whitespace from name string
			$pair[0] = preg_replace('/^\s*(\S+)\s*$/', '$1', $pair[0]);

			// convert % escaped characters back
			$pair[0] = preg_replace('/%(\d+)%/e', 'chr($1)', $pair[0]);
			if (isset($pair[1])) $pair[1] = preg_replace('/%(\d+)%/e', 'chr($1)', $pair[1]);

			if (!isset($pair[1])) {
				if ($value != '') $value .= '; ';
				$value .= $pair[0];
			// if the array wasn't empty, then we need to make sure that the component is already in the array
			} elseif (!$check_component || isset($components[$pair[0]])) {
				$components[$pair[0]] = $pair[1];
			}

		}// end foreach

	}//end decodeValueString()


}//end class

?>