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
* $Id: replicator.inc,v 1.1.2.3 2004/02/13 17:18:58 brobertson Exp $
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

}//end class

?>