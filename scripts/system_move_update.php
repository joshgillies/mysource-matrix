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
* $Id: system_move_update.php,v 1.22.2.1 2013/10/16 03:16:06 ewang Exp $
*
*/

/**
* Small script to be run AFTER the system root directory is changed
*
* @author  Blair Robertson <blair@squiz.net>
* @version $Revision: 1.22.2.1 $
* @package MySource_Matrix
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

// simple fn to print a prompt and return what the user enters
function get_line($prompt='')
{
	echo $prompt;
	// now get their entry and remove the trailing new line
	return rtrim(fgets(STDIN, 4094));

}//end get_line()


require_once $SYSTEM_ROOT.'/core/include/init.inc';

if ((php_sapi_name() == 'cli')) {
	if (isset($_SERVER['argv'][2])) {
		$old_system_root = rtrim(trim($_SERVER['argv'][2]), '/');
		if (strtolower(get_line('Confirm "'.$old_system_root.'" (Y/N) : ')) != 'y') {
			unset($old_system_root);
		}
	}
}

if (!isset($old_system_root)) {
	do {
		$old_system_root = get_line('Enter the old System Root : ');
	} while (strtolower(get_line('Confirm "'.$old_system_root.'" (Y/N) : ')) != 'y');
}

//$new_system_root = SQ_SYSTEM_ROOT;
// use user entered path (symbolic link friendly)
$new_system_root = $SYSTEM_ROOT;

require_once SQ_FUDGE_PATH.'/general/file_system.inc';
function recurse_find_ffv_files($dir, $old_rep_root, $new_rep_root)
{
	$d = dir($dir);
	while (false !== ($entry = $d->read())) {
		if ($entry == '.' || $entry == '..') continue;

		// if this is a directory
		if (is_dir($dir.'/'.$entry)) {
			// we have found a .FFV dir
			if ($entry == '.FFV') {

				$ffv_dir = $dir.'/'.$entry;
				$ffv_d = dir($ffv_dir);
				while (false !== ($ffv_entry = $ffv_d->read())) {
					if ($ffv_entry == '.' || $ffv_entry == '..') continue;

					// if this is a directory
					if (is_file($ffv_dir.'/'.$ffv_entry)) {
						$ffv_file = $ffv_dir.'/'.$ffv_entry;
						$str = file_to_string($ffv_file);
						if ($str) {
							$str = str_replace('dir="'.$old_rep_root.'"', 'dir="'.$new_rep_root.'"', $str);
							echo "File : $ffv_file\n";
							#pre_echo("FILE : $ffv_file\n CONTENTS : \n$str");
							string_to_file($str, $ffv_file);
						}
					}
				}//end while
				$ffv_d->close();

			// just a normal dir, recurse
			} else {
				recurse_find_ffv_files($dir.'/'.$entry, $old_rep_root, $new_rep_root);

			}//end if
		}//end if
	}//end while
	$d->close();

}//end recurse_find_ffv_files()


$old_rep_path = preg_replace('|/+$|', '', $old_system_root).'/data/file_repository';
$new_rep_path = preg_replace('|/+$|', '', $new_system_root).'/data/file_repository';

pre_echo("OLD : $old_rep_path\nNEW : $new_rep_path");

recurse_find_ffv_files(SQ_DATA_PATH.'/private', $old_rep_path, $new_rep_path);
recurse_find_ffv_files(SQ_DATA_PATH.'/public', $old_rep_path, $new_rep_path);

$old_data_private_path = preg_replace('|/+$|', '', $old_system_root).'/data/';
$new_data_private_path = preg_replace('|/+$|', '', $new_system_root).'/data/';

recurse_data_dir_for_safe_edit_files(SQ_DATA_PATH.'/private', $old_data_private_path, $new_data_private_path);
recurse_data_dir_for_safe_edit_files(SQ_DATA_PATH.'/public', $old_data_private_path, $new_data_private_path);

//Bug #4560 Fix to take care of form_submission file paths stored in its attributes.
echo "Updating Form Submission file paths\n";
$new_root = preg_replace('|/+$|', '', $new_system_root);
$old_root = preg_replace('|/+$|', '', $old_system_root);
pre_echo("OLD : $old_root\nNEW : $new_root");
update_form_submission_filepaths($old_root, $new_root);

function recurse_data_dir_for_safe_edit_files($dir, $old_rep_root, $new_rep_root)
{
    $d = dir($dir);
	$index_to_look = Array ('data_path', 'data_path_public');
    while (false !== ($entry = $d->read())) {
        if ($entry == '.' || $entry == '..') continue;
        // if this is a directory
        if (is_dir($dir.'/'.$entry)) {
            // we have found a .sq_system dir
            if ($entry == '.sq_system') {
                $sq_system_dir = $dir.'/'.$entry;
                $sq_system_d = dir($sq_system_dir);
                while (false !== ($sq_system_entry = $sq_system_d->read())) {
                    if ($sq_system_entry == '.' || $sq_system_entry == '..' || $sq_system_entry != ".object_data") continue;
                    // if this is a directory
                    if (is_file($sq_system_dir.'/'.$sq_system_entry)) {
                        $sq_system_file = $sq_system_dir.'/'.$sq_system_entry;
                        $str = file_to_string($sq_system_file);
                        if ($str) {
							echo "File : $sq_system_file\n";
							preg_match ("/\"[A-Za-z_0-9]+\"/" ,$str , $asset_type);
							$GLOBALS['SQ_SYSTEM']->am->includeAsset(str_replace('"', '', $asset_type[0]));
							$content_array = unserialize($str);
							foreach ($index_to_look as $value) {
								$content_array->$value = str_replace($old_rep_root, $new_rep_root, $content_array->$value);
							}
							$str = serialize($content_array);
							string_to_file($str, $sq_system_file);
                        }
                    }
                }//end while
                $sq_system_d->close();
            // just a normal dir, recurse
            } else {
                recurse_data_dir_for_safe_edit_files($dir.'/'.$entry, $old_rep_root, $new_rep_root);

            }//end if
        }//end if
    }//end while
    $d->close();

}// end recurse_data_dir_for_safe_edit_files()

function update_form_submission_filepaths($old_root, $new_root){
	
	$root_user =& $GLOBALS['SQ_SYSTEM']->am->getSystemAsset('root_user');
	$GLOBALS['SQ_SYSTEM']->setCurrentUser($root_user);
	$GLOBALS['SQ_SYSTEM']->setRunLevel(SQ_RUN_LEVEL_FORCED);
	
	$children = $GLOBALS['SQ_SYSTEM']->am->getChildren(1, 'form_submission');
	foreach (array_keys($children) as $child_id) {
		$asset = $GLOBALS['SQ_SYSTEM']->am->getAsset($child_id);
		$data = $asset->attr('attributes');
		if (isset($data['answers'])) {
			foreach (array_keys($data['answers']) as $question_id) {
				$extra_data = $asset->getExtraData($question_id);
				if (!empty($extra_data['temp_filesystem_path'])) {
					$path = $extra_data['temp_filesystem_path'];
					$new_path = preg_replace("%$old_root%", $new_root, $path, 1);
					if ($new_path != NULL){
						if (strcmp($new_path, $path) != 0) {
							$extra_data['temp_filesystem_path'] = $new_path;
							if ($asset->setExtraData($question_id, $extra_data)){
								$asset->saveAttributes();
								echo "Updated Form Submission ID: $asset->id\n";
							}
						}
					} else {
						echo "Failed to update Form Submission ID: $asset->id\n";
					}
				} 
				if (!empty($extra_data['filesystem_path'])) {
					$path = $extra_data['filesystem_path'];
					$new_path = preg_replace("%$old_root%", $new_root, $path, 1);
					if ($new_path != NULL){
						if (strcmp($new_path, $path) != 0) {
							$extra_data['filesystem_path'] = $new_path;
							if ($asset->setExtraData($question_id, $extra_data)){
								$asset->saveAttributes();
								echo "Updated Form Submission ID: $asset->id\n";
							}
						}
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
