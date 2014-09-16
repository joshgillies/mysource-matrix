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
 * Script to perform charset conversion on the table fields values (tables and fields defined in script)
 * It also regenerates the content files (bodycopy, metadata and design) of asset associated with the db record
 *
 * IMPORTANT: SYSTEM MUST BE BACKEDUP BEFORE RUNNING THIS SCRIPT!!!
 *
 * @author  Chiranjivi Upreti <cupreti@squiz.com.au>
 * @version $Revision: 1.14 $
 * @package MySource_Matrix
 */

error_reporting(E_ALL);
if ((php_sapi_name() != 'cli')) {
	trigger_error("You can only run this script from the command line\n", E_USER_ERROR);
}

$SYSTEM_ROOT = getCLIArg('system');
if (!$SYSTEM_ROOT) {
	echo "ERROR: You need to supply the path to the System Root\n";
	print_usage();
	exit(1);
}

if (!is_dir($SYSTEM_ROOT) || !is_readable($SYSTEM_ROOT.'/core/include/init.inc')) {
	echo "ERROR: Path provided doesn't point to a Matrix installation's System Root. Please provide correct path and try again.\n";
	print_usage();
	exit(1);
}

if (ini_get('memory_limit') != '-1') ini_set('memory_limit', '-1');

$SYS_OLD_ENCODING = getCLIArg('old');
if (!$SYS_OLD_ENCODING || !isValidCharset($SYS_OLD_ENCODING)) {
	echo  "\nERROR: The charset you specified '$SYS_OLD_ENCODING', as system's old encoding is not valid charset type.\n\n";
	print_usage();
	exit(1);
}
define('SYS_OLD_ENCODING',$SYS_OLD_ENCODING);

$SYS_NEW_ENCODING = getCLIArg('new');
if (!isValidCharset($SYS_NEW_ENCODING)) {
	echo  "\nERROR: The charset you specified '".$SYS_NEW_ENCODING."', as system's new encoding is not valid charset type.\n\n";
	print_usage();
	exit(1);
}

if (!empty($SYS_NEW_ENCODING)) {
	define('SYS_NEW_ENCODING', $SYS_NEW_ENCODING);
} else {
	$config_file = file_get_contents($SYSTEM_ROOT.'/data/private/conf/main.inc');
	preg_match("|SQ_CONF_DEFAULT_CHARACTER_SET',\s*'(.*?)'\);|", $config_file, $match);
	if (empty($match[1])) {
		echo "\nERROR: The default charset is not specified in the main.inc. Pleas specify the new charset to convert the system to.\n\n";
		print_usage();
		exit(1);
	}
	define('SYS_NEW_ENCODING', $match[1]);
}

$root_node_id = getCLIArg('rootnode');
$root_node_id = ($root_node_id) ? $root_node_id : 1;

$reportOnly = getCLIArg('report');

// Whether to include rollback tables in the db
$include_rollback = !getCLIArg('ignore-rollback');

// Make sure iconv is available.
if (function_exists('iconv') == FALSE) {
	echo "This script requires the php iconv module which isn't available.\n";
	echo "Install that module and try again.\n";
	exit(1);
}

// Tables where the values are to be looked into
//
// Array(
//		Array(
//			"table"			=> Table that needs checking/fixing
//			"values" 		=> Array of field names whose values needs checking/fixing
//			"asset_assoc"	=> TRUE if the table record is assoicated with an asset ("does the table has 'assetid' field?")
//		),
//	)
//
// NOTE 1: 	If the value field holds serialised value then script must handle that field accourdingly.
//			Currently "custom_val" field of "ast_attr_val" table of attribute types 'option_list','email_format','parameter_map','serialise',
//			'http_request' and 'oauth', and	"data" field of "trig" are only ones that are treated as serialsed values by this script.
// NOTE 2:	There are two entries for 'internal_msg' table below because one targets the messages that are assoicated with an asset,
// 			while other targets non-asset specific messages (message that has empty assetid field, like internal message sent by user or trigger action).
//
$tables = Array(
			Array(
				'table'			=> 'ast',
				'values'		=> Array('name', 'short_name'),
				'asset_assoc'	=> TRUE,
			),
			Array(
				'table'			=> 'ast_attr_val',
				'values'		=> Array('custom_val'),
				'asset_assoc'	=> TRUE,
			),
			Array(
				'table'			=> 'ast_mdata_val',
				'values'		=> Array('value'),
				'asset_assoc'	=> TRUE,
			),
			Array(
				'table'			=> 'ast_mdata_dflt_val',
				'values'		=> Array('default_val'),
				'asset_assoc'	=> TRUE,
			),
			Array(
				'table'			=> 'ast_attr_uniq_val',
				'values'		=> Array('custom_val'),
				'asset_assoc'	=> TRUE,
			),
			Array(
				'table'			=> 'internal_msg',
				'values'		=> Array('subject', 'body'),
				'asset_assoc'	=> TRUE,
			),
			Array(
				'table'			=> 'internal_msg',
				'values'		=> Array('subject', 'body'),
				'asset_assoc'	=> FALSE,
			),
			Array(
				'table'			=> 'trig',
				'values'		=> Array('name', 'data', 'description', 'category'),
				'asset_assoc'	=> FALSE,
			),
			Array(
				'table'			=> 'thes_term',
				'values'		=> Array('term'),
				'asset_assoc'	=> FALSE,
			),
	);

$TABLES = getCLIArg('skip-tables');
if (!empty($TABLES)) {
	$skip_tables = explode(',', $TABLES);
	foreach($tables as $index => $table_data) {
		if (in_array($table_data['table'], $skip_tables)) {
			unset($tables[$index]);
		}//end if
	}//end forach
}

if (SYS_OLD_ENCODING == SYS_NEW_ENCODING) {
	echo  "\nERROR: The old encoding ('" . SYS_OLD_ENCODING . "') is the same as the current/new character set.\n\n";
	print_usage();
	exit(1);
}

// Oracle requires that the encoding be set when you connect to the database.
// Let's make sure:
// 1) It's set to something
// 2) If we're converting to utf-8, it's set to the right thing
//
$conf = require $SYSTEM_ROOT.'/data/private/conf/db.inc';
if ($conf['db']['type'] === 'oci') {
	if (isset($conf['db']['encoding']) === FALSE) {
		echo "Using an oracle database, you must also set the encoding during the database connection.";
		if (SYS_NEW_ENCODING === 'utf-8') {
			echo "\nIt should be set to 'AL32UTF8', but currently it's not set.";
		} else {
			echo "\nIt should be set to match '".SYS_NEW_ENCODING."', but currently it's not set.";
		}
		echo "\nContinue [y/N] ? ";
		$response = trim(fgets(STDIN, 1024));
		if (strtolower($response) != 'y') {
			echo "\nAborting\n";
			exit();
		}
	} else {
		$dbEncoding = $conf['db']['encoding'];
		if (SYS_NEW_ENCODING === 'utf-8' && strtolower($dbEncoding) != 'al32utf8') {
			echo "You are converting the character set to utf-8 but have the database connection";
			echo "\nencoding currently set to '".$dbEncoding."'";
			echo "\nContinue [y/N] ? ";
			$response = trim(fgets(STDIN, 1024));
			if (strtolower($response) != 'y') {
				echo "\nAborting\n";
				exit();
			}
		}
	}
}

if ($root_node_id == 1) {
	echo "\nWARNING: You are running this script on the whole system.\nThis is fine, but it may take a long time\n";
}

define('SCRIPT_LOG_FILE', $SYSTEM_ROOT.'/data/private/logs/'.basename(__FILE__).'.log');

if (!$reportOnly) {
	echo "\nIMPORTANT: This script will perform the charset conversion on the database from the older to the current encoding\n";
	echo "YOU MUST BACKUP YOUR SYSTEM BEFORE RUNNING THIS SCRIPT\n";
	echo "Are you sure you want to proceed (Y/N)? \n";

	$yes_no = rtrim(fgets(STDIN, 4094));
	if (strtolower($yes_no) != 'y') {
		echo "\nScript aborted. \n";
	exit;
	}
}

// File to communicate between the child and parent process
define('SYNC_FILE', $SYSTEM_ROOT.'/data/temp/system_integrity_fix_char_encoding.data');
// Batch size when processing the asset contnet file regeneration
define('BATCH_SIZE', '100');

 // No turning back now. Start char fixing.
$start_time = microtime(TRUE);

// Whether to check rollback tables
$rollback_summary = NULL;
if ($include_rollback) {

	$pid = fork();
	if (!$pid) {

		// NOTE: This seemingly ridiculousness allows us to workaround Oracle, forking and CLOBs
		// if a query is executed that returns more than 1 LOB before a fork occurs,
		// the Oracle DB connection will be lost inside the fork
		require_once $SYSTEM_ROOT.'/core/include/init.inc';

		// Fix the rollback tables
		$summary = fix_db($root_node_id, $tables, TRUE);
		echo "\n*Finished fixing rollback database*";

		// Unlike regular tables, for rollback table entries we dont need to obtain affected assetids
		// list to regenerate the relevant files in the filesystem
		unset($summary['affected_assetids']);
		if (!file_put_contents(SYNC_FILE, serialize($summary))) {
			echo "\nFailed to create the sync file ".SYNC_FILE;
		}

		exit();

	}//end child process

	if (!is_file(SYNC_FILE)) {
		echo "\n";
		echo "ERROR: Expected sync file containing the affected rollback entries not found. Either script ran out of memory while fixing the db,\n";
		echo "or the script did not had write permission on [MATRIX_ROOT]/data/temp directory.\n\n";
		exit(1);
	}
	$rollback_summary = unserialize(file_get_contents(SYNC_FILE));
	unlink(SYNC_FILE);
}

$pid = fork();
if (!$pid) {

	// NOTE: This seemingly ridiculousness allows us to workaround Oracle, forking and CLOBs
	// if a query is executed that returns more than 1 LOB before a fork occurs,
	// the Oracle DB connection will be lost inside the fork
	require_once $SYSTEM_ROOT.'/core/include/init.inc';

	// Fix regular tables
	$summary = fix_db($root_node_id, $tables, FALSE);
	echo "\n*Finished fixing database*\n\n";

	// Get the list of assetids for which we need to regenerate  the filesystem content
	// to reflect the changes made in the db
	$affected_assetids = get_affected_assetids($summary['affected_assetids']);
	echo "\n*Finished compiling affected assets list*\n";

	// Also get the context ids
	$contextids = array_keys($GLOBALS['SQ_SYSTEM']->getAllContexts());

	// Database update summary for both regular and rollback tables
	unset($summary['affected_assetids']);
	$db_summary = Array(
					'regular' => $summary,
					'rollback' => $rollback_summary,
				);

	if (!file_put_contents(SYNC_FILE, serialize(Array('affected_assetids' => $affected_assetids, 'db_summary' => $db_summary, 'contextids' => $contextids)))) {
		echo "\nFailed to create the sync file ".SYNC_FILE;
	}

	exit();

}//end child process

if (!is_file(SYNC_FILE)) {
	echo "\n";
	echo "ERROR: Expected sync file containing the affected assetids not found. Either script ran out of memory while fixing the db,\n";
	echo "or the script did not had write permission on [MATRIX_ROOT]/data/temp directory.\n";
	echo "Either way this means the file content regeneration for the affected assets will have to be done manually.\n";
	echo "\n";
	echo "If the script has finished processing the db, please run the following script as an Apache user to regenerate the asset content files:\n";
	echo "php scripts/regenerate_file_system.php --system=[SYSTEM_ROOT] --all\n\n";

	exit(1);
}

$summary = unserialize(file_get_contents(SYNC_FILE));
unlink(SYNC_FILE);

echo "\n";
if (!$reportOnly) {
	// Fix the filesystem content to reflect the changes made in the db
	regenerate_filesystem_content($summary['affected_assetids'], $summary['contextids']);

	echo "Number of db records replaced successfully: ".$summary['db_summary']['regular']['records_fixed_count']."\n";
	echo "Number of db records failed upating: ".$summary['db_summary']['regular']['error_count']."\n";
	echo "Total warnings recorded: ".$summary['db_summary']['regular']['warning_count']."\n";
	if ($include_rollback) {
		echo "Number of rollback db records replaced successfully: ".$summary['db_summary']['rollback']['records_fixed_count']."\n";
		echo "Number of rollback db records failed upating: ".$summary['db_summary']['rollback']['error_count']."\n";
		echo "Total rollback warnings recorded: ".$summary['db_summary']['rollback']['warning_count']."\n";
	}
} else {
	echo "Number of db records that need replacing: ".$summary['db_summary']['regular']['records_fixed_count']."\n";
	if ($include_rollback) {
		echo "Number of rollback db records that need replacing: ".$summary['db_summary']['rollback']['records_fixed_count']."\n";
	}
}

echo "Total time taken to run the script: ".round(microtime(TRUE)-$start_time, 2)." second(s)\n";

$total_error_count = $summary['db_summary']['regular']['error_count']+$summary['db_summary']['regular']['warning_count'];
if ($include_rollback) {
	$total_error_count += $summary['db_summary']['rollback']['error_count']+$summary['db_summary']['rollback']['warning_count'];
}
if ($total_error_count > 0)	{
	echo "\nPlease check ".SCRIPT_LOG_FILE." file for warnings/errors\n\n";
}
echo "\n";

exit();

// End of Main program /////////////////////////////////


/**
 * Fixes the char encoding in the given tables in the database
 *
 * @param int		$root_node		Assetid of rootnode, all childern of rootnode will be processed for char replacement
 * @param array		$tables			DB tables and colunms info
 * @param boolean	$rollback		If TRUE process rollback tables, else process regular tables
 *
 * @return void
 */
function fix_db($root_node, $tables, $rollback)
{
	global $reportOnly;

	$tables_info = get_tables_info();

	// All the Matrix attribute types with serialised value
	 $serialsed_attrs = Array(
							'option_list',
							'email_format',
							'parameter_map',
							'serialise',
							'http_request',
							'oauth',
						);

	// Get the list of attrids of the type 'serialise'
	$sql = "SELECT attrid FROM sq_ast_attr WHERE type IN ('".implode("','", $serialsed_attrs)."')";
	$serialise_attrids = array_keys(MatrixDAL::executeSqlGrouped($sql));

	if ($root_node == 1) {
		// Run script system wide. Get the asset list from "ast" table directly
		$sql = "SELECT DISTINCT assetid FROM ".($rollback ? 'sq_rb_' : 'sq_')."ast";
		$target_assetids = array_keys(MatrixDAL::executeSqlGrouped($sql));
	} else {
		$target_assetids = array_keys($GLOBALS['SQ_SYSTEM']->am->getChildren($root_node));
		// Since we include the root node, target assetids will always contain atleast one asset id
		array_unshift($target_assetids, $root_node);
	}

	echo "\n\nNumber of assets to look into : ".count($target_assetids)." \n";

	$errors_count = 0;
	$warnings_count = 0;
	$records_fixed_count = 0;
	$invalid_asset_records = Array();

	// Assets that will require filesystem content regeneration
	$affected_assetids = Array();

	$GLOBALS['SQ_SYSTEM']->changeDatabaseConnection('db2');

	// Go through 50 assets at a time. Applicable to asset specific tables only
    $chunks = array_chunk($target_assetids, 50);

	// Counter to count the number of records accessed/processed
	$count = 0;
	foreach($tables as $table_data) {

		$table = isset($table_data['table']) ? $table_data['table'] : '';
		if (empty($table)) {
			continue;
		}

		$key_fields = isset($tables_info[$table]['primary_key']) ? $tables_info[$table]['primary_key'] : '';
		if (empty($key_fields)) {
			echo "\n".'Ignoring table "'.$table.'". Table info for this table not found'." \n";
			continue;
		}
		$value_fields = isset($table_data['values']) ? $table_data['values'] : '';
		if (empty($value_fields)) {
			// Nothing to check
			continue;
		}

		if ($rollback) {
			// Make sure table has rollback trigggers enabled, otherwise it will have rollback table
			if (isset($tables_info[$table]['rollback']) && $tables_info[$table]['rollback']) {
				// Add rollback table primary key field to the table's keys
				$key_fields[] = 'sq_eff_from';
			} else {
				// This table does not has corresponding rollback table
				continue;
			}
		}

		// Prepend table prefix
		$table = !$rollback ? 'sq_'.$table : 'sq_rb_'.$table;

		$asste_specific_table = $table_data['asset_assoc'];
		$select_fields = array_merge($value_fields, $key_fields);
		if ($asste_specific_table && !in_array('assetid', $select_fields)) {
			$select_fields[] = 'assetid';
		}

		echo "\nChecking ".$table." .";

		// For non-asset specific table, this loop will break at end of the very first iteration
		foreach ($chunks as $assetids) {
			$sql  = 'SELECT '.implode(',', $select_fields).' FROM '.$table;
			// For non-asset specific table, "where" condition not is required. We get the whole table in a single go
			if ($asste_specific_table) {
				$sql .= ' WHERE assetid IN (\''.implode('\',\'', $assetids).'\')';
			} else if ($table == 'sq_internal_msg') {
				// Special case for non-asset specific records for 'interal_msg' table
				// Internal message has 'assetid' field but messages not associated with the asset will have empty assetid
				$sql .= " WHERE assetid = '' OR assetid IS NULL";
			}

			$results = MatrixDAL::executeSqlAssoc($sql);
			foreach($results as $record) {
				$count++;
				if ($count % 100 == 0) {
					echo '.';
				}

				// Asset ID associated with this record
				$assetid = $asste_specific_table ? $record['assetid'] : 'n/a';

				// Key field data
				$key_values = Array();
				foreach($key_fields as $key_field) {
					$temp_key_v = array_get_index($record, $key_field, NULL);
					if (is_null($temp_key_v)) {
						// Primary key field must be there
						continue 2;
					}
					$key_values[$key_field] = $temp_key_v;
				}//end foreach

				// Original value field data.
				// This is the one we need to check/fix
				$org_values = Array();
				foreach($value_fields as $value_field) {
					$org_values[$value_field] = array_get_index($record, $value_field, '');
				}//end foreach


				// If it's the same in the new and old encodings, that's good.
				foreach($org_values as $value_field => $value) {
					$checked = @iconv(SYS_OLD_ENCODING, SYS_NEW_ENCODING.'//IGNORE', $value);
					if ($value === $checked) {
						// This field does not requires conversion/checking
						unset($org_values[$value_field]);
					}
				}//end foreach

				if (empty($org_values)) {
					// No field values to convert/check
					continue;
				}

				// Being here means this record contains invalid chars
				$invalid_asset_records[] = array(
											'asset'  => $assetid,
											'table'  => $table,
											'keys'   => $key_values,
											'values' => $org_values,
										);

				$converted_values = Array();
				foreach($org_values as $value_field => $value) {
					// If not valid, convert the values without igonoring or interprating any chars
					if (!isValidValue($value)) {

						// Serialised fields needs to be handled here
						$serialised_value = FALSE;
						if ($table == 'sq_ast_attr_val' && $value_field == 'custom_val' && in_array($record['attrid'], $serialise_attrids)) {
							$serialised_value = TRUE;
						}
						if ($table == 'sq_trig' && $value_field == 'data') {
							$serialised_value = TRUE;
						}

						if ($serialised_value) {
							$us_value = @unserialize($value);
							if ($us_value === FALSE && serialize(FALSE) !== $value) {
								// This has invalid serialsed value, but fix it anyway
								$converted_value = @iconv(SYS_OLD_ENCODING, SYS_NEW_ENCODING.'//IGNORE', $value);

								// Put this error notice in the script log file
								$warnings_count++;
								$msg = 'Serialsed data field "'.$value_field.'" in the table "'.$table.'" (';
								foreach($key_values as $field_name => $value) {
									$msg .= $field_name.'='.$value.'; ';
								}
								$msg = rtrim($msg, '; ').') does not contain unserialisable data. '.($reportOnly ? 'Data can still be converted.' : 'Data will be converted anyway.');
								log_error_msg($msg);

							} else if (is_array($us_value)) {
								array_walk_recursive($us_value, 'fix_char');
								$converted_value = serialize($us_value);
							} else if (is_scalar($us_value)) {
								$us_value = @iconv(SYS_OLD_ENCODING, SYS_NEW_ENCODING.'//IGNORE', $us_value);
								$converted_value = serialize($us_value);
							} else {
								$converted_value = $value;
							}
						} else {
							$converted_value = @iconv(SYS_OLD_ENCODING, SYS_NEW_ENCODING.'//IGNORE', $value);
						}

						// If the converted value is valid in current encoding then its good to go
						// otherwise we'll just not use this value
						if ($converted_value != $value && isValidValue($converted_value)) {
							$value = $converted_value;
							$converted_values[$value_field] = $value;
						}
					} else {
						// if it's a valid encoded value, but was convertable before with iconv using old encoding
						// it might be only because value is already properly encoded with new encoding.  so use md_detect to double check
						$encoding = mb_detect_encoding($value);
						if(strtolower($encoding) === strtolower(SYS_NEW_ENCODING)) {
							unset($org_values[$value_field]);
						}
					}
				}//end foreach

				if (empty($org_values)) {
					// All good
					array_pop($invalid_asset_records);
					continue;
				}

				// If the successfully converted fields count is same as the invalid fields count, we can proceed with the update
				$update_required = count($org_values) == count($converted_values);

				if ($update_required) {
					if (!$reportOnly) {
						$GLOBALS['SQ_SYSTEM']->doTransaction('BEGIN');

						// Generate update sql
						$bind_vars = Array();
						$set_sql = Array();
						foreach($converted_values as $field_name => $value) {
							$set_sql[] = $field_name.'=:'.$field_name.'_v';
							$bind_vars[$field_name.'_v'] = $value;
						}
						$where_sql = Array();
						foreach($key_values as $field_name => $value) {
							$where_sql[] = $field_name.'=:'.$field_name.'_k';
							$bind_vars[$field_name.'_k'] = $value;
						}

						try {
							$sql = 'UPDATE '.$table.'
									SET '.implode(', ', $set_sql).'
									WHERE '.implode(' AND ', $where_sql);

							$update_sql = MatrixDAL::preparePdoQuery($sql);
							foreach($bind_vars as $var_name => $var_value) {
								MatrixDAL::bindValueToPdo($update_sql, $var_name, $var_value);
							}

							// Execute the update query
							$execute = MatrixDAL::executePdoAssoc($update_sql);

							if (count($execute) > 1) {
								foreach($bind_vars as $var_name => $var_value) {
									$sql = str_replace(':'.$var_name, "'".$var_value."'", $sql);
								}
								$errors_count++;

								$msg = 'Executing query "'.$sql.'" will affect '.count($execute).' records, instead of expected single record! Ignoring this sql.';
								log_error_msg($msg);
								$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');

							} else {
								$GLOBALS['SQ_SYSTEM']->doTransaction('COMMIT');

								$records_fixed_count++;
								$affected_assetids[$table][] = $assetid;
							}
						} catch (Exception $e) {
							$errors_count++;
							$msg = "Unexpected error occured while updating database: ".$e->getMessage();
							log_error_msg($msg);

							$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
						}
					} else {
						$records_fixed_count++;
						// For reporting purpose only
						$affected_assetids[$table][] = $assetid;
					}
				} else {
					// Trying to carryout charset conversion for this invalid value still resulted into invalid value
					// Hence record was not updated for this value conversion
					$errors_count++;

					$msg = 'Entry in the table "'.$table.'": '."\n";
						foreach($key_values as $field_name => $field_value) {
							$msg .= $field_name.'="'.$field_value.'"; ';
						}
                    $msg .= "\n". 'contains invalid char(s), which were not replaced because the charset conversion was not successful'.
					$msg .= "\n".'Potentially invalid characters include:'.listProblematicCharacters($org_values);
					log_error_msg($msg);
				}
			}//end foreach records

			if (!$asste_specific_table) {
				// We have processed all the entries for this non-asset specific table
				break;
			}

		}//end foreach assetids chunk
	}//end foreach tables

	$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();

	unset($target_assetids);
	unset($chunks);

	echo "\n";
	$invalid_count = sizeof(array_keys($invalid_asset_records));
	echo "Number of db records with invalid char(s): ".$invalid_count."\n";
	if ($invalid_count > 0) {
	foreach ($invalid_asset_records as $k => $details) {
		echo "\n\tAsset #".$details['asset']." in table ".$details['table'];
			echo "\n\t".'Entry: ';
			foreach($details['keys'] as $field_name => $field_value) {
				echo $field_name.'="'.$field_value.'"; ';
			}
			echo "\n\tPossibly problematic characters: ".listProblematicCharacters($details['values'])."\n";
	}
	echo "\n";
}

	return Array(
			'warning_count' => $warnings_count,
			'error_count' => $errors_count,
			'records_fixed_count' => $records_fixed_count,
			'affected_assetids' => $affected_assetids,
		);

}//end fix_db()


/**
* Callback function for array_walk_recursive()
* to fix the chars in the serialised value
*
* @param $val	Array value
* @param $key	Array key
*
* @return void
*/
function fix_char(&$val, $key)
{
	if (is_string($val)) {
		$check = @iconv(SYS_OLD_ENCODING, SYS_NEW_ENCODING.'//IGNORE', $val);
		// If it requires converting
		if ($val != $check) {
			$val = @iconv(SYS_OLD_ENCODING, SYS_NEW_ENCODING.'//IGNORE', $val);
		}
	}

}//end fix_char


/**
* Get the list of affected assetids based in data updated in the db
* (see fix_db())
*
* @param array $data	Info about the assetids updated in the in db
* Array(<table_name> => Array(<list of assetids>))
*
* @return array
*/
function get_affected_assetids($data)
{
	// List of relevant assetids to regenerate the filesystem content
	$affected_assetids = Array(
				'bodycopy_content_file' => Array(),
				'metadata_file' => Array(),
				'design_file' => Array(),
			);

	echo "Getting the list of assetids that needs content regeneration ...";
	foreach($data as $table_type => $assetids) {
		switch($table_type) {
			case 'sq_ast_mdata_val':
				$affected_assetids['metadata_file'] = array_merge($affected_assetids['metadata_file'], $assetids);
				echo ".";
			break;

			case 'sq_ast_mdata_dflt_val':
				$mm = $GLOBALS['SQ_SYSTEM']->getMetadataManager();
				foreach($assetids as $mfield_assetid) {
					// Get all the asset that has this schema applied
					$schemaids = array_keys($GLOBALS['SQ_SYSTEM']->am->getParents($mfield_assetid, 'metadata_schema'));
					foreach($schemaids as $schemaid) {
						$affected_assetids['metadata_file'] = array_merge($affected_assetids['metadata_file'], array_keys($mm->getSchemaAssets($schemaid)));
					}
					echo ".";
				}//end foreach
			break;

			case 'sq_ast_attr_val':
				// Get list of Design assets that needs to be regenerated
				$affected_assetids['design_file'] = array_keys($GLOBALS['SQ_SYSTEM']->am->getAssetInfo($assetids, Array('design','design_css'), TRUE));

				echo ".";
				// and list of Bodycopy Container assets
				$content_type_assetids = array_keys($GLOBALS['SQ_SYSTEM']->am->getAssetInfo($assetids, Array('content_type'), FALSE));
				foreach($content_type_assetids as $assetid) {
					$bodycopy_container_link = $GLOBALS['SQ_SYSTEM']->am->getLinks($assetid, SQ_LINK_TYPE_2, Array('bodycopy_container'), FALSE, 'minor');
					if (isset($bodycopy_container_link[0]['majorid'])) {
						// This bodycopy content file needs to be generated
						$affected_assetids['bodycopy_content_file'][] = $bodycopy_container_link[0]['majorid'];
					}
					echo ".";
				}//end foreach

			break;
		}//end switch
	}//end foreach

	// Remove the duplicates from the assetid list
	$affected_assetids['metadata_file'] = array_unique($affected_assetids['metadata_file']);
	$affected_assetids['bodycopy_content_file'] = array_unique($affected_assetids['bodycopy_content_file']);
	$affected_assetids['design_file'] = array_unique($affected_assetids['design_file']);

	// Chunk the assets into the batches
	$batched_assetids = Array();
	foreach($affected_assetids as $type => $type_assetids) {
		$start_index = 0;
		$asset_count = count($type_assetids);
		$batched_assetids[$type] = Array();
		while($start_index < $asset_count) {
			$batched_assetids[$type][] = array_slice($type_assetids, $start_index, BATCH_SIZE);
			$start_index += BATCH_SIZE;
		}//end while
	}//end foreach

	unset($affected_assetids);
	echo " done.\n";

	return $batched_assetids;

}//end get_affected_assetids()


/**
* Regenerate the content files in the file system
*
* @param array	$assets_data	Asset that needs content regeneration
* Array(
*	'bodycopy_content_file' => Array(<bodycopy container assetids>),
*	'metadata_file' => Array(<assetids>),
*	'design_file' => Array(<design assetids>),
*   )
* @param array	$contextids
*
* @return void
*/
function regenerate_filesystem_content($assets_data, $contextids)
{
	global $SYSTEM_ROOT;

	echo "\n";

	foreach($assets_data as $type => $assets_batch) {
		if (empty($assets_batch)) {
			continue;
		}

		echo "Regenerating the ".str_replace('_', ' ', $type). " ...";
		foreach($assets_batch as $assetids) {
			$pid = fork();
			if (!$pid) {

				// Do the stuff in the child process
				require_once $SYSTEM_ROOT.'/core/include/init.inc';
				$root_user = $GLOBALS['SQ_SYSTEM']->am->getSystemAsset('root_user');
				$GLOBALS['SQ_SYSTEM']->setCurrentUser($root_user);

				$mm = $GLOBALS['SQ_SYSTEM']->getMetadataManager();
				$GLOBALS['SQ_SYSTEM']->setRunLevel(SQ_RUN_LEVEL_FORCED);

				$processed_design_assetids = Array();
				foreach($contextids as $contextid) {
					$GLOBALS['SQ_SYSTEM']->changeContext($contextid);

					foreach($assetids as $assetid) {
						$asset = $GLOBALS['SQ_SYSTEM']->am->getAsset($assetid);
						if (is_null($asset)) {
							continue;
						}
						if ($type == 'bodycopy_content_file') {
							// Its a bodycopy container asset
							$bodycopy_container_edit_fns = $asset->getEditFns();
							$bodycopy_container_edit_fns->generateContentFile($asset);
						} else if ($type == 'metadata_file') {
							// Do not trigger "update asset" event when regenerating metadata
							$mm->regenerateMetadata($assetid, NULL, FALSE);
						} else {
							// Design parse file are not contextable, so just go for once for design parse file generation
							if (isset($processed_design_assetids[$assetid])) {
								continue;
							}

							// If we're not a design for some reason, continue
							if (!($asset instanceof Design)) continue;
							$design_edit_fns = $asset->getEditFns();

							// Take care of parse file contents
							$parse_file = $asset->data_path.'/parse.txt';
							if (is_file($parse_file)) {
								$parse_file_content = file_get_contents($parse_file);
								if (isValidValue($parse_file_content)) {
									// If file content is already valid, we are done here
									continue;
								}
								$converted_content = @iconv(SYS_OLD_ENCODING, SYS_NEW_ENCODING.'//IGNORE', $parse_file_content);
								if (isValidValue($converted_content)) {
									if (!file_put_contents($parse_file, $converted_content)) {
										echo "\nCould not update the parse file content for the asset #".$asset->id."\n";
									}
								} else {
									echo "\nCould not convert the parse file content for the asset #".$asset->id."\n";
								}
							}
							// Parse and process the design, if successful generate the design file
							if (@$design_edit_fns->parseAndProcessFile($asset)) @$asset->generateDesignFile(false);
							// Update respective design customisations
							$customisation_links = $GLOBALS['SQ_SYSTEM']->am->getLinks($assetid, SQ_LINK_TYPE_2, 'design_customisation', true, 'major', 'customisation');
							foreach($customisation_links as $link) {
								$customisation = $GLOBALS['SQ_SYSTEM']->am->getAsset($link['minorid'], $link['minor_type_code']);
								if (is_null($customisation)) continue;
								@$customisation->updateFromParent($asset);
								$GLOBALS['SQ_SYSTEM']->am->forgetAsset($customisation);
							}

							// Mark that we have processed the affected design's the parse file
							$processed_design_assetids[$asset->id] = 1;
						}

						$asset = $GLOBALS['SQ_SYSTEM']->am->forgetAsset($asset);

						echo ".";
					}//end foreach assetids

					$GLOBALS['SQ_SYSTEM']->restoreContext();
				}//end foreach contexts

				$GLOBALS['SQ_SYSTEM']->restoreRunLevel();
				$GLOBALS['SQ_SYSTEM']->restoreCurrentUser();

				exit();
			}//end child process

		}//end foreach asset batch
		echo " done.\n";

	}//end foreach type

}//end regenerate_filesystem_content()


/**
 * Check if the given value is valid for given charset
 *
 * @parm string	$value		String value to check
 * @parm string 	$charset	Charset
 *
 * return boolean
 */
function isValidValue($value, $charset=SYS_NEW_ENCODING)
{
	$result = ($value == @iconv($charset, $charset."//IGNORE", $value));
	return $result;
}


/**
 * Check if the given value is valid for given charset
 *
 * @parm string	$value		String value to check
 * @parm string 	$charset	Charset
 *
 * return boolean
 */
function isValidCharset($charset)
{
	return 'test' == @iconv($charset, $charset, 'test');
}


/**
 * Logs the error in script error log
 *
 */
function log_error_msg($msg)
{
	$msg = date('j-m-y h-i-s').": ".$msg."\n";
	file_put_contents(SCRIPT_LOG_FILE, $msg, FILE_APPEND);
}


/*
* Fork child process. The parent process will sleep until the child
* exits
*
* @return string
*/
function fork()
{
	$child_pid = pcntl_fork();

	switch ($child_pid) {
		case -1:
			trigger_error("Forking failed!");
			return null;
		break;
		case 0: // child process
			return $child_pid;
		break;
		default : // parent process
			$status = null;
			pcntl_waitpid(-1, $status);
			return $child_pid;
		break;
	}
}//end fork()


/**
 * Get CLI Argument
 * Check to see if the argument is set, if it has a value, return the value
 * otherwise return true if set, or false if not
 *
 * @params $arg string argument
 *
 * @return string/boolean
 * @author Matthew Spurrier
 */
function getCLIArg($arg)
{
	return (count($match = array_values(preg_grep("/--" . $arg . "(\=(.*)|)/i",$_SERVER['argv']))) > 0 === TRUE) ? ((preg_match('/--(.*)=(.*)/',$match[0],$reg)) ? $reg[2] : true) : false;

}//end getCLIArg()


/**
 * Print the usage of this script
 *
 * @return void
 */
function print_usage()
{
	echo "\nThis script performs the charset conversion on the database from the older to the current encoding.";
	echo "\nIt also regenerates the content files (bodycopy, metadata and design) of asset associated with the db record.\n\n";

	echo "Usage: php ".basename(__FILE__)." --system=<SYSTEM_ROOT> --old=<OLD_CHARSET> [--new=<NEW_CHARSET>] [--rootnode=<ROOT_NODE>] [--skip-tables=<TABLES>] [--ignore-rollback] [--report]\n\n";
	echo "\t<SYSTEM_ROOT>		: The root directory of Matrix system.\n";
	echo "\t<OLD_CHARSET>		: Previous charset of the system. (eg. UTF-8, Windows-1252, etc)\n";
	echo "\t<NEW_CHARSET>		: New charset of the system. (eg. UTF-8, Windows-1252, etc)\n";
	echo "\t<ROOT_NODE>			: Assetid of the rootnode (all children of the rootnode will be processed by the script).\n";
	echo "\t<TABLES>			: Comma seperated list of db table names (without prefix) to ignore.\n";
	echo "\t[--report]			: Issue a report only instead of also trying to convert the data.\n";
	echo "\t[--ignore-rollback]	: Do not include rollback tables.\n";

	echo "\nWARNING: IT IS STRONGLY RECOMMENDED THAT YOU BACKUP YOUR SYSTEM BEFORE RUNNING THIS SCRIPT\n\n";

}//end print_usage()


/**
 * Convert all multi-byte characters to entities.
 *
 * @param string $str The string to convert.
 *
 * @return string
 */
function htmlallentities($str)
{
    $res    = '';
    $strlen = strlen($str);
    for ($i = 0; $i < $strlen; $i++) {
        $byte = ord($str[$i]);
        if($byte < 128) // 1-byte char
            $res .= $str[$i];
        elseif($byte < 192) // invalid utf8
            $res .= '&#'.ord($str[$i]).';';
        elseif($byte < 224) // 2-byte char
            $res .= '&#'.((63&$byte)*64 + (63&ord($str[++$i]))).';';
        elseif($byte < 240) // 3-byte char
            $res .= '&#'.((15&$byte)*4096 + (63&ord($str[++$i]))*64 + (63&ord($str[++$i]))).';';
        elseif($byte < 248) // 4-byte char
        $res .= '&#'.((15&$byte)*262144 + (63&ord($str[++$i]))*4096 + (63&ord($str[++$i]))*64 + (63&ord($str[++$i]))).';';
    }

    return $res;

}//end htmlallentities()


/**
 * Report potentially problematic characters.
 *
 * @param array $values An array values with atleast one entry containing problematic chars
 *
 * @return string
 */
function listProblematicCharacters($values)
{
	$str = '';
	foreach($values as $field_name => $value) {
		$entified = htmlallentities($value);
		preg_match_all('/&#([0-9]+);/', $entified, $matches);
		$codes     = array_unique($matches[1]);
		$probChars = '';
		foreach ($codes as $code) {
			$probChars .= html_entity_decode('&#'.$code.';', ENT_COMPAT, 'utf-8').' ('.$code.'), ';
		}

		$str .= "\n\t".$field_name.": ". preg_replace('/,\s*$/', '', $probChars);
	}

	return $str;

}//end listProblematicCharacters()


/**
* Returns the all the db tables info
*
* @return array
* @access public
*/

function get_tables_info()
{
	require_once SQ_LIB_PATH.'/db_install/db_install.inc';
	$packages_installed = $GLOBALS['SQ_SYSTEM']->getInstalledPackages();

	if (empty($packages_installed)) {
		return Array();
	}

	$table_info = Array();
	foreach ($packages_installed as $package_array) {
		if ($package_array['code_name'] == '__core__') {
			$table_file = SQ_CORE_PACKAGE_PATH.'/tables.xml';
		} else {
			$table_file = SQ_PACKAGES_PATH.'/'.$package_array['code_name'].'/tables.xml';
		}

		if (file_exists($table_file)) {
			$table_data = parse_tables_xml($table_file);
			$table_info += $table_data['tables'];
		}
	}

	return $table_info;

}//end get_tables_info()


?>
