<?php

/**
* Transaction_Manager
*
* Purpose
*    Handle transactions for both the DB and the file system
*    Also to handle nested transaction calls
*
* @author  Greg Sherwood <greg@squiz.net>
* @version $Version$ - 1.0
* @package Resolve
*/
class Transaction_Manager extends Resolve_Object
{

	/**
	* Lets us know if we have rolled back
	*
	* @var string()
	*/
	var $_rollback;

	/**
	* How many levels of transactions we are deep
	*
	* @var string()
	*/
	var $_levels;


	/**
	* Constructor
	*
	*/
	function Transaction_Manager()
	{
		$this->Resolve_Object();
		$this->_rollback = false;
		$this->_levels = 0;
	}//end constructor


	/**
	* Returns whether we are in a transaction or not
	*
	* @return boolean
	* @access public
	*/
	function inTransaction()
	{
		return ($this->_levels > 0);
	}//end inTransaction()


	/**
	* Begins a transaction
	* Only does a real begin if this is the first begin
	*
	* @return boolean
	* @access public
	*/
	function begin()
	{
		$this->_levels++;
		if ($this->_levels == 1) {
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
	* @return boolean
	* @access public
	*/
	function commit()
	{
		if ($this->_rollback) {
			trigger_error('Unable to Commit, Transaction has already been aborted', E_USER_WARNING);
			$this->rollback();
			return false;
		}
		$this->_levels--;
		if ($this->_levels == 0) {
			// this is the last commit
			$result = $GLOBALS['SQ_SYSTEM']->db->query('COMMIT');
			if (DB::isError($result)) trigger_error($result->getMessage().'<br/>'.$result->getUserInfo(), E_USER_ERROR);
			$this->_rollback = false;
		}
		return true;
	}//end commit()


	/**
	* Rolls back all open transactions
	* Only does a real rollback if this is the last rollback
	*
	* @return boolean
	* @access public
	*/
	function rollback()
	{
		$this->_rollback = true;
		$this->_levels--;
		if ($this->_levels == 0) {
			// this is the last rollback
			$result = $GLOBALS['SQ_SYSTEM']->db->query('ROLLBACK');
			if (DB::isError($result)) trigger_error($result->getMessage().'<br/>'.$result->getUserInfo(), E_USER_ERROR);
			$this->_rollback = false;
		}
		return true;
	}//end rollback()

}//end class
?>
