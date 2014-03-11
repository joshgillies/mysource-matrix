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
*
*/

/**
* To allow Matrix to be easily moved around file system, we have to use relative path in Matrix 5.
* This script converts configured absolute path to relative path.
*
*/
error_reporting(E_ALL);
if (ini_get('memory_limit') != '-1') ini_set('memory_limit', '-1');
$SYSTEM_ROOT = '';
// from cmd line
if ((php_sapi_name() == 'cli')) {
	if (isset($_SERVER['argv'][1])) $SYSTEM_ROOT = $_SERVER['argv'][1];
	$err_msg = "You need to supply the path to the System Root as the first argument\n";

} else {
	trigger_error("You can only run this script from the command line\n", E_USER_ERROR);
}

if (empty($SYSTEM_ROOT)) {
	echo $err_msg;
	exit();
}

if (!is_dir($SYSTEM_ROOT) || !is_readable($SYSTEM_ROOT.'/core/include/init.inc')) {
	echo "ERROR: Path provided doesn't point to a Matrix installation's System Root. Please provide correct path and try again.\n";
	exit();
}

require_once $SYSTEM_ROOT.'/core/include/init.inc';

echo "Updating assets to use relative path.\n";


// remove unfinished HIPO jobs. They might contain file path
remove_hipo_jobs();

// upgrade import configs
upgrade_import_tool_configs();

// update form actions that uses absolute path
update_form_action_filepaths();

// form submission file uploaded
update_form_submission_filepaths();


echo "Done.\n";




function remove_hipo_jobs(){
    		$GLOBALS['SQ_SYSTEM']->changeDatabaseConnection('db2');
		$GLOBALS['SQ_SYSTEM']->doTransaction('BEGIN');
		$sql = 'DELETE FROM sq_hipo_job';
		$query = MatrixDAL::preparePdoQuery($sql);
		$result = MatrixDAL::execPdoQuery($query);
		$GLOBALS['SQ_SYSTEM']->doTransaction('COMMIT');
		$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();
		if($result)
		    echo "Removed unfinished HIPO jobs.\n";
}


function upgrade_import_tool_configs(){
    	$root_user =& $GLOBALS['SQ_SYSTEM']->am->getSystemAsset('root_user');
	$GLOBALS['SQ_SYSTEM']->setCurrentUser($root_user);
	$GLOBALS['SQ_SYSTEM']->setRunLevel(SQ_RUN_LEVEL_FORCED);
	
	 // update cms export path   
	$import_manager = $GLOBALS['SQ_SYSTEM']->am->getSystemAsset('import_tools_manager');    
	$cms_export_dir = $import_manager->attr('cms_file_export_dir');
	if(!empty($cms_export_dir) && strpos($cms_export_dir, '/') === 0) {
	    $cms_export_dir_new = str_replace(SQ_SYSTEM_ROOT.'/', '', $cms_export_dir);
	    if($cms_export_dir_new !== $cms_export_dir) {
		$import_manager->setAttrValue('cms_file_export_dir', $cms_export_dir_new);
		$import_manager->saveAttributes();
		 echo "Updated Import Tools Manager CMS Export Path\n";
	    }
	    else {
		echo "Absolute path found in form Import Tools Manager CMS FIle Export Dir. It can not be automatically converted to relative path. Location: $cms_export_dir\n";
	    }
	}
	
	// update STRUCTURED_FILE_IMPORT_DIR and BULK_FILE_IMPORT_DIR
	require_once (SQ_FUDGE_PATH.'/general/file_system.inc');
	$config_file = file_to_string(SQ_DATA_PATH.'/private/conf/import_tools_manager.inc');
	if(strpos($config_file, SQ_SYSTEM_ROOT.'/') !== FALSE) {
	    $config_file = str_replace(SQ_SYSTEM_ROOT.'/', '', $config_file);
	    string_to_file($config_file, SQ_DATA_PATH.'/private/conf/import_tools_manager.inc');
	    echo "Updated Import Tools Manager Structed FIle Import and Bulk FIle Import Path\n";
	}
	
	// update word import tool converter
	$converters =  $GLOBALS['SQ_SYSTEM']->am->getChildren('1', 'import_tool_converter_word');
	$converter = $GLOBALS['SQ_SYSTEM']->am->getAsset(array_pop(array_keys($converters)));
	$upload_directory = $converter->attr('upload_directory');
	if(!empty($upload_directory) && strpos($upload_directory, '/') === 0) {
	    $upload_directory_new = str_replace(SQ_SYSTEM_ROOT.'/', '', $upload_directory);
	    if($upload_directory_new !== $upload_directory) {
		$converter->setAttrValue('upload_directory', $upload_directory_new);
		$converter->saveAttributes();
		 echo "Updated Import Tools Manager Word Converter Path\n";
	    }
	    else {
		echo "Absolute path found in form Import Tools Manager Word Converter. It can not be automatically converted to relative path. Location: $upload_directory\n";
	    }
	}
	
	// update file bridge config
	$config_file = file_to_string(SQ_DATA_PATH.'/private/conf/file_bridge.inc');
	if(strpos($config_file, SQ_SYSTEM_ROOT.'/') !== FALSE) {
	    $config_file = str_replace(SQ_SYSTEM_ROOT.'/', '', $config_file);
	    string_to_file($config_file, SQ_DATA_PATH.'/private/conf/file_bridge.inc');
	    echo "Updated File Bridge Path\n";
	}
	
	
    	$GLOBALS['SQ_SYSTEM']->restoreRunLevel();
	$GLOBALS['SQ_SYSTEM']->restoreCurrentUser();
}


function update_form_action_filepaths(){
	
	$root_user =& $GLOBALS['SQ_SYSTEM']->am->getSystemAsset('root_user');
	$GLOBALS['SQ_SYSTEM']->setCurrentUser($root_user);
	$GLOBALS['SQ_SYSTEM']->setRunLevel(SQ_RUN_LEVEL_FORCED);
	
	// a quick query to find out all forms that need update
	$sql = "SELECT DISTINCT a.assetid FROM sq_ast a, sq_ast_attr_val v, sq_ast_attr t WHERE a.assetid = v.assetid AND t.attrid = v.attrid AND t.name = 'actions' AND a.type_code = 'form_email' AND v.custom_val like '%form_action_save_xml%' ";
	$rows = MatrixDAL::executeSqlAll($sql);
	foreach ($rows as $child_data) {
		$child_id = $child_data['assetid'];
		$asset = $GLOBALS['SQ_SYSTEM']->am->getAsset($child_id);
		$data = $asset->attr('actions');
		$changed = FALSE;
		if (!empty($data)) {
			foreach ($data as $index => $action) {
				if(isset($action['type_code']) && $action['type_code'] === 'form_action_save_xml') {
				    if(isset($action['settings']['save_xml']['location']) && !empty($action['settings']['save_xml']['location'])) {
					$location = $action['settings']['save_xml']['location'];
					if(strpos($location, '/') === 0) {
					    if(strpos($location, SQ_SYSTEM_ROOT.'/') === 0) {
						$location = str_replace(SQ_SYSTEM_ROOT.'/', '', $location);
						$data[$index]['settings']['save_xml']['location'] = $location;
						$changed = TRUE;
					    }
					    else {
						echo "Absolute path found in form action #$child_id. It can not be automatically converted to relative path. Location: $location\n";
					    }
					}
				    }
				}
			}
		}
		if($changed) {
		    echo "Updated Form ID: $asset->id\n";
		    $asset->setAttrValue('actions', $data);
		    $asset->saveAttributes();
		}
		$GLOBALS['SQ_SYSTEM']->am->forgetAsset($asset, true);
		unset($asset);
	}
		
	$GLOBALS['SQ_SYSTEM']->restoreRunLevel();
	$GLOBALS['SQ_SYSTEM']->restoreCurrentUser();
}




function update_form_submission_filepaths(){
	
	$root_user =& $GLOBALS['SQ_SYSTEM']->am->getSystemAsset('root_user');
	$GLOBALS['SQ_SYSTEM']->setCurrentUser($root_user);
	$GLOBALS['SQ_SYSTEM']->setRunLevel(SQ_RUN_LEVEL_FORCED);
	
	// a quick query to find out all form submissions that needs update
	$sql = "SELECT DISTINCT a.assetid FROM sq_ast a, sq_ast_attr_val v, sq_ast_attr t WHERE a.assetid = v.assetid AND t.attrid = v.attrid AND t.name = 'attributes' AND a.type_code = 'form_submission' AND v.custom_val like '%filesystem_path%' ";
	$rows = MatrixDAL::executeSqlAll($sql);
	$fields_to_check = array('temp_filesystem_path','filesystem_path');
	foreach ($rows as $child_data) {
		$child_id = $child_data['assetid'];
		$asset = $GLOBALS['SQ_SYSTEM']->am->getAsset($child_id);
		$data = $asset->attr('attributes');
		if (isset($data['answers'])) {
			foreach (array_keys($data['answers']) as $question_id) {
				$extra_data = $asset->getExtraData($question_id);
				$record_changed = FALSE;
				foreach ($fields_to_check as $field) {
					if (!empty($extra_data[$field])) {
						$location = $extra_data[$field];
						if(strpos($location, '/') === 0) {
						    if(strpos($location, SQ_SYSTEM_ROOT.'/') === 0) {
							$location = str_replace(SQ_SYSTEM_ROOT.'/', '', $location);
							$extra_data[$field]  = $location;
							$record_changed = TRUE;
						    }
						    else {
							echo "Absolute path found in form submission #$child_id, but can not be automatically converted. Location: $location\n";
						    }
						}
					}
				 }
				if ($record_changed) {
					if ($asset->setExtraData($question_id, $extra_data)){
						$asset->saveAttributes();
						echo "Updated Form Submission ID: $asset->id\n";
					} else {
						echo "Failed to update Form Submission ID: $asset->id\n";
					}
				}
			}
		}
		$GLOBALS['SQ_SYSTEM']->am->forgetAsset($asset, true);
		unset($asset);
	}
		
	$GLOBALS['SQ_SYSTEM']->restoreRunLevel();
	$GLOBALS['SQ_SYSTEM']->restoreCurrentUser();
}



?>