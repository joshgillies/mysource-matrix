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
* $Id: data_source_db.inc,v 1.13 2005/12/21 21:58:35 rong Exp $
*
*/



/**
* Data_Source_DB
*
* Purpose
*
*
* @author  Greg Sherwoood <greg@squiz.net>
* @version $Revision: 1.13 $
* @package MySource_Matrix_Packages
* @subpackage __core__
*/
	require_once SQ_CORE_PACKAGE_PATH.'/../include/asset.inc';
	require_once SQ_CORE_PACKAGE_PATH.'/data_source/data_source/data_source.inc';
	require_once SQ_LIB_PATH.'/html_form/html_form.inc';
	require_once SQ_CORE_PACKAGE_PATH.'/files/image/image.inc';

class Data_Source_DB extends Data_Source
{


	/**
	* Constructor
	*
	* @param int	$assetid		assetid for the asset
	*
	*/
	function Data_Source_DB($assetid=0)
	{
		$this->Data_Source($assetid);

	}//end constructor


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
			$conn =& $this->connectToDB();
			foreach ($tables as $table) {
				$query = 'select * from '.$table;
				$result = $conn->getAll($query);
				$assert_result = assert_valid_db_result($result, 'Query execution failed : '.$sql, false, false);
				if (!$assert_result) {
					$empty = Array();
					return $empty;
				}
				if (!empty($result)) {
					$record = $result(0);
					foreach ($record as $key => $value) {
						$field[] = $key;
					}
				}
			}
		}

	}//end getFieldNamesFromRecordSet()


	/**
	* Returns an array of the result of query execution
	*
	* @return array
	* @access public
	*/
	function &getResultSet()
	{
		$result = Array();
		$sql = $this->attr('sql');
		$sql = str_replace('<br />', ' ', $sql);
		//if the query is already excuted then we get results from cache otherwise we execute the
		//query (only if in execute mode or the cache manager returns aan empty string instead of an empty array)
		$result = parent::getResultSet($this->attr('sql'));
		if (empty($result)) {
			if (isset($this->_tmp[$sql])) {
				$result = $this->_tmp[$sql];
			}
			// use the sql query as the cache key of this cache entry
			if (!empty($result)) $this->setResultSet($result, $this->attr('sql'));
		}
		$mode = $this->attr('mode');
		//if ((empty($result) && !is_array($result)) || ($mode == 'execute')) {
		if ($mode == 'execute') {
			if (!empty($sql)) {
				$result = &$this->_executeQuery($sql);
				$this->setResultSet($result, $this->attr('sql'));
				$this->_tmp[$sql] = $result;
			}
		}
		return $result;

	}//end getResultSet()


	/**
	* Function that executes the query and assigns the result set to the result attribute
	*
	* @param string	$sql	the sql query to be executed
	*
	* @return boolean
	* @access public
	*/
	function &_executeQuery($sql)
	{
		$result = Array();
		$dsn = $this->attr('dsn');
		if (!empty($dsn)) {
			$conn = $this->connectToDB();
			$result = &$conn->getAll($sql);
			$assert_result = assert_valid_db_result($result, 'Query execution failed : '.$sql, false, false);
			if(!$assert_result) {
				$empty = Array();
				return $empty;
			}
		}
		return $result;

	}//end _executeQuery()


	/**
	* Function to connect to the db and return the db object
	*
	* @return object
	* @access public
	*/
	function connectToDB()
	{
		$dsn = $this->attr('dsn');
		if (!empty($dsn)) {
			$conn = DB::connect($dsn);
			if (DB::isError($conn)) {
				trigger_error($conn->message, E_USER_ERROR);
				return false;
			}
			$conn->setFetchMode(DB_FETCHMODE_ASSOC);
			return $conn;
		}

	}//end connectToDB()


}//end class

?>
