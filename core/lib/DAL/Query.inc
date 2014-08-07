<?php
/**
 * The Query Object used to store database objects.
 *
 * @version	4.0.0
 * @package	MySource4
 * @subpackage DAL
 * @author	 Squiz Pty Ltd <mysource4@squiz.net>
 * @copyright  2006-2007 Squiz Pty Ltd (ABN 77 084 670 600)
 * @license	http://matrix.squiz.net/licence Squiz.Net Open Source Licence
 */

require_once dirname(__FILE__).'/DAL.inc';

/**
 * Query.
 *
 * Object that represents a query in the MySource 4 system.
 *
 * @since 4.0.0
 */
class Query
{

	/**
	 * OCI8: LOB stack.
	 *
	 * Maintains a list of OCI8 LOB resources that we need to release after a
	 * DAL query is run.
	 * The only reason why this is public is because we need to add to it from
	 * the ovened queries..
	 *
	 * @var   array $lobStack
	 * @since 4.0.0
	 */
	public $_lobStack  = Array();


	/**
	 * The ID of this query.
	 *
	 * @var   string $_queryid Full ID of the query.
	 * @since 4.0.0
	 */
	private $_queryid = '';

	/**
	 * Total number of bindings in this query.
	 *
	 * @var   integer $_totalBindings Number of bindings.
	 * @since 4.0.0
	 */
	private $_totalBindings = 0;

	/**
	 * The representation of this query as an array.
	 *
	 * @var   array $_arrayRepresentation The multidimensional structure of the
	 *									array.
	 * @since 4.0.0
	 */
	private $_arrayRepresentation = array();

	/**
	 * The SQL of the query.
	 *
	 * This is the initial conversion. If a complex merge is required, then the
	 * SQL string will need to be regenerated.
	 *
	 * @var   string $_sqlString The SQL of the query.
	 * @since 4.0.0
	 */
	private $_sqlString = '';

	/**
	 * List of bindings.
	 *
	 * @var   array $_bindings Array of bind variables in this query.
	 * @since 4.0.0
	 */
	private $_bindings = array();

	/**
	 * List of keywords.
	 *
	 * Keywords will be replaced just before query prepare.
	 *
	 * @var   array $_keywords Array of keywords in this query.
	 * @since 4.0.0
	 */
	private $_keywords = array();

	/**
	 * Query execution method.
	 *
	 * Query will be executed using the given method (i.e. executeAll).
	 *
	 * @var   string $_execMethod
	 * @since 4.0.0
	 */
	private $_execMethod = NULL;

	/**
	 * Bind map.
	 *
	 * @var   string $_bindMap
	 * @since 4.0.0
	 */
	private $_bindMap = array();


	/**
	 * Creates a new query object with the data provided.
	 *
	 * @param string $queryid			 The ID of the query to bake out.
	 * @param array  $arrayRepresentation The structured array representation
	 *									of the query.
	 * @param string $sqlString		   The SQL of the query.
	 *
	 * @since  4.0.0
	 * @throws DALException If query represantation is invalid.
	 */
	public function __construct($queryid='', array $arrayRepresentation=array(), $sqlString='')
	{
		$this->_queryid = $queryid;

		if (isset($arrayRepresentation['query']) === FALSE) {
			throw new DALException('Invalid array representation');
		}

		$this->_arrayRepresentation = $arrayRepresentation;
		$this->_sqlString		   = $sqlString;

	}//end __construct()


	/**
	 * Returns the type of execution this method has specified.
	 *
	 * @since  4.0.0
	 * @return string
	 */
	public function getQueryType()
	{
		return $this->_arrayRepresentation['query_type'];

	}//end getQueryType()


	/**
	 * Returns the array representation.
	 *
	 * @since  4.0.0
	 * @return string
	 */
	public function getArrayRepresentation()
	{
		return $this->_arrayRepresentation;

	}//end getArrayRepresentation()


	/**
	 * Merge a query into this one.
	 *
	 * @param string $bindName The bind variable the query will replace.
	 * @param Query  $query	The query to merge as part of this one.
	 *
	 * @since  4.0.0
	 * @return boolean
	 */
	public function merge($bindName, Query $query)
	{
		$subQuery	  = clone $query;
		$subQueryArray = $subQuery->getQueryArray();

		// Merge the passed query into the appropriate part of ours.
		// We need to determine if the merging these two queries is simple or complex.
		if ($subQuery->getQueryType() === 'with') {
			$afterPart = $subQueryArray['WITH']['AFTER'];

			// Add this query to after WITH clause.
			$subQueryArray['WITH']['AFTER'] = $this->getQueryArray();


			$this->_arrayRepresentation['query'] = $subQueryArray;
			$this->_findAndReplacePlaceHolder($bindName, $afterPart);
			$this->_arrayRepresentation['query_type'] = 'with';
			$this->_addBindingsFromQuery($subQuery);
		} else {
			$this->_addBindingsFromQuery($subQuery);
			$this->_findAndReplacePlaceHolder($bindName, $subQueryArray);
		}

		return TRUE;

	}//end merge()


	/**
	 * Add bindings from specified query to this query.
	 *
	 * @param Query $subQuery Query object.
	 *
	 * @since  4.0.0
	 * @return void
	 */
	private function _addBindingsFromQuery(Query $subQuery)
	{
		$bindings = $subQuery->getBindings();
		foreach ($bindings as $binding) {
			$newName = $this->_getNextValidBindName();
			$subQuery->_findAndReplacePlaceHolder($binding['name'], $newName);
			$this->_addBindVar($newName, $binding['value'], $binding['type'], $binding['name']);
		}

	}//end _addBindingsFromQuery()


	/**
	 * Returns a valid name for a new bind variable for this query.
	 *
	 * @since  4.0.0
	 * @return string
	 */
	private function _getNextValidBindName()
	{
		$this->_totalBindings++;
		$bindName = ':bind_'.$this->_totalBindings;
		return $bindName;

	}//end _getNextValidBindName()


	/**
	 * Add a bind variable to this query.
	 *
	 * @param string  $name	 Name of the new bind variable.
	 * @param mixed   $value	Value of the bind variable.
	 * @param integer $type	 Data type of the bind variable.
	 * @param string  $realName Real name of the bind var.
	 *
	 * @since  4.0.0
	 * @return void
	 */
	private function _addBindVar($name, $value, $type, $realName)
	{
		$this->_bindings[] = array(
							  'name'  => $name,
							  'value' => $value,
							  'type'  => $type,
							 );

		$this->_bindMap[] = array(
							 $realName => $name,
							);

	}//end _addBindVar()


	/**
	 * Remove bind variables from end of binds array.
	 *
	 * @param string $count Number of bind variables to remove.
	 *
	 * @since  4.0.0
	 * @return void
	 */
	private function _removeLastBindVar($count=1)
	{
		for ($i = 0; $i < $count; $i++) {
			array_pop($this->_bindings);
			array_pop($this->_bindMap);
			$this->_totalBindings--;
		}

	}//end _removeLastBindVar()


	/**
	 * Replaces the given bind varibale name with a new name in SQL string.
	 *
	 * @param string $oldName Old name of the bind variable.
	 * @param string $newName New name of the bind variable.
	 *
	 * @since  4.0.0
	 * @return void
	 */
	public function replaceBindVarName($oldName, $newName)
	{
		 $this->_sqlString = str_replace($oldName, $newName, $this->_sqlString);

	}//end replaceBindVarName()


	/**
	 * Returns this query's id.
	 *
	 * @since  4.0.0
	 * @return string
	 */
	public function getQueryid()
	{
		return $this->_queryid;

	}//end getQueryid()


	/**
	 * Returns list of bind variables.
	 *
	 * @since  4.0.0
	 * @return array
	 */
	public function getBindings()
	{
		return $this->_bindings;

	}//end getBindings()


	/**
	 * Returns the array representaion of this Query.
	 *
	 * @since  4.0.0
	 * @return array
	 */
	public function getQueryArray()
	{
		return $this->_arrayRepresentation['query'];

	}//end getQueryArray()


	/**
	 * Returns the SQL string representaion of this Query.
	 *
	 * @param boolean $convert If TRUE will perform conversion of the query into
	 *						 SQL before returning, otherwise the SQL is
	 *						 returned in its' current form.
	 *
	 * @since  4.0.0
	 * @return array
	 */
	public function getSql($convert=FALSE)
	{
		if ($convert === TRUE || $this->_sqlString === '') {
			 // TODO: Should not include DALBaker here.
			 require_once SQ_LIB_PATH.'/DAL/DALBaker.inc';
			 $converter        = DALBaker::getConverter();
			 $this->_sqlString = $converter->convertToSql($this->_arrayRepresentation['query']);
		}

		return $this->_sqlString;

	}//end getSql()


	/**
	 * Prepare the query for execution and returns the PDOStatement.
	 *
	 * @since  4.0.0
	 * @return PDOStatement
	 */
	public function prepare()
	{
		$sql = $this->getSql();

		// Replace keywords.
		foreach ($this->_keywords as $keyword => $value) {
			$sql = str_replace('['.$keyword.']', $value, $sql);
		}

		// Update sqlString..
		$this->_sqlString = $sql;

		// Prepare PDO statement.
		$dbh   = DAL::getDb();

		if (DAL::getDbType() === 'oci') {

			$query = oci_parse($dbh, $sql);
			// Add bindings
			foreach ($this->_bindings as $binding) {
				// TODO: Handle insert/update of CLOB fields - this needs to be determined at bake time
				if ($binding['type'] === PDO::PARAM_LOB) {
					oci_bind_by_name($query, $binding['name'], $binding['value'], -1, SQLT_CLOB);

					// Add to LOB stack so we can release it later
					$this->_lobStack[] = $binding['value'];
				} else {
					oci_bind_by_name($query, $binding['name'], $binding['value'], -1);
				}
			}

		} else {

			$query = $dbh->prepare($sql);
			// Add bindings to PDO.
			foreach ($this->_bindings as $binding) {
				// $query->bindValue($binding['name'], $binding['value'], $binding['type']);
				$query->bindValue($binding['name'], $binding['value']);
			}

		}

		return $query;

	}//end prepare()


	/**
	 * Set the execution method.
	 *
	 * For example: executeAssoc.
	 *
	 * @param string $method The method to set for the query.
	 *
	 * @since  4.0.0
	 * @return void
	 */
	public function setExecMethod($method)
	{
		$this->_execMethod = $method;

	}//end setExecMethod()


	/**
	 * Returns the execution method.
	 *
	 * For example: executeAssoc
	 *
	 * @since  4.0.0
	 * @return string
	 */
	public function getExecMethod()
	{
		return $this->_execMethod;

	}//end getExecMethod()


	/**
	 * Add binding to the query.
	 *
	 * If value is an array then use arraySplit.
	 * If value is another Query then use merge.
	 *
	 * @param string  $name	 The name of the bind variable.
	 * @param mixed   $value	The value to set the variable to.
	 * @param integer $dataType The type to bind the value with.
	 *
	 * @since  4.0.0
	 * @return void
	 */
	public function bind($name, $value, $dataType=NULL)
	{
		// Depending on the data type replace, merge, split etc.
		if (($value instanceof Query) === TRUE) {
			$this->merge($name, $value);
		} else if (is_array($value) === TRUE) {
			$bindNames = array();
			foreach ($value as $val) {
				$newBindName = $this->_getNextValidBindName();
				$this->_addBindVar($newBindName, $val, $dataType, $name);
				$bindNames[] = $newBindName;
			}

			if ($this->_findAndReplacePlaceHolder($name, implode(',', $bindNames)) === FALSE) {
				$this->_removeLastBindVar(count($bindNames));
			}
		} else {
			$newBindName = $this->_getNextValidBindName();
			$this->_addBindVar($newBindName, $value, $dataType, $name);

			// Find the bind place holder in the query array and replace it.
			if ($this->_findAndReplacePlaceHolder($name, $newBindName) === FALSE) {
				$this->_removeLastBindVar();
			}

		}//end if

	}//end bind()


	/**
	 * Replaces the place holder with given value.
	 * If the value is empty, it's forced to be an empty string so the
	 * query generated doesn't become invalid.
	 *
	 * @param string $placeHolder Name of the place holder.
	 * @param mixed  $value	   Value that will replace the placeHolder.
	 *
	 * @return boolean
	 */
	private function _findAndReplacePlaceHolder($placeHolder, $value)
	{

		// If it's an empty value, force it to be an empty string.
		if (empty($value) === TRUE) {
			$value = "''";
		}

		$count = 0;
		$sql   = $this->getSql();
		$sql   = preg_replace('/'.$placeHolder.'([^a-zA-Z0-9]|$)/', $value.'$1', $sql, 1, $count);

		$this->_sqlString = $sql;

		if ($count === 0) {
			return FALSE;
		}

		return TRUE;

	}//end _findAndReplacePlaceHolder()


	/**
	 * Adds sub queries (hooks) to the current query.
	 *
	 * @param string $subQueryid The sub hookid to merge.
	 * @param array  $queries	The queries to add.
	 *
	 * @since  4.0.0
	 * @return void
	 */
	public function subQueries($subQueryid, array $queries)
	{
		foreach ($queries as $query) {
			if (($query instanceof Query) === TRUE) {
				$query->getSql();
				$this->merge('HOOKID:'.$subQueryid, $query);
			} else {
				$this->bind('HOOKID:'.$subQueryid, $query, PDO::PARAM_STR);
			}
		}

	}//end subQueries()


	/**
	 * Add keywords to the query.
	 *
	 * array('keyword' => value)
	 *
	 * @param array $keywords Array of keywords to add.
	 *
	 * @since  4.0.0
	 * @return void
	 */
	public function keywords(array $keywords)
	{
		$this->_keywords = array_merge($this->_keywords, $keywords);

	}//end keywords()


	/**
	 * Releases any OCI8 CLOB objects that may have been created here
	 *
	 * @since  4.0.0
	 * @return array
	 */
	public function releaseLobs()
	{
		// OCI8:
		if (DAL::getDBType() === 'oci') {
			foreach($this->_lobStack as $key => $lob) {
				$lob->close();
				oci_free_descriptor($lob);
				unset($this->_lobStack[$key]);
			}
		}

	}//end releaseLobs()



}//end class


?>
