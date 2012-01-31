<?php
/**
 * A DSN object represents a single DSN in db.inc file.
 *
 * @version    4.0.0
 * @package    MySource4
 * @subpackage DAL
 * @author	   Squiz Pty Ltd <mysource4@squiz.net>
 * @copyright  2006-2007 Squiz Pty Ltd (ABN 77 084 670 600)
 * @license    http://matrix.squiz.net/licence Squiz.Net Open Source Licence
 */


/**
 * DSN.
 *
 * A DSN object represents a single DSN.
 *
 * DSN provides methods for getting weights, setting failure, etc.
 *
 * @since 4.0.0
 */
class DSN
{
	
	
	/**
	 * The array of a single DSN that usually has the format of
	 * Array(
	 *       'DSN' => 'pgsql:dbname=mysource',
	 *       'user' => 'matrix',
	 *       'password' => '',
	 *       'type' => 'pgsql',
	 *       'weight'=> 1,
	 *       'timeout' => 1,
	 *      )
	 * 
	 * @var array
	 */
	private $_dsn;
	
	/**
	 * The flag to indicate if this DSN has failed.
	 * 
	 * @var boolean
	 */
	private $_failure = FALSE;
	
	
	/**
	 * Constructor
	 * 
	 * @param array $dsn	The array of a single DSN
	 * @access public
	 */
	public function __construct(Array $dsn)
	{
		$this->_dsn = $dsn;
		
	}//end constructor
	
	
	/**
	 * Return the DSN array
	 * 
	 * @return array	Return the single DSN array this object represents
	 * @access public
	 */
	public function getDSN()
	{
		return $this->_dsn;
		
	}//end getDSN()
	
	
	/**
	 * Get the failure value.
	 * 
	 * @return boolean		Return the failure flag value of this object
	 * @access public
	 */
	public function getFailure()
	{
		return $this->_failure;
		
	}//end getFailure()
	
	
	/**
	 * Set the failure value.
	 * 
	 * @param boolean $failure		The boolean value to set for the failure flag
	 * @return void
	 * @access public
	 */
	public function setFailure($failure)
	{
		$this->_failure = $failure;
		
	}//end setFailure()
	
	
	/**
	 * Get the weight of the DSN.
	 * 
	 * @return int		Return the integer value of the DSN weight; Return -1 if there
	 *            		is no weight specified or the value is invalid
	 * @access public
	 */
	public function getWeight()
	{
		$weight = array_get_index($this->_dsn, 'weight', 1);
		if (!is_numeric($weight) || ($weight < 1)) {
			$weight = 1;
		}
		
		return (int)$weight;
		
	}//end getWeight()
	
	
}//end class


?>