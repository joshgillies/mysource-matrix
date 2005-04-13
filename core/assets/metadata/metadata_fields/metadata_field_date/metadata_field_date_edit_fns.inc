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
* $Id: metadata_field_date_edit_fns.inc,v 1.17 2005/04/13 23:49:29 ndvries Exp $
*
*/

require_once dirname(__FILE__).'/../../metadata_field/metadata_field_edit_fns.inc';

/**
* Metadata_Field_Date_Edit_Fns
*
* Purpose
*
*
* @author  Dmitri Iarandine <diarandine@squiz.net>
* @version $Revision: 1.17 $
* @package MySource_Matrix_Packages
* @subpackage __core__
*/
class Metadata_Field_Date_Edit_Fns extends Metadata_Field_Edit_Fns
{


	/**
	* Constructor
	*
	*/
	function Metadata_Field_Date_Edit_Fns()
	{
		$this->Metadata_Field_Edit_Fns();

	}//end constructor


	/**
	* Paint the interface for viewing available date format keys
	*
	* @param object Metadata_Field		&$asset	the asset whose interface we are painting
	* @param object	Backend_Outputter	&$o		the outputter class
	* @param string						$prefix	prefix for the form elements
	*
	* @return boolean
	* @access public
	*/
	function paintDateFormatKeys(&$asset, &$o, $prefix)
	{
		?>
		<b>You can use the following set of Date object keys in order to define your own date format for this field.<br/>For a complete list of available keyword replacements <a href="#" onClick="Javascript: var win = window.open('<?php echo sq_web_path('data')?>/asset_types/metadata_field_date/popups/keywords.html', 'keywords', 'toolbar=0,menubar=0,location=0,status=0,scrollbars=1,resizable=1,width=640,height=520'); return false;">click here </a></b>
		<?php

	}//end paintDateFormatKeys()


	/**
	* Processes the interface for viewing date format keys
	*
	* Note that keyword instructions are not processed so this function always
	* returns false
	*
	* @param object Metadata_Field		$asset	the asset to which we belong
	* @param object	Backend_Outputter	$o		the outputter class
	* @param string						$prefix	prefix for the form element
	*
	* @return boolean (false always)
	* @access public
	*/
	function processDateFormatKeys(&$asset, &$o, $prefix)
	{
		return false;

	}//end processDateFormatKeys()


	/**
	* Print the interface for editing the default date
	*
	* @param object Asset				&$asset	the owning asset
	* @param object Backend_Outputter	&$o		the backend outputter
	* @param string						$prefix	prefix for form elements
	*
	* @return boolean
	* @access public
	*/
	function paintDefaultDateTime(&$asset, &$o, $prefix)
	{
		$default = $asset->attr('default');

		$uses_keyword = (substr($default, 0, 1) == '%');

		// parsing the keyword if its not a datestamp already before proceeding...
		if ($uses_keyword) {
			$temp_val = str_replace('%', '', $default);
			$mm = &$GLOBALS['SQ_SYSTEM']->getMetadataManager();

			$res = $mm->generateKeywordReplacements($asset, Array($temp_val), false); // just return value
			$default = $res[$temp_val];
		}

		$field = $asset->_getDateTimeField($prefix, $default);

		if ($asset->writeAccess('attributes')) {
			$field->printField();
		} else {
			echo $field->format();
		}

	}//end paintDefaultDateTime()


	/**
	* Processes the interface for editing the default date
	*
	* @param object Asset				&$asset	the owning asset
	* @param object Backend_Outputter	&$o		the backend outputter
	* @param string						$prefix	prefix for form elements
	*
	* @return boolean
	* @access public
	*/
	function processDefaultDateTime(&$asset, &$o, $prefix)
	{
		$field = $asset->_getDateTimeField($prefix);

		if ($asset->writeAccess('attributes') && $field->processField()) {
#			$default = strtotime($field->value);
			return $asset->setAttrValue('default', $field->value);
		}

		return false;

	}//end processDefaultDateTime()


	/**
	* Print the interface for editing the time offset
	*
	* @param object Asset				&$asset	the owning asset
	* @param object Backend_Outputter	&$o		the backend outputter
	* @param string						$prefix	prefix for form elements
	*
	* @return boolean
	* @access public
	*/
	function paintOffset(&$asset, &$o, $prefix)
	{
		$offset = $asset->attr('offset');
		if ($asset->writeAccess('attributes')) {
			text_box($prefix.'_offset', $offset, 30);
		} else {
			 echo $offset;
		}

	}//end paintOffset()


	/**
	* Processes the interface for editing the time offset
	*
	* @param object Asset				&$asset	the owning asset
	* @param object Backend_Outputter	&$o		the backend outputter
	* @param string						$prefix	prefix for form elements
	*
	* @return boolean
	* @access public
	*/
	function processOffset(&$asset, &$o, $prefix)
	{
		if (!isset($_POST[$prefix.'_offset'])) return false;
		return $asset->setAttrValue('offset', $_POST[$prefix.'_offset']);

	}//end processOffset()


	/**
	* Paint the interface for dropdown selection box
	*
	* @param object Metadata_Field_Selection	&$asset	the asset whose interface we are painting
	* @param object	Backend_Outputter	&$o		the outputter class
	* @param string						$prefix	prefix for the form elements
	*
	* @return boolean
	* @access public
	*/
	function paintRepKeys(&$asset, &$o, $prefix)
	{
		$write_access = $asset->writeAccess('attributes');
		$default = $asset->attr('default');

		$select_options = Array(
							''					=> '-- keywords --',
							'%asset_created%'	=> 'Creation date',
							'%asset_updated%'	=> 'Last updated date',
							'%asset_published%'	=> 'Last published date',
						  );

		echo 'Selected keyword replacement will be used instead of any entered default date values<br />';

		// displaying default value if any values were actually saved in the array
		if ($write_access) {
			combo_box($prefix.'_repkeys', $select_options, false, $default);
		} else {
			if (strpos($default, '%') === 0) {
				echo $default;
			} else {
				echo 'None selected';
			}
		}

		return true;

	}//end paintRepKeys()


	/**
	* Process the interface for dropdown selection box
	*
	* @param object Metadata_Field_Selection	$asset	the asset to which we belong
	* @param object	Backend_Outputter	$o		the outputter class
	* @param string						$prefix	prefix for the form element
	*
	* @return boolean
	* @access public
	*/
	function processRepKeys(&$asset, &$o, $prefix)
	{
		// need to have write access to make any changes
		if (!$asset->writeAccess('attributes')) return false;

		$new_default = trim($_POST[$prefix.'_repkeys']);
		if ($new_default != '') {
			$asset->setAttrValue('default', $new_default);
		}

		return true;

	}//end processRepKeys()


	/**
	* Print the interface for editing how the editing interface parameters for the date time field that is used
	* to enter the metadata value for an individual asset
	*
	* @param object Asset				&$asset	the owning asset
	* @param object Backend_Outputter	&$o		the backend outputter
	* @param string						$prefix	prefix for form elements
	*
	* @return boolean
	* @access public
	*/
	function paintSetupParams(&$asset, &$o, $prefix)
	{
		$field = $asset->_getDateTimeField($prefix);

		$o->note('Setup table below only affects how the field looks to the metadata editor,<br />NOT how it is displayed in the META field of your web page.');

		if ($asset->writeAccess('attributes')) {
			// custom definition for edit interface
			$arr = Array('s', 'n', 'c');
			$field->printEditOptions($arr);
		} else {
			echo $field->format();
		}

	}//end paintSetupParams()


	/**
	* Processes the interface for editing the Setup Params
	*
	* @param object Asset				&$asset	the owning asset
	* @param object Backend_Outputter	&$o		the backend outputter
	* @param string						$prefix	prefix for form elements
	*
	* @return boolean
	* @access public
	*/
	function processSetupParams(&$asset, &$o, $prefix)
	{
		$field = $asset->_getDateTimeField($prefix);

		if ($asset->writeAccess('attributes') && $field->processEditOptions()) {
			return $asset->setAttrValue('setup_params', $field->parameters);
		}

		return false;

	}//end processSetupParams()


	/**
	* Paint the interface for filling in metadata values
	*
	* @param object Metadata_Field		&$asset			the asset whose interface we are painting
	* @param object	Backend_Outputter	&$o				the outputter class
	* @param string						$value_str		the current value for this field in the asset (NOT $asset)
	*													NULL means that there is no current value set
	* @param boolean					$write_access	does the current user have write access to the asset
	*													that we are setting metadata for (NOT THE SAME AS $asset)
	*
	* @return boolean
	* @access public
	*/
	function paintInlineValueInterface(&$asset, &$o, $value_str, $write_access)
	{
		$prefix = $asset->getPrefix();

		if (is_null($value_str)) {
			$value = $asset->attr('default');
			$value_components = $asset->attr('value_components');
		} else {
			$value='';
			$value_components = $asset->attr('value_components');
			Metadata_Field::decodeValueString($value_str, $value, $value_components);
		}

		$field_name = $asset->name;
		if ($asset->attr('required')) {
			$field_name .= ' <span style="color: #FF0000">*</span>';
		}

		$select_options = Array(
							''					=> '-- keywords --',
							'%asset_created%'	=> 'Creation date',
							'%asset_updated%'	=> 'Last updated date',
							'%asset_published%'	=> 'Last published date',
						  );

		$o->openField($field_name);

			// checking if the value is a keyword replacement
			$uses_keyword = (substr($value, 0, 1) == '%');
			$field = $asset->_getDateTimeField($prefix, $value);

			if ($write_access && $asset->attr('editable')) {
				$field->printField();
				echo ' <b>-OR-</b> ';
				combo_box($prefix.'_repkeys', $select_options, false, $value); // replacement keys
			} else if ($uses_keyword && isset($select_options[$value])) {
				echo '<i>Keyword Replacement: </i>', $select_options[$value];
			} else {
				echo $field->format();
			}

		$o->closeField();

		return true;

	}//end paintInlineValueInterface()


	/**
	* Process the interface for filling in metadata values
	*
	* @param object Metadata_Field	&$asset				the asset whose interface we are processing
	* @param string					&$new_value_str		a string by reference that we can store the new values in
	*
	* @return boolean
	* @access public
	*/
	function processInlineValueInterface(&$asset, &$new_value_str)
	{
		// nothing for us to do
		if (!$asset->attr('editable')) return true;

		$prefix = $asset->getPrefix();

		$value = array_get_index($_POST, $prefix.'_repkeys', '');

		// if we aren't using keywords...get the date that was set
		if ($value == '') {
			$field = $asset->_getDateTimeField($prefix);
			if ($field->processField()) {
				$value = $field->value;
			} else {
				// field could not be processed
				trigger_error('Date field "'.$asset->name.'" could not be processed', E_USER_WARNING);
				return false;
			}
		}

		$new_value_str = Metadata_Field::encodeValueString(trim($value), Array());

		return true;

	}//end processInlineValueInterface()


	/**
	* Output the complete metadata tag for this field
	*
	* @param object Metadata_Field	&$asset		the asset whose interface we are painting
	* @param string					$value_str	the current value for this field in the asset (NOT $asset)
	*											NULL means that there is no current value set
	* @param array					&$tag_value	the metadata tag value
	* @param array					&$keywords	an array we can put keywords in that need replacing
	*
	* @return boolean
	* @access public
	*/
	function generateMetadata(&$asset, $value_str, &$tag_value, &$keywords)
	{
		require_once SQ_FUDGE_PATH.'/general/text.inc';

		$frontend_option = $asset->attr('frontend');
		// if we aren't printing this on the frontend then don't print anything
		if ($frontend_option == 'hide') return true;


		if (is_null($value_str)) {
			$value = $asset->attr('default');
			$value_components = $asset->attr('value_components');
		} else {
			$value='';
			$value_components = $asset->attr('value_components');
			Metadata_Field::decodeValueString($value_str, $value, $value_components);
		}

		$original = $value;

		// we create a new array here to pass to our parent so we dont muck up the
		// referenced timestamp values
		if (substr($value, 0, 1) != '%') {
			$field = $asset->_getDateTimeField('', $value);
			$value = $field->format();
		}

		ob_start();
			parent::generateMetadata($asset, Metadata_Field::encodeValueString($value, $value_components), $tag_value, $keywords);
			$metadata_tag = ob_get_contents();
		ob_end_clean();

		if (substr($original, 0, 1) == '%') {
			// strip off the last % sign from the end of $key string
			$key = substr($original, 0, -1);

			// prepare date format for merging - we will replace all spaces with underscores
			$format = str_replace(' ', '-', $asset->attr('format'));

			// merge the values with '_', add closing % sign and proceed...
			$key = $key.'_'.$format;
			$offset = $asset->attr('offset');
			if (!empty($offset)) {
				$offset = str_replace(' ', '', $offset);
				$key .= '_'.$offset;
			}
			$key .= '%';

			// replace the plain keyword with the one that contains the format
			$tag_value = str_replace($original, $key, $tag_value);

			// replace the plain keyword (or at least it's first occurance in the keywords array)
			// with the one that contains the format
			$keyword = substr($original, 1, -1);
			if (($i = array_search($keyword, $keywords)) !== false) {
				$keywords[$i] = substr($key, 1, -1);
			}

		}// end if

		echo $metadata_tag;

		return true;

	}//end generateMetadata()


}//end class

?>
