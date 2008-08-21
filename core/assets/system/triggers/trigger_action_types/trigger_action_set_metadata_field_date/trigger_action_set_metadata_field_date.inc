<?php
/**
* +--------------------------------------------------------------------+
* | This MySource Matrix CMS file is Copyright (c) Squiz Pty Ltd       |
* | ACN 084 670 600                                                    |
* +--------------------------------------------------------------------+
* | IMPORTANT: Your use of this Software is subject to the terms of    |
* | the Licence provided in the file licence.txt. If you cannot find   |
* | this file please contact Squiz (www.squiz.net) so we may provide   |
* | you a copy.                                                        |
* +--------------------------------------------------------------------+
*
*/

require_once SQ_CORE_PACKAGE_PATH.'/system/triggers/trigger_action/trigger_action.inc';

/**
* Trigger_Action_Set_Metadata_Field_Date
*
* Sets the a metadata field date with the curernt date time plus a variable offset
*
* @author  Geoffroy Noel <gnoel@squiz.co.uk>
* @version $Revision: 1.1 $
* @package MySource_Matrix_Packages
* @subpackage __core__
*/
class Trigger_Action_Set_Metadata_Field_Date extends Trigger_Action
{

	/**
	* Execute this action
	* Set the metadata field date with the current date/time + offset
	*
	* Returns an array of data about what it did, or false on error
	* The settings used by this action are in the form:<BR />
	* <PRE>
	* Array(
	*       'fieldid'		=> int    // metadata_field id
	*		'field_name'	=> string // name of the field
	*       'value'			=> string // the value to set
	*       );
	* </PRE>
	*
	* @param array	$settings	the stored settings for this action
	* @param array	&$state		the state that describes the asset the action is executing on
	*
	* @return mixed array|boolean
	* @access public
	*/
	public static function execute($settings, &$state)
	{
		$mm =& $GLOBALS['SQ_SYSTEM']->getMetadataManager();

		// check required settings
		if (empty($settings['fieldid'])) return FALSE;

		// Format: 2008-02-01 00:00:00
		$value = date('Y-m-d H:i:s', strtotime('+'.$settings['offset_months'].' month '.$settings['offset_days'].' day '.$settings['offset_hours'].' hour '.$settings['offset_mins'].' min'));
		
		if (empty($state['asset'])) {
			// grab the asset if assetid is given, but not the asset.
			if (empty($state['assetid'])) {
				return FALSE;
			} else {
				$state['asset'] = &$GLOBALS['SQ_SYSTEM']->am->getAsset($state['assetid']);
			}
		}

		// verify the fieldid
		$success = $mm->isMetadataFieldAssetid($settings['fieldid']);
		if (!$success) return FALSE;

		// get the asset
		$asset =& $state['asset'];

		// get the current metadata values
		$metadata = $mm->getMetadata($asset->id);

		// get the field name
		$field_name = '';
		if (isset($metadata[$settings['fieldid']])) {
			$field_name = $metadata[$settings['fieldid']][0]['name'];
		}

		// Sanitise the value for equal signs (=)
		// As per Bug #2951 HTML Submitted becomes malformed due to a simple equals sign
		$value = str_replace('=', '\=', $value);

		// set the new value
		$metadata[$settings['fieldid']] = Array(
											0	=>	Array(
														'name'	=> $field_name,
														'value'	=> $value,
													)
										  );

		$success = $mm->setMetadata($asset->id, $metadata);

		// Regenerate Metadata File
		if ($success) $mm->regenerateMetadata($asset->id);

		// all done very smoothly like an ice cream
		if ($success) {
			return Array(
					'fieldid'		=> $settings['fieldid'],
					'field_name'	=> $field_name,
					'value'			=> $value,
				   );
		} else {
			return FALSE;
		}

	}//end execute()


	/**
	* Get the HTML editing interface for this action. Returns the string instead of printing it
	*
	* @param array		$settings		settings that this condition saves in processInterface()
	* @param string		$prefix			unique prefix
	* @param boolean	$write_access	determines whether the interface is editable
	*
	* @return boolean
	* @access public
	*/
	public static function getInterface($settings, $prefix, $write_access=FALSE)
	{
		$metadata_fieldid		= array_get_index($settings, 'fieldid', 0);
		$metadata_field_value	= array_get_index($settings, 'value', NULL);

		// asset finder for Metadata Field
		ob_start();
			if ($write_access) {
				echo translate('trigger_metadata_date_select_field');
				asset_finder($prefix.'[fieldid]', $metadata_fieldid, Array('metadata_field' => 'D'));
			} else {
				echo translate('trigger_metadata_date_show_field');
				echo $metadata_fieldid;
			}
			echo '<br/>';

			$disabled = ($write_access ? '' : 'disabled=disabled');
			ob_start();
			text_box($prefix.'[offset_months]', array_get_index($settings, 'offset_months', 0), 5, 2, FALSE, $disabled);
			echo '|';
			text_box($prefix.'[offset_days]', array_get_index($settings, 'offset_days', 0), 5, 2, FALSE, $disabled);
			echo '|';
			text_box($prefix.'[offset_hours]', array_get_index($settings, 'offset_hours', 0), 5, 2, FALSE, $disabled);
			echo '|';
			text_box($prefix.'[offset_mins]', array_get_index($settings, 'offset_mins', 0), 5, 2, FALSE, $disabled);
			list($text_box_months, $text_box_days, $text_box_mins, $text_box_hours) = explode('|', ob_get_clean());
			echo translate('trigger_metadata_date_offset', $text_box_months, $text_box_days, $text_box_mins, $text_box_hours);
		$output = ob_get_clean();

		return $output;

	}//end getInterface()


	/**
	* Function that handles the conversion of interface to settings
	* together with settings it is expected to populate the hash object
	*
	* @param array	&$settings		a container for any data the action might want to save
	* @param string	$request_data	array of data corresponding to this action as
	*								specified by the $prefix in getInterface.
	*
	* @return boolean
	* @access public
	*/
	public static function processInterface(&$settings, $request_data)
	{
		// process fieldid
		$fieldid_data = array_get_index($request_data, 'fieldid', 0);
		$fieldid = $fieldid_data['assetid'];

		if ($fieldid) {
			$settings['fieldid'] = $fieldid;
			foreach (Array('offset_months', 'offset_days', 'offset_hours', 'offset_mins') as $field_name) {
				$settings[$field_name] = array_get_index($request_data, $field_name, '0');
			}

			$metadata_field =& $GLOBALS['SQ_SYSTEM']->am->getAsset($fieldid);

			// Can we edit the destination field? If we can't then what's the point;
			// we can't even change it through a trigger
			if (!$metadata_field->attr('editable')) {
				trigger_localised_error('CORE0246', E_USER_WARNING, $metadata_field->name, $metadata_field->id);
				return FALSE;
			}

			$settings['field_name']	= $metadata_field->name;
		}

		return FALSE;

	}//end processInterface()


	/**
	* Get the list of locks that the action needs acquire before executing
	*
	* @param array	$settings	the stored settings for this action
	* @param array	&$state		the state that describes the asset the action is executing on
	*
	* @return array
	* @access public
	*/
	public static function getLocks($settings, &$state)
	{
		return Array($state['assetid'] => Array('all'));

	}//end getLocks()


}//end class

?>