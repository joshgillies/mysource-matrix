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
* $Id: antivirus.inc,v 1.4 2012/08/30 00:56:51 ewang Exp $
*
*/


/**
* Antivirus
*
* Purpose
*     Antivirus functions
*
* @author  Benjamin Pearson <bpearson@squiz.net>
* @version $Revision: 1.4 $
* @package Fudge
* @subpackage Antivirus
*/
class Antivirus
{


	/**
	* Scan a file
	*
	* @param string		$path		Path for the file/directory to scan
	* @param string		$report		Return the report
	* @param boolean	$recursive	Scan recursively through directories
	*
	* @return boolean
	* @access public
	*
	*/
	public static function scan_file($path='', &$report, $recursive=FALSE)
	{

		// Default to OK
		$status = TRUE;
		// The check we are using a valid virus scanner
		$using_av = FALSE;
		$scanned = FALSE;

		// Load the virus checker
		require_once SQ_DATA_PATH.'/private/conf/tools.inc';

		// Scan the file
		if (!file_exists(SQ_TOOL_VIRUS_CHECKER_PATH)) {
				// Unable to scan the file
				// Warn the user sometime ho hum
		} else {

			if (empty($path)) {
				return $status;
			} else {
				$file = $path;
				}//end if

				// Check the file exists (it would be a bit dumb to scan nothing!)
				if (is_dir($file) || file_exists($file)) {
					$options = Array();
					switch (SQ_TOOL_VIRUS_CHECKER_TYPE) {
						case 'f-prot':
						$options = Array(
							'-old',
							);
						break;

						default:
						$options = Array();
						if ($recursive) $options[] = '-r';
					}//end switch

					// Scan the file
					$path_to_av = SQ_TOOL_VIRUS_CHECKER_PATH;
					foreach($options as $option) {
						$path_to_av .= ' '.$option;
					}//end foreach
					$command = $path_to_av.' '.escapeshellarg($file).' 2>&1';
					// Run the command
					exec($command, $av, $return);
					

					// Check the result was correct
					// That is, we are using a virus scanner
					// This is so we don't get false positives
					foreach ($av as $line_no => $line) {
						// Scan each line
						if (preg_match('/scanned/', $line)) {
							$scanned = TRUE;
						}//end if
					}//end foreach

					// Save the report to send back
					$report = $av;

					// Sift through the results
					// Flag if infected
					if ($return && $scanned) {
						$status = FALSE;
					}//end if
				}//end if
			}//end if
		// Return

		return $status;

	}//end scan_file()

}//end class

?>
