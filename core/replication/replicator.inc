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
* $Id: replicator.inc,v 1.1.2.2 2004/01/21 18:59:28 brobertson Exp $
* $Name: not supported by cvs2svn $
*/

require_once dirname(__FILE__).'/replication_communication.inc';
require_once dirname(__FILE__).'/replication_remote_system.inc';
require_once SQ_SYSTEM_ROOT.'/data/private/conf/replication.inc';

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
class Replicator extends Replication_Communication
{

	/**
	* @var Array(object Replication_Remote_System)
	*/
	var $_systems = Array();

	/**
	* @var Array(object Replication_Remote_System)
	*/
	var $_log_dir = '';

	/**
	* Constructor
	*
	*/
	function Replicator()
	{
		$this->MySource_Object();
		$this->_log_dir = SQ_DATA_PATH.'/private/logs/replication';
		$this->_loadRemoteSystems();

	}//end constructor


	/**
	* Appends something to the log
	*
	* @param string	$msg		the msg to add to log
	* @param string	$log_type	the log to put the msg  in
	*
	* @access	private
	*/
	function _log($msg, $log_type='general')
	{
		error_log(date('[d-M-Y H:i:s] ').$msg."\n", 3, $this->_log_dir.'/'.$log_type.'.log');
	}// end _log()


	/**
	* Load's all the remote system object's that we have installed into our cache
	*
	* @return void
	* @access private
	*/
	function _loadRemoteSystems()
	{
		trigger_error('LOAD REMOTE SYSTEMS');

	}// end _loadRemoteSystems()


	/**
	* Check's the passed systemid to see whether it's valid or not
	* Returns TRUE if so and the error msg if not
	*
	* @param string	$systemid		the id of the server to create
	* @param string	$auth_method	the authentication method that is used by this server
	*
	* @return boolean
	* @access public
	*/
	function createRemoteSystem($systemid, $auth_method, $enc_method)
	{
		if (true !== ($err_msg = Replication_Remote_System::validSystemId($systemid))) {
			trigger_error('Unable to create a Remote System "'.$this->id.'" : '.$err_msg, E_USER_WARNING);
			return false;
		}

		$rs = new Replication_Remote_System($systemid);
		return $rs->create($auth_method);

	}// end createRemoteSystem()


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

		switch($cmd) {
			case 'init' :
				require_once dirname(__FILE__).'/replication_config.inc';
				$conf = new Replication_Config();
				if ($conf->save(Array())) {
					echo 'Configuration File "'.$conf->config_file.'" saved.'."\n";
					return 0;
				} else {
					return 1;
				}
				break;

			case 'create' :
				pre_echo($_SERVER['argv']);
				if (isset($_SERVER['argv'][2]) && isset($_SERVER['argv'][3])) {
					if ($this->createRemoteSystem($_SERVER['argv'][2], $_SERVER['argv'][3])) {
						echo 'Remote System "'.$_SERVER['argv'][2].'" created.'."\n";
						echo 'You can now edit "'.$conf->config_file.'" saved.'."\n";
						return 0;
					}
				}// end if
				$this->printUsage('create');
				return 1;

				break;


			case '-h'     :
			case '--help' :
			case 'help'   :
				$this->printUsage(isset($_SERVER['argv'][2]) ? $_SERVER['argv'][2] : '');
				break;

			default :
				$this->printUsage();
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
	function printUsage($cmd='')
	{
		if (!SQ_PHP_CLI) {
			trigger_error(__CLASS__.'::'.__FUNCTION__.'() can only be run from the command line', E_USER_ERROR);
		}

		$prog = basename($_SERVER['argv'][0]);

		$descs = Array(
			'help'		=> 'Use "'.$prog.' help <command>" for more info on each command',
			'init'		=> 'Create the initial replication config file',
			'create'	=> 'Create a remote system entry',
		);

		$lines = Array();

		if ($cmd != '' && isset($descs[$cmd])) {
			$lines[] = $descs[$cmd];
			switch($cmd) {
				case 'create' :
					$lines[] = $prog.' create <systemid> <auth method>';
					trigger_error('LIST VALID OF AUTH METHODS HERE');
					break;

				case 'help' :
					$other_descs = $descs;
					unset($other_descs['help']);
					$lines[] = $prog.' help ['.implode('|', array_keys($other_descs)).']';
					break;

				default :
					$lines[] = $prog.' '.$cmd;

			}// end switch
		} else {
			$lines[] = 'Usage : '.$prog.' ['.implode('|', array_keys($descs)).']';
			foreach($descs as $cmd => $desc) {
				$lines[] = '  '.str_pad($cmd, 10, ' ').'- '.$desc;
			}
		}// end if

		echo implode("\n", $lines)."\n";

	}// end printUsage()


	/**
	* Returns a list of Authentication Types installed on this system
	* 
	* @return Array('[code_name]' => '[Display Name]');
	* @static
	* @access public
	*/
	function getAuthTypes()
	{
		$dir = dirname(__FILE__).'/authentication/';
		return Replicator::getClassesFromDir($dir, '/^replication_system_authentication_(.*).inc$/');
	}// end getAuthTypes()


	/**
	* Returns a list of Transport Methods installed on this system
	* 
	* @return Array('[code_name]' => '[Display Name]');
	* @static
	* @access public
	*/
	function getTransportMethods()
	{
		$dir = dirname(__FILE__).'/transport/';
		return Replicator::getClassesFromDir($dir, '/^replication_system_transport_(.*).inc$/');

	}// end getTransportMethods()


	/**
	* Takes a directory, and searchs it for all files matching the passed preg
	* The preg MUST contain ONE sub pattern. This is what is returned.
	*
	* @param string	$dir		the directory to search
	* @param string	$preg_exp	the preg expression containing ONE subpattern
	* 
	* @return Array('[code_name]' => '[Display Name]');
	* @static
	* @access public
	*/
	function getClassesFromDir($dir, $preg_exp)
	{
		$classes = Array();

		$d = dir($dir);
		while (false !== ($entry = $d->read())) {
			if (is_file($dir.'/'.$entry) && preg_match($preg_exp, $entry, $matches)) {
				$classes[$matches[1]] = ucwords(str_replace('_', ' ', $matches[1]));
			}
		}
		$d->close();

		return $classes;

	}// end getClassesFromDir()


	/**
	* The Replicate fn, called from the Squiz Server Process Object
	*
	* @param string		$systemid		the systemid of the system that we are replicating with
	* @param resource	$client_fd		the client socket resource
	* 
	* @return void
	* @access public
	*/
	function replicate($systemid, $client_fd)
	{
		$this->_log("IN REPLICATE FN : $systemid", 'debug');
		if(SQ_REPLICATION_IS_MASTER) {
			$this->_log("I AM THE MASTER, CHECK '$systemid' IS KNOWN TO US", 'debug');

//			replicateMaster($systemid, $client_fd, true);

		} else {
			$this->_log("I AM A SLAVE, CHECK '$systemid' IS OUR MASTER", 'debug');
			// make sure we think we are talking
			if ($systemid != SQ_REPLICATION_MASTER_REPLICATION_SYSTEMID) {
				$this->_log('System called "'.$systemid.'" attempted to replicate with us, but it is not our master server', 'error');
				return;
			}

//			replicateSlave($client_fd, true);

		}

	}// end getClassesFromDir()


//	function replicateMaster($systemid, $client_fd, $received_connection);
//	function replicateSlave($client_fd, $received_connection);

}//end class

?>