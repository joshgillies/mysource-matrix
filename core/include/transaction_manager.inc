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
* $Id: transaction_manager.inc,v 1.30 2012/08/30 01:09:20 ewang Exp $
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
* @version $Revision: 1.30 $
* @package MySource_Matrix
*/
class Transaction_Manager extends MySource_Object
{

	/**
	* Lets us know if we have rolled back
	*
	* @var Array(boolean)
	*/
	private $_rollback = Array();

	/**
	* How many levels of transactions we are deep for each db connection
	*
	* @var Array(int)
	*/
	private $_levels = Array();


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
	public function inTransaction($db_name)
	{
		return !empty($this->_levels[$db_name]);

	}//end inTransaction()


	/**
	* Begins a transaction
	*
	* Only does a real begin if this is the first begin
	*
	* @param string	$db_name	the DAL connection pool identifier
	*
	* @return boolean
	* @access public
	*/
	public function begin($db_name)
	{
		$dsn_array = MatrixDAL::getDsn($db_name);
		$dsn = $dsn_array['dsn'];
		if (isset($dsn_array['userName'])) $dsn .= ';user='.$dsn_array['userName'];

		if (isset($this->_levels[$dsn])) {
			$this->_levels[$dsn]++;
		} else {
			$this->_levels[$dsn] = 1;
		}

		if ($this->_levels[$dsn] == 1) {
			try {
				MatrixDAL::beginTransaction();
			} catch (DALException $e) {
				throw $e;
			}
		}

		return TRUE;

	}//end begin()


	/**
	* Commits a transaction
	*
	* Only does a real commit if this is the last commit
	*
	* @param string	$db_name	the DAL connection pool identifier
	*
	* @return boolean
	* @access public
	*/
	public function commit($db_name)
	{
		$dsn_array = MatrixDAL::getDsn($db_name);
		$dsn = $dsn_array['dsn'];
		if (isset($dsn_array['userName'])) $dsn .= ';user='.$dsn_array['userName'];

		if (empty($this->_levels[$dsn])) {
			// TODO: -TOF- put that line back
			//trigger_localised_error('SYS0219', translate('Unable to Commit, No Transaction is Open'), E_USER_WARNING)
			return FALSE;
		}

		if (!empty($this->_rollback[$dsn])) {
			trigger_localised_error('SYS0220', translate('Unable to Commit, Transaction has already been aborted'), E_USER_WARNING)
			try {
				MatrixDAL::rollback();
			} catch (DALException $e) {
				throw $e;
			}

			return FALSE;
		}
		$this->_levels[$dsn]--;

		if ($this->_levels[$dsn] == 0) {
			// this is the last commit
			try {
				MatrixDAL::commit();
			} catch (DALException $e) {
				throw $e;
			}
		}

		return TRUE;

	}//end commit()


	/**
	* Rolls back all open transactions
	*
	* Only does a real rollback if this is the last rollback
	*
	* @param string	$db_name	the DAL connection pool identifier
	*
	* @return boolean
	* @access public
	*/
	public function rollback($db_name)
	{
		$dsn_array = MatrixDAL::getDsn($db_name);
		$dsn = $dsn_array['dsn'];
		if (isset($dsn_array['userName'])) $dsn .= ';user='.$dsn_array['userName'];

		if (empty($this->_levels[$dsn])) {
			trigger_localised_error('SYS0231', translate('Unable to Rollback, No Transaction is Open'), E_USER_WARNING)
			return FALSE;
		}

		// set the rollback flag
		$this->_rollback[$dsn] = TRUE;
		$this->_levels[$dsn]--;

		if ($this->_levels[$dsn] == 0) {

			// this is the last rollback
			try {
				MatrixDAL::rollback();
			} catch (DALException $e) {
				throw $e;
			}

			// clear the rollback flag as there is no more transaction block
			$this->_rollback[$dsn] = FALSE;
		}

		return TRUE;

	}//end rollback()


}//end class
?>
