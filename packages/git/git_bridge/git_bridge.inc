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

require_once SQ_PACKAGES_PATH.'/filesystem/file_bridge/file_bridge.inc';

/**
* Git_Bridge
*
* Purpose
*	Lists the files in the data public dir (Git repository) as a shadow assets which can we accessed via public URL
*	Also provides basic Git functionality to clone repo, pull files, etc.
*
* @author  Chiranjivi Upreti <cupreti@squiz.net>
* @version $Revision: $
* @package MySource_Matrix_Packages
* @subpackage git
*/
class Git_Bridge extends File_Bridge
{

	/**
	* Git_Lib object to perform Git operations
	*
	* @var object
	*/
	private $_git;


	/**
	* Constructor
	*
	* @param int	$assetid	the asset id to be loaded
	*
	*/
	function __construct($assetid=0)
	{
		parent::__construct($assetid);

		// Set the File bridge's location to the asset's public directory
		// @see File_Bridge::setLocationPath()
		$this->setLocationPath($this->data_path_public);

	}//end constructor


	/**
	* Call the given function from the Git_Lib lib class
	*
	* @param $function	 Name of the git lib function
	* @param $arguments  List of arguments to pass
	*
	* @return mixed
	* @access private
	*/
	function _gitLibRequest($function, $arguments=Array())
	{
		if (!isset($this->_git)) {
			require_once SQ_PACKAGES_PATH.'/git/lib/git_lib.inc';
			$this->_git = new Git_Lib($this->data_path_public);
			$this->_git->setRawCloneUrl($this->fullCloneUrl());
		}

		return call_user_func_array(Array($this->_git, $function), $arguments);	

	}//end _gitLibRequest()


	/**
	* Returns the URL for an asset, closest to the $base_url
	*
	* It returns string if $assetids is an int and an array(int => string) if $assetids is an array
	*
	* @param int|array  $assetids           the id of the asset you want the url for or an array
	*                                       for a whole bunch of them
	* @param string     $base_url           the url that we are want to match closest to
	* @param boolean    $ignore_rollback    ignore rollback checks
	* @param int        $base_contextid     the id of the context that you want the url from
	*
	* @return mixed int|array
	* @access public
	*/
	function getAssetURL($assetids, $base_url=NULL, $ignore_rollback=FALSE, $base_contextid = NULL)
	{
		$id_parts = explode(':', $assetids);
		if (empty($id_parts[1])) {
			return '';
		}

		$file_path = $id_parts[1];
		if (!is_file($this->data_path_public.'/'.$id_parts[1])) {
			return '';
		}

		$relative_path = substr($this->data_path_public, strlen(SQ_DATA_PATH.'/public/')).'/'.$file_path;

		return sq_web_path('data').'/'.$relative_path;

	}//end getAssetURL()


	/**
	* Queue the setting of the value of an attribute
	*
	* This function sets the value of the attribute locally, but does not database updating.
	* The change is queued awaiting the call of saveAttributes().
	*
	* If $filter is omitted or NULL, the default front-end filter setting is used.
	*
	* @param string  $name   the name of the attribute
	* @param string  $value  the new value of the attribute
	* @param boolean $filter whether to filter the value content.
	*
	* @return boolean
	* @access public
	* @see saveAttributes()
	*/
	public function setAttrValue($name, $value, $filter=NULL)
	{
		$success = parent::setAttrValue($name, $value, $filter);

		if ($success && $name == 'sub_dirs') {
			// Sub dirs list have been changed, update the repository to reflect the changes
			$this->_gitLibRequest('filterSubDirectories', Array($value));
		}

		return $success;

	}//end setAttrValue()


	/**
	* Whether this asset allows "notice" linking to its shadow children
	*
	* @return boolean
	* @access public
	*/
	function allowShadowNoticeLink()
	{
		return TRUE;

	}//end allowShadowNoticeLink()


	/**
	* Prints the frontend of this asset
	* Returns JSON output for Git repo specific requests (SQ_GIT_BRIDGE_ACTION)
	*
	* @return void
	* @access public
	*/
	function printFrontend()
	{
		// Handle these git bridge actions here
		$action_type = isset($_REQUEST['SQ_GIT_BRIDGE_ACTION']) ? $_REQUEST['SQ_GIT_BRIDGE_ACTION'] : FALSE;
		$valid_action_types = Array(
								'pull' 			=> SQ_PERMISSION_WRITE,
								'clone' 		=> SQ_PERMISSION_WRITE,
								'lock_status' 	=> SQ_PERMISSION_READ,
							);

		if (isset($valid_action_types[$action_type])) {

			$git_service_request = FALSE;
			$access_required = $valid_action_types[$action_type];
			// If we don't already have read access, check the request is authorised
			if (($access_required == SQ_PERMISSION_READ && !$this->readAccess()) || ($access_required != SQ_PERMISSION_READ && !$this->checkAccess($access_required, ''))) {

				$secret_key = $this->attr('secret_key');
				// If its Git hosting service, validate the secret key signature
				if (!empty($secret_key) && isset($_SERVER['REQUEST_METHOD']) && $_SERVER['REQUEST_METHOD'] == 'POST') {
					// We don't actually use payload data, but we need it to verify the request
					$content_type = isset($_SERVER[ 'CONTENT_TYPE']) ? $_SERVER[ 'CONTENT_TYPE'] : '';
					$raw_post = file_get_contents( 'php://input');
					$payload = '';
					if ($content_type == 'application/x-www-form-urlencoded') {
						$payload = filter_input(INPUT_POST, 'payload');
					} else if ($content_type == 'application/json') {
						$payload = $raw_post;
					}

					$signature = isset($_SERVER[ 'HTTP_X_HUB_SIGNATURE']) ? $_SERVER[ 'HTTP_X_HUB_SIGNATURE'] : '';
					if ($payload && $signature) {
						// Generate the signature and try to match it
						$request_signature = 'sha1='.hash_hmac('sha1', $raw_post, $secret_key, FALSE);
						$git_service_request = $request_signature == $signature;
					}
				}

				// If signature verification fails or is not applicable, try authenticating user via HTTP
				if (!$git_service_request) {
					$this->_authenticateHttpUser();
					// Can't go forward if we still dont have appropriate access
					if (($access_required == SQ_PERMISSION_READ && !$this->readAccess()) || ($access_required != SQ_PERMISSION_READ && !$this->checkAccess($access_required, ''))) {
						header('WWW-Authenticate: Basic realm="Squiz Matrix"');
						header('HTTP/1.0 401 Unauthorised');
						exit;
					}
				}
			}

			$response = Array('action_type' => $action_type);

			// Capture the errors when processing the pull/clone requests
			$process_errors = Array();
			$current_handler = set_error_handler(function($errno, $errstr, $errfile, $errline) use (&$process_errors) {
				$process_errors[] = strip_tags($errstr);
			});

			$response['errors'] = Array();
			switch($action_type) {
				case 'pull':
					// Do the pull request on the current repo
					$response['success'] = FALSE;
					if ($this->updateRepo()) {
						$response['success'] = TRUE;
						$response['update_date'] = $this->latestRepoUpdateDate();
					}
					break;

				case 'clone':
					// Clone the given git URL to the asset's public data directory
					$response['success'] = FALSE;
					if ($this->cloneRepo()) {
						$response['success'] = TRUE;
					}
					break;

				case 'lock_status':
					// Check the repository current lock status (Repo is locked when being updated)
					$repo_locked = $this->repoLocked();
					$last_error = FALSE;
					$response['success'] = $response['locked'] = $repo_locked ? TRUE : FALSE;
					$response['time'] = $repo_locked;
					break;
			}

			if (!$response['success']) {
				$last_error = $this->getLastGitError();
				if ($last_error) {
					$response['errors'] = Array($last_error);
				}
			}
			$response['errors'] = array_unique(array_merge($response['errors'], $process_errors));

			// Kind of redundent here but still do it anyway
			set_error_handler($current_handler);

			if (!headers_sent()) {
				header('Content-Type: application/json');
			}

			echo json_encode($response);
			exit;

		}

		// If not "Git action" request, let the parent handle it
		parent::printFrontend();

	}//end printFrontend()


	/**
	* Authenticate the user via HTTP
	*
	* @return boolean
	* @access private
	*/
	private function _authenticateHttpUser()
	{
		$logged_in = FALSE;
		if (!isset($_SERVER['PHP_AUTH_USER']) || !isset($_SERVER['PHP_AUTH_PW'])) {
			return $logged_in;
		}//end if

		if (!empty($_SERVER['PHP_AUTH_USER'])) {
			$auth_folder = $GLOBALS['SQ_SYSTEM']->am->getSystemAsset('authentication_folder');
			$auth_systems = $auth_folder->getAuthSystems();

			$user = NULL;
			foreach ($auth_systems as $systemid) {
				$system = $GLOBALS['SQ_SYSTEM']->am->getAsset($systemid);
				if (!is_null($system)) {
					$user = $system->authenticateUser($_SERVER['PHP_AUTH_USER'], $_SERVER['PHP_AUTH_PW']);
					if (!is_null($user)) {
						$GLOBALS['SQ_SYSTEM']->loginUser($user);
						$logged_in = TRUE;
						break;
					}//end if
				}//end if
			}//end foreach

		}//end if

		unset($_SERVER['PHP_AUTH_USER']);
		unset($_SERVER['PHP_AUTH_PW']);

		return $logged_in;

	}//end _authenticateHttpUser()


	// Git specific operations //////////

	/**
	* Returns TRUE if the repository is availabe
	*
	* @return boolean
	* @access public
	*/
	public function repoAvailable()
	{
		return $this->_gitLibRequest('repoAvailable');

	}//end repoAvailable()


	/**
	* Returns TRUE if the remote Git URL is accessible
	*
	* @param string $git_url
	*
	* @return boolean
	* @access public
	*/
	public function gitUrlAccessible($git_url)
	{
		return $this->_gitLibRequest('gitUrlAccessible', Array($git_url));

	}//end repoAvailable()


	/**
	* Returns the error recorded if any for last git command execution
	*
	* @return boolean|string
	* @access public
	*/
	function getLastGitError()
	{
		return $this->_gitLibRequest('getLastError');

	}//end getLastGitError()


	/**
	* Returns FALSE if the repository is currently NOT being updated
	* Otherwise returns the date since its been locked
	*
	* @return boolean|string
	* @access public
	*/
	function repoLocked()
	{
		return $this->_gitLibRequest('repoLocked');

	}//end repoLocked()


	/**
	* Removes the repo lock
	*
	* @return boolean
	* @access public
	*/
	function removeRepoLock()
	{
		return $this->_gitLibRequest('removeRepoLock');

	}//end removeRepoLock()


	/**
	* Does the pull request on the current repo
	*
	* @return boolean
	* @access public
	*/
	public function updateRepo()
	{
		if (!$this->repoAvailable()) {
			return FALSE;
		}

		if (!$this->_gitLibRequest('pullRequest')) {
			return FALSE;
		}

		return TRUE;

	}//end updateRepo()


	/**
	* Clone the remote Git repository
	*
	* @return boolean
	* @access public
	*/
	public function cloneRepo()
	{
		// Check if the repo is checked out already
		$full_clone_url = $this->fullCloneUrl();

		$checked_out = $this->_gitLibRequest('repoCheckedOut', Array($full_clone_url));
		if ($checked_out) {
			return TRUE;
		}

		$result = $this->_gitLibRequest('cloneRepo', Array($full_clone_url));
		if ($result) {
			// Filter the selected sub dirs and do the pull
			$result = $this->_gitLibRequest('filterSubDirectories', Array($this->attr('sub_dirs'))) && $this->_gitLibRequest('pullRequest');
		}

		return $result;

	}//end cloneRepo()


	/**
	* Returns the latest repo updated date
	*
	* @return string
	* @access public
	*/
	public function latestRepoUpdateDate()
	{
		$ts = $this->_gitLibRequest('latestPullDate');
		if (!empty($ts)) {
			return date('Y-m-d H:i:s', $ts);
		}

		return translate('Not available');

	}//end latestRepoUpdateDate()


	/**
	* Get the repository's branches list
	*
	* @return boolean
	* @access public
	*/
	public function getRepoBranchNames()
	{
		$current_branch = $this->_gitLibRequest('getBranchName');
		$branch_list = $this->_gitLibRequest('getBranchList');

		$branch_names = Array();
		foreach($branch_list as $branch_name) {
			$branch_names[$branch_name] = $branch_name == $current_branch;
		}

		return $branch_names;

	}//end getRepoBranchNames()


	/**
	* Checkout to the given branch
	*
	* @param string $branch
	*
	* @return string
	* @access public
	*/
	public function switchBranch($branch)
	{
		return $this->_gitLibRequest('switchBranch', Array($branch));

	}//end switchBranch()


	/**
	* Returns the current repo's clone URL
	*
	* @param boolean $full_url	If TRUE, include username/password bit in the HTTPs URL
	*
	* @return string
	* @access public
	*/
	public function getRepoUrl($full_url=TRUE)
	{
		$git_url = $this->_gitLibRequest('getCloneUrl');
		if (!$full_url) {
			$git_url = preg_replace('|(https://)[^@]*@(.*)|i', "$1$2", $git_url);
		}

		return $git_url;

	}//end getRepoUrl()


	/**
	* Add "username" and "password" into the set HTTPs URL
	*
	* @return string
	* @access private
	*/
	public function fullCloneUrl()
	{
		$https_url = $this->attr('clone_url');
		$username = $this->attr('username');
		$password = $this->attr('password');

		if (empty($https_url) || empty($username) || stripos($https_url, 'https://') !== 0) {
			return $https_url;
		}

		return 'https://'.$username.':'.$password.'@'.substr($https_url, strlen('https://'));

	}// end fullCloneUrl()


	/**
	* Returns the list of files and directories under a particular path
	*
	* @param string $path        The full path to the file
	* @param array  $ignore_dirs Ignore these dirs when listing dirs
	*
	* @return array
	* @access private
	* @see File_Bridge::_listFiles()
	*/
	protected function _listFiles($path, $ignore_dirs=Array())
	{
		// Do not display these dirs in the files listing
		$ignore_dirs = Array('CVS', 'svn', '.gitignore', '.git');

		return parent::_listFiles($path, $ignore_dirs);

	}//end _listFiles()


}//end class

?>
