<?php
/**
* DB Extras
*
* Purpose
*     To provide some workarounds for DBs that don't support some useful SQL functionality
*     like Sub-queries
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
* @return string
* @access public
*/
function db_extras_subquery(&$db, $sql, $subs, $default_value=null) {

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

		// Non-Sub Query Compatiable DB's
		default :

			$find    = Array();
			$replace = Array();
			for ($i = 0; $i < count($subs); $i++) {
				$find[$i] = '~SQ'.$i.'~';

				$result = $db->getCol($subs[$i]);
				if (DB::isError($result)) return $result;

				$replace[$i] = '';
				for ($j = 0; $j < count($result); $j++) {
					$replace[$i] .= (($replace[$i]) ? ',' : '').$db->quote($result[$j]);
				}

				if ($replace[$i] == '') $replace[$i] = $db->quote($default_value);

			}// end for

			$sql = str_replace($find, $replace, $sql);
			return $sql;

	}//end switch

}// end db_extras_subquery

?>