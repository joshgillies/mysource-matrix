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
* $Id: trigger_action_set_metadata_value.inc,v 1.5 2006/02/07 02:27:12 sdanis Exp $
*
*/

require_once SQ_CORE_PACKAGE_PATH.'/system/triggers/trigger_action/trigger_action.inc';

/**
* Trigger_Action_Set_Metadata_Value
*
* A trigger action that applies or denies a metadata value
*
* @author  Scott Kim <skim@squiz.net>
* @version $Revision: 1.5 $
* @package MySource_Matrix_Packages
* @subpackage __core__
*/
class Trigger_Action_Set_Metadata_Value extends Trigger_Action
{


	/**
	* Execute this action
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
	function execute($settings, &$state)
	{
		$mm =& $GLOBALS['SQ_SYSTEM']->getMetadataManager();

		// check required settings
		if (empty($settings['fieldid'])) return FALSE;
		if (empty($settings['value'])) return FALSE;
		if (empty($settings['field_name'])) return FALSE;
		if (is_null($state['asset'])) return FALSE;

		// verify the fieldid
		$success = $mm->isMetadataFieldAssetid($settings['fieldid']);
		if (!$success) return FALSE;

		// get the asset
		$asset =& $state['asset'];

		// get the current metadata values
		$metadata = $mm->getMetadata($asset->id);

		$value = '';
		$value_component = Array();

		// set the new value
		$metadata[$settings['fieldid']] = Array(
											'name'	=> $settings['field_name'],
											'value'	=> $settings['value'],
										  );
		$success = $mm->setMetadata($asset->id, $metadata);

		// all done very smoothly like an ice cream
		if ($success) {
			return Array(
					'fieldid'		=> $settings['fieldid'],
					'field_name'	=> $settings['field_name'],
					'value'			=> $settings['value'],
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
	function getInterface($settings, $prefix, $write_access=FALSE)
	{
		$output					= '';
		$metadata_fieldid		= array_get_index($settings, 'fieldid', 0);
		$metadata_field_value	= array_get_index($settings, 'value', '');

		// asset finder for Metadata Field
		ob_start();
			if ($write_access) {
				echo 'Choose Metadata Field&nbsp;&nbsp;&nbsp;';
				asset_finder($prefix.'[fieldid]', $metadata_fieldid, Array('metadata_field' => 'D'));
			} else {
				echo '<b>No metadata field specified</b>';
			}
			$output_part1 = ob_get_contents();
		ob_end_clean();
		$output .= $output_part1;

		// Input field for the value
		ob_start();
			if ($write_access && $metadata_fieldid) {
				echo '<br>Set the value for this field<br>';
				// set the field name
				$metadata_field =& $GLOBALS['SQ_SYSTEM']->am->getAsset($metadata_fieldid);
				$edit_fns = $metadata_field->getEditFns();
				$null = NULL;
				$edit_fns->paintValueInterface($metadata_field, $null, $metadata_field_value, TRUE);
				hidden_field($prefix.'[form_printed]', 1);
			}
			$output_part2 = ob_get_contents();
		ob_end_clean();
		$output .= $output_part2;

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
	function processInterface(&$settings, $request_data)
	{
		// process fieldid
		$fieldid_data = array_get_index($request_data, 'fieldid', 0);
		$form_printed = array_get_index($request_data, 'form_printed', 0);

		$fieldid = $fieldid_data['assetid'];
		$value_accepted = array_get_index($request_data, 'value_accepted', 0);

		if ($fieldid) {
			$settings['fieldid'] = $fieldid;

			$metadata_field =& $GLOBALS['SQ_SYSTEM']->am->getAsset($fieldid);

			// Can we edit the destination field? If we can't then what's the point;
			// we can't even change it through a trigger
			if (!$metadata_field->attr('editable')) {
				trigger_localised_error('CORE0246', E_USER_WARNING, $metadata_field->name, $metadata_field->id);
				return FALSE;
			}

			$edit_fns = $metadata_field->getEditFns();

			$result = '';
			if ($form_printed) {
				$edit_fns->processInlineValueInterface($metadata_field, $result);
			}

			$settings['field_name']	= $metadata_field->name;
			$settings['value']		= $result;
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
	function getLocks($settings, &$state)
	{
		return Array($state['assetid'] => Array('all'));

	}//end getLocks()


}//end class

?>
