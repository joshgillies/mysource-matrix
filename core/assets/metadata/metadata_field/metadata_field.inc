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
* $Id: metadata_field.inc,v 1.25 2006/01/26 22:34:04 lwright Exp $
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
* @version $Revision: 1.25 $
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
		$this->_ser_attrs = TRUE;
		$this->Asset($assetid);

	}//end constructor


	/**
	* Perform any pre-validation required during the creation of this asset
	*
	* Folders ensure a name has been set before they are created
	*
	* @param array	&$link	information used to create the initial link
	*
	* @return boolean
	* @access private
	*/
	function _preCreateCheck(&$link)
	{
		if (!parent::_preCreateCheck($link)) return FALSE;

		$name = trim($this->attr('name'));
		if ($name == '') {
			trigger_localised_error('CORE0083', E_USER_WARNING, $GLOBALS['SQ_SYSTEM']->am->getTypeInfo($this->type(), 'name'));
			return FALSE;
		}

		return TRUE;

	}//end _preCreateCheck()


	/**
	* Returns name of the asset
	*
	* @param boolean	$short_name	whether or not we are after the shortname or the full name
	*
	* @return string
	* @access private
	* @see Asset::_getName()
	*/
	function _getName($short_name=FALSE)
	{
		return $this->attr('name');

	}//end _getName()


	/**
	* Returns an array of all the permitted links type, the type asset and the cardinality
	*
	* @return array
	* @access private
	* @see Asset::_getAllowLinks()
	*/
	function _getAllowedLinks()
	{
		return Array(
				SQ_LINK_TYPE_1	=> Array(),
				SQ_LINK_TYPE_2	=> Array(),
				SQ_LINK_TYPE_3	=> Array(),
				SQ_LINK_NOTICE	=> Array(),
			   );

	}//end _getAllowedLinks()


	/**
	* Get the default values for this field
	*
	* @return array
	* @access public
	*/
	function getDefaultValue()
	{
		$mm =& $GLOBALS['SQ_SYSTEM']->getMetadataManager();
		return $mm->getMetadataFieldDefaultValue($this->id);

	}//end getDefaultValue()


	/**
	* Generate an array of current values for the passed metadata field names
	*
	* @param object	&$asset		the asset whose field values we are getting
	* @param string	$value_str	the current value for this field in the asset (NOT $asset)
	*							NULL means that there is no current value set
	*
	* @return string
	* @access public
	*/
	function getMetadataValue(&$asset, $value_str)
	{
		require_once SQ_FUDGE_PATH.'/general/text.inc';
		$mm =& $GLOBALS['SQ_SYSTEM']->getMetadataManager();

		// if there are no components that we know about and we are not being displayed on the frontend,
		// then just unescape the value string and return that
		if (!$this->attr('value_components') && $this->attr('frontend') == 'hide') {
			Metadata_Field::decodeValueString($value_str, $value='', $components=Array());
			return $value;
		}

		// get the default value
		if (is_null($value_str)) {
			$value_str = $this->getDefaultValue();
		}

		$used_keywords = retrieve_keywords_replacements($value_str, '.');

		$used_keywords = $mm->generateKeywordReplacements($asset, $used_keywords, FALSE);

		return replace_keywords($value_str, $used_keywords);

	}//end getMetadataValue()


	/**
	* Takes a metadata value and value-components and returns a properly escaped string
	*
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
	*   // this will return
	*   This is a test desc; corporateName=squiz UK; jurisdiction=uk; email=brobertson@squiz.co.uk; address=140 Tabernacle St Old Street;
	* </code>
	* Derived from information found here: From http://dublincore.org/documents/dcmi-dcsv/index.shtml
	*
	* @param string	$value		the variable to place the value into
	* @param array	$components	the associative array to put the components into
	*
	* @return string
	* @access public
	* @see decodeValueString()
	*/
	function encodeValueString($value, $components)
	{
		// NOTE: need to have the 4 slashes == 2 slashes in regex == 1 slash literal being matched/printed
		$regex   = '/([;=\\\\])/';
		$replace = '\\\\$1';

		$str = preg_replace($regex, $replace, $value);
		foreach ($components as $k => $v) {
			$str .= '; '.preg_replace($regex, $replace, $k).'='.preg_replace($regex, $replace, $v);
		}// end foreach

		if (!empty($components)) $str .= ';';
		return $str;

	}//end encodeValueString()


	/**
	* Takes a full metadata string (i.e. value and value components) and returns the value and value components
	*
	*  example string :
	* <code>
	*   This is a test desc; corporateName=squiz UK; jurisdiction=uk; email=brobertson@squiz.co.uk; address=140 Tabernacle St Old Street;
	*   // this will return
	*	VALUE : This is a test desc
	*	COMPONENTS : Array
	*				(
	*					[corporateName] => squiz UK
	*					[jurisdiction] => uk
	*					[email] => brobertson@squiz.co.uk
	*					[address] => 140 Tabernacle St Old Street
	*				)
	* </code>
	* Derived from perl-code example located here: From http://dublincore.org/documents/dcmi-dcsv/index.shtml
	*
	* @param string	$str			the string to decode
	* @param string	&$value			the variable to place the value into
	*								the value is defined as the concatentation of all strings that don't
	*								have a component name (ie a 'name=' before them)
	* @param array	&$components	the associative array to put the components into
	*								If the array is NOT empty then only components in $str that are elements
	*								in $components will be set
	*
	* @return void
	* @access public
	* @see encodeValueString()
	*/
	function decodeValueString($str, &$value, &$components)
	{

		// First escape % characters
		$str = str_replace('%', '%'.ord('%').'%', $str);
		// Next change \ escaped characters to ^d^ where d is the character's ascii code
		$str = preg_replace('/\\\\(.)/e', '"%".ord("\\1")."%"', $str);

		$value = '';
		$check_component = !empty($components);
		$parts = explode(';', $str);
		foreach ($parts as $part) {
			if ($part == '') continue;
			$pair = explode('=', $part, 2);

			// strip whitespace from name string
			$pair[0] = preg_replace('/^\s*(\S+)\s*$/', '$1', $pair[0]);

			// trim it
			$pair[0] = trim($pair[0]);

			// convert % escaped characters back
			$pair[0] = preg_replace('/%(\d+)%/e', 'chr($1)', $pair[0]);

			if (isset($pair[1])) {
				$pair[1] = preg_replace('/%(\d+)%/e', 'chr($1)', $pair[1]);
			}

			if (!isset($pair[1])) {
				if ($value != '') $value .= '; ';
				$value .= $pair[0];
			// if the array wasn't empty, then we need to make sure that the component is already in the array
			} else if (!$check_component || isset($components[$pair[0]])) {
				$components[$pair[0]] = $pair[1];
			}

		}// end foreach

	}//end decodeValueString()


	/**
	* Save attributes set with setAttrValue to the database
	*
	* Overridden in Metadata fields so that the default value can be saved
	*
	* @param boolean	$dont_run_updated	if TRUE this->_updated() won't be run
	*										(THIS IS ONLY EVER CALLED FROM Asset::create())
	*
	* @return boolean
	* @access public
	* @see setAttrValue()
	*/
	function saveAttributes($dont_run_updated=FALSE)
	{
		$GLOBALS['SQ_SYSTEM']->changeDatabaseConnection('db2');
		$GLOBALS['SQ_SYSTEM']->doTransaction('BEGIN');

		$default_val = $this->encodeValueString($this->attr('default'), $this->attr('value_components'));

		$mm =& $GLOBALS['SQ_SYSTEM']->getMetadataManager();

		if (!$mm->setMetadataFieldDefaultValue($this->id, $default_val)) {
			$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
			$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();
			return FALSE;
		}

		if (!parent::saveAttributes($dont_run_updated)) {
			$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
			$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();
			return FALSE;
		}

		$GLOBALS['SQ_SYSTEM']->doTransaction('COMMIT');
		$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();

		return TRUE;

	}//end saveAttributes()


	/**
	* Delete this asset from the trash
	*
	* Removes any reference to this field from both the 'sq_ast_mdata_dflt_val'
	* and 'sq_ast_mdata_val' tables to avoid phantom entries for assets
	*
	* @param boolean	$release_lock	should we realease the lock after deleting
	*
	* @return boolean
	* @access public
	*/
	function delete($release_lock=TRUE)
	{
		if (parent::delete($release_lock)) {
			$GLOBALS['SQ_SYSTEM']->changeDatabaseConnection('db2');
			$db = $GLOBALS['SQ_SYSTEM']->db;

			$GLOBALS['SQ_SYSTEM']->doTransaction('BEGIN');

			// First remove any values assets have for this field
			$sql = 'DELETE FROM sq_ast_mdata_val WHERE ';
			$sql .= 'fieldid = '.$db->quote($this->id);

			$result = $db->query($sql);
			assert_valid_db_result($result);

			// Now we have to delete the default value information
			$sql = 'DELETE FROM sq_ast_mdata_dflt_val WHERE ';
			$sql .= 'assetid = '.$db->quote($this->id);

			$result = $db->query($sql);
			assert_valid_db_result($result);

			$GLOBALS['SQ_SYSTEM']->doTransaction('COMMIT');
			$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();

			return TRUE;
		}
		return FALSE;

	}//end delete()


}//end class

?>
