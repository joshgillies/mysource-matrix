<?php
/**
 * DAL. Database Abstraction Layer.
 *
 * @version    4.0.0
 * @package    MySource4
 * @subpackage DAL
 * @author     Squiz Pty Ltd <mysource4@squiz.net>
 * @copyright  2006-2007 Squiz Pty Ltd (ABN 77 084 670 600)
 * @license    http://matrix.squiz.net/licence Squiz.Net Open Source Licence
 */

// Matrix = Channels system not required
//require_once dirname(dirname(__FILE__)).'/Channels/Channels.inc';
require_once dirname(__FILE__).'/Exceptions/DALException.inc';
// We're in a different place here; we can't assume that we're starting from
// the root
//require_once 'DAL/Query.inc';
require_once dirname(__FILE__).'/Query.inc';

/**
 * DAL.
 *
 * DAL (Database Abstraction Layer) class handles parsing of XML SQL queries.
 * It converts XML based SQL queries to generic PHP arrays so that they can be
 * converted to actual SQL queries by the DALConverter class.
 *
 * DAL class can also validates schemas and quries.
 *
 * DAL class also handles database connection and access.
 *
 * @since 4.0.0
 */
class DAL
{

    /**
     * PDO connection pool.
     *
     * Each array element will be keyed by a connection ID, and contain the
     * following elements:
     * - PDO     dbh                 The PDO connection object.
     * - boolean transactionSupport  TRUE if this connection supports transactions.
     * - boolean inTransaction       TRUE if a transaction is active for this connection.
     * - string  dsn                 Data source connection string
     * - string  dbType              Database type
     * - string  userName            User name for connection
     * - string  password            Password for connection
     * - string  dbName              Database name (filled when getDbName() called).
     *
     * @var   array $_dbhPool array of connection information
     * @since 4.0.0
     */
    private static $_dbhPool  = NULL;


    /**
     * PDO connection ID stack.
     *
     * This will be modified using the changeDb() and restoreDb() functions.
     * The "top" of the stack is the current connection and will be returned
     * by the getDb() function.
     *
     * @var   array $_dbStack Array of connection IDs
     * @since 4.0.0
     */
    private static $_dbStack  = Array();


    /**
     * Transaction handler.
     *
     * If a transaction is already started then another transaction will not
     * be started. Commit and rollBack will set this var to FALSE.
     *
     * @var   boolean $_inTransaction Transaction handler.
     * @since 4.0.0
     */
    private static $_inTransaction = FALSE;

    /**
     * Valid database names.
     *
     * A database type must be specified in the DALConfig file.
     *
     * @var   array $_dbNames Valid database types and their names.
     * @since 4.0.0
     */
    private static $_dbNames = array(
                                'db2'   => 'DB2',
                                'dblib' => 'SQLServer',
                                'mysql' => 'MySQL',
                                'oci'   => 'Oracle',
                                'pgsql' => 'PostgreSQL',
                               );

    /**
     * Name of the current database.
     *
     * @var   string $_currentDbName
     * @since 4.0.0
     */
    private static $_currentDbName = NULL;

    /**
     * If set to true DB transactions will be enabled.
     *
     * If false, calling transaction methods will have no affect.
     *
     * @var   boolean $_transactionEnabled
     * @since 4.0.0
     */
    private static $_transactionEnabled = FALSE;

    /**
     * List of querys that were executed and their timings.
     *
     * @var   array $_queryLog
     * @since 4.0.0
     */
    private static $_queryLog = array();

    /**
     * Log queries.
     *
     * @var   boolean $_logQueries
     * @since 4.0.0
     */
    private static $_logQueries = FALSE;

    /**
     * Current query.
     *
     * @var   array $_currentQueryLog
     * @since 4.0.0
     */
    private static $_currentQueryLog = NULL;


    /**
     * DAL constructor.
     *
     * Private to avoid instantiating the object.
     * All DAL methods should be called statically.
     *
     * @since 4.0.0
     */
    private function __construct()
    {

    }//end __construct()


    /**
     * Returns the PDO query object for the given system and queryid.
     *
     * @param string  $systemid   System name that the query belongs to.
     * @param string  $queryid    ID of the query.
     * @param boolean $bindValues If TRUE, will add the query to a PDOStatement
     *                            object and bind its' variables. Otherwise, it
     *                            will return an array representation of it.
     *
     * @since  4.0.0
     * @return object
     * @throws DALException If failed to load the query.
     */
    public static function getQuery($systemid, $queryid, $bindValues=TRUE)
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
            // Get the event data for the current channel.
			$eventData = Channels::getCurrentEventBasketData();
            return call_user_func(array($class, $function), $eventData, $bindValues);
        }

    }//end getQuery()


    /**
     * Returns all rows as an associative array.
     *
     * @param Query   $dalQuery SQL Query to execute.
     * @param integer $col      Column to retrieve.
     *
     * @since  4.0.0
     * @return array
     */
    public static function getAssoc(Query $dalQuery, $col=NULL)
    {
        self::_startQueryTimer($dalQuery->getQueryid());

        $query  = $dalQuery->prepare();
        $result = self::executePdoAssoc($query);

        self::_endQueryTimer();

        return $result;

    }//end getAssoc()


    /**
     * Returns all rows in an array.
     *
     * @param Query $dalQuery SQL Query to execute.
     *
     * @since  4.0.0
     * @return array
     */
    public static function getAll(Query $dalQuery)
    {
        self::_startQueryTimer($dalQuery->getQueryid());

        $query  = $dalQuery->prepare();
        $result = self::executePdoAll($query);

        self::_endQueryTimer();

        return $result;

    }//end getAll()


    /**
     * Returns a single value.
     *
     * @param Query $dalQuery SQL Query to execute.
     *
     * @since  4.0.0
     * @return array
     */
    public static function getOne(Query $dalQuery)
    {
        self::_startQueryTimer($dalQuery->getQueryid());

        $query = $dalQuery->prepare();
        $result = self::executePdoOne($query);

        self::_endQueryTimer();

        return $result;

    }//end getOne()


    /**
     * Returns an array of data about the SQL query.
     *
     * This is used if the query is not to be executed yet, and rather a
     * representation of the query's SQL, along with its' bindings and their
     * values.
     *
     * @param string $systemid System name that the query belongs to.
     * @param string $queryid  ID of the query.
     * @param string $method   Method used to retrieve the query(getAssoc etc).
     *
     * @since  4.0.0
     * @return array
     */
    public static function getDALQuery($systemid, $queryid, $method)
    {
        $queryArray = self::getQuery($systemid, $queryid, FALSE);
        // Add the method data to the query array, as this will allow
        // methods that this is passed to to execute it.
        $queryArray->setExecMethod($method);
        return $queryArray;

    }//end getDALQuery()


    /**
     * Returns all rows grouped by first column.
     *
     * @param Query $dalQuery SQL Query to execute.
     *
     * @since  4.0.0
     * @return array
     * @throws DALException If query execution fails.
     */
    public static function getGrouped(Query $dalQuery)
    {
        self::_startQueryTimer($dalQuery->getQueryid());

        $query  = $dalQuery->prepare();
        $result = self::executePdoGrouped($query);

        self::_endQueryTimer();

        return $result;

    }//end getGrouped()


    /**
     * Returns all rows grouped by first column.
     *
     * @param string $systemid System's ID.
     * @param string $queryid  ID of the SQL query.
     *
     * @since  4.0.0
     * @return array
     */
    public static function executeGrouped($systemid, $queryid)
    {
        $query = self::getQuery($systemid, $queryid);
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
    public static function executeAssoc($systemid, $queryid, $col=NULL)
    {
        $query = self::getQuery($systemid, $queryid);
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
    public static function executeAll($systemid, $queryid)
    {
        $query = self::getQuery($systemid, $queryid);
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
    public static function executeOne($systemid, $queryid)
    {
        $query  = self::getQuery($systemid, $queryid);
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
    public static function executeQuery($systemid, $queryid=NULL)
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
            $query = self::getQuery($systemid, $queryid);
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
     * Alias for executeQuery.
     *
     * @param string|object $systemid System's ID.
     * @param string        $queryid  ID of the SQL query.
     *
     * @since  4.0.0
     * @return integer
     */
    public static function getExecuteQuery($systemid, $queryid=NULL)
    {
        return self::executeQuery($systemid, $queryid);

    }//end getExecuteQuery()


    /**
     * Alias for executeAll.
     *
     * @param string $systemid System's ID.
     * @param string $queryid  ID of the SQL query.
     *
     * @since  4.0.0
     * @return integer
     */
    public static function getExecuteAll($systemid, $queryid)
    {
        return self::executeAll($systemid, $queryid);

    }//end getExecuteAll()


    /**
     * Executes a DALQuery.
     *
     * @param Query  $query      The Query to execute.
     * @param string $execMethod Optional execute method i.e. Assoc, Grouped.
     *
     * @since  4.0.0
     * @return array
     */
    public static function executeDALQuery(Query $query, $execMethod=NULL)
    {

        if ($execMethod === NULL) {
            $execMethod = $query->getExecMethod();
        }

        $methodArray = array(
                        'self',
                        'get'.ucfirst($execMethod),
                       );
        $result = call_user_func($methodArray, $query);
        return $result;

    }//end executeDALQuery()


    /**
     * Executes the given PDO statment.
     *
     * @param PDOStatement $query PDOStatement object.
     *
     * @since  4.0.0
     * @return void
     */
    public static function execPdoQuery(PDOStatement $query)
    {
        $query->execute();

    }//end execPdoQuery()


    /**
     * Executes the given PDO statement and returns all rows in an
     * associative array.
     *
     * If optional parameter $col is specified, this does NOT return an
     * associative array, but rather values from a single column.
     *
     * @param PDOStatement $query  PDOStatement object to execute.
     * @param integer      $col    Column to retrieve (zero-indexed).
     *
     * @since  4.0.0
     * @return array
     */
    public static function executePdoAssoc(PDOStatement $query, $col=NULL)
    {
        try {
            self::execPdoQuery($query);
        } catch (PDOException $e) {
            throw new DALException($e->getMessage());
        }

        $result = NULL;
        if ($col !== NULL) {
            $result = $query->fetchAll(PDO::FETCH_COLUMN, $col);
        } else {
            $result = $query->fetchAll(PDO::FETCH_ASSOC);
        }

        return $result;

    }//end executePdoAssoc()


    /**
     * Executes the given PDO statement and returns all rows in an array.
     *
     * @param PDOStatement $query  PDOStatement object to execute.
     *
     * @since  4.0.0
     * @return array
     */
    public static function executePdoAll(PDOStatement $query)
    {
        try {
            self::execPdoQuery($query);
        } catch (PDOException $e) {
            throw $e;
        }

        $result = $query->fetchAll();

        return $result;

    }//end executePdoAll()


    /**
     * Executes the given PDO statement and returns all rows grouped by
     * the first column.
     *
     * @param PDOStatement $query  PDOStatement object to execute.
     *
     * @since  4.0.0
     * @return array
     */
    public static function executePdoGrouped(PDOStatement $query)
    {
        $result = array();
        try {
            self::execPdoQuery($query);
        } catch (PDOException $e) {
            throw new DALException($e->getMessage());
        }

        while ($row = $query->fetch(PDO::FETCH_NUM)) {
            if (isset($result[$row[0]]) === FALSE) {
                $result[$row[0]] = array();
            }

            $current = array();
            $i       = 1;
            while (isset($row[$i]) === TRUE) {
                $current[] = $row[$i];
                $i++;
            }

            $result[$row[0]][] = $current;
        }

        return $result;

    }//end executePdoGrouped()


    /**
     * Executes the given PDO statement and returns all rows grouped by
     * the first column, with subsequent fields keyed by their field name.
     *
     * @param PDOStatement $query  PDOStatement object to execute.
     *
     * @since  4.0.0
     * @return array
     */
    public static function executePdoGroupedAssoc(PDOStatement $query)
    {
        $result = array();
        try {
            self::execPdoQuery($query);
        } catch (PDOException $e) {
            throw new DALException($e->getMessage());
        }

        while ($row = $query->fetch(PDO::FETCH_ASSOC)) {
            // If we don't know the first field yet, find out what it is
			// because it should be the same for each row
            if (!isset($first_field)) {
                reset($row);
                $first_field = key($row);
            }
			$first_value = $row[$first_field];

            if (isset($result[$first_value]) === FALSE) {
                $result[$first_value] = array();
            }

            $current = $row;
            unset($current[$first_field]);
            $result[$first_value][] = $current;
        }

        return $result;

    }//end executePdoGroupedAssoc()


    /**
     * Executes the given PDO statement and returns a single value
     *
     * @param PDOStatement $query  PDOStatement object to execute.
     *
     * @since  4.0.0
     * @return array
     */
    public static function executePdoOne(PDOStatement $query)
    {
        try {
            self::execPdoQuery($query);
        } catch (PDOException $e) {
            throw new DALException($e->getMessage());
        }

        $result = $query->fetchColumn();

        return $result;

    }//end executePdoOne()


    /**
     * Executes the given SQL query.
     *
     * Returns the number of rows affected by the statement.
     *
     * @param string $sql Sql query to execute.
     *
     * @since  4.0.0
     * @return integer
     */
    public static function executeSql($sql)
    {
        $dbh  = self::getDb();
        $rows = $dbh->exec($sql);
        return $rows;

    }//end executeSql()


    /**
     * Executes the given SQL query and returns all rows in an associative
     * array.
     *
     * If optional parameter $col is specified, this does NOT return an
     * associative array, but rather values from a single column.
     *
     * @param string  $sql  Sql query to execute.
     * @param integer $col  Column to retrieve (zero-indexed).
     *
     * @since  4.0.0
     * @return array
     */
    public static function executeSqlAssoc($sql, $col=NULL)
    {
        $dbh = self::getDb();

		$query = $dbh->prepare($sql);
        return self::executePdoAssoc($query, $col);

    }//end executeSqlAssoc()


    /**
     * Executes the given SQL query and returns all rows in an array.
     *
     * @param string $sql Sql query to execute.
     *
     * @since  4.0.0
     * @return array
     */
    public static function executeSqlAll($sql)
    {
        $dbh = self::getDb();

        $query = $dbh->prepare($sql);
        return self::executePdoAll($query);

    }//end executeSqlAll()


    /**
     * Executes the given SQL query and returns all rows grouped by the
     * first column.
     *
     * @param string  $sql  Sql query to execute.
     *
     * @since  4.0.0
     * @return array
     */
    public static function executeSqlGrouped($sql)
    {
        $dbh = self::getDb();

        $query = $dbh->prepare($sql);
        return self::executePdoGrouped($query);

    }//end executeSqlGrouped()


    /**
     * Executes the given SQL query and returns all rows grouped by the
     * first column, with subsequent fields keyed by their field name.
     *
     * @param string  $sql  Sql query to execute.
     *
     * @since  4.0.0
     * @return array
     */
    public static function executeSqlGroupedAssoc($sql)
    {
        $dbh = self::getDb();

        $query = $dbh->prepare($sql);
        return self::executePdoGroupedAssoc($query);

    }//end executeSqlGroupedAssoc()


    /**
     * Executes the given SQL query and returns a single value
     *
     * @param string $sql Sql query to execute.
     *
     * @since  4.0.0
     * @return array
     */
    public static function executeSqlOne($sql)
    {
        $dbh = self::getDb();

        $query = $dbh->prepare($sql);
        return self::executePdoOne($query);

    }//end executeSqlOne()


    /**
     * Executes multiple queries in one SQL string.
     *
     * Each sql statement must be separated by a semi-colon. Create table
     * statements should use this function since they may have multiple queries
     * to create indexes, constraints, etc. Also, it should not be used for
     * output statements (i.e. SELECT).
     *
     * @param string $sql SQL statements.
     *
     * @since  4.0.0
     * @return void
     */
    public static function executeQueries($sql)
    {
        // Split SQL statements.
        $queries = split(';', $sql);
        if (empty($queries) === FALSE) {
            foreach ($queries as $query) {
                if (trim($query) !== '') {
                    self::executeSql($query);
                }
            }
        }

    }//end executeQueries()


    /**
     * Executes an SQL statement, returning a result set as a PDOStatement.
     *
     * @param string $sql The sql statement.
     *
     * @since  4.0.0
     * @return PDOStatement
     */
    public static function query($sql)
    {
        $dbh    = self::getDb();
        $result = $dbh->query($sql);
        return $result;

    }//end query()


    /**
     * Initiates a transaction in the current database.
     *
     * @since  4.0.0
     * @return void
     */
    public static function beginTransaction()
    {
        $currentDb = self::getCurrentDbId();
        $dbInfo    = self::$_dbhPool[$currentDb];

        if ($dbInfo['transactionSupport'] === TRUE && $dbInfo['transactionSupport'] === FALSE) {
            $dbh = self::getDb();
            $dbh->beginTransaction();
            self::$_dbhPool[$currentDb]['inTransaction'] = TRUE;
        }

    }//end beginTransaction()


    /**
     * Commits a transaction in the current database.
     *
     * @since  4.0.0
     * @return void
     */
    public static function commit()
    {
        $currentDb = self::getCurrentDbId();

        if (self::$_dbhPool[$currentDb]['inTransaction'] === TRUE) {
            $dbh = self::getDb();
            $dbh->commit();
            self::$_dbhPool[$currentDb]['inTransaction'] = FALSE;
        }

    }//end commit()


    /**
     * Rolls back a transaction in the current database.
     *
     * All changes made since last commit will be lost.
     *
     * @since  4.0.0
     * @return void
     */
    public static function rollBack()
    {
        $currentDb = self::getCurrentDbId();

        if (self::$_dbhPool[$currentDb]['inTransaction'] === TRUE) {
            $dbh = self::getDb();
            $dbh->rollBack();
            self::$_dbhPool[$currentDb]['inTransaction'] = FALSE;
        }

    }//end rollBack()


    /**
     * Returns the ID of the last inserted row or sequence value.
     *
     * !!!Remove this funcion!!!
     *
     * @param string $sequenceName Name of the sequence from which the ID
     *                             should be returned.
     *
     * @since  4.0.0
     * @return integer
     */
    public static function lastInsertId($sequenceName)
    {
        $dbh = self::getDb();
        return (int)$dbh->lastInsertId($sequenceName);

    }//end lastInsertId()


    /*
        Database Connection
    */


    /**
     * Connects to the selected database.
     *
     * Throws DALException if cannot connect to db else returns PDO object.
     *
     * @param array  $dsn      DSN array.
     * @param string $conn_id  Unique connection ID
     *
     * @since  4.0.0
     * @return object
     * @throws DALException If db connection fails.
     */
    public static function dbConnect(array $dsn=NULL, $conn_id='db')
    {
        try {
            if ($dsn === NULL) {
                // If DSN is not specified, get our DSN list; if a config entry
                // does not exist for this connection, there is a problem.
                $dsn = self::getDsnFromConfig($conn_id);
            }

            $user = NULL;
            $pass = NULL;
            if (isset($dsn['user']) === TRUE) {
                $user = $dsn['user'];
            }

            if (isset($dsn['password']) === TRUE) {
                $pass = $dsn['password'];
            }

            $new_dbh = new PDO($dsn['DSN'], $user, $pass);
            $new_dbh->setAttribute(PDO::ATTR_ERRMODE, PDO::ERRMODE_EXCEPTION);
            $new_dbh->setAttribute(PDO::ATTR_CASE, PDO::CASE_LOWER);
            $new_dbh->setAttribute(PDO::ATTR_ORACLE_NULLS, PDO::NULL_EMPTY_STRING);

            if ($dsn['type'] === 'mysql') {
                $new_dbh->setAttribute(PDO::ATTR_EMULATE_PREPARES, TRUE);
                $new_dbh->setAttribute(PDO::MYSQL_ATTR_USE_BUFFERED_QUERY, TRUE);
            }

            self::$_dbhPool[$conn_id] = array(
                                             'dbh'      => $new_dbh,
                                             'dsn'      => $dsn['DSN'],
                                             'dbType'   => $dsn['type'],
                                             'userName' => $user,
                                             'password' => $pass,
                                        );

            // Check if this database supports db transactions.
            // We need to change first because transaction functions won't
            // like no current database
            self::changeDb($conn_id);

            if (self::transactionSupport($conn_id) === TRUE) {
                self::$_dbhPool[$conn_id]['transactionSupport'] = TRUE;
            } else {
                self::$_dbhPool[$conn_id]['transactionSupport'] = FALSE;
            }

            self::restoreDb();

            self::$_dbhPool[$conn_id]['inTransaction'] = FALSE;

        } catch (PDOException $e) {
            throw new DALException('DBError!:'.$e->getMessage());
        }

        return self::$_dbhPool[$conn_id]['dbh'];

    }//end dbConnect()


    /**
     * Change database connection.
     *
     * @param string $conn_id   Connection ID to change
     *
     * @since  4.0.0
     * @return void
     * @throws DALException If the connection identified by the ID is not (yet) connected.
     */
    public static function changeDb($conn_id)
    {
        if (self::dbConnected($conn_id) === FALSE) {
            throw new DALException('Connection "'.$conn_id.'" must be connected before it can be changed to.');
        }

        array_push(self::$_dbStack, $conn_id);

    }//end changeDb()


    /**
     * Restore database connection.
     *
     * @since  4.0.0
     * @return void
     * @throws DALException If the database connection stack is empty.
     */
    public static function restoreDb()
    {
        if (empty(self::$_dbStack) === TRUE) {
            throw new DALException('The database connection stack is empty; there is no previous connection to restore.');
        }

        array_pop(self::$_dbStack);

    }//end restoreDb()


    /**
     * Get the current database connection ID.
     *
     * @since  4.0.0
     * @return void
     * @throws DALException If the database connection stack is empty.
     */
    public static function getCurrentDbId()
    {
        if (empty(self::$_dbStack) === TRUE) {
            throw new DALException('The database connection stack is empty.');
        }

        $conn_id = self::$_dbStack[count(self::$_dbStack) - 1];

        return $conn_id;

    }//end restoreDb()


    /**
     * Checks if the database supports transactions.
     *
     * If PDO does not throw an exception when beginTransaction is called then
     * transactions are supported.
     *
     * @param string  $conn_id  Connection ID to close.
     *
     * @since  4.0.0
     * @return boolean
     */
    public static function transactionSupport($conn_id=NULL)
    {
        try {
            // Test if db transactions work.
            $dbh = self::getDb($conn_id);
            $dbh->beginTransaction();
            self::$_dbhPool[$conn_id]['inTransaction'] = TRUE;
        } catch (PDOException $e) {
            return FALSE;
        }

        // Close dbt.
        self::rollBack();
        return TRUE;

    }//end transactionSupport()


    /**
     * Close the specified database connection.
     *
     * @param string  $conn_id  Connection ID to close.
     *
     * @since  4.0.0
     * @return void
     * @throws DALException If connection doesn't exist, or connection is still in the stack.
     */
    public static function dbClose($conn_id)
    {
        if (isset(self::$_dbhPool[$conn_id]) === FALSE) {
            // Can't close a connection that isn't open...
            throw new DALException('Connection with ID "'.$conn_id.'" cannot be closed because a connection of that ID is not open.');
        } else if (array_search($conn_id, self::$_dbStack) !== FALSE) {
            // If this connection is still in the database stack, we should not
            // allow them to close the connection.
            throw new DALException('Connection with ID "'.$conn_id.'" cannot be closed because it still exists in the stack.');
        } else {
            unset(self::$_dbhPool[$conn_id]);
        }

    }//end dbClose()


    /**
     * Returns true if there is an active database connection.
     *
     * If the optional $conn_id parameter is provided, this function will return
     * the existence or otherwise of that connection. If omitted, it will return
     * the existence of ANY open connection.
     *
     * @param string  $conn_id  Connection ID to check.
     *
     * @since  4.0.0
     * @return boolean
     */
    public static function dbConnected($conn_id=NULL)
    {
        if (is_null($conn_id) === TRUE) {
            return !empty(self::$_dbhPool);
        } else {
            return isset(self::$_dbhPool[$conn_id]);
        }

    }//end dbConnected()


    /**
     * Returns the DSN array for the current database configuration.
     *
     * @param string  $conn_id  Connection ID to get details from. If omitted, use current connection.
     *
     * @since  4.0.0
     * @return array
     * @throws DALException If DSN not configured.
     */
    public static function getDsn($conn_id=NULL)
    {
        if (is_null($conn_id)) {
            $conn_id = self::getCurrentDbId();
        }

        if (isset(self::$_dbhPool[$conn_id]) === FALSE) {
            $error = 'DAL configuration not defined for connection ID "'.$conn_id.'".';
            throw new DALException($error);
        } else {
            $dsn = self::$_dbhPool[$conn_id];
        }

        return $dsn;

    }//end getDsn()



    /**
     * Gets the DSN array for the specified connection ID from the DAL configuration.
     *
     * @param string  $conn_id  Connection ID to get details from.
     *
     * @since  4.0.0
     * @return array
     * @throws DALException If DSN not configured.
     */
    public static function getDsnFromConfig($conn_id)
    {
        $dsn = include dirname(__FILE__).'/DALConf.inc';
        if (isset($dsn[$conn_id]) === FALSE) {
            $error = 'DAL configuration not defined for connection ID "'.$conn_id.'".';
            throw new DALException($error);
        } else if (isset($dsn[$conn_id]['DSN']) === FALSE) {
            $error = 'DAL configuration for connection ID "'.$conn_id.'" does not define a DSN.';
            throw new DALException($error);
        }

        return $dsn[$conn_id];

    }//end getDsnFromConfig()


    /**
     * Returns the current database name using its DSN.
     *
     * @since  4.0.0
     * @return string
     */
    public static function getDbName($conn_id)
    {
        if (self::$_[$conn_id] === NULL) {
            // Try to get the database name from DSN string.
            $dsn = self::getDsn();
            if (isset($dsn['DSN']) === TRUE) {
                $matches = array();
                $reg     = '/.*dbname=([a-zA-Z0-9_]+)[;]?/i';
                preg_match($reg, $dsn['DSN'], $matches);
                if (isset($matches[1]) === TRUE) {
                    self::$_currentDbName = $matches[1];
                }
            }
        }

        return self::$_currentDbName;

    }//end getDbName()


    /**
     * Returns the PDO object for the current database.
     *
     * @param string  $conn_id  If provided, the function returns a specific connection.
     *                          If omitted, the function returns the current connection.
     *
     * @since  4.0.0
     * @return PDO
     */
    public static function getDb($conn_id=NULL)
    {
        if (is_null($conn_id)) {
            if (empty(self::$_dbStack) === TRUE) {
                throw new DALException('Cannot get current database because no current database exists.');
            } else {
                $conn_id = self::getCurrentDbId();
            }
        }

        // If we were provided a connection ID and it doesn't exist, try
        // connecting to it using DALConf settings; otherwise, fail.
        if (isset(self::$_dbhPool[$conn_id]) === FALSE) {
            try {
                self::dbConnect(NULL, $conn_id);
            } catch (DALException $e) {
                throw new DALException($e->getMessage());
            }
        }

        return self::$_dbhPool[$conn_id]['dbh'];

    }//end getDb()


    /**
     * Returns the current database connection's type.
     *
     * @param boolean $readable If true readable name will be returned.
     *                          For example, myql => Mysql, pgsql => Postgres.
     * @param string  $conn_id  If provided, the function checks this specified connection.
     *                          If omitted, the function returns the current connection.
     *
     * @since  4.0.0
     * @return string
     * @throws DALException If db type is not found or it is invalid.
     */
    public static function getDbType($readable=FALSE, $conn_id=NULL)
    {
        if (is_null($conn_id)) {
            $conn_id = self::getCurrentDbId();
        }

        //$conf = include dirname(__FILE__).'/DALConf.inc';
        $dbInfo = self::$_dbhPool[$conn_id];
        if (isset($dbInfo['dbType']) === FALSE) {
            throw new DALException('Could not find "type" in config.');
        } else if (isset(self::$_dbNames[$dbInfo['dbType']]) === FALSE) {
            $msg = $dbInfo['dbType'].' is not a valid database type.';
            throw new DALException($msg);
        }

        if ($readable === TRUE) {
            return self::$_dbNames[$dbInfo['dbType']];
        }

        return $dbInfo['dbType'];

    }//end getDbType()


    /**
     * Requires the queries class file for a system.
     *
     * @param string $systemid ID of the system.
     *
     * @since  4.0.0
     * @return void
     */
    public static function requireQueryClass($systemid)
    {
        $classFile = $systemid.'Queries.inc';
        $path      = self::getOvenPath($systemid).'/'.$classFile;

        require_once($path);

    }//end requireQueryClass()


    /**
     * Returns the oven dir path.
     *
     * If system name is provided then, that system's oven dir will be returned.
     *
     * @param string $system System name.
     *
     * @since  4.0.0
     * @return string
     */
    public static function getOvenPath($system=NULL)
    {
        $path = dirname(__FILE__).'/Oven';

        if ($system !== '') {
            $path .= '/'.$system;
        }

        return $path;

    }//end getOvenPath()


    /**
     * Returns the path to store query objects.
     *
     * @since  4.0.0
     * @return string
     */
    public static function getQueryStorePath()
    {
        $path = dirname(__FILE__).'/QueryStore';
        return $path;

    }//end getQueryStorePath()


    /**
     * Returns the path to store query objects.
     *
     * @param string $id The ID of the query to retrieve.
     *
     * @since  4.0.0
     * @return Query
     * @throws DALException When object is not found.
     */
    public static function getQueryObject($id)
    {
        $queryStore = self::getQueryStorePath();
        $path       = $queryStore.'/'.$id.'.qob';
        if (file_exists($path) === FALSE) {
            $msg = 'Query object doesn\'t exist';
            throw new DALException($msg);
        }

        $content  = file_get_contents($path);
        $queryObj = unserialize($content);
        return $queryObj;

    }//end getQueryObject()


    /**
     * Split SQL quries that are in a file and seperated by "--$$".
     *
     * @param string $path File path of the SQL file.
     *
     * @since  4.0.0
     * @return array
     */
    public static function getSqlQueriesFromFile($path)
    {
        $queries = array();
        if (file_exists($path) === TRUE) {
            $fileContents = file_get_contents($path);
            $queries      = explode('--$$', $fileContents);
        }

        return $queries;

    }//end getSqlQueriesFromFile()


    /**
     * Returns TRUE if passed variable is a DAL query.
     *
     * @param mixed $query Variable to test.
     *
     * @since  4.0.0
     * @return boolean
     */
    public static function isDALQuery($query)
    {
        if (($query instanceof Query) === TRUE) {
            return TRUE;
        }

        return FALSE;

    }//end isDALQuery()


    /**
     * Starts query timer for given queryid.
     *
     * @param string $queryid Id of the query.
     *
     * @since  4.0.0
     * @return void
     */
    protected static function _startQueryTimer($queryid)
    {
        if (self::$_logQueries === TRUE) {
            self::$_currentQueryLog = array();
            self::$_currentQueryLog['id']    = $queryid;
            self::$_currentQueryLog['start'] = microtime(TRUE);
        }

    }//end _startQueryTimer()


    /**
     * Stops query timer.
     *
     * @since  4.0.0
     * @return void
     */
    protected static function _endQueryTimer()
    {
        if (self::$_currentQueryLog !== NULL) {
            if (isset(self::$_queryLog[self::$_currentQueryLog['id']]) === FALSE) {
                self::$_queryLog[self::$_currentQueryLog['id']] = array();
            }

            self::$_queryLog[self::$_currentQueryLog['id']][] = (microtime(TRUE) - self::$_currentQueryLog['start']);
        }

        self::$_currentQueryLog = NULL;

    }//end _endQueryTimer()


    /**
     * Returns list of logged queries.
     *
     * @since  4.0.0
     * @return array
     */
    public static function getLoggedQueries()
    {
        return self::$_queryLog;

    }//end getLoggedQueries()


}//end class


?>
