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
* $Id: metadata_field_date_edit_fns.inc,v 1.5.2.3 2004/03/02 18:35:51 brobertson Exp $
* $Name: not supported by cvs2svn $
*/

require_once SQ_FUDGE_PATH.'/datetime_field/datetime_field.inc';
require_once dirname(__FILE__).'/../../metadata_field/metadata_field_edit_fns.inc';

/**
* Metadata_Field_Text_Edit_Fns
*
* Purpose
*
*
* @author  Dmitri Iarandine <diarandine@squiz.net>
* @version $Version$ - 1.0
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
		Metadata_Field_Edit_Fns::Metadata_Field_Edit_Fns(); // calling super

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
	* Getting the date-time field (taking CronManager implementation as template)
	*
	* @param object Asset				&$asset	the owning asset
	* @param string						$prefix	prefix for form elements
	*
	* @return object DateTime_Field
	* @access private
	*/
	function _getEpochDateTimeField(&$asset, $prefix, $value=null)
	{
		// only if we are printing the default value
		if (is_null($value)) {
			$value = (int) $asset->attr('default');
		}

		$epoch_date = getdate($value);
		$value_arr = Array(
						'y' => $epoch_date['year'],
						'm' => $epoch_date['mon'],
						'd' => $epoch_date['mday'],
						'h' => $epoch_date['hours'],
						'i' => $epoch_date['minutes'],
						's' => $epoch_date['seconds'],
					);

		// parameters
		$parameters = $asset->attr('setup_params');
		if (empty($parameters)) {
			$date = getdate();
			$parameters = Array(
				'min'			=> '2003-01-01 00:00:00',
				'max'			=> ($date['year'] + 1).'-12-31 23:59:59',
				'allow_circa'	=> '0',
				'show'			=> Array('y', 'm', 'd', 'h', 'i', 's'),
				'null'			=> Array(),
				'style'			=> Array(
								'y' => 's',
								'm' => 's',
								'd' => 's',
								'h' => 's',
								'i' => 's',
								's' => 's',
							),
			);
		}
		$parameters['print_format'] = $asset->attr('format');

		$value = '';
		$field = new DateTime_Field($prefix.'_epoch', $value, $parameters);
		$value = $field->unitsArrayToStr($value_arr);
		$field->setValue($value);

		return $field;

	}// end _getEpochDateTimeField()


	/**
	* Print the interface for editing the cron manager's Epoch
	*
	* @param object Asset				&$asset	the owning asset
	* @param object Backend_Outputter	&$o		the backend outputter
	* @param string						$prefix	prefix for form elements
	*
	* @return boolean
	* @access public
	*/
	function paintEpoch(&$asset, &$o, $prefix)
	{
		$default = $asset->attr('default');
		$params = $asset->attr('setup_params');

		$percent_pos = strpos($default, '%'); // where the percent sign was found (if at all)

		// parsing the keyword if its not a datestamp already before proceeding...
		if ($percent_pos === 0) {
				$temp_val = str_replace("%", "", $default);
				$keys = Array();
				array_push($keys, $temp_val);
				$mm = &$GLOBALS['SQ_SYSTEM']->getMetadataManager();

				$res = $mm->generateKeywordReplacements($asset, $keys, false); // just return value

				foreach($res as $key => $val) {
					$default = strtotime($val);
				}
		}

		$field = $this->_getEpochDateTimeField($asset, $prefix, $default);

		echo "Value in this field is relevant only if you do not select a keyword replacement<br />";

		// take into account lock, plus only root can edit
		if ($asset->writeAccess('attributes')) $field->printField();
		else echo $field->format();

	}//end paintEpoch()


	/**
	* Processes the interface for editing the cron manager's Epoch
	*
	* @param object Asset				&$asset	the owning asset
	* @param object Backend_Outputter	&$o		the backend outputter
	* @param string						$prefix	prefix for form elements
	*
	* @return boolean
	* @access public
	*/
	function processEpoch(&$asset, &$o, $prefix)
	{
		$field = $this->_getEpochDateTimeField($asset, $prefix);

		if ($asset->writeAccess('attributes') && $field->processField()) {
			$epoch = strtotime($field->value);
			return $asset->setAttrValue('default', $epoch);
		}

		return false;

	}//end processEpoch()


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
							'%asset_updated%'	=> 'Last updated date',
							'%asset_created%'	=> 'Creation date'
		);

		echo "Selected keyword replacement will be used instead of any entered default date values<br />";

		// displaying default value if any values were actually saved in the array
		if ($write_access) {
			combo_box($prefix.'_repkeys', $select_options, false, $default);
		} else {
			if (strpos($default, '%') === 0) echo $default;
			else echo 'None selected';
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
		if ($new_default != '') $asset->setAttrValue('default', $new_default);

		return true;

	}//end processRepKeys()


	/**
	* Print the interface for editing the cron manager's Epoch
	*
	* @param object Asset				&$asset	the owning asset
	* @param object Backend_Outputter	&$o		the backend outputter
	* @param string						$prefix	prefix for form elements
	*
	* @return boolean
	* @access public
	*/
	function paintEpochSetup(&$asset, &$o, $prefix)
	{
		$field = $this->_getEpochDateTimeField($asset, $prefix);
		
		echo "Setup table below only affects how the field looks to the metadata editor,<br /> NOT how it is displayed in the META field of your web page.";

		if ($asset->writeAccess('attributes')) {
			// custom definition for edit interface
			$arr = Array('s', 'n', 'c');
			$field->printEditOptions($arr);
		} else {
			echo $field->format();
		}

	}//end paintEpochSetup()


	/**
	* Processes the interface for editing the cron manager's Epoch
	*
	* @param object Asset				&$asset	the owning asset
	* @param object Backend_Outputter	&$o		the backend outputter
	* @param string						$prefix	prefix for form elements
	*
	* @return boolean
	* @access public
	*/
	function processEpochSetup(&$asset, &$o, $prefix)
	{
		$field = $this->_getEpochDateTimeField($asset, $prefix);

		if ($asset->writeAccess('attributes') && $field->processEditOptions()) {
			return $asset->setAttrValue('setup_params', $field->parameters);
		}

		return false;

	}//end processEpochSetup()


	/**
	* Paint the interface for filling in metadata values
	*
	* @param object Metadata_Field		&$asset			the asset whose interface we are painting
	* @param object	Backend_Outputter	&$o				the outputter class
	* @param array						$values			current metadata values set for this asset
	* @param boolean					$write_access	does the current user have write access to the asset
	*													that we are setting metadata for (NOT THE SAME AS $asset)
	*
	* @return boolean
	* @access public
	*/
	function paintInlineValueInterface(&$asset, &$o, $values, $write_access)
	{
		$prefix = $asset->getPrefix();
		$field_name = $asset->name;

		if ($asset->attr('required')) $field_name .= ' <span style="color: #FF0000">*</span>';

		$select_options = Array(
							''					=> '-- keywords --',
							'%asset_updated%'	=> 'Last updated date',
							'%asset_created%'	=> 'Creation date'
		);

		$o->openField($field_name);

			$params = $asset->attr('setup_params');
			$default = $asset->attr('default');

			// make sure we have at least something in the values var
			if (empty($values)) $values['value'] = $default;

			// checking if the value must be parsed before processing
			$percent_pos = strpos($values['value'], '%');
			$initial = $values['value']; // default value for the dropdown box

			if ($percent_pos === 0) {
				$temp_val = str_replace('%', '', $values['value']);
				$keys = Array();
				array_push($keys, $temp_val);

				$mm = &$GLOBALS['SQ_SYSTEM']->getMetadataManager();
				$res = $mm->generateKeywordReplacements($asset, $keys, false); // just return value
				foreach($res as $key => $val) $values['value'] = strtotime($val);
			}

			$field = $this->_getEpochDateTimeField($asset, $prefix, $values['value']);

			// take into account lock, plus only root can edit
			if ($write_access && $asset->attr('editable')) {
				$field->printField();
				echo " <b>-OR-</b> ";
				combo_box($prefix.'_repkeys', $select_options, false, $initial); // replacement keys
			} else {
				echo $field->format();
			}

		$o->closeField();

		return true;

	}//end paintInlineValueInterface()


	/**
	* Process the interface for filling in metadata values
	*
	* @param object Metadata_Field	&$asset			the asset whose interface we are painting
	* @param array					&$new_values	an array we can store the new values in
	*
	* @return boolean
	* @access public
	*/
	function processInlineValueInterface(&$asset, &$new_values)
	{
		$prefix = $asset->getPrefix();

		if ($asset->attr('editable')) {

			$field = $this->_getEpochDateTimeField($asset, $prefix);

			if ($field->processField()) {
				$new_default = trim($_POST[$prefix.'_repkeys']);
				if ($new_default != '') $new_values[$asset->name]['value'] = $new_default;
				else $new_values[$asset->name]['value'] = strtotime($field->value);
			} else {
				// field could not be processed
				trigger_error('Date field "'.$asset->name.'" could not be processed', E_USER_WARNING);
				return false;
			}

		}//end if editable

		return true;

	}//end processInlineValueInterface()


	/**
	* Output the complete metadata tag for this field
	*
	* @param object Metadata_Field	&$asset		the asset whose interface we are painting
	* @param array					&$values		the current metadata values
	 @param array					&$tag_value	the metadata tag value
	* @param array					&$keywords	an array we can put keywords in that need replacing
	*
	* @return boolean
	* @access public
	*/
	function generateMetadata(&$asset, &$values, &$tag_value, &$keywords)
	{
		if (!isset($values['using_default'])) $values['using_default'] = false;
		if (isset($values['value']) && !$values['using_default']) {
			$value = $values['value'];
			$values['using_default'] = false;
		} else {
			$value = $asset->attr('default');
			$values['value'] = $value;
			$values['using_default'] = true;
		}

		$original = $value;

		// we create a new array here to pass to our parent so we dont muck up the
		// referenced timestamp values
		$new_values = $values;
		if (strpos($value, '%') === false) {
			$field = $this->_getEpochDateTimeField($asset, '', $value);
			$new_values['value'] = $field->format();
		} else {
			$new_values['value'] = $value;
		}
		// we dont want out default value restored even if we are using it
		// because we have already formatted it here
		$new_values['using_default'] = false;

		ob_start();
			parent::generateMetadata($asset, $new_values, $tag_value, $keywords);
			$metadata_tag = ob_get_contents();
		ob_end_clean();

		if (strpos($original, '%') === 0) {
			// strip off the last % sign from the end of $key string
			$keylen = strlen($original);
			$key = substr($original, 0, $keylen-1);

			// prepare date format for merging - we will replace all spaces with underscores
			$format = str_replace(' ', '-', $asset->attr('format'));

			// merge the values with '_', add closing % sign and proceed...
			$key = $key.'_'.$format.'%';
			$metadata_tag = str_replace($original, $key, $metadata_tag);
			$keywords[] = substr($key, 1, -1);
		}

		echo $metadata_tag;

		return true;

	}//end generateMetadata()


	/**
	* Generate an array of current values for the passed metadata field names
	*
	* @param object Asset			&$asset		the asset whose field values we are getting
	* @param object Metadata_Field	&$field		the asset whose interface we are painting
	* @param array					$values		the current metadata values
	* @param array					&$fields	an array of field names that need replacing
	*
	* @access public
	* @return boolean
	*/
	function getMetadataFieldValues(&$asset, &$field, $values, &$fields)
	{
		if (!isset($fields[$field->name])) return;

		if (isset($values['value'])) $default = $values['value'];
		else $default = $field->attr('default');

		$used_keywords = Array();

		if (strpos($default, '%') === false) {
			$datetime_field = $this->_getEpochDateTimeField($field, '', $default);
			$field_value = $datetime_field->format();

		} else {
		
			// strip off the last % sign from the end of $key string
			$keylen = strlen($default);
			$key = substr($default, 0, $keylen-1);

			// prepare date format for merging - we will replace all spaces with underscores
			$format = str_replace(' ', '-', $field->attr('format'));

			// merge the values with '_', add closing % sign and proceed...
			$field_value = $key.'_'.$format.'%';
			$used_keywords[] = substr($field_value, 1, -1);
		}

		$mm = &$GLOBALS['SQ_SYSTEM']->getMetadataManager();
		$used_keywords = $mm->generateKeywordReplacements($asset, $used_keywords, false);

		$fields[$field->name] = replace_keywords($field_value, $used_keywords);

		return true;

	}//end getMetadataFieldValues()


}//end class

?>
