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
* $Id: MatrixDALBaker.inc,v 1.1 2007/10/29 05:54:11 lwright Exp $
*
*/


require_once SQ_LIB_PATH.'/DAL/DALBaker.inc';
require_once SQ_LIB_PATH.'/FileSystem/FileSystem.inc';
require_once SQ_LIB_PATH.'/XML/XML.inc';

/**
* MatrixDALBaker
*
* Purpose
*    Extends and overrides the Database Abstraction Layer (DAL) baker class,
*    which creates tables and queries optimised for YOU (!), to make it more
*    useful for MySource Matrix (MySource 3.x).
*
*    MySource Matrix will only use the query-baking facilities of the DAL baker;
*    the schema-baking will not be used as Matrix already has the "tables.xml"
*    structure.
*
*    Major differences provided by the Matrix DAL extension:
*    - "Systems" as used in MySource 4.x are not a MySource Matrix concept -
*      their closest analogies in Matrix are packages and assets. When defining
*      "systems" from Matrix DAL's perspective, we will define three levels:
*      the core, package level, and asset level.
*
* @since   MySource 3.18.0 RC1
* @author  Squiz Pty Ltd <matrix@squiz.net>
* @version $Revision: 1.1 $
* @package MySource_Matrix
*/
class MatrixDALBaker extends DALBaker
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
     * Add queries for the MySource Matrix core.
     *
     * These core-level queries are necessary for operation of the core
     * components of MySource Matrix (eg. those in the core/include, core/hipo,
     * core/cron directories and so on). Its queries.xml file will live in a
     * "core/db" directory underneath the Matrix system root.
     * Returns TRUE if the queries.xml file for the core was found.
     *
     * @since MySource 3.18.0 RC1
     * @return boolean
     */
    public static function addCoreQueries()
    {
        $filename = SQ_SYSTEM_ROOT.'/core/db/queries.xml';

        if (file_exists($filename)) {
            parent::processQueriesFile('core', $filename);
            echo self::_queryRequiresRollback('core', 'isAssetDependentlyLinked') ? 1 : 0;
            throw new Exception();
            return TRUE;
        }


        return FALSE;

    }//end addPackageQueries()


    /**
     * Add queries for a certain MySource Matrix package.
     *
     * These package-level queries will be named "[package_name]_package" and
     * their queries.xml file lives in the root directory of the package.
     * Returns TRUE if the queries.xml file for a package was found. The caller
     * use the return to avoid baking queries for packages without queries.
     *
     * @param string $package  The package to be processed.
     *
     * @since MySource 3.18.0 RC1
     * @return boolean
     * @throws DALBakerException If the package has not yet been installed.
     */
    public static function addPackageQueries($package)
    {
        $packages = $GLOBALS['SQ_SYSTEM']->getInstalledPackages();
        if (in_array($package, $packages)) {
            throw new DALBakerException('Unable to install queries for package "'.$package.'"; the package must be installed first.');
        }

        if ($package === 'core') {
            $filename = SQ_CORE_PACKAGE_PATH.'/queries.xml';
        } else {
            $filename = SQ_PACKAGES_PATH.'/'.$package.'/queries.xml';
        }

        //$ = $package_obj->getInstalledAssets();
        if (file_exists($filename)) {
            parent::processQueriesFile($package.'_package', $filename);
            return TRUE;
        }

        return FALSE;

    }//end addPackageQueries()


    /**
     * Add queries for a certain MySource Matrix asset type.
     *
     * These asset-type-level queries will be named the same as the asset type.
     * Their queries.xml file lives in the "db" directory of the asset.
     * Returns TRUE if the queries.xml file for an asset was found. The caller
     * use the return to avoid baking queries for asset types without queries.
     *
     * @param string $assetType  The asset type code to be processed.
     *
     * @since MySource 3.18.0 RC1
     * @return boolean
     * @throws DALBakerException If the asset has not yet been installed.
     */
    public static function addAssetTypeQueries($assetType)
    {
        if (!$GLOBALS['SQ_SYSTEM']->am->installed($assetType)) {
            throw new DALBakerException('Unable to install queries for asset "'.$assetType.'"; the asset must be installed first.');
        }

        $dir = $GLOBALS['SQ_SYSTEM']->am->getTypeInfo($assetType, 'dir');
        $filename = $dir.'/db/queries.xml';

        if (file_exists($filename)) {
            parent::processQueriesFile($assetType, $filename);
            return TRUE;
        }

        return FALSE;

    }//end addAssetTypeQueries()


    /**
     * Bakes out a system's queries and business logic into PHP files.
     *
     * Calls bakeQueryMethod() for each query that the system has defined.
     * For the query to be baked correctly it is required that its' XML files
     * have been created in the system's oven directory.
     *
     * @param string $systemName Name of the system to bake the code for.
     *
     * @since  4.0.0
     * @return void
     * @throws DALBakerException If the system name supplied is empty.
     */
    public static function bakeQueriesFile($systemName)
    {
        if (trim($systemName) === '') {
            throw new DALBakerException('System name is empty');
        }

        // Write the file.
        $ovenDir = SQ_LIB_PATH.'/DAL/Oven/'.$systemName;

        if (file_exists($ovenDir) === FALSE) {
            mkdir($ovenDir);
        }

        $fileName = $ovenDir.'/'.$systemName.'Queries.inc';

        $contents  = '<?'."php\n\n";
        $contents .= "require_once SQ_LIB_PATH.'/MatrixDAL/MatrixDAL.inc';\n";
        $contents .= "class $systemName"."Queries\n{\n\n";
        // Determine what XML files are present in the oven directory for this
        // system, and print out the method for each.
        $types    = array('.xml');
        $xmlFiles = FileSystem::listDirectory($ovenDir, $types, TRUE, FALSE);
        foreach ($xmlFiles as $queryName) {
            $queryName = basename($queryName, '.xml');
            $contents .= self::bakeQueryMethod($systemName, $queryName);
        }

        $contents .= "\n\n}\n?>";

        file_put_contents($fileName, $contents);

    }//end bakeQueriesFile()


    /**
     * Bakes out a method for a single query.
     *
     * Reads in the XML files that have been generated for this query, gets all
     * fragments and their assertions, and bakes out an appropriate method that
     * will return an SQL statement that corresponds to the current system state
     * and the systems currently installed.
     *
     * @param string $systemName The name of the system to which the query
     *                           belongs.
     * @param string $queryName  The name of the query to bake.
     *
     * @since  4.0.0
     * @return string
     */
    public static function bakeQueryMethod($systemName, $queryName)
    {
        // Output the method signature.
        $methodName = 'prepare'.ucwords($queryName).'Query';
        $contents   = "public static function $methodName(array \$data, \$bind)\n{\n";
        // Print out the initialisation of hard coded values.
        $contents .= self::_printPlaceHolderVariables($systemName, $queryName);
        // Get the values required for our assertions.
        $contents .= "try {\n";
        // Loop through each assertion, and output a call that assigns its
        // value to a variable.
        $assertions = self::_getQueryAssertions($systemName, $queryName);
        // Print out the code for including the assertions systems and
        // initialising their values.
        $contents .= self::_printAssertionCalls($assertions);
        $contents .= "} catch (ChannelException \$e) {\n";
        $contents .= "    // Exception thrown in the called channel.\n";
        $contents .= "}\n";

        // Get all the conditions that represent all of the query's
        // combinations, and add it to the contents.
        $queryBody  = self::mergeQuery($systemName, $queryName);
        $contents  .= $queryBody;
        $subQueries = self::printSubQueries($systemName, $queryName);
        $contents  .= $subQueries;
        $contents  .= self::_printKeywords($systemName, $queryName);
        $contents  .= self::_printBindings($systemName, $queryName);
        $contents  .= "\n}//end $methodName()\n";

        return $contents;

    }//end bakeQueryMethod()


    /**
     * Prints code to prepare the query and bind the placeholders to data.
     *
     * This returns the code that is executed after the $sql variable is
     * obtained from the assertion's conditions. The $dbh variable is
     * initialised by getting the database connection from the DAL. Using the
     * connection, the obtained $sql is prepared. All bindings are then obtained
     * from the XML for the query. If they are a placeholder for a hard-coded
     * value, then the placeholder is bound to the initialised variable,
     * otherwise, the binding is done for an index into the passed $data array.
     *
     * @param string  $systemName    The system to print the bindings for.
     * @param string  $queryName     The query to print the bindings for.
     * @param boolean $printBindings If TRUE, will print bindings, otherwise
     *                               will print entries into the query array.
     *
     * @since  4.0.0
     * @return string
     */
    protected static function _printBindings($systemName, $queryName, $printBindings=TRUE)
    {
        $doc      = self::getQueryXML($systemName, $queryName);
        $contents = '';

        $bindingList = $doc->getElementsByTagName('binding');
        // Create an array containing our place holder names, so we can easily
        // find if a variable name has been generated by the system.
        $placeHolders     = array();
        $placeHoldersList = $doc->getElementsByTagName('placeholder');
        foreach ($placeHoldersList as $element) {
            $varName                = $element->getAttribute('var_name');
            $varValue               = $element->getAttribute('value');
            $placeHolders[$varName] = $varValue;
        }

        $usedNames = array();
        foreach ($bindingList as $binding) {
            $bindName = $binding->getAttribute('name');

            // We need to determine what data type the column is.
            $columnType = $binding->getAttribute('column_type');
            if ($columnType !== '') {
                //$dataType = self::_getPdoDataType($system, $table, $column, $columnType);
            } else {
                $column   = $binding->getAttribute('column');
                $table    = $binding->getAttribute('table');
                $system   = $binding->getAttribute('system');
                //$dataType = self::_getPdoDataType($system, $table, $column);
            }

            $varName = '';
            $index   = substr($bindName, 1);
            // Check if the binding is a hard coded value. If it is, set its'
            // variable to be the same as defined in _printPlaceHoldersVariables
            // otherwise print an index into the $data array.
            if (isset($placeHolders[$bindName]) === TRUE) {
                // This is a place holder for a hard-coded value.
                $varName = '$'.substr($bindName, 1);
            } else {
                // This is not hard-coded, so should be referenced in the $data
                // array.
                $varName = "\$data['$index']";
            }

            if ($printBindings === TRUE) {
                // Add check for array values.
                $bindCall = '';
                if (isset($placeHolders[$bindName]) === FALSE) {
                    $bindCall = "\nif (array_key_exists('$index',\$data) === TRUE) {\n";
                }

                //$bindCall .= "\$query->bind('$bindName', $varName, $dataType);\n";
                $bindCall .= "\$query->bind('$bindName', $varName);\n";

                if (isset($placeHolders[$bindName]) === FALSE) {
                    $bindCall .= "}\n\n";
                }

                // Add this binding to the contents.
                $contents .= $bindCall;
            }

        }//end foreach

        $contents .= "return \$query;\n";

        return $contents;

    }//end _printBindings()


    /**
     * Determines from the MySource Matrix tables.xml schema(s) whether a query
     * requires a rollback version to be generated for it.
     *
     * This will be the case if any of the tables it uses requires a rollback
     * table.
     *
     * This function will only return a positive result for SELECT queries -
     * queries that involve manipulation have their rollback actions covered
     * by the trigger system. If the query is not a SELECT query, then there
     * will be no tables found, and therefore this function will return in the
     * negative.
     *
     * TODO: need to check to see whether SELECT parts of INSERT...SELECT
     * combination queries need this too - probably not, as you are very
     * unlikely to have to add non-live data from another table.
     *
     * TODO: need to implement SELECT sub-query checking for SELECT...SELECT
     * situations.
     *
     * @param string  $systemName    The system to search for.
     * @param string  $queryName     The query to search for.
     *
     * @since  MySource 3.18.0 RC1
     * @return boolean
     */
    protected static function _queryRequiresRollback($systemName, $queryName)
    {
        $doc = self::getQueryXML($systemName, $queryName);
        $xpath = new DOMXPath($doc);
        $nodes = $xpath->query('//query/primary/select/from/table');
        foreach ($nodes as $node) {
            $table = $node->nodeValue;
            if (self::_tableRequiresRollback($table)) {
                return TRUE;
            }
        }

        return FALSE;

    }//end _queryRequiresRollback()


    /**
     * Determines from the MySource Matrix tables.xml schema whether a table
     * requires a rollback table to be generated for it.
     *
     * The core package will always be searched in, even if no packages (not
     * even core) have been installed. This is due to the core package
     * including the required tables for the core of MySource Matrix.
     *
     * @param string $table  The table to be searched for.
     *
     * @since  MySource 3.18.0 RC1
     * @return boolean
     */
    protected static function _tableRequiresRollback($table)
    {
        // lop off 'sq_' prefix if it exists
        if (substr($table, 0, 3) == 'sq_') {
            $table = substr($table, 3);
        }

        // This might be called before anything actually is installed, so if
        // the core queries haven't yet been installed, assume that there are
        // no packages installed.
        $ovenDir = SQ_LIB_PATH.'/DAL/Oven/core';

        if (file_exists($ovenDir) === TRUE) {
            $packages = $GLOBALS['SQ_SYSTEM']->getInstalledPackages();
        } else {
            $packages = Array();
        }

        // If no packages have been installed yet, we want to at least include
        // the core package.
        if (empty($packages)) {
            $packages[] = '__core__';
        }

        // Loop through each tables.xml file, see if we can find a table entry
        // for this table that requires rollback.
        foreach ($packages as $package_name) {
            if ($package_name === '__core__') {
                $dir = SQ_CORE_PACKAGE_PATH;
            } else {
                $dir = SQ_PACKAGES_PATH.'/'.$package_name;
            }
            $filename = $dir.'/tables.xml';

            if (file_exists($filename) === TRUE) {
                $doc = new DOMDocument('1.0', 'iso-8859-1');
                $doc->load($filename);

                $xpath = new DOMXPath($doc);
                $count = $xpath->evaluate('count(/schema/tables/table[@name="'.$table.'" and @require_rollback="1"])');
                if ($count > 0) return TRUE;
            }
        }

        // could not find a table, so return FALSE
        return FALSE;

    }//end _tableRequiresRollback()


    /**
     * Processes a queries.xml file to prepare it for baking
     *
     * @param string $systemName The system to add queries to.
     * @param string $fileName   The name of the queries file.
     *
     * @since  4.0.0
     * @return void
     * @throws DALBakerException When an ID isn't specified for a query.
     */
    public static function processQueriesFile($systemName, $fileName)
    {
        if (file_exists($fileName) === FALSE) {
            return FALSE;
        }

        $doc = new DomDocument();
        $doc->load($fileName);
        $queryNode = $doc->getElementsByTagName('queries')->item(0);

        $currentDbType = DAL::getDbType();

        foreach ($queryNode->childNodes as $child) {
            if ($child->nodeType !== 1) {
                // Text Node. Probably just whitespace or a comment. We don't
                // want to deal with it.
                continue;
            }

            if ($child->tagName === 'query') {
                if ($child->hasAttribute('id') === FALSE) {
                    $error = 'ID not specified for query in '.$systemName;
                    throw new DALBakerException($error);
                }

                if ($child->hasAttribute('databases') === TRUE) {
                    $dbString = $child->getAttribute('databases');
                    $dbString = str_replace(' ', '', $dbString);
                    $dbs      = explode(',', $dbString);
                    if (in_array($currentDbType, $dbs) === FALSE) {
                        $id = $child->getAttribute('id');
                        echo 'Skipped Query: '.$id." (not for this DB type)\n";
                        continue;
                    }
                }

                if ($child->hasAttribute('hookid') === TRUE) {
                    $hookId = $child->getAttribute('hookid');
                    if (substr_count($hookId, '.') === 1) {
                        // We have a fragment.
                        self::addQueryFragment($systemName, $child);
                    } else {
                        // We have a sub query with a three part hookid (*.*.*).
                        self::addSubQuery($systemName, $child);
                    }
                } else {
                    // New Query.
                    self::addQuery($systemName, $child);
                }
            }//end if
        }//end foreach childnodes

    }//end processQueriesFile()


}
?>
