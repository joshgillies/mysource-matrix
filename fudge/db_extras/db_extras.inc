<?php
/**
* +--------------------------------------------------------------------+
* | Squiz.net Open Source Licence                                      |
* +--------------------------------------------------------------------+
* | Copyright (c), 2003 Squiz Pty Ltd (ABN 77 084 670 600).            |
* +--------------------------------------------------------------------+
* | This source file may be used subject to, and only in accordance    |
* | with, the Squiz Open Source Licence Agreement found at             |
* | http://www.squiz.net/licence.                                      |
* | Make sure you have read and accept the terms of that licence,      |
* | including its limitations of liability and disclaimers, before     |
* | using this software in any way. Your use of this software is       |
* | deemed to constitute agreement to be bound by that licence. If you |
* | modify, adapt or enhance this software, you agree to assign your   |
* | intellectual property rights in the modification, adaptation and   |
* | enhancement to Squiz Pty Ltd for use and distribution under that   |
* | licence.                                                           |
* +--------------------------------------------------------------------+
*
* $Id: db_extras.inc,v 1.10 2003/11/26 00:51:18 gsherwood Exp $
* $Name: not supported by cvs2svn $
*/

/**
* DB Extras
*
* Purpose
*     To provide some workarounds for DBs that don't support some useful SQL functionality
*     like Sub-Queries
*
* @author  Blair Robertson <blair@squiz.net>
* @version $Version$ - 1.0
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

	switch($db->phptype) {
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

			}// end for

			$sql = str_replace($find, $replace, $sql);
			return $sql;

	}//end switch

}// end db_extras_subquery()


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

	switch($db->phptype) {
		// INSERT INTO ... SELECT FROM Compatable DB's
		case 'pgsql' :

			$result = $db->query($insert.' '.$select);
			if (DB::isError($result)) return $result;
			return TRUE;

			break;

		// Non Compatiable DB's
		default :

			$select_values = $db->getAll($select, Array(), DB_FETCHMODE_ORDERED);
			if (DB::isError($select_values)) return $select_values;

			for ($i = 0, $total = count($select_values); $i < $total; $i++) {
				$field_total = count($select_values[$i]);
				if (!$field_total) continue;
				$sql = $insert.' VALUES (';
				for ($j = 0; $j < $field_total; $j++) {
					$sql .= (($j) ? ',' : '').$db->quote($select_values[$i][$j]);
				}
				$sql .= ')';
				$result = $db->query($sql);
				if (DB::isError($result)) return $result;
			}

			return TRUE;

	}//end switch

}// end db_extras_insert_select()

?>
