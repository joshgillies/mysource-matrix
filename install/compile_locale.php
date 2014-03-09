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
* $Id: compile_locale.php,v 1.21 2012/08/30 01:11:22 ewang Exp $
*
*/


/**
* Install: Compile Locale script (formerly step 4)
*
* Compiles languages on the system
*
* @author  Luke Wright <lwright@squiz.net>
* @version $Revision: 1.21 $
* @package MySource_Matrix
* @subpackage install
*/
ini_set('memory_limit', -1);
error_reporting(E_ALL);
$SYSTEM_ROOT = '';

if ((php_sapi_name() == 'cli')) {
	if (isset($_SERVER['argv'][1])) {
		$SYSTEM_ROOT = $_SERVER['argv'][1];
	}
	$err_msg = "ERROR: You need to supply the path to the System Root as the first argument.\n";

} else {
	$err_msg = '
	<div style="background-color: red; color: white; font-weight: bold;">
		You can only run the '.$_SERVER['argv'][0].' script from the command line
	</div>
	';
	echo $err_msg;
	exit(1);
}

if (empty($SYSTEM_ROOT)) {
	$err_msg .= "Usage: php install/compile_locale.php <PATH_TO_MATRIX>\n";
	echo $err_msg;
	exit(1);
}

if (!is_dir($SYSTEM_ROOT) || !is_readable($SYSTEM_ROOT.'/core/include/init.inc')) {
	$err_msg = "ERROR: Path provided doesn't point to a Matrix installation's System Root. Please provide correct path and try again.\n";
	$err_msg .= "Usage: php install/compile_locale.php <PATH_TO_MATRIX>\n";
	echo $err_msg;
	exit(1);
}

require_once 'Console/Getopt.php';

$shortopt = '';
$longopt = Array('locale=');

$con = new Console_Getopt;
$args = $con->readPHPArgv();
array_shift($args);			// remove the system root
$options = $con->getopt($args, $shortopt, $longopt);

if (is_array($options[0])) {
	$locale_list = get_console_list($options[0]);
}

if (empty($locale_list)) {
	echo "\nWARNING: You did not specify a --locale parameter.\n";
	echo "This is okay but be aware that all locales will be compiled, which may take a while if you have multiple locales on your system.\n\n";
}

// dont set SQ_INSTALL flag before this include because we want
// a complete load now that the database has been created

define('SQ_SYSTEM_ROOT',  $SYSTEM_ROOT);
require_once $SYSTEM_ROOT.'/core/include/init.inc';

// firstly let's check that we are OK for the version
if (version_compare(PHP_VERSION, SQ_REQUIRED_PHP_VERSION, '<')) {
	trigger_error('<i>'.SQ_SYSTEM_LONG_NAME.'</i> requires PHP Version '.SQ_REQUIRED_PHP_VERSION.'.<br/> You may need to upgrade.<br/> Your current version is '.PHP_VERSION, E_USER_ERROR);
}

// Clean up any remembered data.
require_once $SYSTEM_ROOT.'/core/include/deja_vu.inc';
$deja_vu = new Deja_Vu();
if ($deja_vu->enabled()) {
	$deja_vu->forgetAll();
}

// get the list of functions used during install
require_once $SYSTEM_ROOT.'/install/install.inc';
require_once SQ_FUDGE_PATH.'/general/file_system.inc';

// let everyone know we are installing
$GLOBALS['SQ_SYSTEM']->setRunLevel(SQ_RUN_LEVEL_FORCED);

// regenerate the configs
if (!regenerate_configs()) {
	trigger_error('Config Generation Failed', E_USER_ERROR);
}

// list of languages where we need to compile these for
$string_locales = Array();
$error_locales  = Array();
$message_locales = Array();

$exitRC = 0;
$errors = array();
// flag that controls when 'compiling edit interfaces' message is printed
$first_ei = TRUE;

$asset_screen_dir = SQ_DATA_PATH.'/private/asset_types/asset/localised_screens';
if (!create_directory($asset_screen_dir, FALSE)) {
	$errors[] = 'Unable to create directory '.$asset_screen_dir.'.';
	$exitRC = 1;
}

// do it for each asset type ...
$asset_types = $GLOBALS['SQ_SYSTEM']->am->getAssetTypes();

// ... but also, give the asset type array a little base asset injection
$base_asset = Array(
				'type_code'	=> 'asset',
				'dir'		=> 'core/include/asset_edit',
				'name'		=> 'Base Asset',
			  );
array_unshift($asset_types, $base_asset);

// also add top-level global strings - this will not appear on screen as there
// are no static screens for it (as it is not really an asset), but is important
// to catch languages which may only have strings in the top level
$global_strings = Array(
					'type_code'	=> '',
					'dir'		=> 'core',
					'name'		=> 'Global Strings',
				  );
array_unshift($asset_types, $global_strings);

$locale_names = array_keys($locale_list);
foreach ($locale_names as $locale) {
	list($country,$lang,$variant) = $GLOBALS['SQ_SYSTEM']->lm->getLocaleParts($locale);
	if (!in_array($country, $locale_names)) {
		$locale_list[$country] = $locale_list[$locale];
	}

	if (!empty($lang)) {
		if (!in_array($country.'_'.$lang, $locale_names)) {
			$locale_list[$country.'_'.$lang] = $locale_list[$locale];
		}

		if (!empty($variant)) {
			if (!in_array($country.'_'.$lang.'@'.$variant, $locale_names)) {
				$locale_list[$country.'_'.$lang.'@'.$variant] = $locale_list[$locale];
			}
		}
	}
}

foreach ($asset_types as $asset_type) {

	$type_code = $asset_type['type_code'];

	$local_screen_dir = SQ_DATA_PATH.'/private/asset_types/'.$type_code.'/localised_screens';
	$matches = Array();
	$screens = Array();

	$base_path = SQ_SYSTEM_ROOT.'/'.$asset_type['dir'].'/locale';
	$dirs_to_read = Array($base_path);

	while (!empty($dirs_to_read)) {
		$dir_read = array_shift($dirs_to_read);

		$d = @opendir($dir_read);
		if ($d) {

			// work out the locale name by taking the directory and replacing
			// the slashes with the appropriate underscore and (possibly) at sign
			$locale_name = str_replace($base_path.'/', '', $dir_read);
			if (($slash_pos = strpos($locale_name, '/')) !== FALSE) {
				$locale_name{$slash_pos} = '_';
				if (($slash_pos = strpos($locale_name, '/')) !== FALSE) {
					$locale_name{$slash_pos} = '@';
				}
			}

			while (FALSE !== ($entry = readdir($d))) {
				if (($entry{0} == '.') || ($entry == 'CVS')) {
					continue;
				}

				if (is_dir($dir_read.'/'.$entry)) {
					$dirs_to_read[] = $dir_read.'/'.$entry;
				}

				if (preg_match('|^lang\_((static_)?screen\_.*)\.xml$|', $entry, $matches)) {
					if (!empty($locale_list) && (!in_array($locale_name, array_keys($locale_list))
						|| (!in_array('all', $locale_list[$locale_name])
						&& !in_array('screens', $locale_list[$locale_name])))) {
						continue;
					}

					if (!isset($screens[$locale_name])) {
						$screens[$locale_name] = Array();
					}

					$screens[$locale_name][] = Array(
												'dir'		=> $dir_read,
												'screen'	=> $matches[1],
											   );
				} else if (preg_match('|^lang\_strings\.xml$|', $entry, $matches)) {
					list($country,$lang,$variant) = $GLOBALS['SQ_SYSTEM']->lm->getLocaleParts($locale_name);
					if (!in_array($country, $string_locales)) {
						$string_locales[] = $country;
					}

					if (!empty($lang)) {
						if (!in_array($country.'_'.$lang, $string_locales)) {
							$string_locales[] = $country.'_'.$lang;
						}

						if (!empty($variant)) {
							if (!in_array($country.'_'.$lang.'@'.$variant, $string_locales)) {
								$string_locales[] = $country.'_'.$lang.'@'.$variant;
							}
						}
					}
				} else if (preg_match('|^lang\_errors\.xml$|', $entry, $matches)) {
					list($country,$lang,$variant) = $GLOBALS['SQ_SYSTEM']->lm->getLocaleParts($locale_name);
					if (!in_array($country, $error_locales)) {
						$error_locales[] = $country;
					}

					if (!empty($lang)) {
						if (!in_array($country.'_'.$lang, $error_locales)) {
							$error_locales[] = $country.'_'.$lang;
						}

						if (!empty($variant)) {
							if (!in_array($country.'_'.$lang.'@'.$variant, $error_locales)) {
								$error_locales[] = $country.'_'.$lang.'@'.$variant;
							}
						}
					}
				} else if (preg_match('|^lang\_messages\.xml$|', $entry, $matches)) {
					list($country,$lang,$variant) = $GLOBALS['SQ_SYSTEM']->lm->getLocaleParts($locale_name);
					if (!in_array($country, $message_locales)) {
						$message_locales[] = $country;
					}

					if (!empty($lang)) {
						if (!in_array($country.'_'.$lang, $message_locales)) {
							$message_locales[] = $country.'_'.$lang;
						}

						if (!empty($variant)) {
							if (!in_array($country.'_'.$lang.'@'.$variant, $message_locales)) {
								$message_locales[] = $country.'_'.$lang.'@'.$variant;
							}
						}
					}
				}
			}//end while

			closedir($d);

		}//end if
	}//end while

	$all_screens = Array();
	$d = @opendir(SQ_SYSTEM_ROOT.'/'.$asset_type['dir']);
	if ($d) {

		while (FALSE !== ($entry = readdir($d))) {
			if (preg_match('|^edit\_interface\_((static_)?screen\_.*).xml$|', $entry, $matches)) {
				$all_screens[] = $matches[1];
			}
		}
		closedir($d);

	}

	// if there are edit interface files AND there are screens that we are localising...
	if (!empty($all_screens) && !empty($screens)) {
		if ($first_ei) {
			$first_ei = FALSE;
			echo 'Compiling localised edit interfaces.. ';
		}
	}

	if (!empty($screens)) {
		foreach ($screens as $locale => $locale_screens) {

			foreach ($locale_screens as $screen_type) {
				if (!file_exists($local_screen_dir)) {
					if (!create_directory($local_screen_dir, FALSE)) {
						$errors[] = 'Unable to create directory '.$local_screen_dir.' for \''.$screen_type['screen'].'\' screen.';
						$exitRC = 1;
						continue;
					}
				}

				$screen_xml = NULL;
				if (strpos($screen_type['screen'], 'static_') === 0) {
					$screen_xml = build_localised_static_screen($type_code, $screen_type['screen'], $locale);
				} else {
					$screen_xml = build_localised_screen($type_code, $screen_type['screen'], $locale);
				}

				$result = string_to_file($screen_xml->asXML(), $local_screen_dir.'/'.$screen_type['screen'].'.'.$locale);
				if (!$result) {
					$exitRC = 1;
					$errors[] = 'Unable to save file '.$local_screen_dir.'/'.$screen_type['screen'].'.'.$locale;
				}
			}
		}
	}
}//end foreach asset_type

if (empty($errors)) {
	echo "Done.\n";
} else {
	echo "Done, but with errors.\n";
}

// compile the strings for each locale where a lang_strings.xml exists
foreach ($string_locales as $locale) {
	if (!empty($locale_list) && (!in_array($locale, array_keys($locale_list))
		|| (!in_array('all', $locale_list[$locale])
		&& !in_array('strings', $locale_list[$locale])))) {
		continue;
	}

	echo 'Compiling strings for locale '.$locale.'.. ';
	$build_errors = build_locale_string_file($locale);
	if (!empty($build_errors)) {
		echo "Done, but with errors.\n";
		$exitRC = 1;
		$errors = array_merge($errors, $build_errors);
	} else {
		echo "Done.\n";
	}
}

// then, compile errors for each locale (using lang_errors.xml)
foreach ($error_locales as $locale) {
	if (!empty($locale_list) && (!in_array($locale, array_keys($locale_list))
		|| (!in_array('all', $locale_list[$locale])
		&& !in_array('errors', $locale_list[$locale])))) {
		continue;
	}

	echo 'Compiling localised errors for locale '.$locale.'.. ';
	$build_errors = build_locale_error_file($locale);
	if (!empty($build_errors)) {
		echo "Done, but with errors.\n";
		$exitRC = 1;
		$errors = array_merge($errors, $build_errors);
	} else {
		echo "Done.\n";
	}
}

// finally, compile internal messages for each locale (using lang_messages.xml)
foreach ($message_locales as $locale) {
if (!empty($locale_list) && (!in_array($locale, array_keys($locale_list))
		|| (!in_array('all', $locale_list[$locale])
		&& !in_array('messages', $locale_list[$locale])))) {
		continue;
	}

	echo 'Compiling localised internal messages for locale '.$locale.'.. ';
	$build_errors = build_locale_internal_messages_file($locale);
	if (!empty($build_errors)) {
		echo "Done, but with errors.\n";
		$exitRC = 1;
		$errors = array_merge($errors, $build_errors);
	} else {
		echo "Done.\n";
	}
}

$GLOBALS['SQ_SYSTEM']->restoreRunLevel();

if ($exitRC == 0) {
	echo "\n";
	echo "compile_locale.php completed successfully.\n";
	echo "\n";
} else {
	echo "\n";
	echo "compile_locale.php had errors.\n";
	echo "\n";
	trigger_error(implode("\n", $errors), E_USER_ERROR);
	exit($exitRC);
}

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
	$list = Array();

	foreach ($options as $option) {
		// if nothing set, skip this entry
		if (!isset($option[0]) || !isset($option[1])) {
			continue;
		}

		if ($option[0] != '--locale') continue;

		// Now process the list
		$parts = explode('-', $option[1]);

		$types = Array();
		if (count($parts) == 2 && strlen($parts[1])) {
			$types = explode(',', $parts[1]);
		} else {
			$types = Array('all');
		}

		$list[$parts[0]] = $types;
	}

	return $list;

}//end get_console_list()


?>
