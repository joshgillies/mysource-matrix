<?php

require_once dirname(__FILE__).'/hipo_server_common.inc';

/**
* HIPO_Server
*    Inspired by PEAR::Net_Server
* @version
* @author
*/
class HIPO_Server extends HIPO_Server_Common
{
	/**
	* the process id of the server
	* @var integer
	*/
	var $_server_pid = 0;

	/**
	* the file with the config settings for the server
	* @var integer
	*/
	var $_conf_file = '';

	/**
	* domain to connect to
	* @var string
	*/
	var $_domain = 'localhost';

	/**
	* port to connect to
	* @var integer
	*/
	var $_port = 10000;

	/**
	* maximum of backlog in queue
	* @var integer
	*/
	var $max_queue = 100;

	/**
	* the base file (socket) descriptor
	* @var resource
	*/
	var $_init_fd = null;

	/**
	* the signals that we need to capture
	* @var Array()
	*/
	var $_signals = Array(SIGTERM, SIGINT, SIGCHLD, SIGUSR1, SIGUSR2);

	/**
	* Array of childids to pids, used to send additional info to the processes while they are running
	* Also acts as a counter for the number of children we have
	* @var Array()
	*/
	var $_tasks = Array();

	/**
	* maximum amount of tasks
	* @var integer
	*/
	var $max_tasks = -1;

	/**
	* The info for the Systems that have registered with the server
	* Form :
	* Array(
	*		'[some system id]' => '[path to system root]',
	*		...
	* );
	* @var Array()
	*/
	var $_systems = Array();

	/**
	* Constructor
	*
	* @param	string	$domain			domain to connect to
	* @param	integer	$port			port to connect on
	* @param	string	$conf_file		the config file for the server
	*
	* @access	public
	*/
	function HIPO_Server($domain, $port, $conf_file)
	{
		$this->HIPO_Server_Common();
		$this->_domain		= $domain;
		$this->_port		= $port;
		$this->_conf_file	= $conf_file;
		$this->_server_pid	= posix_getpid();

		// this is only needed, when server is not run in CLI
		set_time_limit(0);

		$this->_loadConf();

	}// end constructor


	/**
	* (Re)load the config settings from the conf file
	*
	* @access	private
	*/
	function _loadConf()
	{
		$this->_systems = Array();
		if (file_exists($this->_conf_file)) {
			// include on purpose so that if it fails we don't bring the server down
			$included = include($this->_conf_file);
			if($included) {
				// now assign the systems array from the $systems from the include file
				foreach($systems as $systemid => $system_root) {
					if (!is_dir($system_root)) {
						trigger_error('Unable to register system "'.$systemid.'", the system root supplied is not valid', E_USER_NOTICE);
						continue;
					} elseif (!is_readable($system_root.'/core/include/init.inc')) {
						trigger_error('Unable to register system "'.$systemid.'", the init.inc is not readable in the supplied system root', E_USER_NOTICE);
						continue;
					}

					error_log('System "'.$systemid.'" loaded');
					$this->_systems[$systemid] = $system_root;

				}// end foreach

				error_log('Config File '.$this->_conf_file.' Loaded');
				return;

			}// end if
		}// end if

		error_log('No Config File Loaded');

	}// end _loadConf()


	/**
	* start the server
	*
	* @access	public
	*/
	function start()
	{
		if (isset($GLOBALS[__CLASS__.'_Status']) && $GLOBALS[__CLASS__.'_Status'] == 'running') {
			trigger_error('Server Already Started', E_USER_WARNING);
			return;
		}

		$this->_init_fd = @socket_create(AF_INET, SOCK_STREAM, 0);
		if (!$this->_init_fd) {
			trigger_error('Could not create socket : '.$this->getLastSocketError(), E_USER_WARNING);
			return;
		}

		// adress may be reused
		socket_set_option($this->_init_fd, SOL_SOCKET, SO_REUSEADDR, 1);

		// bind the socket
		if (!@socket_bind($this->_init_fd, $this->_domain, $this->_port)) {
			$error = $this->getLastSocketError($this->_init_fd);
			@socket_close($this->_init_fd);
			trigger_error('Could not bind socket to '.$this->_domain.' on port '.$this->_port.' ('.$error.').', E_USER_WARNING);
			return;
		}

		// listen on selected port
		if (!@socket_listen($this->_init_fd, $this->max_queue)) {
			$error = $this->getLastSocketError($this->_init_fd);
			@socket_close($this->_init_fd);
			trigger_error('Could not listen ('.$error.').', E_USER_WARNING);
			return;
		}

		error_log('Listening on port '.$this->_port.'. Server started at '.date('H:i:s', time()));
		error_log('PID : '.posix_getpid());

		// this allows the shutdown function to check whether the server is already shut down
		$GLOBALS[__CLASS__.'_Status'] = 'running';
		// this ensures that the server will be shutdown correctly
		#register_shutdown_function(array($this, 'shutDown'));

		// tick use required as of PHP 4.3.0 for signals
		declare (ticks = 1);
		foreach($this->_signals as $sig) {
			pcntl_signal($sig, array(&$this, 'sigHandler'));
		}

		while(true)
		{
			$read_fds   = array($this->_init_fd);

			// block and wait for data or new connection,
			// NOTE: $w and $e are passed by ref
			$ready = @socket_select($read_fds, $w = NULL, $e = NULL, NULL);

			if ($ready === false) {
				// if we were interupted by a signal, just keep going
				if (socket_last_error() == SOCKET_EINTR) {
					continue;
				} else {
					error_log('socket_select failed : '.$this->getLastSocketError());
					$this->shutDown();
					exit();
				}
			}

			// check for new connection, really should always be the case
			// this is as much for my realisation of what's happening with the $read_fds
			// as anything else
			if (!in_array($this->_init_fd, $read_fds)) continue;

			if (($client_fd = socket_accept($this->_init_fd)) === FALSE) {
				error_log('socket_accept failed : '.$this->getLastSocketError());
			}

			$this->_processChild($client_fd);

		}// end while

	}// end start()


	/**
	* shutdown server
	*
	* @access   public
	*/
	function shutDown()
	{
		// only shut down if we are the main server, not a child,
		// (needed because of the register_shutdown_function() call)
		if (posix_getpid() != $this->_server_pid) return;

		if (isset($GLOBALS[__CLASS__.'_Status']) && $GLOBALS[__CLASS__.'_Status'] == 'running') {
			$GLOBALS[__CLASS__.'_Status'] = 'shutdown';

			@socket_close($this->_init_fd);

			// tell all our kids to die
			foreach($this->_tasks as $child_pid) {
				posix_kill($child_pid, SIGTERM);
			}

			// reap children, waiting for them all to die
			while( ($child_pid = pcntl_waitpid(-1, $status, 0)) > 0) {
				$taskid = array_search($child_pid, $this->_tasks);
				if ($taskid === false) continue;
				unset($this->_tasks[$taskid]);
			}

			error_log('Server Shutdown : '.posix_getpid());

		}// end if

		exit();

	}// end shutDown()


	/**
	* The handler for the any signals from the system
	*
	* @return int	$signo	the signal number
	* @access public
	*/
	function sigHandler($signo)
	{
		switch($signo) {
			case SIGINT:
			case SIGTERM:
				error_log('Forcing Termination...');
				$this->shutDown();
				exit();
				break;
			case SIGUSR1:
				error_log('Caught SIGUSR1...Re-Reading in Config Files');
				$this->_loadConf();
				break;
			case SIGUSR2:
				error_log('Caught SIGUSR2 in '.posix_getpid().' ... Print_R of Object');
				error_log(print_r($this, true));
				break;
			case SIGCHLD:
				// reap any and all zombie children
				while( ($child_pid = pcntl_waitpid(-1, $status, WNOHANG)) > 0) {
					$taskid = array_search($child_pid, $this->_tasks);
					if ($taskid === false) continue;
					unset($this->_tasks[$taskid]);
				}
				break;
			default:
				trigger_error('Signal '.$signo.' not handled', E_USER_NOTICE);
				break;
		}// end switch

	}// end sigHandler()


	/**
	* does the actually do the processing for the request in the child (ie forked) process
	*
	* @param resource	$client_fd	the client socket resource
	*
	* @access private
	* @return void
	*/
	function _processChild($client_fd)
	{
		// check for maximum amount of connections
		if ($this->max_tasks > 0 && count($this->_tasks) >= $this->max_tasks) {
			error_log('Too many connections.');
			socket_close($client_fd);
			return;
		}// end if

		socket_set_option($client_fd, SOL_SOCKET, SO_REUSEADDR, 1);

		$data = $this->_readFromSocket($client_fd);
		echo "DATA : ".get_class($data)."\n";
		if (!is_null($data) && $data->root->name == 'hipo_comm' && isset($data->root->attributes['systemid']) && isset($data->root->children[0])) {
			if (isset($this->_systems[$data->root->attributes['systemid']])) {

				// make sure they aren't trying anything tricky
				$data->root->name = strtolower(preg_replace('/[^a-z_]/', '', $data->root->name));

				switch ($data->root->children[0]->name) {
					case 'hipo_job' :
						$this->_processHIPOJob($client_fd, $data->root->children[0], $data->root->attributes['systemid']);
						return;
						break;

					default :
						trigger_error('Unknown Task : '.$data->root->children[0]->name, E_USER_NOTICE);

				}// end switch

			} else {
				trigger_error('Unknown System : '.$data->root->attributes['systemid'], E_USER_NOTICE);

			}// end switch

		}// end if

		socket_close($client_fd);


	}// end _processChild()

	/**
	* Looks after processing
	*
	* @param resource				$client_fd		the client socket resource
	* @param object XML_Tree_Node	$client_data	the data sent by the client (excluding the system information)
	* @param string					$systemid		the systemid that we are currently working on
	*
	* @access private
	* @return void
	*/
	function _processHIPOJob($client_fd, $client_data, $systemid)
	{

		switch ($client_data->attributes['command']) {
			case 'start' :
				$taskid = $this->_fork($client_fd);
				// we are the server, get out of here
				if ($taskid == '') return;
				require_once $this->_systems[$systemid].'/core/include/init.inc';

				require_once 'XML/Tree.php';
				$output = new XML_Tree();
				$output_root = &$output->addRoot('output');

				$hh = &$GLOBALS['SQ_SYSTEM']->getHipoHerder();
				$process = $hh->prepareProcessServer($client_data, $output_root, $taskid);

echo "OUTPUT : \n";
$output->dump();

				// right now send back the output from the pre-processing, then close the connection
				// and we will start doing the work
				$this->_writeToSocket($client_fd, $output);
				socket_close($client_fd);

				if ($process) {
					#echo "DO PROCESS....\n";
					#sleep(10);
					echo "START PROCESS....\n";
					$hh->processServer();
					echo "FINISH PROCESS....\n";
				}
				// because we are forked, just exit out of here
				exit();

				break;

			// right we need to send an SIGINT to child process to get it to abort
			case 'abort' :

				require_once 'XML/Tree.php';
				$output = new XML_Tree();
				$output_root = &$output->addRoot('output');

				if (empty($client_data->attributes['taskid']) || empty($this->_tasks[$client_data->attributes['taskid']])) {
					$output_root->name    = 'error';
					$output_root->content = 'No TaskID Supplied or invalid TaskID';

				} else {
					$child_pid = $this->_tasks[$client_data->attributes['taskid']];
					if (posix_kill($child_pid, SIGTERM)) {
						$output_root->name    = 'ack';
					} else {
						$output_root->name    = 'error';
						$output_root->content = 'No TaskID Supplied or invalid TaskID';
					}
				}

echo "OUTPUT : \n";
$output->dump();

				// right now send back the output from the pre-processing, then close the connection
				// and we will start doing the work
				$this->_writeToSocket($client_fd, $output);
				socket_close($client_fd);

				break;

			default :
				error_log('Unknown Task : '.$client_data->name);
				socket_close($client_fd);

		}// end switch

	}// end _processHIPOJob()


	/**
	* Fork this server process
	*
	* @return string	returns a blank string for the parent (ie server) process and the taskid for the child
	*
	*/
	function _fork($client_fd)
	{
		do {
			$taskid = md5(uniqid(rand(), true));
		} while(isset($this->_children[$taskid]));

		$child_pid = pcntl_fork();
		switch($child_pid) {
			case -1 :
				error_log('Fork failed.');
				trigger_error("Could not fork!!\nDying...\n", E_USER_WARNING);
				$this->shutDown();
				exit();
				break;

			// we are the child
			case 0 :

				// reset the child's signals to default
				foreach($this->_signals as $sig) {
					pcntl_signal($sig, SIG_DFL);
				}

				socket_close($this->_init_fd);
				socket_set_option($client_fd, SOL_SOCKET, SO_REUSEADDR, 1);

				return $taskid;
				break;

			// we are the parent, close the child socket, the forked child has it's copy
			default :
				error_log('Forked PID : '.$child_pid);
				$this->_tasks[$taskid] = $child_pid;
				print_r($this->_tasks);
				socket_close($client_fd);
				return '';

		}// end switch

	}// end _fork()

}// end class



?>