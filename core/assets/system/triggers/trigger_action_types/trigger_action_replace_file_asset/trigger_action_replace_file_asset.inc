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
* $Id: trigger_action_replace_file_asset.inc,v 1.8 2012/08/30 01:09:13 ewang Exp $
*
*/

require_once SQ_LIB_PATH.'/html_form/html_form.inc';
require_once SQ_INCLUDE_PATH.'/general_occasional.inc';
require_once SQ_CORE_PACKAGE_PATH.'/system/triggers/trigger_action/trigger_action.inc';

/**
* Trigger Action Reaplce File Asset
*
* A trigger action that replaces an existing file-based asset
*
*
* @author  Avi Miller <avi.miller@squiz.net>
* @version $Revision: 1.8 $
* @package MySource_Matrix_Packages
* @subpackage __core__
*/
class Trigger_Action_Replace_File_Asset extends Trigger_Action
{


	/**
	* Execute this action
	* Returns an array of data about what it did, or false on error
	*
	* The settings used by this action are in the form:<BR />
	* <PRE>
	* Array(
	*		'file_path'	   => string  // path to the file to use
	*      );
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
		// check required settings
		if (empty($settings['file_path'])) return FALSE;
		if (!is_readable($settings['file_path'])) {
			trigger_localised_error('CORE0276', E_USER_WARNING, htmlentities($settings['file_path'], ENT_COMPAT, SQ_CONF_DEFAULT_CHARACTER_SET));
			return FALSE;
		}

		if (empty($state['asset'])) {
			// grab the asset if assetid is given but not the asset.
			if (empty($state['assetid'])) {
				return FALSE;
			} else {
				$state['asset'] = $GLOBALS['SQ_SYSTEM']->am->getAsset($state['assetid']);
			}
		}

		if (is_null($state['asset'])) return FALSE;
		if (!($state['asset'] instanceof File)) {
			trigger_error('Unable to replace file content for a '.$state['asset']->type().' asset', E_USER_NOTICE);
			return FALSE;
		}

		$edit_fns = $state['asset']->getEditFns();
		$lock_acquired = ($GLOBALS['SQ_SYSTEM']->am->acquireLock($state['asset']->id, 'attributes') == 1);

		$info = Array(
					'name'				=> basename($settings['file_path']),
					'tmp_name'			=> $settings['file_path'],
					'non_uploaded_file'	=> TRUE,
				);
		$o = NULL; // as long as we don't try to redirect, this will be OK
		$res = $edit_fns->processFileUpload($state['asset'], $o, '', $info, FALSE);
		if ($lock_acquired) {
			$GLOBALS['SQ_SYSTEM']->am->releaseLock($state['asset']->id, 'attributes');
		}
		return $res ? $info : FALSE;

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
		$file_path = array_get_index($settings, 'file_path', '');
		ob_start();
			if ($write_access) {
				text_box($prefix.'[file_path]', $file_path, 40);
			} else {
				echo '<b>"'.$file_path.'"</b>';
			}
		$value = ob_get_clean();
		$res = '<div class="sq-break-line-inputs">'.sprintf(translate('trigger_replace_file'), $value).'</div>';
		if ($write_access) {
			$res .= '<p>'.translate('trigger_replace_file_type_warning').'</p>';
		}
		return $res;

	}//end getInterface()


	/**
	* Handle the conversion of interface to settings
	* together with settings it is expected to populate the hash object
	*
	* @param array	&$settings		a container for any data the action might want to save
	* @param string	$request_data	array of data corresponding to this action as specified
	*								by the $prefix in getInterface.
	*
	* @return boolean
	* @access public
	*/
	public static function processInterface(&$settings, $request_data)
	{
		if (!empty($request_data['file_path'])) {
			$settings['file_path'] = $request_data['file_path'];
		}
		return FALSE;

	}//end processInterface()


}//end class

?>
