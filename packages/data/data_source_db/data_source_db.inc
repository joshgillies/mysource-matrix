<?php
/**
* +--------------------------------------------------------------------+
* | This MySource Matrix Module file is Copyright (c) Squiz Pty Ltd    |
* | ACN 084 670 600                                                    |
* +--------------------------------------------------------------------+
* | IMPORTANT: This Module is not available under an open source       |
* | license and consequently distribution of this and any other files  |
* | that comprise this Module is prohibited. You may only use this     |
* | Module if you have the written consent of Squiz.                   |
* +--------------------------------------------------------------------+
*
* $Id: data_source_db.inc,v 1.24 2007/06/25 01:38:34 lwright Exp $
*
*/

check_ssv_licence();
require_once SQ_CORE_PACKAGE_PATH.'/../include/asset.inc';
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
* @version $Revision: 1.24 $
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
	function Data_Source_DB($assetid=0)
	{
		$this->Data_Source($assetid);

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
			$conn =& $this->connectToDB();
			foreach ($tables as $table) {
				$query = 'select * from '.$table;
				$result = $conn->getAll($query);
				$assert_result = assert_valid_db_result($result, 'Query execution failed : '.$sql, FALSE, FALSE);
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

		$query_string = $this->_getExecutableQueryString();
		if (empty($query_string)) return $result;

		// check the cache
		$hash = md5($query_string);
		$result = $this->getCachedResultSet($hash);
		if ($result !== FALSE) return $result;

		if ($this->attr('mode') != 'execute') return $result;

		if (isset($this->_tmp[$hash])) {
			$result = unserialize($this->_tmp[$hash]);
			return $result;
		}

		$result =& $this->_executeQuery($query_string);
		$this->cacheResultSet($result, $query_string);
		$this->_tmp[$hash] = serialize($result);

		return $result;

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
		$query_string = @$this->_tmp['query_string'];
		if (!is_null($query_string)) return $query_string;

		$sql = $this->attr('sql');

		$keyword_wrapper = '%%';
		$keyword_pattern = '('.$keyword_wrapper.'([a-zA-Z_\-0-9\.]+)'.$keyword_wrapper.')';

		// insert the dynamic parameters into the query
		preg_match_all ('/'.$keyword_pattern.'/', $sql, $matches, PREG_PATTERN_ORDER);
		if (empty($matches[1])) {
			$this->_tmp['query_string'] = $sql;
			return $this->_tmp['query_string'];
		}

		$raw_keywords =& $matches[1];
		$keywords =& $matches[2];

		$DB =& $this->connectToDB();
		if ($DB === FALSE) return $sql;

		foreach ($keywords as $keyword) {
			$value = $this->getDynamicVariableValue($keyword);
			$replacements[] = $DB->escapeSimple($value);
			$patterns[] = '/('.$keyword_wrapper.$keyword.$keyword_wrapper.')/';
		}

		$sql = preg_replace($patterns, $replacements, $sql);

		$this->_tmp['query_string'] = $sql;
		return $this->_tmp['query_string'];

	}//end _getExecutableQueryString()


	/**
	* Get a value of a dynamic variable
	*
	* @param string	$variable_name	Name of variable
	*
	* @return string
	* @access public
	*/
	function getDynamicVariableValue($variable_name)
	{
		$dynamic_vars = $this->attr('dynamic_vars');
		if (!isset($dynamic_vars[$variable_name])) return '';

		$parameter_map =& $this->getAttribute('parameter_map');
		$value = $parameter_map->getParameterValue($variable_name);

		if (empty($value)) {
			$value = array_get_index($dynamic_vars, $variable_name, '');
		}

		return $value;

	}//end getDynamicVariableValue()


	/**
	* Function that executes the query and assigns the result set to the result attribute
	*
	* @param string	$sql	the sql query to be executed
	*
	* @return array
	* @access public
	*/
	function &_executeQuery($sql)
	{
		$result = Array();

		$DB =& $this->connectToDB();
		if ($DB === FALSE) return $result;
		$sql = trim($sql, ';');
		$db_result =& $DB->query($sql);
		$assert_result = assert_valid_db_result($db_result, 'Query execution failed : '.$sql, FALSE, FALSE);
		if (!$assert_result) return $result;

		if ($db_result === DB_OK) return $result;

		while ($row =& $db_result->fetchRow()) {
			$result[] = $row;
		}

		$db_result->free();

		return $result;

	}//end _executeQuery()


	/**
	* Function to connect to the db and return the db object
	*
	* @return object
	* @access public
	*/
	function &connectToDB()
	{
		$false = FALSE;

		if (isset($this->_tmp['db_connection'])) {
			return $this->_tmp['db_connection'];
		}

		$dsn = $this->getDSN();
		if (empty($dsn)) return $false;

		$conn =& DB::connect($dsn);
		if (DB::isError($conn)) {
			trigger_error($conn->message, E_USER_WARNING);
			return $false;
		}

		$conn->setFetchMode(DB_FETCHMODE_ASSOC);
		$this->_tmp['db_connection'] =& $conn;

		return $this->_tmp['db_connection'];

	}//end connectToDB()


	/**
	* Get the DSN for db connection
	*
	* @return string
	* @access public
	*/
	function getDSN()
	{
		if (isset($this->_tmp['db_DSN'])) {
			return $this->_tmp['db_DSN'];
		}

		$dsn = '';

		$connector_asset =& $this->_getConnectorAsset();
		if (!empty($connector_asset)) {
			$dsn = $connector_asset->getDSN();
		}

		if (empty($dsn)) $dsn = $this->attr('dsn');

		$this->_tmp['db_DSN'] = $dsn;

		return $dsn;

	}//end getDSN()


	/**
	* Get the connector asset
	*
	* @return object
	* @access public
	*/
	function &_getConnectorAsset()
	{
		$connector_asset = NULL;

		$connector_link = $GLOBALS['SQ_SYSTEM']->am->getLink($this->id, SQ_LINK_NOTICE, 'db_connector', FALSE, 'db_connector');
		$connector_asset_id = $connector_link['minorid'];

		if (!empty($connector_asset_id)) {
			$connector_asset =& $GLOBALS['SQ_SYSTEM']->am->getAsset($connector_asset_id);
		}

		return $connector_asset;

	}//end _getConnectorAsset()


}//end class

?>
