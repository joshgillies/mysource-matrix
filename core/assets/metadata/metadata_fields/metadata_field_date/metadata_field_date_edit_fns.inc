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
* $Id: metadata_field_date_edit_fns.inc,v 1.3 2004/02/03 22:34:10 mmcintyre Exp $
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
				'print_format'	=> $asset->attr('format'),
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

		$value = '';
		$field = new DateTime_Field($prefix.'_epoch', $value, $parameters);
		$value = $field->unitsArrayToStr($value_arr);
		$field->setValue($value);

		return $field;

	}// end _getEpochDateTimeField()


	/**
	* Prints the interface for editing the cron manager's Epoch
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

		// take into account lock, plus only root can edit
		if ($asset->writeAccess('attributes')) {
			$field->printField();
		} else {
			echo $field->format();
		}// end if

	}// end paintEpoch()


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

		// take into account lock, plus only root can edit
		if ($asset->writeAccess('attributes')) {

			if ($field->processField()) {

				$epoch = strtotime($field->value);
				return $asset->setAttrValue('default', $epoch);

			}// end if
		
		}// end if

		return false;

	}// end processEpoch()


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
							''					=> '- replacement keys -',
							'%asset_updated%'	=> 'Last updated Date',
							'%asset_created%'	=> 'Creation date'
		);

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
	* Processes the interface for dropdown selection box
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
			$asset->setAttrValue('default', $new_default); // saving anyway
		}

		return true;

	}//end processRepKeys()


	/**
	* Prints the interface for editing the cron manager's Epoch
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
		$params = $asset->attr('setup_params');
		
		if (is_null($params) || !isset($params) || empty($params)) {
			$field = $this->_getEpochDateTimeField($asset, $prefix);
		} else {
			$field = $this->_getEpochDateTimeField($asset, $prefix);
		}

		// take into account lock, plus only root can edit
		if ($asset->writeAccess('attributes')) {
			$arr = Array('s', 'n', 'c'); // custom definition for edit interface
			$field->printEditOptions($arr);
		} else {
			echo $field->format();
		}// end if

	}// end paintEpochSetup()


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

		// take into account lock, plus only root can edit
		if ($asset->writeAccess('attributes')) {
			if ($field->processEditOptions()) {

				return $asset->setAttrValue('setup_params', $field->parameters);

			}// end if
		}// end if

		return false;

	}// end processEpochSetup()


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
							''					=> '- replacement keys -',
							'%asset_updated%'	=> 'Last updated Date',
							'%asset_created%'	=> 'Creation date'
		);

		$o->openField($field_name);

			$params = $asset->attr('setup_params');
			$default = $asset->attr('default');

			// checking if the value must be parsed before processing
			$percent_pos = strpos($values['value'], '%'); 
			$initial = $values['value']; // default value for the dropdown box

			if ($percent_pos === 0) {
				$temp_val = str_replace("%", "", $values['value']);
				$keys = Array(); 
				array_push($keys, $temp_val);
				$mm = &$GLOBALS['SQ_SYSTEM']->getMetadataManager();

				$res = $mm->generateKeywordReplacements($asset, $keys, false); // just return value

				foreach($res as $key => $val) {
					$values['value'] = strtotime($val);
				}
			}

			$field = $this->_getEpochDateTimeField($asset, $prefix, $values['value']);

			// take into account lock, plus only root can edit
			if ($write_access) {
				$field->printField();
				echo " <b>-OR-</b> ";
				combo_box($prefix.'_repkeys', $select_options, false, $initial); // replacement keys
			} else {
				echo $field->format();
			}// end if

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
				$value_arr = $field->strToUnitsArray($field->value);

				// saving submitted value as a default
				$new_default = trim($_POST[$prefix.'_repkeys']);

				if ($new_default != '') {
					$new_values[$asset->name]['value'] = $new_default;
				} else {
					$new_values[$asset->name]['value'] = strtotime($field->value);
				}

			} else {
				// field could not be processed
				trigger_error('Date field "'.$asset->name.'" could not be processed', E_USER_WARNING);
				return false;
			}

		}

		return true;

	}//end processInlineValueInterface()


	/**
	* Output the complete metadata tag for this field
	*
	* @param object Metadata_Field	&$asset		the asset whose interface we are painting
	* @param array					$values		the current metadata values
	* @param array					&$keywords	an array we can put keywords in that need replacing
	*
	* @return boolean
	* @access public
	*/
	function generateMetadata(&$asset, $values, &$keywords)
	{
		if (isset($values['value'])) $value = $values['value'];
		else $value = $asset->attr('default');

		$original = $value;

		if (strpos($value, '%') === false) {
			$field = $this->_getEpochDateTimeField($asset, '', $value);
			$values['value'] = $field->format();
		} else {
			$values['value'] = $value;
		}

		ob_start();
			parent::generateMetadata($asset, $values, $keywords);
			$metadata_tag = ob_get_contents();
		ob_end_clean();

		$format = $asset->attr('format'); 
		$percent_pos = strpos($original, '%'); // where the percent sign was found (if at all)

		if ($percent_pos === 0) { // '%' sign was found at 0 position

			// 1). strip off the last % sign from the end of $key string
			$keylen = strlen($original); 
			$key = substr($original, 0, $keylen-1);
			
			// 2). prepare date format for merging - we will replace all spaces with underscores
			$format = str_replace(" ", "-", $format);
			
			// 3). merge the values with '_', add closing % sign and proceed...
			$key = $key . "_" . $format . "%";
			$metadata_tag = str_replace($original, $key, $metadata_tag);
			$keywords[] = substr($key, 1,-1);

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

		require_once SQ_FUDGE_PATH.'/general/text.inc';
		$mm = &$GLOBALS['SQ_SYSTEM']->getMetadataManager();

		if (isset($values['value'])) $default = $values['value'];
		else $default = $field->attr('default');

		$used_keywords = retrieve_keywords_replacements($default);

		$value_string = '';
		$current_vc = $field->attr('value_components');
		if (!empty($current_vc)) {
			foreach ($current_vc as $vc_name => $vc_value) {
				if (isset($values['value_components'][$vc_name])) {
					$vc_value = $values['value_components'][$vc_name];
				}
				$used_keywords = array_merge($used_keywords, retrieve_keywords_replacements($vc_value));
				$value_string .= $vc_name.'='.$vc_value.'; ';
			}
		}

		$used_keywords = $mm->generateKeywordReplacements($asset, $used_keywords, false);

		$field_value = $default;
		if (trim($value_string) != '') $field_value .= '; '.trim($value_string);
		$fields[$field->name] = replace_keywords($field_value, $used_keywords);

		return true;

	}//end getMetadataFieldValues()


}//end class

?>
