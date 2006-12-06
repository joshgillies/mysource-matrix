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
* $Id: db_extras.inc,v 1.18 2006/12/06 05:15:10 bcaldwell Exp $
*
*/

/**
* DB Extras
*
* Purpose
*     To provide some workarounds for DBs that don't support some useful SQL functionality
*     like Sub-Queries
*
* @author  Blair Robertson <blair@squiz.net>
* @version $Revision: 1.18 $
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
		// Sub Query Compatable DB's
		case 'pgsql' :

			$find = Array();
			for ($i = 0; $i < count($subs); $i++) {
				$find[] = '~SQ'.$i.'~';
			}

			$sql = str_replace($find, $subs, $sql);
			return $sql;

			break;

		// Non Compatiable DB's
		default :

			$find    = Array();
			$replace = Array();
			for ($i = 0; $i < count($subs); $i++) {
				$find[$i] = '~SQ'.$i.'~';

				$result = $db->getCol($subs[$i]);
				if (DB::isError($result)) return $result;

				$replace[$i] = '';
				for ($j = 0, $total= count($result); $j < $total; $j++) {
					$replace[$i] .= (($replace[$i]) ? ',' : '').$db->quote($result[$j]);
				}

				if ($replace[$i] == '') $replace[$i] = $db->quote($default_value);

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

			$result = $db->query($insert.' '.$select);
			if (DB::isError($result)) return $result;
			return TRUE;

			break;

		// Non Compatible DB's
		default :

			$select_result = $db->query($select);
			if (DB::isError($select_result)) return $select_result;

			$row = Array();
			while (DB_OK === $select_result->fetchInto($row, DB_FETCHMODE_ORDERED)) {
				$field_total = count($row);
				if (!$field_total) continue;
				$sql = $insert.' VALUES (';
				for ($j = 0; $j < $field_total; $j++) {
					$sql .= (($j) ? ',' : '').$db->quote($row[$j]);
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
*           WHERE '.db_extras_bitand('link_type, '.$link_types).' > 0';
* </pre>
*
* @param &object DB	$db		the db object
* @param int		$val1	the first value
* @param int		$val2	the second value
*
* @return string the sql fragment
* @access public
*/
function db_extras_bitand(&$db, $val1, $val2)
{
	if ((int) $val1 != $val1) $val1 = $db->quoteSmart($val1);
	if ((int) $val2 != $val2) $val2 = $db->quoteSmart($val2);

	if ($db->phptype == 'oci8') {
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
* @param &object DB	$db				the db object
* @param string		$iso8601_date	the date in iso8601 format
*
* @return string
* @access public
*/
function db_extras_todate(&$db, $iso8601_date)
{
	if ($db->phptype == 'pgsql') {
		return 'TO_TIMESTAMP('.$db->quote($iso8601_date).','.$db->quote('YYYY-MM-DD HH24:MI:SS').')';
	} else if ($db->phptype == 'oci8') {
		return 'TO_DATE('.$db->quote($iso8601_date).','.$db->quote('YYYY-MM-DD HH24:MI:SS').')';
	}

}//end db_extras_todate()


/**
* Prepares a date for insertion or update in a prepare statement
*
* @param &object DB	$db	the db object
*
* @return string
* @access public
*/
function db_extras_prepare_todate(&$db)
{
	if ($db->phptype == 'pgsql') {
		return 'TO_TIMESTAMP(?, ?)';
	} else if ($db->phptype == 'oci8') {
		return 'TO_DATE(?, ?)';
	}

}//end db_extras_prepare_todate()


/**
* Returns the result of a function call to a database.
*
* @param &object DB	$db		the PEAR:DB object
* @param string 	$sql	the function call including the arguments to the function
*
* @return the result of the function call
* @access public
*/
function db_get_function_result(&$db, $sql)
{
	$result = null;

	if ($db->phptype == 'pgsql') {
		$sql = 'SELECT '.$sql;
		$result = $db->getOne($sql);
		assert_valid_db_result($result);

	} else if ($db->phptype == 'oci8') {
		// the reason that we don't do 'SELECT function() from DUAL' here is that
		// if the function peforms any DML operations, we will get an error that
		// you cannot perform DML operations from within a select query.
		$sql = 'BEGIN :res := '.$sql.'; END;';

		if (!$stmt = ociparse($db->connection, $sql)) {
			$msg = ocierror($db->connection);
			trigger_exception($msg['message'], false, true);
		}

		if (!ocibindbyname($stmt, ':res', $result, 2000)) {
			$msg = ocierror($db->connection);
			trigger_exception($msg['message'], false, true);
		}

		if (!ociexecute($stmt)) {
			$msg = ocierror($db->connection);
			trigger_exception($msg['message'], false, true);
		}
	}
	return $result;

}//end db_get_function_result()


?>
