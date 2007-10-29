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
* $Id: MatrixDAL.inc,v 1.1 2007/10/29 05:54:11 lwright Exp $
*
*/


require_once SQ_LIB_PATH.'/DAL/DAL.inc';

/**
* MatrixDAL
*
* Purpose
*    Extends and overrides the Database Abstraction Layer (DAL) class, in
*    particular removes dependencies to MySource 4.x objects and concepts, in
*    order to make advanced capabilities of the DAL useful to MySource Matrix
*    (MySource 3.x).
*
*    Major differences provided by the Matrix DAL extension:
*    - "Systems" as used in MySource 4.x are not a MySource Matrix concept -
*      their closest analogies in Matrix are packages and assets. When defining
*      "systems" from Matrix DAL's perspective, we will define three levels:
*      the core, package level, and asset level.
*
*    - The DAL, by default, uses an "event basket", defined in the MySource 4.x
*      core, which holds data for a certain channel. This is then passed to
*      database queries. The Matrix DAL will somehow re-create this "event
*      basket", or otherwise allow something similar to plain variable binding.
*
* @since   MySource 3.18.0
* @author  Squiz Pty Ltd <matrix@squiz.net>
* @version $Revision: 1.1 $
* @package MySource_Matrix
*/
class MatrixDAL extends DAL
{


	/**
	* Constructor
	*
	*/
	function __construct()
	{
		parent::__construct();

	}//end constructor


    /**
     * Returns the PDO query object for the given system and queryid.
	 *
	 *
     *
     * @param string  $systemid   System name that the query belongs to.
     * @param string  $queryid    ID of the query.
	 * @param array   $params     Parameters for the query.
     * @param boolean $bindValues If TRUE, will add the query to a PDOStatement
     *                            object and bind its' variables. Otherwise, it
     *                            will return an array representation of it.
     *
     * @since  4.0.0
     * @return object
     * @throws DALException If failed to load the query.
     */
    public static function getQuery($systemid, $queryid, $params=Array(), $bindValues=TRUE)
    {
        // Work out the class and method name to call.
        $class    = $systemid.'Queries';
        $function = 'prepare'.$queryid.'Query';

        self::requireQueryClass($systemid);

        // Should we include the class here?
        if (method_exists($class, $function) === FALSE) {
            // Query not found. Throw DALException exception?
            throw new DALException('Could not load query '.$queryid.' in system '.$systemid);
        } else {
            return call_user_func(array($class, $function), $params, $bindValues);
        }

    }//end getQuery()




    /**
     * Returns all rows grouped by first column.
     *
     * @param string $systemid System's ID.
     * @param string $queryid  ID of the SQL query.
     *
     * @since  4.0.0
     * @return array
     */
    public static function executeGrouped($systemid, $queryid, $params=NULL)
    {
        $query = self::getQuery($systemid, $queryid, $params);
        return self::getGrouped($query);

    }//end executeGrouped()


    /**
     * Returns all rows as an associative array.
     *
     * This function will call getQuery() and getAssoc().
     *
     * @param string  $systemid System's ID.
     * @param string  $queryid  ID of the SQL query.
     * @param integer $col      Column to retrieve.
     *
     * @since  4.0.0
     * @return array
     */
    public static function executeAssoc($systemid, $queryid, $col=NULL, $params=NULL)
    {
		if ((func_num_args() === 3) && is_array($col)) {
			$params = $col;
			$col    = NULL;
		}
        $query = self::getQuery($systemid, $queryid, $params);
        $res   = self::getAssoc($query, $col);

        return $res;

    }//end executeAssoc()


    /**
     * Returns all rows in an array.
     *
     * This function will call getQuery() and getAll().
     *
     * @param string $systemid System's ID.
     * @param string $queryid  ID of the SQL query.
     *
     * @since  4.0.0
     * @return array
     */
    public static function executeAll($systemid, $queryid, $params=NULL)
    {
        $query = self::getQuery($systemid, $queryid, $params);
        return self::getAll($query);

    }//end executeAll()


    /**
     * Returns a single value.
     *
     * This function will call getQuery() and getOne().
     *
     * @param string $systemid System's ID.
     * @param string $queryid  ID of the SQL query.
     *
     * @since  4.0.0
     * @return array
     */
    public static function executeOne($systemid, $queryid, $params=NULL)
    {
        $query  = self::getQuery($systemid, $queryid, $params);
        $result = self::getOne($query);

        // Oracle returns resourceid for LOB fields.
        // If result is a resource then retreive its contents.
        if (is_resource($result) === TRUE) {
            $result = stream_get_contents($result);
        }

        return $result;

    }//end executeOne()


    /**
     * Executes a query and returns the number of rows affected.
     *
     * Note: $systemid can be a PDOStatement object.
     * If $systemid is PDOStatement then $queryid (2nd argument) can be NULL.
     *
     * @param string|object $systemid System's ID.
     * @param string        $queryid  ID of the SQL query.
     *
     * @since  4.0.0
     * @return integer
     * @throws DALException If no query provided or query execution failed.
     */
    public static function executeQuery($systemid, $queryid=NULL, $params=NULL)
    {
        $query = NULL;
        if (($systemid instanceof Query) === TRUE) {
            self::_startQueryTimer($systemid->getQueryid());
            $query = $systemid;
            $query = $query->prepare();
        } else if ($queryid === NULL) {
            throw new DALException('No queryid provided for executeQuery.');
        } else {
            self::_startQueryTimer($queryid);
            $query = self::getQuery($systemid, $queryid, $params);
            $query = $query->prepare();
        }

        try {
            self::execPdoQuery($query);
        } catch (PDOException $e) {
            throw new DALException($e->getMessage());
        }

        $rows = (int)$query->rowCount();

        // DB2 returns -1 rows if no rows were affected.
        if ($rows === -1) {
            $rows = 0;
        }

        self::_endQueryTimer();

        return $rows;

    }//end executeQuery()


    /**
     * Retrieves the PDO data type for a column.
     *
     * Used when printing bindings for queries.
     *
     * @param string $systemId System the column is from.
     * @param string $table    Table the column is in.
     * @param string $column   The column name.
     * @param string $dataType The datatype to bind this value with. Will
     *                         override any other value.
     *
     * @since  4.0.0
     * @return string
     */
    protected static function _getPdoDataType($systemId, $table, $column, $dataType=NULL)
    {
        if ($dataType === NULL) {
			return 'PDO::PARAM_STR';
        //    $dataType = self::getTableColumnTypes($systemId, $table, $column);
        }

        switch (strtoupper($dataType)) {
            case 'NUMERIC':
            case 'DECIMAL':
            case 'INTEGER':
            case 'SMALLINT':
            case 'REAL':
            case 'DOUBLE PRECISION':
            case 'FLOAT':
                $pdoType = 'PDO::PARAM_INT';
            break;
            case 'BOOLEAN':
                $pdoType = 'PDO::PARAM_BOOL';
            break;
            default:
                $pdoType = 'PDO::PARAM_STR';
        }

        return $pdoType;

    }//end _getPdoDataType()


}
?>
