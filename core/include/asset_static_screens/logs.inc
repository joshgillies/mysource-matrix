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
* $Id: logs.inc,v 1.2.2.2 2004/03/02 18:29:12 brobertson Exp $
* $Name: not supported by cvs2svn $
*/

require_once SQ_LIB_PATH.'/html_form/html_form.inc';

/**
* Logs Static Screen Functions
*
* @author  Greg Sherwood <gsherwood@squiz.net>
* @version $Version$ - 1.0
* @package MySource_Matrix
*/


/**
* Paints the interface for viewing logged information about the asset
*
* @param object	Asset					&$owner	the asset whose interface we are painting
* @param object	Backend_Outputter		&$o		the outputter class
* @param object	Asset_Edit_Interface	&$ei	the edit interface
*
* @return boolean
* @access public
*/
function paintLogs(&$owner, &$o, &$ei)
{
	// prefix for all the form vars
	$prefix = $owner->getPrefix();

	$db = &$GLOBALS['SQ_SYSTEM']->db;

	$sql   = 'SELECT DISTINCT type FROM '.SQ_TABLE_RUNNING_PREFIX.'internal_message ';
	$where = 'userto = 0 AND type LIKE '.$db->quote('asset%');
	$where = $GLOBALS['SQ_SYSTEM']->constructRollbackWhereClause($where);

	$log_types = $db->getCol($sql.$where);
	if (DB::isError($log_types)) {
		trigger_error($log_types->getMessage().'<br/>'.$log_types->getUserInfo(), E_USER_WARNING);
		return false;
	}

	$active_log_type = (isset($_REQUEST['sq_backend_log_type'])) ? $_REQUEST['sq_backend_log_type'] : $log_types[0];
	$o->addFormActionGetVar('sq_backend_log_type', $active_log_type);
	$o->openSection('Logged Messages');
		$o->openField('Log Type');
			$type_options = Array();
			foreach ($log_types as $type) $type_options[$type] = ucwords(str_replace('.', ' ', $type));
			combo_box('sq_backend_log_type', $type_options, false, $active_log_type);
			echo '&nbsp;';
			normal_button($prefix.'_go', 'View', 'submit_form()');
		$o->closeField();

	$ms = &$GLOBALS['SQ_SYSTEM']->getMessagingService();

	switch ($active_log_type) {

		case 'asset.linking' :
			$minor_logs = $ms->getLogs($active_log_type, Array('minorid' => $owner->id));
			$major_logs = $ms->getLogs($active_log_type, Array('majorid' => $owner->id));
			$logs = array_merge($minor_logs, $major_logs);
			usort($logs, Array('Messaging_Service', 'sortMessages'));
		break;

		case 'asset.workflow' :
			// for workflow we want to show the user log messages as well
			if (isset($_REQUEST['sq_workflow_log_version'])) {
				$o->addFormActionGetVar('sq_workflow_log_version', $_REQUEST['sq_workflow_log_version']);
				$logs = $ms->getLogs($active_log_type, Array('assetid' => $owner->id,
															 'version' => $_REQUEST['sq_workflow_log_version']
															 )
							);
				$user_logs = $ms->getLogs($active_log_type.'.userlog', Array('assetid' => $owner->id,
																			 'version' => $_REQUEST['sq_workflow_log_version']
																			 )
							);
			} else {
				$logs = $ms->getLogs($active_log_type, Array('assetid' => $owner->id));
				$user_logs = $ms->getLogs($active_log_type.'.userlog', Array('assetid' => $owner->id));
			}
			$logs = array_merge($logs, $user_logs);
			usort($logs, Array('Messaging_Service', 'sortMessages'));
		break;

		default :
			$logs = $ms->getLogs($active_log_type, Array('assetid' => $owner->id));
		break;

	}//end switch

	if (empty($logs)) {
		$o->openSection('There are no logged messages of this type for "'.$owner->name.'"');
	} else {
		$o->openField('');
			?>
			<table class="sq-backend-table">
				<tr>
					<td class="sq-backend-table-header" nowrap width="120">Log Time</td>
					<td class="sq-backend-table-header" nowrap width="120">Logged By</td>
					<td class="sq-backend-table-header" width="100%">Message</td>
				</tr>
			<?php
			$num_printed  = 0;
			$num_per_page = 10;
			$num_logs = count($logs);
			$start_printing_at = 0;
			if (isset($_REQUEST[$prefix.'_start_at'])) $start_printing_at = (int) $_REQUEST[$prefix.'_start_at'];
			for ($i = 0; $i < $num_logs; $i++) {
				if ($i < $start_printing_at) {
					$i++;
					continue;
				}
				$log_data = $logs[$i];
				?>
				<tr>
					<td class="sq-backend-table-cell" nowrap><?php echo ts_iso8601($log_data['sent']); ?></td>
					<td class="sq-backend-table-cell" nowrap><?php echo $log_data['from_name']; ?></td>
					<td class="sq-backend-table-cell" width="100%"><b><?php echo $log_data['subject']; ?></b><br/><?php echo $log_data['body']; ?></td>
				</tr>
				<?php
				$num_printed++;
				if ($num_printed >= $num_per_page) break;
			}
			?>
				<tr>
					<td colspan="3" align="center" class="sq-backend-table-header">
					<?php
					hidden_field($prefix.'_start_at', $start_printing_at);
					if ($start_printing_at > 0) {
						?><a style="text-decoration: none;" href="#" onClick="Javascript: set_hidden_field('<?php echo $prefix; ?>_start_at', '<?php echo $start_printing_at - $num_per_page; ?>'); set_hidden_field('process_form', '0'); submit_form(); return false;"><?php
					} else {
						?><span style="color: #84848F"><?php
					}
					?>
					&lt;&lt; Previous Page
					<?php
					if ($start_printing_at > 0) {
						?></a><?php
					} else {
						?></span><?php
					}
					?>
					&nbsp;<b>Page <?php echo round(($start_printing_at + $num_per_page) / $num_per_page); ?> of <?php echo round(($num_logs / $num_per_page) + 0.5); ?></b>&nbsp;

					<?php
					if (($start_printing_at + $num_per_page) < $num_logs) {
						?><a style="text-decoration: none;" href="#" onClick="Javascript: set_hidden_field('<?php echo $prefix; ?>_start_at', '<?php echo $start_printing_at + $num_per_page; ?>'); set_hidden_field('process_form', '0'); submit_form(); return false;"><?php
					} else {
						?><span style="color: #84848F"><?php
					}
					?>
					Next Page &gt;&gt;
					<?php
					if (($start_printing_at + $num_per_page) < $num_logs) {
						?></a><?php
					} else {
						?></span><?php
					}
					?>
					</td>
				</tr>
			</table>
			<?php
		$o->closeField();
	}
	$o->closeSection();

	return false;

}//end paintLogs()


/**
* Processes the interface for viewing logged information about the asset
*
* @param object	Asset					&$owner	the asset whose interface we are painting
* @param object	Backend_Outputter		&$o		the outputter class
* @param object	Asset_Edit_Interface	&$ei	the edit interface
*
* @return boolean
* @access public
*/
function processLogs(&$owner, &$o, &$ei)
{
	return true;

}//end processLogs()


?>
