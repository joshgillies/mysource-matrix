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
* $Id: locking_method_redis.inc,v 1.8 2013/07/25 23:23:48 lwright Exp $
*
*/

require_once SQ_CORE_PACKAGE_PATH.'/system/locking/locking_method/locking_method.inc';
require_once SQ_LIB_PATH.'/MatrixRedis/MatrixRedis.inc';


/**
* Locking_Method_Redis
*
* Purpose
*    System locking mechanism storing locking data in a Redis server.
*
*
* @author        Chris Smith <csmith@squiz.net>
* @version        $Revision: 1.8 $
* @package        MySource_Matrix_Packages
* @subpackage    __core__
*/
class Locking_Method_Redis extends Locking_Method
{


    /**
     * A cache of the redis connection.
     * Set up in _initRedis().
     */
    private static $_redis = NULL;


    /**
    * Constructor
    *
    * This doesn't really matter in this case, as it will only be called
    * statically.
    *
    * @param string    $assetid    The ID of the asset to load
    *
    */
    public function __construct($assetid=0)
    {
        parent::__construct($assetid);

    }//end __construct()


    /**
     * Initialise the redis object with data from the redis.inc file
     */
    private static function _initRedis()
    {
        if (self::$_redis !== NULL) {
            return TRUE;
        }

        $connection = MatrixRedis::getConnection('locking');
        if ($connection === FALSE) {
            return FALSE;
        }

        self::$_redis = $connection;
        return TRUE;

    }//end _initRedis()


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
    * @param string    $lockid            The ID of the lock to acquire
    * @param string    $source_lockid    The ID that triggered the lock acquisition
    * @param int    $expires        Timestamp of the expiry time
    *
    * @return boolean
    */
    public static function acquireLock($lockid, $source_lockid='', $expires=0)
    {
        if (!self::_initRedis()) {
            return FALSE;
        }

        // if we have no source, we are the asset aquiring the locks
        if (empty($source_lockid)) $source_lockid = $lockid;

        $current_time = time();

        if (!is_null($expires)) {
            $expires = (empty($expires)) ? ($current_time + SQ_CONF_LOCK_LENGTH) : (int) $expires;
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

        // Store lock data in the value.
        $lock_data = Array(
                        'lockid'        => $lockid,
                        'source_lockid' => $source_lockid,
                        'userid'        => $GLOBALS['SQ_SYSTEM']->currentUserId(),
                        'expires'       => ts_iso8601($expires),
                     );

        self::$_redis->set('lock:'.$lockid, serialize($lock_data), $expire_seconds);

        $chain_lockids = self::$_redis->get('lockchain:'.$source_lockid);
        if ($chain_lockids !== FALSE) {
            $chain_lockids = unserialize($chain_lockids);

            // check whether we have to add this lock to the chain
            if (array_search($lockid, $chain_lockids) === FALSE) {
                $chain_lockids[] = $lockid;
            }
        } else {
            // We need to create a new lock chain
            $chain_lockids = Array($lockid);
        }
        // Set up the lock chain (or reset its expiry time)
        self::$_redis->set('lockchain:'.$source_lockid, serialize($chain_lockids), $expire_seconds);

        // Add it to the userlocks list as well.
        self::$_redis->sAdd('userlocks:'.$GLOBALS['SQ_SYSTEM']->currentUserId(), $lockid);

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
    * @param string    $lockid            The ID of the lock to update
    * @param int    $expires        Timestamp of the expiry time
    *
    * @see Locking_Method::acquireLock()
    * @return boolean
    */
    public static function updateLock($lockid, $expires)
    {
        if (!self::_initRedis()) {
            return FALSE;
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

        // Get the links in the chain, and update them.
        $chain_lockids = self::$_redis->get('lockchain:'.$current_lock['source_lockid']);
        if ($chain_lockids !== FALSE) {
            $chain_lockids = unserialize($chain_lockids);
            $lock_data = Array(
                            'source_lockid'    => $current_lock['source_lockid'],
                            'userid'        => $current_lock['userid'],
                            'expires'        => ts_iso8601($expires),
                     );
            foreach ($chain_lockids as $chain_lockid) {
                $lock_data['lockid'] = $chain_lockid;
                self::$_redis->set('lock:'.$chain_lockid, serialize($lock_data), $expire_seconds);
            }
            // Reset the lock chain's expiry time
            self::$_redis->set('lockchain:'.$current_lock['source_lockid'], serialize($chain_lockids), $expire_seconds);
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
    * @param string    $lockid            The ID of the lock to release
    *
    * @see Locking_Method::acquireLock()
    * @return boolean
    */
    public static function releaseLock($lockid)
    {
        if (!self::_initRedis()) {
            return FALSE;
        }

        $current_lock = self::getLockInfo($lockid, FALSE, FALSE);
        if (empty($current_lock)) return TRUE;

        // Get the links in the chain, and nuke them.
        $chain_lockids = self::$_redis->get('lockchain:'.$current_lock['source_lockid']);
        if ($chain_lockids !== FALSE) {
            $chain_lockids = unserialize($chain_lockids);
            foreach ($chain_lockids as $chain_lockid) {
                // Get the lock, look up the 'owner' of the lock, and remove it from their list.
                $chainInfo = self::$_redis->get('lock:'.$chain_lockid);
                if ($chainInfo) {
                    $chainInfo = unserialize($chainInfo);
                    self::$_redis->sRemove('userlocks:'.$chainInfo['userid'], $chain_lockid);
                }

                self::$_redis->delete('lock:'.$chain_lockid);
            }
            self::$_redis->delete('lockchain:'.$current_lock['source_lockid']);
        }

        self::$_redis->delete('lock:'.$lockid);
        self::$_redis->sRemove('userlocks:'.$current_lock['userid'], $lockid);

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
    * @param string|array    $lockid            The id of the lock to get info for
    * @param boolean        $full_chain        TRUE = Return the full lock chain
    * @param boolean        $check_expires    TRUE = Clean up expired locks found
    * @param boolean        $allow_only_one    TRUE = Force only one lock returned
    *
    * @return array
    * @access public
    */
    public static function getLockInfo($lockid, $full_chain=FALSE, $check_expires=TRUE, $allow_only_one=TRUE)
    {
        if (!self::_initRedis()) {
            return FALSE;
        }

        $results = Array();
        if (is_array($lockid) === FALSE) $lockid = Array($lockid);

        foreach ($lockid as $this_lockid) {
            $main_lock_data = self::$_redis->get('lock:'.$this_lockid);
            if ($main_lock_data !== FALSE) {
                $main_lock_data = unserialize($main_lock_data);
                $main_lock_data['expires'] = empty($main_lock_data['expires']) ? NULL : iso8601_ts($main_lock_data['expires']);
                $results[$this_lockid] = $main_lock_data;
            }

            if ($full_chain === TRUE) {
                $results[$this_lockid]['chained_assets'] = Array();
                $chain_lockids = self::$_redis->get('lockchain:'.$main_lock_data['source_lockid']);
                if ($chain_lockids !== FALSE) {
                    $chain_lockids = unserialize($chain_lockids);
                    foreach ($chain_lockids as $chain_lockid) {
                        if ($chain_lockid !== $this_lockid) {
                            $chain_lock_data = self::$_redis->get('lock:'.$chain_lockid);
                            if ($chain_lock_data !== FALSE) {
                                $chain_lock_data = unserialize($chain_lock_data);
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
     * Find lock information.
     *
     * Both getActiveLocks() and deleteExpiredLocks() do the same thing (trawling keys and checking expiry times),
     * only deleteExpiredLocks() doesn't need the locks to be returned.
     *
     * @param string         $owner_assetid          The asset ID of the owning user to filter.
     *                                          If null, then all locks are returned or cleaned up.
     * @param boolean        $include_expired        TRUE = expired locks will also be returned
     * @param boolean        $returnLocks            If true, return lock info, otherwise it returns a boolean.
     */
    private static function _findLocks($owner_assetid, $include_expired=FALSE, $returnLocks=TRUE)
    {
        $locksfound = array();

        if (!self::_initRedis()) {
            if ($returnLocks) {
                return $locksfound;
            }
            return TRUE;
        }

        $redisPrefix = '';
        if (empty($owner_assetid) || $owner_assetid === NULL) {
            $lockKeys    = self::$_redis->keys('userlocks:*');
            $redisPrefix = self::$_redis->getOption(Redis::OPT_PREFIX);
        } else {
            $lockKeys = array('userlocks:'.$owner_assetid);
        }

        $timeNow = time();

        foreach ($lockKeys as $lockKey) {
            // Keys found by wildcard keep the prefix in the key name, it needs
            // to be stripped off before using the key again in another call.
            if (!empty($redisPrefix)) {
                $lockKey = str_replace($redisPrefix, '', $lockKey);
            }

            // Get the list of user locks.
            $userlocks = self::$_redis->sGetMembers($lockKey);

            if (empty($userlocks)) {
                continue;
            }

            // Now we have the lock names, check if they have expired or not.
            foreach ($userlocks as $lockid) {
                $lockInfo = self::$_redis->get('lock:'.$lockid);

                // If this is false, it means redis has cleaned up the lock (it has expired).
                // Just clean up the set and keep going.
                if (!$lockInfo) {
                    self::$_redis->sRemove($lockKey, $lockid);
                    continue;
                }

                $lock_data = unserialize($lockInfo);
                if ($include_expired) {
                    $locksfound[] = $lock_data;
                    continue;
                }

                // If it's not expired, excellent. Otherwise clean up the redis set.
                if ($lock_data['expires'] === NULL || (strtotime($lock_data['expires']) > $timeNow)) {
                    $locksfound[] = $lock_data;
                } else {
                    self::$_redis->sRemove($lockKey, $lockid);
                }
            }

        }

        if ($returnLocks) {
            return $locksfound;
        }

        return TRUE;

    }//end _findLocks()


    /**
     * Get a list of all active locks, optionally filtered by lock owner
     *
     * Requires the ability to filter the list of locks by owner asset ID.
     *
     * If $owner_assetid omitted, all locks will be returned.
     *
     * @param string         $owner_assetid          The asset ID of the owning user to filter
     * @param boolean        $include_expired        TRUE = expired locks will also be returned
     *
     * @return array
     */
    public static function getActiveLocks($owner_assetid=NULL, $include_expired=FALSE)
    {
        $locksfound = self::_findLocks($owner_assetid, $include_expired, TRUE);

        return $locksfound;

    }//end getActiveLocks()


    /**
    * Does this locking method support the getting of active locks?
    *
    * @return boolean
    */
    public static function supportsGetActiveLocks()
    {
        return TRUE;

    }//end supportsGetActiveLocks()


    /**
     * Change the owner of a lock from one user to another
     *
     * Requires the ability to filter the list of locks by owner asset ID.
     * Not entirely useful with core users. Mostly useful for users being drawn
     * in from external sources, such as LDAP, where the DN might change and
     * therefore, possibly the user.
     *
     * @param string $old_assetid    The asset ID of the old user owner
     * @param string $new_assetid    The new user asset ID to own the locks
     *
     * @return boolean
     */
    public static function changeLockOwner($old_assetid, $new_assetid)
    {
        if (!self::_initRedis()) {
            return FALSE;
        }

        // Get the list of user locks.
        $userlocks = self::$_redis->sGetMembers('userlocks:'.$old_assetid);
        if (empty($userlocks)) {
            return TRUE;
        }

        foreach ($userlocks as $lockid) {
            // Get the old lock.
            $lock_data = unserialize(self::$_redis->get('lock:'.$lockid));

            // Switch the user, set the new expiry.
            $lock_data['userid'] = $new_assetid;
            $expire_seconds = min(1, (strtotime($lock_data['expires']) - time()));

            // Do this as a 'transaction'.
            $res = self::$_redis->multi()
                ->set('lock:'.$lockid, serialize($lock_data), $expire_seconds) // update the lock info.
                ->sAdd('userlocks:'.$new_assetid, $lockid) // Add it to the new userlocks asset id.
                ->sRemove('userlocks:'.$old_assetid, $lockid) // and remove it from the old one.
                ->exec();
        }

        return TRUE;

    }//end changeLockOwner()


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
        return TRUE;

    }//end supportsChangeLockOwner()


    /**
     * Does this locking method support deleting expired locks?
     *
     * @return boolean
     */
    public static function supportsDeletingExpiredLocks()
    {
        return TRUE;

    }//end supportsDeletingExpiredLocks()


    /**
     * Delete expired locks.
     *
     * _findLocks does all the work for us, it looks at all user lock info and
     * will delete expired locks as it finds them. We just don't want the results back.
     *
     * The keys in redis itself will automatically expire and clean themselves up, this just
     * needs to clean up the userlock info (which is a set, and can't be expired).
     */
    public static function deleteExpiredLocks()
    {
        self::_findLocks(NULL, FALSE, FALSE);
        return TRUE;

    }//end deleteExpiredLocks()


}//end class

?>
