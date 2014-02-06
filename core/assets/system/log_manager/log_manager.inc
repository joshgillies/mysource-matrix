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
* $Id: log_manager.inc,v 1.33 2013/04/23 00:51:11 ewang Exp $
*/

require_once SQ_INCLUDE_PATH.'/asset.inc';

/**
* Log_Manager
*
* Purpose
*    Manages the log files. All logging in the system is handled by this class
*
* @author  Andrei Railean <arailean@squiz.net>
* @author  Robert Howard <rhoward@squiz.net>
* @version $Revision: 1.33 $
* @package MySource_Matrix_Packages
* @subpackage __core__
*/
class Log_Manager extends Asset
{

	protected $log_level_map;

	protected $min_tail_lines = 10;


	/**
	* Constructor
	*
	* @param int	$assetid	the asset id to be loaded
	*
	*/
	function __construct($assetid=0)
	{
		$this->_ser_attrs = TRUE;
		parent::__construct($assetid);

	}//end constructor


	/**
	* Create this asset
	*
	* Returns either the linkid or FALSE on error
	*
	* @param array	&$link	information used to create the initial link
	*
	* @return mixed int|boolean
	* @access public
	* @see Asset::create()
	*/
	public function create(Array &$link)
	{
		require_once SQ_CORE_PACKAGE_PATH.'/system/system_asset_fns.inc';
		if (!system_asset_fns_create_pre_check($this)) {
			return FALSE;
		}

		$GLOBALS['SQ_SYSTEM']->changeDatabaseConnection('db2');
		$GLOBALS['SQ_SYSTEM']->doTransaction('BEGIN');

		if ($linkid = parent::create($link)) {
			if (!system_asset_fns_create_cleanup($this)) {
				$linkid = FALSE;
			}
		}

		if ($linkid) {
			$GLOBALS['SQ_SYSTEM']->doTransaction('COMMIT');
		} else {
			$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
		}

		$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();
		return $linkid;

	}//end create()


	/**
	* Return an array of all the permitted link types, the type asset and the cardinality
	*
	* @return array
	* @access private
	* @see Asset::_getAllowLinks()
	*/
	public function _getAllowedLinks()
	{
		return Array(
				SQ_LINK_TYPE_1	=> Array(),
				SQ_LINK_TYPE_2	=> Array(),
				SQ_LINK_TYPE_3	=> Array(),
				SQ_LINK_NOTICE	=> Array(),
			   );

	}//end _getAllowedLinks()


	/**
	* Return FALSE always because we don't want to be able to clone a log manager
	*
	* @return boolean
	* @access public
	*/
	public function canClone()
	{
		return FALSE;

	}//end canClone()


	/**
	* Return a name to use to describe this instance of the asset
	*
	* @param boolean	$short_name	whether or not we are after the shortname or the full name
	*
	* @return string
	* @access private
	* @see Asset::_getName()
	*/
	protected function _getName($short_name=FALSE)
	{
		return $GLOBALS['SQ_SYSTEM']->am->getTypeInfo($this->type(), 'name');

	}//end _getName()


//--        LOG REGISTRY & SETTINGS        --//


	/**
	* Returns a list of all the logs and their details
	* If an unregistered log is found in the log directory, this function will return it with default settings
	* If no logs are found, an empty array will be returned
	*
	* Logs are returned in the following format:
	* <pre>
	* Array(
	*       logname => Array(
	*                        rotation => Array(
	*                                          last_timestamp        => int  // unix time
	*                                          min_index             => int  // the smallest index a log rotation can have
	*                                          max_index             => int  // the largest index a log rotation can have
	*                                          interval => int  // the number of seconds between rotations
	*                                          ),
	*                        ),
	*       )
	* </pre>
	*
	* @return array
	* @access public
	*/
	public function getLogs()
	{
		$files = glob(SQ_LOG_PATH.'/*.log');
		$logs = $this->attr('logs');

		foreach ($files as $file) {
			$logname = $this->_getLogPrefixFromFileName($file);
			if (!empty($logname)) {
				if (!isset($logs[$logname])) {
					$logs[$logname] = $this->_setLogDefaults();
				}
			}
		}

		return $logs;

	}//end getLogs()


	/**
	* Determines the prefix of a log by the filename
	*
	* Returns FALSE if it can't work it out
	*
	* @param string	$file	The filename to get the prefix from
	*
	* @return mixed boolean|string
	* @access private
	*/
	protected function _getLogPrefixFromFileName($file)
	{
		preg_match('/^'.preg_quote(SQ_LOG_PATH.'/', '/').'(\S*).log$/', $file, $matches);
		if (isset($matches[1]) && !empty($matches[1])) {
			return $matches[1];
		} else {
			return FALSE;
		}

	}//end _getLogPrefixFromFileName()


	/**
	* Returns the settings for one log (or FALSE if it doesn't exist)
	*
	* @param string	$logname	the name of the log
	*
	* @return mixed boolean|array
	* @access public
	*/
	public function getLogSettings($logname)
	{
		$logs = $this->getLogs();
		if (isset($logs[$logname]) && !empty($logs[$logname])) {
			return $logs[$logname];
		} else {
			return FALSE;
		}

	}//end getLogSettings()


//--        LOG READING & WRITING        --//


	/**
	* Returns a line from a specified logfile
	*
	* @param string	$logname		the logfile prefix
	* @param int	$rotation_index	optional, the rotation index of the past log to access
	* @param int	$offset			start from this offset
	*
	* @return mixed boolean|array
	* @access public
	*/
	public function &getLogLine($logname, $rotation_index=NULL, $offset=0)
	{
		$rotation_suffix = (!is_null($rotation_index) && is_numeric($rotation_index)) ? '.'.$rotation_index : '';

		$read_data = $this->_readLineFromFile(SQ_LOG_PATH.'/'.$logname.SQ_CONF_LOG_EXTENSION.$rotation_suffix, $offset);

		if (!empty($read_data)) {
			$read_data['line'] = $this->_decodeLine(array_get_index($read_data, 'line', FALSE));
			return $read_data;
		} else {
			return FALSE;
		}

	}//end getLogLine()


	/**
	* Returns the size of the log in bytes
	*
	* @param string		$logname		the name of the log to get the size of
	* @param NULL|int	$rotation_index	the index of the log rotation
	*
	* @return int
	* @access private
	*/
	public function getLogSize($logname, $rotation_index=NULL)
	{
		$rotation_suffix = (!is_null($rotation_index) && is_numeric($rotation_index)) ? '.'.$rotation_index : '';
		return $this->_getFileSize(SQ_LOG_PATH.'/'.$logname.SQ_CONF_LOG_EXTENSION.$rotation_suffix);

	}//end getLogSize()


	/**
	* Returns a reference to a new Log_Iterator that acts as a bridge between a log_report and the log_manager
	*
	* @param string		$logname		the name of the log
	* @param NULL|int	$rotation_index	the index of the log rotation
	*
	* @return object
	* @access private
	*/
	public function getLogIterator($logname, $rotation_index=NULL)
	{
		include_once 'log_iterator.inc';
		// start up the generate report HIPO
		$iterator = new Log_Iterator($logname, $rotation_index);
		return $iterator;

	}//end getLogIterator()


	/**
	* Sets the default settings for 'logs', if they are not already set
	* Returns the updated settings
	*
	* @param array	$log	The array containing the settings for a particular log type, ie. $_vars['logs']['logname']
	*
	* @return array
	* @access private
	*/
	protected function _setLogDefaults($log=Array())
	{
		$rotation = array_get_index($log, 'rotation', Array());

		$log['rotation']['interval']       = array_get_index($rotation, 'interval', 86400);
		$log['rotation']['min_index']      = array_get_index($rotation, 'min_index', 1);
		$log['rotation']['max_index']      = array_get_index($rotation, 'max_index', 10);
		$log['rotation']['last_timestamp'] = array_get_index($rotation, 'last_timestamp', 0);

		return $log;

	}//end _setLogDefaults()


	/**
	* Decodes a line after being read from a logfile
	*
	* Returns an array in the following format:
	* <pre>
	* Array(
	*       'userid'    => int,
	*       'user'      => string,
	*       'date'      => int,
	*       'level'     => int,
	*       'data'     => string|array,
	*       );
	* </pre>
	* or FALSE if the metadata can't be decoded or the line is empty
	*
	* @param string	$line	The line of raw data to decode
	*
	* @return mixed boolean|array
	* @access private
	*/
	protected function _decodeLine($line)
	{
		if (empty($line)) return FALSE;

		// matches the first three brackets containing metadata, then splits off the rest of the line containing the log data
		// the string contain data in the format:
		// [DATE][USERNAME:USERID][LEVEL(LEVELID)][FLAG] DATA
		// NOTE: only flags ' ' and 'S' are read - raw ('R') log entries are ignored
		preg_match('/\[([^[\]]*)\]\[([^[\]]*):([^[\]]*)\]\[(\d+):([^[\]]*)\]\[(.)\] (.*)/', $line, $matches);

		if (empty($matches)) return FALSE;

		// unmangle the user name
		$user = $matches[3];
		$user = str_replace('&#91;', '[', $user);
		$user = str_replace('&#93;', ']', $user);
		$user = str_replace('&#58;', ':', $user);
		$user = str_replace('&#10;', "\n", $user);
		$data = str_replace('&#13;', "\r", $user);

		$data = $matches[7];
		$data = str_replace('&#10;', "\n", $data);
		$data = str_replace('&#13;', "\r", $data);

		$decoded_line = Array(
							'userid'	=> $matches[2],
							'user'		=> $user,
							'date'		=> strtotime($matches[1]),
							'level'		=> (int) $matches[4],
							'data'		=> $data,
						);

		// unserialise the data if it is marked as 'S'(erialised)
		if ($matches[6] == 'S') {
			$decoded_line['data'] = unserialize($decoded_line['data']);
		}

		return $decoded_line;

	}//end _decodeLine()


	/**
	* Read a line starting from a particular offset, and return it along with the new offset
	*
	* The returned array is in the format:
	* <pre>
	* Array(
	*       'line'   => string   // the line read from the file
	*       'offset' => integer  // the new offset
	*       }
	* </pre>
	*
	* @param string	$file	the logfile to read
	* @param int	$offset	the offset from which the read will start, defaults to 0
	*
	* @return mixed boolean|array
	* @access private
	*/
	protected function _readLineFromFile($file, $offset=0)
	{
		if (empty($file)) return FALSE;

		$file_contents = Array();

		if (file_exists($file) && $handle = fopen($file, 'r')) {
			if (fseek($handle, $offset) >= 0 && !feof($handle)) {
				$line = fgets($handle);
				// new offset
				$offset = ftell($handle);
				$result = Array(
							'line'		=> trim($line),
							'offset'	=> $offset,
						  );
			} else {
				$result = FALSE;
			}
			fclose($handle);
		} else {
			trigger_localised_error('CORE0018', E_USER_ERROR, $file);
			$result = FALSE;
		}

		return $result;

	}//end _readLineFromFile()


	/**
	* Returns the size (in bytes) of a particular file
	*
	* @param string	$file	the file to read
	*
	* @return int
	* @access private
	*/
	protected function _getFileSize($file)
	{
		if (empty($file)) return FALSE;

		$filesize = FALSE;
		if (file_exists($file)) {
			$filesize = sprintf('%u', filesize($file));
		}

		return $filesize;

	}//end _getFileSize()


//--        LOG ROTATION        --//


	/**
	* Rotates all the logs in the system
	* Returns a list of logs that were rotated
	*
	* @return array
	* @access public
	*/
	public function rotateLogs()
	{
		$logs = $this->getLogs();
		$status = FALSE;
		$rotated_logs = Array();
		$rotate_hr = $this->attr('log_rotate_time');
		$rotate_hr_fwd = ($rotate_hr > 22) ? '0' : ($rotate_hr + 1);
		$time_to_rotate = mktime($rotate_hr, '0', '0');
		$time_to_rotate_fwd = mktime($rotate_hr_fwd, '0', '0');

		// Rotate the logs within the hour it should take place
		if (time() >= $time_to_rotate && time() <= $time_to_rotate_fwd) {
			foreach ($logs as $logname => $log_data) {
				// determine whether we need to rotate the log
				// this formula should calculate:
				// [Time to Rotate] - [Last Rotation] >= [Within 24 Hours]
				// interval is needed to make sure we do not rotate many times within the day
				if ($time_to_rotate - $log_data['rotation']['last_timestamp'] >= ($log_data['rotation']['interval']-360)) {
					if ($this->rotateLog($logname, $log_data)) {
						$logs[$logname] = $log_data;
						$rotated_logs[] = $logname;
					}
				}

			}
		}//end if

		// turn on forced run level, as not everything has write privileges to log_manager's attributes
		$GLOBALS['SQ_SYSTEM']->setRunLevel(SQ_RUN_LEVEL_FORCED);

		$this->setAttrValue('logs', $logs);
		$this->saveAttributes();

		$GLOBALS['SQ_SYSTEM']->restoreRunLevel();

		return $rotated_logs;

	}//end rotateLogs()


	/**
	* Rotates the specified log, modifies and returns the settings it was passed
	*
	* @param string	$logname	the log to rotate
	* @param string	&$log_data	the settings associated with the log
	*
	* @return mixed boolean|array
	* @access public
	*/
	public function rotateLog($logname, &$log_data)
	{
		$rotation       = array_get_index($log_data, 'rotation', Array());
		$interval       = array_get_index($rotation, 'interval');
		$min_index      = array_get_index($rotation, 'min_index');
		$max_index      = array_get_index($rotation, 'max_index');
		$last_timestamp = array_get_index($rotation, 'last_timestamp');

		// check to see if we have the settings we need, otherwise, fail
		if (empty($rotation) || empty($interval) || !is_numeric($min_index) || empty($max_index) || $min_index > $max_index) {
			$status = FALSE;
		} else {

			/**
			* Get an array of existing log filenames
			* Saves doing file_exists checks inside the while loop below
			* but does have small room for a race condition
			* - file exists when we do the glob
			* - doesn't exist by the time we get to the rotate part.
			* or vice-versa
			*/
			$existing_logs = glob(SQ_LOG_PATH . '/' . $logname . SQ_CONF_LOG_EXTENSION . '*');

			/**
			* Start at $max_index and work our way backwards.
			* If the log exists, move it to the next name.
			*/
			$log_id = $max_index;
			while ($log_id > 0) {
				$full_log_path = SQ_LOG_PATH . '/' . $logname . SQ_CONF_LOG_EXTENSION;
				/**
				 * The max_index file is a special case
				 * the highest file just gets deleted, not moved.
				 * once we've dealt with the max_index case, skip to the next log to look at.
				 */
				if ($log_id == $max_index) {
					if (in_array($full_log_path . '.' . $max_index, $existing_logs)) {
						$log_deleted = @unlink($full_log_path . '.' . $max_index);
						if (!$log_deleted) {
							return FALSE;
						}
					}

					$log_id--;
					continue;
				}

				if (in_array($full_log_path . '.' . $log_id, $existing_logs)) {
					$status = $this->_rotateLogFiles($full_log_path, $log_id, ($log_id+1));
					// if we can't move a log file, break out of the loop.
					if (!$status) {
						break;
					}
				}
				$log_id--;
			}

			/**
			* After moving the rest, move the original log file.
			*/
			$status = $this->_rotateLogFiles(SQ_LOG_PATH.'/'.$logname.SQ_CONF_LOG_EXTENSION, '', '1');

			if ($status) {
				$rotate_hr = $this->attr('log_rotate_time');
				$time_to_rotate = mktime($rotate_hr, '0', '0');
				$log_data['rotation']['interval']       = $interval;
				$log_data['rotation']['min_index']      = $min_index;
				$log_data['rotation']['max_index']      = $max_index;
				$log_data['rotation']['last_timestamp'] = $time_to_rotate;
			} else {
				$status = FALSE;
			}

		}
		return $status;

	}//end rotateLog()


	/**
	* Rotate a logfile from the specified rotation index to the new rotation index. eg "$file.$current_index" to "$file.$new_rotation"
	* if current_rotation is empty, we're trying to move 'file.log' for example to 'file.log.1'
	* if it's not empty, we're moving 'file.log.1' to 'file.log.2'
	*
	* @param string	$file				the logfile to rotate
	* @param string	$current_rotation	the current rotation index
	* @param string	$new_rotation		the new rotation index
	*
	* @return boolean
	* @access private
	*/
	protected function _rotateLogFiles($file, $current_rotation, $new_rotation)
	{
		require_once SQ_FUDGE_PATH.'/general/file_system.inc';
		if (file_exists($file)) {
			$orig_filename = $file;
			if (!empty($current_rotation)) {
				$orig_filename .= '.' . $current_rotation;
			}
			// Bug #4499 - if the file is locked a move file will not capture log data, so copy and truncate
			if (copy_file($orig_filename, $file.'.'.$new_rotation)) {
				// Set the original modification time on the rotated file
				touch($file.'.'.$new_rotation, filemtime($orig_filename));
				return truncate_file($orig_filename);
			} else {
				return FALSE;
			}//end if
		} else {
			return FALSE;
		}

	}//end _rotateLogFiles()


	/**
	* Get URL of this asset for direct communication
	*
	* Returns a backend URL of this asset that can service direct connections
	* (ignoring backend frames)
	*
	* @return void
	* @access public
	*/
	public function getCommunicatorUrl()
	{
		return $this->getBackendHref().'&'.$this->getPrefix().'_direct_connection=true&ignore_frames=1';

	}//end getCommunicatorUrl()


	/**
	* Backend Painter
	*
	* @param Backend_Outputter	$o	the Backend Outputter class
	*
	* @return void
	* @access public
	*/
	public function paintBackend(Backend_Outputter $o)
	{
		$prefix = $this->getPrefix();

		$direct = array_get_index($_REQUEST, $prefix.'_direct_connection', FALSE);

		// if a direct connection is requested, pass the control
		// to the direct Communicator, otherwise take normal course of action
		if ($direct) {
			$this->_directCommunicator();
			exit; // need this since Matrix tries to print extras after this section is done
		} else {
			parent::paintBackend($o);
		}

	}//end paintBackend()


	/**
	* Direct Communicator
	*
	* Performs the direct communication bypassing matrix interface
	*
	* @return void
	* @access public
	*/
	protected function _directCommunicator()
	{
		require_once SQ_FUDGE_PATH.'/general/file_system.inc';

		$prefix = $this->getPrefix();

		$action  = array_get_index($_REQUEST, $prefix.'_action');
		$value   = array_get_index($_REQUEST, $prefix.'_value');
		$logname = array_get_index($_REQUEST, $prefix.'_log');
		$offset  = array_get_index($_REQUEST, $prefix.'_offset');
		$numlines= array_get_index($_REQUEST, $prefix.'_num_lines');

		$content = '';

		// whitelist filtering for file name injection
		$logname = preg_replace('/[^a-zA-Z0-9_-]+/','', $logname);

		if (empty($action)) {
			echo translate('no_action_supplied');
			return TRUE;

		} else if ($action == 'monitor' && !empty($logname)) {
			$num_lines = empty($value) ? (int)$numlines : (int)$value;
			$filename = SQ_LOG_PATH.'/'.$logname.SQ_CONF_LOG_EXTENSION;

			if (!is_readable($filename)) {
				echo translate('file_not_readable');
				return TRUE;
			}

			if (empty($offset)) {
				$lines = get_last_lines_from_file($filename, $num_lines);
				$new_offset = filesize($filename);
			} else {
				$handle = fopen($filename, 'r');
				if (!$handle) {
					echo translate('cannot_open_file');
					return;
				}

				$error_status = fseek($handle, $offset);
				if ($error_status) {
					fclose($handle);
					$lines = get_last_lines_from_file($filename, $num_lines);
					$new_offset = filesize($filename);
				} else {
					while (!feof($handle)) {
						$lines[] = rtrim(fgets($handle));
					}

					$new_offset = ftell($handle);
					fclose($handle);
					if (count($lines) < $num_lines) {
						$old = $lines;
						$lines = get_last_lines_from_file($filename, $num_lines);
						$new_offset = filesize($filename);
					}
				}
			}

			$date_string = date('G:i:s - d M');
			$content = '
				<table class="sq-backend-table sq-log-monitor-table">
					<!--tr>
						<th style="width: 150px">'.translate('metadata').'</th>
						<th>'.translate('message').'</th>
					</tr-->';

			if (empty($lines)) {
				$content .= translate('empty');
			} else {
				foreach ($lines as $line) {
					if (empty($line)) continue;
					$d_line = $this->_decodeLine($line);
					if ($d_line !== FALSE) {
						$message = $d_line['data'];
						if (is_array($message)) {
							$message = '<pre>'.print_r($message, TRUE).'</pre>';
						}
						$message = htmlspecialchars($message);
						$message = nl2br($message);
						$error_type_name   = get_error_name($d_line['level']);
						$error_colours = get_error_colour($d_line['level']);
						$error_type_colour = $error_colours[0];
						$log_date = date('G:i:s - d M',$d_line['date']);

						$content .= '
						<tr class="sq-error-type-'.$error_type_name.'">
							<td style="background-color: '.$error_type_colour.';">
								'.$log_date.'<br />
								'.translate('user').':&nbsp;<strong>'.$d_line['user'].'</strong>&nbsp;('.$d_line['userid'].')<br />
								'.translate('level').':&nbsp;<strong>'.$error_type_name.'</strong><br />
							</td>
							<td><span style="font-family: \'Courier New\', Courier, monospace;">'.$message.'</span></td>
						</tr>';
					} else {
						$line = htmlspecialchars($line);
						$line = nl2br($line);
						$content .= '
						<tr class="alt sq-error-type-raw">
							<td>
								<strong>'.translate('raw_entry').':</strong>
							</td>
							<td><span style="font-family: \'Courier New\', Courier, monospace;">'.$line.'<span></td>
						</tr>';
					}

				}//end foreach
			}//end else

			$content .= '</table><p align="center" style="font-size: 11px; margin-bottom: 7px;">End of log</p>';

		} else {
			trigger_localised_error('CORE0125', E_USER_WARNING);
			return TRUE;
		}

		?>
		<html>
				<head>
					<title><?php echo translate('log_monitor'); ?></title>
					<link rel="stylesheet" type="text/css" href="<?php echo sq_web_path('lib');?>/web/css/edit.css" />
					<script type="text/javascript">
						//<![CDATA[
						/**
						* Get the absolute co-ordinates of an element
						*
						* @param Object   elt   The element to find the position of
						* @return Array   The co-ordinates (x=>123, y=>456)
						*/
						function getEltPosition(elt)
						{
							var posX = 0;
							var posY = 0;
							while (elt != null) {
							   posX += elt.offsetLeft;
							   posY += elt.offsetTop;
							   elt = elt.offsetParent;
							}
							return {x:posX,y:posY};

						}//end getEltPosition()

						/**
						* Scroll to the bottom of the page
						*
						* @return boolean
						*/
						function scrolToBottom()
						{
							elt = document.getElementById("sq-log-end");
							coords = getEltPosition(elt);

							window.scrollTo(0,coords.y);
							return true;

						}//end getEltPosition()
						//]]>
					</script>
				</head>
				<body onload="scrolToBottom(); parent.setLastRefreshInfo('<?php echo $date_string; ?>','<?php echo count($lines); ?>','<?php echo $new_offset; ?>')">
					<?php echo $content; ?>
					<a name="bottom" id="sq-log-end" />
				</body>
		</html>

		<?php
		return TRUE;

	}//end _directCommunicator()


}//end class


?>
