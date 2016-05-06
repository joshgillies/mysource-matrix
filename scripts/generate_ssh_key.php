<?php
/**
* +--------------------------------------------------------------------+
* | This MySource Matrix CMS file is Copyright (c) Squiz Pty Ltd       |
* | ABN 084 670 600                                                    |
* +--------------------------------------------------------------------+
* | IMPORTANT: Your use of this Software is subject to the terms of    |
* | the Licence provided in the file licence.txt. If you cannot find   |
* | this file please contact Squiz (www.squiz.com.au) so we may provide|
* | you a copy.                                                        |
* +--------------------------------------------------------------------+
*
*/

/**
* Generate the SSH RSA public/private key pair for the given Matrix instance
* The key filename generated is set to SQ_TOOL_SSH_KEYGEN_PATH
* See Squizmap #8006
*
* @author Chiranjivi Upreti <cupreti@squiz.com.au>
* @version $Revision: $
* @package MySource_Matrix
*/

if (ini_get('memory_limit') != '-1') ini_set('memory_limit', -1);
error_reporting(E_ALL);
if ((php_sapi_name() != 'cli')) trigger_error("You can only run this script from the command line\n", E_USER_ERROR);

$SYSTEM_ROOT = (isset($_SERVER['argv'][1])) ? $_SERVER['argv'][1] : '';
$generate_key = in_array('--generate', $_SERVER['argv']);
$show_info = in_array('--info', $_SERVER['argv']);

if (empty($SYSTEM_ROOT) || !is_dir($SYSTEM_ROOT) || !is_readable($SYSTEM_ROOT.'/core/include/init.inc') || (!$generate_key && !$show_info)) {
	print_usage();
}

require_once $SYSTEM_ROOT.'/core/include/init.inc';
require_once SQ_INCLUDE_PATH.'/general_occasional.inc';

$root_user = $GLOBALS['SQ_SYSTEM']->am->getSystemAsset('root_user');
$GLOBALS['SQ_SYSTEM']->setCurrentUser($root_user);

if (empty($_SERVER['HOME']) || !is_dir($_SERVER['HOME'])) {
	trigger_error(translate('Could not locate the home directory.'), E_USER_WARNING);
	exit(1);
}

require_once SQ_DATA_PATH.'/private/conf/tools.inc';

// SSH key dir
$ssh_key_dir = rtrim($_SERVER['HOME'], '/').'/.ssh';

// SSH key file path. The key filename is based on the Matrix instance system root path
$key_file_path = $ssh_key_dir.'/matrix_key_'.md5(SQ_SYSTEM_ROOT);

if ($generate_key) {
	// Create ssh key dir if not already there
	if (!is_dir($ssh_key_dir)) {
		if (!mkdir($ssh_key_dir, '0700')) {
			trigger_error('Could not create the ssh key directory "'.$ssh_key_dir.'".');
			exit(1);
		}
	}
	if (!is_executable(SQ_TOOL_SSH_KEYGEN_PATH)) {
		trigger_error('Supplied path to "ssh-keygen" tool is not valid.', E_USER_WARNING);
		exit(1);
	}

	if (is_file($key_file_path)) {
		echo "SSH key file ".$key_file_path." already exists. Overwrite [y/n]?";
		$response = trim(fgets(STDIN, 1024));
		if ($response !== 'y') {
			echo "Aborting.\n";
			exit();
		}
		unlink($key_file_path);
		if (is_file($key_file_path.'.pub')) {
			unlink($key_file_path.'.pub');
		}
	}

	// Generate the key pair
	$command = SQ_TOOL_SSH_KEYGEN_PATH.' -t rsa -N "" -C "Matrix system name : '.SQ_CONF_SYSTEM_NAME.'" -f '.$key_file_path;
	echo "Generating public/private rsa key pair ...\n";
	$output = executeShellCommand($command, FALSE);
	if (!$output) {
		trigger_error('Error occured when generating the SSH key.', E_USER_WARNING);
		exit(1);
	}
	echo $output;

	// Set appropriate file modes on the ssh key files
	chmod($key_file_path, 0600);
	chmod($key_file_path.'.pub', 0600);

	// Set the key file path to the Matrix external config
	ob_start();
		include_once SQ_INCLUDE_PATH.'/external_tools_config.inc';
		$system_config = new External_Tools_Config();
		$GLOBALS['SQ_SYSTEM']->setRunLevel(SQ_RUN_LEVEL_FORCED);
			$result = $system_config->save(Array('SQ_TOOL_SSH_KEY_PATH' => $key_file_path));
		$GLOBALS['SQ_SYSTEM']->restoreRunLevel();
	ob_end_clean();
	
	if (!$result) {
		trigger_error('Error occured when setting the SSH key file path in external config. Please set SQ_TOOL_SSH_KEY_PATH" to "'.$key_file_path.'" in the external tools config file.', E_USER_WARNING);
	}
	
	// Add host key entries for gitlab.squiz.net and github.com to the "known hosts" by default
	// Other host entries if required are to be added manually
	$default_hosts = Array(
						'gitlab.squiz.net',
						'github.com',
					);
	if (!is_file($ssh_key_dir.'/known_hosts')) {
		touch($ssh_key_dir.'/known_hosts');
	}
	foreach($default_hosts as $host) {
		$cmd = SQ_TOOL_SSH_KEYSCAN_PATH.' -t rsa,dsa '.$host.' 2>&1 | sort -u - '.$ssh_key_dir.'/known_hosts > '.$ssh_key_dir.'/known_hosts.tmp';
		$ret_val = executeShellCommand($cmd, FALSE, FALSE) !== FALSE && executeShellCommand('mv '.$ssh_key_dir.'/known_hosts.tmp '.$ssh_key_dir.'/known_hosts', FALSE) !== FALSE;
		if ($ret_val===FALSE) {
			trigger_error('Error occured when adding the host key for "'.$host.'" to the known host file,', E_USER_WARNING);
		}
	}

} else {
	// In report mode display the info on the existing key
	if (is_file($key_file_path)) {
		echo "Fingerprint: ";
		echo executeShellCommand(SQ_TOOL_SSH_KEYGEN_PATH.' -lf '.escapeshellarg($key_file_path), FALSE);
		echo "Generated on ".date("Y-m-d H:i:s.", filemtime($key_file_path))."\n";
	} else {
		echo "SSH key has not been generated for this instance.\n";
	}
}

$GLOBALS['SQ_SYSTEM']->restoreCurrentUser();
exit();


/**
* Print the usage of this script
*
* @return void
*/
function print_usage()
{
	echo "This script generates the RSA key for the specified Matrix instance.\n\n";
	echo "Usage php  php ".basename(__FILE__)." <SYSTEM_ROOT> [--generate] [--info]\n\n";
	echo "\t<SYSTEM_ROOT> : The root directory of Matrix system.\n";
	echo "\t[--generate]  : Generate the RSA key.\n";
	echo "\t[--info]      : Show the existing SSH key info if any.\n";
	echo "\n";

	exit();

}//end print_usage()
?>
