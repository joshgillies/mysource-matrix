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
* $Id: MatrixDALBaker.inc,v 1.17 2012/08/30 01:09:20 ewang Exp $
*
*/


require_once SQ_LIB_PATH.'/DAL/DAL.inc';
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
* @version $Revision: 1.17 $
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
            self::processQueriesFile('core', $filename);
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

        if (file_exists($filename)) {
            self::processQueriesFile($package.'_package', $filename);
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
        $filename = SQ_SYSTEM_ROOT.'/'.$dir.'/db/queries.xml';

        if (file_exists($filename)) {
            self::processQueriesFile($assetType, $filename);
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
     * @return boolean
     * @throws DALBakerException If the system name supplied is empty.
     */
    public static function bakeQueriesFile($systemName)
    {
        if (trim($systemName) === '') {
            throw new DALBakerException('System name is empty');
        }

        // Write the file.
        $ovenDir = DAL::getOvenPath($systemName);

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
            if (substr($queryName, -3) !== '.rb') {
                $contents .= self::bakeQueryMethod($systemName, $queryName);
            }
        }

        $contents .= "\n\n}\n?>";

		$result = file_put_contents($fileName, $contents);
		if ($result !== FALSE) {
			return TRUE;
		}
		return FALSE;

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
        if (self::_queryRequiresRollback($systemName, $queryName)) {
            $contents  .= 'if (SQ_ROLLBACK_VIEW) {'."\n";
            $queryBody  = self::mergeQuery($systemName, $queryName.'.rb');
			$allBindVars = self::findRollbackBindVars($queryBody);
            $contents  .= $queryBody;
			if (!empty($allBindVars)) {
				foreach ($allBindVars as $bindVar) {
	            	$contents  .= '$query->bind(\''.$bindVar.'\', $_SESSION[\'sq_rollback_view\'][\'rollback_time\']);'."\n";
				}//end foreach
			} else {
	            $contents  .= '$query->bind(\':rbtime\', $_SESSION[\'sq_rollback_view\'][\'rollback_time\']);'."\n";
			}//end if
            $contents  .= '} else {'."\n";
            $queryBody  = self::mergeQuery($systemName, $queryName);
            $contents  .= $queryBody;
            $contents  .= '}';
        } else {
            $queryBody  = self::mergeQuery($systemName, $queryName);
            $contents  .= $queryBody;
        }
        $subQueries = self::printSubQueries($systemName, $queryName);
        $contents  .= $subQueries;
        $contents  .= self::_printKeywords($systemName, $queryName);
        $contents  .= self::_printBindings($systemName, $queryName);
        $contents  .= "\n}//end $methodName()\n";

        return $contents;

    }//end bakeQueryMethod()


    /**
     * Find and returns the bind vars for rollback ie. "rbtime" 
     *
     * @param string $queryBody  The return from mergeQuery which contains the query information
     *
     * @return array
     */
     public static function findRollbackBindVars($queryBody)
     {
       $sqlQuery = preg_match_all("/\('(.*)'\)/", $queryBody, $matches);

       $allBindVars = Array();
       if (isset($matches[1]) && !empty($matches[1])) {
	       foreach ($matches[1] as $match) {
	           $query = DAL::getQueryObject($match);
               $rollbackBinds = preg_match_all('/(:rbtime[0-9]*)\b/i', $query->getSql(), $foundBindVars);
               if (isset($foundBindVars[1]) && !empty($foundBindVars[1])) {
                   $allBindVars = array_merge($allBindVars, $foundBindVars[1]);
               }//end if
           }//end foreach
		   return $allBindVars;
       }//end if


       return Array();

     }//end findRollbackBindVars()


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
        $lobCount  = 0;

        foreach ($bindingList as $binding) {
            $requiresLob = FALSE;
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

				if (empty($table) === TRUE) {
					// We are not tied to any column. Therefore we have no idea whether
					// it requires a LOB or not. I think it's fair to assume that if
					// it's a placeholder, then it will not need a LOB.
					if (array_key_exists($bindName, $placeHolders) === FALSE) {
						$requiresLob = NULL;
					}
				} else if (self::_fieldRequiresOciClob($table, $column) === TRUE) {
                    // OCI;
                    $requiresLob = TRUE;
                }
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

                if (MatrixDAL::getDbType() === 'oci') {
					if ($requiresLob === TRUE) {
	                    $lobCount++;
    	                $bindCall .= "\$lob{$lobCount} = oci_new_descriptor(MatrixDAL::getDb(), OCI_D_LOB);\n";
        	            $bindCall .= "\$query->bind('$bindName', \$lob{$lobCount}, PDO::PARAM_LOB);\n";
            	        $bindCall .= "\$lob{$lobCount}->writeTemporary($varName, OCI_TEMP_CLOB);\n";
					} else if ($requiresLob === NULL) {
						// Not tied to a column. If we got passed a LOB, then use it,
						// otherwise, pass it as normal. Note that we cannot use instanceof()
						// because of the dodgy class name OCI8 extension uses.
	                    $bindCall .= "if (is_object($varName) && get_class($varName) === 'OCI-Lob') {\n";
	                    $bindCall .= "\$query->bind('$bindName', $varName, PDO::PARAM_LOB);\n";
	                    $bindCall .= "} else {\n";
	                    $bindCall .= "\$query->bind('$bindName', $varName);\n";
	                    $bindCall .= "}\n";
					} else {
	                    $bindCall .= "\$query->bind('$bindName', $varName);\n";
					}
                } else {
                //$bindCall .= "\$query->bind('$bindName', $varName, $dataType);\n";
                    $bindCall .= "\$query->bind('$bindName', $varName);\n";
                }

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
        $filePath = realpath(SQ_SYSTEM_ROOT).'/data/private/Oven/'.$systemName.'/'.$queryName.'.xml';
        $doc = new DOMDocument();
        $doc->loadXML(file_get_contents($filePath));
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
        // TODO:
        $ovenDir = DAL::getOvenPath('core').'/coreQueries.xml';

        try {
            $packages = $GLOBALS['SQ_SYSTEM']->getInstalledPackages();
        } catch (Exception $e) {
            $packages = Array();
        }

        // If no packages have been installed yet, we want to at least include
        // the core package.
		//
		// This is a multi-dimensional array because that's what getInstalledPackages
		// returns later. We only care about including the code_name when we install.

		if (empty($packages)) {
			$packages[] = array(
							'code_name'	=> '__core__',
						  );
		}

		// Loop through each tables.xml file, see if we can find a table entry
		// for this table that requires rollback.
		foreach ($packages as $_key => $package_details) {
			$package_name = $package_details['code_name'];

            if ($package_name === '__core__') {
                $dir = SQ_CORE_PACKAGE_PATH;
            } else {
                $dir = SQ_PACKAGES_PATH.'/'.$package_name;
            }
            $filename = $dir.'/tables.xml';

            if (file_exists($filename) === TRUE) {
                $doc = new DOMDocument('1.0', 'iso-8859-1');
                $doc->loadXML(file_get_contents($filename));

                $xpath = new DOMXPath($doc);
                $count = $xpath->evaluate('count(/schema/tables/table[@name="'.$table.'" and @require_rollback="1"])');
                if ($count > 0) return TRUE;
			}
        }

        // could not find a table, so return FALSE
        return FALSE;

    }//end _tableRequiresRollback()


    /**
     * Determines from the MySource Matrix tables.xml schema whether a
     * certain field is defined as a CLOB in Oracle.
     *
     * This is necessary to know for Oracle, as there is an extra step that
     * must be handled - a "LOB descriptor" must be made for a field, and then
     * that descriptor is bound, rather than the field value itself.
     *
     * Even when PDO works again, the query must be rewritten in a different
     * way because of how Oracle handles LOB fields.
     *
     * @param string $table  The table to be searched for.
     * @param string $field  The field to be searched for.
     *
     * @since  MySource 3.18.0 RC2
     * @return boolean
     */
    protected static function _fieldRequiresOciClob($table, $field)
    {
        // OCI:
        if ((empty($table) === TRUE) || (empty($field) === TRUE)) {
            // This bind variable was not completely inside a "column" attribute
            // (eg. it might be inside a function). Assume it's not a LOB for now
            return FALSE;
        }

        // lop off 'sq_' prefix if it exists
        if (substr($table, 0, 3) == 'sq_') {
            $table = substr($table, 3);
        }

        // Is this a view? Assume innocent until proven guilty
        if (substr($table, 0, 3) == 'vw_') {
            return FALSE;
        }

        // lop off 'rb_' prefix if it exists. If this is a rollback table, then
        // the type of column will be the same as that of the main table
        if (substr($table, 0, 3) == 'rb_') {
            $table = substr($table, 3);
        }

        // This might be called before anything actually is installed, so if
        // the core queries haven't yet been installed, assume that there are
        // no packages installed.
        // TODO:
        $ovenDir = DAL::getOvenPath('core').'/coreQueries.xml';

        try {
            $packages = $GLOBALS['SQ_SYSTEM']->getInstalledPackages();
        } catch (Exception $e) {
            $packages = Array();
        }

        // If no packages have been installed yet, we want to at least include
        // the core package.
        if (empty($packages)) {
            $packages[] = Array('code_name' => '__core__');
        }

        // Loop through each tables.xml file, see if we can find a table entry
        // for this table that requires rollback.
        foreach ($packages as $package_name) {
            $package_name = $package_name['code_name'];
            if ($package_name === '__core__') {
                $dir = SQ_CORE_PACKAGE_PATH;
            } else {
                $dir = SQ_PACKAGES_PATH.'/'.$package_name;
            }
            $filename = $dir.'/tables.xml';

            if (file_exists($filename) === TRUE) {
                $doc = new DOMDocument('1.0', 'iso-8859-1');
                $doc->loadXML(file_get_contents($filename));

                // We are going to take advantage of the fact that a [C]LOB
                // field in Oracle is always going to be specified as a "type
                // variation" in Matrix's "tables.xml" file. PostgreSQL types
                // generally come first, and "TEXT" was also used for MySQL
                // when Matrix supported that, so Oracle's type is demoted to
                // a type variation.

                // First find out if such a field actually exists...
                $xpath = new DOMXPath($doc);
                $count = $xpath->evaluate('count(/schema/tables/table[@name="'.$table.'"]/columns/column[@name="'.$field.'"])');
                if ($count > 0) {
                    // we found it, let's see if it has a OCI variation, and
                    // that it's of the correct type
                    $nodes = $xpath->query('/schema/tables/table[@name="'.$table.'"]/columns/column[@name="'.$field.'"]/type_variations/oci');
                    foreach ($nodes as $node) {
                        if (strtoupper($node->nodeValue) == 'CLOB') {
                            return TRUE;
                        }
                    }

                    // we couldn't find anything
                    return FALSE;
                }
            }
        }

        // could not find the field! Throw an exception.
        return FALSE;
        //throw new Exception('Could not determine if '.$table.'.'.$field.' requires a LOB. The field does not exist in the table definition.');

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
        $doc->loadXML(file_get_contents($fileName));
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
                    $id = $child->getAttribute('id');
                    self::addQuery($systemName, $child);
                    if (self::_queryRequiresRollback($systemName, $id)) {
                        $rollback_child = self::_createRollbackXML($systemName, $child);
                        $rollback_child->setAttribute('id', $id.'.rb');
                        self::addQuery($systemName, $rollback_child);
                    }
                }
            }//end if
        }//end foreach childnodes

    }//end processQueriesFile()


    /**
     * Creates rollback XML for a query that requires a rollback version to be
     * created.
     *
     * @param string  $systemName The system to add queries to.
     * @param DOMNode $queryNode  The DOM node containing the
     *
     * @since  MySource 3.18.0 RC1
     * @return DOMNode
     */
    protected static function _createRollbackXML($systemName, DOMNode $queryNode)
    {
        $doc = new DOMDocument();
        $queryNode = $doc->importNode($queryNode, TRUE);
        $doc->appendChild($queryNode);
        $xpath = new DOMXPath($doc);

        $whereNodes = $xpath->query('//query//select/where');
        if (count($whereNodes) == 0)
        {
            $whereNode = NULL;
        } else {
            $whereNode = $whereNodes->item(0);
        }

        $nodes = $xpath->query('//query//select/from/table');
        foreach ($nodes as $node) {
            $table = $node->nodeValue;

            if (self::_tableRequiresRollback($table)) {
                // TODO: replace "from" tables with 'sq_rb_'

                // Add rollback where clause
                $fieldNodes = $xpath->query('//query//select/fields/field');
                foreach ($fieldNodes as $fieldNode) {
                    $table = $fieldNode->getAttribute('table');
                    if (self::_tableRequiresRollback($table)) {
                        $rbTableName = substr_replace($table, 'sq_rb_', 0, 3);
                        $fieldNode->setAttribute('table', $rbTableName);
                    }
                }

                // Replace where attributes
                $condNodes = $xpath->query('//query//select/where//*');
                foreach ($condNodes as $condNode) {
                    $table = $condNode->getAttribute('table');
                    if (self::_tableRequiresRollback($table)) {
                        $rbTableName = substr_replace($table, 'sq_rb_', 0, 3);
                        $condNode->setAttribute('table', $rbTableName);
                    }
                }

                // Replace join attributes
                $condNodes = $xpath->query('//query//select/joins/join/field');
                foreach ($condNodes as $condNode) {
                    $table = $condNode->getAttribute('table');
                    if (self::_tableRequiresRollback($table)) {
                        $rbTableName = substr_replace($table, 'sq_rb_', 0, 3);
                        $condNode->setAttribute('table', $rbTableName);
                    }
                }
                
				// Replace order-by attributes
                $orderByNodes = $xpath->query('//query//select/order-by//*');
                foreach ($orderByNodes as $orderByNode) {
                    $table = $orderByNode->getAttribute('table');
                    if (self::_tableRequiresRollback($table)) {
                        $rbTableName = substr_replace($table, 'sq_rb_', 0, 3);
                        $orderByNode->setAttribute('table', $rbTableName);
                    }
                }
				
				// Replace group-by attributes
                $groupByNodes = $xpath->query('//query//select/group-by//*');
                foreach ($groupByNodes as $groupByNode) {
                    $table = $groupByNode->getAttribute('table');
                    if (self::_tableRequiresRollback($table)) {
                        $rbTableName = substr_replace($table, 'sq_rb_', 0, 3);
                        $groupByNode->setAttribute('table', $rbTableName);
                    }
                }
            }
        }

        // Add rollback where queries, no matter where they are found
        $fromNodes = $xpath->query('//query//select/from/table');
		$count = 0;
        foreach ($fromNodes as $fromNode) {
            $table = trim($fromNode->nodeValue);
            $rollbackTable = substr_replace($table, 'sq_rb_', 0, 3);

            if (self::_tableRequiresRollback($table)) {
                $fromNode->nodeValue = substr_replace($table, 'sq_rb_', 0, 3);

                // Go back to the table's select node, find its where node
                $selectNode = $fromNode->parentNode->parentNode;
                $whereNode = NULL;
                foreach ($selectNode->childNodes as $childNode) {
                    if ($childNode->nodeName == 'where') {
                        $whereNode = $childNode;
                        break;
                    }
                }

                // If no where node exists, create it
                if (is_null($whereNode)) {
                    $whereNode = $doc->createElement('where');
                    $selectNode->appendChild($whereNode);
                }

                $lessNode    = $doc->createElement('equal-or-less', ':rbtime'.$count);
				$count++;
                $lessNode->setAttribute('table', $rollbackTable);
                $lessNode->setAttribute('column', 'sq_eff_from');
                $whereNode->appendChild($lessNode);

                $orNode      = $doc->createElement('or');
                $whereNode->appendChild($orNode);

                $greaterNode = $doc->createElement('equal-or-greater', ':rbtime'.$count);
				$count++;
                $greaterNode->setAttribute('table', $rollbackTable);
                $greaterNode->setAttribute('column', 'sq_eff_to');
                $orNode->appendChild($greaterNode);

                $nullNode    = $doc->createElement('is-null');
                $nullFieldNode = $doc->createElement('field');
                $nullFieldNode->setAttribute('table', $rollbackTable);
                $nullFieldNode->setAttribute('column', 'sq_eff_to');
                $nullNode->appendChild($nullFieldNode);
                $orNode->appendChild($nullNode);
            }
        }

        return $queryNode;

    }//end _createRollbackXML()


}
?>
