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
* $Id: replicator_slave.inc,v 1.1.2.1 2004/02/14 12:39:14 brobertson Exp $
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
			trigger_error('Replicator Slave appears to have already been replicated, if you are sure that this not the case, then remove the initialise flag file', E_USER_WARNING);
			return false;
		}

		$ms = new Replication_Remote_Master_System(false);

		if (!$ms->setAuthType($auth_type) || !$ms->setTransportMethod($transport_method) ||!$ms->save()) {
			trigger_error('Unable to create Replication Remote System "'.$this->id.'" : unable to create Master Remote Replication System', E_USER_WARNING);
			return false;
		}
		
		return parent::initialise();

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
					echo 'Slave Replication System Initialised.'."\n";
					echo 'You may now edit this system configuration file for the Master System : '."\n";
					$ms = &$this->getMasterSystem();
					echo '  '.$ms->confFile()."\n";
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
			'help'		=> 'Use "'.$prog.' help <command>" for more info on each command',
			'init'		=> 'Intialise this system to be a Slave System in a replication cluster',
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
				$lines[] = '  '.str_pad($cmd, 10, ' ').'- '.$desc;
			}
		}// end if

		echo implode("\n", $lines)."\n";

	}// end runPrintUsage()

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
		if(SQ_REPLICATION_IS_MASTER) {
			$this->_log('This System is NOT designated as a Slave System, why are we using a '.__CLASS__.' object', 'error');
			trigger_error('This System is NOT designated as a Slave System, why are we using a '.__CLASS__.' object', E_USER_WARNING);
			return;
		}

		// now because of the nature of server rep and it never really being seen we are 
		// going to force the issue with logging errors and set the error log file
		$old_log_errors = ini_set('log_errors', 1);
		$old_error_log  = ini_set('error_log', $this->_log_dir.'/error.log');
				
		$this->_log("I AM A SLAVE, CHECK '$systemid' IS OUR MASTER", 'debug');
		// make sure we think we are talking to the right server
		if ($systemid != SQ_REPLICATION_MASTER_REPLICATION_SYSTEMID) {
			trigger_error('System called "'.$systemid.'" attempted to replicate with us, but it is not our master server', E_USER_WARNING);
		} else {
			$this->_log("CHECK '$systemid' IS OUR MASTER, DO THE STUFF", 'debug');
//			replicateSlave($client_fd, true);
#function replicateSlave($client_fd, $received_connection);
		}

		ini_set('log_errors', $old_log_errors);
		ini_set('error_log', $old_error_log);

	}// end replicate()

}//end class

?>