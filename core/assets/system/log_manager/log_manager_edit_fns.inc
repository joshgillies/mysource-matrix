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
* $Id: log_manager_edit_fns.inc,v 1.15 2012/08/30 01:09:11 ewang Exp $
*
*/

require_once SQ_INCLUDE_PATH.'/asset_edit/asset_edit_fns.inc';
require_once SQ_FUDGE_PATH.'/general/file_system.inc';


/**
* Log_Manager_Edit_Fns
*
* Purpose
*   Log Manager's Edit Functions
*
* @author  Andrei Railean <arailean@squiz.net>
* @author  Robert Howard  <rhoward@squiz.net>
* @version $Revision: 1.15 $
* @package MySource_Matrix_Packages
* @subpackage __core__
*/
class Log_Manager_Edit_Fns extends Asset_Edit_Fns
{


	/**
	* Constructor
	*
	*/
	function Log_Manager_Edit_Fns()
	{
		$this->Asset_Edit_Fns();

		// disable all static screens
		$this->static_screens = Array();

		// enable just the details
		$this->static_screens['details']['name']         =translate('Details');

		$this->static_screens['details']['force_unlock'] = FALSE;
		$this->static_screens['details']['lock_type']    = 'none';

	}//end constructor


	/**
	* Paint Log List
	*
	* @param object	&$lm	log manager
	* @param object	&$o		the Backend Outputter object
	* @param string	$prefix	prefix for the form elements
	*
	* @return boolean
	* @access public
	*/
	function paintLogList(&$lm, &$o, $prefix)
	{
		$download_file = array_get_index($_POST, $prefix.'_download_file', '');
		if ($download_file !== '') {
			$this->processDownloadLogFile($lm, $o, $prefix);
		}

		$write_access = $lm->writeAccess();

		$o->openRaw();
		$logs = $lm->getLogs();
		if (empty($logs)) {
			echo '<h2>'.translate('No Logs Found').'</h2>';

			return $write_access;
		}

		?>
		<script type="text/javascript">
			//<![CDATA[
			var prefix 			 = "<?php echo $prefix; ?>";
			var communicator_url = "<?php echo $lm->getCommunicatorUrl(); ?>";
			var action_prefix    = prefix + "_action";
			var action_value     = prefix + "_value";
			var logname_index    = prefix + "_log";
			var offset_index     = prefix + "_offset";
			var num_lines_index  = prefix + "_num_lines";

			var refresh_interval = 30000; //seconds
			var last_offset = null;
			var num_lines = 50;

			var log_refresh = true;
			var refresher   = null;
			var current_log = null;

			/**
			* Start monitoring a new log from the known log list
			*/
			function monitorLog(logname)
			{
				if (logname == null) {
					logname = current_log;
				} else {
					current_log = logname;
				}

				if (logname == null) {
					return false;
				}

				document.getElementById('sq-log-monitor-initial-message').style.display = 'none';
				document.getElementById('sq-log-monitor-container').style.display = 'block';
				setIdValue('sq-log-name', logname);
				document.getElementById('sq-log-num-lines').defaultValue = num_lines;

				// initial setting
				if (log_refresh && null === refresher) {
					enableRefresh();
				}

				last_offset = null;
				loadLog();

				return false;

			}//end monitorLog()



			/**
			* Rorces the refresh of the log
			*
			* Loads log and re-sets the timeout counter
			*/
			function forceRefresh()
			{
				loadLog();
				enableRefresh();

			}//end forceRefresh()


			/**
			* Load current log into the monitor frame
			*/
			function loadLog()
			{
				request_url = communicator_url+'&'+action_prefix+'=monitor&'+logname_index+'='+current_log+'&rand='+ Math.random()+'&'+num_lines_index+'='+num_lines;
				if (null !== last_offset) {
					request_url = request_url+'&'+offset_index+'='+last_offset;
				}

				document.getElementById('sq-log-monitor').src = request_url;
				return true;

			}//end loadLog()


			//// Setter Functions
			/**
			* Load current log into the monitor frame
			*/
			function setRefreshInterval(interval)
			{
				var value = parseInt(interval);
				if (!isNaN(value)) {
					refresh_interval = value * 1000;
					enableRefresh();
				}

			}//end setRefreshInterval()


			/**
			* Set the number of lines to display
			*/
			function setNumLines(new_num_lines)
			{
				var value = parseInt(new_num_lines);
				if (!isNaN(value)) {
					num_lines = value;
				}
				forceRefresh();
			}//end setRefreshInterval()

			/**
			* Sets the parameters of the last refresh
			*
			* This function is used by the page included by the monitor
			* when loaded, that function passes us some data that we can use in the next request
			* or just for information display
			*
			* @param date_string	value	number of bytes representing the offset into the log file
			* @return void
			*/
			function setLastRefreshInfo(date_string, num_rows, offset)
			{
				setIdValue('sq-log-last-refresh', date_string);
				setIdValue('sq-log-total-rows', num_rows);
				setLastOffset(offset);

			}//end setLastRefreshInfo()


			function setIdValue(id, value)
			{
				document.getElementById(id).innerHTML = value;

			}//end setIdValue()


			/**
			* Set the offset at the end of the last line read from the log
			*
			* this is required for smart reloading, i.e. obtaining entries starting from where
			* the previous request finished
			*
			* @param integer	value	number of bytes representing the offset into the log file
			* @return void
			*/
			function setLastOffset(value)
			{
				var value = parseInt(value);
				if (!isNaN(value)) {
					last_offset = value;
				}

			}//end setLastOffset()


			/**
			* Enable log refresh
			*
			* Sets the appropriate labels and kills the existing timer and starts a new one
			*
			* @return void
			*/
			function enableRefresh()
			{
				if (null !== refresher) {
					clearTimeout(refresher);
				}
				refresher = setInterval(loadLog, refresh_interval);

				setIdValue('sq-log-refresh-status', 'ON');
				document.getElementById('sq-log-refresh-enabler').value=js_translate('Disable');


				log_refresh = true;

			}//end enableRefresh()


			/**
			* Disable log refresh
			*
			* Sets the appropriate labels and kills the timer
			*/
			function disableRefresh()
			{
				if (null !== refresher) {
					clearInterval(refresher);
					refresher = null;
				}

				setIdValue('sq-log-refresh-status', 'OFF');
				document.getElementById('sq-log-refresh-enabler').value=js_translate('Enable');


				log_refresh = false;

			}//end disableRefresh()


			/**
			* Toggle Refresh to the opposite state
			*/
			function toggleLogRefreshStatus()
			{

				if (!log_refresh) {
					enableRefresh();
				} else {
					disableRefresh();
				}

				return true;

			}//end toggleLogRefreshStatus()


			//]]>
		</script>
		<table class="sq-backend-table">
			<tr>
				<th style="width: 10%"><?php echo translate('Log Name'); ?></th>

				<th style="width: 50%"><?php echo translate('Info'); ?></th>

				<th>Action</th>
			</tr>
		<?php
		foreach ($logs as $logname => $rotation) {
			$filepath = SQ_LOG_PATH.'/'.$logname.SQ_CONF_LOG_EXTENSION;
			if (!file_exists($filepath)) continue;

			$last_modified = date('G:i:s - M d', filemtime($filepath));

			$last_rotated = array_get_index(array_get_index($rotation,'rotation'), 'last_timestamp');
			if (empty($last_rotated)) {
				$last_rotated = translate('Never');

			} else {
				$last_rotated = date('G:i:s - M d', $last_rotated);
			}

			?>
			<tr>
				<td><strong><?php echo $logname; ?></strong></td>
				<td>
					<?php echo translate('Modified'); ?>: <strong><?php echo $last_modified; ?></strong>

					<br />
					<?php echo translate('Current log size'); ?>: <strong><?php echo easy_filesize(filesize($filepath)); ?></strong>

					<br />
					<?php echo translate('Rotated'); ?>: <strong><?php echo $last_rotated; ?></strong>

				</td>
				<td>
					<input type="button" onclick="monitorLog('<?php echo $logname; ?>');" value="<?php echo translate('Monitor'); ?>" />

					<?php normal_button($prefix.'_export_to_csv', translate('Download File'), 'this.form.'.$prefix.'_download_file.value = \''.$logname.'\'; this.form.submit(); SQ_FORM_SUBMITTED = false; this.form.'.$prefix.'_download_file.value = 0; return true;');

					?>
				</td>
			</tr>
			<?php
		}
		echo '</table>';
		hidden_field($prefix.'_download_file', 0);

		$o->closeRaw();
		$o->closeSection();

		$o->openSection(translate('Log Monitor'));

			$o->openField('', 'wide_col');
				?>
				<div id="sq-log-monitor-initial-message">
					<?php echo translate('To monitor a log, click "Monitor" in the list of known logs (above).'); ?>

				</div>
				<div id="sq-log-monitor-container" class="sq-log-monitor-container" style="display: none;">
					<table class="sq-backend-table">
						<tr>
							<th><?php echo translate('Log'); ?></th>

							<th><?php echo translate('Loaded'); ?></th>

							<th><?php echo translate('Rows displayed'); ?></th>

							<th><?php echo translate('Max Rows'); ?></th>

							<th><?php echo translate('Auto-Refresh'); ?></th>

							<th><?php echo translate('Refresh Interval'); ?></th>

						</tr>
						<tr>
							<td><strong id="sq-log-name"></strong></td>
							<td><strong id="sq-log-last-refresh"></strong></td>
							<td><strong id="sq-log-total-rows"></strong></td>
							<td>
								<select id="sq-log-num-lines" onchange="setNumLines(this.value)">
									<option value="10">10</option>
									<option value="50" selected="selected">50</option>
									<option value="100">100</option>
									<option value="500">500</option>
									<option value="1000">1000</option>

								</select>
								</td>
							<td>
								<strong id="sq-log-refresh-status">&nbsp;</strong>
								<input type="button" onclick="toggleLogRefreshStatus();" id="sq-log-refresh-enabler" value="" />
							</td>
							<td>
								<select name="sq-log-refresh-interval" id="sq-log-refresh-interval" onchange="setRefreshInterval(this.value)">
									<option value="15">15 <?php echo translate('sec'); ?></option>

									<option value="30" selected="selected">30 <?php echo translate('sec'); ?></option>

									<option value="60">1 <?php echo translate('min'); ?></option>

									<option value="300">5 <?php echo translate('min'); ?></option>

								</select>
								<input type="button" onclick="forceRefresh();" id="sq-log-refresh-force" value="<?php echo translate('Refresh Now'); ?>" />

							</td>
						</tr>
					</table>

					<table class="sq-backend-table sq-log-monitor-table-outer">
						<tr>
							<th><?php echo translate('Metadata'); ?></th>

							<th><?php echo translate('Message'); ?></th>

						</tr>
						<tr>
							<td colspan="2" style="padding: 0 !important">
								<iframe id="sq-log-monitor" src="" style="height: 600px; width: 100%;"></iframe>
							</td>
						</tr>
					</table>

				</div>
				<?php
			$o->closeField();
		$o->closeSection();

		return FALSE;

	}//end paintLogList()


	/**
	* Process the downloading of a log file
	*
	* @param Remap_Manager		$asset	The asset that we are painting
	* @param Backend_Outputter	$o		The outputter to inject the output into
	* @param string				$prefix Prefix for form elements
	*
	* @return boolean
	*/
	public function processDownloadLogFile(Log_Manager $asset, Backend_Outputter $o, $prefix)
	{
		if ($asset->writeAccess() === FALSE) {
			return FALSE;
		}

		$download_file = array_get_index($_POST, $prefix.'_download_file', '');

		if ($download_file !== '') {
			// Get all the valid log files in that directory, and see whether our
			// file is one of them
			$log_file = SQ_DATA_PATH.'/private/logs/'.$download_file.SQ_CONF_LOG_EXTENSION;
			$valid_log_files = glob(SQ_DATA_PATH.'/private/logs/*'.SQ_CONF_LOG_EXTENSION);

			if (array_search($log_file, $valid_log_files) !== FALSE) {
				$fp = fopen($log_file, 'r');
				if ($fp === FALSE) {
					trigger_localised_error('CORE0288', E_USER_WARNING, $download_file);
					return FALSE;
				}

				// Only do the headers if we were able to read the file
				header('Content-Type: text/plain');
				header('Content-Disposition: attachment; filename="'.$download_file.SQ_CONF_LOG_EXTENSION.'"');
				header('Content-Length: '.filesize($log_file));
				header('Last-Modified: '.gmdate('r', filemtime($log_file)));

				while (feof($fp) === FALSE) {
					$chunk = fread($fp, 4096);
					if ($chunk === FALSE) {
						// Uh oh...
						exit(1);
					} else {
						echo $chunk;
					}
				}
				fclose($fp);

				exit(0);
			}

		}//end if download file exists

		return FALSE;

	}//end processDownloadLogFile()


}//end class

?>
