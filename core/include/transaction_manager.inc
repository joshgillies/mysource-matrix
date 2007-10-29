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
* $Id: transaction_manager.inc,v 1.23 2007/10/29 22:23:54 lwright Exp $
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
* @version $Revision: 1.23 $
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
	public function inTransaction(&$db)
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
	public function begin($db_name)
	{
		$dsn_array = DAL::getDsn($db_name);
		$dsn = $dsn_array['dsn'];

		if (isset($this->_levels[$dsn])) {
			$this->_levels[$dsn]++;
		} else {
			$this->_levels[$dsn] = 1;
		}

		if ($this->_levels[$dsn] == 1) {
			//if ($dsn['type'] != 'oci8') {
				// first time we have started a transaction
				try {
					DAL::beginTransaction();
				} catch (DALException $e) {
					throw $e;
				}
				//assert_valid_db_result($result);
			//}
		}

		return TRUE;

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
	public function commit($db_name)
	{
		$dsn_array = DAL::getDsn($db_name);
		$dsn = $dsn_array['dsn'];

		if (empty($this->_levels[$dsn])) {
			trigger_localised_error('SYS0219', E_USER_WARNING);
			return FALSE;
		}

		if (!empty($this->_rollback[$dsn])) {
			trigger_localised_error('SYS0220', E_USER_WARNING);
			try {
				DAL::rollback();
			} catch (DALException $e) {
				throw $e;
			}

			return FALSE;
		}
		$this->_levels[$dsn]--;

		if ($this->_levels[$dsn] == 0) {
			// this is the last commit
			try {
				DAL::commit();
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
	* @param object	&$db	the database object on which to perform the rollback
	*
	* @return boolean
	* @access public
	*/
	public function rollback($db_name)
	{
		$dsn_array = DAL::getDsn($db_name);
		$dsn = $dsn_array['dsn'];

		if (empty($this->_levels[$dsn])) {
			trigger_localised_error('SYS0231', E_USER_WARNING);
			return false;
		}

		// set the rollback flag
		$this->_rollback[$dsn] = true;
		$this->_levels[$dsn]--;

		if ($this->_levels[$dsn] == 0) {

			// this is the last rollback
			try {
				DAL::rollback();
			} catch (DALException $e) {
				throw $e;
			}

			// clear the rollback flag as there is no more transaction block
			$this->_rollback[$dsn] = false;
		}
		return true;

	}//end rollback()


}//end class
?>
