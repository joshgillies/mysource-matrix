<?php
/**
 * The DSN Manager maintains the list of DSN and DSNPool objects of the connections
 * configured in the db.inc file, e.g.'db', 'db2', 'd3', etc.
 *
 * @version    4.0.0
 * @package    MySource4
 * @subpackage DAL
 * @author	   Squiz Pty Ltd <mysource4@squiz.net>
 * @copyright  2006-2007 Squiz Pty Ltd (ABN 77 084 670 600)
 * @license    http://matrix.squiz.net/licence Squiz.Net Open Source Licence
 */

require_once dirname(__FILE__).'/DSN.inc';
require_once dirname(__FILE__).'/DSNPool.inc';

/**
 * DSNManager.
 *
 * DSNManager class stores the list of DSN and DSNPool objects of the connections
 * using the connection ids like 'db', 'db2', etc
 * 
 * DSNManager class also provides the methods to process the DSN list.
 *
 * @since 4.0.0
 */
class DSNManager
{
	
	
	/**
	 * The list of connection ids that support DSNPool. We only support read DB ('db')
	 * for now but we can extend the list to include write DB ('db2') if we have the
	 * right multi-master DB servers set up.
	 * 
	 * @var array
	 */
	private static $_dsnpool_supported_conns = Array('db', 'dbsearch');
	
	/**
	 * The list of DSN objects which can be both DSN and DSNPool objects
	 * 
	 * @var array
	 */
	private static $_dsns = Array();
	
	
	/**
	 * Represent a DSN array with a DSN or DSNPool object and add it to the DSN list
	 * 
	 * @param string $conn_id	The id of the connection to add DSN
	 * @param array $dsn		The DSN to add to the list
	 * @return boolean			Return TRUE if the DSN can be added; Return FALSE if it
	 *                			is already in the list
	 * @access public
	 */
	public static function addDSN($conn_id = 'db', Array $dsn)
	{
		// If the DSN of the provided connection is already in the list, return FALSE immediately.
		if (isset(self::$_dsns[$conn_id])) {
			return FALSE;
		}
		
		// Parse the DSN to convert the one entry DSN array to a DSN
		self::_parseDSN($dsn);
		
		// If the DSN is a DSN array, create a DSNPool for it
		if (self::_isDSNArray($dsn)) {
			// Throw an exception if this connection does not support DSN Pool
			if (!self::isDSNPoolSupportedConnection($conn_id)) {
				throw new DALException("The connection '$conn_id' does not support DSN Pool.");
			}
			
			$new_dsn = new DSNPool($dsn);
		} else {
			$new_dsn = new DSN($dsn);
		}
		
		// Add the DSN or DSNPool object to the DSN list
		self::$_dsns[$conn_id] = $new_dsn;
		
		return TRUE;
		
	}//end addDSN()
	
	
	/**
	 * Get the DSN array of a connection. If the connection has a DSN Pool, it will return the DSN array
	 * of a random DSN in the Pool.
	 * 
	 * @param string $conn_id	The id of the connection to get the DSN from
	 * @return NULL|array		The DSN array of a single DSN object or the DSN array of a random DNS
	 *                   		object in the DSN Pool. Return NULL if there is no DSN for the connection.
	 * @access public
	 */
	public static function getDSN($conn_id = 'db')
	{
		if (!isset(self::$_dsns[$conn_id])) {
			return NULL;
		}
		
		return self::$_dsns[$conn_id]->getDSN();
		
	}//end getDSN()
	
	
	/**
	 * Register failure for the current DSN in the DSN Pool.
	 * 
	 * @param string $conn_id	The id of the connection to register DSN failure
	 * @return void
	 * @access public
	 */
	public static function registerDSNFailure($conn_id = 'db')
	{
		if (self::isDSNPool($conn_id)) {
			self::$_dsns[$conn_id]->registerFailureDSN();
		}
		
	}//end registerDSNFailure()
	
	
	/**
	 * Check if a connection has a DSN pool.
	 * 
	 * @param string $conn_id	The id of the connection to check
	 * @return boolean			Return TRUE if the connection has a DSN Pool; otherwise, return FALSE
	 * @access public
	 */
	public static function isDSNPool($conn_id = 'db')
	{
		if (!isset(self::$_dsns[$conn_id])) {
			return FALSE;
		}
		
		return (self::$_dsns[$conn_id] instanceof DSNPool);
		
	}//end isDSNPool()
	
	
	/**
	 * Check if the connection supports DSN Pool.
	 * 
	 * @param string $conn_id	The id of the connection to check
	 * @return boolean			Return TRUE if the connection supports DSN Pool; otherwise, return FALSE
	 * @access public
	 */
	public static function isDSNPoolSupportedConnection($conn_id = 'db')
	{
		return in_array($conn_id, self::$_dsnpool_supported_conns);
		
	}//end isDSNPoolSupportedConnection()
	
	
	/**
	 * Parse the DSN to convert a one entry DSN array to a single DSN so that a DSN object will be created
	 * instead of a DSNPool object.
	 *  
	 * @param array $dsn	The DSN array to parse
	 * @return void
	 * @access private
	 */
	private static function _parseDSN(&$dsn)
	{
		if (self::_isDSNArray($dsn) && (count($dsn) == 1)) {
			$dsn = $dsn[0];
		}
		
	}//end _parseDSN()
	
	
	/**
	 * Check if a DSN is a single DSN or an array.
	 * 
	 * @param array $dsn	The DSN array to check
	 * return boolean		Return TRUE if the DSN is a DSN array; otherwise, return FALSE
	 * @access private
	 */
	private static function _isDSNArray($dsn)
	{
		return !isset($dsn['DSN']);
		
	}//end _isDSNArray()
	
	
}//end class


?>