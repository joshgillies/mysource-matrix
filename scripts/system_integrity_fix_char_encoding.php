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
 * $Id: system_integrity_fix_char_encoding.php,v 1.14 2013/04/30 03:18:08 ewang Exp $
 */

/**
 * Script to replace the non-utf8 smart quotes chars by their regular counterpart chars and
 * if string is still invalid after replacement, perform charset conversion on string
 * and then regenerates the content files (bodycopy, metadata and design) of the affected assets
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

// Make sure iconv is available.
if (function_exists('iconv') == FALSE) {
    echo "This script requires the php iconv module which isn't available.\n";
    echo "Install that module and try again.\n";
    exit(1);
}

// Tables where the values are to fixed
//
// Array(
//		<db table> => Array(
//					"assetid" 	=> <fieldname containing the assetid>,
//					"contextid" => <fieldname containing the record's contextid>,
//					"value" 	=> <fieldname containing the record's value>,
//					"key" 		=> [<third field as part of the record's primary key>],
//				),
//		)
$tables = Array(
    'sq_ast_attr_val'       => Array(
        'assetid' 		=> 'assetid',
        'contextid'		=> 'contextid',
        'value'			=> 'custom_val',
        'key'			=> 'attrid',
    ),
    'sq_ast_mdata_val'      => Array(
        'assetid' 		=> 'assetid',
        'contextid'		=> 'contextid',
        'value'			=> 'value',
        'key'			=> 'fieldid',
    ),
    'sq_ast_mdata_dflt_val' => Array(
        'assetid'		=> 'assetid',
        'contextid'		=> 'contextid',
        'value'			=> 'default_val',
        'key'			=> '',
    ),
    'sq_ast_attr_uniq_val'  => Array(
        'assetid'		=> 'assetid',
        'contextid'		=> 'contextid',
        'value'			=> 'custom_val',
        'key'			=> 'owning_attrid',
    ),
);

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
    echo "\nIMPORTANT: This script will replace all the smart quote chars by their regular counterpart chars. And if value string is still\n";
    echo "invalid in the current system's charset then it performs charset conversion on string from older to current encoding\n";
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

$pid = fork();
if (!$pid) {

    // NOTE: This seemingly ridiculousness allows us to workaround Oracle, forking and CLOBs
    // if a query is executed that returns more than 1 LOB before a fork occurs,
    // the Oracle DB connection will be lost inside the fork
	require_once $SYSTEM_ROOT.'/core/include/init.inc';

	$summary = fix_db($root_node_id, $tables);

	// Get the list of assetids for which we need to regenerate  the filesystem content
	// to reflect the changes made in the db
	$affected_assetids = get_affected_assetids($summary['affected_assetids']);

	// Also get the context ids
	$contextids = array_keys($GLOBALS['SQ_SYSTEM']->getAllContexts());

	file_put_contents(SYNC_FILE, serialize(Array('affected_assetids' => $affected_assetids, 'db_summary' => $summary, 'contextids' => $contextids)));

	exit();

}//end child process

if (!is_file(SYNC_FILE)) {
	echo "Expected sync file containing the affected assetids not found. Only database was updated\n";
	exit(1);
}

$summary = unserialize(file_get_contents(SYNC_FILE));

// Fix the filesystem content to reflect the changes made in the db
if ($reportOnly == FALSE) {
	regenerate_filesystem_content($summary['affected_assetids'], $summary['contextids']);

	echo "Number of db records replaced successfully: ".$summary['db_summary']['records_fixed_count']."\n";
	echo "Total errors recorded: ".$summary['db_summary']['error_count']."\n";
} else {
	echo "Number of db records that need replacing: ".$summary['db_summary']['records_fixed_count']."\n";
}

echo "Total time taken to run the script: ".round(microtime(TRUE)-$start_time, 2)." second(s)\n";

if ($summary['db_summary']['error_count'] > 0)	{
	echo "\nPlease check ".SCRIPT_LOG_FILE." file for errors\n\n";
}
echo "\n";

exit();

// End of Main program /////////////////////////////////


/**
 * Fixes the char encoding in the given tables in the database
 *
 * @param int 	$root_node		Assetid of rootnode, all childern of rootnode will be processed for char replacement
 * @param array	$tables			DB tables and colunms info
 *
 * @return void
 */
function fix_db($root_node, $tables)
{
    global $reportOnly;

	// Get the list of attrids of the type 'serialise'
	$sql = "SELECT attrid FROM sq_ast_attr where type='serialise'";
	$serialise_attrids = array_keys(MatrixDAL::executeSqlGrouped($sql));

    $target_assetids = array_keys($GLOBALS['SQ_SYSTEM']->am->getChildren($root_node));
    array_unshift($target_assetids, $root_node);

    if (empty($target_assetids)) {
        echo "\n\nAsset #${root_node} not found or no assets found underneath\n";
        return;
    }
    echo "\n\nNumber of assets to look into : ".count($target_assetids)." \n";

    $errors                = Array();
    $records_fixed_count   = 0;
    $invalid_asset_records = Array();

	// Assets that will require filesystem content regeneration
	$affected_assetids = Array();

    $GLOBALS['SQ_SYSTEM']->changeDatabaseConnection('db2');

    $chunks = array_chunk($target_assetids, 50);

    // Go through 50 assets at a time.
    $count = 0;
    foreach ($chunks as $assetids) {
        foreach($tables as $table => $fields) {
            $sql  = 'SELECT '.trim(implode(',',$fields),',').' FROM '.$table;
            $sql .= ' WHERE assetid IN (\''.implode('\',\'', $assetids).'\')';

            $results = MatrixDAL::executeSqlAssoc($sql);

            foreach($results as $record) {
                $count++;
                if ($count % 100 == 0) {
                    echo '.';
                }

                $value     = isset($record[$fields['value']])     ? $record[$fields['value']]     : NULL;
                $assetid   = isset($record[$fields['assetid']])   ? $record[$fields['assetid']]   : NULL;
                $key       = isset($record[$fields['key']])       ? $record[$fields['key']]       : NULL;
                $contextid = isset($record[$fields['contextid']]) ? $record[$fields['contextid']] : NULL;

                if (is_null($value) || is_null($assetid) || is_null($contextid) || ($fields['key'] && is_null($key))) {
                    continue;
                }

                if (empty($value)) {
                    continue;
                }

                // If it's the same in the new and old encodings, that's good.
                $checked = @iconv(SYS_OLD_ENCODING, SYS_NEW_ENCODING.'//IGNORE', $value);

                if ($value === $checked) {
                    continue;
                }

                $update_required = FALSE;
                $invalid_asset_records[] = array(
                    'asset' => $assetid,
                    'table' => $table,
                    'value' => $value,
                );

				// Check if the value is now valid
                if (!isValidValue($value)) {
                    // String might also contains the char(s) from older encoding which is/are not valid for current one
                    // See if we can convert these without igonoring or interprating any chars
					if ($table == 'sq_ast_attr_val' && in_array($key, $serialise_attrids)) {
						// Need to handle serialised data differently
						$us_value = @unserialize($value);
						if (is_array($us_value)) {
							array_walk_recursive($us_value, 'fix_char');
							$converted_value = serialize($us_value);
						} else if (is_string($us_value)) {
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
                        $update_required = TRUE;
                    }
                }
               else {
	// if it's a valid encoded value, but was convertable before with iconv using old encoding
	// it might be only because value is already properly encoded with new encoding.  so use md_detect to double check
	$encoding = mb_detect_encoding($value);
	if($encoding === SYS_NEW_ENCODING) {
		 array_pop($invalid_asset_records);
		continue;
	}
              }

                if ($update_required) {
                    if (!$reportOnly) {
                        $GLOBALS['SQ_SYSTEM']->doTransaction('BEGIN');
                        try {
                            $sql = "UPDATE
                                $table
                                SET
                                ".$fields['value']."=:value
                                WHERE
                                ".$fields['assetid']."=:assetid".
                                " AND ".$fields['contextid']."=:contextid".
                                (!is_null($key) ? " AND ".$fields['key']."=:key" : "");


                            $update_sql = MatrixDAL::preparePdoQuery($sql);

                            MatrixDAL::bindValueToPdo($update_sql, 'value', $value);
                            MatrixDAL::bindValueToPdo($update_sql, 'assetid', $assetid);
                            MatrixDAL::bindValueToPdo($update_sql, 'contextid', $contextid);
                            if (!is_null($key)) MatrixDAL::bindValueToPdo($update_sql, 'key', $key);

                            $execute = MatrixDAL::executePdoAssoc($update_sql);
                            if (count($execute) > 1) {
                                $sql = str_replace(':assetid', $assetid, $sql);
                                $sql = str_replace(':contextid', $contextid, $sql);
                                $sql = str_replace(':contextid', $contextid, $sql);
                                $sql = !is_null($key) ? str_replace(':key', $key, $sql) : $sql;

                                $errors[] = array(
                                    'asset' => $assetid,
                                    'table' => $table,
                                );

                                $msg = "Executing query \"$sql\" will affect ".count($execute)." (more than 1) records! Ignoring this sql.";
                                log_error_msg($msg);

                                $GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');

                                continue;
                            }

                            $GLOBALS['SQ_SYSTEM']->doTransaction('COMMIT');
                            $records_fixed_count++;
							$affected_assetids[$table][] = $assetid;

                        } catch (Exception $e) {
                            $errors[] = array(
								'asset' => $assetid,
								'table' => $table,
							);
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
                    // This record contained invalid value. Either the invalid char(s) in it was/were not in the replacement array
                    // or trying to carryout charset conversion (without losing any data) still resulted into invalid value
                    // Hence replacement was not carried out.
                    $errors[] = array(
                                 'asset' => $assetid,
                                 'table' => $table,
                                 'value' => $value,
                                );

                    $msg = "Asset with ".$fields['assetid']."=#$assetid, ".
                        (!is_null($key) ? $fields['key']."=#$key, and " : "and ").
                        $fields['contextid']."=#$contextid in table $table ".
                        "contains invalid char(s), which were not replaced because ".
                        "either those invalid chars were not defined in the replacement array or the charset conversion was not successful".
                        "\nPotentially invalid characters include: ".listProblematicCharacters($value);
                    log_error_msg($msg);
                }

            }//end foreach
        }//end foreach
    }

    $GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();

    unset($target_assetids);

    echo "\n";

    $invalid_count = sizeof(array_keys($invalid_asset_records));
    echo "Number of db records with invalid char(s): ".$invalid_count."\n";
    if ($invalid_count > 0) {
        foreach ($invalid_asset_records as $k => $details) {
            echo "\tAsset: ".$details['asset']." in table ".$details['table'];
            echo "\tPossibly problematic characters: ".listProblematicCharacters($details['value'])."\n";
        }
        echo "\n";
    }

	return Array(
			'error_count' => sizeof(array_keys($errors)),
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
					$schemaid = array_keys($GLOBALS['SQ_SYSTEM']->am->getParents($mfield_assetid, 'metadata_schema'));
					$affected_assetids['metadata_file'] = array_merge($affected_assetids['metadata_file'], $mm->getSchemaAssetids());
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
							// If we're not a design for some reason, continue
							if (!($asset instanceof Design)) continue;
							$design_edit_fns = $asset->getEditFns();

							// Take care of parse file contents
							$parse_file = $asset->data_path.'/parse.txt';
							if (is_file($parse_file)) {
								$parse_file_content = file_get_contents($parse_file);
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
    echo "\nThis script replaces all the non-utf8 smart quotes chars by their respective regular couterpart chars.";
    echo "\nIf string is still invalid in current charset encoding aftet the replacement then script will perform chaset";
    echo "\nconversion on string from previous charset to the current one.\n\n";

    echo "Usage: php ".basename(__FILE__)." --system=<SYSTEM_ROOT> --old=<OLD_CHARSET> [--new=<NEW_CHARSET>] [--rootnode=<ROOT_NODE>] [--report]\n\n";
    echo "\t<SYSTEM_ROOT> : The root directory of Matrix system.\n";
    echo "\t<OLD_CHARSET> : Previous charset of the system. (eg. UTF-8, Windows-1252, etc)\n";
    echo "\t<NEW_CHARSET> : New charset of the system. (eg. UTF-8, Windows-1252, etc)\n";
    echo "\t<ROOT_NODE>   : Assetid of the rootnode (all children of the rootnode will be processed by the script).\n";
    echo "\t<--report>    : Issue a report only instead of also trying to convert the assets.\n";

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
 * @param string $value A string containing problematic characters.
 *
 * @return string
 */
function listProblematicCharacters($value)
{
    $entified = htmlallentities($value);
    preg_match_all('/&#([0-9]+);/', $entified, $matches);
    $codes     = array_unique($matches[1]);
    $probChars = '';
    foreach ($codes as $code) {
		$probChars .= html_entity_decode('&#'.$code.';', ENT_COMPAT, 'utf-8').' ('.$code.'), ';
    }

    return preg_replace('/,\s*$/', '', $probChars);

}//end listProblematicCharacters()


?>

