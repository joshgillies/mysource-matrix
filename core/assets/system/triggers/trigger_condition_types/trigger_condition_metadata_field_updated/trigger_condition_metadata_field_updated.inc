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
* $Id: trigger_condition_metadata_field_updated.inc,v 1.3 2012/08/30 01:09:15 ewang Exp $
*
*/

require_once SQ_CORE_PACKAGE_PATH.'/system/triggers/trigger_condition/trigger_condition.inc';

/**
* Trigger Condition
*
* Checks whether a metadata field has been updated
*
*
* @author  Benjamin Pearson <bpearson@squiz.net>
* @version $Revision: 1.3 $
* @package MySource_Matrix_Packages
* @subpackage __core__
*/
class Trigger_Condition_Metadata_Field_Updated extends Trigger_Condition
{


	/**
	* Evaluate this condition
	*
	* The settings used by this condition are in the form:
	* <PRE>
	* Array(
	*       'field'        => string   // The fieldid of the metadata field to match
	*       );
	* </PRE>
	*
	* @param array	$settings	the stored settings for this condition
	* @param array	&$state		the state that describes the asset the condition is executing on
	*
	* @return boolean
	* @access public
	*/
	public static function evaluate($settings, &$state)
	{
		if (!isset($settings['field'])) return FALSE;
		if (empty($settings['field'])) return FALSE;

		// No event data set
		if (!isset($state['event']['data']['fieldids'])) return FALSE;
		if (empty($state['event']['data']['fieldids'])) return FALSE;

		$fields_changed = $state['event']['data']['fieldids'];

		if (in_array($settings['field'], $fields_changed)) return TRUE;

		return FALSE;

	}//end evaluate()


	/**
	* Get the HTML editing interface for this condition. Returns the string instead of printing it
	*
	* @param array		$settings		settings that this condition save in processInterface()
	* @param string		$prefix			unique prefix
	* @param boolean	$write_access	write access flag
	*
	* @return boolean
	* @access public
	*/
	public static function getInterface($settings, $prefix, $write_access=FALSE)
	{
		// Get defaults
		$field = array_get_index($settings, 'field', 0);

		// Paint the interface
		ob_start();
			if ($write_access) {
				echo translate('Metadata Field').': ';

				echo asset_finder($prefix.'[metadata_field]', $field, Array('metadata_field'=>'D'));
				echo '<br /><br />';
				echo translate('<strong>Note:</strong> When this metadata field is updated, the condition will be met and the trigger will fire.');

			} else {
				if (empty($field)) {
					echo translate('No asset selected');

				} else {
					echo get_asset_tag_line($field);
				}//end if
			}//end if
			$contents = ob_get_contents();
		ob_end_clean();

		return $contents;

	}//end getInterface()


	/**
	* Function that handles the conversion of interface to settings
	*
	* @param array	&$settings		a container for any data the condition might want to save
	* @param array	$request_data	array of data corresponding to this condition
	*								as specified by the $prefix in the getInterface.
	*
	* @return mixed boolean|string
	* @access public
	*/
	public static function processInterface(&$settings, $request_data)
	{
		if (empty($request_data)) {
			return translate('There was an error with input data');

		}

		$options = array_get_index($request_data, 'metadata_field', Array());
		$settings['field'] = array_get_index($options, 'assetid', 0);

		return FALSE;

	}//end processInterface()


	/**
	* Determines whether a condition is allowed to be used multiple times in a trigger
	*
	* @return boolean
	* @access public
	*/
	public static function allowMultiple()
	{
		return TRUE;

	}//end allowMultiple()


}//end class

?>
