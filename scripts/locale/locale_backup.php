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
* $Id: locale_backup.php,v 1.12 2012/10/05 07:20:38 akarelia Exp $
*
*/


/**
* Locale Backup Script
*
* Compiles all of the localisation files in the system to larger ones - one for
* each of screens, strings, errors and localised messages - so that it is easier
* to localise in one go.
*
* Usage (from System Root):
*	php scripts/locale/locale_backup.php . --locale=en --output=./locale_backup
*
* Parameters:
*	--locale	is as it is in install/compile_locale.php - each option
*				specifies a language, and each can be further qualified with the
*				type of localisation to backup. If omitted completely, the
*				script will do everything.
*
*				Examples:
*					--locale=en					Will back up everything in
*												English
*					--locale=en --locale=fr		Will back up everything in
*												English and French
*					--locale=en-screens			Will back up only localised
*												screens in English
*					--locale=en-screens,errors	Will back up localised screens
*												AND errors only in English
*
*	--output	specifies the output directory of the backed up files. If
*				omitted, defaults to [SYSTEM ROOT]/data/temp/locale_backup.
*
* @author  Luke Wright <lwright@squiz.net>
* @version $Revision: 1.12 $
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

require_once $SYSTEM_ROOT.'/core/include/init.inc';

// only use console stuff if we're running from the command line
if ($cli) {
	require_once 'Console/Getopt.php';

	$shortopt = '';
	$longopt = Array('locale=', 'output=');

	$con  = new Console_Getopt;
	$args = $con->readPHPArgv();
	array_shift($args);			// remove the system root
	$options = $con->getopt($args, $shortopt, $longopt);

	if (is_array($options[0])) {
		$opt_list = get_console_list($options[0]);
		$locale_list = $opt_list['locale'];
	}

}

// if locale list empty, do everything
if (empty($locale_list)) {
	echo "\nWARNING: You did not specify a --locale parameter. This is okay but be aware that all locales will be compiled, which may take a while if you have multiple locales on your system\n\n";
}


$root_user = &$GLOBALS['SQ_SYSTEM']->am->getSystemAsset('root_user');

// log in as root
if (!$GLOBALS['SQ_SYSTEM']->setCurrentUser($root_user)) {
	echo "Failed login in as root user\n";
	exit()
}

// get the list of functions used during install
require_once $SYSTEM_ROOT.'/install/install.inc';
require_once SQ_FUDGE_PATH.'/general/file_system.inc';
require_once 'XML/Tree.php';

// list of languages where we need to compile these for
$string_locales = Array();
$error_locales  = Array();
$message_locales = Array();
$all_screens = Array();
$screens = Array();

// flag that controls when 'compiling edit interfaces' message is printed
$first_ei = true;

if (!empty($opt_list['dir'])) {
	$locale_backup_dir = $opt_list['dir'];
} else {
	$locale_backup_dir = SQ_DATA_PATH.'/temp/locale_backup';
}

create_directory($locale_backup_dir);

// do it for each asset type ...
$asset_types = $GLOBALS['SQ_SYSTEM']->am->getAssetTypes();

// ... but also, give the asset type array a little base asset injection
array_unshift($asset_types, Array(
								  'type_code' => 'asset',
								  'dir'       => 'core/include/asset_edit',
								  'name'      => 'Base Asset',
								 ));

foreach($GLOBALS['SQ_SYSTEM']->getInstalledPackages() as $package) {
	if ($package['code_name'] == '__core__') continue;
	array_unshift($asset_types, Array(
									  'type_code' => '',
									  'dir'       => 'packages/'.$package['code_name'],
									  'name'      => $package['name'],
									 ));
}

// also add top-level global strings - this will not appear on screen as there
// are no static screens for it (as it is not really an asset), but is important
// to catch languages which may only have strings in the top level
array_unshift($asset_types, Array(
								  'type_code' => '',
								  'dir'       => 'core',
								  'name'      => 'Global Strings',
								  ));

$locale_names = array_keys($locale_list);
foreach($locale_names as $locale) {
	$locale_parts = $GLOBALS['SQ_SYSTEM']->lm->getCumulativeLocaleParts($locale);
	foreach($locale_parts as $locale_part) {
		if (!in_array($locale_part, $locale_names)) {
			$locale_list[$locale_part] = $locale_list[$locale];
		}
	}
}

foreach ($asset_types as $asset_type) {

	$type_code = $asset_type['type_code'];

	$local_screen_dir = SQ_DATA_PATH.'/private/asset_types/'.$type_code.'/localised_screens';
	$matches = Array();

	$base_path = SQ_SYSTEM_ROOT.'/'.$asset_type['dir'].'/locale';
	$dirs_to_read = Array($base_path);

	while (!empty($dirs_to_read)) {
		$dir_read = array_shift($dirs_to_read);

		$d = @opendir($dir_read);
		if ($d) {

			// work out the locale name by taking the directory and replacing
			// the slashes with the appropriate underscore and (possibly) at sign
			$locale_name = str_replace($base_path.'/', '', $dir_read);
			if (($slash_pos = strpos($locale_name, '/')) !== false) {
				$locale_name{$slash_pos} = '_';
				if (($slash_pos = strpos($locale_name, '/')) !== false) {
					$locale_name{$slash_pos} = '@';
				}
			}

			while (false !== ($entry = readdir($d))) {
				if (($entry == '..') || ($entry == '.') || ($entry == 'CVS')) continue;

				if (is_dir($dir_read.'/'.$entry)) {
					if (!in_array($dir_read.'/'.$entry, $dirs_to_read)) {
						$dirs_to_read[] = $dir_read.'/'.$entry;
					}
				}

				// get the locale parts
				$locale_parts = $GLOBALS['SQ_SYSTEM']->lm->getCumulativeLocaleParts($locale_name);

				if (preg_match('|lang\_((static_)?screen\_.*)\.xml|', $entry, $matches)) {
					// screen files
					if (!empty($locale_list) && (!in_array($locale_name, array_keys($locale_list))
						|| (!in_array('all', $locale_list[$locale_name])
						&& !in_array('screens', $locale_list[$locale_name])))) {
						continue;
					}

					if (!isset($screens[$locale_name])) {
						$screens[$locale_name] = Array();
					}

					$screens[$locale_name][] = $dir_read.'/lang_'.$matches[1].'.xml';

				} else if (preg_match('|lang\_strings\.xml|', $entry, $matches)) {
					// string files
					foreach($locale_parts as $locale_part) {
						if (!in_array($locale_part, $string_locales)) {
							$string_locales[$locale_part][] = $base_path;
						}
					}

				} else if (preg_match('|lang\_errors\.xml|', $entry, $matches)) {
					// error files
					foreach($locale_parts as $locale_part) {
						if (!in_array($locale_part, $error_locales)) {
							$error_locales[$locale_part][] = $base_path;
						}
					}

				} else if (preg_match('|lang\_messages\.xml|', $entry, $matches)) {
					// internal message files
					foreach($locale_parts as $locale_part) {
						if (!in_array($locale_part, $message_locales)) {
							$message_locales[$locale_part][] = $base_path;
						}
					}
				}
			}
			closedir($d);

		}
	}

	//$all_screens = Array();
	$d = @opendir(SQ_SYSTEM_ROOT.'/'.$asset_type['dir']);
	if ($d) {

		while (false !== ($entry = readdir($d))) {
			if (preg_match('|edit\_interface\_((static_)?screen\_.*).xml|', $entry, $matches)) {
				$all_screens[] = $base_path.'/'.$entry;
			}
		}
		closedir($d);

	}

}


// compile string locales...
foreach($string_locales as $locale => $loc_files) {
	$loc_files = array_unique($loc_files);
	if (!empty($locale_list) && !in_array('strings', array_get_index($locale_list, $locale, Array())) &&
		!in_array('all', array_get_index($locale_list, $locale, Array()))) {
		continue;
	}
	$string_file = '<?xml version="1.0" ?>'."\n";
	$string_file .= '<files locale="'.$locale.'">'."\n";

	foreach($loc_files as $loc_file) {
		$string_file .= '<file source="'.substr(str_replace(SQ_SYSTEM_ROOT, '', $loc_file),1).'"  name="lang_strings.xml">'."\n";
		$string_file .= str_replace('<?xml version="1.0" ?>', '',
							file_get_contents($loc_file.'/'.str_replace('_', '/', str_replace('@', '/', $locale)).'/lang_strings.xml')."\n");
		$string_file .= '</file>'."\n";
	}

	$string_file .= '</files>'."\n";

	create_directory($locale_backup_dir.'/'.$locale);
	string_to_file($string_file, $locale_backup_dir.'/'.$locale.'/strings.xml');
}


// compile error locales...
$error_locales = array_unique($error_locales);
foreach($error_locales as $locale => $loc_files) {
	$loc_files = array_unique($loc_files);
	if (!empty($locale_list) && !in_array('errors', array_get_index($locale_list, $locale, Array())) &&
		!in_array('all', array_get_index($locale_list, $locale, Array()))) {
		continue;
	}
	$error_file = '<?xml version="1.0" ?>'."\n";
	$error_file .= '<files locale="'.$locale.'">'."\n";

	foreach($loc_files as $loc_file) {
		$error_file .= '<file source="'.substr(str_replace(SQ_SYSTEM_ROOT, '', $loc_file),1).'" name="lang_errors.xml">'."\n";
		$error_file .= str_replace('<?xml version="1.0" ?>', '',
							file_get_contents($loc_file.'/'.str_replace('_', '/', str_replace('@', '/', $locale)).'/lang_errors.xml')."\n");

		//$error_file .= process_errors_file($loc_file.'/'.str_replace('_', '/', str_replace('@', '/', $locale)).'/lang_errors.xml');
		$error_file .= '</file>'."\n";
	}

	$error_file .= '</files>'."\n";

	create_directory($locale_backup_dir.'/'.$locale);
	string_to_file($error_file, $locale_backup_dir.'/'.$locale.'/errors.xml');
}


// compile internal message locales...
foreach($message_locales as $locale => $loc_files) {
	$loc_files = array_unique($loc_files);
	if (!empty($locale_list) && !in_array('messages', array_get_index($locale_list, $locale, Array())) &&
		!in_array('all', array_get_index($locale_list, $locale, Array()))) {
		continue;
	}
	$message_file = '<?xml version="1.0" ?>'."\n";
	$message_file .= '<files locale="'.$locale.'">'."\n";

	foreach($loc_files as $loc_file) {
		$message_file .= '<file source="'.substr(str_replace(SQ_SYSTEM_ROOT, '', $loc_file),1).'" name="lang_messages.xml">'."\n";
		$message_file .= str_replace('<?xml version="1.0" ?>', '',
							file_get_contents($loc_file.'/'.str_replace('_', '/', str_replace('@', '/', $locale)).'/lang_messages.xml')."\n");
		$message_file .= '</file>'."\n";
	}

	$message_file .= '</files>'."\n";

	create_directory($locale_backup_dir.'/'.$locale);
	string_to_file($message_file, $locale_backup_dir.'/'.$locale.'/internal_messages.xml');
}

// finally, localised screens...
foreach($screens as $locale => $loc_files) {
	$loc_files = array_unique($loc_files);
	if (!empty($locale_list) && !in_array('screens', array_get_index($locale_list, $locale, Array())) &&
		!in_array('all', array_get_index($locale_list, $locale, Array()))) {
		continue;
	}
	$message_file = '<?xml version="1.0" ?>'."\n";
	$message_file .= '<files locale="'.$locale.'">'."\n";

	foreach($loc_files as $loc_file) {
		$folder = substr($loc_file, 0, strpos($loc_file, '/locale/') + strlen('/locale'));
		$file_name = substr($loc_file, strrpos($loc_file, '/') + 1);

		$message_file .= '<file source="'.substr(str_replace(SQ_SYSTEM_ROOT, '', $folder),1).'" name="'.$file_name.'">'."\n";
		$message_file .= str_replace('<?xml version="1.0" ?>', '',
							file_get_contents($loc_file)."\n");
		$message_file .= '</file>'."\n";
	}

	$message_file .= '</files>'."\n";

	create_directory($locale_backup_dir.'/'.$locale);
	string_to_file($message_file, $locale_backup_dir.'/'.$locale.'/screens.xml');
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
	$list = Array('locale' => Array(), 'dir' => '');

	foreach ($options as $option) {
		// if nothing set, skip this entry
		if (!isset($option[0]) || !isset($option[1])) continue;

		switch($option[0]) {
			case '--locale':
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

			case '--output':
				$list['dir'] = $option[1];
			break;
		}

	}

	return $list;

}//end get_console_list()
?>
