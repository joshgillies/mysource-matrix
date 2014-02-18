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
* $Id: backend_user.inc,v 1.14 2012/08/30 01:09:16 ewang Exp $
*
*/


require_once SQ_CORE_PACKAGE_PATH.'/users/user/user.inc';


/**
* Backend_User
*
* Purpose
*
*
* @author  Greg Sherwood <greg@squiz.net>
* @version $Revision: 1.14 $
* @package MySource_Matrix_Packages
* @subpackage __core__
*/
class Backend_User extends User
{


	/**
	* Constructor
	*
	* @param int	$assetid	the asset id to be loaded
	*
	*/
	function Backend_User($assetid=0)
	{
		$this->User($assetid);

	}//end constructor


	/**
	* Check if this user can access the backend interface
	*
	* This will be disabled by default for a standard user, and enabled for
	* users who are allowed backend access (eg. backend user).
	*
	* @return boolean
	* @access public
	*/
	function canAccessBackend()
	{
		return TRUE;

	}//end canAccessBackend()

	
	/*
	* Delete backend user asset from the trash
	*
	* Note that the asset <i>MUST</i> be in the trash to delete it <i>FROM</i> the trash.
	*
	* @param boolean    $release_lock   should we realease the lock after deleting
	*
	* @return boolean
	* @access public
	*/
	public function delete($release_lock=TRUE, $check_locked=TRUE)
	{	
		$success = FALSE;

		if (parent::delete($release_lock, $check_locked)) {

			$sql = 'SELECT lockid FROM '.SQ_TABLE_RUNNING_PREFIX.'lock WHERE userid = \''.$this->id.'\'';

			try {
				$query = MatrixDAL::preparePdoQuery($sql);
				$result = MatrixDAL::executePdoAssoc($query);
			} catch (Exception $e) {
				throw new Exception('Unable to get lock info for backend user due to database error: '.$e->getMessage());
			}

			$success = TRUE;
			
			// Release all the locks held by this backend user
			foreach($result as $key => $val) {
				$lockid = $val['lockid'];
				
				if (($err_msg = $GLOBALS['SQ_SYSTEM']->releaseLock($lockid)) !== TRUE) {
					trigger_localised_error('SYS0109', E_USER_NOTICE, $lock_type, $asset->name, $err_msg);
					$success = FALSE;
					continue;
				}			
			}// End foreach
		}

		return $success;
	
	}

}//end class
?>
