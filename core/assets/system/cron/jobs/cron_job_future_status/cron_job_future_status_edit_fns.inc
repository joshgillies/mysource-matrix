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
* $Id: cron_job_future_status_edit_fns.inc,v 1.24 2012/08/30 01:09:11 ewang Exp $
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
* @version $Revision: 1.24 $
* @package MySource_Matrix_Packages
* @subpackage __core__
*/
class Cron_Job_Future_Status_Edit_Fns extends Cron_Job_Edit_Fns
{


	/**
	* Constructor
	*
	* @see Asset_Edit_Fns::__construct()
	*/
	function __construct()
	{
		parent::__construct();

	}//end constructor


	/**
	* Prints the tasks interface in a one line format
	*
	* @param Cron_Job_Future_Status	$asset					The owning asset
	* @param Backend_Outputter		$o						the Backend Outputter object
	* @param string					$prefix					prefix for form elements
	* @param string					$when					prepared when string
	* @param string					$cascade				cascade the status change
	* @param string					$form_extra_elements	extra form elements
	* @param array					$exclude_status			exclude these statuses from the list
	*
	* @return boolean
	* @access public
	*/
	public function paintInLineBackend(Cron_Job_Future_Status $asset, Backend_Outputter $o, $prefix, $when='', $cascade=FALSE, $form_extra_elements='', Array $exclude_status=Array())
	{
		echo '<span class="sq-inline-fields-wrapper">';
		echo translate('Set Status to');


		$descs = $asset->_getAllStatiiDescs();
		foreach ($exclude_status as $status) {
			unset($descs[$status]);
		}

		if ($asset->writeAccess('attributes')) {
			combo_box($prefix.'_status', $descs, FALSE, $asset->attr('status'), NULL, $form_extra_elements.' onchange="checkStatusDisplayWarningFutureStatus()"');
		} else {
			echo $descs[$asset->attr('status')];
		}

		echo '&nbsp;'.strtolower(translate('At')).'&nbsp;';

		$this->_paintWhenBox($asset, $o, $prefix, $when, $form_extra_elements);

		echo '&nbsp;';
			echo '<span class="sq-keep-inline">';
				check_box($prefix.'_change_status_all_children', '1', $cascade, 'checkStatusDisplayWarningFutureStatus()');
				label(translate('Cascade Status Change'), $prefix.'_change_status_all_children');

			echo '</span>';
		echo '</span>';

		echo '<div id="warning_message_for_live_plus_cascade_fs" style="display:none" class="sq-warning-message top-margin">'.translate('<strong>Warning:</strong> Cascading a Live status to child assets may result in unintended canceling of Safe Editing of those children which can result in loss of content.').'</div>';

		echo '<div id="warning_message_for_uc_or_archive_fs" style="display:none" class="sq-warning-message top-margin">'.translate('<strong>Warning:</strong> You are about to change the status to a non-Live status. This will make the asset inaccessible to users with read access to it.').'</div>';

		echo '<div id="warning_message_for_non_live_plus_cascade_fs" style="display:none" class="sq-warning-message top-margin">'.translate('<strong>Warning:</strong> Cascading a non-Live status to child assets may result in unintended un-publishing of those children.').'</div>';

		echo '<div id="warning_message_for_stream_cascade_fs" style="display:none" class="sq-warning-message top-margin">'.translate('<strong>Warning:</strong> Cascading this workflow initiation status to child assets will initiate the workflow for all children that are in a workflow ready state. The chosen workflow stream will also be cascaded if applicable.').'</div>';

		return TRUE;

	}//end paintInLineBackend()


	/**
	* Process the tasks interface
	*
	* @param Cron_Job_Future_Status	$asset	The owning asset
	* @param Backend_Outputter		$o		the Backend Outputter object
	* @param string					$prefix	prefix for form elements
	*
	* @return boolean
	* @access public
	*/
	public function processInLineBackend(Cron_Job_Future_Status $asset, Backend_Outputter $o, $prefix)
	{
		if (!empty($_POST[$prefix.'_status']) && $asset->setAttrValue('status', $_POST[$prefix.'_status'])) {
			$dependants_only = (isset($_POST[$prefix.'_change_status_all_children']) && $_POST[$prefix.'_change_status_all_children']) ? FALSE : TRUE;
			if ($when = $this->_processWhenBox($asset, $o, $prefix)) {
				return ($asset->setAttrValue('when', $when) && $asset->setAttrValue('dependants_only', $dependants_only));
			} else {
				trigger_localised_error('CRON0046', translate('To add a future status you must specify the date and time at which the status should be changed'), E_USER_NOTICE);
			}
		}

		return FALSE;

	}//end processInLineBackend()


}//end class

?>
