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
* $Id: cron_job_future_status.inc,v 1.16 2004/05/13 00:43:22 gsherwood Exp $
* $Name: not supported by cvs2svn $
*/


require_once SQ_CORE_PACKAGE_PATH.'/system/cron/cron_job/cron_job.inc';

/**
* Cron_Job_Future_Status
*
* Purpose
*
*
* @author  Blair Robertson <blair@squiz.net>
* @version $Version$ - 1.0
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
	* @access public
	*/
	function _getName($short_name=false)
	{
		$asset = &$this->getAssetToUpdate();
		if (is_null($asset)) {
			return 'Asset-less Future Status Cron Job';
		} elseif ($short_name) {
			return 'FS for "'.$asset->short_name.'" (#'.$asset->id.')';
		} else {
			return 'Future Status for "'.$asset->name.'" (#'.$asset->id.')';
		}

	}//end _getName();


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
		// Basically because we can be altering the status of any asset type
		// we need to make sure that we can cope with the possibility of
		// changing the status of a type that is used by the parent class
		// EG, 'user' for the running user
		for(reset($links[SQ_LINK_NOTICE]); (NULL !== ($type = key($links[SQ_LINK_NOTICE]))); next($links[SQ_LINK_NOTICE])) {
			if ($links[SQ_LINK_NOTICE][$type]['card'] != 'M') {
				$links[SQ_LINK_NOTICE][$type]['card'] = (int) $links[SQ_LINK_NOTICE][$type]['card'] + 1;
			}
		}

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
		switch($name) {
			case 'status' :
				$value = (int) $value;
				if (!in_array($value, array_keys($this->_getAllStatii()))) {
					trigger_error('Unable to set the status, it is not valid', E_USER_NOTICE);
					return false;
				}
				break;

			case 'type' :
				trigger_error('You can\'t set the type of a future status cron job it can only be a one off event', E_USER_NOTICE);
				return false;
				break;
		}// end if

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
			foreach($statii as $value => $code) { 
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
			trigger_error('Unable to set asset to update, the asset must be a asset object', E_asset_WARNING);
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
			$vars = Array('assetid' => $asset->id, 'new_status' => $this->attr('status'));
			$status_errors = $hh->freestyleHipo('hipo_job_edit_status', $vars);
			if (empty($this->_errors)) {
				// all OK, and we can be removed because we have done our one off task
				$msg = 'Altered Status of "'.$asset->name.'" (#'.$asset->id.') from "'.$descs[$old_status].'" to "'.$descs[$asset->status].'"';
				return SQ_CRON_JOB_COMPLETED | SQ_CRON_JOB_REMOVE;
			} else {
				// process failed
				trigger_error('Changing Status of "'.$asset->name.'" (#'.$asset->id.') Failed', E_USER_WARNING);
			}

		} else {
			// asset not found
			trigger_error('Couldn\'t find asset to update the status of', E_USER_WARNING);
		}

		// we can be removed because the error is beyound our control
		return SQ_CRON_JOB_ERROR | SQ_CRON_JOB_REMOVE;

	}// end _exec()


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
		$cron_mgr = &$GLOBALS['SQ_SYSTEM']->am->getSystemAsset('cron_manager');
		if (is_null($cron_mgr)) {
			$blank = Array();
			return $blank;
		}

		// get all active future status jobs
		$all_jobs = &$cron_mgr->getJobs(__CLASS__, false);

		$GLOBALS['SQ_SYSTEM']->am->forgetAsset($cron_mgr);

		$dates = Array();
		for($i = 0, $total = count($all_jobs); $i < $total; $i++) {
			
			$link = $GLOBALS['SQ_SYSTEM']->am->getLink($all_jobs[$i]->id, SQ_LINK_NOTICE, 'asset', false, 'updating');
			if (!empty($link) && $link['minorid'] == $asset->id) {
				$dates[$i] = $all_jobs[$i]->attr('when');
			}

		}// end for

		asort($dates, SORT_STRING);
		$assets_jobs = Array();

		foreach($dates as $i => $date) {
			$assets_jobs[] = &$all_jobs[$i];
		}// end for

		return $assets_jobs;

	}// end getActiveJobs()


}//end class
?>
