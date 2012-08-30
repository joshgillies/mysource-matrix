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
* $Id: locking_method.inc,v 1.3 2012/08/30 01:09:11 ewang Exp $
*
*/

require_once SQ_INCLUDE_PATH.'/asset.inc';


/**
* Locking_Method
*
* Purpose
*    Abstract class for providing asset locking.
*
*    It is expected that subclassed locking methods implement all static
*    functions described here.
*
*    This replaces the all-purpose locking system provided in the MySource
*    class, and therefore is the low-level system working with lock IDs
*    (therefore, it can be used to lock things other than assets).
*
*    Asset locking generally results in lock IDs of "asset.###.lock_type"
*    where ### is the asset ID.
*
* @author		Luke Wright <lwright@squiz.net>
* @version		$Revision: 1.3 $
* @package		MySource_Matrix_Packages
* @subpackage	__core__
*/
class Locking_Method extends Asset
{


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
		trigger_error('This locking method has not implemented acquiring of locks', E_USER_ERROR);

	}//end acquireLock()


	/**
	* Update the expiry time on a lock
	*
	* The expiry time will be updated on all locks that show $lockid as its
	* source lock ID. The $expires parameter follows the same rules as
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
	public static function updateLock($lockid, $expires=0)
	{
		trigger_error('This locking method has not implemented updating of locks', E_USER_ERROR);

	}//end acquireLock()


	/**
	* Release a lock
	*
	* All locks that show $lockid as its source lock ID will be released. This
	* is to ensure that dependant groups of assets (for which the source lock is
	* primarily used for) are locked and unlocked in one group.
	*
	* @param string	$lockid			The ID of the lock to release
	*
	* @see Locking_Method::acquireLock()
	* @return boolean
	*/
	public static function releaseLock($lockid)
	{
		trigger_error('This locking method has not implemented releasing of locks', E_USER_ERROR);

	}//end releaseLock()


	/**
	* Returns an array of information about the passed lock
	*
	* If $full_chain is TRUE, all locks with the same source lock ID as that
	* of $lockid will be returned (this could return locks from both parents and
	* children if this is in the middle of a full chain).
	*
	* If $check_expires is TRUE, any locks that have expired will be released,
	* and not returned. Only locks that would have been returned by this
	* function are tested; it does not do a general "sweep" of the locks table.
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
		trigger_error('This locking method has not implemented getting of lock info', E_USER_ERROR);

	}//end getLockInfo()





	/**
	* Get a list of all active locks, optionally filtered by lock owner
	*
	* Requires the ability to filter the list of locks by owner asset ID. If
	* not, boolean FALSE will be returned.
	*
	* An exception should be raised if there are database issues. Simply rethrow
	* the exception and let the calling function handle the error message.
	*
	* If $owner_assetid omitted, all locks will be returned.
	*
	* @param string		$owner_assetid		The asset ID of the owning user to filter
	* @param boolean	$include_expired	TRUE = expired locks will also be returned
	*
	* @return array
	*/
	public static function getActiveLocks($owner_assetid=NULL, $include_expired=FALSE)
	{
		if (self::supportsGetActiveLocks() === FALSE) {
			trigger_error('This locking method does not support getting of active locks', E_USER_WARNING);
			return FALSE;
		} else {
			trigger_error('This locking method supports getting of active locks, but has not implemented getActiveLocks method', E_USER_ERROR);
		}

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
	*
	* Not entirely useful with core users. Mostly useful for users being drawn
	* in from external sources, such as LDAP, where the DN might change and
	* therefore, possibly the user.
	*
	* @param string	$old_assetid	The asset ID of the old user owner
	* @param string	$new_assetid	The new user asset ID to own the locks
	*
	* @return boolean
	*/
	public static function changeLockOwner($old_assetid, $new_assetid)
	{
		if (self::supportsGetActiveLocks() === FALSE) {
			trigger_error('This locking method does not support changing of lock owner', E_USER_WARNING);
			return FALSE;
		} else {
			trigger_error('This locking method supports changing of lock owner, but has not implemented changeLockOwner method', E_USER_ERROR);
		}

	}//end changeLockOwner()


	/**
	* Does this locking method support the changing of lock ownership?
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


}//end class

?>
