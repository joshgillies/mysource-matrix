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
* $Id: system_move_update.php,v 1.13 2006/12/06 05:39:51 bcaldwell Exp $
*
*/

/**
* Small script to be run AFTER the system root directory is changed
*
* @author  Blair Robertson <blair@squiz.net>
* @version $Revision: 1.13 $
* @package MySource_Matrix
*/
error_reporting(E_ALL);
$SYSTEM_ROOT = '';
// from cmd line
if ((php_sapi_name() == 'cli')) {
	if (isset($_SERVER['argv'][1])) $SYSTEM_ROOT = $_SERVER['argv'][1];
	$err_msg = "You need to supply the path to the System Root as the first argument\n";

} else {
	if (isset($_GET['SYSTEM_ROOT'])) $SYSTEM_ROOT = $_GET['SYSTEM_ROOT'];
	$err_msg = '
	<div style="background-color: red; color: white; font-weight: bold;">
		You need to supply the path to the System Root as a query string variable called SYSTEM_ROOT
	</div>
	';
}

if (empty($SYSTEM_ROOT) || !is_dir($SYSTEM_ROOT)) {
	trigger_error($err_msg, E_USER_ERROR);
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

?>
