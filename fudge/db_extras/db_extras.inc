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
* $Id: db_extras.inc,v 1.29.4.1 2013/08/30 03:43:01 ewang Exp $
*
*/

/**
* DB Extras
*
* Purpose
*     To provide some workarounds for DBs that don't support some useful SQL functionality
*     like Sub-Queries.
*
*     Since 3.18.0 RC1, these functions have been re-written to reflect the removal of
*     support for the PEAR DB module, and the addition of PHP 5's PDO and the Squiz Database
*     Abstraction Layer (DAL). As it is not possible to glean the database type from a PDO
*     DB object, the functions have been changed to use the database type instead.
*     Send DAL::getDbType() to the functions instead of $db.
*
* @author  Blair Robertson <blair@squiz.net>
* @version $Revision: 1.29.4.1 $
* @package Fudge
* @subpackage DB_Extras
*/


/**
* Takes a DB object, the primary SQL and the sub-queries,
* and generates an SQL statement that can be used in the main program
*
* NOTE: This is only guaranteed to work for simple sub-queries, where there
*       is no interaction between the current row in the primary sql statement
*       and the sub-queries and the sub-query only returns ONE column
*
* @param object DB	$db				reference to the db object
* @param string		$sql			The primary sql statement, marked up with the Sub-query replacements
* @param Array()	$subs			An array of the sub-queries to be performed
* @param mixed		$default_value	The default value to use when no values are found in the sub-query and we have a non subquery compatible DB
*
* @return mixed string or object DB_Error
* @access public
*/
function db_extras_subquery(&$db, $sql, $subs, $default_value=null)
{

	switch ($db->phptype) {
		// Sub-query compatible DB's
		case 'pgsql':
		case 'oci8':

			$find = Array();
			for ($i = 0; $i < count($subs); $i++) {
				$find[] = '~SQ'.$i.'~';
			}

			$sql = str_replace($find, $subs, $sql);
			return $sql;

			break;

		// Non-compatible DB's (...none now MySQL is not supported)
		default:

			$find    = Array();
			$replace = Array();
			for ($i = 0; $i < count($subs); $i++) {
				$find[$i] = '~SQ'.$i.'~';

				$result = MatrixDAL::executeSqlAssoc($subs[$i], 0);

				$replace[$i] = '';
				for ($j = 0, $total= count($result); $j < $total; $j++) {
					$replace[$i] .= (($replace[$i]) ? ',' : '').MatrixDAL::quote($result[$j]);
				}

				if ($replace[$i] == '') $replace[$i] = MatrixDAL::quote($default_value);

			}//end for

			$sql = str_replace($find, $replace, $sql);
			return $sql;

	}//end switch

}//end db_extras_subquery()


/**
* Takes a DB object, the insert statement and the select statement and
* and performs the insertions.
*
* Basically if you are in MySQL, it will do the select then foreach returned row perform an insert
* this is because you cannot have the table that you are inserting to in the from clause of the select
*
* @param object DB	$db				reference to the db object
* @param string		$insert			the insert part of the statement
* @param string		$select			An array of the sub-queries to be performed
*
* @return mixed TRUE or object DB_Error
* @access public
*/
function db_extras_insert_select(&$db, $insert, $select)
{

	switch ($db->phptype) {
		// INSERT INTO ... SELECT FROM Compatible DB's
		case 'pgsql' :
		case 'oci8'  :

			$result = MatrixDAL::executeSql($insert.' '.$select);
			if (DB::isError($result)) return $result;
			return TRUE;

			break;

		// Non Compatible DB's
		default :
			// ??? A bit difficult with
			$select_result = $db->query($select);
			if (DB::isError($select_result)) return $select_result;

			$row = Array();
			while (DB_OK === $select_result->fetchInto($row, DB_FETCHMODE_ORDERED)) {
				$field_total = count($row);
				if (!$field_total) continue;
				$sql = $insert.' VALUES (';
				for ($j = 0; $j < $field_total; $j++) {
					$sql .= (($j) ? ',' : '').MatrixDAL::quote($row[$j]);
				}
				$sql .= ')';

				$insert_result = $db->query($sql);
				if (DB::isError($insert_result)) {
					$select_result->free();
					return $insert_result;
				}
			}
			$select_result->free();

			return TRUE;

	}//end switch

}//end db_extras_insert_select()


/**
* Generates a sql fragment for performing bitand operations depending
* on the current database type.
*
* Example use:
* <pre>
*   $sql = 'SELECT link_type FROM sq_ast_lnk
*           WHERE '.db_extras_bitand($db_type, 'link_type', $link_types).' > 0';
* </pre>
*
* @param string	$db_type	the database type
* @param int	$val1		the first value
* @param int	$val2		the second value
*
* @return string the sql fragment
* @access public
*/
function db_extras_bitand($db_type, $val1, $val2)
{
	if ((int) $val1 != $val1) $val1 = MatrixDAL::quote($val1);
	if ((int) $val2 != $val2) $val2 = MatrixDAL::quote($val2);

	if ($db_type == 'oci') {
		$bit_op = 'BITAND('.$val1.', '.$val2.')';
	} else {
		$bit_op = '('.$val1.' & '.$val2.')';
	}

	return $bit_op;

}//end db_extras_bitand()


/**
* Returns an sql fragment to be used for inserting dates
* into the database.
*
* @param string		$db_type		the database type
* @param string		$iso8601_date	the date in iso8601 format
* @param boolean	$add_quotes		if TRUE, add single quotes around the date.
*									Omit if converting a bind variable or field
*
* @return string
* @access public
*/
function db_extras_todate($db_type, $iso8601_date, $add_quotes=TRUE)
{
	// TODO:
	if ($add_quotes === TRUE) {
		$iso8601_date = '\''.$iso8601_date.'\'';
	}

	if ($db_type == 'pgsql') {
		return 'TO_TIMESTAMP('.$iso8601_date.', \'YYYY-MM-DD HH24:MI:SS\')';
	} else if ($db_type == 'oci') {
		return 'TO_DATE('.$iso8601_date.',\'YYYY-MM-DD HH24:MI:SS\')';
	}

}//end db_extras_todate()


/**
* Prepares a date for insertion or update in a prepare statement
*
* @param string	$db_type	the database type
*
* @return string
* @access public
*/
function db_extras_prepare_todate($db_type)
{
	if ($db_type == 'pgsql') {
		return 'TO_TIMESTAMP(?, ?)';
	} else if ($db_type == 'oci8') {
		return 'TO_DATE(?, ?)';
	}

}//end db_extras_prepare_todate()


/**
* Returns the result of a function call to a database.
*
* @param string	$db_type	the database type
* @param string	$sql		the function call including the arguments to the function
* @param array	$bind_vars	an array of values to bind into the query - to be bound
*							by value only
*
* @return the result of the function call
* @access public
*/
function db_get_function_result($db_type, $sql, $bind_vars=Array())
{
	$db = MatrixDAL::getDb();
	$result = NULL;

	if ($db_type == 'pgsql') {
		$sql = 'SELECT '.$sql;
		$query	= MatrixDAL::preparePdoQuery($sql);
		foreach ($bind_vars as $key => $value) {
			MatrixDAL::bindValueToPdo($query, $key, $value);
		}
		$result = MatrixDAL::executePdoOne($query);

	} else if ($db_type == 'oci') {
		// the reason that we don't do 'SELECT function() from DUAL' here is that
		// if the function peforms any DML operations, we will get an error that
		// you cannot perform DML operations from within a select query.
		$sql = 'BEGIN :res := '.$sql.'; END;';

		try {
			$query = MatrixDAL::preparePdoQuery($sql);
			MatrixDAL::bindValueToPdoByRef($query, 'res', $result, PDO::PARAM_INPUT_OUTPUT, 4000);

			// Bind the other variables for input only
			foreach ($bind_vars as $key => $value) {
				MatrixDAL::bindValueToPdo($query, $key, $value);
			}

			MatrixDAL::execPdoQuery($query);
		} catch (Exception $e) {
			throw new Exception('Could not run function due to database error: '.$e->getMessage());
		}

	}
	return $result;

}//end db_get_function_result()


/**
* Converts an SQL query into a row-limited query.
* Appropriated from DALConverter classes.
*
* @param string	$sql		SQL query
* @param string	$db_type	the database type
* @param int	$limit		row limit
* @param int	$offset		Offset (row 0 = start from first row)
*
* @return the result of the function call
* @access public
*/
function db_extras_modify_limit_clause($sql, $db_type, $limit, $offset=0)
{
    	$limit = (int) $limit;
	$offset = (int) $offset;
	
	if ($db_type == 'oci') {
		if ($offset > 0) {
			$inner_sql = 'SELECT original_query.*, rownum rnum FROM (' . $sql . ') original_query WHERE rownum <= ' . ($offset + $limit);
			$sql = 'SELECT * FROM (' . $inner_sql . ') WHERE rnum > ' . $offset;
		} else {
			$sql  = 'SELECT * FROM ('.$sql.')';
			$sql .= ' WHERE ROWNUM BETWEEN 0 AND '.$limit;
		}
	} else { // psql, db2, mysql
		$sql  = $sql.' LIMIT '.$limit.' OFFSET '.$offset;
	}

	return $sql;

}//end db_modify_limit_clause()


?>
