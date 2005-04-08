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
* $Id: log_manager.inc,v 1.10 2005/04/08 08:13:42 arailean Exp $
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
* @version $Revision: 1.10 $
* @package MySource_Matrix_Packages
* @subpackage __core__
*/
class Log_Manager extends Asset
{

	var $log_level_map;

	var $min_tail_lines = 10;


	// Constructor
	function Log_Manager($assetid=0)
	{
		$this->_ser_attrs = true;
		$this->Asset($assetid);

	}//end constructor


	/**
	* Create this asset
	*
	* @param array	&$link	information used to create the initial link
	*
	* @see Asset::create()
	* @return int|false
	* @access public
	*/
	function create(&$link)
	{
		require_once SQ_CORE_PACKAGE_PATH.'/system/system_asset_fns.inc';
		if (!system_asset_fns_create_pre_check($this)) {
			return false;
		}

		$GLOBALS['SQ_SYSTEM']->doTransaction('BEGIN');

		if ($linkid = parent::create($link)) {
			if (!system_asset_fns_create_cleanup($this)) {
				$linkid = false;
			}
		}

		if ($linkid) {
			$GLOBALS['SQ_SYSTEM']->doTransaction('COMMIT');
		} else {
			$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
		}

		return $linkid;

	}//end create()


	/**
	* Return an array of all the permitted link types, the type asset and the cardinality
	*
	* @see Asset::_getAllowLinks()
	* @return array
	* @access private
	*/
	function _getAllowedLinks()
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
	function canClone()
	{
		return false;

	}//end canClone()


	/**
	* Return a name to use to describe this instance of the asset
	*
	* @param bool	$short_name		whether or not we are after the shortname or the full name
	*
	* @see Asset::_getName()
	* @access private
	*/
	function _getName($short_name=false)
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
	*                                          last_index            => int  // the index of the last rotation
	*                                          last_timestamp        => int  // unix time
	*                                          min_index             => int  // the smallest index a log rotation can have
	*                                          max_index             => int  // the largest index a log rotation can have
	*                                          interval => int  // the number of seconds between rotations
	*                                          ),
	*                        ),
	*       )
	* </pre>
	*
	* @access public
	* @return Array(mixed)
	*/
	function getLogs()
	{
		$files = glob(SQ_LOG_PATH.'/*.log');
		$logs = $this->attr('logs');

		foreach ($files as $file) {
			$logname = $this->_getLogPrefixFromFileName($file);
			if (!empty($logname) && !empty($logname)) {
				if (!isset($logs[$logname])) {
					$logs[$logname] = $this->_setLogDefaults($logname);
				}
			}

		}

		return $logs;

	}//end getLogs()


	/**
	* Determines the prefix of a log by the filename
	*
	* @param string		$file	The filename to get the prefix from
	*
	* @access private
	* @return false|string
	*/
	function _getLogPrefixFromFileName($file)
	{
		preg_match('/^'.preg_quote(SQ_LOG_PATH.'/', '/').'(\S*).log$/', $file, $matches);
		if (isset($matches[1]) && !empty($matches[1])) {
			return $matches[1];
		} else {
			return false;
		}

	}//end _getLogPrefixFromFileName()


	/**
	* Returns the settings for one log
	*
	* @access public
	* @return false|Array(mixed)
	*/
	function getLogSettings($logname)
	{
		$logs = $this->getLogs();
		if (isset($logs[$logname]) && !empty($logs[$logname])) {
			return $logs[$logname];
		} else {
			return false;
		}

	}//end getLogSettings()


//--        LOG READING & WRITING        --//


	/**
	* Returns a line from a specified logfile
	*
	* @param string		$logname			the logfile prefix
	* @param int		$rotation_index		optional, the rotation index of the past log to access
	* @param boolean	$restart_read		if true, then the file will be read from the start, instead of using the stored offset
	*
	* @access public
	* @return false|array
	*/
	function getLogLine($logname, $rotation_index=null, $offset=0)
	{
		$rotation_suffix = (!is_null($rotation_index) && is_numeric($rotation_index)) ? '.'.$rotation_index : '';

		$read_data = $this->_readLineFromFile(SQ_LOG_PATH.'/'.$logname.SQ_CONF_LOG_EXTENSION.$rotation_suffix, $offset);

		if (!empty($read_data)) {
			$read_data['line'] = $this->_decodeLine(array_get_index($read_data, 'line', false));
			return $read_data;
		} else {
			return false;
		}

	}//end getLogLine()


	/**
	* Returns the size of the log in bytes
	*
	* @param string		$logname			the name of the log to get the size of
	* @param null|int	$rotation_index		the index of the log rotation
	*
	* @access private
	* @return integer
	*/
	function getLogSize($logname, $rotation_index=null)
	{
		$rotation_suffix = (!is_null($rotation_index) && is_numeric($rotation_index)) ? '.'.$rotation_index : '';
		return $this->_getFileSize(SQ_LOG_PATH.'/'.$logname.SQ_CONF_LOG_EXTENSION.$rotation_suffix);

	}//end getLogSize()


	/**
	* Returns a reference to a new Log_Iterator that acts as a bridge between a log_report and the log_manager
	*
	* @param string		$logname			the name of the log
	* @param null|int	$rotation_index		the index of the log rotation
	*
	* @access private
	* @return object Log_Iterator
	*/
	function &getLogIterator($logname, $rotation_index=null)
	{
		include_once "log_iterator.inc";
		// start up the generate report HIPO
		$iterator = new Log_Iterator($logname, $rotation_index);
		return $iterator;

	}//end getLogIterator()


	/**
	* Sets the default settings for 'logs', if they are not already set
	* Returns the updated settings
	*
	* @param Array(mixed)	$log	The array containing the settings for a particular log type, ie. $_vars['logs']['logname']
	*
	* @access private
	* @return Array(mixed)
	*/
	function _setLogDefaults($log)
	{
		$rotation = array_get_index($log, 'rotation', Array());

		$log['rotation']['interval']       = array_get_index($rotation, 'interval', 86400);
		$log['rotation']['min_index']      = array_get_index($rotation, 'min_index', 1);
		$log['rotation']['max_index']      = array_get_index($rotation, 'max_index', 10);
		$log['rotation']['last_index']     = array_get_index($rotation, 'last_index', null);
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
	*
	* @param string		$line	The line of raw data to decode
	*
	* @access private
	* @return false|string
	*/
	function _decodeLine($line)
	{
		if (empty($line)) {
			return false;
		}

		// matches the first three brackets containing metadata, then splits off the rest of the line containing the log data
		// the string contain data in the format:
		// [DATE][USERNAME:USERID][LEVEL(LEVELID)][FLAG] DATA
		// NOTE: only flags ' ' and 'S' are read - raw ('R') log entries are ignored
		preg_match('/\[([^[\]]*)\]\[([^[\]]*):([^[\]]*)\]\[(\d+):([^[\]]*)\]\[(.)\] (.*)/', $line, $matches);

		if (empty($matches)) {
			return false;
		}

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
	* @param string		$file		the logfile to read
	* @param int		$offset		the offset from which the read will start, defaults to 0
	*
	* @access private
	* @return mixed false|array
	*/
	function _readLineFromFile($file, $offset=0)
	{
		if (empty($file)) {
			return false;
		}

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
				$result = false;
			}
			fclose($handle);
		} else {
			trigger_error('Could not read from logfile: '.$file, E_USER_ERROR);
			$result = false;
		}

		return $result;

	}//end _readLineFromFile()


	/**
	* Returns the size (in bytes) of a particular file
	*
	* @param string		$file	the file to read
	*
	* @access private
	* @return int
	*/
	function _getFileSize($file)
	{
		if (empty($file)) {
			return false;
		}

		$filesize = false;
		if (file_exists($file)) {
			$filesize = sprintf("%u", filesize($file));
		}

		return $filesize;

	}//end _getFileSize()


//--        LOG ROTATION        --//


	/**
	* Rotates all the logs in the system
	* Returns a list of logs that were rotated
	*
	* @access public
	* @return Array(string)
	*/
	function rotateLogs()
	{
		$logs = $this->getLogs();
		$status = false;
		$rotated_logs = Array();

		foreach ($logs as $logname => $log_data) {

			// determine whether we need to rotate the log
			if (time() - $log_data['rotation']['last_timestamp'] > $log_data['rotation']['interval']) {
				$log_data = $this->rotateLog($logname, $log_data);
				if (!empty($log_data)) {
					$logs[$logname] = $log_data;
					$rotated_logs[] = $logname;
				}
			}

		}


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
	* @param string		$logname	the log to rotate
	* @param string     $log_data   the settings associated with the log
	*
	* @access public
	* @return false|Array(mixed)
	*/
	function rotateLog($logname, $log_data)
	{
		$rotation       = array_get_index($log_data, 'rotation', Array());
		$interval       = array_get_index($rotation, 'interval');
		$min_rotation   = array_get_index($rotation, 'min_index');
		$max_rotation   = array_get_index($rotation, 'max_index');
		$last_rotation  = array_get_index($rotation, 'last_index');
		$last_timestamp = array_get_index($rotation, 'last_timestamp');

		// check to see if we have the settings we need, otherwise, fail
		if (empty($rotation) || empty($interval) || empty($min_rotation) || empty($max_rotation)) {
			$status = false;
		} else {

			// if we can't increment the rotation index any higher, start at the beginning again
			$next_rotation = (($last_rotation + 1) > $max_rotation) ? $min_rotation : $last_rotation + 1;
			$status = $this->_rotateLogFiles(SQ_LOG_PATH.'/'.$logname.SQ_CONF_LOG_EXTENSION, $next_rotation);

			if ($status) {
				$status['rotation']['interval']       = $interval;
				$status['rotation']['min_index']      = $min_rotation;
				$status['rotation']['max_index']      = $max_rotation;
				$status['rotation']['last_index']     = $next_rotation;
				$status['rotation']['last_timestamp'] = time();
			} else {
				$status = false;
			}

		}
		return $status;

	}//end rotateLog()


	/**
	* Rotate out a logfile to a file with the specified rotation index appended, eg. "$file.$rotation_index"
	*
	* @param string		$file				the logfile to rotate
	* @param string		$rotation_index		the rotation index the rotation will use
	*
	* @access private
	* @return boolean
	*/
	function _rotateLogFiles($file, $rotation_index)
	{
		require_once SQ_FUDGE_PATH.'/general/file_system.inc';
		if (file_exists($file)) {
			return move_file($file, $file.'.'.$rotation_index);
		} else {
			return false;
		}

	}//end _rotateLogFiles()


	/**
	* Get URL of this asset for direct communication
	*
	* Returns a backend URL of this asset that can service direct connections
	* (ignoring backend frames)
	*
	* @access public
	* @return void
	*/
	function getCommunicatorUrl()
	{
		return $this->getBackendHref().'&'.$this->getPrefix().'_direct_connection=true&ignore_frames=1';

	}//end getCommunicatorUrl()


	/**
	* Backend Painter
	*
	* @param $o object    bakcend outputter
	*
	* @access public
	* @return void
	*/
	function paintBackend(&$o)
	{
		$prefix = $this->getPrefix();

		$direct = array_get_index($_REQUEST, $prefix.'_direct_connection', false);

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
	* @access public
	* @return void
	*/
	function _directCommunicator()
	{
		require_once SQ_FUDGE_PATH.'/general/file_system.inc';

		$prefix = $this->getPrefix();

		$action  = array_get_index($_REQUEST, $prefix.'_action');
		$value   = array_get_index($_REQUEST, $prefix.'_value');
		$logname = array_get_index($_REQUEST, $prefix.'_log');
		$offset  = array_get_index($_REQUEST, $prefix.'_offset');

		$content = '';

		if (empty($action)) {
			echo "No action supplied";
			return true;

		} else if ($action == 'monitor' && !empty($logname)) {
			$num_lines = empty($value)?$this->min_tail_lines:(int)$value;
			$filename = SQ_LOG_PATH.'/'.$logname.SQ_CONF_LOG_EXTENSION;

			if (!is_readable($filename)) {
				echo "File cannot is not readable";
				return true;
			}

			if (empty($offset)) {
				$lines = get_last_lines_from_file($filename, $num_lines);
				$new_offset = filesize($filename);
			} else {
				$handle = fopen($filename, 'r');
				if (!$handle) {
					echo "Cannot open file";
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
			$current_time = date('r');

			$content = '<table class="sq-backend-table"><tr><th style="width: 150px">Meta Data</th><th>Message</th></tr>';
			$date_string = date("G:i:s - d M");
			if (empty($lines)) {
				$content .= "Empty";
			} else {
				foreach ($lines as $line) {
					if (empty($line)) {
						continue;
					}
					$d_line = $this->_decodeLine($line);
					if ($d_line !== false) {
						$content .= '
						<tr>
							<td style="background-color: #eee;">
								'.date('G:i:s - d M',$d_line['date']).'<br />
								User:&nbsp;<strong>'.$d_line['user'].'</strong>&nbsp;('.$d_line['userid'].')<br />
								Level:&nbsp;<strong>'.get_error_name($d_line['level']).'</strong><br />
							</td>
							<td><span style="font: mono;">'.$d_line['data'].'<span></td>
						</tr>';
					} else {
						$content .= '
						<tr class="alt">
							<td style="color: red; background-color: #eef; text-align: right;">
								<strong>RAW:</strong>
							</td>
							<td><span style="font: mono;">'.$line.'<span></td>
						</tr>';
					}

				}
			}

			$content .= '</table>';

		} else {
			trigger_error('Unknown action or invalid input', E_USER_WARNING);
			return true;
		}

		?>
		<html>
				<head>
					<title>Log Monitor</title>
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
		return true;

	}//end _directCommunicator()


}//end class


?>
