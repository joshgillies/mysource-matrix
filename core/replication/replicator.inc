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
* $Id: replicator.inc,v 1.1.2.6 2004/02/21 14:44:47 brobertson Exp $
* $Name: not supported by cvs2svn $
*/

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
* @abstract
*/
class Replicator extends MySource_Object
{

	/**
	* The path to the directory that we will put all data
	* @var string
	*/
	var $_data_dir = '';

	/**
	* The path to the directory that we will put all our logs (because we can have diff types)
	* @var string
	*/
	var $_log_dir = '';

	/**
	* Constructor
	*
	*/
	function Replicator()
	{
		$this->MySource_Object();
		$this->_data_dir = SQ_DATA_PATH.'/private/replication';
		$this->_log_dir  = SQ_DATA_PATH.'/private/logs/replication';

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
	* Process the commands to the replicator from the command line
	*
	* @return int	the exit status of the command (ie the one that should be passed to exit())
	* @access public
	*/
	function run()
	{
		return 1; // remember 1 is error from cmd line
	}// end run()


	/**
	* Simple fn to print a prompt and return what the user enters
	*
	* @param string	$prompt	prompt to display before cursor
	*
	* @return string
	* @access private
	*/
	function _getCmdLineInput($prompt='')
	{
		if (!SQ_PHP_CLI) {
			trigger_error(__CLASS__.'::'.__FUNCTION__.'() can only be run from the command line', E_USER_ERROR);
		}
		echo $prompt;
		// now get their entry and remove the trailing new line
		return rtrim(fgets(STDIN, 4094));

	}// end _getCmdLineInput() 


	/**
	* Attempt to initialise this replicator type
	*
	* @return boolean
	* @access public
	* @abstract
	*/
	function initialise()
	{
		require_once SQ_FUDGE_PATH.'/general/file_system.inc';
		// make sure the directory exists
		if (!create_directory($this->_data_dir)) return false;
		return touch($this->_data_dir.'/.'.get_class($this).'_initialised');
	}// end initialise()


	/**
	* Whether we think that this replicator has been initialised on this system
	*
	* @return boolean
	* @access public
	*/
	function initialised()
	{
		// no need for caching, see clearstatcache()
		return file_exists($this->_data_dir.'/.'.get_class($this).'_initialised');
	}// end initialised()


	/**
	* Returns a list of Authentication Types installed on this system
	* 
	* @return Array('[code_name]' => '[Display Name]');
	* @static
	* @access public
	*/
	function getAuthTypes()
	{
		static $auth_types = Array();
		if (empty($auth_types)) {
			$dir = SQ_SYSTEM_ROOT.'/core/replication/authentication/';
			$auth_types = Replicator::getClassesFromDir($dir, '/^replication_system_authentication_(.*).inc$/');
		}
		return $auth_types;

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
		static $trans_meths = Array();
		if (empty($trans_meths)) {
			$dir = SQ_SYSTEM_ROOT.'/core/replication/transport/';
			$trans_meths = Replicator::getClassesFromDir($dir, '/^replication_system_transport_(.*).inc$/');
		}
		return $trans_meths;

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
	* Check's the passed system name to see whether it's valid or not
	* Returns TRUE if so and the error msg if not
	*
	* @param string	$system_name	the system name to check
	*
	* @return mixed	true if valid, string err msg if not
	* @access public
	* @static
	*/
	function validSystemName($system_name)
	{
		if (!preg_match('/^[a-z][a-z0-9.-]*$/', $system_name)) {
			return 'Must be all lower-case, with an a-z as the first character, followed by zero or more a-z, 0-9, . or - characters';
		}
		return true;

	}// end validSystemName()


	/**
	* Paints the interface for our config interface
	*
	* @param object Backend_Outputter	&$o				reference to the backend outputter
	* @param boolean					$write_access	whether the replication config is locked or not
	*
	* @return void
	* @access public
	* @see processBackend()
	*/
	function paintBackend(&$o, $write_access=false)
	{
	}// end paintBackend()


	/**
	* Saves the config settings that were submitted
	*
	* @param object Backend_Outputter	&$o			reference to the backend outputter
	*
	* @return boolean	false on error, true on everything else (including nothing happening)
	* @access public
	* @see paintBackend()
	*/
	function processBackend(&$o)
	{
		return true;
	}// end processBackend()


	/**
	* The Replicate fn, called from the Squiz Server Process Object
	*
	* @param string		$systemid		the systemid of the system that we are replicating with
	* @param resource	$client_fd		the client socket resource
	* 
	* @return void
	* @access public
	* @abstract
	*/
	function replicate($systemid, $client_fd)
	{
	}// end replicate()


	/**
	* Sets up the error handler, which is going to be logging to our log, then sending error to current
	* error handler
	*
	* @returns void
	* @access private
	*/
	function _setErrorHandler() 
	{
		$this->_tmp['old_error_handler'] = set_error_handler(Array(&$this, '_errorHandler'));
		// now because of the nature of server rep and it never really being seen we are
		// going to force the issue with logging errors and set the error log file
		$this->_tmp['old_log_errors'] = ini_set('log_errors', 1);

	}// end _setErrorHandler()


	/**
	* Restores the error handler, to what it was previously
	*
	* @returns void
	* @access private
	*/
	function _restoreErrorHandler() 
	{
		unset($this->_tmp['old_error_handler']);
		restore_error_handler();
		ini_set('log_errors', $this->_tmp['old_log_errors']);
	}// end _restoreErrorHandler()


	/**
	* This function captures all errors and warnings that occur during the execution of replicate()
	*
	* @param int	$err_no		The type of error (E_*)
	* @param string	$err_msg	The error message
	* @param string	$err_file	The file the error occured in
	* @param string	$err_line	The line the error occured on
	*
	* @returns void
	* @access private
	* @see replicate()
	*/
	function _errorHandler($err_no, $err_msg, $err_file, $err_line)
	{
		$terminate = ((E_USER_ERROR | E_ERROR) & $err_no);

		// if the function didn't have an '@' prepended OR if we are about to terminate
		// catch the error
		if (error_reporting() || $terminate) {

			// Strip out the file path begining
			$err_file = str_replace(Array("\r", SQ_SYSTEM_ROOT), '', $err_file);
			$err_msg  = str_replace(Array("\r", SQ_SYSTEM_ROOT), '', $err_msg);

			$text_msg = strip_tags(preg_replace(Array('/<br\\/?>/i', '/<p[^>]*>/i'), Array("\n", "\n\n"), $err_msg));

			$err_name = '';
			switch($err_no) {
				case E_USER_ERROR	: $err_name = SQ_SYSTEM_SHORT_NAME.' Error';	break;
				case E_USER_WARNING	: $err_name = SQ_SYSTEM_SHORT_NAME.' Warning';	break;
				case E_USER_NOTICE	: $err_name = SQ_SYSTEM_SHORT_NAME.' Notice';	break;
				case E_ERROR		: $err_name = 'PHP Error';						break;
				case E_WARNING		: $err_name = 'PHP Warning';					break;
				case E_NOTICE		: $err_name = 'PHP Notice';						break;
				default				: $err_name = 'Unknown Error Type - '.$err_no;
			}

			$log_msg = '['.strtolower($err_name).'] ('.$err_file.':'.$err_line.') - '.$text_msg;
			$this->_log($log_msg, 'error');

		}// end if error_reporting

		if (!empty($this->_tmp['old_error_handler'])) {
			// if it's an array, it's an object and method combination
			if (is_array($this->_tmp['old_error_handler'])) {
				$fn = $this->_tmp['old_error_handler'][1];
				$this->_tmp['old_error_handler'][0]->$fn($err_no, $err_msg, $err_file, $err_line);

			// else just a plain error handler
			} else {
				$this->_tmp['old_error_handler']($err_no, $err_msg, $err_file, $err_line);
			}
		}

		// we may never get here if there is an old error handler, because it should do any termination for us
		if ($terminate) {
			exit(1);
		}


	}// end _errorHandler()

}//end class

?>