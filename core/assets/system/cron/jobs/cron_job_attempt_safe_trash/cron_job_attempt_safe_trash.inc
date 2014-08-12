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
* $Id: cron_job_attempt_safe_trash.inc,v 1.7 2012/08/30 01:09:10 ewang Exp $
*
*/


require_once SQ_CORE_PACKAGE_PATH.'/system/cron/cron_job/cron_job.inc';

/**
* Cron_Job_Attempt_Safe_Trash
*
* Purpose
*
*
* @author  Blair Robertson <blair@squiz.net>
* @version $Revision: 1.7 $
* @package MySource_Matrix_Packages
* @subpackage __core__
*/
class Cron_Job_Attempt_Safe_Trash extends Cron_Job
{


	/**
	* Constructor
	*
	* @param int	$assetid	the asset id to be loaded
	*
	*/
	function __construct($assetid=0)
	{
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
				trigger_localised_error('CRON0018', E_USER_WARNING);
				return FALSE;
			}

			$link = $this->_tmp['asset_in_link'];
			$asset = $GLOBALS['SQ_SYSTEM']->am->getAsset($link['minorid'], $link['minor_type_code']);
			if (is_null($asset) || !$this->setAssetToUpdate($asset)) {
				trigger_localised_error('CRON0017', E_USER_WARNING);
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
			return translate('cron_job_assetless_attempt_safe_trash');
		} else if ($short_name) {
			return translate('cron_safe_trash_for', translate('asset_format', $asset->short_name, $asset->id));
		} else {
			return translate('cron_attempt_safe_trash_for', translate('asset_format', $asset->name, $asset->id));
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
	* For Attempt Safe Trash Jobs, admins of the asset to update can also remove jobs
	*
	* @return boolean
	* @access public
	*/
	public function canDelete()
	{
		if (parent::canDelete()) return TRUE;

		// read-only?
		if ($this->attr('read_only')) return FALSE;

		// TODO: will this restriction be allowed too?
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
			case 'type' :
				trigger_localised_error('CRON0038', E_USER_NOTICE);
				return FALSE;
			break;
		}//end if

		return parent::setAttrValue($name, $value);

	}//end setAttrValue()


	/**
	* Sets the asset that this cron job will be altering the status on
	*
	* @param Asset	$asset	a reference to the asset
	*
	* @return boolean
	* @access public
	*/
	public function setAssetToUpdate(Asset $asset)
	{
		if (!($asset instanceof Asset)) {
			trigger_localised_error('CRON0030', E_USER_WARNING);
			return FALSE;
		}

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
	* Get the asset that this cron job will be altering the status on
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

			// Can we safe trash?
			$safe_trash_enabled = $GLOBALS['SQ_SYSTEM']->getUserPrefs('user', 'SQ_USER_SAFE_TYPE3_TRASH');
			$safe_trash_errors = $GLOBALS['SQ_SYSTEM']->am->canSafeTrashAsset($asset->id);

			if ($safe_trash_enabled && !empty($safe_trash_errors)) {
			   $msg = 'Cannot trash '.$asset->id;
			   // We keep this job hanging around until we can delete it
			   return SQ_CRON_JOB_COMPLETED;
			} else {
			   $GLOBALS['SQ_SYSTEM']->am->trashAsset($asset->id);
			   $msg = 'Successfully trashed '.$asset->id;
			   // We can remove it now
			   return SQ_CRON_JOB_COMPLETED | SQ_CRON_JOB_REMOVE;
			}

		} else {
			// asset not found
			trigger_localised_error('CRON0020', E_USER_WARNING);
		}

		// we can be removed because the error is beyound our control
		return SQ_CRON_JOB_ERROR | SQ_CRON_JOB_REMOVE;

	}//end _exec()


}//end class

?>