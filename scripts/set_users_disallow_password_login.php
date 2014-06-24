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
*/


/**
* set_users_disallow_password_login.php
*
* Purpose
*    Script to set all users underneath a certain root node to either 
*    "disallow password login" or unset this setting.
*
* Synopsis
*    php set_users_disallow_password_login.php <system_root> <root_assetid> <setting>
*      - system_root  : the filesystem root of the Matrix installation
*      - root_assetid : asset ID of the root node to change from
*      - setting      : 1 to set disallow password login, 0 to unset it
*      - linked user only      : 1 to only apply changes to SAML/Oauth linked user, 0 to apply this change to all users
*
*    Change "php" to the appropriate CLI compilation of PHP for your system
*    (eg. sometimes it will be installed to "php-cli" instead).
*
*
*/


//--        DRIVER CODE        --//


error_reporting(E_ALL);
if ((php_sapi_name() != 'cli')) {
	trigger_error('This script must be run from the command line', E_USER_ERROR);
	exit(1);
}

// Handle cases where the number of parameters is less than expected.
// (Note: the count is one more than you may think, because $ARGV[0] is the
// script name.)
$ARGV = $_SERVER['argv'];
switch (count($ARGV)) {
	case 1:
		error_line('You must specify a system root to your MySource Matrix installation');
		short_usage();
		exit(1);
	break;

	case 2:
		// This may be sorta legal if we are asking for the long usage
		if ($ARGV[1] == '--help') {
			long_usage();
			exit(0);
		} else {
			error_line('You must provide the asset ID of the root node to search under');
			short_usage();
			exit(1);
		}
	break;

	case 3:
		error_line('You must tell the script whether users are to be disallowed for password login or unset this attribute');
		short_usage();
		exit(1);

	case 4:
		error_line('You must tell the script if it should apply for SAML/Oauth linked user or all users');
		short_usage();
		exit(1);

	break;
}

list($SYSTEM_ROOT, $ROOT_ASSETID, $DISALLOW_SETTING, $LINKED_ONLY) = array_slice($ARGV, 1, 4);

if (!is_dir($SYSTEM_ROOT) || !is_readable($SYSTEM_ROOT.'/core/include/init.inc')) {
	error_line('ERROR: Path provided doesn\'t point to a Matrix installation\'s System Root. Please provide correct path and try again.');;
	exit(1);
}

require_once $SYSTEM_ROOT.'/core/include/init.inc';

// Have we been passed a dodgy root asset ID? 
if (!assert_valid_assetid($ROOT_ASSETID, '', TRUE, FALSE)) {
	error_line('Root asset ID "'.$ROOT_ASSETID.'" does not appear to be valid');
	exit(1);
}

$root_assetid_details = $GLOBALS['SQ_SYSTEM']->am->getAssetInfo($ROOT_ASSETID);
if (empty($root_assetid_details)) {
	error_line('Root asset ID "'.$ROOT_ASSETID.'" does not exist');
	exit(1);
}

// The "disallow" seting is valid or not?
if (($DISALLOW_SETTING !== '0') && ($DISALLOW_SETTING !== '1')) {
	error_line('The disallow password login setting is not invalid - it must be either "1" (disallow) or "0" (allow)');
	exit(1);
}

// The "linked only" seting is valid or not?
if (($DISALLOW_SETTING !== '0') && ($DISALLOW_SETTING !== '1')) {
	error_line('The linked user only setting is not invalid - it must be either "1" (only linked SAML/Oauth user) or "0" (all users)');
	exit(1);
}


$user_assetids = $GLOBALS['SQ_SYSTEM']->am->getChildren($ROOT_ASSETID, 'user', FALSE);
// if we are only interested in SAML/Oauth linked accounts, make sure we filter those don't
if($LINKED_ONLY) {
	foreach ($user_assetids as $id => $details) {
		// filter SAML
		$not_linked_saml = FALSE;
		$sql='select count(*) from sq_saml_lnk where assetid = '.MatrixDAL::quote($id);
		$result = MatrixDAL::executeSqlAssoc($sql);
		if(isset($result[0]['count']) && empty($result[0]['count'])) {
			$not_linked_saml = TRUE;
		}
		// filter Oauth
		$not_linked_oauth = FALSE;
		$sql='select count(*) from sq_oauth_lnk where matrix_userid = '.MatrixDAL::quote($id);
		$result = MatrixDAL::executeSqlAssoc($sql);
		if(isset($result[0]['count']) && empty($result[0]['count'])) {
			$not_linked_oauth = TRUE;
		}
		if($not_linked_saml && $not_linked_oauth) {
			unset($user_assetids[$id]);
		}

	}
}
echo 'Found '.count($user_assetids).' User asset(s) underneath asset ID #'.$ROOT_ASSETID."\n";

echo 'Are you sure you want to '.($DISALLOW_SETTING ? 'enable' : 'disable').' Disallow Password Login setting on these assets (Y/N)?';
$yes_no = rtrim(fgets(STDIN, 4094));
if (strtolower($yes_no) != 'y') {
	echo "\nScript aborted. \n";
	exit;
}



// no assets to work on, just exit normally, otherwise continue
if (count($user_assetids) > 0) {
	// log in as root
	$root_user = $GLOBALS['SQ_SYSTEM']->am->getSystemAsset('root_user');
	if (!$GLOBALS['SQ_SYSTEM']->setCurrentUser($root_user)) {
	    error_line('Could not log in as root user');
    	exit(1);
	}

	$GLOBALS['SQ_SYSTEM']->setRunLevel(SQ_RUN_LEVEL_FORCED);
	echo "\n";
	do_set_disallow($ROOT_ASSETID, $DISALLOW_SETTING, $user_assetids);
	echo "\n";
	$GLOBALS['SQ_SYSTEM']->restoreRunLevel();

	// Log out the root user
	$GLOBALS['SQ_SYSTEM']->restoreCurrentUser();
	unset($root_user);
}

exit(0);


//--        ACTION FUNCTIONS        --//


/**
* Actually perform the changes to the files
*
* @param string	$root_node			Asset ID of the root node to search for Files from
* @param int	$setting			The disallow setting
* @param array	$file_assretids		assetid of all the file type assets found under the root node
*
* @return void
*/
function do_set_disallow($root_node, $setting, $user_assetids)
{
	status_message_start('Updating attributes...');	
	foreach ($user_assetids as $id => $details) {
		$user = $GLOBALS['SQ_SYSTEM']->am->getAsset($id);
		$user->setAttrValue('disallow_password_login', $setting);
		$user->saveAttributes();
		$GLOBALS['SQ_SYSTEM']->am->forgetAsset($user);
		unset($user);
	}
	status_message_result('OK');
}//end do_set_disallow()


/**
* Print a status message start for each asset
*
* Format is intended to be like "Asset name [ STATUS ]".
*
* @param string	$name	The name of the asset
* @param string	$status	The status to show for the asset
*
* @return void;
*/
function status_message_start($name) {
	printf('%-59s ', $name);
	return;

}//end status_message()


/**
* Print a status message result for each asset
*
* Format is intended to be like "Asset name [ STATUS ]".
*
* @param string	$status	The status to show for the asset
*
* @return void;
*/
function status_message_result($status) {
	printf('[ %-2s ]'."\n", $status);
	return;

}//end status_message()


//--        USAGE DISPLAY        --//


/**
* Displays a "short usage" message
*
* The short usage message is shown when not enough parameters are provided.
*
* @return void
*/
function short_usage()
{
	if (isset($_SERVER['_'])) {
		$php_name = basename($_SERVER['_']);
	} else {
		$php_name = 'php';
	}
	
	echo 'Usage: '.$php_name.' '.$_SERVER['argv'][0].' <system_root> <root_assetid> <setting>'."\n";
	echo 'Try "'.$php_name.' '.$_SERVER['argv'][0].' --help" for more information.'."\n";
	return;

}//end short_usage()


function long_usage()
{
	if (isset($_SERVER['_'])) {
		$php_name = basename($_SERVER['_']);
	} else {
		$php_name = 'php';
	}
	
	echo 'Usage: '.$php_name.' '.$_SERVER['argv'][0].' <system_root> <root_assetid> <setting> <linked_user_only>'."\n";
	echo 'Sets all User assets underneath asset #<root_assetid> to either Disallow Password Login or'    ."\n";
	echo 'unset this setting, depending on the value of <setting>.'                                   ."\n\n";

	echo 'Users set as Disallowed Password Login will not be allowed to login via standard username and password. '    ."\n";
	echo 'Only an external login system such as SAML or Oauth2 can authenticate this user.' ."\n\n";

	echo 'Arguments:'                                                                         ."\n";
	echo '  system_root    The root directory of this Matrix installation.'          ."\n";
	echo '  root_assetid   The root ID of the asset to look for User assets underneath.'           ."\n";
	echo '                 (This must be provided. Do not use 1, unless you want to block all users in system)'       ."\n";
	echo '  setting        Determines what happens to Users assets underneath the root nodes:' ."\n";
	echo '                     1   Sets Users to "Disallow Password Login".'                             ."\n";
	echo '                     0   Unsets this setting.'                               ."\n";
	echo '  linked_user_only   Determines if the change shold apply to SAML/Oauth linked users only or all users:' ."\n";
	echo '                     1   Only apply to linked users.'                             ."\n";
	echo '                     0   Apply to all users.'                               ."\n";
	return;

}//end long_usage()


/**
* Displays an error line in the UNIX style
*
* @return void
*/
function error_line($error)
{
	echo $_SERVER['argv'][0].': '.$error."\n";
	return;

}//end error_line()
?>
