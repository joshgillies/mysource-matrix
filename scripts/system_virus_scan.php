#!/usr/bin/php
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
* $Id: system_virus_scan.php,v 1.4 2012/08/30 01:04:53 ewang Exp $
*
*/

/**
* Script to check the system for viruii 
*
* @author  Benjamin Pearson <bpearson@squiz.net>
* @version $Revision: 1.4 $
* @package MySource_Matrix
*/
error_reporting(E_ALL);
if ((php_sapi_name() != 'cli')) trigger_error("You can only run this script from the command line\n", E_USER_ERROR);

$SYSTEM_ROOT = (isset($_SERVER['argv'][1])) ? $_SERVER['argv'][1] : '';
if (empty($SYSTEM_ROOT)) {
	echo "ERROR: You need to supply the path to the System Root as the first argument\n";
	exit();
}

if (!is_dir($SYSTEM_ROOT) || !is_readable($SYSTEM_ROOT.'/core/include/init.inc')) {
	echo "ERROR: Path provided doesn't point to a Matrix installation's System Root. Please provide correct path and try again.\n";
	exit();
}

require_once $SYSTEM_ROOT.'/core/include/init.inc';
require_once(SQ_FUDGE_PATH.'/general/file_system.inc');
require_once(SQ_FUDGE_PATH.'/antivirus/antivirus.inc');

// Scan the files for viruii
$report = '';
$recursive = TRUE;
$result = Antivirus::scan_file($SYSTEM_ROOT.'/data', $report, $recursive);

// If a virus is found, scan the file for the sucker!
if (!$result) {
	$virus_count = 0;
	$count = 0;
	foreach ($report as $line_no => $line) {
		// Scan each line
		if (preg_match('/FOUND/', $line)) {
			list($file, $virus_info) = explode(':', $line);
		}//end if
		if (preg_match('/Infection/', $line)) {
			list($file, $virus_info) = explode('Infection', $line);
			if (!empty($file)) {
				if (strpos($file, '->') !== FALSE) {
					// Virus was found in an archive
					$file_parts = explode('->', $file);
					$file = $file_parts[0];
				}//end if
			}//end if
		}//end if
		if (preg_match('/^Infected/', $line)) {
			list($info, $infected_files) = explode(':', $line);
			if (!empty($infected_files)) {
				$virus_count = $infected_files;
			}//end if
		}//end if

		// Found a virus on this file, let's operate on it!
		if (isset($file)) {
			$count++;
			if (is_file_versioning($file)) {
				$fv = $GLOBALS['SQ_SYSTEM']->getFileVersioning();
				$fv->remove($file);
			} else {
				// Don't think this is part of file versioning
				// So we must delete this file (after all it is infected)
				unlink($file);
			}//end if

			// All finished, clear it
			unset($file);
		}//end if
	}//end foreach

	// Display summary
	if ($count == $virus_count) {
		echo 'Found '.$virus_count." Infected files\n";
	} else {
		echo "An error occurred\n";
	}//end if
}//end if


/**
 * Check if a file is part of file versioning
 *
 * @param string	$real_file	The file to check
 *
 * @return boolean
 * @access public
 */
function is_file_versioning($real_file)
{
	$ffv_dir = dirname($real_file).'/.FFV';
	if (!is_dir($ffv_dir)) return FALSE;

	$ffv_file = $ffv_dir.'/'.basename($real_file);
	if (!is_file($ffv_file)) {
		return FALSE;
	}

	return TRUE;
}//end is_file_versioning()

?>
