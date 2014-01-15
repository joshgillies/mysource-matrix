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
* $Id: rollback_management.inc,v 1.12 2013/03/01 06:14:00 cupreti Exp $
*
*/


/**
* Rollback Management Functions
*
* Purpose
* 	Functions to manage data stored in Rollback
*
* @author Marc McIntyre <mmcintyre@squiz.net>
* @author Greg Sherwood <gsherwood@squiz.net>
* @author Mark Brydon <mbrydon@squiz.net>
* @version $Revision: 1.12 $
* @package MySource_Matrix_Packages
* @subpackage __core__
*/


/**
* Truncates rollback tables
*
* @param string	$table_name	the tablename to update
*
* @return void
* @access public
*/
function truncate_rollback_entries($table_name)
{
	$sql = 'TRUNCATE TABLE sq_rb_'.$table_name;
	try {
		$result = MatrixDAL::executeSql($sql);
	} catch (Exception $e) {
		throw new Exception('Unable to truncate table '.$table_name.' due to the following error: '.$e->getMessage());
	}//end try catch

}//end truncate_rollback_entries()


/**
* Closes off rollback entries to the specified date
*
* @param string	$table_name	the tablename to update
* @param string	$date		the date to close to
*
* @return int
* @access public
*/
function close_rollback_entries($table_name, $date)
{
	$sql = 'UPDATE sq_rb_'.$table_name.' SET sq_eff_to = :date1 WHERE sq_eff_to IS NULL';
	$affected_rows = 0;

	try {
		$query = MatrixDAL::preparePdoQuery($sql);
		MatrixDAL::bindValueToPdo($query, 'date1', $date);
		$affected_rows = MatrixDAL::execPdoQuery($query);
	} catch (Exception $e) {
		throw new Exception('Unable to update rollback table '.$table_name.' due to the following error:'.$e->getMessage());
	}//end try catch

	return $affected_rows;

}//end close_rollback_entries()


/**
* Opens any rollback entries that have not already been opened
*
* @param string	$table_name	the tablename to update
* @param array	$table_colums	the columns to update
* @param string	$date		the date to close to
*
* @return int
* @access public
*/
function open_rollback_entries($table_name, $table_columns, $date)
{
	$columns = $table_columns[$table_name]['columns'];
	$sql = 'INSERT INTO sq_rb_'.$table_name.' ('.implode(', ', $columns).
		', sq_eff_from, sq_eff_to)
		SELECT '.implode(',', $columns).',:date1 , NULL FROM sq_'.$table_name;
	$affected_rows = 0;

	try {
		$query = MatrixDAL::preparePdoQuery($sql);
		MatrixDAL::bindValueToPdo($query, 'date1', $date);
		$affected_rows = MatrixDAL::execPdoQuery($query);
	} catch (Exception $e) {
		throw new Exception('Unable to insert into rollback table '.$table_name.' due to the following error:'.$e->getMessage());
	}//end try catch

	return $affected_rows;

}//end open_rollback_entries()


/**
* Aligns all the minimum eff_from entries in the specified rollback table so
* they all start at a specified date
*
* @param string	$table_name	the tablename to update
* @param string	$date		the date to close to
*
* @return int
* @access public
*/
function align_rollback_entries($table_name, $date)
{
	$sql = 'UPDATE sq_rb_'.$table_name.'
			SET sq_eff_from = :date1
			WHERE sq_eff_from < :date2';
	$affected_rows = 0;

	try {
		$query = MatrixDAL::preparePdoQuery($sql);
		MatrixDAL::bindValueToPdo($query, 'date1', $date);
		MatrixDAL::bindValueToPdo($query, 'date2', $date);
		$affected_rows = MatrixDAL::execPdoQuery($query);
	} catch (Exception $e) {
		throw new Exception('Unable to update rollback table '.$table_name.' due to the following error:'.$e->getMessage());
	}//end try catch

	return $affected_rows;

}//end align_rollback_entries()


/**
* Deletes the rollback entries that started before the specified date
*
* @param string	$table_name	the tablename to update
* @param string	$date		the date to close to
*
* @return int
* @access public
*/
function delete_rollback_entries($table_name, $date)
{
	$sql = 'DELETE FROM sq_rb_'.$table_name.'
		WHERE sq_eff_to <= :date1';
	$affected_rows = 0;

	try {
		$query = MatrixDAL::preparePdoQuery($sql);
		MatrixDAL::bindValueToPdo($query, 'date1', $date);
		$affected_rows = MatrixDAL::execPdoQuery($query);
	} catch (Exception $e) {
		throw new Exception('Unable to delete rollback table '.$table_name.' due to the following error:'.$e->getMessage());
	}//end try catch

	return $affected_rows;

}//end delete_rollback_entries()


/**
* Deletes the redundant rollback entries
*
* @param string	$table_name	the tablename to update
*
* @return int
* @access public
*/
function delete_redundant_rollback_entries($table_name)
{	
	$relevant_tables = Array(
			'ast' => Array(
						'assetid',
					), 
			'ast_attr_val' => Array(
						'assetid',
						'attrid',
						'contextid',
					),
		);
	if (!in_array($table_name, array_keys($relevant_tables))) {
		return 0;
	}

	// For now, we only clean entries for Cron Manager
	$assets = Array(
			$GLOBALS['SQ_SYSTEM']->am->getSystemAsset('cron_manager'),
		);


	$affected_rows = 0;
	foreach($assets as $asset) {
		
		// Get all the entries for the given asset
		$sql = 'SELECT * FROM sq_rb_'.$table_name.'
				WHERE assetid=:assetid';
		try {
			$query = MatrixDAL::preparePdoQuery($sql);
			MatrixDAL::bindValueToPdo($query, 'assetid', $asset->id);
			$result = MatrixDAL::executePdoAssoc($query);
		} catch (Exception $e) {
			throw new Exception('Unable get asset info from table '.$table_name.' for assetid #'.$asset->id.' due to the following error: '.$e->getMessage());
		}//end try catch
		
		// Get the very first rollback entries in the table
		$org_rows = Array();
		foreach($result as $index => $row) {
			$key = '';
			foreach($relevant_tables[$table_name] as $col) {
				if (!isset($row[$col])) {
					trigger_error('Expected column "'.$col.'" not found in rollback table "'.$table_name.'"');
					return 0;
				}
				$key .= $row[$col].'.';
			}//end foreach
			if (!empty($key)) {
				if (!empty($org_rows[$key]['sq_eff_from'])) {
					if (strtotime($row['sq_eff_from']) < strtotime($org_rows[$key]['sq_eff_from'])) {
						$org_rows[$key]['sq_eff_from'] = $row['sq_eff_from'];
						$org_rows[$key]['index'] = $index;
					}
				} else {
					$org_rows[$key]['sq_eff_from'] = $row['sq_eff_from'];
					$org_rows[$key]['index'] = $index;
				}
			}
		}//end foreach

		if (!empty($org_rows)) {
			// Keep the very first entries and delete the rest
			foreach($org_rows as $org_row) {
				$rb_row = $result[$org_row['index']];
				
				// Open these rollback entires
				$where ='WHERE sq_eff_from=\''.$rb_row['sq_eff_from'].'\' AND '.
						(is_null($rb_row['sq_eff_to']) ? 'sq_eff_to IS NULL ' : 'sq_eff_to=\''.$rb_row['sq_eff_to'].'\' ');
				foreach($relevant_tables[$table_name] as $col) {
					$where .= ' AND '.$col.'=:'.$col;
				}//end foreach
				$sql = 'UPDATE sq_rb_'.$table_name.'
						SET sq_eff_to = NULL '.
						$where;
				try {
					$query = MatrixDAL::preparePdoQuery($sql);
					foreach($relevant_tables[$table_name] as $col) {
						MatrixDAL::bindValueToPdo($query, $col, $rb_row[$col]);
					}
					MatrixDAL::execPdoQuery($query);
				} catch (Exception $e) {
					throw new Exception('Unable to update rollback table '.$table_name.' due to the following error:'.$e->getMessage());
				}//end try catch
			
				unset($result[$org_row['index']]);
			}//end foreach
			
			// Get rid of rest
			foreach($result as $row) {

				$where ='WHERE sq_eff_from=\''.$row['sq_eff_from'].'\' AND '.
						(is_null($row['sq_eff_to']) ? 'sq_eff_to IS NULL ' : 'sq_eff_to=\''.$row['sq_eff_to'].'\' ');
				foreach($relevant_tables[$table_name] as $col) {
					$where .= ' AND '.$col.'=:'.$col;
				}//end foreach
				$sql = 'DELETE FROM sq_rb_'.$table_name.' '.
						$where;
				try {
					$query = MatrixDAL::preparePdoQuery($sql);
					foreach($relevant_tables[$table_name] as $col) {
						MatrixDAL::bindValueToPdo($query, $col, $row[$col]);
						$sql = str_replace(':'.$col, $row[$col], $sql);
					}
					$affected_rows += MatrixDAL::execPdoQuery($query);
				} catch (Exception $e) {
					throw new Exception('Unable to delete rollback table '.$table_name.' due to the following error:'.$e->getMessage());
				}
			}//end foreach
		}//end if
		
	}//end foreach

	return $affected_rows;

}//end delete_redundant_rollback_entries()


/**
* Returns the table names of the database tables that require rollback
*
* @return array
* @access public
*/
function get_rollback_table_names()
{
	$table_names = Array();

	$packages_installed = $GLOBALS['SQ_SYSTEM']->getInstalledPackages();

	if (empty($packages_installed)) return Array();

	foreach ($packages_installed as $package_array) {
		if ($package_array['code_name'] == '__core__') {
			$table_file = SQ_CORE_PACKAGE_PATH.'/tables.xml';
		} else {
			$table_file = SQ_PACKAGES_PATH.'/'.$package_array['code_name'].'/tables.xml';
		}

		if (!file_exists($table_file)) continue;

		try {
			$root = simplexml_load_string(file_get_contents($table_file), 'SimpleXMLElement', LIBXML_NOCDATA);
		} catch (Exception $e) {
			throw new Exception('Unable to parse table file : '.$table_file.' due to the following error: '.$e->getMessage());
		}//end try catch

		foreach ($root->children() as $child) {
			$first_child_name = $child->getName();
			break;
		}//end foreach
		if ($root->getName() != 'schema' || $first_child_name != 'tables') {
			trigger_error('Invalid table schema for file "'.$table_file.'"', E_USER_ERROR);
		}

		$table_root = $child;

		foreach ($table_root->children() as $table_child) {
			if ((string) $table_child->attributes()->require_rollback) {
				$table_name = (string) $table_child->attributes()->name;
				array_push($table_names, $table_name);
			}//end if
		}//end foreach
	}

	return $table_names;

}//end get_rollback_table_names()


/**
* Purge file versioning database entries and files before a certain date
*
* @param string	$date		the date to purge to
* @param int	$num_rows	the number of rows to operate on (used in chunked queries)
*
* @return void
* @access public
*/
function purge_file_versioning($date, $num_rows=0)
{
	$history_table = 'sq_file_vers_history';
	$file_table = 'sq_file_vers_file';

	// Get all the file versioning entries
	$sql = 'SELECT * FROM '.$history_table.' h JOIN '.$file_table.' f ON h.fileid = f.fileid WHERE to_date <= :date1';
	if ($num_rows > 0) {
		$sql = db_extras_modify_limit_clause($sql, MatrixDAL::getDbType(), $num_rows);
	}

	try {
		$query = MatrixDAL::preparePdoQuery($sql);
		MatrixDAL::bindValueToPdo($query, 'date1', $date);
		$result = MatrixDAL::executePdoAssoc($query);
	} catch (Exception $e) {
		throw new Exception('Unable to select from rollback table '.$table_name.' due to the following error:'.$e->getMessage());
	}//end try catch

	$count = 0;

	// Delete the files - if it isn't there then don't worry because it means the
	// DB entry shouldn't have been there in the first place
	foreach ($result as $row) {
		$ffv_file = SQ_SYSTEM_ROOT.'/data/file_repository/'.$row['path'].'/'.$row['filename'].',ffv'.$row['version'];
		if (file_exists($ffv_file)) {
			unlink($ffv_file);
		}
		$count++;
	}

	// If we're chunkified, then ensure that only the chunked entries are dealt with
	$chunk = Array();
	$chunk_query = '';
	reset($result);
	if (($num_rows > 0) && (!empty($result))) {
		foreach ($result as $row) {
			$chunk[] = '\''.((string) $row['fileid']).'\'';
		}

		$chunk_query = ' AND fileid IN ('.implode(', ',$chunk).')';
	}

	// Now delete the entries from the table
	$sql = 'DELETE FROM '.$history_table.' WHERE to_date <= :date1'.$chunk_query;
	$affected_rows = 0;

	try {
		$query = MatrixDAL::preparePdoQuery($sql);
		MatrixDAL::bindValueToPdo($query, 'date1', $date);
		$affected_rows = MatrixDAL::execPdoQuery($query);
	} catch (Exception $e) {
		throw new Exception('Unable to delete from rollback table '.$table_name.' due to the following error:'.$e->getMessage());
	}//end try catch

	return $affected_rows;

}//end purge_file_versioning()

/**
 * Check whether rollback entries have been found in any of the database tables.
 * If there is at least one row, this will return TRUE (meaning rollback has already
 * been enabled before and doesn't need enabling again).
 * If there are no rows found, it returns FALSE (rollback can safely be enabled).
 *
 * @param mixed $tables The database tables currently installed. Comes from
 *                      get_rollback_table_names.
 *
 * @return boolean Returns NULL if no tables were passed in.
 *                 Returns TRUE if any entries are found.
 *                 Returns FALSE if no entries are found in any table.
 */
function rollback_found(array $tables=array())
{
	if (empty($tables)) {
		return NULL;
	}

	$queries = array();
	foreach ($tables as $table) {
		$sql = "SELECT 1 FROM sq_rb_".$table." WHERE sq_eff_to IS NULL";
		$sql = db_extras_modify_limit_clause($sql, MatrixDAL::getDbType(), 1);
		$queries[] = "(".$sql.")";
	}
	$query  = "SELECT COUNT(*) FROM (".implode(" UNION ALL ", $queries).") rbcheck";
	$result = MatrixDAL::preparePdoQuery($query);
	$count  = MatrixDAL::executePdoOne($result);
	if ($count > 0) {
		return TRUE;
	}
	return FALSE;
}

?>
