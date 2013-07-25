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
* $Id: locking_method_memcache.inc,v 1.8 2013/07/25 23:23:48 lwright Exp $
*
*/

require_once SQ_CORE_PACKAGE_PATH.'/system/locking/locking_method/locking_method.inc';


/**
* Locking_Method_Memcache
*
* Purpose
*    System locking mechanism storing locking data in a Memcache server.
*
*
* @author		Luke Wright <lwright@squiz.net>
* @version		$Revision: 1.8 $
* @package		MySource_Matrix_Packages
* @subpackage	__core__
*/
class Locking_Method_Memcache extends Locking_Method
{


	public static $memcache = NULL;


	/**
	* Constructor
	*
	* This doesn't really matter in this case, as it will only be called
	* statically.
	*
	* @param string	$assetid	The ID of the asset to load
	*
	*/
	public function __construct($assetid=0)
	{
		parent::__construct($assetid);

	}//end __construct()


	/**
	* Initialise the memcache object with data from the memcache.inc file
	*
	* TODO: allow us to specify separate servers for cache managing than that
	* for locking.
	*
	*/
	public static function _initMemcache()
	{
		assert_true(extension_loaded('memcache'), 'Cannot use Memcache Locking Method; it requires the memcache PECL extension installed within PHP, which is not installed');
		assert_true(file_exists(SQ_DATA_PATH.'/private/conf/memcache.inc'), 'Cannot use Memcache Locking Method; the Memcache configuration file is not set');

		$memcache_conf = require(SQ_DATA_PATH.'/private/conf/memcache.inc');
		$hosts =& $memcache_conf['hosts'];
		$services =& $memcache_conf['services'];

		assert_true(count($hosts) > 0, 'Cannot use Memcache Locking Method; no hosts are defined in the Memcache configuration file');
		assert_true(array_key_exists('locking', $services) === TRUE, 'Cannot use Memcache Locking Method; no Memcache hosts are assigned to locking');
		assert_true(count($services['locking']) > 0, 'Cannot use Memcache Locking Method; no Memcache hosts are assigned to locking');

		// If PHP has the memcache module installed, instantiate it and try to load some config.
		self::$memcache = new Memcache;

		foreach ($services['locking'] as $host_key => $weight) {
			assert_true(array_key_exists($host_key, $hosts) === TRUE, 'Cannot use Memcache Locking Method; host key "'.$host_key.'" assigned for use for locking but not defined as a host');
			$host = $hosts[$host_key];
			self::$memcache->addServer($host['host'], $host['port'], $host['persistent'], $weight, $host['timeout'], $host['retry_interval'], $host['status'], Array('Locking_Method_Memcache', 'failureCallback'));
		}

		self::$memcache->setCompressThreshold($memcache_conf['compression_threshold'], $memcache_conf['compression_min_saving']);

	}//end _initMemcacheObj()


	/**
	* Acquire a lock on part of the system
	*
	* The $source_lockid parameter should be set if an asset is being locked
	* as a result of a dependant parent being locked. If the source lock is
	* released, this lock will be too. If this lock is not dependent on any
	* other, use blank.
	*
	* Setting $expires to zero (or omitted) will result in the default lock
	* length being used (as defined as SQ_CONF_LOCK_LENGTH in the System
	* Configuration file).
	*
	* If $expires is NULL, there will be no expiry time set on the lock. It will
	* remain locked until unlocked by the user.
	*
	* Otherwise, $expires should be a UNIX timestamp of the expiry time. (ie. it
	* is not a lock length.)
	*
	* The locking method should throw an exception if there is a problem.
	*
	* @param string	$lockid			The ID of the lock to acquire
	* @param string	$source_lockid	The ID that triggered the lock acquisition
	* @param int	$expires		Timestamp of the expiry time
	*
	* @return boolean
	*/
	public static function acquireLock($lockid, $source_lockid='', $expires=0)
	{
		if (self::$memcache === NULL) {
			self::_initMemcache();
		}

		// if we have no source, we are the asset aquiring the locks
		if (empty($source_lockid)) $source_lockid = $lockid;

		$current_time = time();

		if (!is_null($expires)) {
			$expires = (empty($expires)) ? ($current_time + SQ_CONF_LOCK_LENGTH) : (int) $expires;
			//$expires = ts_iso8601($expires);
		}

		// Change the expire time into a length in seconds, to avoid clashes
		// between clocks on different servers. If calculated time is less than
		// one second, make it 1 second because a time of zero is "never expire"
		if ($expires !== NULL) {
			$expire_seconds = $expires - $current_time;
			if ($expire_seconds < 1) $expire_seconds = 1;
		} else {
			$expire_seconds = 0;
		}

		// Store lock data in the value...this will get automatically serialised
		// by the Memcache module
		$lock_data = Array(
						'lockid'		=> $lockid,
						'source_lockid'	=> $source_lockid,
						'userid'		=> $GLOBALS['SQ_SYSTEM']->currentUserId(),
						'expires'		=> ts_iso8601($expires),
					 );

		self::$memcache->set('lock:'.$lockid, $lock_data, 0, $expire_seconds);

		$chain_lockids = self::$memcache->get('lockchain:'.$source_lockid);
		if ($chain_lockids !== FALSE) {
			// check whether we have to add this lock to the chain
			if (array_search($lockid, $chain_lockids) === FALSE) {
				$chain_lockids[] = $lockid;
			}
		} else {
			// We need to create a new lock chain
			$chain_lockids = Array($lockid);
		}
		// Set up the lock chain (or reset its expiry time)
		self::$memcache->set('lockchain:'.$source_lockid, $chain_lockids, 0, $expire_seconds);

		return TRUE;

	}//end acquireLock()


	/**
	* Update the expiry time on a lock
	*
	* The expiry time will be updated on all locks that share the same source
	* lock ID as $lockid. The $expires parameter follows the same rules as
	* acquireLock().
	*
	* If a lock is set to indefinite (NULL), this function will not change this,
	* but will still return TRUE. You must release an indefinite lock before
	* setting a definite expiry date on it.
	*
	* The locking method should throw an exception if there is a problem with
	* updating the lock expiry.
	*
	* @param string	$lockid			The ID of the lock to update
	* @param int	$expires		Timestamp of the expiry time
	*
	* @see Locking_Method::acquireLock()
	* @return boolean
	*/
	public static function updateLock($lockid, $expires)
	{
		if (self::$memcache === NULL) {
			self::_initMemcache();
		}

		$current_time = time();
		$current_lock = self::getLockInfo($lockid);

		// If there is no current lock (it may have expired), or if the lock
		// is indefinite, do nothing
		if (empty($current_lock) || empty($current_lock['expires'])) {
			return TRUE;
		}

		if (!is_null($expires)) {
			$expires = (!$expires) ? (time() + SQ_CONF_LOCK_LENGTH) : (int) $expires;
		}

		// Change the expire time into a length in seconds, to avoid clashes
		// between clocks on different servers. If calculated time is less than
		// one second, make it 1 second because a time of zero is "never expire"
		if ($expires !== NULL) {
			$expire_seconds = $expires - $current_time;
			if ($expire_seconds < 1) $expire_seconds = 1;
		} else {
			$expire_seconds = 0;
		}

		// Get the links in the chain, and update them. It'll get automatically
		// serialised by Memcache PECL.
		$chain_lockids = self::$memcache->get('lockchain:'.$current_lock['source_lockid']);
		if ($chain_lockids !== FALSE) {
			$lock_data = Array(
							'source_lockid'	=> $current_lock['source_lockid'],
							'userid'		=> $current_lock['userid'],
							'expires'		=> ts_iso8601($expires),
					 );
			foreach ($chain_lockids as $chain_lockid) {
				$lock_data['lockid'] = $chain_lockid;
				self::$memcache->set('lock:'.$chain_lockid, $lock_data, 0, $expire_seconds);
			}
			// Reset the lock chain's expiry time
			self::$memcache->set('lockchain:'.$current_lock['source_lockid'], $chain_lockids, 0, $expire_seconds);
		}

		return TRUE;

	}//end updateLock()


	/**
	* Release a lock
	*
	* All locks that share the same source lock ID as $lockid will be released,
	* to ensure that dependant groups of assets (for which the source lock is
	* primarily used for) are locked and unlocked in one group.
	*
	* @param string	$lockid			The ID of the lock to release
	*
	* @see Locking_Method::acquireLock()
	* @return boolean
	*/
	public static function releaseLock($lockid)
	{
		if (self::$memcache === NULL) {
			self::_initMemcache();
		}

		$current_lock = self::getLockInfo($lockid, FALSE, FALSE);
		if (empty($current_lock)) return TRUE;

		// Get the links in the chain, and nuke them.
		$chain_lockids = self::$memcache->get('lockchain:'.$current_lock['source_lockid']);
		if ($chain_lockids !== FALSE) {
			foreach ($chain_lockids as $chain_lockid) {
				self::$memcache->delete('lock:'.$chain_lockid);
			}
			self::$memcache->delete('lockchain:'.$current_lock['source_lockid']);
		}

		return TRUE;

	}//end releaseLock()


	/**
	* Returns an array of information about the passed lock
	*
	* If $full_chain is TRUE, all locks with the same source lock ID as that
	* of $lockid will be returned (this could return locks from both parents and
	* children if this is in the middle of a full chain).
	*
	* The $check_expires variable is unused in this method. Memcache is
	* responsible for releasing expired lock data, so it's hard to still return
	* expired locks.
	*
	* @param string|array	$lockid			The id of the lock to get info for
	* @param boolean		$full_chain		TRUE = Return the full lock chain
	* @param boolean		$check_expires	TRUE = Clean up expired locks found
	* @param boolean		$allow_only_one	TRUE = Force only one lock returned
	*
	* @return array
	* @access public
	*/
	public static function getLockInfo($lockid, $full_chain=FALSE, $check_expires=TRUE, $allow_only_one=TRUE)
	{
		if (self::$memcache === NULL) {
			self::_initMemcache();
		}

		$results = Array();
		if (is_array($lockid) === FALSE) $lockid = Array($lockid);

		foreach ($lockid as $this_lockid) {
			$main_lock_data = self::$memcache->get('lock:'.$this_lockid);
			if ($main_lock_data !== FALSE) {
				$main_lock_data['expires'] = empty($main_lock_data['expires']) ? NULL : iso8601_ts($main_lock_data['expires']);
				$results[$this_lockid] = $main_lock_data;
			}

			if ($full_chain === TRUE) {
				$results[$this_lockid]['chained_assets'] = Array();
				$chain_lockids = self::$memcache->get('lockchain:'.$main_lock_data['source_lockid']);
				if ($chain_lockids !== FALSE) {

					foreach ($chain_lockids as $chain_lockid) {
						if ($chain_lockid !== $this_lockid) {
							$chain_lock_data = self::$memcache->get('lock:'.$chain_lockid);
							if ($chain_lock_data !== FALSE) {
								$chain_lock_data['expires'] = empty($chain_lock_data['expires']) ? NULL : iso8601_ts($chain_lock_data['expires']);
								$results[$this_lockid]['chained_assets'][$chain_lockid] = $chain_lock_data;
							}

						}

					}

				}

			}//end if full chain

		}//end foreach main lockid

		if (!empty($results) && $allow_only_one) {
			$results = reset($results);
		}

		return $results;

	}//end getLockInfo()


	/**
	* Does this locking method support the getting of active locks?
	*
	* (No, rather unfortunately. :-/)
	*
	* @return boolean
	*/
	public static function supportsGetActiveLocks()
	{
		return FALSE;

	}//end supportsGetActiveLocks()


	/**
	* Does this locking method support the changing of lock ownership?
	*
	* We could change the owner of a single lock, but not a sweeping change
	* from one owner to another, which is what changeLockOwner() is about.
	*
	* @return boolean
	*/
	public static function supportsChangeLockOwner()
	{
		return FALSE;

	}//end supportsChangeLockOwner()


    /**
     * Does this locking method support deleting expired locks?
     *
     * Memcache has no way to do this unfortunately.
     *
     * @return boolean
     */
    public static function supportsDeletingExpiredLocks()
    {
        return FALSE;

    }//end supportsDeletingExpiredLocks()


	/**
	* Failure Callback function for Memcache::addServer
	* 
	* @param $hostname
	* @param $port
	* 
	* @return void
	* @access public
	*/
	public static function failureCallback($hostname, $port)
	{
		log_error(get_class()." failure communicating with $hostname:$port", E_USER_WARNING);

	}//end failureCallback()
	

}//end class

?>
