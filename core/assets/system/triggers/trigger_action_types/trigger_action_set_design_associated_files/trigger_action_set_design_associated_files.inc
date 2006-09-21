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
* $Id: trigger_action_set_design_associated_files.inc,v 1.1 2006/09/21 23:32:57 skim Exp $
*
*/


require_once SQ_FUDGE_PATH.'/general/file_system.inc';
require_once SQ_CORE_PACKAGE_PATH.'/system/triggers/trigger_action/trigger_action.inc';


/**
* Trigger Action Set Design Associated Files
*
* A trigger action that sets the associated files for the design
*
*
* @author  Scott Kim <skim@squiz.net>
* @version $Revision: 1.1 $
* @package MySource_Matrix_Packages
* @subpackage __core__
*/
class Trigger_Action_Set_Design_Associated_Files extends Trigger_Action
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
	function execute($settings, &$state)
	{
		// check settings, state
		if (empty($settings['file_path']) || !file_exists($settings['file_path'])) {
			// if no settings, fail
			return FALSE;
		}

		// state, design asset
		if (empty($state['asset'])) {
			// grab the asset if assetid is given, but not the asset.
			if (empty($state['assetid'])) {
				return FALSE;
			} else {
				$state['asset'] =& $GLOBALS['SQ_SYSTEM']->am->getAsset($state['assetid']);
			}
		}

		if (is_null($state['asset'])) return FALSE;

		$asset =& $state['asset'];
		$edit_fns = $asset->getEditFns();

		require_once SQ_FUDGE_PATH.'/general/file_system.inc';

		$changes = FALSE;

		// if this is a tar archive, extract it
		if (preg_match('/\\.tar\\.gz$/', $settings['file_path'])
			|| preg_match('/\\.tgz$/', $settings['file_path'])
			|| preg_match('/\\.tar$/', $settings['file_path']) ) {

			require_once 'Archive/Tar.php';
			$tar_ball =& new Archive_Tar($settings['file_path']);
			if (($contents = $tar_ball->listContent()) != 0) {

				// Basically we want to make sure that all the files in the
				// tar ball end up in a single directory, so we have to mess about with the extracting a bit
				$extract_list = Array();
				foreach ($contents as $entry) {
					if ($entry['typeflag'] != '0' && $entry['typeflag'] != '') {
						continue;
					}

					$k = dirname($entry['filename']);
					if (!isset($extract_list[$k])) {
						$extract_list[$k] = Array();
					}
					$extract_list[$k][] = $entry['filename'];

				}//end foreach

				foreach ($extract_list as $remove_path => $files) {
					// extract the files from the tar archive to a temporary directory
					$files_dir = $asset->data_path.'/.temp_files';
					if (!create_directory($files_dir)) return FALSE;
					if (!clear_directory($files_dir)) return FALSE;

					$result = $tar_ball->extractList($files, $files_dir, $remove_path);
					if (PEAR::isError($result)) {
						trigger_error($result->getMessage().'<br/>'.$result->getUserInfo(), E_USER_WARNING);
						break;
					} else {
						$GLOBALS['SQ_SYSTEM']->am->includeAsset('file');
						$design_link = Array('asset' => &$asset, 'link_type' => SQ_LINK_TYPE_2, 'value' => '', 'sort_order' => 1, 'is_dependant' => 1);

						foreach ($files as $filename) {
							$filename = preg_replace('|^'.$remove_path.'/|', '', $filename);
							$temp_path = $files_dir.'/'.$filename;
							$temp_info = Array('name' => $filename, 'tmp_name' => $temp_path);
							$temp_info['non_uploaded_file'] = TRUE;
							$GLOBALS['SQ_SYSTEM']->setRunLevel(SQ_RUN_LEVEL_FORCED);
							if ($edit_fns->_processUploadedFile($asset, $temp_info)) {
								$changes = TRUE;
							}
							$GLOBALS['SQ_SYSTEM']->restoreRunLevel();
						}
					}
					delete_directory($files_dir);

				}//end foreach

			}//end if

		// just create the file asset
		} else {
			$GLOBALS['SQ_SYSTEM']->setRunLevel(SQ_RUN_LEVEL_FORCED);
			if ($edit_fns->_processUploadedFile($asset, $info)) {
				$changes = TRUE;
			}
			$GLOBALS['SQ_SYSTEM']->restoreRunLevel();

		}//end if tar archive

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
	function getInterface($settings, $prefix, $write_access=FALSE)
	{
		$file_path = array_get_index($settings, 'file_path', '');
		ob_start();
		?>
		<table border="0">
			<tr>
				<td valign="top"><b><?php echo 'Path to the parse file'; ?></b></td>
				<td valign="top" style="padding-bottom: 10px;"><?php
					// level
					if ($write_access) {
						echo text_box($prefix.'[file_path]', $file_path, '50');
					} else {
						echo $file_path.'&nbsp;';
					}
				?>
				</td>
			</tr>
		</table>
		<?php

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
	function processInterface(&$settings, $request_data)
	{
		$file_path = array_get_index($request_data, 'file_path', '');
		$file_path = trim($file_path);
		if ($file_path == '') {
			return 'file path is not specified.';
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
	function getLocks($settings, &$state)
	{
		return Array(
				$state['assetid']	=> Array(
										'attributes',
									   ),
			   );

	}//end getLocks()


}//end class

?>