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
* $Id: hipo_job_trigger_batch_events.inc,v 1.17 2012/08/30 01:09:19 ewang Exp $
*
*/

require_once SQ_SYSTEM_ROOT.'/core/hipo/hipo_job.inc';

/**
* Highly Intensive Processing Object (HIPO) to batch-broadcast events as part of the Trigger Manager's Batch Processing interface
*
* <pre>
* Initial Running Vars :
*	Array(int)      assets                - the asset IDs of the assets that will broadcast events
*	Array(string)   events                - an array of event types that each asset will broadcast
*	Array			event_parameters	  - extra parameters to pass to 3rd argument of Mysouce::broadcastTriggerEvent().
*   int             codename_target       - the id of the target asset, for use in the codename
*   int             codename_propagate    - shows whether the descendants of the target were also processed by the HIPO. Used by getCodeName()
* </pre>
*
* @author  Robert Howard <rhoward@squiz.net>
* @author  Andrei Railean <arailean@squiz.net>
* @version $Revision: 1.17 $
* @package MySource_Matrix
* @subpackage HIPO
*/
class HIPO_Job_Trigger_Batch_Events extends HIPO_Job
{


	/**
	* Constructor
	*
	* @param string	$code_name        a unique codename the HIPO
	*/
	function HIPO_Job_Trigger_Batch_Events($code_name='')
	{
		$this->HIPO_Job($code_name);

	}//end constructor


	/**
	* Paints backend of this job
	*
	* @param object Backend_Outputter	&$o			reference to the backend outputter
	* @param string						$class		calling class name
	*
	* @return void
	* @access private
	*/
	public static function paintConfig(&$o, $class, $write_access)
	{
		// threshhold HIPO config entry
		$o->openField(translate('trigger_execution_threshold'));

		echo '<span class="sq-inline-fields-wrapper">';
		if ($write_access) {
			text_box($class.'[SQ_HIPO_TRIGGER_BATCH_EVENTS_THRESHOLD]', SQ_HIPO_TRIGGER_BATCH_EVENTS_THRESHOLD, 1);
		} else {
			echo SQ_HIPO_TRIGGER_BATCH_EVENTS_THRESHOLD;
		}
		echo ' '.translate('assets');
		echo '</span>';

		$o->closeField();

	}//end paintConfig()


	/**
	* Sets some info about thresholds that this hipo requires.
	*
	* @return Array
	* @access public
	*/
	public static function getConfigVars()
	{
		return Array(
				'SQ_HIPO_TRIGGER_BATCH_EVENTS_THRESHOLD'	=> Array('editable'	=> 1, 'default'	=> 0),
			   );

	}//end getConfigVars()


	/**
	* Returns a unique codename the HIPO can use to ensure it is not being run twice
	*
	* The Trigger Batch Events HIPO appends the current user, the ID of the target, and whether the
	* 'include children' flag is set, so that any other batching jobs on the selected assets do not
	* start while it is running
	*
	* @return string
	* @access public
	*/
	function getCodeName()
	{
		return parent::getCodeName().'-u'.$GLOBALS['SQ_SYSTEM']->currentUserId().'-a'.$this->_running_vars['codename_target'].'-'.$this->_running_vars['codename_propagate'];

	}//end getCodeName()


	/**
	* Returns the (localised) HIPO name
	*
	* @return string
	* @access public
	*/
	function getHipoName()
	{
		return translate('hipo_name_trigger_batch_events');

	}//end getHipoName()


	/**
	* Returns the steps in this hipo, possibly with localised step names
	* and messages
	*
	* @return string
	* @access public
	*/
	function getInitialStepData()
	{
		return Array(
				Array(
					'name'			=> translate('hipo_processing_assets'),
					'function_call'	=> Array(
										'process_function'	=> 'processAssets',
									   ),
					'running_mode'	=> 'server',
					'auto_step'		=> true,
					'allow_cancel'	=> true,
					'percent_done'	=> 0,
					'complete'		=> false,
					'message'		=> '',
				),
			   );

	}//end getInitialStepData()


	/**
	* Returns the current Threshold.
	*
	* Returns a value that represents the total percentage that this hipo would run in
	* freestyle mode in its current state.
	*
	* @return float
	* @access public
	*/
	function getThresholdPercentageRequired()
	{
		if (SQ_HIPO_TRIGGER_BATCH_EVENTS_THRESHOLD == 0) {
			return 0;
		}

		if (empty($this->_running_vars['assets'])) return 0;

		return (($this->_running_vars['total_steps'] / SQ_HIPO_TRIGGER_BATCH_EVENTS_THRESHOLD) * 100);

	}//end getThresholdPercentageRequired()


	/**
	* Performs the duties of this hipo, without showing any output on the frontend.
	*
	* @return boolean
	* @access public
	*/
	function freestyle()
	{
		while (!empty($this->_running_vars['assets'])) {
			if (!$this->processAssets($this->_steps[0], get_class($this))) {
				return false;
			}
		}
		return true;

	}//end freestyle()


	/**
	* Prepares the running vars.
	*
	* @return boolean
	* @access public
	*/
	function prepare()
	{
		// to start the batch events, we need to have asset ids and the events to fire on each
		if (empty($this->_running_vars['assets']) || empty($this->_running_vars['events'])) {
			trigger_localised_error('HIPO0002', E_USER_WARNING);
			return false;
		}

		// total steps is the number of assets * the number of events to broadcast for each.
		$this->_running_vars['total_steps'] = count($this->_running_vars['assets']) * count($this->_running_vars['events']);
		$this->_running_vars['step_value']  = (int) (100 / $this->_running_vars['total_steps']);

		if (empty($this->_running_vars['event_parameters'])) $this->_running_vars['event_parameters'] = NULL;

		return parent::prepare();

	}//end prepare()


	/**
	* Broadcast an event on a single asset in the to-do list
	*
	* @param array()	&$step_data	a reference to the array of information about the current step
	* @param string		$prefix		prefix for form vars
	*
	* @return boolean
	* @access public
	*/
	function processAssets(&$step_data, $prefix)
	{
		if (empty($this->_running_vars['current_events'])) {
			if (empty($this->_running_vars['assets'])) {
				// update percentages, set completed
				$step_data['complete'] = true;
				$step_data['percent_done'] = 100;
				return true;
			}
			$this->_running_vars['current_events'] = $this->_running_vars['events'];
			$this->_running_vars['current_assetid'] = array_pop($this->_running_vars['assets']);
		}

		$current_event = array_pop($this->_running_vars['current_events']);
		$asset = $GLOBALS['SQ_SYSTEM']->am->getAsset($this->_running_vars['current_assetid']);
		$GLOBALS['SQ_SYSTEM']->broadcastTriggerEvent($current_event, $asset, $this->_running_vars['event_parameters']);

		$step_data['message'] = sprintf(translate('hipo_broadcast_event'), $current_event, $this->_running_vars['current_assetid']);
		$step_data['percent_done'] += $this->_running_vars['step_value'];
		$step_data['complete']     = false;

		return true;

	}//end processAssets()


}//end class

?>
