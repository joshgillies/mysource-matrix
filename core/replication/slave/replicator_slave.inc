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
* $Id: replicator_slave.inc,v 1.1.2.4 2004/03/09 18:20:16 brobertson Exp $
* $Name: not supported by cvs2svn $
*/

require_once SQ_SYSTEM_ROOT.'/core/replication/replicator.inc';
require_once SQ_SYSTEM_ROOT.'/data/private/conf/replication.inc';
require_once SQ_SYSTEM_ROOT.'/core/replication/slave/replication_remote_master_system.inc';

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
class Replicator_Slave extends Replicator
{

	/**
	* Constructor
	*
	*/
	function Replicator_Slave()
	{
		$this->Replicator();

		// make sure that we have the zero system id
		if (SQ_CONF_SYSTEM_ID == 0) {
			trigger_error('This system is not able to be a Replicator Slave if it has a SYSTEM_ID of 0 (zero)', E_USER_ERROR);
		}

	}//end constructor


	/**
	* Attempt to initialise this replicator type
	*
	* @param string	$auth_type			the authentication type that is going to be used with the master system
	* @param string	$transport_method	the transport method that is going to be used with the master system
	*
	* @return boolean	indicating success or not
	* @access public
	*/
	function initialise($auth_type, $transport_method)
	{
		if ($this->initialised()) {
			trigger_error('Replicator Slave appears to have already been initialised, if you are sure that this not the case, then remove the initialise flag file', E_USER_WARNING);
			return false;
		}

		$GLOBALS['SQ_SYSTEM']->doTransaction('BEGIN');

		require_once SQ_LIB_PATH.'/db_install/db_install.inc';
		if (!db_install(dirname(__FILE__).'/tables.xml')) {
			trigger_error('Unable to initialise Slave Replicator System : Database Table Install Failure', E_USER_WARNING);
			$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
			return false;
		}

		$ms = new Replication_Remote_Master_System(false);

		if (!$ms->setAuthType($auth_type) || !$ms->setTransportMethod($transport_method) ||!$ms->save()) {
			trigger_error('Unable to initialise Slave Replicator System : unable to create Master Remote Replication System', E_USER_WARNING);
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
	* Load's the remote master replication system, caches it and returns a reference to that
	*
	* @return object Replication_Remote_Master_System	NULL on error
	* @access public
	*/
	function &getMasterSystem()
	{
		static $null = null;
		if (!$this->initialised()) return $null;

		if (!isset($this->_tmp['master_system'])) {
			$this->_tmp['master_system'] = new Replication_Remote_Master_System();
			if (is_null($this->_tmp['master_system']->id)) {
				unset($this->_tmp['master_system']);
				return $null;
			}
		}

		return $this->_tmp['master_system'];

	}// end getMasterSystem()


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
		$cmd = (isset($_SERVER['argv'][1])) ? $_SERVER['argv'][1] : '';

		$title_lines = Array();


		$title = SQ_SYSTEM_LONG_NAME.' System Replicator';
		$title_lines[] = '+-'.str_repeat('-', strlen($title)).'-+';
		$title_lines[] = '| '.$title.' |';
		$title_lines[] = '+-'.str_repeat('-', strlen($title)).'-+';
		$title_lines[] = '| '.str_pad(' SLAVE SYSTEM ', strlen($title), ' ', STR_PAD_BOTH).' |';
		$title_lines[] = '+-'.str_repeat('-', strlen($title)).'-+';
		$title_lines[] = '';
		echo implode("\n", $title_lines)."\n";

		switch($cmd) {
			case 'init' :
				if (!isset($_SERVER['argv'][2]) || !isset($_SERVER['argv'][3])) {
					$this->runPrintUsage($cmd);
					return 1;
				}

				if ($this->initialise($_SERVER['argv'][2], $_SERVER['argv'][3])) {
					echo "Slave Replication System Initialised.\n";
					echo "You may now edit this system configuration file for the Master System : \n";
					$ms = &$this->getMasterSystem();
					echo '  '.$ms->confFile()."\n";
					return 0;
				} else {
					return 1;
				}
				break;

			case 'test' :
				if ($this->testConnect()) {
					return 0;
				} else {
					return 1;
				}
				break;


#			case 'install' :
#				if ($this->_install()) {
#					echo "Slave Replication System Installed!!!.\n";
#					return 0;
#				} else {
#					return 1;
#				}
#				break;

			case 'versions_init' :
				if ($this->_versions_init()) {
					echo "Slave Replication System Versions Initialised!!!.\n";
					return 0;
				} else {
					return 1;
				}
				break;

			case 'replicate' :
				if ($this->_replicate()) {
					echo "Slave Replication Successful!!!.\n";
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
	* @param string	$cmd	show specific usage for the cmd
	*
	* @return void
	* @access public
	*/
	function runPrintUsage($cmd='')
	{
		if (!SQ_PHP_CLI) {
			trigger_error(__CLASS__.'::'.__FUNCTION__.'() can only be run from the command line', E_USER_ERROR);
		}

		$prog = basename($_SERVER['argv'][0]);

		$descs = Array(
			'help'			=> 'Use "'.$prog.' help <command>" for more info on each command',
			'init'			=> 'Intialise this system to be a Slave System in a replication cluster',
			'test'			=> 'Attempts to connect to the Master Server',
			//'install'		=> 'Connect to the Master Server and download the contents of the system',
			'versions_init'	=> 'We have a copy of the data, so we now connect to the Master Server and tell it what versions of each asset we have',
			'replicate'		=> 'Contact the Master server and initiate a replication',
		);

		$lines = Array();

		if ($cmd != '' && isset($descs[$cmd])) {
			switch($cmd) {
				case 'init' :
					$lines[] = 'Usage : '.$prog.' init <master system auth type> <master system transport method>';
					$lines[] = $descs[$cmd];
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
					$lines[] = 'Usage : '.$prog.' help ['.implode('|', array_keys($other_descs)).']';
					$lines[] = $descs[$cmd];
					break;

				default :
					$lines[] = 'Usage : '.$prog.' '.$cmd;
					$lines[] = $descs[$cmd];

			}// end switch

		} else {
			$lines[] = 'Usage : '.$prog.' ['.implode('|', array_keys($descs)).']';
			foreach($descs as $cmd => $desc) {
				$lines[] = '  '.str_pad($cmd, 15, ' ').'- '.$desc;
			}
		}// end if

		echo implode("\n", $lines)."\n";

	}// end runPrintUsage()


	/**
	* Attempt to connect to the master server, used for checking authentication and transport settings
	*
	* @return boolean	indicating success or not
	* @access public
	*/
	function testConnect()
	{
		if (!$this->initialised()) {
			trigger_error('Test Connect Failed : Replicator Slave does not appear to be initialised', E_USER_WARNING);
			return false;
		}

		$ms = &$this->getMasterSystem();
		if (is_null($ms)) {
			trigger_error('Test Connect Failed : unable to get the master system', E_USER_WARNING);
			return false;
		}

		if (!$ms->connect()) {
			trigger_error('Test Connect Failed : unable to connect to the master system', E_USER_WARNING);
			return false;
		}

		if (true !== ($err_msg = $ms->transport->receiveAndSendVar('Select Mode', 'test'))) {
			trigger_error('Test Connect Failed : '.$err_msg, E_USER_WARNING);
			return;
		}

		if (true !== ($err_msg = $ms->transport->readVar($data=null, $headers=null))) {
			trigger_error('Test Connect Failed : '.$err_msg, E_USER_WARNING);
			break;
		}

		echo "Connection Successful.\nMessage From Master Server :\n";
		echo ((is_array($data)) ? implode("\n", $data) : $data)."\n";

		$ms->disconnect();

		return true;

	}// end testConnect()


	/**
	* Attempt to initialise this replicator type
	*
	* @return boolean	indicating success or not
	* @access private
	*/
	function _install()
	{
#		if ($this->installed()) {
#			trigger_error('Replicator Slave appears to have already been installed, if you are sure that this not the case, then remove the isntalled flag file', E_USER_WARNING);
#			return false;
#		}
#
#		$e = 'Slave Install : ';
#
#		$ms = &$this->getMasterSystem();
#		if (is_null($ms)) {
#			trigger_error($e.'Unable to get the master system', E_USER_WARNING);
#			return false;
#		}
#
#		if (!$ms->connect()) {
#			trigger_error($e.'Unable to connect to the master system', E_USER_WARNING);
#			return false;
#		}
#
#		if (true !== ($err_msg = $ms->transport->receiveAndSendVar('Select Mode', 'install_slave'))) {
#			trigger_error($e.$err_msg, E_USER_WARNING);
#			return false;
#		}
#
#		if (true !== ($err_msg = $ms->transport->receiveAndSendVar('Start Checks', 'ack'))) {
#			trigger_error($e.$err_msg, E_USER_WARNING);
#			return false;
#		}
#
#		if (true !== ($err_msg = $ms->transport->readVar($checks=null, $headers=null))) {
#			trigger_error($e.$err_msg, E_USER_WARNING);
#			return false;
#		}
#
#		$checks_ok = true;
#		foreach($checks as $name => $data) {
#			switch($name) {
#				case 'php' :
#					if(version_compare(PHP_VERSION, $data['version'], '<')) {
#						trigger_error($e.'Failed PHP Version Check - Master has PHP Version '.$data['version'], E_USER_WARNING);
#						$checks_ok = false;
#						break 2;
#					}
#					break;
#				case 'system' :
#					if(version_compare(SQ_SYSTEM_VERSION, $data['version'], '<')) {
#						trigger_error($e.'Failed System Version Check - Master has System Version '.$data['version'], E_USER_WARNING);
#						$checks_ok = false;
#						break 2;
#					}
#					break;
#
#				case 'crypt' :
#					if (crypt($data['text'], $data['encrypted']) != $data['encrypted']) {
#						trigger_error($e.'Failed crypt() Check', E_USER_WARNING);
#						$checks_ok = false;
#						break 2;
#					}
#					break;
#
#				case 'db' :
#					if ($GLOBALS['SQ_SYSTEM']->db->phptype != $data['phptype']) {
#						trigger_error($e.'Failed db type Check', E_USER_WARNING);
#						$checks_ok = false;
#						break 2;
#					}
#					break;
#
#				case 'asset_tree' :
#					if (SQ_CONF_ASSET_TREE_BASE != $data['SQ_CONF_ASSET_TREE_BASE']) {
#						trigger_error($e.'Failed asset tree Check - Local SQ_CONF_ASSET_TREE_BASE is '.SQ_CONF_ASSET_TREE_BASE.', Server SQ_CONF_ASSET_TREE_BASE is '.$data['SQ_CONF_ASSET_TREE_BASE'], E_USER_WARNING);
#						$checks_ok = false;
#						break 2;
#					}
#					if (SQ_CONF_ASSET_TREE_SIZE != $data['SQ_CONF_ASSET_TREE_SIZE']) {
#						trigger_error($e.'Failed asset tree Check - Local SQ_CONF_ASSET_TREE_SIZE is '.SQ_CONF_ASSET_TREE_SIZE.', Server SQ_CONF_ASSET_TREE_SIZE is '.$data['SQ_CONF_ASSET_TREE_SIZE'], E_USER_WARNING);
#						$checks_ok = false;
#						break 2;
#					}
#					break;
#
#				default :
#					trigger_error($e.'Unknown check "'.$name.'"', E_USER_WARNING);
#					$checks_ok = false;
#					break 2;
#
#			}// end switch
#
#		}// end foreach
#
#		if (!$ms->transport->writeVar('Checks '.(($checks_ok) ? 'OK' : 'Failed'))) {
#			trigger_error($e.'Write Fail', E_USER_WARNING);
#			return false;
#		}
#		if (!$checks_ok) return false;
#
#		// OK we should be getting the versions file next
#		if (!$ms->transport->readVar($num_assets=null, $headers=null)) {
#			trigger_error($e.'Unable to get Number of Assets to check', E_USER_WARNING);
#			return false;
#		}
#
#		if (!is_int($num_assets)) {
#			trigger_error($e.'Unable to get Number of Assets to check', E_USER_WARNING);
#			return false;
#		}
#
#		// OK we should be getting the versions file next
#		if (!$ms->transport->readFile($filename=null, $headers=null)) {
#			trigger_error($e.'Error Reading Asset Versions File', E_USER_WARNING);
#			return false;
#		}
#
#		if (basename($filename) != 'asset_versions.txt') {
#			trigger_error($e.'Error Reading Asset Versions File', E_USER_WARNING);
#			return false;
#		}
#
#		$av_f = fopen($filename, 'rb');
#		if (!$av_f) {
#			trigger_error($e.'Unable to open file "'.$av_filename.'" for reading', E_USER_WARNING);
#			return false;
#		}
#
#		$additions_f = fopen($this->dataPath().'/asset_additions.txt', 'wb');
#		if (!$av_f) {
#			trigger_error($e.'Unable to open file "'.$av_filename.'" for reading', E_USER_WARNING);
#			return false;
#		}
#
#		$changed_f = fopen($this->dataPath().'/asset_changes.txt', 'wb');
#		if (!$av_f) {
#			trigger_error($e.'Unable to open file "'.$av_filename.'" for reading', E_USER_WARNING);
#			return false;
#		}
#
#		$deletions_f = fopen($this->dataPath().'/asset_deletions.txt', 'wb');
#		if (!$av_f) {
#			trigger_error($e.'Unable to open file "'.$av_filename.'" for reading', E_USER_WARNING);
#			return false;
#		}
#
#		$db = &$GLOBALS['SQ_SYSTEM']->db;
#
#
#		$additions
#
#
#
#		$i = 0;
#		$num_per_batch = 15;
#		$master_versions = Array();
#		$quoted_assetids = Array();
#		while($vars = fscanf($av_f, "%s\t%s")) {
#			$master_versions[$vars[0]] = $vars[1];
#			$quoted_assetids[$i % $num_per_batch] = $db->quote($vars[0]);
#			$i++;
#			if ($i % $num_per_batch == 0 || $i == $num_assets) {
#				pre_echo($master_versions);
#				$sql = 'SELECT assetid, version
#						FROM '.SQ_TABLE_PREFIX.'asset
#						WHERE assetid IN ('.implode(',', $quoted_assetids).')
#						ORDER BY assetid';
#pre_echo($sql);
#				$slave_versions = $GLOBALS['SQ_SYSTEM']->db->getAssoc($sql);
#				if (DB::isError($slave_versions)) {
#					trigger_error($e.$slave_versions->getMessage().'<br/>'.$slave_versions->getUserInfo(), E_USER_WARNING);
#					fclose($av_f);
#					return;
#				}
#
#				$diff = array_diff_assoc($master_versions, $slave_versions);
#
#				$diff = array_diff_assoc($master_versions, $slave_versions);
#
#				unset($master_versions);
#				$master_versions = Array();
#				unset($quoted_assetids);
#				$quoted_assetids = Array();
#			}// end if
#		}// end while
#
#		fclose($av_f);
#
#		if ($i < $num_assets) {
#			trigger_error($e.'Different number of assets read to those supposedly sent', E_USER_WARNING);
#			return false;
#		}
#
#
#//		require_once SQ_FUDGE_PATH.'/general/file_system.inc';
#//		// make sure the directory exists
#//		if (!create_directory($this->_data_dir)) return false;
#//		return touch($this->_data_dir.'/.'.get_class($this).'_installed');
#
#		return true;

	}// end _install()


	/**
	* Whether we think that this slave replicator has been installed on this system
	*
	* @return boolean
	* @access public
	*/
	function installed()
	{
		// no need for caching, see clearstatcache()
		return file_exists($this->_data_dir.'/.'.get_class($this).'_installed');
	}// end installed()


	/**
	* Whether we think that this slave replicator has been installed on this system
	*
	* @return boolean
	* @access public
	*/
	function versions_initialised()
	{
		// no need for caching, see clearstatcache()
		return file_exists($this->_data_dir.'/.'.get_class($this).'_versions_initialised');
	}// end versions_initialised()


	/**
	* We have had all the data from the master imported into our system, so we now need to connect to the Master 
	* and tell it what version of each asset we have
	*
	* @return boolean	indicating success or not
	* @access private
	*/
	function _versions_init()
	{
		if ($this->versions_initialised()) {
			trigger_error('Replicator Slave appears to have already had its asset versions initialised, if you are sure that this not the case, then remove the flag file', E_USER_WARNING);
			return false;
		}

		$e = 'Slave Version Init : ';

		$ms = &$this->getMasterSystem();
		if (is_null($ms)) {
			trigger_error($e.'Unable to get the master system', E_USER_WARNING);
			return false;
		}

		if (!$ms->connect()) {
			trigger_error($e.'Unable to connect to the master system', E_USER_WARNING);
			return false;
		}

		if (true !== ($err_msg = $ms->transport->receiveAndSendVar('Select Mode', 'slave_versions_init'))) {
			trigger_error($e.$err_msg, E_USER_WARNING);
			return false;
		}

		$db = &$GLOBALS['SQ_SYSTEM']->db;
		$GLOBALS['SQ_SYSTEM']->doTransaction('BEGIN');
		$sql = 'SELECT assetid, version 
				FROM '.SQ_TABLE_PREFIX.'asset';

		$result = $db->query($sql);
		if (DB::isError($result)) trigger_error($result->getMessage().'<br/>'.$result->getUserInfo(), E_USER_ERROR);

		if (true !== ($err_msg = $ms->transport->receiveAndSendVar('Num Assets', $result->numRows()))) {
			trigger_error($e.$err_msg, E_USER_WARNING);
			$result->free();
			return false;
		}

		if (true !== ($err_msg = $ms->transport->receiveAndSendVar('Send Assets', 'ack'))) {
			trigger_error($e.$err_msg, E_USER_WARNING);
			$result->free();
			return false;
		}

		while (null !== ($row = $result->fetchRow())) {

			$sql = 'INSERT INTO '.SQ_TABLE_PREFIX.'replication_remote_master_asset_versions
					(assetid, version)
					VALUES
					('.$db->quote($row['assetid']).', '.$db->quote($row['version']).')';
$this->_log($sql, 'debug');
			$insert_result = $db->query($sql);
			if (DB::isError($result)) {
				trigger_error($e.$insert_result->getMessage()."\n".$insert_result->getUserInfo(), E_USER_ERROR);
				$result->free();
				$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
				return false;
			}

			if (!$ms->transport->writeVar($row)) {
				trigger_error($e.'Write Fail', E_USER_WARNING);
				$result->free();
				$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
				return false;
			}
		}//end while

		$result->free();

		if (true !== ($err_msg = $ms->transport->receiveAndSendVar('Got Assets', 'ack'))) {
			trigger_error($e.$err_msg, E_USER_WARNING);
			$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
			return false;
		}

		require_once SQ_FUDGE_PATH.'/general/file_system.inc';
		// make sure the directory exists
		if (!create_directory($this->_data_dir) || !touch($this->_data_dir.'/.'.get_class($this).'_versions_initialised')) {
			$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
			return false;
		}

		$GLOBALS['SQ_SYSTEM']->doTransaction('COMMIT');

		return true;

	}// end _versions_init()


	/**
	* The serverConnection fn, called from the Squiz Server Process Object
	* Basically if this is called it means that the Master Server initated a connection to us.
	* And once we authenticate that it is the master server, we essentially say "what do you want?"
	*
	* @param string		$systemid		the systemid of the system that we are dealing with (to be authenticated). 
	*									Because we are the slave this will always be zero
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

		if (SQ_REPLICATION_IS_MASTER) {
			trigger_error('This System is NOT designated as a Slave System, why are we using a '.__CLASS__.' object', E_USER_WARNING);
			$this->_restoreErrorHandler();
			return;
		}

		$ms = &$this->getMasterSystem();
		if (is_null($ms)) {
			trigger_error('Unable to get the Remote Master System', E_USER_WARNING);
			$this->_restoreErrorHandler();
			return;
		}

		// make sure we think we are talking to the right server
		if ($systemid != $ms->id) {
			trigger_error('System called "'.$systemid.'" attempted to replicate with us, but it is not our master server', E_USER_WARNING);
			$this->_restoreErrorHandler();
			return;
		}

		$ms->transport->setSocketFd($client_fd);

		$this->_log("AUTHENTICATE WITH MASTER SERVER", 'debug');

		if (!$ms->authenticate(false)) {
			trigger_error('Unable to authenticate Master Server', E_USER_WARNING);
		} else {

			$this->_log("AUTHENTICATED!!!!!!!!", 'debug');

//			$f = fopen('/home/brobertson/workspace/v3/text.txt', 'rb');
//			$d = dir(SQ_SYSTEM_ROOT.'/core/lib/js');
//
//			while(true) {
//				$this->_log("ASK 'What do you want ?'", 'debug');
//				if (true !== ($err_msg = $ms->transport->sendAndReceiveVar('What do you want ?', $cmd=null, $headers=null))) {
//					trigger_error('Replicate Error ('.$ms->name.' ['.$ms->transport->getPeerInfo().']) : '.$err_msg, E_USER_WARNING);
//					return;
//				}
//
//				$this->_log("What they want : $cmd", 'debug');
//
//				switch($cmd) {
//					case 'text_file' :
//
//						if (!$f) {
//							if (!$ms->transport->writeVar(1)) {
//								trigger_error('Replicate Error ('.$ms->name.' ['.$ms->transport->getPeerInfo().']) : '.$err_msg, E_USER_WARNING);
//								break 2;
//							}
//
//						} else {
//							$line = fgets($f, 4096);
//							if ($line === false) {
//								if (!$ms->transport->writeVar(2)) {
//									trigger_error('Replicate Error ('.$ms->name.' ['.$ms->transport->getPeerInfo().']) : '.$err_msg, E_USER_WARNING);
//									break 2;
//								}
//							} else {
//								if (!$ms->transport->writeVar($line)) {
//									trigger_error('Replicate Error ('.$ms->name.' ['.$ms->transport->getPeerInfo().']) : '.$err_msg, E_USER_WARNING);
//									break 2;
//								}
//							}
//						}
//						break;
//
//					case 'file' :
//
//						$file = '';
//
//						while (false !== ($entry = $d->read())) {
//							if (is_file($d->path.'/'.$entry)) {
//								$file = str_replace(SQ_SYSTEM_ROOT.'/', '', $d->path).'/'.$entry;
//								break;
//							}
//						}
//
//						// no more files
//						if ($file === '') {
//							if (!$ms->transport->writeVar(1)) {
//								trigger_error('Replicate Error ('.$ms->name.' ['.$ms->transport->getPeerInfo().']) : '.$err_msg, E_USER_WARNING);
//								break 2;
//							}
//
//						} else {
//							$this->_log("Send File : $file", 'debug');
//							if (true !== ($err_msg = $ms->transport->writeFile($file))) {
//								trigger_error('Replicate Error ('.$ms->name.' ['.$ms->transport->getPeerInfo().']) : '.$err_msg, E_USER_WARNING);
//								break 2;
//							}
//						}
//						break;
//
//					case 'exit' :
//						break 2; // out of switch and while loop
//
//					default :
//						trigger_error('Replicate Error ('.$ms->name.' ['.$ms->transport->getPeerInfo().']) : unknown cmd "'.$cmd.'"', E_USER_WARNING);
//						break 2; // out of switch and while loop
//				}// end switch
//
//			}// end while
//
//			$d->close();
//			fclose($f);

		}// end if authenticate

		$ms->transport->removeSocketFd(true);
		ini_set('log_errors', $old_log_errors);
		$this->_restoreErrorHandler();

	}// end serverConnection()

}//end class

?>