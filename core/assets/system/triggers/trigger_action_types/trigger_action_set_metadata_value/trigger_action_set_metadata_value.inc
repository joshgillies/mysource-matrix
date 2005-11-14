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
* $Id: trigger_action_set_metadata_value.inc,v 1.1 2005/11/14 23:42:06 skim Exp $
*
*/

require_once SQ_CORE_PACKAGE_PATH.'/system/triggers/trigger_action/trigger_action.inc';

/**
* Trigger_Action_Set_Metadata_Value
*
* A trigger action that applies or denies a metadata value
*
* @author  Scott Kim <skim@squiz.net>
* @version $Revision: 1.1 $
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
	*       'fieldid'	=> int    // metadata_field id
	*       'value'		=> string // the value to set
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
		if (empty($settings['fieldid'])) return false;
		if (empty($settings['value'])) return false;

		// verify the fieldid
		$success = $mm->isMetadataFieldAssetid($settings['fieldid']);
		if (!$success) return false;

		// set the default field value
		$success = $mm->setMetadataFieldDefaultValue($settings['fieldid'], $settings['value']);

		// all done very smoothly like an ice cream
		if ($success) {
			return Array(
					'fieldid'	=> $settings['fieldid'],
					'value'		=> $settings['value'],
				   );
		} else {
			return false;
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
	function getInterface($settings, $prefix, $write_access=false)
	{
		$output					= '';
		$metadata_fieldid		= array_get_index($settings, 'fieldid', 0);
		$metadata_field_value	= array_get_index($settings, 'value', '');


		// asset finder for Metadata Field
		ob_start();
			if ($write_access) {
				echo 'Choose Metadata Field ';
				asset_finder($prefix.'[fieldid]', $metadata_fieldid, Array('metadata_field' => 'D'));
			} else {
				echo '<b>No metadata field specified</b>';
			}
			$output_part1 = ob_get_contents();
		ob_end_clean();
		$output .= $output_part1;

		// Input field for the value
		ob_start();
			if ($write_access) {
				echo '&nbsp;&nbsp;Enter the value for this field ';
				text_box($prefix.'[value]', $metadata_field_value, 30);
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
		$fieldid = $fieldid_data['assetid'];
		if (empty($fieldid)) {
			return 'Metadata Field is not specified';
		} else {
			$settings['fieldid'] = $fieldid;

			// now we have fieldid, so find the value for the field
			$value = array_get_index($request_data, 'value', '');
			if (empty($value)) {
				return 'Metadata Field is not specified';
			} else {
				$settings['value'] = $value;
			}
		}

		return false;

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
