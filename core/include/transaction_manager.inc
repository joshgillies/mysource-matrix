<?php
/**
* +--------------------------------------------------------------------+
* | MySource 3 - MySource Matrix                                       |
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
* $Id: transaction_manager.inc,v 1.8 2003/11/18 15:37:34 brobertson Exp $
* $Name: not supported by cvs2svn $
*/


/**
* Transaction_Manager
*
* Purpose
*    Handle transactions for both the DB and the file system
*    Also to handle nested transaction calls
*
* @author  Greg Sherwood <greg@squiz.net>
* @version $Version$ - 1.0
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
	* @param $db_name	The name of the database object in the $GLOBALS['SQ_SYSTEM'] object
	*
	* @return boolean
	* @access public
	*/
	function inTransaction($db_name)
	{
		return !empty($this->_levels[$db_name]);
	}//end inTransaction()


	/**
	* Begins a transaction
	* Only does a real begin if this is the first begin
	*
	* @param $db_name	The name of the database object in the $GLOBALS['SQ_SYSTEM'] object
	*
	* @return boolean
	* @access public
	*/
	function begin($db_name)
	{
		if (isset($this->_levels[$db_name])) $this->_levels[$db_name]++;
		else $this->_levels[$db_name] = 1;

		if ($this->_levels[$db_name] == 1) {
			// first time we have started a transaction
			$result = $GLOBALS['SQ_SYSTEM']->db->query('BEGIN');
			if (DB::isError($result)) trigger_error($result->getMessage().'<br/>'.$result->getUserInfo(), E_USER_ERROR);
		}
		return true;
	}//end begin()


	/**
	* Commits a transaction
	* Only does a real commit if this is the last commit
	*
	* @param $db_name	The name of the database object in the $GLOBALS['SQ_SYSTEM'] object
	*
	* @return boolean
	* @access public
	*/
	function commit($db_name)
	{
		if (empty($this->_levels[$db_name])) {
			trigger_error('Unable to Commit, No Transaction is Open', E_USER_WARNING);
			return false;
		}

		if (!empty($this->_rollback[$db_name])) {
			trigger_error('Unable to Commit, Transaction has already been aborted', E_USER_WARNING);
			$this->rollback($db_name);
			return false;
		}
		$this->_levels[$db_name]--;
		if ($this->_levels[$db_name] == 0) {
			// this is the last commit
			$result = $GLOBALS['SQ_SYSTEM']->db->query('COMMIT');
			if (DB::isError($result)) trigger_error($result->getMessage().'<br/>'.$result->getUserInfo(), E_USER_ERROR);
		}
		return true;
	}//end commit()


	/**
	* Rolls back all open transactions
	* Only does a real rollback if this is the last rollback
	*
	* @param $db_name	The name of the database object in the $GLOBALS['SQ_SYSTEM'] object
	*
	* @return boolean
	* @access public
	*/
	function rollback($db_name)
	{
		if (empty($this->_levels[$db_name])) {
			trigger_error('Unable to Commit, No Transaction is Open', E_USER_WARNING);
			return false;
		}

		$this->_rollback[$db_name] = true;
		$this->_levels[$db_name]--;
		if ($this->_levels[$db_name] == 0) {
			// this is the last rollback
			$result = $GLOBALS['SQ_SYSTEM']->db->query('ROLLBACK');
			if (DB::isError($result)) trigger_error($result->getMessage().'<br/>'.$result->getUserInfo(), E_USER_ERROR);
		}
		return true;
	}//end rollback()

}//end class
?>
