<?php
/**
* +--------------------------------------------------------------------+
* | Squiz.net Open Source Licence                                      |
* +--------------------------------------------------------------------+
* | Copyright (c), 2003 Squiz Pty Ltd (ABN 77 084 670 600).            |
* +--------------------------------------------------------------------+
* | This source file may be used subject to, and only in accordance    |
* | with, the Squiz Open Source Licence Agreement found at             |
* | http://www.squiz.net/licence.                                      |
* | Make sure you have read and accept the terms of that licence,      |
* | including its limitations of liability and disclaimers, before     |
* | using this software in any way. Your use of this software is       |
* | deemed to constitute agreement to be bound by that licence. If you |
* | modify, adapt or enhance this software, you agree to assign your   |
* | intellectual property rights in the modification, adaptation and   |
* | enhancement to Squiz Pty Ltd for use and distribution under that   |
* | licence.                                                           |
* +--------------------------------------------------------------------+
*
* $Id: cron_job_future_lineage.inc,v 1.21.2.1 2006/06/27 01:34:11 lwright Exp $
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
* @version $Revision: 1.21.2.1 $
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
	function Cron_Job_Future_Lineage($assetid=0)
	{
		$this->_ser_attrs = TRUE;
		$this->Cron_Job($assetid);

	}//end constructor


	/**
	* Perform any additional processing required during the creation of this asset
	*
	* @param array	&$link	information used to create the initial link
	*
	* @return boolean
	* @access private
	*/
	function _createAdditional(&$link)
	{
		if (!parent::_createAdditional($link)) return FALSE;

		if (!empty($this->_tmp['asset_in_link'])) {
			if (!$GLOBALS['SQ_SYSTEM']->am->acquireLock($this->id, 'links')) {
				trigger_localised_error('CRON0018', E_USER_WARNING);
				return FALSE;
			}

			foreach ($this->_tmp['asset_in_link'] as $side_of_link => $link) {
				$asset =& $GLOBALS['SQ_SYSTEM']->am->getAsset($link['minorid'], $link['minor_type_code']);
				if (is_null($asset) || !$this->setAssetInLink($asset, $side_of_link)) {
					trigger_localised_error('CRON0017', E_USER_WARNING);
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
	function _getName($short_name=FALSE)
	{
		$minor =& $this->getAssetInLink('minor');
		if (is_null($minor)) {
			return translate('cron_job_asset-less_future_linking');
		} else if ($short_name) {
			return translate('cron_fl_for', translate('asset_format', $minor->short_name, $minor->id));
		} else {
			return translate('cron_future_linking_for', translate('asset_format', $minor->name, $minor->id));
		}//end if

	}//end _getName()


	/**
	* Returns an array of all the permitted links type, the type asset and the cardinality
	*
	* @return array
	* @access private
	* @see Asset::_getAllowLinks()
	*/
	function _getAllowedLinks()
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
	function canDelete()
	{
		if (parent::canDelete()) return TRUE;

		$minor =& $this->getAssetInLink('minor');
		if (!is_null($minor)) return $minor->adminAccess('');
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
	function setAttrValue($name, $value)
	{
		switch ($name) {
			case 'type':
				trigger_localised_error('CRON0039', E_USER_NOTICE);
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
	* @param object	&$asset			a reference to the asset
	* @param string	$side_of_link	the side of the link that this asset is to be - [major|minor]
	*
	* @return boolean
	* @access public
	*/
	function setAssetInLink(&$asset, $side_of_link)
	{
		if ($side_of_link != 'major' && $side_of_link != 'minor') {
			trigger_localised_error('CRON0033', E_USER_WARNING, $side_of_link);
			return FALSE;
		}

		if (!is_a($asset, 'asset')) {
			trigger_localised_error('CRON0030', E_USER_WARNING);
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
	function &getAssetInLink($side_of_link)
	{
		$null = NULL; // because we need to return by ref
		if ($side_of_link != 'major' && $side_of_link != 'minor') {
			trigger_localised_error('CRON0033', E_USER_WARNING, $side_of_link);
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
	function readableDescription()
	{
		$minor =& $this->getAssetInLink('minor');
		if (is_null($minor)) {
			return translate('no_minor_asset_found');
		}

		$str = '';

		if ($this->attr('delete_linkid')) {
			$str .= translate('deleting_link_id', $this->attr('delete_linkid'));
			$link = $GLOBALS['SQ_SYSTEM']->am->getLinkById($this->attr('delete_linkid'));

			if (!empty($link)) {
				$old_major =& $GLOBALS['SQ_SYSTEM']->am->getAsset($link['majorid'], $link['major_type_code']);
				if (!is_null($old_major)) {
					$str = ' - '.translate('asset_from_under_asset', translate('asset_format', $minor->name, $minor->id), translate('asset_format', $old_major->name, $old_major->id));
				}
			}
		}

		$new_major =& $this->getAssetInLink('major');

		if ($this->attr('delete_linkid') && !is_null($new_major)) {
			$str .= "\n";
		}

		if (!is_null($new_major)) {
			$str .= translate('create_new_link_for', translate('asset_format', $minor->name, $minor->id), translate('asset_format', $new_major->name, $new_major->id));
		}

		return $str;

	}//end readableDescription()


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
	function _exec(&$msg)
	{
		// we can be removed because the error is beyound our control
		$failed_ret_val = SQ_CRON_JOB_ERROR | SQ_CRON_JOB_REMOVE;

		// need to get this here because we are could be deleting the link below
		$desc = $this->readableDescription();

		$minor =& $this->getAssetInLink('minor');
		if (is_null($minor)) {
			trigger_localised_error('CRON0019', E_USER_WARNING);
			return $failed_ret_val;
		}

		$GLOBALS['SQ_SYSTEM']->changeDatabaseConnection('db2');
		$GLOBALS['SQ_SYSTEM']->doTransaction('BEGIN');

		$delete_link_success = TRUE;
		if ($this->attr('delete_link_all') == TRUE) {
			$links = $GLOBALS['SQ_SYSTEM']->am->getLinks($minor->id, SQ_SC_LINK_SIGNIFICANT, '', TRUE, 'minor');
			foreach ($links as $link) {
				if (!$delete_link_success = $this->_execDeleteLink($link['linkid'])) {
					break;
				}//end if
			}
		} else if ($this->attr('delete_linkid') > 0) {
			$delete_link_success = $this->_execDeleteLink($this->attr('delete_linkid'));
		}//end if

		if (!$delete_link_success) return $failed_ret_val;

		$new_major =& $this->getAssetInLink('major');
		if (!is_null($new_major)) {

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

			$linkid = $new_major->createLink($minor,
											 $link_info['link_type'],
											 $link_info['value'],
											 $link_info['sort_order'],
											 $link_info['is_dependant'],
											 $link_info['is_exclusive']
											);

			$create_ok = ($linkid > 0);

			if (!$create_ok) {
				trigger_localised_error('CRON0004', E_USER_WARNING, $new_major->name, $new_major->id);
				$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
				$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();
				return $failed_ret_val;
			}

		}//end if

		$hh =& $GLOBALS['SQ_SYSTEM']->getHipoHerder();
		$vars = Array('assetids' => Array($minor->id));
		$lookup_errors = $hh->freestyleHipo('hipo_job_update_lookups', $vars);
		if (!empty($lookup_errors)) {
			trigger_localised_error('CRON0008', E_USER_WARNING, $minor->name, $minor->id);
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
	function _execDeleteLink($linkid)
	{
		$delete_ok = FALSE;

		$link = $GLOBALS['SQ_SYSTEM']->am->getLinkById($linkid);
		if (!empty($link)) {
			$old_major =& $GLOBALS['SQ_SYSTEM']->am->getAsset($link['majorid'], $link['major_type_code']);
			if (!is_null($old_major) && $GLOBALS['SQ_SYSTEM']->am->acquireLock($old_major->id, 'links')) {
				$delete_ok = $old_major->deleteLink($linkid, TRUE);
			}
		}

		if (!$delete_ok) {
			trigger_localised_error('CRON0007', E_USER_WARNING, $linkid);
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
	* @param object	&$asset	the asset to get the future status jobs for
	*
	* @return array
	* @access public
	* @static
	*/
	function &getActiveJobs(&$asset)
	{
		$cron_mgr =& $GLOBALS['SQ_SYSTEM']->am->getSystemAsset('cron_manager');
		if (is_null($cron_mgr)) {
			$blank = Array();
			return $blank;
		}

		// get all active future lineage jobs
		$all_jobs = $cron_mgr->getJobs(__CLASS__, FALSE);

		$dates = Array();
		for ($i = 0, $total = count($all_jobs); $i < $total; $i++) {

			$link = $GLOBALS['SQ_SYSTEM']->am->getLink($all_jobs[$i]->id, SQ_LINK_NOTICE, 'asset', FALSE, 'minor_asset');
			if (!empty($link) && $link['minorid'] == $asset->id) {
				$dates[$i] = $all_jobs[$i]->attr('when');
			}

		}//end for

		asort($dates, SORT_STRING);
		$assets_jobs = Array();

		foreach ($dates as $i => $date) {
			$assets_jobs[] =& $all_jobs[$i];
		}//end for

		return $assets_jobs;

	}//end getActiveJobs()


}//end class

?>
