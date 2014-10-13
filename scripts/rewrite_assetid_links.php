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
 * The script to rewrite the ./?a=xxx links to the full URL based the supplied parameters.
 * IMPORTANT: SYSTEM MUST BE BACKEDUP BEFORE RUNNING THIS SCRIPT!!!
 *
 * @author  Chiranjivi Upreti <cupreti@squiz.com.au>
 * @version $Revision: $
 * @package MySource_Matrix
 */

error_reporting(E_STRICT);
if ((php_sapi_name() != 'cli')) {
	trigger_error("You can only run this script from the command line\n", E_USER_ERROR);
}

$SYSTEM_ROOT = getCLIArg('system');
if (!$SYSTEM_ROOT) {
	echo "ERROR: You need to supply the path to the System Root\n";
	print_usage();
}

if (!is_dir($SYSTEM_ROOT) || !is_readable($SYSTEM_ROOT.'/core/include/init.inc')) {
	$msg = "ERROR: Path provided doesn't point to a Matrix installation's System Root. Please provide correct path and try again.\n";
	print_usage();
}

if (ini_get('memory_limit') != '-1') ini_set('memory_limit', '-1');

require_once $SYSTEM_ROOT.'/core/include/init.inc';

$keep_site_assetids = getCLIArg('sites-to-keep');
if (!_valid_site_assetids($keep_site_assetids)) {
	echo "ERROR: You need to specify the comma seperated list of valid site asset ids that to be kept.\n";
	print_usage();
}

$delete_site_assetids = getCLIArg('sites-to-delete');
if (!_valid_site_assetids($delete_site_assetids)) {
	echo "ERROR: You need to specify the comma seperated list of valid site asset ids that are to be deleted.\n";
	print_usage();
}

$REWRITE_URLS = getCLIArg('rewrite-urls');
if (!_valid_site_url($REWRITE_URLS)) {
	echo "ERROR: You need to specify the comma seperated list of valid urls (full URL with protocol bit) to rewrite ./?a=[ASSETID] links to.\n";
	print_usage();
}

$report_mode = !getCLIArg('execute');

if (!$report_mode) {
	echo "WARNING: The script will go through all the assets in the site to keep and rewrite the asset id links (./?a=xxx) in asset attribute and metadata values that are linked to the site to keep, but not to site to 'delete'. It highly recommended to backup the system before running this script.\n";
	echo "Are you sure you want to proceed (Y/n)? \n";
	$yes_no = rtrim(fgets(STDIN, 4094));
	if ($yes_no != 'Y') {
		echo "\nScript aborted. \n";
		exit;
	}
}

// All the assetids belonging to the site(s) to be kept
// These are the assets we will looking to the rewrite the "./?a=xxx" links
$KEEP_ASSETIDS = Array();
foreach($keep_site_assetids as $site_assetid) {
	$KEEP_ASSETIDS += $GLOBALS['SQ_SYSTEM']->am->getChildren($site_assetid);
}

// All the assetids belonging to the site(s) to be deleted
$DELETE_ASSETIDS = Array();
foreach($delete_site_assetids as $site_assetid) {
	$DELETE_ASSETIDS += $GLOBALS['SQ_SYSTEM']->am->getChildren($site_assetid);
}


$GLOBALS['SQ_SYSTEM']->changeDatabaseConnection('db2');
if (!$report_mode) {
	$GLOBALS['SQ_SYSTEM']->doTransaction('BEGIN');
}


// Global variables
$REMAPPED_URLS = Array();
$ERRORS = Array();
$REPORT = Array(
			// Matching URL found for ./?a=xxx link
			'match' => Array(),
			// Matching URL not found for ./?a=xxx link
			'no-match' => Array(),
			// Asset id in /?a=xxx link not found in both 'delete' and 'keep' sites
			'dead' => Array(),
			// Notice links with major id exist in 'keep' and minor does not in 'delete' site
			// Yes thats right, this bit is not quite related to ./?a=xxx links
			'notice_links' => Array(),

		);

// All the tables info
$tables_info = get_tables_info();

// Table/field where the values are to be looked into
$tables = Array(
			'ast_attr_val' => 'custom_val',
			'ast_mdata_val' => 'value',
	);

$count = 0;
$records_updated_count = 0;
foreach($tables as $table => $value_field) {

	// Select the 'value' and 'key' fields from the table
	$key_fields = isset($tables_info[$table]['primary_key']) ? $tables_info[$table]['primary_key'] : '';
	if (empty($key_fields)) {
		$ERRORS[] = 'Ignoring table "'.$table.'". Table info for this table not found';
		continue;
	}
	$select_fields = array_merge(Array($value_field), $key_fields);
	// We want 'assetid' to be at first item in the select
	if ($select_fields[0] != 'assetid') {
		$assetid_index = array_search('assetid', $select_fields);
		unset($select_fields[$assetid_index]);
		array_unshift($select_fields, 'assetid');
	}

	// Go through 100 assets at a time to prevent memory bust
	$chunks = array_chunk(array_keys($KEEP_ASSETIDS), 100);

	foreach($chunks as $assetids) {

		$sql  = 'SELECT '.implode(',', $select_fields).' FROM sq_'.$table.' WHERE assetid IN (\''.implode('\',\'', $assetids).'\')';
		$results = MatrixDAL::executeSqlAssoc($sql);

		foreach($results as $record) {
			$count++;
			if ($count % 100 == 0) {
				echo '.';
			}

			// Key fields data
			$key_values = Array();
			foreach($key_fields as $key_field) {
				$temp_key_v = array_get_index($record, $key_field, NULL);
				if (is_null($temp_key_v)) {
					// Primary key field must be there
					continue 2;
				}
				$key_values[$key_field] = $temp_key_v;
			}//end foreach

			// Value field to check
			$org_value = array_get_index($record, $value_field, '');

			if (empty($org_value)) {
				continue;
			}

			// Replace the "./?a=xxx" links in the content
			$new_value = _rewrite_assetid_links($org_value, $record, $table == 'ast_mdata_val');

			if ($new_value !== FALSE && !$report_mode) {

				// Generate update sql
				$bind_vars = Array();
				$set_sql = $value_field.'=:'.$value_field.'_v';
				$bind_vars[$value_field.'_v'] = $new_value;

				$where_sql = Array();
				foreach($key_values as $field_name => $value) {
					$where_sql[] = $field_name.'=:'.$field_name.'_k';
					$bind_vars[$field_name.'_k'] = $value;
				}

				try {
					$sql = 'UPDATE sq_'.$table.'
							SET '.$set_sql.'
							WHERE '.implode(' AND ', $where_sql);

					$update_sql = MatrixDAL::preparePdoQuery($sql);
					foreach($bind_vars as $var_name => $var_value) {
						MatrixDAL::bindValueToPdo($update_sql, $var_name, $var_value);
					}

					// Execute the update query
					$execute = MatrixDAL::execPdoQuery($update_sql);
					$records_updated_count++;

					if (count($execute) > 1) {
						// This should never happen
						foreach($bind_vars as $var_name => $var_value) {
							$sql = str_replace(':'.$var_name, "'".$var_value."'", $sql);
						}
						$ERRORS[] = 'Executing query "'.$sql.'" will affect '.count($execute).' records, instead of expected single record.';
					}
				} catch (Exception $e) {
					$ERRORS[] = "Unexpected error occured while updating database: ".$e->getMessage();
				}//end foreach records

				if (!empty($ERRORS)) {
					break 2;
				}
			}
		}//end foreach assetids chunk

	}//end foreach records
}//end foreach tables

// Ok this bit is not quite related to rest of the script
// Get the all notice links and check if minor assetid is outside the site to keeep
$sql = 'SELECT majorid, minorid, value
		FROM sq_ast_lnk
		WHERE
			link_type = '.MatrixDAL::quote(SQ_LINK_NOTICE);
$results = MatrixDAL::executeSqlAssoc($sql);
foreach($results as $row) {
	$majorid = $row['majorid'];
	$minorid = $row['minorid'];
	$link_value = $row['value'];

	// Report major ids that are in 'site to keep' but whose minor id is not
	if (isset($KEEP_ASSETIDS[$majorid]) && !isset($KEEP_ASSETIDS[$minorid])) {
		if (isset($DELETE_ASSETIDS[$minorid])) {
			$REPORT['notice_links'][] = $majorid.','.$minorid.','.$link_value.',Not found';
		} else {
			$REPORT['notice_links'][] = $majorid.','.$minorid.','.$link_value.',Linked outside';
		}
	}
}//end foreach


if (!$report_mode) {
	$GLOBALS['SQ_SYSTEM']->doTransaction(empty($ERRORS) ? 'COMMIT' : 'ROLLBACK');
}
$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();

if (!empty($ERRORS)) {
	echo "Following error(s) occured when running the script:\n".implode("\n", $ERRORS)."\n";
	echo "Database was not updated.\n";
	exit(1);
}

// Write to the report file
_log_script_report();

// Print the report summary
echo "\nTotal asset id links matched to the rewrite URL(s): ".count($REPORT['match']);
echo "\nTotal asset id links not matched to the rewrite URL(s): ".count($REPORT['no-match']).'*';
echo "\nTotal asset id links that does not belongs to both 'delete' and 'keep' sites: ".count($REPORT['dead']).'*';
echo "\nTotal assets notice linked outside the site(s) to keep: ".count($REPORT['notice_links']).'*';
if ((count($REPORT['no-match'])+count($REPORT['dead'])+count($REPORT['notice_links'])) > 0) {
	echo "\nManual review of (*) items required. Please check 'data/private/logs/rewrite_assetid_links_new.php.log' file for the full report.";
} else {
	echo "\nPlease check 'data/private/logs/rewrite_assetid_links_new.php.log' file for the full report";
}
echo "\n";

if ($report_mode) {
	echo "\nThe script ran in 'report' mode. Database was not updated.\n";
} else if ($records_updated_count > 0) {
	echo "\n".$records_updated_count." db records were updated.";
	echo "\nPlease run the script script/regenerate_file_system.php to regenerate asset content files";
}
echo "\n";

exit;

// End of main program //////////////////////////


/**
* Replace all the ./?a=[assetid] links in the given value with the full URL
* Returns boolean FALSE if the value does not contain assetid links
*
* @param string $content     Content where the assetid links will be re-written
* @param array  $record      Asset id, context id info of the record
* @param array  $is_metadata If the record value is metadata value
*
* @return void
*/
function _rewrite_assetid_links($content, $record, $is_metadata)
{
	global $ERRORS, $REPORT;
	global $REMAPPED_URLS, $REWRITE_URLS;
	global $KEEP_ASSETIDS, $DELETE_ASSETIDS;

	$assetid = $record['assetid'];
	$contextid = $record['contextid'];

	// Get all the matrix ./a=xx links in the content
	if ($is_metadata) {
		// '=' char is escaped in the metadata value
		preg_match_all('!\./\?a\\\=([0-9]+(?:\:[0-9a-z]+\$?)?)!msi', $content, $matches);
	} else {
		preg_match_all('!\./\?a=([0-9]+(?:\:[0-9a-z]+\$?)?)!msi', $content, $matches);
	}
	if (empty($matches[1])) {
		return FALSE;
	}

	$new_assetids = Array();
	foreach($matches[1] as $index => $value) {
		// Trimming '$' for image var assetid in ./?a=xxx link
		$link_assetid = rtrim($value, '$');

		// Use 'real assetid' to check if asset belongs to 'keep'/'delete' sites
		$assetid_bits = explode(':', $link_assetid);
		$real_assetid = $assetid_bits[0];
		if (isset($KEEP_ASSETIDS[$real_assetid]) && !isset($DELETE_ASSETIDS[$real_assetid])) {
			// We do not need to remap this asset link
			continue;
		}

		if (!isset($DELETE_ASSETIDS[$real_assetid])) {
			// This does not belong to both 'keep' and 'delete' site
			$REPORT['dead'][] = $assetid.','.$contextid.','.$link_assetid.',,0,0';
			continue;
		}

		// If have alrady evaluted the URL for this assetid link
		if (!isset($REMAPPED_URLS[$value]['url'])) {
			$new_assetids[$value] = $link_assetid;
		} else if (!$REMAPPED_URLS[$value]['url']) {
			$REPORT['no-match'][] = $assetid.','.$contextid.','.$REMAPPED_URLS[$value]['info'];
		} else if ($REMAPPED_URLS[$value]) {
			$REPORT['match'][] = $assetid.','.$contextid.','.$REMAPPED_URLS[$value]['info'];
		}
	}//end foreach

	if (!empty($new_assetids)) {
		foreach($new_assetids as $link_assetid_str => $link_assetid) {
			$asset_urls = $GLOBALS['SQ_SYSTEM']->am->getUrls($link_assetid);
			if (empty($asset_urls)) {
				continue;
			}
			$link_urls = Array();
			foreach($asset_urls as $url_info) {
				if ($url_info['http']) {
					$link_urls[] = 'http://'.$url_info['url'];
				}
				if ($url_info['https']) {
					$link_urls[] = 'https://'.$url_info['url'];
				}
			}

			foreach($REWRITE_URLS as $base_url) {
				foreach($link_urls as $link_url) {
					if (!empty($link_url) && strpos($link_url.'/', $base_url.'/') === 0) {
						$REMAPPED_URLS[$link_assetid_str]['url'] = $link_url;
						$REMAPPED_URLS[$link_assetid_str]['info'] = $link_assetid.','.implode('^', $link_urls).','.$base_url.','.(isset($KEEP_ASSETIDS[$link_assetid]) ? '1' : '0').',1';
						$REPORT['match'][] = $assetid.','.$contextid.','.$REMAPPED_URLS[$link_assetid_str]['info'];
						break 2;
					}
				}//end foreach
			}//end foreach rewrite urls
			if (!isset($REMAPPED_URLS[$link_assetid_str])) {
				// This asset urls does not match with any rewrite urls
				$REMAPPED_URLS[$link_assetid_str]['url'] = '';
				$REMAPPED_URLS[$link_assetid_str]['info'] = $link_assetid.','.implode('^', $link_urls).',,'.(isset($KEEP_ASSETIDS[$link_assetid]) ? '1' : '0').',1';
				$REPORT['no-match'][] = $assetid.','.$contextid.','.$REMAPPED_URLS[$link_assetid_str]['info'];
			}//end if

		}//end foreach

	}//end if

	$replacements = Array();
	foreach($matches[1] as $value) {
		if (!empty($REMAPPED_URLS[$value]['url'])) {
			if ($is_metadata) {
				$replacements['./?a\='.$value] = $REMAPPED_URLS[$value]['url'];
			} else {
				$replacements['./?a='.$value] = $REMAPPED_URLS[$value]['url'];
			}
		}//end if
	}//end foreach

	// Sort the replacement by assetid length
	uksort($replacements, function($a, $b) {
							 return strlen($a) < strlen($b);
			}
		);

	if (!empty($replacements)) {
		if (!_is_serialised($content)) {
			$content = str_replace(array_keys($replacements), $replacements, $content);
		} else {
			$array_data = unserialize($content);
			array_walk_recursive($array_data, function(&$item, $key) use ($replacements) {
												$item = str_replace(array_keys($replacements), $replacements, $item);
					}
				);
			$content = serialize($array_data);
		}
		return $content;
	}
	return FALSE;

}//end _rewrite_assetid_links()


/**
* Check if the given string is serialised string
*
* @param string $value
*
* @return boolean
*/
function _is_serialised($value)
{
	// For the purpose of this script, we dont care about serialised boolean false value
	return @unserialize($value);

}//end _is_serialised()


/**
* Returns the array valid site assetids from the  list of comma seperated site assetids string
*
* @param string $value
*
* @return boolean
*/
function _valid_site_assetids(&$value)
{
	$assetids = explode(',', $value);
	$valid_sites = array_keys($GLOBALS['SQ_SYSTEM']->am->getAssetInfo($assetids, Array('site'), TRUE, 'name'));
	$invalid_sites = array_diff($assetids, $valid_sites);
	if ($invalid_sites) {
		echo "ERROR: Following assetids are not site assets:\n".implode("\n", $invalid_sites)."\n";
		return FALSE;
	}
	if (empty($valid_sites)) {
		return FALSE;
	}

	$value = $valid_sites;

	return TRUE;

}//end _valid_site_assetids()


/**
* Returns TRUE if all the comma seperate URLs are valid site URLs
*
* @param string $value
*
* @return boolean
*/
function _valid_site_url(&$value)
{
	$urls = explode(',', $value);
	if (empty($urls)) {
		return FALSE;
	}

	$root_urls = explode("\n", SQ_CONF_SYSTEM_ROOT_URLS);

	$sql = 'SELECT url FROM sq_ast_url WHERE ';
	$bind_vars = Array();
	$protocol = Array();
	$valid_urls = Array();
	foreach($urls as $index => $full_url) {
		$url_parts = explode('://', $full_url);
		if (count($url_parts) != 2) {
			continue;
		}
		if (in_array($url_parts[1], $root_urls) && ($url_parts[0] == 'http' || $url_parts[0] == 'https')) {
			$valid_urls[] = $full_url;
		} else {
			$protocol_field = $url_parts[0] == 'http' ? 'http' : 'https';
			$sql .= ' (url = :url'.$index.' AND '.$protocol_field.' = \'1\') OR';

			$bind_vars['url'.$index] = $url_parts[1];
			$protocol[$url_parts[1]] = $protocol_field;
		}
	}
	$sql =substr($sql, 0, strlen($sql)-2);

	$query = MatrixDAL::preparePdoQuery($sql);
	foreach($bind_vars as $var_name => $var_value) {
		MatrixDAL::bindValueToPdo($query, $var_name, $var_value);
	}
	$result = MatrixDAL::executePdoAssoc($query);

	foreach($result as $row) {
		$valid_urls[] = $protocol[$row['url']].'://'.$row['url'];
	}
	$invalid_sites = array_diff($urls, $valid_urls);
	if ($invalid_sites) {
		echo "ERROR: Following urls does not belongs to any site:\n".implode("\n", $invalid_sites)."\n";
		return FALSE;
	}
	if (empty($valid_urls)) {
		return FALSE;
	}

	// Sort urls by length i.e. longer url on top
	uasort($valid_urls, function ($a, $b) {
							return strlen($a) < strlen($b);
			}
		);

	$value = $valid_urls;

	return TRUE;

}//end _valid_site_url()


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
	echo "\nThe script will go through all the assets in the site to keep and rewrite the asset id links (./?a=xxx) in asset attribute and metadata values that are linked to the site to keep, but not to site to 'delete'. It highly recommended to backup the system before running this script.\n";

	echo "Usage: php ".basename(__FILE__)." --system=<SYSTEM_ROOT> --sites-to-keep=<SITES_TO_KEEP> --sites-to-delete=<SITES_TO_DELETE> --rewrite-urls=<REWRITE_URLS> [--execute]\n\n";
	echo "\t<SYSTEM_ROOT>       : The root directory of Matrix system.\n";
	echo "\t<SITES_TO_KEEP>     : Comma seperated list of asset id of sites to keep.\n";
	echo "\t<SITES_TO_DELETE>   : Comma seperated list of asset id of sites to delete.\n";
	echo "\t<REWRITE_URLS>      : Comma seperated list of full site URLs to map asset id links to.\n";
	echo "\t[--execute]         : If specifed update the db, otherwise run in 'report' mode.\n";

	echo "\nNOTE: It is highly recommended to backup the system before running this script.\n";

	exit(1);

}//end print_usage()


/**
* Logs the report in script log file
*
* @return void
* @access public
*/
function _log_script_report()
{
	global $REPORT;
	$script_log_file = SQ_LOG_PATH.'/'.basename(__FILE__).'.log';

	$report_output = '';
	foreach($REPORT as $report_type => $data) {
		$report_output .= strtoupper($report_type)."\n";
		foreach($data as $report_row) {
			$report_output .= $report_row."\n";
		}//end foreach
		$report_output .= "\n\n";
	}//end foreach

	file_put_contents($script_log_file, $report_output);

}//end _log_script_report()


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
