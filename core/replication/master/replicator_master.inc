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
* $Id: replicator_master.inc,v 1.1.2.10 2004/03/09 18:11:53 brobertson Exp $
* $Name: not supported by cvs2svn $
*/

require_once SQ_SYSTEM_ROOT.'/core/replication/replicator.inc';
require_once SQ_SYSTEM_ROOT.'/data/private/conf/replication.inc';
require_once SQ_SYSTEM_ROOT.'/core/replication/master/replication_remote_slave_system.inc';

/**
* Replicator
*
* Purpose
*
*
*
* @author  Blair Robertson <blair@squiz.net>
* @version $Version$ - 1.0
* @package MySource_Matrix
*/
class Replicator_Master extends Replicator
{

	/**
	* Constructor
	*
	*/
	function Replicator_Master()
	{
		$this->Replicator();

		// make sure that we have the zero system id
		if (SQ_CONF_SYSTEM_ID != 0) {
			trigger_error('This system is not able to be the Replicator Master if it does not have a SYSTEM_ID of 0 (zero)', E_USER_ERROR);
		}

	}//end constructor


	/**
	* Attempt to initialise this replicator type
	*
	* @return boolean
	* @access public
	*/
	function initialise()
	{
		if ($this->initialised()) {
			trigger_error('Replicator Master appears to have already been replicated, if you are sure that this not the case, then remove the initialise flag file', E_USER_WARNING);
			return false;
		}

		$GLOBALS['SQ_SYSTEM']->doTransaction('BEGIN');


		require_once SQ_LIB_PATH.'/db_install/db_install.inc';
		if (!db_install(dirname(__FILE__).'/tables.xml')) {
			trigger_error('Unable to initialise Master Replicator System : Database Table Install Failure', E_USER_WARNING);
			$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
			return false;
		}

		if (parent::initialise()) {
			$GLOBALS['SQ_SYSTEM']->doTransaction('COMMIT');
			return true;
		} else {
			$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
			return false;
		}

	}// end initialise()


	/**
	* Load's a remote slave replication system, caches it and returns a reference to that
	*
	* @return object Replication_Remote_Slave_System	NULL on error
	* @access public
	*/
	function &getSlaveSystem($systemid)
	{
		static $null = null;
		if (!$this->initialised()) return $null;

		if (!isset($this->_tmp['systems'][$systemid])) {
			$this->_tmp['systems'][$systemid] = new Replication_Remote_Slave_System($systemid);
			if (is_null($this->_tmp['systems'][$systemid]->id)) {
				unset($this->_tmp['systems'][$systemid]);
				return $null;
			}
		}

		return $this->_tmp['systems'][$systemid];

	}// end getSlaveSystem()


	/**
	* Returns a list of remote systems that exist on this master server
	*
	* @param boolean $active	get active  systems
	* @param boolean $retired	get retired systems
	*
	* @return Array('systemid' => 'name');
	* @access public
	*/
	function getSlaveSystemsList($active=true, $retired=false)
	{
		if (!$this->initialised()) return Array();

		$sql = 'SELECT systemid, name
				FROM '.SQ_TABLE_PREFIX.'replication_remote_slave_system';
		if ($active && !$retired) {
			$sql .= ' WHERE retired IS NULL';
		} elseif (!$active && $retired) {
			$sql .= ' WHERE retired IS NOT NULL';
		}

		$result = $GLOBALS['SQ_SYSTEM']->db->getAssoc($sql, false, Array(), DB_FETCHMODE_ASSOC);
		if (DB::isError($result)) {
			trigger_error($result->getMessage().'<br/>'.$result->getUserInfo(), E_USER_WARNING);
			return Array();
		}

		return $result;

	}// end getSlaveSystemsList()


	/**
	* Process the commands to the replicator from the command line
	*
	* @return int	the exit status of the command (ie the one that should be passed to exit())
	* @access public
	*/
	function run()
	{
		if (!SQ_PHP_CLI) {
			trigger_error(__CLASS__.'::'.__FUNCTION__.'() can only be run from the command line', E_USER_ERROR);
		}
		$mode = (isset($_SERVER['argv'][1])) ? $_SERVER['argv'][1] : '';

		$title_lines = Array();


		$title = SQ_SYSTEM_LONG_NAME.' System Replicator';
		$title_lines[] = '+-'.str_repeat('-', strlen($title)).'-+';
		$title_lines[] = '| '.$title.' |';
		$title_lines[] = '+-'.str_repeat('-', strlen($title)).'-+';
		$title_lines[] = '| '.str_pad(' MASTER SYSTEM ', strlen($title), ' ', STR_PAD_BOTH).' |';
		$title_lines[] = '+-'.str_repeat('-', strlen($title)).'-+';
		$title_lines[] = '';
		echo implode("\n", $title_lines)."\n";

		switch($mode) {
			case 'init' :
				if ($this->initialise()) {
					echo 'Master Replication System Initialised'."\n";
					return 0;
				} else {
					return 1;
				}
				break;

			case 'slave-create' :
				if (!isset($_SERVER['argv'][2]) || !isset($_SERVER['argv'][3]) || !isset($_SERVER['argv'][4])) {
					$this->runPrintUsage($mode);
					return 1;
				}

				$rs = new Replication_Remote_Slave_System();
				if (!$rs->create($_SERVER['argv'][2], $_SERVER['argv'][3], $_SERVER['argv'][4])) {
					return 1;
				}
				echo 'Remote System "'.$rs->name.'" created.'."\n";
				echo 'System Id : '.$rs->id."\n";
				echo 'You may now edit this system configuration file :'."\n";
				echo '  '.$rs->confFile()."\n";
				return 0;

				break;

			case 'slave-list' :

				$list_type = (isset($_SERVER['argv'][2])) ? $_SERVER['argv'][2] : 'active';
				switch($list_type) {
					case 'all' :
						$active  = true;
						$retired = true;
						break;
					case 'active' :
						$active  = true;
						$retired = false;
						break;
					case 'retired' :
						$active  = false;
						$retired = true;
						break;
					default :
					$this->runPrintUsage($mode);
					return 1;
				}// end switch

				$remote_systems = $this->getSlaveSystemsList($active, $retired);
				if (empty($remote_systems)) {
					echo "No Remote Systems found\n";
					return 0;
				}

				echo ucwords($list_type).' Remote Systems'."\n";

				echo str_pad(' ID', 5, ' ', STR_PAD_LEFT).' : Name'."\n";
				echo str_pad(' --', 5, ' ', STR_PAD_LEFT).'   ----'."\n";
				foreach ($remote_systems as $id => $name) {
					echo str_pad($id, 5, ' ', STR_PAD_LEFT).' : '.$name."\n";
				}// end foreach

				return 0;

				break;

			case 'slave-retire' :
				if (!isset($_SERVER['argv'][2])) {
					$this->runPrintUsage($mode);
					return 1;
				}// end if

				$rs = &$this->getSlaveSystem($_SERVER['argv'][2]);
				if (!$rs->id) {
					trigger_error($_SERVER['argv'][2].' is not a valid system', E_USER_WARNING);
					return 1;
				}

				if ($rs->isRetired()) {
					echo 'Remote System #'.$rs->id.' ("'.$rs->name.'") is already retired'."\n";
					return 0;
				}

				$confirm = $this->_getCmdLineInput('Are you sure you want to retire System '.$rs->id.' ("'.$rs->name.'") ? (y/N) ');
				if (strtolower($confirm) != 'y') {
					return 0;
				}
				if (!$rs->retire()) {
					return 1;
				}

				echo 'Remote System "'.$rs->name.'" (#'.$rs->id.') permanently retired.'."\n";
				echo 'System Id : '.$rs->id."\n";
				return 0;

				break;

			case 'install-point-create' :

				if (false !== ($install_point = $this->createInstallPoint())) {
					echo 'Installation Point Created.'."\n";
					echo 'Available here : .'."\n";
					echo '  '.$install_point."\n";
					return 0;
				} else {
					return 1;
				}

				break;

			case '-h'     :
			case '--help' :
			case 'help'   :
				$this->runPrintUsage(isset($_SERVER['argv'][2]) ? $_SERVER['argv'][2] : '');
				break;

			default :
				$this->runPrintUsage();
				return 1;

		}// end switch

	}// end run()


	/**
	* Print out the command line usage for the replicator
	*
	* @param string	$mode	show specific usage for the cmd
	*
	* @return void
	* @access public
	*/
	function runPrintUsage($mode='')
	{
		if (!SQ_PHP_CLI) {
			trigger_error(__CLASS__.'::'.__FUNCTION__.'() can only be run from the command line', E_USER_ERROR);
		}

		$prog = basename($_SERVER['argv'][0]);

		$descs = Array(
			'help'					=> 'Use "'.$prog.' help <command>" for more info on each command',
			'init'					=> 'Intialise this system to be a Master Server in a replication cluster',
			'slave-create'			=> 'Create a Remote Slave System entry',
			'slave-list'			=> 'List the Remote Slave Systems',
			'slave-retire'			=> 'Retire a Remote Slave System entry from service',
			//'install-point-create'	=> 'Create a Remote Slave System replication installation point',
			//'install-point-list'	=> 'List a Remote Slave System replication installation point',
			//'install-point-delete'	=> 'Delete a Remote Slave System replication installation point',
		);

		$lines = Array();

		if ($mode != '' && isset($descs[$mode])) {
			switch($mode) {
				case 'slave-create' :
					$lines[] = 'Usage : '.$prog.' '.$mode.' <system name> <auth type> <transport method>';
					$lines[] = $descs[$mode];
					$lines[] = '  Available Authentication Types : ';
					foreach(Replicator::getAuthTypes() as $code_name => $name) {
						$lines[] = '    - '.$code_name;
					}
					$lines[] = '  Available Transport Methods : ';
					foreach(Replicator::getTransportMethods() as $code_name => $name) {
						$lines[] = '    - '.$code_name;
					}
					break;

				case 'slave-list' :
					$lines[] = 'Usage : '.$prog.' '.$mode.' [active|retired|all]';
					$lines[] = $descs[$mode];
					$lines[] = '  active  - list only active systems (default)';
					$lines[] = '  retired - list only retired systems';
					$lines[] = '  all     - list all active systems';
					break;

				case 'slave-retire' :
					$lines[] = 'Usage : '.$prog.' '.$mode.' <system id>';
					$lines[] = $descs[$mode];
					$lines[] = '  Available Authentication Types : ';
					foreach(Replicator::getAuthTypes() as $code_name => $name) {
						$lines[] = '    - '.$code_name;
					}
					$lines[] = '  Available Transport Methods : ';
					foreach(Replicator::getTransportMethods() as $code_name => $name) {
						$lines[] = '    - '.$code_name;
					}
					break;

				case 'help' :
					$other_descs = $descs;
					unset($other_descs['help']);
					$lines[] = 'Usage : '.$prog.' '.$mode.' ['.implode('|', array_keys($other_descs)).']';
					$lines[] = $descs[$mode];
					break;

				default :
					$lines[] = 'Usage : '.$prog.' '.$mode;
					$lines[] = $descs[$mode];

			}// end switch

		} else {
			$lines[] = 'Usage : '.$prog.' ['.implode('|', array_keys($descs)).']';
			foreach($descs as $mode => $desc) {
				$lines[] = '  '.str_pad($mode, 21, ' ').'- '.$desc;
			}
		}// end if

		echo implode("\n", $lines)."\n";

	}// end runPrintUsage()



	/**
	* Create an installation point, which is used to initialise new slave systems
	*
	* @return string	returns the path to the replication point directory or FALSE on error
	* @access public
	*/
	function createInstallPoint()
	{
		// THERE ARE ISSUES WITH THIS IDEA
		// main one is that asset attributes aren't necessarily going to have the same attributeids....
#		trigger_error('NOT IMPLEMENETED YET', E_USER_ERROR);
#
#
#		$at_time = time();
#		$at_time_db = ts_iso8601($at_time);
#
#		require_once SQ_FUDGE_PATH.'/general/file_system.inc';
#		require_once SQ_FUDGE_PATH.'/var_serialise/var_serialise.inc';
#
#		$dir = SQ_DATA_PATH.'/private/replication/install_points/'.str_replace(' ', '_', $at_time_db);
#		pre_echo($dir);
#
#		if (file_exists($dir)) {
#			trigger_error('Dir "'.$dir.'" already exists, someone must be running a create install point at the same time', E_USER_WARNING);
#			return false;
#		}
#		if (!create_directory($dir)) return false;
#		if (!create_directory($dir.'/tables')) return false;
#
#		require SQ_DATA_PATH.'/private/db/table_columns.inc';
#
#		// remove a few tables that we don't care about
#		unset($tables['lock']);
#
#		$db = &$GLOBALS['SQ_SYSTEM']->db;
#
#		$all_ok = true;
#		// OK first dump all the tables that don't have the rollback information,
#		// because these could change at any point, whereas we can use the rollback tables to
#		// get the contents of the other tables at a certain point in time
#		foreach(Array('package', $tables as $table_name => $data) {
#			if ($tables[$table_name]['rollback']) continue;
#
#			$f = fopen($dir.'/tables/'.$table_name, 'wb');
#			if (!$f) return false;
#
#			$sql = 'SELECT '.implode(',', $tables[$table_name]['columns']).'
#					FROM '.SQ_TABLE_PREFIX.$table_name;
#pre_echo($sql);
#			$result = $db->query($sql);
#			if (DB::isError($result)) {
#				trigger_error($result->getMessage().'<br/>'.$result->getUserInfo(), E_USER_WARNING);
#				return false;
#			}
#
#			$table_ok = true;
#			while (null !== ($row = $result->fetchRow())) {
#				$str = '$row = "'.var_serialise($row, 1).'";'."\n";
#				if (!fwrite($f, $str)) return false;
#			}//end while
#
#			$result->free();
#			fclose($f);
#
#		}// end foreach
#
#		return false;

	}//end createInstallPoint()


	/**
	* Paints the interface for our config interface
	*
	* @param object Backend_Outputter	&$o			reference to the backend outputter
	*
	* @return void
	* @access public
	* @see processBackend()
	*/
	function paintBackend(&$o, $write_access=false)
	{

//		if (!empty($this->_systems)) {
//			$o->openSection('Current Remote Systems');
//				for($i = 0, $num_systems = count($this->_systems); $i < $num_systems; $i++) {
//					$this->_systems[$i]->paintBackend($o, $write_access);
//				}
//			$o->closeSection();
//		}

	}// end paintBackend()


	/**
	* Saves the config settings that were submitted
	* NOTE: Does NO write access checking - Assumes that this will not be run if no write access
	*
	* @param object Backend_Outputter	&$o			reference to the backend outputter
	*
	* @return boolean	false on error, true on everything else (including nothing happening)
	* @access public
	* @see paintBackend()
	*/
	function processBackend(&$o)
	{
//		$GLOBALS['SQ_SYSTEM']->doTransaction('BEGIN');
//		if (!empty($this->_systems)) {
//			for($i = 0, $num_systems = count($this->_systems); $i < $num_systems; $i++) {
//				if(!$this->_systems[$i]->processBackend($o)) {
//					$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
//					return false;
//				}
//			}
//		}

		return true;
	}// end processBackend()


	/**
	* The Replicate fn, called from the Squiz Server Process Object
	* Basically if this is called it means that a Slave Server initated a connection to us.
	* So once we authenticate that it is a slave server that we know, we essentially say "what do you want?"
	*
	* @param string		$systemid		the systemid of the system that we are dealing with (to be authenticated)
	* @param resource	$client_fd		the client socket resource
	*
	* @return void
	* @access public
	*/
	function serverConnection($systemid, $client_fd)
	{
		// now because of the nature of server rep and it never really being seen we are
		// going to force the issue with logging errors and set the error log file
		$old_log_errors = ini_set('log_errors', 1);

		$this->_setErrorHandler();

		if(!SQ_REPLICATION_IS_MASTER) {
			trigger_error('This System is NOT designated as a Master System, why are we using a '.__CLASS__.' object', E_USER_WARNING);
			$this->_restoreErrorHandler();
			return;
		}

		$slave = &$this->getSlaveSystem($systemid);
		if (is_null($slave)) {
			trigger_error('Unable to get the Remote Slave System "'.$systemid.'"', E_USER_WARNING);
			$this->_restoreErrorHandler();
			return;
		}

		$slave->transport->setSocketFd($client_fd);

		$this->_log("AUTHENTICATE WITH MASTER SERVER", 'debug');

		if (!$slave->authenticate(false)) {
			trigger_error('Unable to authenticate Slave Server', E_USER_WARNING);
		} else {
			$this->_log("SLAVE ".$slave->name." AUTHENTICATED!!!!!!!!", 'debug');

			if (true !== ($err_msg = $slave->transport->sendAndReceiveVar('Select Mode', $mode=null, $headers=null))) {
				trigger_error('Replicate Error ('.$slave->name.' ['.$slave->transport->getPeerInfo().']) : '.$err_msg, E_USER_WARNING);
				return;
			}

			$this->_log("MODE : $mode", 'debug');

			switch($mode) {
				// they are just testing the connection
				case 'test' :
					$msg = Array(
							'Gday, I am the master server and I know you as "'.$slave->name.'".',
							'Version      : '.SQ_SYSTEM_LONG_NAME,
							'System Name  : '.SQ_CONF_SYSTEM_NAME,
							'Current Time : '.$GLOBALS['SQ_SYSTEM']->datetime(),
							'cya',
							);
					$slave->transport->writeVar($msg);
					break;

				// ah, we have a new slave system that wants to be installed...
				case 'install_slave' :
					$slave->install();
					break;

				// ah, we have a new slave system that has been installed...
				case 'slave_versions_init' :
					$slave->versions_init();
					break;

				// one of our slaves wants to replicate
				case 'slave_replicate' :
					$slave->replicate();
					break;

				default :
					trigger_error('Replicate Error ('.$slave->name.' ['.$slave->transport->getPeerInfo().']) : Mode "'.$mode.'" not known', E_USER_WARNING);
					if (!$slave->transport->writeVar('Mode : "'.$mode.'" not known')) {
						trigger_error('Replicate Error ('.$slave->name.' ['.$slave->transport->getPeerInfo().']) : '.$err_msg, E_USER_WARNING);
					}
			}// end switch

		}// end if authenticate

		$slave->transport->removeSocketFd(true);
		ini_set('log_errors', $old_log_errors);
		$this->_restoreErrorHandler();

	}// serverConnection()


}//end class

?>
