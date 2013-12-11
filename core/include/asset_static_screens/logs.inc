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
* $Id: logs.inc,v 1.40 2012/08/30 01:09:20 ewang Exp $
*
*/

require_once SQ_LIB_PATH.'/html_form/html_form.inc';
require_once SQ_FUDGE_PATH.'/datetime_field/datetime_field.inc';

/**
* Logs Static Screen Functions
*
* @author Greg Sherwood <gsherwood@squiz.net>
* @author Marcel Wegermann	<mwegermann@squiz.net>
* @version $Revision: 1.40 $
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
	$invalid_dates = TRUE;

	$log_types = include SQ_DATA_PATH.'/private/system/core/internal_message_types.inc';

	// if it is in limbo mode there will be already an element with sq_backend_log_type id
	// therefore we use differnet name for the combo box in limbo mode
	if (SQ_IN_LIMBO) {
		$_combo_box_name = 'sq_backend_log_type2';
	} else {
		$_combo_box_name = 'sq_backend_log_type';
	}

	if (isset($_REQUEST[$_combo_box_name])) {
		$active_log_types = $_REQUEST[$_combo_box_name];
	} else {
		$active_log_types = (!empty($log_types) && isset($log_types[0])) ? Array($log_types[0]) : Array();
	}

    // Make sure it's an array - in case the user has clicked a link that's setting
    // the log type to a particular value.
	if (!is_array($active_log_types)) {
		$active_log_types = array($active_log_types);
	}

	$selected_key = NULL;
	$selected_based_url = '';
	$selected_types = Array();
	$o->openSection(translate('logged_messages'));
	
	$type_options = Array();
	foreach ($log_types as $type) {
		// We're only interested in log types starting with 'asset'
		if (strpos($type, 'asset') !== 0) {
			continue;
		}
		
		// html option fields contains complete url to call via javascript self.location
		if (in_array($type, $active_log_types)) {
			$selected_types[] = $type;
		}
		$type_options[$type] = ucwords(str_replace(Array('.', '_'), ' ', $type));
	}
	
	$date = '';
	$date_parameters = Array(
					'min'			=> '1990-01-01 00:00:00',
					'max'			=> '2030-12-31 23:59:59',
					'allow_circa'	=> '0',
					'show'			=> Array('y', 'm', 'd', 'h', 'i', 's'),
					'null'			=> Array(),
					'style'			=> Array(
										'y'	=> 's',
										'm'	=> 's',
										'd'	=> 's',
										'h'	=> 's',
										'i'	=> 's',
										's'	=> 's',
									   ),
				  );

	// get 2 date fields for to and from fields
	$from_field = new DateTime_Field($prefix.'_date_from', $date, $date_parameters);
	$to_field   = new DateTime_Field($prefix.'_date_to', $date, $date_parameters);
	
	//set default from and to dates (from date to be 1 year in the past)
	$default_from_date = (date('Y')-1).date('-m-d H:i:s'); 
	$default_to_date = date('Y-m-d H:i:s');
	
	$from_date = $default_from_date;
	$to_date = $default_to_date;

	// get the values for the from date if the form has been posted
	if ($from_field->processField()) {
		$from_date = $from_field->value;
	}
	// this will look stupid but this is needed
	// or else on screen reload both the date 
	// fields will have the same date on them
	$from_field->setValue($from_date);

	// do the same for the to date
	if ($to_field->processField()) {
		$to_date = $to_field->value;
	}
	$to_field->setValue($to_date);


	if (!( empty($type_options) && empty($selected_types) )) {

		$o->openField(translate('log_type'));

		combo_box($_combo_box_name, $type_options, TRUE, $selected_types, 10);
		echo '&nbsp;';


		$o->closeField();

		$o->openField(translate('from'));
		$from_field->printField();
		$o->closeField();

		$o->openField(translate('to'));
		$to_field->printField();
		echo "<br>";
		// okie dont let the user confuse you with wrong dates
		if (isset($from_date) && isset($to_date) && $from_date > $to_date) {
			echo "<font color=\"red\"><b>ERROR : 'From' date provided is later than 'To' date. </b></font><br>";
			$invalid_dates = FALSE;
		}
		$o->closeField();
		
		$o->openField('&nbsp;');
			normal_button($prefix.'_go', translate('view'), 'this.form.submit()');
		$o->closeField();
	}


	$ms = $GLOBALS['SQ_SYSTEM']->getMessagingService();
	
	$all_logs = Array();
	foreach($selected_types as $active_log_type) {

		if (strpos($active_log_type, 'asset.linking') === 0) {

			$minor_logs = $ms->getLogs($active_log_type, Array('minorid' => $owner->id, 'from_date' => $from_date, 'to_date' => $to_date));
			$major_logs = $ms->getLogs($active_log_type, Array('majorid' => $owner->id, 'from_date' => $from_date, 'to_date' => $to_date));
			$logs = array_merge($minor_logs, $major_logs);

		} else if (strpos($active_log_type, 'asset.workflow') === 0) {

			// for workflow we want to show the user log messages as well
			if (isset($_REQUEST['sq_workflow_log_version'])) {
				$o->addFormActionGetVar('sq_workflow_log_version', $_REQUEST['sq_workflow_log_version']);
				$logs = $ms->getLogs($active_log_type, Array(
														'assetid'	=> $owner->id,
														'version'	=> $_REQUEST['sq_workflow_log_version'],
														'from_date'	=> $from_date,
														'to_date'	=> $to_date,
													   )
									 );
				$user_logs = $ms->getLogs($active_log_type.'.userlog', Array(
																		'assetid'	=> $owner->id,
																		'version'	=> $_REQUEST['sq_workflow_log_version'],
																		'from_date'	=> $from_date,
																		'to_date'	=> $to_date,
																	   )
										  );
			} else {
				$logs = $ms->getLogs($active_log_type, Array('assetid' => $owner->id, 'from_date' => $from_date, 'to_date' => $to_date));
				$user_logs = $ms->getLogs($active_log_type.'.userlog', Array('assetid' => $owner->id, 'from_date' => $from_date, 'to_date' => $to_date));
			}

			$logs = array_merge($logs, $user_logs);

		} else {

			$logs = $ms->getLogs($active_log_type, Array('assetid' => $owner->id, 'from_date' => $from_date, 'to_date' => $to_date));

		}//end if ($active_log_type)

		$all_logs = array_merge($all_logs, $logs);
	}//end foreach ($active_log_types)

	// Sort the logs
	usort($all_logs, Array('Messaging_Service', 'sortMessages'));
	
	if (!empty($selected_types)) {
		$selected_base_url = replace_query_string_vars(Array($_combo_box_name.'[]' => NULL, $owner->type().'_'.$owner->id.'_start_at' => NULL,));
		$selected_base_url .= '&'.$_combo_box_name.'[]='.implode('&'.$_combo_box_name.'[]=', $selected_types);
	}

	// if non of the logs are selected or if the dates
	// were nto right dont display any logs for now
	if (empty($all_logs) || !$invalid_dates) {
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
					<th class="sq-backend-table-header" nowrap width="120"><?php echo translate('log_time'); ?></th>
					<th class="sq-backend-table-header" nowrap width="120"><?php echo translate('logged_by'); ?></th>
					<th class="sq-backend-table-header" width="100%"><?php echo translate('message'); ?></th>
				</tr>
				<?php
				$num_printed  = 0;
				$num_per_page = 10;
				$num_logs = count($all_logs);
				$start_printing_at = 0;
				if (isset($_REQUEST[$prefix.'_start_at'])) {
					$start_printing_at = (int) $_REQUEST[$prefix.'_start_at'];
				}
				for ($i = $start_printing_at; $i < $num_logs; $i++) {
					$log_data = $all_logs[$i];

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
					<th colspan="3" class="sq-backend-table-header sq-pagination-links sq-no-bottom-border">
					<?php

					if ($start_printing_at > 0) {
						?>

						<?php $next_url = $selected_base_url.'&'.$prefix.'_start_at='.($start_printing_at - $num_per_page); ?>
					    <a style="text-decoration: none;" href="<?php echo (htmlspecialchars($next_url)); ?>">
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
						  <a style="text-decoration: none;" href="<?php echo (htmlspecialchars($next_url)); ?>">
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
					</th>
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
