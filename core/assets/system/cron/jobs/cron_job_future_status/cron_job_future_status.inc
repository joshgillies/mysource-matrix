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
* $Id: cron_job_future_status.inc,v 1.33 2005/12/19 00:18:33 bcaldwell Exp $
*
*/


require_once SQ_CORE_PACKAGE_PATH.'/system/cron/cron_job/cron_job.inc';

/**
* Cron_Job_Future_Status
*
* Purpose
*
*
* @author  Blair Robertson <blair@squiz.net>
* @version $Revision: 1.33 $
* @package MySource_Matrix_Packages
* @subpackage __core__
*/
class Cron_Job_Future_Status extends Cron_Job
{


	/**
	* Constructor
	*
	* @param int	$assetid	the asset id to be loaded
	*
	*/
	function Cron_Job_Future_Status($assetid=0)
	{
		$this->Cron_Job($assetid);

	}//end constructor


	/**
	* Return name of the asset
	*
	* @param bool	$short_name	whether or not we are after the shortname or the full name
	*
	* @see Asset::_getName()
	* @return string
	* @access private
	*/
	function _getName($short_name=false)
	{
		$asset = &$this->getAssetToUpdate();
		if (is_null($asset)) {
			return translate('cron_job_asset_less_future_status');
		} else if ($short_name) {
			return translate('cron_fs_for', translate('asset_format', $asset->short_name, $asset->id));
		} else {
			return translate('cron_future_status_for', translate('asset_format', $asset->name, $asset->id));
		}

	}//end _getName()


	/**
	* Returns an array of all the permitted links type, the type asset and the cardinality
	*
	* @see Asset::_getAllowLinks()
	* @return array
	* @access private
	*/
	function _getAllowedLinks()
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
	* For Future Status Jobs, admins of the asset to update can also remove jobs
	*
	* @return boolean
	* @access public
	*/
	function canDelete()
	{
		if (parent::canDelete()) return true;

		// read-only?
		if ($this->attr('read_only')) {
			return false;
		}

		$asset = &$this->getAssetToUpdate();
		if (!is_null($asset)) return $asset->adminAccess('');
		return false;

	}//end canDelete()


	/**
	* Set the value for the passed variable to the passed value
	*
	* @param string		$name				the name of the attribute
	* @param string		$value				the new value of the attribute
	*
	* @return boolean
	* @access public
	*/
	function setAttrValue($name, $value)
	{
		switch ($name) {
			case 'status' :
				$value = (int) $value;
				if (!in_array($value, array_keys($this->_getAllStatii()))) {
					trigger_localised_error('CRON0031', E_USER_NOTICE);
					return false;
				}
				break;

			case 'type' :
				trigger_localised_error('CRON0038', E_USER_NOTICE);
				return false;
				break;
		}//end if

		return parent::setAttrValue($name, $value);

	}//end setAttrValue()


	/**
	* Return an array of all statii that a future page status can be set to
	*
	* @return array of the form Array('[value]' => '[code_name]')
	* @access private
	* @static
	*/
	function _getAllStatii()
	{
		return Array(
				SQ_STATUS_ARCHIVED				=> 'ARCHIVED',
				SQ_STATUS_UNDER_CONSTRUCTION	=> 'UNDER_CONSTRUCTION',
				SQ_STATUS_LIVE					=> 'LIVE',
				SQ_STATUS_LIVE_APPROVAL			=> 'LIVE_APPROVAL',
			   );

	}//end _getAllStatii()


	/**
	* Return an array of all statii in the system in the form Array('[value]' => '[description]')
	*
	* Used for the drop down boxes
	*
	* @return array
	* @access private
	* @static
	*/
	function _getAllStatiiDescs()
	{
		static $descs = Array();

		if (empty($descs)) {
			$descs['']  = '[Select Status]';
			$statii = Cron_Job_Future_Status::_getAllStatii();
			foreach ($statii as $value => $code) {
				$code = strtolower($code);
				$status_code = 'asset_status_'.$code;
				require_once SQ_INCLUDE_PATH.'/asset_status/'.$status_code.'.inc';
				eval('$descs[$value] = '.$status_code.'::getDescription();');
			}
		}

		return $descs;

	}//end _getAllStatiiDescs()


	/**
	* Return the name of the status that this future status object is going to change the asset to
	*
	* @return string
	* @access public
	*/
	function statusName()
	{
		if (!$this->attr('status')) return '[ No Status Set ]';
		$statii = Cron_Job_Future_Status::_getAllStatii();
		$code = strtolower($statii[$this->attr('status')]);
		$status_code = 'asset_status_'.$code;
		require_once SQ_INCLUDE_PATH.'/asset_status/'.$status_code.'.inc';
		eval('$desc = '.$status_code.'::getDescription();');
		return $desc;

	}//end statusName()


	/**
	* Sets the asset that this cron job will be altering the status on
	*
	* @param object Asset	$asset		a reference to the asset
	*
	* @return boolean
	* @access public
	*/
	function setAssetToUpdate(&$asset)
	{
		if (!is_a($asset, 'asset')) {
			trigger_localised_error('CRON0030', E_USER_WARNING);
			return false;
		}

		return (bool) $this->createLink($asset, SQ_LINK_NOTICE, 'updating');

	}//end setAssetToUpdate()


	/**
	* Get the asset that this cron job will be altering the status on
	*
	* @return mixed object Asset or NULL on error
	* @access public
	*/
	function &getAssetToUpdate()
	{
		$null = null; // because we need to return by ref

		$link = $GLOBALS['SQ_SYSTEM']->am->getLink($this->id, SQ_LINK_NOTICE, 'asset', false, 'updating');
		if (empty($link)) return $null;

		return $GLOBALS['SQ_SYSTEM']->am->getAsset($link['minorid'], $link['minor_type_code']);

	}//end getAssetToUpdate()


	/**
	* Actually executes the functionality for this job, called by run()
	*
	* @param string	$msg	accepted by reference, used to return any messages about completion etc from _exec
	*
	* @return int	bitmask of the SQ_CRON_JOB_* constants to tell the cron manager what to do
	* @see Cron_Job::_exec(), Cron_Job::run()
	* @access private
	*/
	function _exec(&$msg)
	{
		$asset = &$this->getAssetToUpdate();
		if (!is_null($asset)) {

			$old_status = $asset->status;

			// process the status change
			$hh = &$GLOBALS['SQ_SYSTEM']->getHipoHerder();
			$vars = Array('assetid' => $asset->id, 'new_status' => $this->attr('status'), 'dependants_only' => $this->attr('dependants_only'));
			
			// we might want to do a status change for all children too
			/*if ($this->attr('cascade'))*/ //$vars['dependants_only'] = 0;
			
			$status_errors = $hh->freestyleHipo('hipo_job_edit_status', $vars);
			if (empty($status_errors)) {
				// all OK, and we can be removed because we have done our one off task
				$msg = translate('cron_altered_asset_status', translate('asset_format', $asset->name, $asset->id), $this->statusName($old_status), $this->statusName($asset->status));
				return SQ_CRON_JOB_COMPLETED | SQ_CRON_JOB_REMOVE;
			} else {
				// process failed
				trigger_localised_error('CRON0003', E_USER_WARNING, $asset->name, $asset->id);
			}

		} else {
			// asset not found
			trigger_localised_error('CRON0020', E_USER_WARNING);
		}

		// we can be removed because the error is beyound our control
		return SQ_CRON_JOB_ERROR | SQ_CRON_JOB_REMOVE;

	}//end _exec()


	/**
	* Returns an array of active future status jobs that are to be run on the passed asset
	* in chronological order
	*
	* @param object Asset	$asset	the asset to get the future status jobs for
	*
	* @return Array() returns an array of references to the Future Status jobs
	* @access public
	* @static
	*/
	function &getActiveJobs(&$asset)
	{
		$links = $GLOBALS['SQ_SYSTEM']->am->getLinks($asset->id, SQ_LINK_NOTICE, __CLASS__, true, 'minor', 'updating');
		$assetids = Array();
		$cm = $GLOBALS['SQ_SYSTEM']->am->getSystemAsset('cron_manager');
		foreach ($links as $link) {
			$cm_links = $GLOBALS['SQ_SYSTEM']->am->getLinkByAsset($cm->id, $link['majorid']);
			if (!empty($cm_links)) {
				//This asset is still linked to the cron manager, so its still active.
				$assetids[] = $link['majorid'];
			}
		}
		$dates = $GLOBALS['SQ_SYSTEM']->am->getAttributeValuesByName('when', 'cron_job_future_status', $assetids);
		asort($dates, SORT_STRING);
		$res = Array();
		foreach ($dates as $assetid => $date) {
			$res[] =& $GLOBALS['SQ_SYSTEM']->am->getAsset($assetid, __CLASS__);
		}
		return $res;

	}//end getActiveJobs()


}//end class

?>
