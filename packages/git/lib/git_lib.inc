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
*/


 require_once SQ_DATA_PATH.'/private/conf/tools.inc';

/**
* Git_Lib
*
* Purpose
*	List git functions used by Git Bridge
*
*
* @author  Chiranjivi Upreti <cupreti@squiz.com.au>
* @version $Revision: $
* @package MySource_Matrix
*/
class Git_Lib
{

	/**
	* Path to the git repository
	*
	* @var string
	*/
	private $_repo_path;


	/**
	* Remote git URL used for cloning the current repository
	*
	* @var string
	*/
	private $_clone_url;


	/**
	* Repository current branch
	*
	* @var string
	*/
	private $_branch;


	/**
	* List of repository branches
	*
	* @var array
	*/
	private $_branch_list;


	/**
	* Raw clone URL supplied
	*
	* @var string
	*/
	private $_raw_clone_url;


	/**
	* Constructor
	*
	* @param string $repo_path	Set the repository path to
	*
	*/
	function __construct($repo_path)
	{
		$this->_repo_path = trim($repo_path);
		$this->_initRepoParameters();

	}//end constructor


	/**
	* Reset the repository parameters
	*
	* @return void
	* @access private
	*/
	function _initRepoParameters()
	{
		$this->_clone_url = '';

		// Set the current branch
		$this->_branch = '';
		$this->_branch_list = Array();


	}//end _initRepoParameters()


	/**
	* Set raw clone URL
	*
	* @param string $raw_clone_url
	*
	* @return void
	* @access public
	*/
	public function setRawCloneUrl($raw_clone_url)
	{
		$this->_raw_clone_url = $raw_clone_url;

	}//end setRawCloneUrl()


	/**
	* Returns TRUE if repo exist in the defined repo path
	*
	* @return boolean
	* @access public
	*/
	public function repoAvailable()
	{
		return $this->_repo_path && is_dir($this->_repo_path.'/.git');

	}//end repoAvailable()


	/**
	* Returns TRUE if the repo is already checked out
	*
	* @return boolean
	* @access public
	*/
	public function repoCheckedOut($clone_url)
	{
		if (!$clone_url) {
			return FALSE;
		}

		if (!is_dir($this->_repo_path.'/.git')) {
			return FALSE;
		}

		$current_repo_url = $this->getCloneUrl();

		if ($clone_url != $current_repo_url) {
			return FALSE;
		}

		return TRUE;

	}//end repoCheckedOut()


	/**
	* Do the pull request on the current repo
	*
	* @return boolean
	* @access public
	*/
	public function pullRequest()
	{
		return $this->_execGit('pull', TRUE, TRUE, TRUE);

	}//end pullRequest()


	/**
	* Returns TRUE if the remote git URL is accesssible
	*
	* @param string $git_url
	*
	* @return boolean
	* @access public
	*/
	function gitUrlAccessible($git_url)
	{
		if (!empty($git_url)) {
			$ls_remote = $this->_execGit('ls-remote '.escapeshellarg($git_url), TRUE, FALSE, TRUE);
			return $ls_remote && strpos($ls_remote, 'master') !== FALSE;
		}

		return FALSE;

	}//end gitUrlAccessible()



	/**
	* Initiate a git repo and fetch the remote git repo
	*
	* @param string 		$clone_url
	*
	* @return boolean
	* @access public
	*/
	public function cloneRepo($clone_url)
	{
		if (!$clone_url || !$this->_repo_path || $this->repoLocked()) {
			return FALSE;
		}

		// Verify the remote git url is valid and accessible
		if (!$this->gitUrlAccessible($clone_url)) {
			$this->_logError(translate('The specified remote Git URL does not points to the valid Git repository.'), FALSE);
			return FALSE;
		}

		$this->_raw_clone_url = $clone_url;

		$repo_path = $this->_repo_path;

		// Delete the existing repo if any
		if (is_dir($repo_path.'/.git')) {
			require_once SQ_FUDGE_PATH.'/general/file_system.inc';
			delete_directory($repo_path);
		}

		// Initiate the repo, set the remote git URL and checkout repo
		// Note that "Sparse Checkout" option is enabled to allow subdirs checkout
		$result = 	$this->_execGit('init '.escapeshellarg($repo_path), TRUE, FALSE, TRUE) !== FALSE &&
					$this->_execGit('remote add -f origin '.escapeshellarg($clone_url), TRUE, TRUE, TRUE) !== FALSE &&
					$this->_execGit('config core.sparseCheckout true', TRUE, TRUE, TRUE) !== FALSE &&
					$this->_execGit('checkout master', TRUE, TRUE, TRUE) !== FALSE;

		if ($result) {
			$this->_initRepoParameters();
			$this->_clone_url = $clone_url;
		}

		return $result !== FALSE;

	}//end cloneRepo()


	/**
	* Filter out the selected git dirs in the repo
	* Uses Git "sparse checkout" option to filter the sub dirs
	*
	* @param string/array 	$sub_dirs	If string, comma seperated sub directories to checkout
	*									Otherwise array of sub directories to checkout
	*
	* @return boolean
	* @access public
	*/
	function filterSubDirectories($sub_dirs)
	{
		if (!$this->repoAvailable()) {
			return FALSE;
		}

		// Validate the sub dir parameters
		if (!is_array($sub_dirs)) {
			$sub_dirs = explode(',', trim($sub_dirs));
		}
		foreach($sub_dirs as $key => $sub_dir) {
			if (trim($sub_dir)) {
				$sub_dirs[$key] = trim($sub_dir, '/ ').'/';
			} else {
				unset($sub_dirs[$key]);
			}
		}

		if (empty($sub_dirs)) {
			// If no sub dirs have been set, select all
			$sub_dirs = Array('*');
		}

		require_once SQ_FUDGE_PATH.'/general/file_system.inc';
		if (!create_directory($this->_repo_path.'/.git/info')) {
			return FALSE;
		}

		return file_put_contents($this->_repo_path.'/.git/info/sparse-checkout', implode("\n", $sub_dirs)) !== FALSE && $this->_execGit('reset --hard');

	}//end filterSubDirectories()


	/**
	* Returns the timestamp of the last pull date
	*
	* @return boolean|string
	* @access public
	*/
	public function latestPullDate()
	{
		$ts = FALSE;
		if (is_file($this->_repo_path.'/.git/FETCH_HEAD')) {
			$ts = $this->_execGit('stat -c %Y .git/FETCH_HEAD', FALSE);
		}

		return $ts;

	}//end latestPullDate()


	/**
	* Set the repository path
	*
	* @param string $repo_path	Repo path root
	*
	* @return void
	* @access public
	*/
	public function setRepoPath($repo_path)
	{
		$this->_repo_path = trim($repo_path);

	}//end setRepoPath()


	/**
	* Set the clone ssh URL
	*
	* @param string $clone_url
	*
	* @return void
	* @access public
	*/
	public function setCloneUrl($clone_url)
	{
		$this->_clone_url = trim($clone_url);

	}//end setCloneUrl()


	/**
	* Set the clone ssh URL
	*
	* @param string $clone_url
	*
	* @return void
	* @access public
	*/
	public function getCloneUrl()
	{
		if (!$this->_clone_url) {
			$this->_clone_url = $this->_getRepoCloneUrl();
		}

		return $this->_clone_url;

	}//end getCloneUrl()


	/**
	* Set the repo branch
	*
	* @param string $branch
	*
	* @return boolean
	* @access public
	*/
	public function switchBranch($branch)
	{
		$success = TRUE;
		$branch = trim($branch);
		if ($branch != $this->getBranchName()) {
			$success = $this->_execGit('checkout '.escapeshellarg($branch));
			if ($success !== FALSE) {
				$this->_branch = $branch;
			}
		}

		return $success;

	}//end switchBranch()


	/**
	* Get repo's currnet branch name
	*
	* @return string
	* @access public
	*/
	public function getBranchName()
	{
		if (!$this->_branch) {
			$this->_populateBranchList();
		}

		return $this->_branch;

	}//end getBranchName()


	/**
	* Get the repo branch list
	*
	* @param boolean $remote
	*
	* @return array
	* @access public
	*/
	public function getBranchList($remote=TRUE)
	{
		if (!$this->_branch_list) {
			$this->_populateBranchList();
		}

		return $this->_branch_list;

	}//end getBranchList()


	/**
	* Returns the error recorded if any for last git command execution
	*
	* @return boolean|string
	* @access public
	*/
	function getLastError()
	{
		// Get git error in the last git request if any
		$err_file = $this->_getErrorFilePath();

		return is_file($err_file) ? file_get_contents($err_file) : FALSE;

	}//end getLastError()


	/**
	* Returns FALSE if the repository is currently NOT being updated
	*
	* @return boolean|string
	* @access public
	*/
	function repoLocked()
	{
		// If lock file exists, repo is getting updated
		 $lock_file = $this->_getLockFilePath();
		if (is_file($lock_file)) {
			return date('Y-m-d H:i:s', filemtime($lock_file));
		}

		return FALSE;

	}//end repoLocked()


	/**
	* Removes the repo lock
	*
	* @return boolean|string
	* @access public
	*/
	function removeRepoLock()
	{
		$lock_file = $this->_getLockFilePath();

		return is_file($lock_file) ? unlink($lock_file) : TRUE;

	}//end removeRepoLock()


	/**
	* Returns the file path of the lock file used for running Git operations
	*
	* @return string
	* @access public
	*/
	private function _getLockFilePath()
	{
		return SQ_TEMP_PATH.'/git_lock_'.md5($this->_repo_path);

	}//end _getLockFilePath()


	/**
	* Returns the file path of the error file that keeps track of error(s) that occured in last command
	*
	* @return string
	* @access private
	*/
	private function _getErrorFilePath()
	{
		return SQ_TEMP_PATH.'/git_error_'.md5($this->_repo_path);

	}//end _getErrorFilePath()


	/**
	* Returns the current repo remote URL
	*
	* @return boolean
	* @access ptivate
	*/
	private function _getRepoCloneUrl()
	{
		return $this->_execGit('config --get remote.origin.url');

	}//end _getRepoCloneUrl()


	/**
	* Populate branch info in the _branch and _branch_list vars
	*
	* @param boolean $remote
	*
	* @return void
	* @access public
	*/
	private function _populateBranchList($remote=TRUE)
	{
		$output = $this->_execGit('branch'.($remote ? ' -a' : ''));
		if ($output) {
			$list = Array();
			$raw_list = explode("\n", $output);
			foreach($raw_list as $branch) {
				$branch = trim($branch);
				if (strpos($branch, 'remotes/origin/') === 0) {
					$branch = substr($branch, 15);
				}

				if (isset($list[$branch]) || strpos($branch, 'HEAD ->') === 0) {
					continue;
				}

				if (strpos($branch, '* ') === 0) {
					$branch = substr($branch, 2);
					$this->_branch = $branch;
				}

				$list[$branch] = $branch;
				ksort($list);
			}

			$this->_branch_list = $list;
		}

	}//end _populateBranchList()


	/**
	* Execute git command
	*
	* @param $string $cmd         Command to execute
	* @param boolean $git_cmd     Whether its git command
	* @param boolean $change_dir  Whether to change to repository dir when executing the command
	* @param boolean $create_lock If TRUE the repo is locked for access till the command finishes
	*
	* @return array|boolean
	* @access private
	*/
	private function _execGit($cmd, $git_cmd=TRUE, $change_dir=TRUE, $create_lock=FALSE)
	{
		if (!is_executable(SQ_TOOL_GIT_PATH)) {
			trigger_error(translate('Path set to the Git binary is not valid.'), E_USER_WARNING);
			return FALSE;
		}

		$lock_file = $this->_getLockFilePath();
		if ($git_cmd && is_file($lock_file)) {
			trigger_error(translate('Cannot access the repository as its being updated now. Please try again later.'), E_USER_WARNING);
			return FALSE;
		}

		// Any error is sent to stderr when running this command is stored in this file
		$err_file = $this->_getErrorFilePath();

		// Change the working directory to the repo's root
		if ($change_dir) {
			$working_dir = getcwd();
			if (!is_dir($this->_repo_path)) {
				return FALSE;
			}
			chdir($this->_repo_path);
		}

		// Remove error file from previous process if any
		if (is_file($err_file)) {
			unlink($err_file);
		}

		if ($git_cmd) {
			$cmd = SQ_TOOL_GIT_PATH.' '.$cmd;

			// If its SSH clone URL, use the set RSA key for this Matrix instance
			if (strpos($this->_raw_clone_url, 'git@') !== FALSE) {
				require_once SQ_INCLUDE_PATH.'/general_occasional.inc';
				$cmd = includeMatrixSshKey($cmd);
			}
		}

		// Pipes are defined in the way the child will use them. So Java/Tika will
		// read from stdin (0) and write to stdout (1) and stderr (2).
		$spec    = Array(
					0 => Array('pipe', 'r'),
					1 =>  Array('pipe', 'w'),
					2 =>  Array('pipe', 'w'),
				);

		$cwd = NULL;
		$env = Array();
		$process = proc_open($cmd, $spec, $pipes, $cwd, $env);

		$retval = 1;
		if (is_resource($process) === TRUE) {
			if ($create_lock) {
				touch($lock_file);
			}
			fclose($pipes[0]);

			// Get the content from stdout after getting the content from stderr
			$content = '';
			do {
				$data = fread($pipes[1], 8092);
				$content .= $data;
			} while (strlen($data) > 0);

			fclose($pipes[1]);

			$errors = stream_get_contents($pipes[2]);
			fclose($pipes[2]);

			$retval = proc_close($process);

			if ($retval!== 0 && !empty($errors)) {
				$cmd_error = trim(implode(', ', array_unique(explode(PHP_EOL, $errors))), ', ');
				$error_msg = 'Error occurred when running git command "'.$cmd.'": '.$cmd_error;
				$error_msg = preg_replace('/(https?:\/\/[^:@]+:)[^@]+/i', '$1*****', $error_msg);
				trigger_error($error_msg, E_USER_WARNING);
				$this->_logError($error_msg);
			}//end if

			if ($create_lock && is_file($lock_file)) {
				unlink($lock_file);
			}
		}

		// Restore current working directory
		if ($change_dir) {
			chdir($working_dir);
		}

		return $retval === 0 ? trim($content) : FALSE;

	}//end _gitCommand()


	/**
	* Log the error into the repo's 'log error file'
	* This will log error recorded if any for last git command execution only
	*
	* @param string $error
	* @param string $overwrite
	*
	* @return boolean
	* @access private
	*/
	private function _logError($error, $overwrite=TRUE)
	{
		$err_file = $this->_getErrorFilePath();
		if ($overwrite || !is_file($err_file)) {
			return file_put_contents($this->_getErrorFilePath(), $error) !== FALSE;
		}

		return TRUE;

	}//end _logError()

}//end class
