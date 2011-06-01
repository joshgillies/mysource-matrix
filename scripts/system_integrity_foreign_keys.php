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
* $Id: system_integrity_foreign_keys.php,v 1.3 2008/02/18 05:28:41 lwright Exp $
*
*/

/**
* System_Integrity_Foreign_Keys Script
*
* Checks the integrity of the database foreign keys
*
* @author Ben Caldwell <bcaldwell@squiz.net>
* @version $Revision: 1.3 $
* @package MySource_Matrix
*/
error_reporting(E_ALL);
if ((php_sapi_name() != 'cli')) {
	trigger_error("You can only run this script from the command line\n", E_USER_ERROR);
}

$SYSTEM_ROOT = (isset($_SERVER['argv'][1])) ? $_SERVER['argv'][1] : '';
if (empty($SYSTEM_ROOT) || !is_dir($SYSTEM_ROOT)) {
	echo "ERROR: You need to supply the path to the System Root as the first argument\n";
	exit();
}

$DELETING_ASSET_TYPE = (isset($_SERVER['argv'][2])) ? $_SERVER['argv'][2] : '';
if (empty($SYSTEM_ROOT) || !is_dir($SYSTEM_ROOT)) {
	echo "ERROR: You need to supply an asset type code as the second argument\n";
}


require_once $SYSTEM_ROOT.'/core/include/init.inc';


// ask for the root password for the system
echo 'Enter the root password for "'.SQ_CONF_SYSTEM_NAME.'": ';
$root_password = rtrim(fgets(STDIN, 4094));

// check that the correct root password was entered
$root_user =& $GLOBALS['SQ_SYSTEM']->am->getSystemAsset('root_user');
if (!$root_user->comparePassword($root_password)) {
	echo "ERROR: The root password entered was incorrect\n";
	exit();
}

// log in as root
if (!$GLOBALS['SQ_SYSTEM']->setCurrentUser($root_user)) {
	trigger_error("Failed loggin in as root user\n", E_USER_ERROR);
	exit();
}

define('SQ_FOREIGN_KEY_INTEGRITY_CHECK_OUTPUT_WIDTH', 72);

class Foreign_Key_Integrity_Check
{


	/**
	* All of the foreign keys we know about
	*
	* @var Array
	* @access private
	*/
	var $fks = Array(
				'assetid'	=> Array(
								'table'	=> 'sq_ast',
								'field'	=> 'assetid',
								'match'	=> Array(
											'assetid',
											'created_userid',
											'updated_userid',
											'published_userid',
											'status_changed_userid',
											'majorid',
											'minorid',
											'schemaid',
											'userid',
											'roleid',
										   ),
							   ),
				'treeid'	=> Array(
								'table'	=> 'sq_ast_lnk_tree',
								'field'	=> 'treeid',
								'match'	=> Array(
											'treeid',
										   ),
							   ),
				'attrid'	=> Array(
								'table'	=> 'sq_ast_attr',
								'field'	=> 'attrid',
								'match'	=> Array(
											'attrid',
											'owning_attrid',
										   ),
							   ),
				'linkid'	=> Array(
								'table'	=> 'sq_ast_lnk',
								'field'	=> 'linkid',
								'match'	=> Array(
											'linkid',
										   ),

							   ),
				'urlid'		=> Array(
								'table'	=> 'sq_ast_url',
								'field'	=> 'urlid',
								'match'	=> Array(
											'urlid',
											'root_urlid',
										   ),
							   ),
				'type_code'	=> Array(
								'table'	=> 'sq_ast_typ',
								'match'	=> Array(
											'type_code',
											'inhd_type_code',
										   ),
							   ),
			   );


	/**
	* All of the tables in the database
	*
	* @var Array
	* @access private
	*/
	var $tables = Array();


	/**
	* Constructor
	*
	* @return void
	* @access public
	*/
	function Foreign_Key_Integrity_Check()
	{
		if (!$this->load()) {
			// todo: trigger_error
		}

	}//end Foreign_Key_Integrity_Check()


	/**
	* Get all of the tables from the database
	*
	* @return Array
	* @access public
	*/
	function tables()
	{
		return $this->tables;

	}//end tables()


	/**
	* Given the name of a table, return all it's columns
	*
	* @param string	$table	the name of the table to get the columns of
	*
	* @return Array
	* @access public
	*/
	function columns($table)
	{
		// if we don't know about this table, die
		if (!in_array($table, $this->tables())) {
			return Array();
		}

		$db =& $GLOBALS['SQ_SYSTEM']->db;
		$table_info = $db->tableInfo($table);

		$columns = Array();
		foreach ($table_info as $column_info) {
			$columns[] = $column_info['name'];
		}
		return $columns;

	}//end columns()


	/**
	* Returns the foreign keys for a given table
	*
	* @param string	$table	the name of the table to get fks for
	*
	* @return Array
	* @access public
	*/
	function fks($table)
	{
		$fks = Array();
		$columns = $this->columns($table);

		foreach ($this->fks as $fk => $fk_info) {
			// get all the names that this foreign key goes by
			$fk_columns = $fk_info['match'];

			// these are the defined fks we found in the specified table
			$match_fks = array_intersect($fk_columns, $columns);
			if (empty($match_fks)) continue;

			// build the information into expected format
			foreach ($match_fks as $fk) {
				$fks[$fk] = $fk_info;
			}
		}
		return $fks;

	}//end fks()


	/**
	* Normal stuff we need to get ready before we do anything else
	*
	* @return boolean
	* @access public
	*/
	function load()
	{
		$db =& $GLOBALS['SQ_SYSTEM']->db;

		// load our tables from the database
		$this->tables = $db->getTables();
		if (empty($this->tables)) {
			return FALSE;
		}
		return TRUE;

	}//end load()


	/**
	* Performs the foreign key integrity check on the database
	*
	* @return void
	* @access public
	*/
	function checkDatabase()
	{
		// we're going to check each table in the database
		$tables = $this->tables();

		foreach ($tables as $table) {
			if (substr($table, 0, 6) != 'sq_ast') {
				continue;
			}
			$this->checkTable($table);
		}

	}//end checkDatabase()


	/**
	* Peforms a foreign key integrity check on the specified table
	*
	* @param string	$table	the name of the table to perform the check on
	*
	* @return boolean
	* @access public
	*/
	function checkTable($table)
	{
		$db =& $GLOBALS['SQ_SYSTEM']->db;

		// we're going to check these keys
		$fks = $this->fks($table);
		if (empty($fks)) {
			return TRUE;
		}
		$this->printTable($table);

		foreach ($fks as $fk_field => $fk_info) {
			// query db for invalid foreign keys
			$sql = "SELECT $fk_field
					FROM $table
					WHERE
						$fk_field > 0
					AND
						$fk_field NOT IN (SELECT ".$fk_field." FROM ".$fk_info['table'].")";
			$assetids = MatrixDAL::executeSqlAssoc($sql, 0);

			$broken_count = count($assetids);
			if (!is_array($assetids)) {
				$broken_count = 0;
			}

			$this->printColumn($fk_info['table'], $fk_field, $broken_count);

			if (is_array($assetids) && !empty($assetids)) {
				$this->printResult($assetids);
			}
		}//end foreach
		echo "\n";

	}//end checkTable()


	/**
	* Print information on the table that is being processed/checked
	*
	* @param string	$table	the name of the table being processed/checked
	*
	* @return void
	* @access private
	*/
	function printTable($table)
	{
		echo "> checking table $table...\n\n";

	}//end printTable()


	/**
	* Print information on the column (read foreign key) currently being checked
	*
	* @param string	$table	the current table being checked
	* @param string	$column	the current column being checked
	* @param int	$count	the number of bad foreign keys found for table/column
	*
	* @return void
	* @access private
	*/
	function printColumn($table, $column, $count)
	{
		$column_text = "$table.$column";
		$count_text = "$count found";

		echo $column_text;
		echo str_repeat(' ', (SQ_FOREIGN_KEY_INTEGRITY_CHECK_OUTPUT_WIDTH - strlen($column_text) - strlen($count_text)));
		echo $count_text."\n";

	}//end printColumn()


	/**
	* Print information on which values for the foreign key are broken
	*
	* @param array	$assetids	an array of values for the incorrect foreign keys
	*
	* @return void
	* @access private
	*/
	function printResult($assetids)
	{
		echo "\t".'\''.implode('\', \'', $assetids).'\''."\n";

	}//end printResult


}//end class


$foreign_key_checker =& new Foreign_Key_Integrity_Check();
$foreign_key_checker->checkDatabase();

?>
