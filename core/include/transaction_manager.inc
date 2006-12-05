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
* $Id: transaction_manager.inc,v 1.22 2006/12/05 05:36:14 bcaldwell Exp $
*
*/


/**
* Transaction_Manager
*
* Purpose
*    Handle transactions for both the DB and the file system
*    Also to handle nested transaction calls
*
* @author  Greg Sherwood <greg@squiz.net>
* @version $Revision: 1.22 $
* @package MySource_Matrix
*/
class Transaction_Manager extends MySource_Object
{

	/**
	* Lets us know if we have rolled back
	*
	* @var Array(boolean)
	*/
	var $_rollback = Array();

	/**
	* How many levels of transactions we are deep for each db connection
	*
	* @var Array(int)
	*/
	var $_levels = Array();


	/**
	* Constructor
	*
	*/
	function Transaction_Manager()
	{
		$this->MySource_Object();

	}//end constructor


	/**
	* Returns whether we are in a transaction or not
	*
	* @param object	&$db	the database object to check
	*
	* @return boolean
	* @access public
	*/
	function inTransaction(&$db)
	{
		return !empty($this->_levels[$this->_getId($db)]);

	}//end inTransaction()


	/**
	* Begins a transaction
	*
	* Only does a real begin if this is the first begin
	*
	* @param object	&$db	the database object on which to begin the transaction
	*
	* @return boolean
	* @access public
	*/
	function begin(&$db)
	{
		$db_name = $this->_getId($db);

		if (isset($this->_levels[$db_name])) {
			$this->_levels[$db_name]++;
		} else {
			$this->_levels[$db_name] = 1;
		}

		if ($this->_levels[$db_name] == 1) {
			if ($GLOBALS['SQ_SYSTEM']->db->phptype != 'oci8') {
				// first time we have started a transaction
				$result = $db->query('BEGIN');
				assert_valid_db_result($result);
			}
		}
		return true;

	}//end begin()


	/**
	* Commits a transaction
	*
	* Only does a real commit if this is the last commit
	*
	* @param object	&$db	the database object on which to perform the commit
	*
	* @return boolean
	* @access public
	*/
	function commit(&$db)
	{
		$db_name = $this->_getId($db);

		if (empty($this->_levels[$db_name])) {
			trigger_localised_error('SYS0219', E_USER_WARNING);
			return false;
		}

		if (!empty($this->_rollback[$db_name])) {
			trigger_localised_error('SYS0220', E_USER_WARNING);
			$this->rollback($db_name);
			return false;
		}
		$this->_levels[$db_name]--;

		if ($this->_levels[$db_name] == 0) {
			// this is the last commit
			$result = $db->commit();
			assert_valid_db_result($result);
		}
		return true;

	}//end commit()


	/**
	* Rolls back all open transactions
	*
	* Only does a real rollback if this is the last rollback
	*
	* @param object	&$db	the database object on which to perform the rollback
	*
	* @return boolean
	* @access public
	*/
	function rollback(&$db)
	{
		$db_name = $this->_getId($db);

		if (empty($this->_levels[$db_name])) {
			trigger_localised_error('SYS0231', E_USER_WARNING);
			return false;
		}

		// set the rollback flag
		$this->_rollback[$db_name] = true;
		$this->_levels[$db_name]--;

		if ($this->_levels[$db_name] == 0) {

			// this is the last rollback
			$result = $GLOBALS['SQ_SYSTEM']->db->rollback();
			assert_valid_db_result($result);

			// clear the rollback flag as there is no more transaction block
			$this->_rollback[$db_name] = false;
		}
		return true;

	}//end rollback()


	/**
	* Returns the string that identifies the passed db object
	*
	* @param object &$db	the database object to return the id string for
	*
	* @return string
	* @access private
	*/
	function _getId(&$db)
	{
		assert_not_null($db);
		return (string) $db->connection;

	}//end getConnectionId()


}//end class
?>
