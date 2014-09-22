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
* $Id: cron_job_future_lineage.inc,v 1.40 2013/01/21 08:03:35 cupreti Exp $
*
*/


require_once SQ_CORE_PACKAGE_PATH.'/system/cron/cron_job/cron_job.inc';

/**
* Cron_Job_Future_Lineage
*
* Purpose
*
*
* @author  Blair Robertson <blair@squiz.net>
* @version $Revision: 1.40 $
* @package MySource_Matrix_Packages
* @subpackage __core__
*/
class Cron_Job_Future_Lineage extends Cron_Job
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
				trigger_localised_error('CRON0018', translate('Unable to create future lineage cron job, unable to acquire "links" lock'), E_USER_WARNING)
				return FALSE;
			}

			foreach ($this->_tmp['asset_in_link'] as $side_of_link => $link) {
				$asset = $GLOBALS['SQ_SYSTEM']->am->getAsset($link['minorid'], $link['minor_type_code']);
				if (is_null($asset) || !$this->setAssetInLink($asset, $side_of_link)) {
					trigger_localised_error('CRON0017', translate('Unable to create future lineage cron job, Unable to set asset in link'), E_USER_WARNING)
					return FALSE;
				}
			}
			unset($this->_tmp['asset_in_link']);
			$GLOBALS['SQ_SYSTEM']->am->releaseLock($this->id, 'links');
		}
		return TRUE;

	}//end _createAdditional()


	/**
	* Returns name of the asset
	*
	* @param boolean	$short_name	whether or not we are after the shortname or the full name
	*
	* @return string
	* @access private
	* @see Asset::_getName()
	*/
	protected function _getName($short_name=FALSE)
	{
		$asset = $this->attr('reverse_mode') ? $this->getAssetInLink('major') : $this->getAssetInLink('minor');
		if (is_null($asset)) {
			return translate('Asset-less Future Linking Scheduled Job');

		} else if ($short_name) {
			return sprintf(translate('FL for %s'), sprintf(translate('"%1$s" [#%2$s]'), $asset->short_name, $asset->id));


		} else {
			return sprintf(translate('Future Linking for %s'), sprintf(translate('"%1$s" [#%2$s]'), $asset->name, $asset->id));


		}//end if

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
			$links[SQ_LINK_NOTICE]['asset'] = Array('card' => 2);
		}

		return $links;

	}//end _getAllowedLinks()


	/**
	* Can this asset have its last significant link removed (putting it in the trash)?
	* For Cron Jobs this can only be done by the running user or a sys admin
	* For Future Lineage Jobs, admins of the minor asset to update can also remove jobs
	*
	* @return boolean
	* @access public
	*/
	public function canDelete()
	{
		if (parent::canDelete()) return TRUE;

		$asset = $this->attr('reverse_mode') ? $this->getAssetInLink('major') : $this->getAssetInLink('minor');
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
			case 'type':
				trigger_localised_error('CRON0039', translate('Unable to set the type of a future status cron job; it can only be a one off event'), E_USER_NOTICE)
				return FALSE;
			break;
			case 'delete_linkid':
				if ($value > 0) {
					$this->setAttrValue('delete_link_all', FALSE);
				}
			break;
			case 'delete_link_all':
				if ($value == TRUE) {
					$this->setAttrValue('delete_linkid', 0);
				}
			break;
		}

		return parent::setAttrValue($name, $value);

	}//end setAttrValue()


	/**
	* Sets the asset that this cron job will be altering the status on
	*
	* @param Asset	$asset			a reference to the asset
	* @param string	$side_of_link	the side of the link that this asset is to be - [major|minor]
	*
	* @return boolean
	* @access public
	*/
	public function setAssetInLink(Asset $asset, $side_of_link)
	{
		if ($side_of_link != 'major' && $side_of_link != 'minor') {
			trigger_localised_error('CRON0033', sprintf(translate('Unknown Side of Link "%s"'), $side_of_link), E_USER_WARNING)
			return FALSE;
		}

		if (!($asset instanceof Asset)) {
			trigger_localised_error('CRON0030', translate('Unable to set asset to update, the asset must be a asset object'), E_USER_WARNING)
			return FALSE;
		}

		if ($this->id) {
			return (bool) $this->createLink($asset, SQ_LINK_NOTICE, $side_of_link.'_asset');
		} else {
			if (!isset($this->_tmp['asset_in_link'])) {
				$this->_tmp['asset_in_link'] = Array();
			}
			$this->_tmp['asset_in_link'][$side_of_link] = Array('minorid' => $asset->id, 'minor_type_code' => $asset->type());
			return TRUE;
		}

	}//end setAssetInLink()


	/**
	* Gets the user that this cron job is to run as while executing
	*
	* @param string	$side_of_link	the side of the link that this asset is to be - [major|minor]
	*
	* @return mixed object|NULL
	* @access public
	*/
	public function getAssetInLink($side_of_link)
	{
		$null = NULL; // because we need to return by ref
		if ($side_of_link != 'major' && $side_of_link != 'minor') {
			trigger_localised_error('CRON0033', sprintf(translate('Unknown Side of Link "%s"'), $side_of_link), E_USER_WARNING)
			return $null;
		}

		if ($this->id) {
			$link = $GLOBALS['SQ_SYSTEM']->am->getLink($this->id, SQ_LINK_NOTICE, 'asset', FALSE, $side_of_link.'_asset');
		} else {
			$link = (isset($this->_tmp['asset_in_link'][$side_of_link])) ? $this->_tmp['asset_in_link'][$side_of_link] : Array();
		}

		if (empty($link)) return $null;
		return $GLOBALS['SQ_SYSTEM']->am->getAsset($link['minorid'], $link['minor_type_code']);

	}//end getAssetInLink()


	/**
	* Returns a description of what this future lineage job is going to do
	*
	* @return string
	* @access public
	*/
	public function readableDescription()
	{
		if($this->attr('reverse_mode')) return $this->_readableDescriptionReverse();

		$minor = $this->getAssetInLink('minor');


		if (is_null($minor)) {
			return translate('No Minor Asset Found');

		}


		$str = '';

		if ($this->attr('delete_linkid')) {
			$str .= sprintf(translate('Deleting Link Id #%s'), $this->attr('delete_linkid'));

			$link = $GLOBALS['SQ_SYSTEM']->am->getLinkById($this->attr('delete_linkid'));

			if (!empty($link)) {
				$old_major = $GLOBALS['SQ_SYSTEM']->am->getAsset($link['majorid'], $link['major_type_code']);
				if (!is_null($old_major)) {
					$str = sprintf(translate('Delete link for %1$s under %2$s'), sprintf(translate('"%1$s" [#%2$s]'), $minor->name, $minor->id), sprintf(translate('"%1$s" [#%2$s]'), $old_major->name, $old_major->id));


				}
			}
		}

		$new_major = $this->getAssetInLink('major');

		if ($this->attr('delete_linkid') && !is_null($new_major)) {
			$str .= "\n";
		}

		if (!is_null($new_major)) {
			$str .= sprintf(translate('Creating a new link for %1$s under %2$s'), sprintf(translate('"%1$s" [#%2$s]'), $minor->name, $minor->id), sprintf(translate('"%1$s" [#%2$s]'), $new_major->name, $new_major->id));


		}

		return $str;

	}//end readableDescription()

	/**
	* Returns a description of what this future lineage job is going to do for reverse mode
	*
	* @return string
	* @access public
	*/
	public function _readableDescriptionReverse()
	{
		$major = $this->getAssetInLink('major');
		$new_minor =  $this->getAssetInLink('minor');

		if(is_null($major)){
			return translate('No Major Asset Found');

		}

		$str = '';

		if ($this->attr('delete_linkid')) {
			$str .= sprintf(translate('Deleting Link Id #%s'), $this->attr('delete_linkid'));

			$link = $GLOBALS['SQ_SYSTEM']->am->getLinkById($this->attr('delete_linkid'));

			if (!empty($link)) {
				$old_minor = $GLOBALS['SQ_SYSTEM']->am->getAsset($link['minorid'], $link['minor_type_code']);
				if (!is_null($old_minor)) {
					$str = sprintf(translate('Delete link for %1$s under %2$s'), sprintf(translate('"%1$s" [#%2$s]'), $old_minor->name, $old_minor->id), sprintf(translate('"%1$s" [#%2$s]'), $major->name, $major->id));


				}

			}
		}


		if ($this->attr('delete_linkid') && !is_null($new_minor)) {
			$str .= "\n";
		}

		if (!is_null($new_minor)) {
			$str .= sprintf(translate('Creating a new link for %1$s under %2$s'), sprintf(translate('"%1$s" [#%2$s]'), $new_minor->name, $new_minor->id), sprintf(translate('"%1$s" [#%2$s]'), $major->name, $major->id));


		}

		return $str;

	}//end _readableDescriptionReverse()


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
		// we can be removed because the error is beyound our control
		$failed_ret_val = SQ_CRON_JOB_ERROR | SQ_CRON_JOB_REMOVE;

		// need to get this here because we are could be deleting the link below

		$desc = $this->readableDescription();
		$major = $this->getAssetInLink('major');
		$minor = $this->getAssetInLink('minor');

		//in reverse mode, the current asset is major asset
		$main_asset = $this->attr('reverse_mode') ? $major : $minor;
		$target_asset = $this->attr('reverse_mode') ? $minor : $major;
		$side_of_link = $this->attr('reverse_mode') ? 'major' : 'minor';


		if (is_null($main_asset)) {
			trigger_localised_error('CRON0019', translate('Unable to find Minor Asset to use to update the lineage'), E_USER_WARNING)
			return $failed_ret_val;
		}


		// if we are moving to the trash folder, we need to check if the safe trash pref has been enabled

		$safe_trash = $GLOBALS['SQ_SYSTEM']->getUserPrefs('user', 'SQ_USER_SAFE_TYPE3_TRASH');
		$can_safe_trash = TRUE;

		if ($safe_trash) {
			$exclude_self = FALSE;
			//if minor asset is empty, it must be deletion of children links
			// so have to check the minor asset/assets
			if(empty($minor)){
				if ($this->attr('delete_link_all')){
					$minor = $major;
					$exclude_self = TRUE;
				}
				else if($this->attr('delete_linkid') > 0){
					$link = $GLOBALS['SQ_SYSTEM']->am->getLinkById($this->attr('delete_linkid'));
					$minor = $GLOBALS['SQ_SYSTEM']->am->getAsset($link['minorid'], $link['minor_type_code']);
				}

			}

			$can_safe_trash = $this->_canSafeTrash($minor, $exclude_self);
		}

		$existing_links = Array();
		if ($this->attr('delete_link_all') == TRUE && $can_safe_trash) {
			$links = $GLOBALS['SQ_SYSTEM']->am->getLinks($main_asset->id, SQ_SC_LINK_SIGNIFICANT, '', TRUE, $side_of_link);
			foreach ($links as $link) {
				$existing_links[] = $link['linkid'];
			}
		} else if ($this->attr('delete_linkid') > 0 && $can_safe_trash) {
			$existing_links[] = $this->attr('delete_linkid');
		}//end if

		$GLOBALS['SQ_SYSTEM']->changeDatabaseConnection('db2');
		$GLOBALS['SQ_SYSTEM']->doTransaction('BEGIN');

		if (!is_null($target_asset)) {
			// if moving to the trash, and it is not safe to do so, throw an error
			$trash_folder = $GLOBALS['SQ_SYSTEM']->am->getSystemAsset('trash_folder');
			if ($major->id == $trash_folder->id && !$can_safe_trash) {
				trigger_localised_error('CRON0051', sprintf(translate('Unable to set future lineage, asset %s is not safe to be trashed.'), $minor->id), E_USER_WARNING)
				$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
				$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();
				return $failed_ret_val;
			}

			$link_info = $this->attr('link_info');
			if (!isset($link_info['value'])) {
				$link_info['value'] = '';
			}
			if (!isset($link_info['sort_order'])) {
				$link_info['sort_order'] = NULL;
			}
			if (!isset($link_info['is_dependant'])) {
				$link_info['is_dependant'] = '0';
			}
			if (!isset($link_info['is_exclusive'])) {
				$link_info['is_exclusive'] = '0';
			}

			$linkid = $major->createLink($minor,
											 $link_info['link_type'],
											 $link_info['value'],
											 $link_info['sort_order'],
											 $link_info['is_dependant'],
											 $link_info['is_exclusive']
											);

			$create_ok = ($linkid > 0);

			if (!$create_ok) {
				trigger_localised_error('CRON0004', sprintf(translate('Failed to create New Link under "%1$s" (Id: #%2$s)'), $major->name, $major->id), E_USER_WARNING)
				$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
				$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();
				return $failed_ret_val;
			}

		}//end if

		// Now delete the previous links
		$delete_link_success = TRUE;
		foreach ($existing_links as $linkid) {
			if (!$delete_link_success = $this->_execDeleteLink($linkid)) {
				break;
			}//end if
		}
		if (!$delete_link_success) {
			return $failed_ret_val;
		}

		//update lookups for all assets involved
		if(!empty($minor)){
			$this->_tmp['_update_lookups_assets'][] = $minor->id;
		}
		$hh = $GLOBALS['SQ_SYSTEM']->getHipoHerder();

		$vars = Array('assetids' => $this->_tmp['_update_lookups_assets']);
		$lookup_errors = $hh->freestyleHipo('hipo_job_update_lookups', $vars);
		if (!empty($lookup_errors)) {
			trigger_localised_error('CRON0008', sprintf(translate('Future Lineage of "%1$s" (Id: #%2$s) failed updating lookups'), $main_asset->name, $main_asset->id), E_USER_WARNING)
			$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
			$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();
			return $failed_ret_val;
		}

		$msg = "Successful in \n".$desc;
		$GLOBALS['SQ_SYSTEM']->doTransaction('COMMIT');
		$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();

		// all OK, and we can be removed because we have done our one off task
		return SQ_CRON_JOB_COMPLETED | SQ_CRON_JOB_REMOVE;

	}//end _exec()


	/**
	* Given the ID of a link, attempts to delete it.
	*
	* @param int	$linkid	The ID of the link to delete
	*
	* @return boolean
	* @access private
	*/
	protected function _execDeleteLink($linkid)
	{
		$delete_ok = FALSE;
		$link = $GLOBALS['SQ_SYSTEM']->am->getLinkById($linkid);

		if (!empty($link)) {
			$old_major = $GLOBALS['SQ_SYSTEM']->am->getAsset($link['majorid'], $link['major_type_code']);
			//keep all minor assets which must need a lookup update later
			$minor = $GLOBALS['SQ_SYSTEM']->am->getAsset($link['minorid'], $link['minor_type_code']);
			$this->_tmp['_update_lookups_assets'][] = $minor->id;
			if (!is_null($old_major) && $GLOBALS['SQ_SYSTEM']->am->acquireLock($old_major->id, 'links')) {
				$delete_ok = $old_major->deleteLink($linkid, TRUE);
			}
		}

		if (!$delete_ok) {
			trigger_localised_error('CRON0007', sprintf(translate('Delete of Link #%s Failed'), $linkid), E_USER_WARNING)
			$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
			$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();
			return FALSE;
		}
		return TRUE;

	}//end _execDeleteLink()


	/**
	* Returns an array of active future lineage jobs that are to be run on the passed asset
	* returned in chronological order
	*
	* @param Asset	$asset	the asset to get the future status jobs for
	* @param Boolean $reverse_mode get active jobs for linked parents or children
	*
	* @return array
	* @access public
	* @static
	*/
	public static function &getActiveJobs(Asset $asset, $reverse_mode = FALSE)
	{
		$cron_mgr = $GLOBALS['SQ_SYSTEM']->am->getSystemAsset('cron_manager');
		if (is_null($cron_mgr)) {
			$blank = Array();
			return $blank;
		}

		// get all active future lineage jobs
		$all_jobs = $cron_mgr->getJobs(strtolower(__CLASS__), FALSE);

		$dates = Array();
		for ($i = 0, $total = count($all_jobs); $i < $total; $i++) {
			$link = $GLOBALS['SQ_SYSTEM']->am->getLink($all_jobs[$i]->id, SQ_LINK_NOTICE, 'asset', FALSE, $reverse_mode ? 'major_asset' : 'minor_asset');
			if (!empty($link) && $link['minorid'] == $asset->id) {
				$dates[$i] = $all_jobs[$i]->attr('when');
			}

		}//end for

		asort($dates, SORT_STRING);
		$assets_jobs = Array();

		foreach ($dates as $i => $date) {
			$assets_jobs[] = $all_jobs[$i];
		}//end for

		return $assets_jobs;

	}//end getActiveJobs()


	/**
	* Given the $minor asset, check if this asset can be safe trashed
	*
	* @param Asset	 $minor the asset to be checked
	* @param Boolean $exclude_self exclude the asset itself while checking safe trash status
	*
	* @return boolean
	* @access protected
	*/
	protected function _canSafeTrash ($minor, $exclude_self = FALSE) {
		$can_safe_trash = TRUE;
		if(!$exclude_self){
			// safe trash is enabled, and asset is LIVE
			if ($minor->status >= SQ_STATUS_LIVE ) {
				$can_safe_trash = FALSE;
			}
			// safe trash is enabled, and there are notice link(s) to other assets
			$affected_links = $GLOBALS['SQ_SYSTEM']->am->getLinks($minor->id, SQ_LINK_TYPE_3 + SQ_LINK_NOTICE, '', TRUE, 'minor');
			foreach ($affected_links as $info) {
				// make sure that we are not treating notice link to this future cron job as problematic link
				if ($info['major_type_code'] != $this->type()) {
					$can_safe_trash = FALSE;
					break;
				}
			}
		}

		// check the same thing for all children
		$ret_val = $GLOBALS['SQ_SYSTEM']->am->generateGetChildrenQuery($minor, '', TRUE, FALSE);
		$children  = $ret_val['sql_array'];
		$bind_vars = $ret_val['bind_vars'];

		if (!empty($children)) {
			// modify the selection so that we only get 1 column
			$children['select'] = '(SELECT DISTINCT l.minorid';
			if (isset($children['union_select'])) {
				$children['union_select'] = 'UNION ALL SELECT sl.minorid';
			}

			$sql = 'SELECT
						majorid, minorid, name, linkid, status, link_type
					FROM
						'.SQ_TABLE_RUNNING_PREFIX.'ast_lnk lnk
						INNER JOIN '.SQ_TABLE_RUNNING_PREFIX.'ast ast ON (ast.assetid = lnk.minorid)
					WHERE
						(
							(link_type = :link_type_3 OR link_type = :link_notice) OR
							(status >= :status_live)
						) AND
						assetid IN ('.implode(' ', $children).')';

			try {
				$query = MatrixDAL::preparePdoQuery($sql);
				foreach ($bind_vars as $bind_var => $bind_value) {
					MatrixDAL::bindValueToPdo($query, $bind_var, $bind_value);
				}
				MatrixDAL::bindValueToPdo($query, 'link_type_3', SQ_LINK_TYPE_3);
				MatrixDAL::bindValueToPdo($query, 'link_notice', SQ_LINK_NOTICE);
				MatrixDAL::bindValueToPdo($query, 'status_live', SQ_STATUS_LIVE);
				$result = MatrixDAL::executePdoAll($query);
			} catch (DALException $e) {
				throw new Exception ('Unable to check if asset "'.$minor->name.'" (#'.$minor->id.') can be safe-trashed due to database error: '.$e->getMessage());
			}
			if (!empty($result)) {
				$can_safe_trash = FALSE;
			}
		}
		return $can_safe_trash;
	}//end _canSafeTrash()


}//end class

?>
