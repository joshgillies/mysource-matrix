<?php
/**
* +--------------------------------------------------------------------+
* | This MySource Matrix CMS file is Copyright (c) Squiz Pty Ltd       |
* | ACN 084 670 600                                                    |
* +--------------------------------------------------------------------+
* | IMPORTANT: Your use of this Software is subject to the terms of    |
* | the Licence provided in the file licence.txt. If you cannot find   |
* | this file please contact Squiz (www.squiz.net) so we may provide   |
* | you a copy.                                                        |
* +--------------------------------------------------------------------+
*
* $Id: logs.inc,v 1.31 2011/04/18 06:34:46 mhaidar Exp $
*
*/

require_once SQ_LIB_PATH.'/html_form/html_form.inc';

/**
* Logs Static Screen Functions
*
* @author Greg Sherwood <gsherwood@squiz.net>
* @author Marcel Wegermann	<mwegermann@squiz.net>
* @version $Revision: 1.31 $
* @package MySource_Matrix
*/


/**
* Paints the interface for viewing logged information about the asset
*
* @param object	&$owner		the asset whose interface we are painting
* @param object	&$o			the outputter class
* @param object	&$ei		the edit interface
* @param array	$keywords	list of keywords to replace
*
* @return boolean
* @access public
*/
function paintLogs(&$owner, &$o, &$ei, $keywords=NULL)
{


	// prefix for all the form vars
	$prefix = $owner->getPrefix();

	$db = MatrixDAL::getDb();
	try {
		$sql = "SELECT DISTINCT 
				    type
				FROM
				    ".SQ_TABLE_RUNNING_PREFIX."internal_msg
				WHERE
				    userto = '0'
				    AND type LIKE 'asset%'
				";
		$log_types = MatrixDAL::executeSqlAssoc($sql, 0);
	} catch (Exception $e) {
		throw new Exception('Failed to get log messages due to database error: '.$e->getMessage());
	}

	if (isset($_REQUEST['sq_backend_log_type'])) {
		$active_log_type = $_REQUEST['sq_backend_log_type'];
	} else {
		$active_log_type = (!empty($log_types) && isset($log_types[0])) ? $log_types[0] : '';
	}

	$selected_key = NULL;
	$selected_base_url = '';
	$o->addFormActionGetVar('sq_backend_log_type', $active_log_type);
	$o->openSection(translate('logged_messages'));


	$o->openRaw();

	$type_options = Array();
	foreach ($log_types as $type) {
		// html option fields contains complete url to call via javascript self.location
		$base_url = replace_query_string_vars(Array('sq_backend_log_type' => $type, $owner->type().'_'.$owner->id.'_start_at' => NULL,));
		if ($type == $active_log_type) {
			$selected_base_url = $base_url;
		}
		$type_options[$base_url] = ucwords(str_replace(Array('.', '_'), ' ', $type));
	}

	if (!( empty($type_options) && empty($selected_base_url) )) {

		echo '<b>'.translate('log_type').'</b>&nbsp;';

		// if it is in limbo mode there will be already an element with sq_backend_log_type id
		$_combo_box_name = 'sq_backend_log_type';

		// if it is in limbo mode there will be already an element with sq_backend_log_type id
		// therefore we will change the name of the combo box
		if (SQ_IN_LIMBO) {
			$_combo_box_name = 'sq_backend_log_type2';
		}

		combo_box($_combo_box_name, $type_options, FALSE, $selected_base_url);
		echo '&nbsp;';

		normal_button($prefix.'_go', translate('view'), 'self.location = document.getElementById(\''.$_combo_box_name.'\').value');
	}

	$o->closeRaw();

	$ms = $GLOBALS['SQ_SYSTEM']->getMessagingService();

	if (strpos($active_log_type, 'asset.linking') === 0) {

		$minor_logs = $ms->getLogs($active_log_type, Array('minorid' => $owner->id));
		$major_logs = $ms->getLogs($active_log_type, Array('majorid' => $owner->id));
		$logs = array_merge($minor_logs, $major_logs);
		usort($logs, Array('Messaging_Service', 'sortMessages'));

	} else if (strpos($active_log_type, 'asset.workflow') === 0) {

		// for workflow we want to show the user log messages as well
		if (isset($_REQUEST['sq_workflow_log_version'])) {
			$o->addFormActionGetVar('sq_workflow_log_version', $_REQUEST['sq_workflow_log_version']);
			$logs = $ms->getLogs($active_log_type, Array(
													'assetid'	=> $owner->id,
													'version'	=> $_REQUEST['sq_workflow_log_version'],
												   )
								 );
			$user_logs = $ms->getLogs($active_log_type.'.userlog', Array(
																	'assetid'	=> $owner->id,
																	'version'	=> $_REQUEST['sq_workflow_log_version'],
																   )
									  );
		} else {
			$logs = $ms->getLogs($active_log_type, Array('assetid' => $owner->id));
			$user_logs = $ms->getLogs($active_log_type.'.userlog', Array('assetid' => $owner->id));
		}

		$logs = array_merge($logs, $user_logs);
		usort($logs, Array('Messaging_Service', 'sortMessages'));

	} else {

		$logs = $ms->getLogs($active_log_type, Array('assetid' => $owner->id));

	}//end if ($active_log_type)

	if (empty($logs)) {
		$o->openRaw();
			if (!( empty($type_options) && empty($selected_base_url) )) {
				echo '<b>'.translate('no_logged_messages', htmlspecialchars($owner->name, ENT_COMPAT, SQ_CONF_DEFAULT_CHARACTER_SET)).'</b>';
			} else {
				echo '<b>'.translate('no_logged_messages_found', htmlspecialchars($owner->name, ENT_COMPAT, SQ_CONF_DEFAULT_CHARACTER_SET)).'</b>';
			}
		$o->closeRaw();
	} else {
		$o->openRaw();
		$lm = $GLOBALS['SQ_SYSTEM']->lm;
			?>
			<table class="sq-backend-table">
				<tr>
					<td class="sq-backend-table-header" nowrap width="120"><?php echo translate('log_time'); ?></td>
					<td class="sq-backend-table-header" nowrap width="120"><?php echo translate('logged_by'); ?></td>
					<td class="sq-backend-table-header" width="100%"><?php echo translate('message'); ?></td>
				</tr>
				<?php
				$num_printed  = 0;
				$num_per_page = 10;
				$num_logs = count($logs);
				$start_printing_at = 0;
				if (isset($_REQUEST[$prefix.'_start_at'])) {
					$start_printing_at = (int) $_REQUEST[$prefix.'_start_at'];
				}
				for ($i = $start_printing_at; $i < $num_logs; $i++) {
					$log_data = $logs[$i];

					?>
					<tr>
						<td class="sq-backend-table-cell" nowrap><?php echo ts_iso8601($log_data['sent']); ?></td>
						<td class="sq-backend-table-cell" nowrap><?php echo $log_data['from_name']; ?></td>
						<td class="sq-backend-table-cell" width="100%"><b><?php echo $log_data['subject']; ?></b><br/><?php echo nl2br(htmlentities($log_data['body'])); ?></td>
					</tr>
					<?php
					$num_printed++;
					if ($num_printed >= $num_per_page) break;
				}
				?>
				<tr>
					<td colspan="3" align="center" class="sq-backend-table-header">
					<?php

					if ($start_printing_at > 0) {
						?>

						<?php $next_url = $selected_base_url.'&'.$prefix.'_start_at='.($start_printing_at - $num_per_page); ?>
						 <a style="text-decoration: none;" href="<?php echo $next_url; ?>">
						<?php

					} else {
						?><span style="color: #84848F"><?php
					}
					?>
					&lt;&lt; <?php echo translate('previous_page');

					if ($start_printing_at > 0) {
						echo '</a>';
					} else {
						echo '</span>';
					}
					?>
					&nbsp;<b><?php echo translate('page_number', (floor($start_printing_at / $num_per_page) + 1), ceil($num_logs / $num_per_page)); ?></b>&nbsp;

					<?php
					if (($start_printing_at + $num_per_page) < $num_logs) {
						?>

						 <?php $next_url = $selected_base_url.'&'.$prefix.'_start_at='.($start_printing_at + $num_per_page); ?>
						  <a style="text-decoration: none;" href="<?php echo $next_url; ?>">
						 <?php

					} else {
						?><span style="color: #84848F"><?php
					}

					echo translate('next_page'); ?> &gt;&gt;
					<?php
					if (($start_printing_at + $num_per_page) < $num_logs) {
						echo '</a>';
					} else {
						echo '</span>';
					}
					?>
					</td>
				</tr>
			</table>
			<?php
		$o->closeRaw();
	}//end else


	$o->closeSection();

	return FALSE;

}//end paintLogs()


/**
* Processes the interface for viewing logged information about the asset
*
* @param object	&$owner	the asset whose interface we are painting
* @param object	&$o		the outputter class
* @param object	&$ei	the edit interface
*
* @return boolean
* @access public
*/
function processLogs(&$owner, &$o, &$ei)
{
	return TRUE;

}//end processLogs()


?>
