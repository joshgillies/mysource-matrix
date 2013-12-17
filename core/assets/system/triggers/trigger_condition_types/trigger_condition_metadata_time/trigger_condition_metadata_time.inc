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
* $Id: trigger_condition_metadata_time.inc,v 1.2 2012/08/30 01:09:15 ewang Exp $
*
*/

require_once SQ_CORE_PACKAGE_PATH.'/system/triggers/trigger_condition/trigger_condition.inc';

/**
* Trigger_Condition_Metadata_Time
*
* A condition to filter Metadata Time events by the metatdafield they carry
*
* @author  Geoffroy Noel <gnoel@squiz.co.uk>
* @version $Revision: 1.2 $
* @package MySource_Matrix_Packages
* @subpackage __core__
*/
class Trigger_Condition_Metadata_Time extends Trigger_Condition
{

	/**
	* Evaluate this condition
	*
	* The settings used by this condition are in the form:
	* <PRE>
	* Array(
	*       'trigger_metadata_date_fieldid'	=> string	// the assetid of the metadata field we need to filter on
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
		if (empty($settings['trigger_metadata_date_fieldid'])) return FALSE;
		if (empty($state['asset'])) return FALSE;

		$ret = FALSE;

		$mdata_fieldid = isset($state['event']['data']['trigger_metadata_date_fieldid']) ? $state['event']['data']['trigger_metadata_date_fieldid'] : '';

		if (!empty($mdata_fieldid) && ($settings['trigger_metadata_date_fieldid'] == $mdata_fieldid)) {
			$ret = TRUE;
		}

		return $ret;

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
		require_once SQ_LIB_PATH.'/html_form/html_form.inc';

		// mdata field
		ob_start();
			echo translate('trigger_metadata_date_fieldid');
			$fieldid = array_get_index($settings, 'trigger_metadata_date_fieldid', 0);
			if ($write_access) {
				asset_finder($prefix.'[trigger_metadata_date_fieldid]', $fieldid, Array('metadata_field_date' => 'D'));
			} else {
				if (!empty($fieldid)) {
					echo get_asset_tag_line($fieldid);
				} else {
					echo '<em>None selected</em>';
				}
				//echo '<b>#'.$fieldid.'</b>';
			}
		$ret = ob_get_clean();

		return $ret;

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
			return translate('trigger_input_data_error');
		}

		$settings['trigger_metadata_date_fieldid'] = empty($request_data['trigger_metadata_date_fieldid']['assetid']) ? 0 : $request_data['trigger_metadata_date_fieldid']['assetid'];

		return FALSE;

	}//end processInterface()


}//end class

?>