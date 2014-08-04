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
* Delete test_message.php in data/public location, because we move it to lib/web now.
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

echo "Note: data/public/test_message.php has been moved to core/lib/web/test_message.php as a result of RM #5722\n";
$confirm = null;
while ($confirm != 'y' && $confirm != 'n') {
	$confirm = strtolower(get_line('Delete data/public/test_message.php? (y/n)? : '));
	if ($confirm != 'y' && $confirm != 'n') {
		echo 'Please answer y or n'."\n";
	}
}

if($confirm == 'y') {
	echo "Removing..";
	$file_path = $SYSTEM_ROOT.'/data/public/test_message.php';
	if(is_file($file_path))
		unlink($file_path);
	echo "done.\n";
}



/**
* Prints the specified prompt message and returns the line from stdin
*
* @param string $prompt the message to display to the user
*
* @return string
* @access public
*/
function get_line($prompt='')
{
    echo $prompt;
    // now get their entry and remove the trailing new line
    return rtrim(fgets(STDIN, 4096));

}//end get_line()

?>