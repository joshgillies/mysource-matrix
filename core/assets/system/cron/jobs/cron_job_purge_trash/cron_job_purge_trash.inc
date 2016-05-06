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
*/


require_once SQ_CORE_PACKAGE_PATH.'/system/cron/cron_job/cron_job.inc';

/**
* Cron_Job_Purge_Trash
*
* Purpose
*
*
* @author  Ash Karelia <akarelia@squiz.com.au>>
* @package MySource_Matrix_Packages
* @subpackage __core__
*/
class Cron_Job_Purge_Trash extends Cron_Job
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
		return translate('Purge Trash Schedule Job');

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
		return parent::setAttrValue($name, $value);

	}//end setAttrValue()


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
		$trash_asset = $GLOBALS['SQ_SYSTEM']->am->getSystemAsset('trash_folder');
		$scheduling_enabled = $trash_asset->attr('scheduling_status');
		$job_link_age = $trash_asset->attr('job_link_age');
		$job_purge_node = $trash_asset->attr('job_purge_node');

		if ($scheduling_enabled) {
			// process the status change
			$hh = $GLOBALS['SQ_SYSTEM']->getHipoHerder();
			$vars = Array();
			if (!empty($job_link_age)) $vars['link_age'] = $job_link_age;

			if (!empty($job_purge_node)) {
				$link_info = $GLOBALS['SQ_SYSTEM']->am->getLinkByAsset($trash_asset->id, $job_purge_node);
				$purge_root_linkid = array_get_index($link_info, 'linkid', '');
				$vars = array_merge(Array('purge_root_linkid' => $purge_root_linkid), $vars);
			}

			// Let's go trashing!
			$purge_errors = $hh->freestyleHipo('hipo_job_purge_trash', $vars);
			if (empty($purge_errors)) {
				// all OK, and we can be removed because we have done our task
				$msg = translate('Scheduled job to purge Trash successfully completed');

				if ($this->attr('type') == 'one_off') {
					$this->setAttrValue('type', 'repeating');
					$this->setAttrValue('when', 'TI=,-,60');
					$this->saveAttributes();
				}
				return SQ_CRON_JOB_COMPLETED;
			} else {
				// process failed
				trigger_localised_error('CRON0003', translate('Scheduled job to purge Trash failed'), E_USER_WARNING);
				// we have errors so dump them out too
				foreach ($purge_errors as $error) {
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
			if ($this->attr('type') == 'one_off') {
				$this->setAttrValue('type', 'repeating');
				$this->setAttrValue('when', 'TI=,-,60');
				$this->saveAttributes();
			}
			return SQ_CRON_JOB_COMPLETED;
		}

		// we can be removed because the error is beyound our control
		return SQ_CRON_JOB_ERROR;

	}//end _exec()


}//end class

?>
