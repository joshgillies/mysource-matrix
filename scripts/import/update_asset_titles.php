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
* $Id: update_asset_titles.php,v 1.2 2012/08/30 01:04:53 ewang Exp $
*
*/

/**
* Assign file titles from a CSV file
* Command-line only
*
* @author  Mark Brydon <mbrydon@squiz.net>
* 11 Dec 2006
*
*
* Purpose:
* 		To assign titles to assets in a MySource Matrix system from (asset ID, asset title) pairs
*		in a provided CSV file.
*
* Documentation:
*		A CSV file without a header line is required to assign titles to assets.
*		The expected format is shown below:
*
*			123, Title 1
*			124, Title 2
*			(etc.)
*
*		The user will provided with an option to ignore assets that do not have a 'title' attribute (eg; Site assets)
*		if they have been provided in the file. When encountered, this prompt will also allow the script to be aborted.
*
*/


/**
* Prints out some basic help info detailing how to use this script
*
* @return void
* @access public
*/
function printUsage()
{
	printStdErr("Asset title attribute assignment from a CSV file\n");
	printStdErr('Usage: update_asset_titles [system root] [csv file]');
	printStdErr('system root  : The Matrix System root directory. The "title" attributes of assets on this system will be changed');
	printStdErr("csv file     : A comma separated values file containing (asset ID, asset title) pairs\n");

}//end printUsage()


/**
* Prints the supplied string to "standard error" (STDERR) instead of the "standard output" (STDOUT) stream
*
* @param string	$string	The string to write to STDERR
*
* @return void
* @access public
*/
function printStdErr($string)
{
	fwrite(STDERR, "$string\n");

}//end printStdErr()


/**
* Waits for command-line user input that matches one of the specified valid input values, and returns the value entered
*
* @param array	$valid_choices	A set of valid user input (in lowercase) to be accepted
*
* @return string
* @access public
*/
function getUserInput($valid_choices)
{
	$valid_input = FALSE;
	while (!$valid_input) {
		$user_input = rtrim(strtolower(fgets(STDIN, 1024)));
		$valid_input = in_array($user_input, $valid_choices);
	}

	return $user_input;

}//end getUserInput()


/**
* Assigns a title attribute value to a selected asset
*
* @param int	$asset_id		The asset ID
* @param string	$asset_title	The title to be set for the asset
*
* @return boolean
* @access public
*/
function setAssetTitle($asset_id, $asset_title)
{
	$success = FALSE;

	$asset =& $GLOBALS['SQ_SYSTEM']->am->getAsset($asset_id);
	if ($asset->id) {
		// Make sure that we have a 'title' attribute
		$current_title = $asset->attr('title');
		if ($current_title == NULL) {
			printStdErr('* Asset ID '.$asset_id." does not have a title field, so one cannot be assigned.\n");

			printStdErr('User Options ------------');
			printStdErr('(I)gnore this asset and continue with modification of titles');
			printStdErr('(C)ancel modification script');

			$user_choice = getUserInput(Array('i','c'));
			if ($user_choice == 'c') {
				printStdErr("\n- Titles modification cancelled by user");
				exit(-7);
			}
		} else {
			// Set the 'title' attribute for the asset
			$asset->setAttrValue('title', $asset_title);
			$asset->saveAttributes();

			// Read back the title to ensure that it is set as expected
			$success = ($asset->attr('title') == $asset_title);
			if (!$success) {
				printStdErr('* The title "'.$asset_title.'" could not be set for asset ID '.$asset_id."\n");

				printStdErr('User Options ------------');
				printStdErr('(I)gnore this asset and continue with modification of titles');
				printStdErr('(C)ancel modification script');

				$user_choice = getUserInput(Array('i','c'));
				if ($user_choice == 'c') {
					printStdErr("\n- Titles modification cancelled by user");
					exit(-8);
				}

			}
		}
	}//end if

	return $success;

}//end setAssetTitle()


/************************** MAIN PROGRAM ****************************/

if ((php_sapi_name() != 'cli')) {
	trigger_error("You can only run this script from the command line\n", E_USER_ERROR);
}

// Matrix system root directory
$argv = $_SERVER['argv'];
$SYSTEM_ROOT = (isset($argv[1])) ? $argv[1] : '';
if (empty($SYSTEM_ROOT) || !is_dir($SYSTEM_ROOT)) {
	printUsage();
	printStdErr("* The path to the System Root must be specified as the first parameter\n");
	exit(-1);
}

// Has a CSV filename been supplied?
$csv_filename	= $argv[2];
if (empty($csv_filename)) {
	printUsage();
	printStdErr("* A CSV filename must be specified as the second parameter\n");
	exit(-2);
}

// Does the supplied CSV file exist?
$csv_fd = fopen($csv_filename, 'r');
if (!$csv_fd) {
	printUsage();
	printStdErr("* The supplied CSV file was not found\n");
	exit(-3);
}

// Initialise Matrix
require_once $GLOBALS['SYSTEM_ROOT'].'/core/include/init.inc';

// Provide the user with one last chance to abort this important operation
$system_name = SQ_CONF_SYSTEM_NAME;
echo('PLEASE NOTE: The titles of assets in the '.(($system_name != '') ? '"'.$system_name.'" ' : '')."system will be modified by this script.\n");
echo('(C)ancel or (O)k? ');

$user_choice = getUserInput(Array('c','o'));
if ($user_choice == 'c') {
	printStdErr("\n- Title modification cancelled by user");
	exit(-4);
}

// Forcibly set the title when modifying assets
$GLOBALS['SQ_SYSTEM']->setRunLevel(SQ_RUN_LEVEL_FORCED);

// Yippee, we have the appropriate file. Let's process it now
echo("\n- Assigning titles from CSV file...\n\n");

$num_assets_updated = 0;
$num_assets_in_file = 0;

// Loop through each CSV record
while (($data = fgetcsv($csv_fd, 1024, ',')) !== FALSE) {
	$num_fields = count($data);

	// Ensure that we have the two columns required for the script
	if ($num_fields != 2) {
		printStdErr("* Only two columns (asset ID, asset title) are expected in the supplied CSV file\n");
		exit(-5);
	}

	// The asset ID should be in the first column, and the title should be in the second
	$data[0] = trim($data[0]);

	$asset_id = (int)$data[0];
	$asset_title = trim($data[1]);

	// Ensure that the asset ID is a valid number which is greater than zero
	if (($asset_id != $data[0]) || ($asset_id == 0)) {
		printStdErr('* An invalid asset ID ('.$data[0].") was encountered in the supplied CSV file\n");
		exit(-6);
	}

	$num_assets_in_file++;

	// Now set the title for the asset
	echo '-- Asset #'.$asset_id.': '.$asset_title."\n";
	$success = setAssetTitle($asset_id, $asset_title);

	if ($success) $num_assets_updated++;

}

// Close the CSV file
fclose($csv_fd);

// We're done. Display some totals to show what has been accomplished
echo "\n- All done, stats below:\n";
echo 'Asset titles modified : '.$num_assets_updated."\n";
echo 'Asset records in file : '.$num_assets_in_file."\n";

// Restore the Matrix run level
$GLOBALS['SQ_SYSTEM']->restoreRunLevel();

?>
