<?php
/**
* +--------------------------------------------------------------------+
* | This MySource Matrix CMS file is Copyright (c) Squiz Pty Ltd	   |
* | ACN 084 670 600													   |
* +--------------------------------------------------------------------+
* | IMPORTANT: Your use of this Software is subject to the terms of    |
* | the Licence provided in the file licence.txt. If you cannot find   |
* | this file please contact Squiz (www.squiz.com.au) so we may provide|
* | you a copy.														   |
* +--------------------------------------------------------------------+
*
* $Id: data_source_db.inc,v 1.53 2013/08/16 06:38:19 ewang Exp $
*
*/


require_once SQ_INCLUDE_PATH.'/asset.inc';
require_once SQ_CORE_PACKAGE_PATH.'/data_source/data_source/data_source.inc';
require_once SQ_LIB_PATH.'/html_form/html_form.inc';
require_once SQ_CORE_PACKAGE_PATH.'/files/image/image.inc';

/**
* Data_Source_DB
*
* Purpose
*
*
* @author  Greg Sherwoood <greg@squiz.net>
* @version $Revision: 1.53 $
* @package MySource_Matrix_Packages
* @subpackage __core__
*/
class Data_Source_DB extends Data_Source
{


	/**
	* Constructor
	*
	* @param int	$assetid		assetid for the asset
	*
	*/
	function __construct($assetid=0)
	{
		parent::__construct($assetid);

	}//end constructor


	/**
	* Returns an array of all the permitted links type, the type asset and the cardinality
	* In the form
	*
	*   Array('[link_type]' => Array('[type_code]' => Array('card' => [cardinality], 'exclusive' => [exclusive])));
	* Where:
	*   link_type   = SQ_LINK_TYPE_1|SQ_LINK_TYPE_2|SQ_LINK_TYPE_3|SQ_LINK_NOTICE
	*   cardinality = [max number of links]|'M'
	*   exclusive   = TRUE|FALSE
	*
	* @return array
	* @access private
	*/
	function _getAllowedLinks()
	{
		// any link is allowed
		$allowed_link['asset']['card'] = 'M';
		$allowed_link['asset']['exclusive'] = FALSE;

		$links[SQ_LINK_TYPE_1] = $allowed_link;
		$links[SQ_LINK_TYPE_2] = $allowed_link;
		$links[SQ_LINK_TYPE_3] = $allowed_link;
		$links[SQ_LINK_NOTICE] = $allowed_link;

		return $links;

	}//end _getAllowedLinks()


	/**
	* Returns an array of the column names of the result set
	*
	* @return array
	* @access public
	*/
	function getFieldNamesFromRecordSet()
	{
		$fields = $this->attr('fields');
		if ($fields != '*') {
			$fields = explode(', ', $fields);
			return $fields;
		} else {
			$tables = explode(', ', $this->attr('tables'));
			$fields = Array();
			$conn = $this->connectToDB();
			foreach ($tables as $table) {
				try {
					$table = preg_replace('/[^a-zA-Z_1-9\-\.]+/', '', $table);
					$sql = 'select * from '.$table;
					$result = MatrixDAL::executeSqlAssoc($sql);

					if (!empty($result)) {
						$record = $result(0);
						foreach ($record as $key => $value) {
							$field[] = $key;
						}
					}

				} catch (DALException $e) {
					trigger_localised_error('DATA0001', E_USER_WARNING, $e->getMessage(), $sql);
					$empty = Array();
					return $empty;
				}

			}//end foreach
		}//end if

	}//end getFieldNamesFromRecordSet()


	/**
	* Returns an array of the result of query execution
	*
	* @return array
	* @access public
	*/
	function &getResultSet()
	{
		$GLOBALS['SQ_SYSTEM']->pm->startTimer($this);
		$query_string = $this->_getExecutableQueryString();
		if (!empty($query_string)) {
			$hash = md5($query_string);

			// Check the local cache
			if (!isset($this->_tmp[$hash])) {

				// Check the system cache
				$result = $this->getCachedResultSet($hash);
				if ($result !== FALSE) {
					$this->_tmp[$hash] = $result;
				} else {
					// Not cached at all, so hit the DB
					$this->_tmp[$hash] = $this->_executeQuery($query_string);
					$this->cacheResultSet($this->_tmp[$hash], $hash);
				}
			}
			$GLOBALS['SQ_SYSTEM']->pm->stopTimer($this);
			return $this->_tmp[$hash];
		}

		$GLOBALS['SQ_SYSTEM']->pm->stopTimer($this);
		// Fall-through failure case
		$res = Array();
		return $res;

	}//end getResultSet()


	/**
	* Get an executable query string
	*
	* The string returned is ready to execute, with all the
	*
	* @return string
	* @access private
	*/
	function _getExecutableQueryString()
	{
		$sql = $this->attr('sql');

		$keyword_wrapper = '%%';
		$keyword_pattern = '('.$keyword_wrapper.'([a-zA-Z_\-0-9\.]+)'.$keyword_wrapper.')';

		// insert the dynamic parameters into the query
		preg_match_all ('/'.$keyword_pattern.'/', $sql, $matches, PREG_PATTERN_ORDER);
		if (empty($matches[1])) {
			return $sql;
		}

		$raw_keywords =& $matches[1];
		$keywords =& $matches[2];

		$db = $this->connectToDB();
		if ($db === FALSE) return $sql;

		foreach ($keywords as $keyword) {
			$value = $this->getDynamicVariableValue($keyword);
			$value = MatrixDAL::quote($value, array_get_index($this->getDSN(), 'type', NULL));
			$replacements[] = str_replace('$', '\$', $value);
			$patterns[] = '/('.$keyword_wrapper.$keyword.$keyword_wrapper.')/';
		}

		$sql = preg_replace($patterns, $replacements, $sql);

		return $sql;

	}//end _getExecutableQueryString()


	/**
	* Function that executes the query and assigns the result set to the result attribute
	*
	* @param string	$sql	the sql query to be executed
	*
	* @return array
	* @access public
	*/
	function _executeQuery($sql)
	{
		$db = $this->connectToDB();
		if ($db === FALSE) return Array();
		$sql = trim($sql, '; ');
		try {
			MatrixDAL::changeDb('data_source_db_'.$this->id);
			$empty_result = TRUE;
			if (MatrixDAL::getDbType() === 'oci' && preg_match('/^\s*(:?delete )|(:?insert )|(:?update )/msi', $sql)) {
				MatrixDAL::setOciCommitOnSuccess(TRUE);
				MatrixDAL::executeQueries($sql);
				MatrixDAL::setOciCommitOnSuccess(FALSE);
			} else {
				$result = MatrixDAL::executeSqlAssoc($sql);
				// if query returns a non-empty result but with empty rows, like when using 
				// delete, update and insert queries, then make this result empty altogether
				foreach($result as $row) {
					if (!empty($row)) {
						$empty_result = FALSE;
						break;
					}
				}
			}//end else
			MatrixDAL::restoreDb();
		} catch (Exception $e) {
			trigger_localised_error('DATA0001', E_USER_WARNING, $e->getMessage(), $sql);
			MatrixDAL::restoreDb();
			return Array();
		}
		
		return $empty_result ? Array() : $result;

	}//end _executeQuery()


	/**
	* Function to connect to the db and return the db object
	*
	* @return object
	* @access public
	*/
	function connectToDB()
	{
		if (isset($this->_tmp['db_connection'])) {
			return $this->_tmp['db_connection'];
		}

		$dsn = $this->getDSN();
		if (empty($dsn)) return FALSE;

		// If there is an "oci:" prefix, strip it and store this data in the DB type index
		$dsn_lower = strtolower($dsn['DSN']);
		if (substr($dsn_lower, 0, 4) == 'oci:') {
			$dsn['DSN'] = substr($dsn['DSN'], 4);
			$dsn['type'] = 'oci';

			// Also we may have arrived from the separate fields interface or manually entered a "dbname=" PDO_OCI-type DSN
			// In such cases we will be nice and strip the leading "dbname=" for a simple PHP OCI-friendly format
			$dsn_lower = strtolower($dsn['DSN']);
			if (substr($dsn_lower, 0, 7) == 'dbname=') {
				$dsn['DSN'] = substr($dsn['DSN'], 7);
			} 
		}

		try {
			$this->_tmp['db_connection'] = MatrixDAL::dbConnect($dsn, 'data_source_db_'.$this->id);
		} catch (Exception $e) {
			$this->_tmp['db_conection_exception'] = $e;			
			return FALSE;
		}

		
		return $this->_tmp['db_connection'];

	}//end connectToDB()


	/**
	* Get the DSN for db connection
	*
	* @return array
	* @access public
	*/
	function getDSN()
	{
		if (isset($this->_tmp['db_DSN'])) {
			return $this->_tmp['db_DSN'];
		}

		$dsn = '';

		$connector_asset = $this->_getConnectorAsset();
		if (!empty($connector_asset)) {
			$dsn = $connector_asset->getDSN();
		}

		if (empty($dsn)) {
			$dsn = $this->constructDSNArray($this);
		}

		$this->_tmp['db_DSN'] = $dsn;

		return $dsn;

	}//end getDSN()


	/**
	* Get the DSN array for a db connection
	*
	* @param Asset	$asset	The asset to source the DSN data from
	*
	* @return array
	* @access public
	*/
	function constructDSNArray($asset)
	{
		$dsn = $asset->attr('dsn');
		$user = $asset->attr('user');
		$password = $asset->attr('password');
		$type = $asset->attr('db_type');
		switch ($type) {
			case 'mysql':
				$type = 'mysql';
			break;
			case 'oracle':
			case 'oci':
				$type = 'oci';
			break;
			case 'pgsql':
				$type = 'pgsql';
			break;
			default:
				$type = 'pgsql';
			break;
		}
		
		// If there is an "oci:" prefix, strip it and store this data in the DB type index
		$dsn_lower = strtolower($dsn);
		if (substr($dsn_lower, 0, 4) == 'oci:') {
			$dsn = substr($dsn, 4);
			$type = 'oci';
			// Also we may have arrived from the separate fields interface or manually entered a "dbname=" PDO_OCI-type DSN
			// In such cases we will be nice and strip the leading "dbname=" for a simple PHP OCI-friendly format
			$dsn_lower = strtolower($dsn);
			if (substr($dsn_lower, 0, 7) == 'dbname=') {
				$dsn = substr($dsn, 7);
			} 
		}
				
		return Array('DSN' => $dsn, 'user' => $user, 'password' => $password, 'type' => $type);

	}//end constructDSNArray()


	/**
	* Get the connector asset
	*
	* @return object
	* @access public
	*/
	function _getConnectorAsset()
	{
		$connector_asset = NULL;

		$connector_link = $GLOBALS['SQ_SYSTEM']->am->getLink($this->id, SQ_LINK_NOTICE, 'db_connector', FALSE, 'db_connector');
		if (empty($connector_link)) {
			$connector_asset_id = NULL;
		} else {
			$connector_asset_id = $connector_link['minorid'];
		}

		if (!empty($connector_asset_id)) {
			$connector_asset = $GLOBALS['SQ_SYSTEM']->am->getAsset($connector_asset_id);
		}

		return $connector_asset;

	}//end _getConnectorAsset()


}//end class

?>
