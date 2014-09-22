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
* $Id: hipo_job_generate_report.inc,v 1.19 2012/08/30 01:09:18 ewang Exp $
*
*/

require_once SQ_SYSTEM_ROOT.'/core/hipo/hipo_job.inc';

/**
* Highly Intensive Processing Object (HIPO) to generate reports
*
* <pre>
* Initial Running Vars :
*    int    assetid  -   the assetid of the report being generated
* </pre>
*
* @author  Greg Sherwood <greg@squiz.net>
* @version $Revision: 1.19 $
* @package MySource_Matrix
* @subpackage HIPO
*/
class HIPO_Job_Generate_Report extends HIPO_Job
{


	/**
	* Constructor
	*
	* @param string	$code_name		a unique codename the HIPO
	*/
	function HIPO_Job_Generate_Report($code_name='')
	{
		$this->HIPO_Job($code_name);

	}//end constructor


	/**
	* Paints backend of this job
	*
	* @param object	&$o		reference to the backend outputter
	* @param string	$class	calling class name
	*
	* @return void
	* @access private
	*/
	public static function paintConfig(&$o, $class, $write_access)
	{
		// metadata regeneration threshhold HIPO config entry
		$o->openField(translate('Report Regeneration Threshold'));


		echo '<span class="sq-inline-fields-wrapper">';
		if ($write_access) {
			text_box($class.'[SQ_HIPO_REGENERATE_REPORT_THRESHOLD]', SQ_HIPO_REGENERATE_REPORT_THRESHOLD, 1);
		} else {
			echo SQ_HIPO_REGENERATE_REPORT_THRESHOLD;
		}
		echo ' '.translate('assets');

		echo '</span>';

		$o->closeField();

	}//end paintConfig()


	/**
	* Sets some info about thresholds that this hipo requires.
	*
	* @return array
	* @access public
	*/
	public static function getConfigVars()
	{
		return Array(
				'SQ_HIPO_REGENERATE_REPORT_THRESHOLD'	=> Array('editable'	=> 1, 'default'	=> 20),
			   );

	}//end getConfigVars()


	/**
	* Returns the current Threshold.
	*
	* Returns a value that represents the total percentage that this hipo would run in freestyle mode in its current state.
	*
	* @return float
	* @access public
	*/
	function getThresholdPercentageRequired()
	{
		if (SQ_HIPO_REGENERATE_REPORT_THRESHOLD == 0) return 0;
		if (!isset($this->_running_vars['todo'])) return 0;
		if (!is_array($this->_running_vars['todo'])) return 0;
		return (((count($this->_running_vars['todo']) + 1) / SQ_HIPO_REGENERATE_REPORT_THRESHOLD) * 100);

	}//end getThresholdPercentageRequired()


	/**
	* Returns a unique codename the HIPO can use to ensure it is not being run twice
	*
	* The Acquire Lock HIPO appends the ID of the asset that the lock is being acquired on
	* (the top most parent) so that any other locking jobs on this asset do not start while it
	* is running. Due to locking of the interface, nobody should be allowed to run this HIPO at
	* the same time on the same parent asset.
	*
	* @return string
	* @access public
	*/
	function getCodeName()
	{
		return parent::getCodeName().'-'.$this->_running_vars['assetid'];

	}//end getCodeName()


	/**
	* Returns the (localised) HIPO name
	*
	* @return string
	* @access public
	*/
	function getHipoName()
	{
		return translate('Generate Report');


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
					'name'			=> translate('Generating Report'),

					'function_call'	=> Array(
										'process_function'	=> 'processReport',
									   ),
					'running_mode'	=> 'server',
					'auto_step'		=> TRUE,
					'allow_cancel'	=> TRUE,
					'percent_done'	=> 0,
					'complete'		=> FALSE,
					'message'		=> '',
				),
			   );

	}//end getInitialStepData()


	/**
	* Performs the duties of this hipo, without showing any output on the frontend.
	*
	* @return boolean
	* @access public
	*/
	function freestyle()
	{
		while (empty($this->_running_vars['complete']) && !empty($this->_running_vars['assetid'])) {
			if (!$this->processReport($this->_steps[0], get_class_lower($this))) {
				return FALSE;
			}
		}
		return TRUE;

	}//end freestyle()


	/**
	* Prepares the running vars
	*
	* @return boolean
	* @access public
	*/
	function prepare()
	{
		if (empty($this->_running_vars['assetid'])) {
			trigger_localised_error('HIPO0008', translate('Unable to initialise Generate Report HIPO, no report asset ID specified'), E_USER_WARNING)
			return FALSE;
		}

		$asset = $GLOBALS['SQ_SYSTEM']->am->getAsset($this->_running_vars['assetid']);
		$this->_running_vars['type_code'] = $asset->type();
		$this->_running_vars['complete']  = FALSE;
		return parent::prepare();

	}//end prepare()


	/**
	* Generate the report
	*
	* We pass the work off to the report asset to do for us
	*
	* @param array	&$step_data	a reference to the array of information about the current step
	* @param string	$prefix		prefix for form vars
	*
	* @return boolean
	* @access public
	*/
	function processReport(&$step_data, $prefix)
	{
		if ($step_data['complete']) {
			$this->_running_vars['complete'] = TRUE;
		}
		$asset = $GLOBALS['SQ_SYSTEM']->am->getAsset($this->_running_vars['assetid'], $this->_running_vars['type_code']);
		$edit_fns = $asset->getEditFns();

		return $edit_fns->generateReport($this, $step_data, $prefix);

	}//end processReport()


}//end class

?>
