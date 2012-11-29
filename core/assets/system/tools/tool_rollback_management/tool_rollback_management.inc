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
* $Id: tool_rollback_management.inc,v 1.6 2012/11/29 23:59:54 ewang Exp $
*
*/


require_once SQ_SYSTEM_ROOT.'/core/assets/system/tool/tool.inc';
require_once SQ_INCLUDE_PATH.'/rollback_management.inc';

/**
* Rollback Management Tool
*
* Purpose
* 	A tool to manage the amount of data stored in Rollback
*
* @author Mark Brydon <mbrydon@squiz.net>
* @version $Revision: 1.6 $
* @package MySource_Matrix_Packages
* @subpackage __core__
*/

class Tool_Rollback_Management extends Tool
{


	/**
	* Constructor
	*
	* @param int	$assetid  the asset id to be loaded
	*
	*/
	function Tool_Rollback_Management($assetid=0)
	{
		$this->Tool($assetid);

	}//end constructor


	/**
	* Paints the tool interface.
	*
	* @param object	&$o		reference to the backend outputter of the object painting the tool
	* @param string	$type_code	the type code of the tool being painted
	*
	* @return void
	* @access public
	* @static
	*/
	public static function paintTool(&$o, $type_code)
	{
		$tool_info = $GLOBALS['SQ_SYSTEM']->am->getTypeInfo($type_code);

		$o->openSection($tool_info['name']);

		// Allow System Admins to run this tool
		if (!$GLOBALS['SQ_SYSTEM']->userSystemAdmin() && !$GLOBALS['SQ_SYSTEM']->userRoot()) {
			$o->openField('');
				$o->note('Access denied to run this tool');
			$o->closeField();
			return;
		}

		$o->openField(translate('tool_rollback_rb_status'));
			$options = Array(
						'0'	=> 'Disabled',
						'1'	=> 'Enabled',
					   );

			hidden_field($type_code.'_rollback_status_old', (SQ_CONF_ROLLBACK_ENABLED) ? '1' : '0');
			combo_box($type_code.'_rollback_status', $options, FALSE, (SQ_CONF_ROLLBACK_ENABLED) ? '1' : '0');
		$o->closeField();


		$o->openField('');
		$o->closeField();

		$o->openField(translate('tool_rollback_reset_all_rb_entries'));
			$options = Array(
						'0'	=> translate('no'),
						'1'	=> translate('yes'),
					   );
			combo_box($type_code.'_reset_rollback_entries', $options, FALSE, '');
			$o->note('<b>'.translate('tool_rollback_reset_all_note').'</b>');
		$o->closeField();

		$o->openField(translate('tool_rollback_purge_rb_entries_before'));
			text_box($type_code.'_purge_rollback_entries_period_amount', '', 4, 4);

			$options = Array(
						'h'	=> 'hours',
						'd'	=> 'days',
						'w'	=> 'weeks',
						'm'	=> 'months',
						'y'	=> 'years',
					   );

			combo_box($type_code.'_purge_rollback_entries_period_timeframe', $options, FALSE, '');
		$o->closeField();

		$o->openField(translate('tool_rollback_purge_file_versioning_entries_before'));
			text_box($type_code.'_purge_file_entries_period_amount', '', 4, 4);

			$options = Array(
						'h'	=> 'hours',
						'd'	=> 'days',
						'w'	=> 'weeks',
						'm'	=> 'months',
						'y'	=> 'years',
					   );

			combo_box($type_code.'_purge_file_entries_period_timeframe', $options, FALSE, '');
		$o->closeField();

		$o->openField(translate('tool_delete_redundant_entries'));
			$options = Array(
						'0'	=> 'No',
						'1'	=> 'Yes',
					   );
			combo_box($type_code.'_delete_redundant_entries', $options, FALSE, 0, 0, "onChange=\"Javascript: if (this.value == 1) { if(!confirm(js_translate('tool_delete_redundant_entries_confirm'))) { this.value = 0; } } \"");
			$o->note(translate('tool_delete_redundant_entries_note'));
		$o->closeField();

		$o->openField('');
			if (isset($_REQUEST['redirect_to_sys_maintenance'])) {
				$redirect_to_system_maintenance = (int)$_REQUEST['redirect_to_sys_maintenance'];
				if ($redirect_to_system_maintenance) {
					hidden_field('redirect_to_sys_maintenance', $redirect_to_system_maintenance);
				}
			}
		$o->closeField();

		$o->openField(translate('note'));
			$o->note(translate('tool_rollback_usage_note'));
		$o->closeField();


		$o->closeSection();

	}//end paintTool()


	/**
	* Processes the user input and performs the function of the tool.
	*
	* @param object	&$o			reference to the backend outputter of the object painting the tool
	* @param string	$type_code	the type code of the tool being painted
	*
	* @return boolean
	* @access public
	* @static
	*/
	public static function processTool(&$o, $type_code)
	{
		// Allow System Admins to run this tool
		if (!$GLOBALS['SQ_SYSTEM']->userSystemAdmin() && !$GLOBALS['SQ_SYSTEM']->userRoot()) {
			return FALSE;
		}

		$settings = Array();

		$redirect_url = '';

		if (isset($_REQUEST['redirect_to_sys_maintenance'])) {
			$redirect_to_system_maintenance = (int)$_REQUEST['redirect_to_sys_maintenance'];
			if ($redirect_to_system_maintenance) {
				$redirect_url = sq_web_path('admin').'/?SQ_BACKEND_PAGE=main&backend_section=sys_maintenance&expand=rollback';
			}
		}

		$time_units = Array(
						'h'	=> 'hour',
						'd'	=> 'day',
						'w'	=> 'week',
						'm'	=> 'month',
						'y'	=> 'year',
					  );

		$rollback_date = date('Y-m-d H:i:s');
		$purge_rb_date = '';
		$purge_fv_date = '';

		$reset_rollback_entries = $_REQUEST[$type_code.'_reset_rollback_entries'];


		$purge_rb_period = (int) $_REQUEST[$type_code.'_purge_rollback_entries_period_amount'];
		$purge_fv_period = (int) $_REQUEST[$type_code.'_purge_file_entries_period_amount'];

		$purge_rb_time_unit = $_REQUEST[$type_code.'_purge_rollback_entries_period_timeframe'];
		$purge_fv_time_unit = $_REQUEST[$type_code.'_purge_file_entries_period_timeframe'];

		if ($purge_rb_period > 0) {
			$time_unit = $time_units[$purge_rb_time_unit];
			if ($purge_rb_period > 1) $time_unit .= 's';
			$purge_rb_date = date('Y-m-d H:i:s', strtotime('-'.$purge_rb_period.' '.$time_unit));
		}

		if ($purge_fv_period > 0) {
			$time_unit = $time_units[$purge_fv_time_unit];
			if ($purge_fv_period > 1) $time_unit .= 's';
			$purge_fv_date = date('Y-m-d H:i:s', strtotime('-'.$purge_fv_period.' '.$time_unit));
		}

		$rollback_status = $_REQUEST[$type_code.'_rollback_status'];
		$enable_rollback = FALSE;
		$disable_rollback = FALSE;

		if ($rollback_status != $_REQUEST[$type_code.'_rollback_status_old']) {
			$enable_rollback = ($rollback_status == '1');
			$disable_rollback = ($rollback_status == '0');
		}

		$reset_rollback = ($_REQUEST[$type_code.'_reset_rollback_entries'] == '1');
		$delete_redundant_entries = ($_REQUEST[$type_code.'_delete_redundant_entries'] == '1');

		$hh = $GLOBALS['SQ_SYSTEM']->getHipoHerder();
		$vars = Array(
					'enable_rollback'	=> $enable_rollback,
					'disable_rollback'	=> $disable_rollback,
					'rollback_date'		=> $rollback_date,
					'purge_fv_date'		=> $purge_fv_date,
					'purge_rb_date'		=> $purge_rb_date,
					'reset_rollback'	=> $reset_rollback,
					'delete_redundant_entries' => $delete_redundant_entries,
					'fail_on_error'		=> TRUE,
				);

		$hh->queueHipo('hipo_job_rollback_management', $vars, $redirect_url);

		$url = $hh->runQueuedJobs();
		if (!empty($url)) $o->setRedirect($url);

		return TRUE;

	}//end processTool()


}//end class
?>
