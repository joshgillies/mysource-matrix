<?php
/**
* Copyright (c) 2003 - Squiz Pty Ltd
*
* $Id: cron_job_future_status_edit_fns.inc,v 1.5 2003/09/26 05:26:27 brobertson Exp $
* $Name: not supported by cvs2svn $
*/


require_once SQ_CORE_PACKAGE_PATH.'/system/cron/cron_job/cron_job_edit_fns.inc';

/**
* Cron_Job_Future_Status_Edit_Fns
*
* Purpose
*
*
* @author  Blair Robertson <blair@squiz.net>
* @version $Version$ - 1.0
* @package MySource_Matrix_Packages
* @subpackage __core__
*/
class Cron_Job_Future_Status_Edit_Fns extends Cron_Job_Edit_Fns
{

	/**
	* Constructor
	*
	*/
	function Cron_Job_Future_Status_Edit_Fns()
	{
		$this->Cron_Job_Edit_Fns();
	}//end constructor


	/**
	* Prints the tasks interface in a one line format
	*
	* @param object Asset				&$asset	the owning asset
	* @param object Backend_Outputter	&$o		the backend outputter
	* @param string						$prefix	prefix for form elements
	*
	* @return boolean
	* @access public
	*/
	function paintInLineBackend(&$asset, &$o, $prefix)
	{
		echo 'Set Status to ';
		$descs = $asset->_getAllStatiiDescs();
		if ($asset->writeAccess()) {
			combo_box($prefix.'_status', $descs, false, $asset->attr('status'));
		} else {
			echo $descs[$asset->attr('status')];
		}
		echo ' at ';
		$this->_paintWhenBox($asset, $o, $prefix);
		return true;
	}//end paintInLineBackend()


	/**
	* Process the tasks interface
	*
	* @param object Asset				&$asset	the owning asset
	* @param object Backend_Outputter	&$o		the backend outputter
	* @param string						$prefix	prefix for form elements
	*
	* @return boolean
	* @access public
	*/
	function processInLineBackend(&$asset, &$o, $prefix)
	{
		if (!empty($_POST[$prefix.'_status']) && $asset->setAttrValue('status', $_POST[$prefix.'_status'])) {
			return $this->_processWhenBox($asset, $o, $prefix);
		}
		return false;

	}//end processInLineBackend()


}//end class
?>
