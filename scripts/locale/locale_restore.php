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
* $Id: locale_restore.php,v 1.7 2012/10/05 07:20:38 akarelia Exp $
*
*/


/**
* Locale Restore Script
*
* Gathers a set of locale files compiled by Locale Backup, and returns them to
* their proper place in the Matrix installation.
*
* Sample Usage (from System Root):
*	php scripts/locale/locale_restore.php . -d ./locale_backup -rf
*
* Parameters:
*	-d	Specifies the directory of the backed up files. If
*		omitted, defaults to [SYSTEM ROOT]/data/temp/locale_backup.
*
*	-r	Recurse subdirectories.
*
*	-f	Force overwriting of langauge files. If not set, this script will
*		ask you to confirm overwriting of each file that already exists.
*
* Note:
*	Locale does not have to be specified at the command line - each compiled
*	XML file has locale info that this script uses..
*
* @author  Luke Wright <lwright@squiz.net>
* @version $Revision: 1.7 $
* @package MySource_Matrix
* @subpackage install
*/
if (ini_get('memory_limit') != '-1') ini_set('memory_limit', '-1');
error_reporting(E_ALL);
$SYSTEM_ROOT = '';
$exs = Array();


// from cmd line
$cli = true;

if ((php_sapi_name() == 'cli')) {
	if (isset($_SERVER['argv'][1])) {
		$SYSTEM_ROOT = $_SERVER['argv'][1];
	}
	$err_msg = "You need to supply the path to the System Root as the first argument\n";

} else {
	$err_msg = '
	<div style="background-color: red; color: white; font-weight: bold;">
		You can only run the '.$_SERVER['argv'][0].' script from the command line
	</div>
	';
}

if (empty($SYSTEM_ROOT)) {
	echo $err_msg;
	exit();
}

if (!is_dir($SYSTEM_ROOT) || !is_readable($SYSTEM_ROOT.'/core/include/init.inc')) {
	echo "ERROR: Path provided doesn't point to a Matrix installation's System Root. Please provide correct path and try again.\n";
	exit();
}

require_once $SYSTEM_ROOT.'/install/install.inc';


// only use console stuff if we're running from the command line
if ($cli) {
	require_once 'Console/Getopt.php';

	$shortopt = 'd:rf';
	$longopt = Array();

	$con  = new Console_Getopt;
	$args = $con->readPHPArgv();
	array_shift($args);			// remove the system root
	$options = $con->getopt($args, $shortopt, $longopt);

	if (is_array($options[0])) {
		$opt_list = get_console_list($options[0]);
		$locale_list = $opt_list['locale'];
	}

}

require_once $SYSTEM_ROOT.'/core/include/init.inc';

$root_user = &$GLOBALS['SQ_SYSTEM']->am->getSystemAsset('root_user');

// log in as root
if (!$GLOBALS['SQ_SYSTEM']->setCurrentUser($root_user)) {
	echo "Failed login in as root user\n";
	exit();
}

// get the list of functions used during install
require_once SQ_FUDGE_PATH.'/general/file_system.inc';
require_once 'XML/Tree.php';

if (empty($opt_list['dir'])) {
	$opt_list['dir'] = SQ_SYSTEM_ROOT.'/data/temp/locale_backup';
}

$file_list = Array();
$folder_list = Array(realpath($opt_list['dir']));

while(!empty($folder_list)) {
	$folder = array_pop($folder_list);
	$d = @opendir($folder);
	bam($folder);

	if ($d) {
		while (false !== ($entry = readdir($d))) {
			if (($entry == '..') || ($entry == '.') || ($entry == 'CVS')) continue;
			$file = $folder.'/'.$entry;

			// if this is a folder, add it in only if -r has been included
			if (is_dir($file) && $opt_list['recurse']) {
				array_push($folder_list, $file);
			}

			// if this is one of the files that we've been expecting, then
			// go ahead and process it
			if (substr($entry,-4) == '.xml') {
				process_file(realpath($SYSTEM_ROOT), $file, $opt_list['force']);
			}

		}

	}

}

// End of script
exit(0);



/**
* Gets a list of supplied package options from the command line arguments given
*
* Returns an array in the format needed for package_list
*
* @param array	$options	the options as retrieved from Console::getopts
*
* @return array
* @access public
*/
function get_console_list($options)
{
	$list = Array(
				'locale'	=> Array(),
				'dir'		=> '',
				'force'		=> false,
				'recurse'	=> false,
			);

	foreach ($options as $option) {
		// if nothing set, skip this entry
		if (!isset($option[0])) continue;

		switch($option[0]) {
			case '--locale':
				if (!isset($option[1])) continue;
				// Now process the list
				$parts = explode('-', $option[1]);

				$types = Array();
				if ((count($parts) == 2) && strlen($parts[1])) {
					$types = explode(',', $parts[1]);
				} else {
					$types = Array('all');
				}

				$list['locale'][$parts[0]] = $types;
			break;

			case 'd':
				if (!isset($option[1])) continue;
				$list['dir'] = $option[1];
			break;

			case 'r':
				$list['recurse'] = true;
			break;

			case 'f':
				$list['force'] = true;
			break;
		}

	}

	return $list;

}//end get_console_list()


/**
*
* @param string	$file	the file to run through
*
* @return boolean
* @access public
*/
function process_strings_file($path, $file)
{
	bam('STRINGS '.$file);
	$xml_object = new XML_Tree($file);
	$root = $xml_object->getTreeFromFile();
	return true;

}//end process_strings_file()


/**
*
* @param string	$file	the file to run through
*
* @return boolean
* @access public
*/
function process_internal_messages_file($path, $file)
{
	bam('MESSAGES '.$file);
	$xml_object = new XML_Tree($file);
	$root = $xml_object->getTreeFromFile();
	return true;

}//end process_internal_messages_file()


/**
*
* @param string	$file	the file to run through
*
* @return boolean
* @access public
*/
function process_errors_file($path, $file)
{
	bam('ERRORS '.$file);
	$xml_object = new XML_Tree($file);
	$root = $xml_object->getTreeFromFile();

	assert_equals($root->name, 'files');
	echo $locale = $root->attributes['locale'];
	for(reset($root->children); null !== ($file_index = key($root->children)); next($root->children)) {
		$file_tag =& $root->children[$file_index];
		$file_name = $path.'/'.$file_tag->attributes['source'].'/'.$locale.'/lang_errors.xml';

		// now create the file (and directory if required)
		create_directory($path.'/'.$file_tag->attributes['source'].'/'.$locale);
		string_to_file($file_tag->children[0]->get(), $file_name);
	}

	return true;

}//end process_errors_file()


/**
*
* @param string	$file	the file to run through
*
* @return boolean
* @access public
*/
function process_file($path, $file, $force)
{
	$file_contents = file_get_contents($file);
	preg_match('|<files locale="([^"]*)">.*</file>|s', $file_contents, $match);

	// get locale and replace delimiters between locale parts with slashes
	$locale = $match[1];
	$locale = str_replace('@', '/', $locale);
	$locale = str_replace('_', '/', $locale);

	$old_force = $force;

	preg_match_all('|<file source="([^"]*)" name="([^"]*)">(.*)</file>|Us', $file_contents, $matches, PREG_SET_ORDER);
	foreach($matches as $match) {
		$file_name = $path.'/'.$match[1].'/'.$locale.'/'.$match[2];
		$file_str = '<?xml version="1.0" ?>'."\n".$match[3];

		if (!file_exists($file_name)) {
			$force = true;
		} else {
			$force = $old_force;
		}

		if (!$force) {
			// ask for the root password for the system
			echo 'Overwrite file '.hide_system_root($file_name).'?';
			$overwrite_confirm = '';
			while((strlen($overwrite_confirm) == 0) ||
				((strtolower($overwrite_confirm{0}) != 'y') && strtolower($overwrite_confirm{0}) != 'n')) {
				$overwrite_confirm = rtrim(fgets(STDIN, 4094));
			}
			if (strtolower($overwrite_confirm{0}) == 'y') {
				$force = true;
			}
		}

		if ($force) {
			// create directory and dump file
			create_directory($path.'/'.$match[1].'/'.$locale);
			string_to_file($file_str, $file_name);
			echo 'Wrote '.hide_system_root($file_name)."\n";
		}
	}

	return true;

}//end process_localised_screens_file()
?>
