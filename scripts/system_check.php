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
* $Id: system_check.php,v 1.6 2012/08/30 01:04:53 ewang Exp $
*
*/

/**
* Check the system for possible errors and problems and only report them 
*
* Syntax:
*		--system=[MATRIX_ROOT]
*		--verbose
*		--no-colours
*
* @author  Benjamin Pearson <bpearson@squiz.net>
* @version $Revision: 1.6 $
* @package MySource_Matrix
*/
error_reporting(E_ALL);
if (ini_get('memory_limit') != '-1') ini_set('memory_limit', '-1');
if (php_sapi_name() != 'cli') {
	trigger_error("You can only run this script from the command line\n", E_USER_ERROR);
	exit(1);
}//end if

$help  = "";
$help .= "Syntax: system_check.php [ARGS]\n";
$help .= "Where [ARGS] can be:\n";
$help .= "\t--system=[MATRIX_ROOT]\tThe path to the matrix system\n";
$help .= "\t--verbose\t\tShow more detailed errors\n";
$help .= "\t--colours\t\tUse colours\n";
$help .= "\t--stats\t\t\tShow statistics for this process\n";
$help .= "\t--help\t\t\tShow this help screen\n";
$help .= "\t--execute\t\tMake a script execute it's action if it has one\n";
$help .= "\t[test_to_run]\n";

// Defaults
$SYSTEM_ROOT = '';
$VERBOSE = FALSE;
$COLOURS = FALSE;
$STATS = FALSE;
$tests_to_run = Array();
$EXECUTE = FALSE;

// Process arguments
foreach ($_SERVER['argv'] as $placement => $argument) {
	if (!empty($placement)) {
		$argument = ltrim($argument, '-');
		if (strpos($argument, '=') !== FALSE) {
			list($command, $parameter) = explode('=', $argument);
		} else {
			$command = $argument;
			$parameter = '';
		}//end if
		switch ($command) {
			case 'system':
				$SYSTEM_ROOT = $parameter;
			break;
			case 'verbose':
				$VERBOSE = TRUE;
			break;
			case 'colours':
				$COLOURS = TRUE;
			break;
			case 'stats':
				$STATS = TRUE;
			break;
			case 'execute':
				$EXECUTE = TRUE;
			break;
			case 'help':
				echo $help;
				exit();
			break;
			default:
				$tests_to_run[] = $command;
		}//end switch
	}//end if
}//end foreach

if (empty($SYSTEM_ROOT)) {
	echo "ERROR: You need to supply the path to the System Root as the first argument\n";
	echo $help;
	exit();
}

if (!is_dir($SYSTEM_ROOT) || !is_readable($SYSTEM_ROOT.'/core/include/init.inc')) {
	echo "ERROR: Path provided doesn't point to a Matrix installation's System Root. Please provide correct path and try again.\n";
	echo $help;
	exit();
}

define('SQ_SYSTEM_ROOT', $SYSTEM_ROOT);
require_once SQ_SYSTEM_ROOT.'/core/include/init.inc';
require_once SQ_FUDGE_PATH.'/general/file_system.inc';

// Starting stats reporting
mem_check(NULL, TRUE);
speed_check('', FALSE, FALSE);

// Deep system checking requires full access
$root_user = $GLOBALS['SQ_SYSTEM']->am->getSystemAsset('root_user');
$GLOBALS['SQ_SYSTEM']->setRunLevel(SQ_RUN_LEVEL_FORCED);
$GLOBALS['SQ_SYSTEM']->setCurrentUser($root_user);

// Check the system here
$test_dir = dirname(__FILE__).'/system_tests';
$tests = list_files($test_dir.'/*.inc');
foreach ($tests as $test) {
	// Only use files starting with test and ending with .inc
	if (!preg_match('/^test(.*)\.inc$/i', $test)) continue;

	$test_basename = basename($test, '.inc');
	if (!empty($tests_to_run)) {
		if (!in_array($test_basename, $tests_to_run)) {
			continue;
		}
	}

	include_once $test_dir.'/'.$test;
	$class_name = str_replace('_', ' ', $test_basename);
	$class_name = ucwords($class_name);
	$class_name = str_replace(' ', '_', $class_name);
	$messages = Array();
	$errors = Array();
	$runTestMethod = TRUE;
	if ($EXECUTE) {
		if (method_exists($class_name, 'execute')) {
			$status = call_user_func_array(Array($class_name, 'execute'), Array(&$messages, &$errors));
			$runTestMethod = FALSE;
		} else {
			echo "Test " . $test . " doesn't have an 'execute' option.\n";
		}
	}

	if ($runTestMethod) {
		$status = call_user_func_array(Array($class_name, 'test'), Array(&$messages, &$errors));
	}

	$name = call_user_func_array(Array($class_name, 'getName'), Array());

	showStatus($name, $status, $messages, $errors, $VERBOSE, $COLOURS);
}//end foreach

// All done, cleaning up
$GLOBALS['SQ_SYSTEM']->restoreCurrentUser();
$GLOBALS['SQ_SYSTEM']->restoreRunLevel();

// Show stats
if ($STATS) {
	echo "Memory Usage: ".mem_check(NULL, TRUE)."\n";
	echo "Time taken: ";
	speed_check('', FALSE, FALSE);
	echo "seconds\n";
}//end if
exit();

/**
* Show the status of the current step
*
* @param string		$name		The name of the current step
* @param bool/int	$status		The actual status (pass/fail)
* @param array		$messages	The messages to show
* @param array		$errors		The errors to show if verbose
* @param boolean	$verbose	Show errors or not
* @param boolean	$colours	Use pretty colours (look at those pretty colours)
*
* @return void
* @access public
*/
function showStatus($name, $status, $messages=Array(), $errors=Array(), $verbose=FALSE, $colours=TRUE)
{
	// Local variables
	if ($colours) {
		$statusOk   = "\033[50G[  \033[1;32mOK\033[0m  ]";
		$statusErr  = "\033[50G[  \033[1;31m!!\033[0m  ]";
		$statusWarn = "\033[50G[  \033[1;33m??\033[0m  ]";
		$statusInfo = "\033[50G[ \033[1;34minfo\033[0m ]";
	} else {
		$statusOk   = "\033[50G[  OK  ]";
		$statusErr  = "\033[50G[  !!  ]";
		$statusWarn = "\033[50G[  ??  ]";
		$statusInfo = "\033[50G[ info ]";
	}//end if

	// Current step
	echo $name;
	if (is_bool($status)) {
		echo ($status) ? $statusOk : $statusErr;
	} else {
		switch ($status) {
			case '1':
				echo $statusOk;
			break;
			case '2':
				echo $statusInfo;
			break;
			case '3':
				echo $statusWarn;
			break;
			default:
				echo $statusErr;
		}//end switch
	}//end if
	echo "\n";

	// Messages
	if (($status !== TRUE || $status !== '1') && !empty($messages)) {
		// Show messages
		foreach ($messages as $message) {
			echo "\t".$message."\n";
		}//end foreach
	}//end if

	// Errors
	if (($status !== TRUE || $status !== '1') && $verbose && !empty($errors)) {
		// Show errors
		foreach ($errors as $error) {
			echo "\t".$error."\n";
		}//end foreach
	}//end if

}//end showStatus()


?>
