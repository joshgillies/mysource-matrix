<?php
/**
* +--------------------------------------------------------------------+
* | This MySource Matrix CMS file is Copyright (c) Squiz Pty Ltd       |
* | ACN 084 670 600                                                    |
* +--------------------------------------------------------------------+
* | IMPORTANT: Your use of this Software is subject to the terms of    |
* | the Licence provided in the file licence.txt. If you cannot find   |
* | this file please contact Squiz (www.squiz.net) so we may provide   |
* | you a copy.                                                        |
* +--------------------------------------------------------------------+
*
* $Id: check_requirements.php,v 1.3 2009/09/23 04:27:46 csmith Exp $
*
*/

/**
 * Requirements checking script
 * It goes through the core package and all other packages
 * to make sure requirements (or suggested packages) are installed
 *
 * This will help work out what's missing from a server
 *
 * @author  Chris Smith <csmith@squiz.net>
 * @version $Revision: 1.3 $
 * @package MySource_Matrix
 * @subpackage install
 */
error_reporting(E_ALL);
$SYSTEM_ROOT = '';

if ((php_sapi_name() == 'cli')) {
	if (isset($_SERVER['argv'][1])) {
		$SYSTEM_ROOT = $_SERVER['argv'][1];
	}

	$err_msg = "You need to supply the path to the System Root as the first argument\n";

} else {
	if (isset($_GET['SYSTEM_ROOT'])) {
		$SYSTEM_ROOT = $_GET['SYSTEM_ROOT'];
	}

	$err_msg = '
	<div style="background-color: red; color: white; font-weight: bold;">
		You need to supply the path to the System Root as a query string variable called SYSTEM_ROOT
	</div>
	';
}

if (empty($SYSTEM_ROOT) || !is_dir($SYSTEM_ROOT)) {
	trigger_error($err_msg, E_USER_ERROR);
}

/**
 * This is used later on to keep track of what's missing
 * broken into categories.
 */
$missing_modules = array (
	'php_extension' => array (
		'required' => array(),
		'suggested' => array(),
		'out_of_date' => array(),
	),
	'pear_package' => array (
		'required' => array(),
		'suggested' => array(),
		'out_of_date' => array(),
	),
	'external_program' => array (
		'required' => array(),
		'suggested' => array(),
		'out_of_date' => array(),
	),
);

define('SQ_PACKAGES_PATH', $SYSTEM_ROOT . '/packages');
require_once $SYSTEM_ROOT.'/install/install.inc';

/**
 * Get a list of packages so we can see
 * if they have any specific requirements.
 */
$packages = get_package_list();

/**
 * Check the core package
 */
check_requirement_file($SYSTEM_ROOT.'/core/assets/requirements.xml', 'core');

/**
 * Now check for any specific package requirements.
 */
foreach ($packages as $package) {
	$xml_file = SQ_PACKAGES_PATH.'/'.$package.'/requirements.xml';
	if (file_exists($xml_file)) {
		check_requirement_file($xml_file, $package);
	}
}

/**
 * Now we print the report out.
 * Go through the module types
 * then the categories (required/suggested/out_of_date)
 * to work out what to show.
 */
$anything_missing = false;

$check_types = array_keys($missing_modules);
foreach ($check_types as $check_type) {
	$types = array_keys($missing_modules[$check_type]);

	switch ($check_type)
	{
		case 'php_extension':
			$prefix = 'These php extensions are ';
		break;
		case 'pear_package':
			$prefix = 'These pear packages are ';
		break;
		case 'external_program':
			$prefix = 'These external programs are ';
		break;
	}

	foreach ($types as $type) {
		if (empty($missing_modules[$check_type][$type])) {
			continue;
		}

		if ($anything_missing === false) {
			echo "The following requirements or suggestions are made:\n\n";
			$anything_missing = true;
		}

		switch ($type)
		{
			case 'required':
			case 'suggested':
				$heading = $type;
				$suffix = " by the %s package";
			break;
			case 'out_of_date':
				$heading = "found, but out of date";
				$suffix = " for the %s package";
			break;
		}

		/**
		 * We're going to sort the list of items
		 * so they'll be put into categories.
		 */
		$list = array();

		foreach ($missing_modules[$check_type][$type] as $item) {
			$req_by = $item['required_by'];
			if (!isset($list[$req_by])) {
				$list[$req_by] = array();
			}
			unset($item['required_by']);
			$list[$req_by][] = $item;
			sort($list[$req_by]);
		}

		asort($list);

		foreach ($list as $req_by => $details) {
			printf($prefix . $heading . $suffix . "\n", $req_by);
			echo str_repeat('-', 20) . "\n";
			foreach ($details as $item) {
				echo "\t" . $item['name'];
				if (isset($item['version_found'])) {
					echo " (Found version " . $item['version_found'] . " but require at least " . $item['version_required'] . ")";
				}
				echo "\n";
			}
			echo "\n";
		}
	}
}

/**
 * Everything's good to go! Yay!
 */
if (!$anything_missing) {
	echo "Everything appears to be available and up to date.\n";
}

/**
 * check_requirement_file
 * Tries to load up the file passed in and parse it out as simple xml.
 * Then, passes the requirement list to the check_requirement function
 * which does all of the actual work.
 *
 * @param $file String The full path to the xml file to load up
 * @param $package_name String The name of the package we're loading the xml for. Used later for reporting.
 *
 * @return Returns void. Returns early if a file doesn't exist or can't be parsed.
 */
function check_requirement_file($file='', $package_name='core')
{
	if (!is_file($file)) {
		trigger_error("File '".$file."' doesn't exist", E_USER_WARNING);
		return;
	}

	try {
		$requirement_list = new SimpleXMLElement($file, LIBXML_NOCDATA, TRUE);
	} catch (Exception $e) {
		trigger_error('Could not parse requirements XML file (' . $file . '): '.$e->getMessage(), E_USER_WARNING);
		return;
	}

	foreach ($requirement_list->requirement as $requirement_check) {
		check_requirement($requirement_check, $package_name);
	}
}

/**
 * check_requirement
 * This goes through the requirements passed in
 * to see if they are met.
 *
 * The following check_type's are allowed:
 * - php_extension
 * - pear_package
 * - external_program
 *
 * A php extension is the name of the module to load (eg 'pgsql'), not a common name nor a specific function name.
 * A pear package is checked against what is installed with pear.
 * Only 3 external programs are currently supported
 * - tidy
 * - antiword
 * - pdftohtml
 *
 * mainly because each program has it's own way of specifying the 'version' switches and it's own version number
 * so if you add a new program as a requirement, it needs to be added here.
 *
 * As requirements are checked, they are added to the missing_modules array
 * which is later printed out as a report of what is missing or out of date
 * (for external_program's and pear_package's).
 */
function check_requirement($requirement_check, $package_name='core')
{
	global $missing_modules;

	/**
	 * Cache the pear package list and also the php extension list.
	 */
	static $pear_package_list = array();
	static $php_extension_list = array();

	if (empty($pear_package_list)) {
		$rc = -1;
		$pear_list = array();
		exec('pear list 2>/dev/null', $pear_list, $rc);

		/**
		 * No 'pear' installed? No point checking anything else really.
		 */
		if (empty($pear_list) || $rc !== 0) {
			$msg = "Unable to exec the 'pear list' command.\n";
			$msg .= "Please check pear is installed and in your path then try again.\n";
			echo $msg;
			exit(1);
		}

		/**
		 * lines 1-3 from 'pear list' are always
		 *
		 * INSTALLED PACKAGES, CHANNEL PEAR.PHP.NET:
		 * =========================================
		 * PACKAGE           VERSION STATE
		 *
		 * so we can unset those.
		 */
		unset($pear_list[0]);
		unset($pear_list[1]);
		unset($pear_list[2]);

		/**
		 * Now go through the rest of the list and turn them into an array.
		 * The lines look like
		 * PACKAGE           VERSION STATE
		 *
		 * and package names never have spaces so we can split on spaces and get the name & version
		 */
		foreach ($pear_list as $line) {
			$line = preg_replace('/\s+/', ' ', $line);
			list($name, $version, $state) = explode(' ', $line);
			$pear_package_list[strtolower($name)] = $version;
		}
	}

	if (empty($php_extension_list)) {
		$php_extension_list = get_loaded_extensions();
	}

	$check_type = null;
	if (isset($requirement_check->php_extension)) {
		$check_type = 'php_extension';
	}

	if (isset($requirement_check->pear_package)) {
		$check_type = 'pear_package';
	}

	if (isset($requirement_check->external_program)) {
		$check_type = 'external_program';
	}

	$check_ok = false;
	$check_alternative = true;
	$check_version = false;

	$extra_info = '';

	$description = '';
	if ($requirement_check->description) {
		$description = (string)$requirement_check->description;
	}

	switch ($check_type)
	{
		case 'php_extension':
			$check_name = (string)$requirement_check->php_extension;
			if (in_array($check_name, $php_extension_list)) {
				$check_ok = true;
				if (!$requirement_check->suggested) {
					$check_alternative = false;
				}
				break;
			}
		break;

		case 'pear_package':
			$check_name = strtolower($requirement_check->pear_package);

			if (isset($pear_package_list[$check_name])) {
				$check_version = true;
				$check_ok = true;
				$version_found = $pear_package_list[$check_name];
				$version_required = (string)$requirement_check->version;
			}
		break;

		case 'external_program':
			$external_program = (string)$requirement_check->external_program;

			$version_required = (string)$requirement_check->version;

			$cmd = $external_program;
			$check_name = $external_program;

			if (isset($requirement_check->version_arguments)) {
				$cmd .= ' ' . escapeshellarg((string)$requirement_check->version_arguments);
			}

			if (isset($requirement_check->uses_stderr)) {
				$cmd .= ' 2>&1';
			}

			$cmd_output = array();
			$cmd_return_code = -1;
			exec($cmd, $cmd_output, $cmd_return_code);

			$expected_return_code = 0;
			if (isset($requirement_check->expected_return_code)) {
				$expected_return_code = $requirement_check->expected_return_code;
			}

			/**
			 * If we don't get the right return code, or the output is empty
			 * break here
			 * it'll be marked as 'invalid' (depending on the 'suggested' setting)
			 */
			if ($cmd_return_code != $expected_return_code || empty($cmd_output)) {
				break;
			}

			$check_version = true;

			/**
			 * External program have no set way to return version numbers or
			 * even how they specify version numbers
			 * So checking versions is hardcoded for those.
			 */
			switch ($external_program)
			{
				/**
				 * pdftohtml version output looks like
				 *
				 * $ pdftohtml -v
				 * pdftohtml version 0.10.5
				 * Copyright 2005-2009 The Poppler Developers - http://poppler.freedesktop.org
				 * Copyright 1999-2003 Gueorgui Ovtcharov and Rainer Dorsch
				 * Copyright 1996-2004 Glyph & Cog, LLC
				 *
				 * or
				 *
				 * $ pdftohtml -v
				 * pdftohtml version 0.36 http://pdftohtml.sourceforge.net/, based on Xpdf version 3.00
				 * Copyright 1999-2003 Gueorgui Ovtcharov and Rainer Dorsch
				 * Copyright 1996-2004 Glyph & Cog, LLC
				 *
				 */
				case 'pdftohtml':
					$check_ok = true;

					$version_line = $cmd_output[0];
					/**
					 * The version number is the 3rd column (based on space separated values)
					 */
					list($intro, $version_kw, $version_found) = explode(' ', $version_line);
				break;

				/**
				 * tidy version output looks like
				 *
				 * $ tidy -v
				 * HTML Tidy for Linux/x86 released on 7 December 2008
				 *
				 */
				case 'tidy':
					/**
					 * We're going to do our own version checking here
					 * so disable the default one.
					 */
					$check_version = false;

					$version_line = $cmd_output[0];
					/**
					 * The version number is the string after 'released on '
					 */
					$match_found = preg_match('/released on (.*)$/', $version_line, $matches);
					if (!$match_found) {
						$extra_info = " (version checking not working)";
						break;
					}
					$check_ok = true;

					/**
					 * Since we're dealing with dates here,
					 * do our own version checks.
					 *
					 * If it's not recent enough,
					 * add it to the out of date list ourselves.
					 */
					$version_found = $matches[1];
					$version_found_date = strtotime($version_found);
					$version_required_date = strtotime($version_required);

					if ($version_found_date < $version_required_date) {
						$missing_modules['external_program']['out_of_date'][] = array(
							'name' => $external_program,
							'version_found' => $version_found,
							'version_required' => $version_required,
							'required_by' => $package_name,
						);
					}
				break;

				/**
				 * antiword -h looks like
				 *
				 * $ antiword -h
				 * Name: antiword
				 * Purpose: Display MS-Word files
				 * Author: (C) 1998-2005 Adri van Os
				 * Version: 0.37  (21 Oct 2005)
				 * Status: GNU General Public License
				 * .....
				 *
				 * The version is always on the 4th line (but php is zero based, so line #3).
				 */
				case 'antiword':
					if (!isset($cmd_output[3])) {
						$extra_info = " (version checking not working)";
						break;
					}

					$version_line = $cmd_output[3];
					$match_found = preg_match('/Version: (.*?)\s+/', $version_line, $matches);
					if ($match_found) {
						$check_ok = true;
						$version_found = $matches[1];
					} else {
						$extra_info = " (version checking not working)";
					}
				break;

				/**
				 * squidclient looks like this
				 * $ squidclient 2>&1
				 * Usage: squidclient [-arsv] [-i IMS] [-h remote host] [-l local host] [-p port] [-m method] [-t count] [-I ping-interval] [-H 'strings'] [-T timeout] url
				 *
				 * there's no version number for the version in debian etch.
				 * So all we can do is check it's available.
				 */
				case 'squidclient':
					$check_ok = true;
					$check_version = false;
				break;

				/**
				 * clamscan looks like this
				 * $ clamscan --version
				 * ClamAV 0.94.2/9824/Wed Sep 23 10:50:20 2009
				 *
				 */
				case 'clamscan':
					$version_line = $cmd_output[0];
					$match_found = preg_match('%ClamAV (.*?)/%', $version_line, $matches);
					if ($match_found) {
						$check_ok = true;
						$version_found = $matches[1];
					} else {
						$extra_info = " (version checking not working)";
					}
				break;

				/**
				 * $ fpscan --version
				 *
				 * F-PROT Antivirus version 6.2.1.4252 (built: 2008-04-28T16-44-10)
				 * FRISK Software International (C) Copyright 1989-2007
				 *
				 * Engine version: 4.4.4.56
				 * Virus signatures: 20090922174870b7cbcfe94821d0361d46523945909c
				 *                   (/home/csmith/matrix/fp/f-prot/antivir.def)
				 *
				 */
				case 'fpscan':
					if (!isset($cmd_output[1])) {
						$extra_info = " (version checking not working)";
						break;
					}
					$version_line = $cmd_output[1];
					$match_found = preg_match('/ version (.*?) /', $version_line, $matches);
					if ($match_found) {
						$check_ok = true;
						$version_found = $matches[1];
					} else {
						$extra_info = " (version checking not working)";
					}

				break;

				default:
					$missing_modules['external_program']['required'][] = array (
						'name' => $external_program . " (Unknown program, can't check version numbers)",
						'required_by' => $package_name,
					);
					return;
				break;

			}
		break;

		case null:
			echo "Unknown type ('" . $check_type . "')\n";
			return;
		break;

	}

	if ($check_version) {
		$version_check = version_compare($version_found, $version_required, '>=');
		if ($version_check !== true) {
			$missing_modules[$check_type]['out_of_date'][] = array(
				'name' => $check_name . $description . $extra_info,
				'version_found' => $version_found,
				'version_required' => $version_required,
				'required_by' => $package_name
			);
		}
	}

	if (!$check_ok) {
		if ($requirement_check->suggested) {
			$missing_modules[$check_type]['suggested'][] = array(
				'name' => $check_name . $description . $extra_info,
				'required_by' => $package_name
			);
		} else {
			$missing_modules[$check_type]['required'][] = array(
				'name' => $check_name . $description . $extra_info,
				'required_by' => $package_name
			);
		}
	}

	/**
	 * If we should check the alternative requirement
	 * AND there is one defined..
	 * do it!
	 */
	if ($check_alternative && $requirement_check->alternative) {
		check_requirement($requirement_check->alternative, $package_name);
	}
}

