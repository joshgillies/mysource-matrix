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
* $Id: cron_job_future_permission.inc,v 1.3 2012/08/30 01:09:11 ewang Exp $
*
*/


require_once SQ_CORE_PACKAGE_PATH.'/system/cron/cron_job/cron_job.inc';

/**
* Cron_Job_Future_Permission
*
* Purpose
*
*
* @author  Anh Ta <ata@squiz.co.uk>
* @version $Revision: 1.3 $
* @package MySource_Matrix_Packages
* @subpackage __core__
*/
class Cron_Job_Future_Permission extends Cron_Job
{


	/**
	* Constructor
	*
	* @param int	$assetid	the asset id to be loaded
	*
	*/
	function __construct($assetid=0)
	{
		$this->_ser_attrs = TRUE;
		parent::__construct($assetid);

	}//end constructor


	/**
	* Perform any additional processing required during the creation of this asset
	*
	* @param array	&$link	information used to create the initial link
	*
	* @return boolean
	* @access private
	*/
	protected function _createAdditional(Array &$link)
	{
		if (!parent::_createAdditional($link)) return FALSE;

		if (!empty($this->_tmp['asset_in_link'])) {
			if (!$GLOBALS['SQ_SYSTEM']->am->acquireLock($this->id, 'links')) {
				trigger_localised_error('CRON0059', translate('Unable to create future permission cron job, unable to acquire "links" lock'), E_USER_WARNING)
				return FALSE;
			}

			$link = $this->_tmp['asset_in_link'];
			$asset = $GLOBALS['SQ_SYSTEM']->am->getAsset($link['minorid'], $link['minor_type_code']);
			if (is_null($asset) || !$this->setAssetToUpdate($asset)) {
				trigger_localised_error('CRON0060', translate('Unable to create future permission cron job, Unable to set asset in link'), E_USER_WARNING)
				return FALSE;
			}

			unset($this->_tmp['asset_in_link']);
			$GLOBALS['SQ_SYSTEM']->am->releaseLock($this->id, 'links');
		}
		return TRUE;

	}//end _createAdditional()


	/**
	* Return name of the asset
	*
	* @param boolean	$short_name	whether or not we are after the shortname or the full name
	*
	* @return string
	* @access private
	* @see Asset::_getName()
	*/
	protected function _getName($short_name=FALSE)
	{
		$asset = $this->getAssetToUpdate();
		if (is_null($asset)) {
			return translate('Asset-less Future Permission Scheduled Job');

		} else if ($short_name) {
			return sprintf(translate('FP for %s'), sprintf(translate('"%1$s" [#%2$s]'), $asset->short_name, $asset->id));


		} else {
			return sprintf(translate('Future Permission for %s'), sprintf(translate('"%1$s" [#%2$s]'), $asset->name, $asset->id));


		}

	}//end _getName()


	/**
	* Returns an array of all the permitted links type, the type asset and the cardinality
	*
	* @return array
	* @access private
	* @see Asset::_getAllowLinks()
	*/
	public function _getAllowedLinks()
	{
		$links = parent::_getAllowedLinks();

		// make sure that we can link to every asset
		if (empty($links[SQ_LINK_NOTICE]['asset'])) {
			$links[SQ_LINK_NOTICE]['asset'] = Array('card' => 1);
		}

		return $links;

	}//end _getAllowedLinks()


	/**
	* Can this asset have its last significant link removed (putting it in the trash)?
	*
	* For Cron Jobs this can only be done by the running user or a sys admin
	* For Future Permission Jobs, admins of the asset to update can also remove jobs
	*
	* @return boolean
	* @access public
	*/
	public function canDelete()
	{
		if (parent::canDelete()) return TRUE;

		// read-only?
		if ($this->attr('read_only')) return FALSE;

		$asset = $this->getAssetToUpdate();
		if (!is_null($asset)) return $asset->adminAccess('');
		return FALSE;

	}//end canDelete()


	/**
	* Set the value for the passed variable to the passed value
	*
	* @param string	$name	the name of the attribute
	* @param string	$value	the new value of the attribute
	*
	* @return boolean
	* @access public
	*/
	public function setAttrValue($name, $value)
	{
		switch ($name) {
			case 'permission' :
				$value = (int) $value;
				if (!in_array($value, $this->_getAllPermissions())) {
					trigger_localised_error('CRON0061', translate('Unable to set the permission, it is not valid'), E_USER_NOTICE)
					return FALSE;
				}
			break;

			case 'type' :
				trigger_localised_error('CRON0062', translate('Unable to set the type of a future permission cron job; it can only be a one off event'), E_USER_NOTICE)
				return FALSE;
			break;
		}//end if

		return parent::setAttrValue($name, $value);

	}//end setAttrValue()


	/**
	* Return an array of all permissions that a future asset permission can be set to
	* of the form Array('[code_name]' => '[value]')
	*
	* @return array
	* @access private
	* @static
	*/
	protected static function _getAllPermissions()
	{
		return Array('read' => SQ_PERMISSION_READ, 'write' => SQ_PERMISSION_WRITE, 'admin' => SQ_PERMISSION_ADMIN);

	}//end _getAllPermissions()


	/**
	* Return an array of all descriptions of permissions in the system in the form Array('[value]' => '[description]')
	*
	* Used for the drop down boxes
	*
	* @return array
	* @access private
	* @static
	*/
	public static function getAllPermissionDescs()
	{
		static $descs = Array();

		if (empty($descs)) {
			$permissions = self::_getAllPermissions();
			foreach ($permissions as $name => $value) {
				$descs[$value] = ucfirst($name);
			}
		}

		return $descs;

	}//end getAllPermissionDescs()


	/**
	* Return the name of the permission that this future permission object is going to change the asset to
	*
	* @return string
	* @access public
	*/
	public function permissionName()
	{
		$descs = self::getAllPermissionDescs();
		return $descs[$this->attr('permission')];

	}//end permissionName()


	/**
	* Return the list of userids that will be granted permissions
	*
	* @param boolean	$grant	TRUE to get grant list, FALSE to get deny list
	*
	* @return array
	* @access public
	*/
	public function getGrantedUsers($grant = TRUE)
	{
		$grant_int = $grant? 1 : 0;
		$userid_grant_list = $this->attr('userids');
		$userids = Array();
		foreach ($userid_grant_list as $userid => $user_grant) {
			if ($user_grant == $grant_int) {
				$userids[] = $userid;
			}
		}

		return $userids;

	}//end getGrantedUsers()

	/**
	 * Return the name of the action of the cron job. It is either 'Add' or 'Remove'.
	 *
	 * @return string
	 * @access public
	 */
	public function actionName()
	{
		return ($this->attr('Add')? translate('Add') : translate('Remove'));



	}//end actionName()

	/**
	* Sets the asset that this cron job will be altering the permission on
	*
	* @param Asset	$asset	a reference to the asset
	*
	* @return boolean
	* @access public
	*/
	public function setAssetToUpdate(Asset $asset)
	{
		if ($this->id) {
			$existing_links = $GLOBALS['SQ_SYSTEM']->am->getLinks($this->id, SQ_LINK_NOTICE, '', TRUE, 'major', $value='updating');
			$link_exists = FALSE;
			foreach ($existing_links as $old_link) {
				if ($old_link['minorid'] == $asset->id) {
					$link_exists = TRUE;
				} else {
					$GLOBALS['SQ_SYSTEM']->am->deleteAssetLink($old_link['linkid']);
				}
			}
			if ($link_exists) {
				return TRUE;
			} else {
				return (bool)  $this->createLink($asset, SQ_LINK_NOTICE, 'updating');
			}
		} else {
			if (!isset($this->_tmp['asset_in_link'])) {
				$this->_tmp['asset_in_link'] = Array();
			}
			$this->_tmp['asset_in_link'] = Array('minorid' => $asset->id, 'minor_type_code' => $asset->type());
			return TRUE;
		}

	}//end setAssetToUpdate()


	/**
	* Get the asset that this cron job will be altering the permission on
	*
	* @return mixed object|NULL
	* @access public
	*/
	public function getAssetToUpdate()
	{
		// because we need to return by ref
		$null = NULL;

		if ($this->id) {
			$link = $GLOBALS['SQ_SYSTEM']->am->getLink($this->id, SQ_LINK_NOTICE, 'asset', FALSE, 'updating');
		} else {
			$link = (isset($this->_tmp['asset_in_link'])) ? $this->_tmp['asset_in_link'] : Array();
		}
		if (empty($link)) return $null;

		return $GLOBALS['SQ_SYSTEM']->am->getAsset($link['minorid'], $link['minor_type_code']);

	}//end getAssetToUpdate()


	/**
	* Actually executes the functionality for this job, called by run()
	*
	* Returns a bitmask of the SQ_CRON_JOB_* constants to tell the cron manager what to do
	*
	* @param string	&$msg	used to return any messages about completion etc from _exec
	*
	* @return int
	* @access private
	* @see Cron_Job::_exec(), Cron_Job::run()
	*/
	protected function _exec(&$msg)
	{
		$asset = $this->getAssetToUpdate();
		if (!is_null($asset)) {
			$add_permissions = $this->attr('add');
			$permission = $this->attr('permission');
			$userid_grant_list = $this->attr('userids');
			$cascade_to_new = $this->attr('cascade_to_new');
			$previous_access = NULL;
			$dependants_only = $this->attr('dependants_only');
			$permission_vars = Array();
			foreach ($userid_grant_list as $userid => $user_grant) {
				$granted = $add_permissions? $user_grant : -1;
				if (!$add_permissions) {
					$previous_access = $user_grant;
				}
				$permission_vars[] = Array(
												'permission'		=> $permission,
												'assetids'			=> Array($asset->id),
												'granted'			=> $granted,
												'userid'			=> $userid,
												'cascades'			=> $cascade_to_new,
												'previous_access'	=> $previous_access,
												'dependants_only'	=> !(!$dependants_only && $cascade_to_new), //consistency with the permission screen logic
											);
			}

			$hh = $GLOBALS['SQ_SYSTEM']->getHipoHerder();
			$vars = Array(
						'permission_changes'	=> $permission_vars,
					);
			$errors = $hh->freestyleHipo('hipo_job_edit_permissions', $vars);
			if (empty($errors)) {
				//success
				return SQ_CRON_JOB_COMPLETED | SQ_CRON_JOB_REMOVE;
			} else {
				// process failed
				trigger_localised_error('CRON0063', sprintf(translate('Failed changing Permission of "%1$s" (Id: #%2$s)'), $asset->name, $asset->id), E_USER_WARNING)
			}

		} else {
			// asset not found
			trigger_localised_error('CRON0064', translate('Unable to find asset to update the permission of'), E_USER_WARNING)
		}

		// we can be removed because the error is beyound our control
		return SQ_CRON_JOB_ERROR | SQ_CRON_JOB_REMOVE;

	}//end _exec()


	/**
	* Returns an array of active future permission jobs that are to be run on the passed asset
	* in chronological order
	*
	* @param Asset	$asset	the asset to get the future permission jobs for
	*
	* @return array
	* @access public
	* @static
	*/
	public static function &getActiveJobs(Asset $asset)
	{
		$links = $GLOBALS['SQ_SYSTEM']->am->getLinks($asset->id, SQ_LINK_NOTICE, 'cron_job_future_permission', TRUE, 'minor', 'updating');
		$assetids = Array();
		foreach ($links as $link) {
			$assetids[] = $link['majorid'];
		}

		sort($assetids);

		$fp_jobs = Array();
		foreach ($assetids as $assetid) {
			$fp_jobs[] = $GLOBALS['SQ_SYSTEM']->am->getAsset($assetid, 'cron_job_future_permission');
		}

		return $fp_jobs;

	}//end getActiveJobs()


}//end class

?>
