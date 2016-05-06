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
* $Id: trigger_action_set_design_parse_file.inc,v 1.8 2012/08/30 01:09:14 ewang Exp $
*
*/


require_once SQ_FUDGE_PATH.'/general/file_system.inc';
require_once SQ_CORE_PACKAGE_PATH.'/system/triggers/trigger_action/trigger_action.inc';


/**
* Trigger Action Set Design Parse File
*
* A trigger action that sets the parse file for the design
*
*
* @author  Scott Kim <skim@squiz.net>
* @version $Revision: 1.8 $
* @package MySource_Matrix_Packages
* @subpackage __core__
*/
class Trigger_Action_Set_Design_Parse_File extends Trigger_Action
{


	/**
	* Execute this action
	* Returns an array of data about what it did, or false on error
	*
	* The settings used by this action are in the form:<BR />
	* <PRE>
	* Array(
	*		'file_path'	   => string  // path to the file to use
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
		if (empty($settings['file_path'])) {
			return FALSE;
		}

		if (!authorised_file_path($settings['file_path'])) {
			trigger_localised_error('CORE0328', sprintf(translate('Access to the file "%s" is not authorised for this operation.'), htmlentities($settings['file_path'], ENT_COMPAT, SQ_CONF_DEFAULT_CHARACTER_SET)), E_USER_WARNING);
			return FALSE;
		}

		// check settings, state
		if (!file_exists($settings['file_path'])) {
			// if no settings, fail
			return FALSE;
		}

		// state, design asset
		if (empty($state['asset'])) {
			// grab the asset if assetid is given, but not the asset.
			if (empty($state['assetid'])) {
				return FALSE;
			} else {
				$state['asset'] = $GLOBALS['SQ_SYSTEM']->am->getAsset($state['assetid']);
			}
		}

		if (is_null($state['asset'])) return FALSE;

		$asset =& $state['asset'];
		$edit_fns = $asset->getEditFns();

		$parse_file  = $asset->data_path.'/parse.txt';
		$changes     = FALSE;
		$new_version = FALSE;

		$fv = $GLOBALS['SQ_SYSTEM']->getFileVersioning();

		// delete existing parse file if it exists
		if (is_file($parse_file)) {
			$new_version = TRUE;
			if (!unlink($parse_file)) {
				trigger_localised_error('CORE0164', translate('Failed deleting existing parse file'), E_USER_WARNING);
				return FALSE;
			}
		}

		// copy over the uploaded file
		if (copy_file($settings['file_path'], $parse_file)) {
			// tell the asset to update the customisation at the end of the interface processing
			$asset->_tmp['update_customisations'] = TRUE;
			$asset->_tmp['generate_design'] = TRUE;
			$changes = $edit_fns->parseAndProcessFile($asset);

			// parseAndProcessFile()
			$parse_file = $asset->data_path.'/parse.txt';
			$str = file_to_string($parse_file);
			$contents = $edit_fns->_parseString($asset, $str);

			$GLOBALS['SQ_SYSTEM']->setRunLevel(SQ_RUN_LEVEL_FORCED);
			$changes = (!is_null($contents) && $edit_fns->_processContents($asset, $contents) && $asset->saveAttributes());
			$GLOBALS['SQ_SYSTEM']->restoreRunLevel();

			// if we are overwriting our current parse file, we need to add a new version to the repository
			if ($new_version) {
				$file_status = $fv->upToDate($parse_file);
				if (FUDGE_FV_MODIFIED & $file_status) {
					if (!$fv->commit($parse_file, '')) {
						trigger_localised_error('CORE0160', translate('Failed committing file version'), E_USER_WARNING);
					}
				}
			} else {
				// attempt to add the parse file to the repository
				if (!$fv->add($asset->data_path_suffix, $parse_file, '')) {
					trigger_localised_error('CORE0157', translate('Failed adding parse file version'), E_USER_WARNING);
				}
			}
		}//end if file uploaded

		// make sure we have the latest version of our file
		if (!$fv->checkOut($asset->data_path_suffix.'/parse.txt', $asset->data_path)) {
			trigger_localised_error('CORE0158', translate('Failed checking out latest version of parse file'), E_USER_WARNING);
			return FALSE;
		}

		$GLOBALS['SQ_SYSTEM']->setRunLevel(SQ_RUN_LEVEL_FORCED);
		$asset->generateDesignFile(TRUE);
		$GLOBALS['SQ_SYSTEM']->restoreRunLevel();

		return Array(
				'assetid'	=> $state['asset']->id,
			   );

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
		echo 'Path to the associated file for the design: ';
		if ($write_access) {
			echo text_box($prefix.'[file_path]', $file_path, '50');
		} else {
			echo '<strong>"'.$file_path.'"</strong>';
		}
		return ob_get_clean();

	}//end getInterface()


	/**
	* Function that handles the conversion of interface to settings
	* together with settings it is expected to populate the hash object
	*
	* @param array	&$settings		a container for any data the action might want to save
	* @param string	$request_data	array of data corresponding to this action
	*								as specified by the $prefix in getInterface.
	*
	* @return boolean
	* @access public
	*/
	public static function processInterface(&$settings, $request_data)
	{
		$file_path = array_get_index($request_data, 'file_path', '');
		$file_path = trim($file_path);
		if ($file_path == '') {
			return 'file path is not specified.';
		}

		if (!authorised_file_path($file_path)) {
			trigger_localised_error('CORE0328', sprintf(translate('Access to the file "%s" is not authorised for this operation.'), htmlentities($file_path, ENT_COMPAT, SQ_CONF_DEFAULT_CHARACTER_SET)), E_USER_WARNING);
			$file_path = '';
		}
		$settings['file_path'] = $file_path;

		return FALSE;

	}//end processInterface()


	/**
	* Get the list of locks that the action needs to acquire before executing
	*
	* @param array	$settings	a container for any data the action might want to save
	* @param array	&$state		the state that describes the asset the action is executing on
	*
	* @return array
	* @access public
	*/
	public static function getLocks($settings, &$state)
	{
		return Array(
				$state['assetid']	=> Array(
										'attributes',
									   ),
			   );

	}//end getLocks()


}//end class

?>
