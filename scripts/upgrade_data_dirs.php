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
* $Id: upgrade_data_dirs.php,v 1.6.2.5 2007/04/11 00:33:22 skim Exp $
*
*/

/**
*
* @author Scott Kim <skim@squiz.net>
* @version $Revision: 1.6.2.5 $
* @package MySource_Matrix
*/
error_reporting(E_ALL);
if ((php_sapi_name() != 'cli')) {
	trigger_error("You can only run this script from the command line\n", E_USER_ERROR);
}

require_once 'Console/Getopt.php';

$shortopt = 's:';
$longopt = Array('report-only', 'start-upgrade', 'show-details', 'fix-lookups');

$args = Console_Getopt::readPHPArgv();
array_shift($args);
$options = Console_Getopt::getopt($args, $shortopt, $longopt);
if (empty($options[0])) usage();

$REPORT_ONLY = FALSE;
$START_UPGRADE = FALSE;
$SHOW_DETAILS = FALSE;
$FIX_LOOKUPS = FALSE;
$_SYSTEM_ROOT = '';
foreach ($options[0] as $option) {
	switch ($option[0]) {
		case 's':
			if (empty($option[1])) usage();
			if (!is_dir($option[1])) usage();
			$_SYSTEM_ROOT = $option[1];
		break;
		case '--report-only':
			$REPORT_ONLY = TRUE;
		break;
		case '--start-upgrade':
			$START_UPGRADE = TRUE;
		break;
		case '--show-details':
			$SHOW_DETAILS = TRUE;
		break;
		case '--fix-lookups':
			$FIX_LOOKUPS = TRUE;
		break;
	}

}
if (empty($_SYSTEM_ROOT)) usage();


ini_set('memory_limit', '256M');
define('SQ_SYSTEM_ROOT', $_SYSTEM_ROOT);
define('SQ_LOG_PATH', '');
include_once $_SYSTEM_ROOT.'/data/private/conf/main.inc';

$data_dirs = Array('/data/private/assets', '/data/public/assets', '/data/file_repository/assets');
$lookup_tables = Array('sq_ast_lookup', 'sq_ast_lookup_value', 'sq_ast_lookup_remap');
$skip_dirs = Array('.stop_pruning', '.cvsignore', '.', '..', 'CVS', 'cvs');

$log = read_log();
$count = Array(
			'total_count'				=> 0,
			'total_count_ex'			=> 0,
			'total_query_count'			=> 0,
			'total_query_count_ex'		=> 0,
			'data_private_count'		=> 0,
			'data_private_count_ex'		=> 0,
			'data_public_count'			=> 0,
			'data_public_count_ex'		=> 0,
			'file_vers_count'			=> 0,
			'file_vers_count_ex'		=> 0,
			'file_vers_query_count'		=> 0,
			'file_vers_query_count_ex'	=> 0,
			'sq_ast_lookup'				=> 0,
			'sq_ast_lookup_ex'			=> 0,
			'sq_ast_lookup_value'		=> 0,
			'sq_ast_lookup_value_ex'	=> 0,
			'sq_ast_lookup_remap'		=> 0,
			'sq_ast_lookup_remap_ex'	=> 0,
		 );


// Fix wrong lookup values due to the wrong hash function
if ($FIX_LOOKUPS) {
	require_once 'DB.php';
	$db = NULL;
	$db =& DB::connect(SQ_CONF_DB2_DSN);
	if (PEAR::isError($db)) {
		print_log("[ERROR Failed to connect to database] ".$db->getMessage(), 0, TRUE);
		$db->disconnect();
		exit();
	}
	$db->autoCommit(FALSE);


	// Process private/public assets data directories
	foreach ($data_dirs as $data_path) {
		if ($dh = opendir(SQ_SYSTEM_ROOT.$data_path)) {
			while (false !== ($asset_type = readdir($dh))) {
				if (array_search($asset_type, $skip_dirs) !== FALSE) continue;

				// $file == asset type
				if ($dh2 = opendir(SQ_SYSTEM_ROOT.$data_path.'/'.$asset_type)) {
					while (false !== ($hash = readdir($dh2))) {
						if (array_search($hash, $skip_dirs) !== FALSE) continue;

						// $file == asset type
						if ($dh3 = opendir(SQ_SYSTEM_ROOT.$data_path.'/'.$asset_type.'/'.$hash)) {
							while (false !== ($assetid = readdir($dh3))) {
								if (array_search($assetid, $skip_dirs) !== FALSE) continue;

								fix_lookups($asset_type, $hash, $assetid);

							}//end while assetid
							closedir($dh3);

						}
					}//end while hash
					closedir($dh2);

				}
			}//end while asset_type

			closedir($dh);
		}//end if opendir

	}//end foreach()
	$db->disconnect();
	exit();
}

generate_report();
if (!$START_UPGRADE) exit();

require_once 'DB.php';
$db = NULL;
$db =& DB::connect(SQ_CONF_DB2_DSN);
if (PEAR::isError($db)) {
	print_log("[ERROR Failed to connect to database] ".$db->getMessage(), 0, TRUE);
	$db->disconnect();
	exit();
}
$db->autoCommit(FALSE);

// Process private/public assets data directories
foreach ($data_dirs as $data_path) {
	if ($dh = opendir(SQ_SYSTEM_ROOT.$data_path)) {
		while (false !== ($file = readdir($dh))) {
			if (array_search($file, $skip_dirs) !== FALSE) continue;

			if (is_dir(SQ_SYSTEM_ROOT.$data_path.'/'.$file)) {
				print_log("[START] $data_path/$file", 0);

				if ($data_path == '/data/file_repository/assets') {
					process_file_versioning($data_path, $file);
				} else {
					process($data_path, $file);
				}
			}
		}//end while readdir
		closedir($dh);

	}//end if opendir

}//end foreach()

if (!$REPORT_ONLY) {
	generate_report(TRUE);
}

// Disconnect DB
$db->disconnect();


/**
* Process data directories for assets
*
* @return void
* @access public
*/
function process($base_dir, $curr_dir, $handle_file_vers=FALSE)
{
	global $data_dirs, $skip_dirs, $log, $count;
	if ($dh = opendir(SQ_SYSTEM_ROOT.$base_dir.'/'.$curr_dir)) {

		require_once SQ_SYSTEM_ROOT.'/core/include/general.inc';
		require_once SQ_SYSTEM_ROOT.'/fudge/general/file_system.inc';

		while (FALSE !== ($assetid = readdir($dh))) {
			if (array_search($assetid, $skip_dirs) !== FALSE) continue;

			// Assumption. No assetid starts with zero
			if (substr($assetid, 0, 1) == '0') continue;

			$hash = get_asset_hash($assetid);
			$old_dir_path = SQ_SYSTEM_ROOT.$base_dir.'/'.$curr_dir.'/'.$assetid;
			$new_dir_path = SQ_SYSTEM_ROOT.$base_dir.'/'.$curr_dir.'/'.$hash.'/'.$assetid;

			if (_copy_over_folder($old_dir_path, $new_dir_path)) {
				if ($base_dir == '/data/private/assets') {
					$count['data_private_count']++;
				} else if ($base_dir == '/data/public/assets') {
					$count['data_public_count']++;
					_process_public_dir_lookup($base_dir, $curr_dir, $assetid);
				}

				print_log("[SUCCESS] $old_dir_path", 1);
			} else {
				_generate_result_report();
				exit();
			}


		}//end while readdir

	}//end if opendir

}//end process()


/**
* Process file versioning repository
*
* Need to update file table
*
* @return void
* @access public
*/
function process_file_versioning($base_dir, $curr_dir)
{
	global $data_dirs, $skip_dirs, $log, $db, $count;
	if ($dh = opendir(SQ_SYSTEM_ROOT.$base_dir.'/'.$curr_dir)) {

		require_once SQ_SYSTEM_ROOT.'/core/include/general.inc';
		require_once SQ_SYSTEM_ROOT.'/fudge/general/file_system.inc';

		while (FALSE !== ($assetid = readdir($dh))) {
			if (array_search($assetid, $skip_dirs) !== FALSE) continue;

			// Assumption. No assetid starts with zero
			if (substr($assetid, 0, 1) == '0') continue;

			$is_error = FALSE;
			if (is_dir(SQ_SYSTEM_ROOT.$base_dir.'/'.$curr_dir.'/'.$assetid)) {

				$hash = get_asset_hash($assetid);
				$old_dir_path = SQ_SYSTEM_ROOT.$base_dir.'/'.$curr_dir.'/'.$assetid;
				$new_dir_path = SQ_SYSTEM_ROOT.$base_dir.'/'.$curr_dir.'/'.$hash.'/'.$assetid;

				$old_file_vers_path = 'assets/'.$curr_dir.'/'.$assetid;
				$new_file_vers_path = 'assets/'.$curr_dir.'/'.$hash.'/'.$assetid;

				if (_copy_over_folder($old_dir_path, $new_dir_path)) {
					$count['file_vers_count']++;
					print_log("[SUCCESS] $old_dir_path", 1);
				} else {
					_generate_result_report();
					exit();
				}

				// Process sq_file_vers_file table
				$sql = 'SELECT
							fileid, path, filename
						FROM
							sq_file_vers_file
						WHERE
							path = '.$db->quoteSmart($old_file_vers_path);
				// Fix for image varieties
				if (file_exists($new_dir_path.'/varieties') && is_dir($new_dir_path.'/varieties')) {
					$sql .= '
						OR path = '.$db->quoteSmart($old_file_vers_path.'/varieties').'
					';
				}

				$result =& $db->getAll($sql);
				if (PEAR::isError($result)) {
					print_log("[$old_file_vers_path :(] SELECT query failed. ".$result->getMessage(), 0, TRUE);
					_generate_result_report();
					exit();
				}

				if (empty($result)) {
					print_log("[$old_file_vers_path :(] does not exist in sq_file_vers_file table.", 2);
				} else {
					foreach ($result as $file_data) {
						$fileid		= $file_data[0];
						$file_path	= $file_data[1];
						$file_name	= $file_data[2];

						if ($db->phptype != 'oci8') {
							$result =& $db->query('BEGIN');
							if (PEAR::isError($result)) {
								$db->disconnect();
								print_log("[fileid:$fileid :(] transaction can not start. ".$result->getMessage(), 0, TRUE);
								_generate_result_report();
								exit();
							}
						}

						// Fix for image varieties
						$varieties = FALSE;
						if (substr($file_path, -10) == '/varieties') {
							$varieties = TRUE;
							$this_file_vers_path = $new_file_vers_path.'/varieties';
						} else {
							$this_file_vers_path = $new_file_vers_path;
						}

						$sql = 'UPDATE
									sq_file_vers_file
								SET
									path = '.$db->quoteSmart($this_file_vers_path).'
								WHERE
									fileid = '.$db->quoteSmart($fileid);
						$result =& $db->query($sql);
						if (PEAR::isError($result)) {
							$result =& $db->rollback();
							$db->disconnect();
							print_log("[fileid:$fileid :( ] update FAILED. ".$result->getMessage(), 0, TRUE);
							_generate_result_report();
							exit();
						} else {
							$result = $db->commit();
							record_log($old_file_vers_path, 'path updated');
							print_log("[SUCCESS] fileid:$fileid update completed.", 2);
							if (!$varieties) {
								$count['file_vers_query_count']++;
							}
						}
					}

				}//end if
			}

		}//end while

	}//end if

}//end process_file_versioning()


/**
* Fix asset lookup values generated due to the wrong hash function
*
* @return void
* @access public
*/
function fix_lookups($asset_type, $hash, $assetid)
{
	global $data_dirs, $skip_dirs, $log, $db, $count;
	$sql = 'SELECT url FROM sq_ast_lookup WHERE assetid = '.$db->quoteSmart($assetid).';';
	$result =& $db->getAll($sql);
	if (empty($result)) return;

	$updated_url = Array();
	foreach ($result as $data) {
		$url = $data[0];
		if (($index = strpos($url, '/'.$assetid.'/')) !== FALSE) {
			$old_hash = substr($url, $index - 5, 6);

			if ($old_hash != '/'.$hash.'/') {
				$new_url = substr($url, 0, ($index)).'/'.$hash.substr($url, $index);

				// Fix lookup table
				$sql = 'UPDATE
							sq_ast_lookup
						SET
							url = '.$db->quoteSmart($new_url).'
						WHERE
							url = '.$db->quoteSmart($url);

				if ($db->phptype != 'oci8') {
					$result =& $db->query('BEGIN');
				}
				$result =& $db->query($sql);
				if (PEAR::isError($result)) {
					print_log($result->getMessage(), 0, TRUE);
					$db->rollback();
				} else {
					$db->commit();
					print_log("[FIX LOOKUP SUCCESS sq_ast_lookup]\nFROM: $url\nTO  : $new_url\n", 0);
					$updated_url[$url] = $new_url;
				}

			}
		}
	}

	if (!empty($updated_url)) {
		foreach ($updated_url as $url => $new_url) {
			foreach (Array('sq_ast_lookup_value', 'sq_ast_lookup_remap') as $table) {

				if ($table == 'sq_ast_lookup_value') {
					$sql = 'SELECT url FROM '.$table.' WHERE url = '.$db->quoteSmart($url).';';
				} else if ($table == 'sq_ast_lookup_remap') {
					$sql = 'SELECT url FROM '.$table.' WHERE url LIKE '.$db->quoteSmart('%'.$url).';';
				}

				$result =& $db->getAll($sql);
				if (PEAR::isError($result)) {
					echo "$sql\n";
					print_log($result->getMessage(), 0, TRUE);
				} else {
					foreach ($result as $data) {
						$url = $data[0];
						if ($table == 'sq_ast_lookup_value') {
							$sql = 'UPDATE
										'.$table.'
									SET
										url = '.$db->quoteSmart($new_url).'
									WHERE
										url = '.$db->quoteSmart($url);
						} else if ($table == 'sq_ast_lookup_remap') {
							$new_url = str_replace($url, $new_url, $url);
							$sql = 'UPDATE
										'.$table.'
									SET
										url = '.$db->quoteSmart($new_url).'
									WHERE
										url = '.$db->quoteSmart($url);
						}

						if ($db->phptype != 'oci8') {
							$result =& $db->query('BEGIN');
						}
						$result =& $db->query($sql);
						if (PEAR::isError($result)) {
							print_log($result->getMessage(), 0, TRUE);
							$db->rollback();
						} else {
							if ($table == 'sq_ast_lookup_value') {
								print_log("[FIX LOOKUP SUCCESS sq_ast_lookup_value]\nFROM: $key\nTO  : $new_url\n", 0);
							} else if ($table == 'sq_ast_lookup_remap') {
								print_log("[FIX LOOKUP SUCCESS sq_ast_lookup_remap]\nFROM: $key\nTO  : $new_url\n", 0);
							}
							$db->commit();
						}
					}
				}

			}//end foreach lookup tables
		}//end foreach updated_url
	}//end if updated_url

}//end fix_lookups()


/**
* Copy all the files and sub directories to the new directory
*
* @return void
* @access public
*/
function _copy_over_folder($old_dir_path, $new_dir_path)
{
	global $log;
	$result = copy_directory($old_dir_path, $new_dir_path);

	if ($result) {
		$result = delete_directory($old_dir_path);
	} else {
		print_log("[$old_dir_path] Failed to copy the contents from the old directory to the new directory. [$new_dir_path].", 0, TRUE);
		return FALSE;
	}

	if ($result) {
		record_log($new_dir_path, 'directory moved');
	} else {
		print_log("[$old_dir_path] Failed to remove the old directory.", 0, TRUE);
		return FALSE;
	}

	return TRUE;

}//end _copy_over_folder()


/**
* Extra process for the public data directory.
*
* Need to update public lookup values
*
* @return void
* @access public
*/
function _process_public_dir_lookup($base_dir, $curr_dir, $assetid)
{
	global $lookup_tables, $db, $count;

	foreach ($lookup_tables as $table) {
		$sql = 'SELECT url FROM '.$table.' WHERE url LIKE ';

		$hash = get_asset_hash($assetid);

		// Remove '__data' to include System Static URL
		$old_public_url = '/assets/'.$curr_dir.'/'.$assetid.'/';
		$new_public_url = '/assets/'.$curr_dir.'/'.$hash.'/'.$assetid.'/';

		switch ($table) {
			case 'sq_ast_lookup' :
				$sql .= $db->quoteSmart('%'.$old_public_url.'%');
			break;
			case 'sq_ast_lookup_value' :
				$sql .= $db->quoteSmart('%'.$old_public_url.'%');
			break;
			case 'sq_ast_lookup_remap' :
				$sql .= $db->quoteSmart('%'.$old_public_url.'%').' OR remap_url LIKE'.$db->quoteSmart('%'.$old_public_url.'%');
			break;
		}

		$result =& $db->getAll($sql, TRUE);

		if (PEAR::isError($result)) {
			print_log($result->getMessage(), 0, TRUE);
			_generate_result_report();
			exit();
		} else {

			if (!empty($result)) {

				foreach ($result as $urls) {
					$url = $urls[0];
					// Need to update lookup tables
					switch ($table) {
						case 'sq_ast_lookup' :
						case 'sq_ast_lookup_value' :
							$new_url = str_replace($old_public_url, $new_public_url, $url);
							$sql = 'UPDATE
										'.$table.'
									SET
										url = '.$db->quoteSmart($new_url).'
									WHERE
										url = '.$db->quoteSmart($url);

							$new_url = str_replace($old_public_url, $new_public_url, $url);
						break;
						case 'sq_ast_lookup_remap' :
							if (strpos($url, $old_public_url) !== FALSE) {
								$new_url = str_replace($old_public_url, $new_public_url, $url);
								$sql = 'UPDATE
											'.$table.'
										SET
											url = '.$db->quoteSmart($new_url).'
										WHERE
											url = '.$db->quoteSmart($url);
								$new_url = str_replace($old_public_url, $new_public_url, $url);
							} else if (strpos($data[0], $old_public_url) !== FALSE) {
								$new_url = str_replace($old_public_url, $new_public_url, $data[0]);
								$sql = 'UPDATE
											'.$table.'
										SET
											remap_url = '.$db->quoteSmart($new_url).'
										WHERE
											remap_url = '.$db->quoteSmart($data[0]);
								$new_url = str_replace($old_public_url, $new_public_url, $data[0]);
							}
						break;

					}//end switch

					if ($db->phptype != 'oci8') {
						$result =& $db->query('BEGIN');
					}
					if (PEAR::isError($result)) {
						print_log($result->getMessage(), 0, TRUE);
						$db->disconnect();
						_generate_result_report();
						exit();
					}

					$result =& $db->query($sql);
					if (PEAR::isError($result)) {
						$result = $db->rollback();
						print_log($result->getMessage(), 0, TRUE);
						$db->disconnect();
						_generate_result_report();
						exit();
					} else {
						$result = $db->commit();
						print_log("[SUCCESS] lookup url $table update completed.", 2);
						record_log($new_public_url, 'url updated');
						$count[$table]++;
					}

				}//end foreach

			}//end if
		}
	}

}//end _process_public_dir_lookup()


/**
* Prints the usage for this script and exits
*
* @return void
* @access public
*/
function usage()
{
	echo "*************************************************************************\n";
	echo "*    DON'T FORGET TO BACK UP YOUR SYSTEM BEFORE YOU RUN THIS SCRIPT     *\n";
	echo "*************************************************************************\n\n";
	echo "USAGE: upgrade_data_dirs.php -s <system_root> [--start-upgrade] [--report-only] [--show-details]\n\n";
	echo "--start-upgrade : This option should be specified to start the upgrade\n";
	echo "--report-only   : The script generates the report only without any changes\n";
	echo "--show-details  : Prints the detailed progress\n";
	echo "--fix-lookups   : Use only if the script ran with the wrong hash function\n\n";
	echo "*************************************************************************\n";
	echo "*    DON'T FORGET TO BACK UP YOUR SYSTEM BEFORE YOU RUN THIS SCRIPT     *\n";
	echo "*************************************************************************\n";
	exit();

}//end usage()


/**
* Generates the report
*
* @return void
* @access public
*/
function generate_report($with_result=FALSE)
{
	global $data_dirs, $skip_dirs, $count, $log, $lookup_tables;

	if (!$with_result) {
		foreach ($data_dirs as $data_path) {
			if ($dh = opendir(SQ_SYSTEM_ROOT.$data_path)) {
				while (false !== ($file = readdir($dh))) {
					if (array_search($file, $skip_dirs) !== FALSE) continue;
					if (is_dir(SQ_SYSTEM_ROOT.$data_path.'/'.$file)) {
						if ($dh2 = opendir(SQ_SYSTEM_ROOT.$data_path.'/'.$file)) {

								require_once 'DB.php';
								$db =& DB::connect(SQ_CONF_DB2_DSN);
								if (PEAR::isError($db)) {
									print_log("[ERROR Failed to connect to database] ".$db->getMessage(), 0, TRUE);
									$db->disconnect();
									exit();
								}
								$db->autoCommit(FALSE);

							while (FALSE !== ($assetid = readdir($dh2))) {
								if (array_search($assetid, $skip_dirs) !== FALSE) continue;

								// Assumption. No assetid starts with zero
								if (substr($assetid, 0, 1) == '0') continue;

								if ($data_path == '/data/private/assets') {
									$count['data_private_count_ex']++;
								} else if ($data_path == '/data/public/assets') {
									$count['data_public_count_ex']++;

									foreach ($lookup_tables as $table) {
										switch ($table) {
											case 'sq_ast_lookup' :
											case 'sq_ast_lookup_value' :
												$sql = 'SELECT
															count(*)
														FROM
															'.$table.'
														WHERE
															url LIKE '.$db->quoteSmart('%/assets/'.$file.'/'.$assetid.'%');
											break;
											case 'sq_ast_lookup_remap' :
												$sql = 'SELECT
															count(*)
														FROM
															'.$table.'
														WHERE
															url LIKE '.$db->quoteSmart('%/assets/'.$file.'/'.$assetid.'%').' OR
															remap_url LIKE'.$db->quoteSmart('%/assets/'.$file.'/'.$assetid.'%');
											break;
										}

										$result =& $db->getRow($sql);
										if (PEAR::isError($result)) {
											print_log($result->getMessage(), 0, TRUE);
											exit();
										} else {
											$count[$table.'_ex'] += $result[0];
										}
									}

								} else if ($data_path == '/data/file_repository/assets') {
									$count['file_vers_count_ex']++;

									$old_file_vers_path = 'assets/'.$file.'/'.$assetid;
									$sql = 'SELECT
												*
											FROM
												sq_file_vers_file
											WHERE
												path = '.$db->quoteSmart($old_file_vers_path);
									$result =& $db->getAll($sql);
									if (PEAR::isError($result)) {
										print_log("[$old_file_vers_path :(] SELECT query failed. ".$result->getMessage(), 0, TRUE);
										exit();
									}
									if (!empty($result)) {
										$count['file_vers_query_count_ex'] += count($result);
									}
								}
							}
							if (!is_null($db)) {
								//$db->disconnect();
							}
							closedir($dh2);
						}
					}
				}
				closedir($dh);
			}
		}

		$count['total_count_ex'] = $count['data_private_count_ex'] + $count['data_public_count_ex'] + $count['file_vers_count_ex'];
		$count['total_query_count_ex'] = $count['file_vers_query_count_ex'] + $count['sq_ast_lookup_ex'] + $count['sq_ast_lookup_value_ex'] + $count['sq_ast_lookup_remap_ex'];
		echo "*************************************************************************\n";
		echo "*    DON'T FORGET TO BACK UP YOUR SYSTEM BEFORE YOU RUN THIS SCRIPT     *\n";
		echo "*************************************************************************\n\n";
		echo "+-----------------------------------------------------+\n";
		echo "| Total ".$count['total_count_ex']." directories will be updated\n";
		echo "| Total ".$count['total_query_count_ex']." DB entries will be updated\n";
		echo "+-----------------------------------------------------+\n";
		echo "| Data Private      : ".$count['data_private_count_ex']."\n";
		echo "| Data Public       : ".$count['data_public_count_ex']."\n";
		echo "| File Repository   : ".$count['file_vers_count_ex']."\n";
		echo "| sq_file_vers_file : ".$count['file_vers_query_count_ex']."\n";
		echo "| sq_lookup         : ".$count['sq_ast_lookup_ex']."\n";
		echo "| sq_lookup_value   : ".$count['sq_ast_lookup_value_ex']."\n";
		echo "| sq_lookup_remap   : ".$count['sq_ast_lookup_remap_ex']."\n";
		echo "+-----------------------------------------------------+\n";
	}

	if ($with_result) _generate_result_report();

}//end generate_report()


/**
* Generates the result report
*
* @return void
* @access public
*/
function _generate_result_report()
{
	global $count;

	$count['total_count'] = $count['data_private_count'] + $count['data_public_count'] + $count['file_vers_count'];
	$count['total_query_count'] = $count['file_vers_query_count'] + $count['sq_ast_lookup'] + $count['sq_ast_lookup_value'] + $count['sq_ast_lookup_remap'];
	echo "\n+-----------------------------------------------------+\n";
	echo "| Result\n";
	echo "+-----------------------------------------------------+\n";
	echo "| Total ".$count['total_count']." directories have been be updated\n";
	echo "| Total ".$count['total_query_count']." DB entries have updated\n";
	echo "+-----------------------------------------------------+\n";
	echo "| Data Private      : ".$count['data_private_count']."\n";
	echo "| Data Public       : ".$count['data_public_count']."\n";
	echo "| File Repository   : ".$count['file_vers_count']."\n";
	echo "| sq_file_vers_file : ".$count['file_vers_query_count']."\n";
	echo "| sq_lookup         : ".$count['sq_ast_lookup']."\n";
	echo "| sq_lookup_value   : ".$count['sq_ast_lookup_value']."\n";
	echo "| sq_lookup_remap   : ".$count['sq_ast_lookup_remap']."\n";
	echo "+-----------------------------------------------------+\n";

	if (is_error()) {
		echo "+-----------------------------------------------------+\n";
		echo "| Inconsistency found during the upgrade\n";
		echo "+-----------------------------------------------------+\n";
		echo "| Most of cases you can ignore this, however please check\n";
		echo "| the frontend display to confirm the upgrade was done properly\n";
		echo "| and check ".SQ_SYSTEM_ROOT."/data/upgrade_data_dir_log.txt\n";
		echo "| file for  more details\n";
		echo "+-----------------------------------------------------+\n";
	} else {
		echo "+-----------------------------------------------------+\n";
		echo "| Congratulations!\n";
		echo "+-----------------------------------------------------+\n";
		echo "| Now you can have about ".(SQ_CONF_NUM_DATA_DIRS * 32000)." same type of assets\n";
		echo "| in the file system without inode problems.\n";
		echo "+-----------------------------------------------------+\n";
		delete_log();
	}

}//end _generate_result_report()


/**
* Returns FALSE if the expected change count does not match
* the result
*
* @return void
* @access public
*/
function is_error()
{
	global $count;
	if ($count['data_private_count_ex'] == $count['data_private_count'] &&
		$count['data_public_count_ex'] == $count['data_public_count'] &&
		$count['file_vers_count_ex'] == $count['file_vers_count'] &&
		$count['file_vers_query_count_ex'] == $count['file_vers_query_count'] &&
		$count['sq_ast_lookup_ex'] == $count['sq_ast_lookup'] &&
		$count['sq_ast_lookup_value_ex'] == $count['sq_ast_lookup_value'] &&
		$count['sq_ast_lookup_remap_ex'] == $count['sq_ast_lookup_remap_ex']) {
			return FALSE;
	}

	return TRUE;

}//end is_error()


/**
* Write the log file
*
* @return void
* @access public
*/
function record_log($key, $value)
{
	global $log;
	$log[$key] = $value;
	$fh = fopen(SQ_SYSTEM_ROOT.'/data/upgrade_data_dir_log.txt', 'w');
	fwrite($fh, serialize($log));
	fclose($fh);

}//end record_log()


/**
* Read the log file
*
* @return void
* @access public
*/
function read_log()
{
	global $log;
	if (file_exists(SQ_SYSTEM_ROOT.'/data/upgrade_data_dir_log.txt')) {
		$log = unserialize(file_get_contents(SQ_SYSTEM_ROOT.'/data/upgrade_data_dir_log.txt'));
	} else {
		$log = Array();
	}

}//end read_log()


/**
* Delete the log file
*
* @return void
* @access public
*/
function delete_log()
{
	global $log;
	if (file_exists(SQ_SYSTEM_ROOT.'/data/upgrade_data_dir_log.txt')) {
		unlink(SQ_SYSTEM_ROOT.'/data/upgrade_data_dir_log.txt');
	}

}//end read_log()


/**
* Print the log message
*
* @return void
* @access public
*/
function print_log($string, $level=0, $error=FALSE)
{
	global $SHOW_DETAILS;
	if ($SHOW_DETAILS) {
		if ($error) {
			echo "******************************************************************\n";
			echo "*             ERROR OCCURRED - Please check the log file\n";
			echo "******************************************************************\n";
			echo "* $string\n";
			echo "******************************************************************\n";
		} else {
			echo str_repeat(' ', $level * 4)."$string\n";
		}
	} else {
		echo '.';
	}
}

?>
