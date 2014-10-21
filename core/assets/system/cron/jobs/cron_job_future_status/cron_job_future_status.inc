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
* $Id: cron_job_future_status.inc,v 1.48 2012/08/30 01:09:11 ewang Exp $
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
* @version $Revision: 1.48 $
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
				trigger_localised_error('CRON0018', translate('Unable to create future lineage cron job, unable to acquire "links" lock'), E_USER_WARNING);
				return FALSE;
			}

			$link = $this->_tmp['asset_in_link'];
			$asset = $GLOBALS['SQ_SYSTEM']->am->getAsset($link['minorid'], $link['minor_type_code']);
			if (is_null($asset) || !$this->setAssetToUpdate($asset)) {
				trigger_localised_error('CRON0017', translate('Unable to create future lineage cron job, Unable to set asset in link'), E_USER_WARNING);
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
			return translate('Asset-less Future Status Scheduled Job');

		} else if ($short_name) {
			return sprintf(translate('FS for %s'), sprintf(translate('"%1$s" [#%2$s]'), $asset->short_name, $asset->id));


		} else {
			return sprintf(translate('Future Status for %s'), sprintf(translate('"%1$s" [#%2$s]'), $asset->name, $asset->id));


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
	* For Future Status Jobs, admins of the asset to update can also remove jobs
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
			case 'status' :
				$value = (int) $value;
				if (!in_array($value, array_keys($this->_getAllStatii()))) {
					trigger_localised_error('CRON0031', translate('Unable to set the status, it is not valid'), E_USER_NOTICE);
					return FALSE;
				}
			break;

			case 'type' :
				trigger_localised_error('CRON0038', translate('Unable to set the type of a future lineage cron job; it can only be a one off event'), E_USER_NOTICE);
				return FALSE;
			break;
		}//end if

		return parent::setAttrValue($name, $value);

	}//end setAttrValue()


	/**
	* Return an array of all statii that a future page status can be set to
	* of the form Array('[value]' => '[code_name]')
	*
	* @return array
	* @access private
	* @static
	*/
	protected static function _getAllStatii()
	{
		return Array(
				SQ_STATUS_ARCHIVED				=> 'ARCHIVED',
				SQ_STATUS_UNDER_CONSTRUCTION	=> 'UNDER_CONSTRUCTION',
				SQ_STATUS_LIVE					=> 'LIVE',
				SQ_STATUS_EDITING				=> 'EDITING',
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
	public static function _getAllStatiiDescs()
	{
		static $descs = Array();

		if (empty($descs)) {
			$descs['']  = '[Select Status]';
			$statii = self::_getAllStatii();
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
	public function statusName()
	{
		if (!$this->attr('status')) return '[ No Status Set ]';
		$statii = self::_getAllStatii();
		$code = strtolower($statii[$this->attr('status')]);
		$status_code = 'asset_status_'.$code;
		require_once SQ_INCLUDE_PATH.'/asset_status/'.$status_code.'.inc';
		eval('$desc = '.$status_code.'::getDescription();');
		return $desc;

	}//end statusName()


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

			$old_status = $asset->status;

			// process the status change
			$hh = $GLOBALS['SQ_SYSTEM']->getHipoHerder();
			$vars = Array('assetid' => $asset->id, 'new_status' => $this->attr('status'), 'dependants_only' => $this->attr('dependants_only'));

			$status_errors = $hh->freestyleHipo('hipo_job_edit_status', $vars);
			if (empty($status_errors)) {
				// all OK, and we can be removed because we have done our one off task
				$msg = sprintf(translate('Altered status of %1$s from %2$s to %3$s'), sprintf(translate('"%1$s" [#%2$s]'), $asset->name, $asset->id), get_status_description($old_status), $this->statusName());


				return SQ_CRON_JOB_COMPLETED | SQ_CRON_JOB_REMOVE;
			} else {
				// process failed
				trigger_localised_error('CRON0003', sprintf(translate('Failed changing Status of "%1$s" (Id: #%2$s)'), $asset->name, $asset->id), E_USER_WARNING);
				// we have errors so dump them out too
				foreach ($status_errors as $error) {
					$line = '';
					if (is_string($error)) {
						$line = $error;
					} else if (is_array($error)) {
						if (isset($error['time'])) $line .= 'Time : '.$error['time'].'. ';
						if (isset($error['message'])) $line .= 'Message : '.$error['message'].'.';
					}
					if ($line !== '') trigger_error($line, E_USER_WARNING);
				}
			}

		} else {
			// asset not found
			trigger_localised_error('CRON0020', translate('Unable to find asset to update the status of'), E_USER_WARNING);
		}

		// we can be removed because the error is beyound our control
		return SQ_CRON_JOB_ERROR | SQ_CRON_JOB_REMOVE;

	}//end _exec()


	/**
	* Returns an array of active future status jobs that are to be run on the passed asset
	* in chronological order
	*
	* @param Asset	$asset	the asset to get the future status jobs for
	*
	* @return array
	* @access public
	* @static
	*/
	public static function &getActiveJobs(Asset $asset)
	{
		$links = $GLOBALS['SQ_SYSTEM']->am->getLinks($asset->id, SQ_LINK_NOTICE, strtolower(__CLASS__), TRUE, 'minor', 'updating');
		$assetids = Array();
		$cm = $GLOBALS['SQ_SYSTEM']->am->getSystemAsset('cron_manager');
		foreach ($links as $link) {
			$cm_links = $GLOBALS['SQ_SYSTEM']->am->getLinkByAsset($cm->id, $link['majorid']);
			if (!empty($cm_links)) {
				// This asset is still linked to the cron manager, so its still active.
				$assetids[] = $link['majorid'];
			}
		}
		$dates = $GLOBALS['SQ_SYSTEM']->am->getAttributeValuesByName('when', 'cron_job_future_status', $assetids);
		asort($dates, SORT_STRING);
		$res = Array();
		foreach ($dates as $assetid => $date) {
			$res[] = $GLOBALS['SQ_SYSTEM']->am->getAsset($assetid, strtolower(__CLASS__));
		}
		return $res;

	}//end getActiveJobs()


}//end class

?>
