<?php
/**
* +--------------------------------------------------------------------+
* | MySource 3 - MySource Matrix                                       |
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
* $Id: cron_job_future_status_edit_fns.inc,v 1.7 2003/11/18 15:37:28 brobertson Exp $
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
		if ($asset->writeAccess('attributes')) {
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
