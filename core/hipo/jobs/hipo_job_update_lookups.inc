<?php
require_once SQ_SYSTEM_ROOT.'/core/hipo/hipo_job.inc';

/**
* Highly Intensive Processing Object (HIPO) to update lookups
*
* @author  Greg Sherwood <greg@squiz.net>
* @version $Version$ - 1.0
* @package Resolve
* @subpackage lib
*/
class HIPO_Job_Update_Lookups extends HIPO_Job
{

	/**
	* The name of this HIPO job (should not be modified by code)
	* @var string
	*/
	var $_hipo_name = 'Update Lookups';

	/**
	* Array of steps to perform in this HIPO
	* @var Array
	*/
	var $_steps = Array(1 => Array( 'name'             => 'Updating Lookups',
									'paint_function'   => 'paintLookups',
									'process_function' => 'processLookups',
									'running_mode'     => 'web',
									'auto_step'        => true,
									'percent_done'     => 0,
									'complete'         => false,
									'message'          => ''),
						);


	/**
	* Constructor
	*
	* @param string	$codename		a unique codename the HIPO
	* @param string	$display_mode	the mode this HIPO is being displayed in
	*								(web = browser, server = HIPO server)
	*/
	function HIPO_Job_Update_Lookups($codename='', $display_mode='web')
	{
		$this->HIPO_Job($codename, $display_mode);

	}//end constructor


	/**
	* Returns a unique codename the HIPO can use to ensure it is not being run twice
	*
	* The Edit Permission HIPO appends the ID of the asset that the permissions are being set at
	* (the top most parent) so that any other permission jobs on this asset do not start while it
	* is running. Due to locking of the interface, nobody should be allowed to run this HIPO at
	* the same time on the same parent asset.
	*
	* @return string
	* @access public
	*/
	function getCodeName()
	{
		return parent::getCodeName().$this->_running_vars['assetid'];

	}//end getCodeName()


	/**
	* Set up vars and database information that the HIPO will need to run
	*
	* The Updtae Lookups HIPO also checks that there is an assetid to start this action on
	* (the top most parent). It also saves a list of assets to perform this action on in the running vars.
	*
	* @return boolean
	* @access public
	*/
	function initialise()
	{
		// to start updating lookups, we need to have an asset id to start updating lookups at
		if (!isset($this->_running_vars['assetid'])) {
			trigger_error('Cannot initialise Update Lookups HIPO, no start asset ID specified', E_USER_WARNING);
			return false;
		}

		// so we have an assetid to start at, but make sure it is a valid assetid
		$asset = &$GLOBALS['SQ_SYSTEM']->am->getAsset($this->_running_vars['assetid']);
		if (is_null($asset)) {
			trigger_error('Cannot initialise Update Lookups HIPO, invalid start asset ID specified ('.$this->_running_vars['assetid'].')', E_USER_WARNING);
			return false;
		}

		// we have a valid asset, so get all the assets below this one in the tree(s) so we know what assets
		// we will be updating the lookups of
		$child_assets = Array();
		$child_assets = array_merge(Array($asset->id), $asset->getChildren());
		$this->_running_vars['todo_assetids'] = $child_assets;
		$this->_running_vars['done_assetids'] = Array();

		unset($asset);

		return parent::initialise();

	}//end initialise()


	/**
	* Paint a progress bar showing where we are up to
	*
	* @param array()	&$step_data	a reference to the array of information about the current step
	* @param string		$prefix		prefix for form vars
	*
	* @return boolean
	* @access public
	*/
	function paintLookups(&$step_data, $prefix)
	{
		$this->paintProgressBar($step_data['percent_done'], '#412F49', $step_data['message']);
		if ($this->_hipo_vars['running_mode'] == 'server') sleep(2);

	}//end paintLookups()


	/**
	* Update lookups for a single asset in the to-do list
	*
	* @param array()	&$step_data	a reference to the array of information about the current step
	* @param string		$prefix		prefix for form vars
	*
	* @return boolean
	* @access public
	*/
	function processLookups(&$step_data, $prefix)
	{
		if (!empty($this->_running_vars['todo_assetids'])) {
			$assetid = array_shift($this->_running_vars['todo_assetids']);
			$asset = &$GLOBALS['SQ_SYSTEM']->am->getAsset($assetid);

			if (!is_null($asset)) {
				if (!$asset->_updateLookups(false)) return false;
				$step_data['message'] = 'Updating Lookups for "'.$asset->name.'"';
				$GLOBALS['SQ_SYSTEM']->am->unregisterAsset($asset);
				unset($asset);
			} else {
				$step_data['message'] = 'Skipping asset '.$assetid;
				$this->_addError('Could not update lookups for asset #'.$assetid.', asset does not exist');
			}

			// add this assetid to the done array so we dont do it again
			$this->_running_vars['done_assetids'][] = $assetid;
		}

		if (empty($this->_running_vars['todo_assetids'])) {
			$step_data['percent_done'] = 100;
			$step_data['complete']     = true;
		} else {
			$total = count($this->_running_vars['todo_assetids']) + count($this->_running_vars['done_assetids']);
			$step_data['percent_done'] =(count($this->_running_vars['done_assetids']) / $total) * 100;
			$step_data['complete']     = false;
		}

		return true;

	}//end processLookups()

}//end class

?>