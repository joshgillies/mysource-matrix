<?php

require_once SQ_CORE_PACKAGE_PATH.'/system/cron/cron_job/cron_job.inc';

/**
* Cron_Job_Future_Status
*
* Purpose
*
*
* @author  Blair Robertson <blair@squiz.net>
* @version $Version$ - 1.0
* @package Resolve_Packages
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
		$this->Asset($assetid);
	}


	/**
	* Returns name of the asset
	*
	* @param bool	$short_name	whether or not we are after the shortname or the full name
	*
	* @see Asset::_getName()
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

		}// end if

	}//end _getName();


	/**
	* Returns an array of all the permitted links type, the type asset and the cardinality
	*
	* @see Asset::_getAllowLinks()
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
	* Set the value for the passed variable to the passed value
	*
	* @param string		$name				the name of the attribute
	* @param string		$value				the new value of the attribute
	* @param boolean	$dont_run_updated	if true this->_updated() won't be run
	*										(THIS IS ONLY EVER CALLED FROM Asset::create())
	*
	* @return boolean
	* @access public
	*/
	function setAttrValue($name, $value, $dont_run_updated=false)
	{
		switch($name) {
			case 'status' :
				$value = (int) $value;
				if (!in_array($value, array_keys($this->_getAllStatii()))) {
					trigger_error('Unable to set the status, it is not valid', E_USER_NOTICE);
					return false;
				}// end if
				break;

			case 'type' :
				if (empty($this->_tmp[__CLASS__.'_in_create'])) {
					trigger_error('You can\'t set the type of a future status cron job it can only be a one off event', E_USER_NOTICE);
					return false;
				}// end if
				break;
		}// end if

		return parent::setAttrValue($name, $value, $dont_run_updated);

	}// end setAttrValue()


	/**
	* returns an array of all statii that a future page status can be set to.
	*
	* @return Array() of the form Array('[value]' => '[code_name]')
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

	}// end _getAllStatii()


	/**
	* returns the name of the status that this future status object is going to change the asset to
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

	}// end statusName()


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

	}// end setAssetToUpdate()


	/**
	* Gets the user that this cron job is to run as while executing
	*
	* @return mixed object User or NULL on error
	* @access public
	*/
	function &getAssetToUpdate()
	{
		$null = null; // because we need to return by ref

		$link = $this->getLink(SQ_LINK_NOTICE, 'asset', false, 'updating');
		if (empty($link)) return $null;

		return $GLOBALS['SQ_SYSTEM']->am->getAsset($link['minorid'], $link['type_code']);

	}// end getAssetToUpdate()


	/**
	* Actually executes the functionality for this job, called by run()
	*
	* @return int	bitmask of the SQ_CRON_JOB_* constants to tell the cron manager what to do
	* @see Cron_Job::_exec(), Cron_Job::run()
	* @access private
	*/
	function _exec()
	{
		trigger_error('TRIGGER TERMINATE ERROR', E_USER_ERROR);



		return SQ_CRON_JOB_ERROR;

		$asset = &$this->getAssetToUpdate();
		if (!is_null($asset)) {

			// Let's just process the status change
			if ($asset->processStatusChange($this->attr('status'))) {
				// All OK, and we can be removed because we have done our one off task
				return SQ_CRON_JOB_COMPLETED | SQ_CRON_JOB_REMOVE;

			// process failed
			} else {
				trigger_error('Changing Status of "'.$asset->name.'" (#'.$asset->id.') Failed', E_USER_WARNING);

			}

		// asset not found
		} else {
			trigger_error('Couldn\'t find asset to update the status of', E_USER_WARNING);

		}

		// we can be removed because the error is beyound our control
		return SQ_CRON_JOB_ERROR | SQ_CRON_JOB_REMOVE;

	}// end _exec()

}//end class
?>
