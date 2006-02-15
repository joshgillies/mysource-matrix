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
* $Id: cron_job_future_status_edit_fns.inc,v 1.16 2006/02/15 05:50:59 tbarrett Exp $
*
*/


require_once SQ_CORE_PACKAGE_PATH.'/system/cron/cron_job/cron_job_edit_fns.inc';

/**
* Cron_Job_Future_Status_Edit_Fns
*
* Purpose
*
*
* @author  Blair Robertson <blair@squiz.net>
* @version $Revision: 1.16 $
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
	* @param object	&$asset					the owning asset
	* @param object	&$o						the Backend Outputter object
	* @param string	$prefix					prefix for form elements
	* @param string	$when					prepared when string
	* @param string	$form_extra_elements	extra form elements
	*
	* @return boolean
	* @access public
	*/
	function paintInLineBackend(&$asset, &$o, $prefix, $when='', $form_extra_elements='')
	{
		echo translate('set_status_to');

		$descs = $asset->_getAllStatiiDescs();
		if ($asset->writeAccess('attributes')) {
			combo_box($prefix.'_status', $descs, FALSE, $asset->attr('status'), NULL, $form_extra_elements);
		} else {
			echo $descs[$asset->attr('status')];
		}

		echo ' '.strtolower(translate('at')).' ';
		$this->_paintWhenBox($asset, $o, $prefix, $when, $form_extra_elements);

		echo '&nbsp;';
		check_box($prefix.'_change_status_all_children');
		echo translate('cascade_status_change');

		return TRUE;

	}//end paintInLineBackend()


	/**
	* Process the tasks interface
	*
	* @param object	&$asset	the owning asset
	* @param object	&$o		the Backend Outputter object
	* @param string	$prefix	prefix for form elements
	*
	* @return boolean
	* @access public
	*/
	function processInLineBackend(&$asset, &$o, $prefix)
	{
		if (!empty($_POST[$prefix.'_status']) && $asset->setAttrValue('status', $_POST[$prefix.'_status'])) {
			$dependants_only = (isset($_POST[$prefix.'_change_status_all_children']) && $_POST[$prefix.'_change_status_all_children']) ? false : true;
			if ($when = $this->_processWhenBox($asset, $o, $prefix)) {
				return ($asset->setAttrValue('when', $when) && $asset->setAttrValue('dependants_only', $dependants_only));
			} else {
				trigger_localised_error('CRON0046', E_USER_NOTICE);
			}
		}

		return FALSE;

	}//end processInLineBackend()


}//end class

?>
