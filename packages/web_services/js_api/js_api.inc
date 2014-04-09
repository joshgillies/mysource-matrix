<?php
/**
* +--------------------------------------------------------------------+
* | This MySource Matrix CMS file is Copyright (c) Squiz Pty Ltd	   |
* | ACN 084 670 600													   |
* +--------------------------------------------------------------------+
* | IMPORTANT: Your use of this Software is subject to the terms of    |
* | the Licence provided in the file licence.txt. If you cannot find   |
* | this file please contact Squiz (www.squiz.com.au) so we may provide|
* | you a copy.														   |
* +--------------------------------------------------------------------+
*
* $Id: js_api.inc,v 1.154.2.3 2013/10/23 04:30:28 akarelia Exp $
*
*/

require_once SQ_CORE_PACKAGE_PATH.'/page/page.inc';

/**
* Javascript API
*
*
* @author  Ash Karelia <akarelia@squiz.com.au>
* @author  Nic Hubbard <nic@zedsaid.com>
* @version $Revision: 1.154.2.3 $
* @package MySource_Matrix_Packages
* @subpackage web_services
*/
class JS_Api extends Page
{

	/*
	* maping array for the function
	* and the attribute to enable them
	*/
	private $_attribute_mapping = Array(
									'getAssetTypes'					=> 'get_asset_types',
									'trashAsset'					=> 'trash_asset',
									'getChildCount'					=> 'get_general',
									'updateMultipleLinks'			=> 'update_link',
									'removeMultipleLinks'			=> 'remove_link',
									'importAssetsFromXML'			=> 'import_assets',
									'getRoles'						=> 'get_roles',
									'executeHTMLTidy'				=> 'execute_html_tidy',
									'getGeneral'					=> 'get_general',
									'getAttributes'					=> 'get_attributes',
									'setMultipleAttributes'			=> 'set_attribute',
									'setAttribute'					=> 'set_attribute',
									'getMetadata'					=> 'get_metadata',
									'setMetadataAllFields'			=> 'set_metadata',
									'setMetadata'					=> 'set_metadata',
									'getChildren'					=> 'get_children',
									'getParents'					=> 'get_parents',
									'getPermissions'				=> 'get_permissions',
									'createAsset'					=> 'create_asset',
									'getLocksInfo'					=> 'get_locks_info',
									'acquireLock'					=> 'acquire_lock',
									'releaseLock'					=> 'release_lock',
									'createLink'					=> 'create_link',
									'removeLink'					=> 'remove_link',
									'moveLink'						=> 'move_link',
									'updateLink'					=> 'update_link',
									'getLinkId'						=> 'get_link_id',
									'getAssetTree'					=> 'get_asset_tree',
									'getKeywordsReplacements'		=> 'get_keywords_replacements',
									'setAssetStatus'				=> 'set_asset_status',
									'getWebPath'					=> 'get_web_path',
									'setWebPath'					=> 'set_web_path',
									'getWorkflowSchema'				=> 'get_workflow_schema',
									'createFileAsset'				=> 'create_asset',
									'setContentOfEditableFileAsset'	=> 'set_file_content',
									'getLineage'					=> 'get_lineage',
									'getLineageFromUrl'				=> 'get_lineage_from_url',
									'getUrlFromLineage'				=> 'get_url_from_lineage',
									'batchRequest'					=> 'batch_requests',
									'cloneAsset'					=> 'clone_asset',
									'showDifference'				=> 'show_diff',
									'setContext'					=> 'set_context',
									'restoreContext'				=> 'set_context',
									'getAlternateContext'			=> 'get_context',
									'getCurrentContext'				=> 'get_context',
									'getAllContexts'				=> 'get_context',
									'getMetadataSchema'				=> 'get_metadata_schema',
									'editMetadataSchema'				=> 'edit_metadata_schema',
								  );

	/*
	* array mapping the function call
	* the permission needed to access them
	*/
	private $_permission_mapping = Array(
									'getGeneral'					=> Array('read_access'),
									'getAttributes'					=> Array('read_access'),
									'getMetadata'					=> Array('read_access'),
									'getChildren'					=> Array('read_access'),
									'getParents'					=> Array('read_access'),
									'getPermissions'				=> Array('read_access'),
									'getLocksInfo'					=> Array('read_access'),
									'getLinkId'						=> Array('read_access'),
									'getAssetTree'					=> Array('read_access'),
									'getKeywordsReplacements'		=> Array('read_access'),
									'getWebPath'					=> Array('read_access'),
									'getWorkflowSchema'				=> Array('read_access'),
									'showDifference'				=> Array('read_access'),
									'setMultipleAttributes'			=> Array('write_access'),
									'setAttribute'					=> Array('write_access'),
									'setMetadataAllFields'			=> Array('write_access'),
									'setMetadata'					=> Array('write_access'),
									'createAsset'					=> Array('write_access', 'ignore_perm'),
									'cloneAsset'					=> Array('write_access', 'ignore_perm'),
									'createFileAsset'				=> Array('write_access', 'ignore_perm'),
									'acquireLock'					=> Array('write_access'),
									'releaseLock'					=> Array('write_access'),
									'createLink'					=> Array('write_access'),
									'removeLink'					=> Array('write_access'),
									'moveLink'						=> Array('write_access'),
									'updateLink'					=> Array('write_access'),
									'setAssetStatus'				=> Array('write_access'),
									'setWebPath'					=> Array('write_access'),
									'setContentOfEditableFileAsset'	=> Array('write_access'),
									'getMetadataSchema'				=> Array('write_access'),
									'editMetadataSchema'			=> Array('write_access'),
									'getAssetTypes'					=> Array('no_check'),
									'trashAsset'					=> Array('no_check'),
									'getChildCount'					=> Array('no_check'),
									'updateMultipleLinks'			=> Array('no_check'),
									'removeMultipleLinks'			=> Array('no_check'),
									'importAssetsFromXML'			=> Array('no_check'),
									'getRoles'						=> Array('no_check'),
									'executeHTMLTidy'				=> Array('no_check'),
									'getLineage'					=> Array('no_check'),
									'getLineageFromUrl'				=> Array('no_check'),
									'getUrlFromLineage'				=> Array('no_check'),
									'batchRequest'					=> Array('no_check'),
									'getAlternateContext'			=> Array('no_check'),
									'getCurrentContext'				=> Array('no_check'),
									'getAllContexts'				=> Array('no_check'),
									'setContext'					=> Array('no_check'),
									'restoreContext'				=> Array('no_check'),
								   );


	/**
	* Constructor
	*
	* @param int	$assetid	the asset id to be loaded
	*
	*/
	function __construct($assetid=0)
	{
		$this->_ser_attrs = TRUE;
		parent::__construct($assetid);

	}//end constructor


	/**
	* Perform any additional processing required during the creation of this asset
	*
	* Pages add a default web path when they are created
	*
	* @param array	&$link	information used to create the initial link
	*
	* @return boolean
	* @access private
	*/
	protected function _createAdditional(Array &$link)
	{
		if (!parent::_createAdditional($link)) return FALSE;

		// Create a random key
		$GLOBALS['SQ_SYSTEM']->am->acquireLock($this->id, 'attributes');
		$key = rand(1000000000, 9999999999);
		$this->setAttrValue('api_key', $key);
		$this->saveAttributes();
		$GLOBALS['SQ_SYSTEM']->am->releaseLock($this->id, 'attributes');

		return $this->makeAndSaveInitialWebPath(strtolower($this->attr('name').'.js'), $link);

	}//end _createAdditional()


	/**
	* Returns an array of all the permitted links type, the type asset and the cardinality
	*
	* @return array
	* @access private
	* @see Asset::_getAllowLinks()
	*/
	public function _getAllowedLinks()
	{
		$links = parent::_getAllowedLinks();
		$links[SQ_LINK_NOTICE] = Array('asset' => Array('card' => 'M', 'exclusive' => FALSE));
		return $links;

	}//end _getAllowedLinks()


	/**
	* Print the frontend of the asset without the design
	*
	* @return void
	* @access public
	*/
	function printFrontend()
	{
		// Make sure the user can log in first
		if (!$this->readAccess()) {
			$GLOBALS['SQ_SYSTEM']->paintLogin(translate('login'), translate('cannot_access_asset', $this->name));
			return;
		}

		// Should we print our JS file, or send back some JSON?
		if (!isset($_REQUEST['key']) && !isset($_REQUEST['id']) && !isset($_REQUEST['type'])) {
			header('Content-Type: text/javascript');
			header('Cache-Control: no-store, no-cache, must-revalidate, post-check=0, pre-check=0');
			if ($this->attr('use_enhanced_js')) {
				require_once 'js_api_enhanced_function_list.inc';
				echo Js_Api_Enhanced_Function_List::paintJSFunctions($this);
			} else {
				require_once 'js_api_standard_function_list.inc';
				echo Js_Api_Standard_Function_List::paintJSFunctions($this);
			}
			return;
		}


		// Overriding the matrix error handler so HTML doesn't get printed to the screen
		require_once dirname(__FILE__).'/js_api_error_handler.inc';
		$old_error_handler = set_error_handler('js_api_error_handler');

		// Get our JSON data that was sent, then encode it into an array
		$api_key = array_get_index($_REQUEST, 'key', '');
		$id = array_get_index($_REQUEST, 'id', '');
		// if id is still empty try getting 'assetid'
		if (empty($id)) $id = array_get_index($_REQUEST, 'assetid', '');
		$function_type = array_get_index($_REQUEST, 'type', '');

		// nonce token check
		$nonce_token = get_unique_token();
		if (!isset($_POST['nonce_token']) || ($_POST['nonce_token'] !== $nonce_token)) {
			$this->returnError('nonce token is invalid');
			restore_error_handler();
			return;
		}

		// Check to see if we have a function type
		if (empty($function_type)) {
			$this->returnError('You must set a function type');
			restore_error_handler();
			return;
		}

		if ($this->attr('sync_context') && isset($_SESSION[SQ_SESSION_SANDBOX_INDEX]['SQ_BACKEND_ALTERNATE_CONTEXT_ID'])) {
			$current_contextid = $_SESSION[SQ_SESSION_SANDBOX_INDEX]['SQ_BACKEND_ALTERNATE_CONTEXT_ID'];
			if (!isset($_SESSION[SQ_SESSION_SANDBOX_INDEX]['SQ_ALTERNATE_CONTEXT_ID']) ||
					$_SESSION[SQ_SESSION_SANDBOX_INDEX]['SQ_ALTERNATE_CONTEXT_ID'] != $current_contextid) {
				$_SESSION[SQ_SESSION_SANDBOX_INDEX]['SQ_ALTERNATE_CONTEXT_ID'] = $current_contextid;
				$GLOBALS['SQ_SYSTEM']->changeContext($current_contextid);
			}
		}

		$this->tmp['batched_request'] = FALSE;
		if ($function_type == 'batchRequest') {
			// check to see if batching is allowed else complain
			if (!$this->_attribute_mapping[$function_type]) {
				$this->returnError('The '.$type.' function must be activated');
				restore_error_handler();
				return;
			}
			if (!function_exists('json_decode')) {
				require_once 'Services/JSON.php';
				$json = new Services_JSON();
				$functions_info = Array();
				$request_data = $json->decode($_REQUEST['functions']);
				$functions_info = $this->object2Array($request_data);
			} else {
				$functions_info = json_decode($_REQUEST['functions'], TRUE);
			}//end else

			foreach ($functions_info as $index => $info) {
				$function_calls[$index] = $info['function'];
			}
			$batching_request = TRUE;
			$this->tmp['batched_request'] = TRUE;
		} else {
			$function_calls = Array($function_type);
			$batching_request = FALSE;
		}

		// Set our API Key
		$check_key = $this->attr('api_key');

		// Check to see if we even have an API Key
		if (!isset($check_key) || $check_key == '' || $api_key == '') {
			$this->returnError('You must enter an API key in order to use this asset');
			restore_error_handler();
			return;
		}

		// Check to see if the API key attribute matches the JSON key
		if ($check_key != $api_key) {
			$this->returnError('The API key does not match, please enter a valid key');
			restore_error_handler();
			return;
		}

		// #4372  Feature Request : JS API ability to specify _admin/_edit access
		// Lets populate our $_REQUEST var so that we can use it later :)
		$_REQUEST['force_simple_edit'] = $this->attr('force_simple_edit');

		$temp_server_vars_php_self = array_get_index($_SERVER, 'PHP_SELF', NULL);;
		$temp_server_vars_request_uri = array_get_index($_SERVER, 'REQUEST_URI', NULL);

		if (isset($_SERVER['HTTP_REFERER']) && !empty($_SERVER['HTTP_REFERER'])) {
			$referer_url = str_replace(current_protocol().'://', '', $_SERVER['HTTP_REFERER']);

			$suffix_to_check = Array(
								SQ_CONF_BACKEND_SUFFIX,
								SQ_CONF_LIMBO_SUFFIX,
								SQ_CONF_NOCACHE_SUFFIX,
								SQ_CONF_RECACHE_SUFFIX,
								SQ_CONF_PERFORMANCE_SUFFIX
							   );
			// if we are in Backend or limbo remove everything after /_edit or /_admin
			// or else getAssetFromURL will get confused and wont return us correct asset
			foreach ($suffix_to_check as $suffix) {
				if (strpos($_SERVER['HTTP_REFERER'], $suffix) !== FALSE) {
					$referer_url = preg_replace('|/'.$suffix.'(.)*|', '', $referer_url);
				}
			}

			$url_asset_object = $GLOBALS['SQ_SYSTEM']->am->getAssetFromURL(current_protocol(), $referer_url, TRUE, TRUE);
			if (!is_null($url_asset_object) ) {
				$host = array_get_index($_SERVER, 'HTTP_HOST', '');
				$_SERVER['PHP_SELF'] = str_replace(current_protocol().'://'.$host, '', $_SERVER['HTTP_REFERER']);
			}

			if (strpos($_SERVER['HTTP_REFERER'], SQ_CONF_LIMBO_SUFFIX) !== FALSE || $this->attr('force_simple_edit')) {
				$_SERVER['PHP_SELF'] = preg_replace('|/'.SQ_CONF_LIMBO_SUFFIX.'(.)*|', '', $_SERVER['PHP_SELF']);
				$_SERVER['PHP_SELF'] = preg_replace('|/'.SQ_CONF_BACKEND_SUFFIX.'(.)*|', '', $_SERVER['PHP_SELF']);
				$_SERVER['PHP_SELF'] = $_SERVER['PHP_SELF'].'/';
				$_REQUEST['force_simple_edit'] = TRUE;
			}

			// If 'PHP_SELF' was updated, set the same to 'REQUEST_URI'
			if (!is_null($url_asset_object) ) {
				$_SERVER['REQUEST_URI'] = $_SERVER['PHP_SELF'];
			}
		}

		// keep are results array empty to start with
		$results = Array();

		// if we are batching the request lets keep the
		// track of the number of functions we are executing
		$track_func = 0;
		$replace_these = Array ('%26' , '%23' , '%3F' , '%2B');
		$replace_with = Array ('&' , '#' , '?' , '+');

		foreach ($function_calls as $type) {
			// lets decide on what arguments we need to pass through for execution
			$blocking = FALSE;
			if ($batching_request) {
				$args = $functions_info[$track_func]['args'];

				$blocking = (isset($functions_info[$track_func]['blocking']) && $functions_info[$track_func]['blocking']) ? TRUE : FALSE;
				// let the user use 'id' and 'asset_id' to pass in the asset id to work on in the argmunets
				// bug fix #5945 Inconsistency with the parameter needs to be passed to (advanced) JS API functions
				// few of the functions in the advanced API passes the assetid as a different index name. Make sure
				// to look for them and change it here.
				if (!isset($args['id'])) {
					if (isset($args['asset_id'])) {
						$args['id'] = $args['asset_id'];
					} else if (isset($args['child_id'])) {
						$args['id'] = $args['child_id'];
					} else if (isset($args['parentID'])) {
						$args['id'] = $args['parentID'];
					} else if (isset($args['parent_id'])) {
						$args['id'] = $args['parent_id'];
					}
				}

				// so we are batching  requests, in this  case we  get the assetid needed to
				// start with from the first function call  and then every consecutive  time
				// we  get it from the output of  previously  executed function  so check if
				// format there is $results_0_*$ type else it might be raw input that we use
				if ($track_func === 0) {
					$id = $args['id'];
				} else {
					// okie so we arent the first called function in batched request
					// and we are to use the assetid from the previous function exec
					// but no we look into any of the argument where the  $result_*$
					// type  keywords might  have been used and  try and evaluate it
					foreach ($args as $idx => $arg) {
						if (preg_match('/[\$%]results_[^\$%]*[\$%]/', $args[$idx], $matches)) {
							if(!isset($matches[0])) continue;
							$keyword = $matches[0];
							$instructions = explode('_', trim($keyword, '$%'));
							// if we know where to take the new id from, do it or else
							// just  continue with the previous one we have been using
							$string_to_eval = '';
							if (count($instructions) >= 3) {
								for ($i = 1; $i < count($instructions); $i++) {
									$string_to_eval .= '[$'."instructions[$i]]";
								}
								eval('$args[$idx] = $results'.$string_to_eval.';');
							}//end if
						}//end if
					}//end foreach
					$id = $args['id'];
				}//end if/else
			} else {
				// old API way
				$args = $_REQUEST;
			}

			// fix our args elements
			foreach ($args as $index => $arg) {
				$args[$index] = str_replace($replace_these, $replace_with, $args[$index]);
			}

			// first thing check if the function that is being tried to executed is enabled
			if ($this->attr($this->_attribute_mapping[$type])) {
				$asset = NULL;
				if ($this->_permission_mapping[$type] != Array('no_check')) {
					// Check to see if we supplied with valid assetid
					if ((preg_match('/^[0-9]*:/i', $id) != FALSE) && !$GLOBALS['SQ_SYSTEM']->am->assetExists($id)) {
						$results[$track_func]['error'] = 'Assetid supplied doesnt exist on the system';
						$track_func++;
						break;
					}
					$asset = $this->setAsset($id);

					// Check to see if passed ID is under our root node, or, if we are using a root node
					if (!$this->checkRoot($asset)) {
						$this->returnError('You do not have permissions to access this asset');
						restore_error_handler();
						return;
					}

					if ($this->_permission_mapping[$type] == Array('read_access')) {
						$ra = $asset->readAccess();
						$wa = FALSE;
					} else {
						// check if there is any runing workflow schema on it
						$wfm = $GLOBALS['SQ_SYSTEM']->getWorkflowManager();
						$running_wfs = $wfm->getSchemas($asset->id, TRUE, TRUE);
						if(!empty($running_wfs)) {
							$wa = $asset->writeAccess();
						} else {
							$wa = $asset->writeAccess('', Array(), FALSE);
						}
						$ra = FALSE;
					}
				}

				$func_name = '_'.$type;
				try {
					$results[$track_func] = $this->$func_name($args, $asset, $ra, $wa);
				} catch (Exception $e) {
					// If any exception was thrown when executing the function, catch it
					$results[$track_func]['error'] = 'Exception occured when executing JS API function "'.$type.'()": '.$e->getMessage();
					js_api_error_handler(E_USER_ERROR, 'EXCEPTION: '.$e->getMessage(), $e->getFile(), $e->getLine());
				}

				// if batching request and one of the function is set to blocking
				// and of the function execution returned error bail out
				if (isset($results[$track_func]['error']) && $blocking) {
					$results['error'] = 'Function '.$type.' set to blocking returned error';
					break;
				}

			} else {
				$this->returnError('The '.$type.' function must be activated');
				restore_error_handler();
				return;
			}
			$track_func++;
		}

		// if we are batching request, send back all the data we have
		// if not, then pick the 0th index of the results because thats our baby
		if ($function_type == 'batchRequest') {
			$data = $results;
		} else {
			$data = $results[$track_func - 1];
		}

		// Send our data as JSON
		if (isset($data)) {
			$this->returnJSON($data, $type);
		}//end if

		// lets restor PHP_SELF just incase if Matrix is going to check if $_SERVER var is
		// exactly the same when it went in THIS function and when it goes out
		$_SERVER['PHP_SELF'] = $temp_server_vars_php_self;
		$_SERVER['REQUEST_URI'] = $temp_server_vars_request_uri;


		// Restore error handler
		restore_error_handler();

	}//end printFrontend()


	/**
	* Description: This will create a link between two assets
	*
	* @param string		$id		The asset that we are getting
	*
	* @return object
	* @access public
	*/
	function setAsset($id)
	{
		// Set some shortcuts
		$am = $GLOBALS['SQ_SYSTEM']->am;

		// Check to see if the asset id is real
		if ($id == '') {
			$this->returnError('You must enter a valid asset id or URL');
			return FALSE;
		}

		// Check to see if we are using an id or URL
		if (is_numeric($id) || (preg_match('/^[0-9]*:/i', $id) != 0)) {

			// Check to see if the asset id is real
			try {
				$valid_assetid = $am->assetExists($id);
			} catch (Exception $e) {
				$this->returnError($id.' is not a valid asset id');
				return FALSE;
			}
			if (!$valid_assetid) {
				$this->returnError($id.' is not a valid asset id');
				return FALSE;
			}
			// Set our asset reference
			return $am->getAsset($id);

		} else {
			// We need to make sure this is a URL
			if (strpos($id, 'http://') !== FALSE || strpos($id, 'https://') !== FALSE) {
				// figure out if we are on dealing with a http or a https url here
				// results will be wrong here further if we let Matrix assume if the
				// protocol is always http
				$url_parts = parse_url($id);
				$protocol = (isset($url_parts['scheme']) && !empty($url_parts['scheme'])) ? $url_parts['scheme'] : 'http';

				// Set our asset reference
				$asset = $am->getAssetFromURL($protocol, strip_url($id, TRUE), TRUE, TRUE);
				if (is_null($asset)) $this->returnError($id.' is not a valid URL');
				return $asset;
			} else {
				$this->returnError($id.' is not a valid URL');
				return FALSE;
			}//end else

		}//end else

	}//end setAsset


	/**
	* Gets all the installed asset types on the system
	*
	* @param array		$args			bundled array of arguments do be used while performing operations
	* @param object		$asset			the asset we are executing this function on
	* @param boolean	$read_access	read access check for the asset we
	*									are dealing with with respect to current user
	* @param boolean	$write_access	write access check for the asset we
	*									are dealing with with respect to current user
	*
	* @return array
	* @access private
	*/
	private function _getAssetTypes($args=Array(), $asset=NULL, $read_access=FALSE, $write_access=FALSE)
	{
		$data = $GLOBALS['SQ_SYSTEM']->am->getAssetTypes(TRUE, TRUE);
		// Sort in A-Z order
		ksort($data);

		return $data;

	}// end _getAssetTypes


	/**
	* trashes the supplied assetids
	*
	* @param array		$args			bundled array of arguments do be used while performing operations
	* @param object		$asset			the asset we are executing this function on
	* @param boolean	$read_access	read access check for the asset we
	*									are dealing with with respect to current user
	* @param boolean	$write_access	write access check for the asset we
	*									are dealing with with respect to current user
	*
	* @return array
	* @access private
	*/
	private function _trashAsset($args=Array(), $asset=NULL, $read_access=FALSE, $write_access=FALSE)
	{
		if (isset($args['asset_ids']) || (isset($args['assetid']) && !empty($args['assetid']))) {
			$assetids = Array();
			if(isset($args['asset_ids'])) {
				$assetids = $args['asset_ids'];
			}
			else {
				$assetids = explode('\\,', $args['assetid']);
			}
			$data = $this->trashAsset($assetids);
		} else {
			$data['error'] = 'Please Enter Asset Id(s) to move to Trash';
		}

		return $data;

	}// end _trashAsset


	/**
	* get the child count for the passed assets.
	* we have an option to go get the child count upto particular level.
	*
	* @param array		$args			bundled array of arguments do be used while performing operations
	* @param object		$asset			the asset we are executing this function on
	* @param boolean	$read_access	read access check for the asset we
	*									are dealing with with respect to current user
	* @param boolean	$write_access	write access check for the asset we
	*									are dealing with with respect to current user
	*
	* @return array
	* @access private
	*/
	private function _getChildCount($args=Array(), $asset=NULL, $read_access=FALSE, $write_access=FALSE)
	{
		if (isset($args['asset_id']) && !empty($args['asset_id'])) {
			$assetid = $args['asset_id'];
			// Check to see how deep we should go
			if ($args['levels'] == '0') {
				$level = NULL;
			} else {
				$level = $args['levels'];
			}
			$children = $GLOBALS['SQ_SYSTEM']->am->getChildren($assetid, '', TRUE, NULL, NULL, NULL, TRUE, NULL, $level);
			$data['child_count'] = count($children);
		} else {
			$data['error'] = 'Please Enter Asset Id to get the child count';
		}

		return $data;

	}// end _getChildCount


	/**
	* Updates multiple links
	*
	* @param array		$args			bundled array of arguments do be used while performing operations
	* @param object		$asset			the asset we are executing this function on
	* @param boolean	$read_access	read access check for the asset we
	*									are dealing with with respect to current user
	* @param boolean	$write_access	write access check for the asset we
	*									are dealing with with respect to current user
	*
	* @return array
	* @access private
	*/
	private function _updateMultipleLinks($args=Array(), $asset=NULL, $read_access=FALSE, $write_access=FALSE)
	{
		$links = $data = Array();
		if(isset($args['link_info']['links'])) {
			foreach ($args['link_info']['links'] as $link) {
			$links['parents'][] = $link['parent'];
			$links['children'][] = $link['child'];
			$links['existing_link_type'][] = $link['existing_link_type'];
			$links['existing_link_value'][] = $link['existing_link_value'];
			$links['link_type'][] = $link['link_type'];
			$links['link_value'][] = $link['link_value'];
			$links['sort_order'][] = $link['sort_order'];
			$links['lock'][] = $link['link_lock'];
			}
		}
		else {
			$links['parents']  = explode('\\,', $args['parent_id']);
			$links['children'] = explode('\\,', $args['child_id']);
			$links['existing_link_type'] = explode('\\,', $args['existing_link_type']);
			$links['existing_link_value'] = explode('\\,', $args['existing_link_value']);
			$links['link_type'] = explode('\\,', $args['link_type']);
			$links['link_value'] = explode('\\,', $args['link_value']);
			$links['sort_order'] = explode('\\,', $args['sort_order']);
			$links['lock'] = explode('\\,', $args['locked']);
		}



		foreach ($links as $elements) {
			if (count($elements) != count($links['parents'])) {
				$data['error'] =  'Please provide correct number of link elements';
				return $data;
			}
		}

		//looks like correct number of elements are passed
		for ($i = 0;  $i < count($links['parents']); $i++) {
			// Update our link
			if($links['children'][$i] == 'Undefined' || $links['children'][$i] == '' ) {
				$data[$i]['error'] = 'Child Id whose link is to be update was not provided';
				continue;
			} else if ($links['parents'][$i] == 'Undefined' || $links['parents'][$i] == ''){
				$data[$i]['error'] = 'Parent Id whose link is to be update was not provided';
				continue;
			}

			$child_asset = $GLOBALS['SQ_SYSTEM']->am->getAsset($links['children'][$i]);
			if(is_null($child_asset)) {
				$data[$i]['error'] = 'Asset #'.$links['children'][$i].' does not exist';
				continue;
			}

			$parent_asset = $GLOBALS['SQ_SYSTEM']->am->getAsset($links['parents'][$i]);
			if(is_null($parent_asset)) {
				$data[$i]['error'] = 'Asset #'.$links['parents'][$i].' does not exist';
				continue;
			}

			if ($child_asset->writeAccess()) {

				$ext_link_type = defined($links['existing_link_type'][$i]) ? constant($links['existing_link_type'][$i]) : SQ_LINK_TYPE_1;
				$ext_link_value = empty($links['existing_link_value'][$i]) ? NULL : $links['existing_link_value'][$i];
				$link_type = defined($links['link_type'][$i]) ? constant($links['link_type'][$i]) : SQ_LINK_TYPE_1;
				$link_value = $links['link_value'][$i];
				$data[$i] = $this->updateLink(
										$links['parents'][$i] ,
										$links['children'][$i] ,
										$ext_link_type,
										$ext_link_value,
										$link_type,
										$link_value ,
										($links['sort_order'][$i] != '' && $links['sort_order'][$i] != 'undefined') ? $links['sort_order'][$i] : NULL ,
										($links['lock'][$i] != '' && $links['lock'][$i] != 'undefined') ? $links['lock'][$i] : NULL
									);
			} else {
				$data[$i] = 'Not enough permission to update link between Asset (#'.$child_asset->id.') and Asset (#'.$parent_asset->id.')';
			}
		}

		return $data;

	}// end _updateMultipleLinks


	/**
	* removes multipls links passed in the argumnets
	*
	* @param array		$args			bundled array of arguments do be used while performing operations
	* @param object		$asset			the asset we are executing this function on
	* @param boolean	$read_access	read access check for the asset we
	*									are dealing with with respect to current user
	* @param boolean	$write_access	write access check for the asset we
	*									are dealing with with respect to current user
	*
	* @return array
	* @access private
	*/
	private function _removeMultipleLinks($args=Array(), $asset=NULL, $read_access=FALSE, $write_access=FALSE)
	{
		$links = $data = Array();
		if(isset($args['link_info']['links'])) {
			foreach ($args['link_info']['links'] as $link) {
			$links['parents'][] = $link['parent'];
			$links['children'][] = $link['child'];
			$links['link_type'][] = $link['link_type'];
			$links['link_value'][] = $link['link_value'];
			}
		}
		else {
			$links['parents']  = explode('\\,', $args['parent_id']);
			$links['children'] = explode('\\,', $args['child_id']);
			$links['link_type'] = explode('\\,', $args['link_type']);
			$links['link_value'] = explode('\\,', $args['link_value']);
		}

		foreach ($links as $elements) {
			if (count($elements) != count($links['parents'])) {
				$data['error'] = 'Please provide correct number of link elements';
				return $data;
			}
		}

		//looks like correct number of elements are passed
		for ($i = 0;  $i < count($links['parents']); $i++) {
			// Update our link
			if($links['children'][$i] == 'Undefined' || $links['children'][$i] == '' || $links['children'][$i] == NULL) {
				$data[$i]['error'] = 'Child Id whose link is to be update was not provided';
				continue;
			} else if ($links['parents'][$i] == 'Undefined' || $links['parents'][$i] == ''|| $links['children'][$i] == NULL){
				$data[$i]['error'] = 'Parent Id whose link is to be update was not provided';
				continue;
			}

			$child_asset = $GLOBALS['SQ_SYSTEM']->am->getAsset($links['children'][$i]);
			if(is_null($child_asset)) {
				$data[$i]['error'] = 'Asset #'.$links['children'][$i].' does not exist';
				continue;
			}

			$parent_asset = $GLOBALS['SQ_SYSTEM']->am->getAsset($links['parents'][$i]);
			if(is_null($parent_asset)) {
				$data[$i]['error'] = 'Asset #'.$links['parents'][$i].' does not exist';
				continue;
			}

			if ($child_asset->writeAccess()) {

				$link_value = $links['link_value'][$i];
				$link_type = defined($links['link_type'][$i]) ? constant($links['link_type'][$i]) : SQ_LINK_TYPE_1;

				$data[$i] = $this->removeLink($parent_asset->id, $child_asset->id, $link_type, $link_value);
			} else {
				$data[$i] = 'Not enough permission to remove link between Asset (#'.$child_asset->id.') and Asset (#'.$parent_asset->id.')';
			}
		}

		return $data;

	}// end _removeMultipleLinks


	/**
	* import assets from a structured XML provided
	*
	* @param array		$args			bundled array of arguments do be used while performing operations
	* @param object		$asset			the asset we are executing this function on
	* @param boolean	$read_access	read access check for the asset we
	*									are dealing with with respect to current user
	* @param boolean	$write_access	write access check for the asset we
	*									are dealing with with respect to current user
	*
	* @return array
	* @access private
	*/
	private function _importAssetsFromXML($args=Array(), $asset=NULL, $read_access=FALSE, $write_access=FALSE)
	{
		$data =Array();
		$filePath = $args['filePath'];
		$import_under = $args['id'];

		//check if the file exists
		if (file_exists($filePath) || $filePath == 'no_file_provided') {
			//check 2 make sure it is a XML file
			if (strtolower(substr($filePath, -3)) == 'xml') {
				//looks all good...makes sure the asset id provided is valid
				if($GLOBALS['SQ_SYSTEM']->am->assetExists($import_under)) {
					// set HIPO running_vars
					$vars['file_info'] = $filePath;
					$vars['create_under_assetid'] = $import_under;
					$vars['delete_after_import'] = FALSE;

					// run HIPO job
					$hh = $GLOBALS['SQ_SYSTEM']->getHipoHerder();
					$errors = $hh->freestyleHipo('hipo_job_import_assets_from_xml', $vars, SQ_PACKAGES_PATH.'/import_tools/hipo_jobs');
					if (empty($errors)) {
						$data['success'] = 'Assets sucessfully imported under Asset #'.$import_under;
					} else {
						$data['error'] = 'Error occured while importing assets  under Asset #'.$import_under;
					}

				} else {
					$data['error'] = 'Assetid provided to import assets under is not valid';
				}
			} else {
				$data['error'] = 'File provided is not XML';
			}
		} else {
			$data['error'] = 'No File path provided OR File does not exist for the provided path';
		}

		return $data;

	}// end _importAssetsFromXML


	/**
	* get the defined roles for particular assets based on the information passed
	*
	* @param array		$args			bundled array of arguments do be used while performing operations
	* @param object		$asset			the asset we are executing this function on
	* @param boolean	$read_access	read access check for the asset we
	*									are dealing with with respect to current user
	* @param boolean	$write_access	write access check for the asset we
	*									are dealing with with respect to current user
	*
	* @return array
	* @access private
	*/
	private function _getRoles($args=Array(), $asset=NULL, $read_access=FALSE, $write_access=FALSE)
	{
		$assetid			= isset($args['id']) && $args['id'] != '' ? $args['id'] : NULL;
		$roleid				= isset($args['roleid']) && $args['roleid'] != '' ? $args['roleid'] : NULL;
		$userid				= isset($args['userid']) && $args['userid'] != '' ? $args['userid'] : NULL;
		$include_assetid	= isset($args['include_assetid']) && $args['include_assetid'] != '' ? $args['include_assetid'] : FALSE;
		$include_globals	= isset($args['include_globals']) && $args['include_globals'] != '' ? $args['include_globals'] : FALSE;
		$expand_groups		= isset($args['expand_groups']) && $args['expand_groups'] != '' ? $args['expand_groups'] : FALSE;
		$inc_dependants		= isset($args['inc_dependants']) && $args['inc_dependants'] != '' ? $args['inc_dependants'] : TRUE;
		$include_parents		= isset($args['include_parents']) && $args['include_parents'] != '' ? $args['include_parents'] : FALSE;
		$type_codes		= (isset($args['type_codes']) && $args['type_codes'] != '') ? explode('\\,', $args['type_codes']) : Array();
		$strict_type_code		= isset($args['strict_type_code']) && $args['strict_type_code'] != '' ? $args['strict_type_code'] : TRUE;

		$data				= $GLOBALS['SQ_SYSTEM']->am->getRole($assetid, $roleid, $userid, $include_assetid, $include_globals, $expand_groups, $inc_dependants, $include_parents, $type_codes,$strict_type_code);

		if(empty($data)) $data[] = 'No roles data is found';
		return $data;

	}// end _getRoles


	/**
	* executes HTML Tidy on the content passed
	*
	* @param array		$args			bundled array of arguments do be used while performing operations
	* @param object		$asset			the asset we are executing this function on
	* @param boolean	$read_access	read access check for the asset we
	*									are dealing with with respect to current user
	* @param boolean	$write_access	write access check for the asset we
	*									are dealing with with respect to current user
	*
	* @return array
	* @access private
	*/
	private function _executeHTMLTidy($args=Array(), $asset=NULL, $read_access=FALSE, $write_access=FALSE)
	{
		$str_to_process = isset($args['content']) && $args['content'] != '' ? $args['content'] : NULL;

		if (!is_null($str_to_process)) {

			global $ROOT_PATH;
			$ROOT_PATH = SQ_FUDGE_PATH.'/wysiwyg/';
			require_once SQ_FUDGE_PATH.'/wysiwyg/plugins/html_tidy/html_tidy.inc';

			$tidy = new HTML_Tidy();
			$tidy->process($str_to_process);

			if ($tidy->htmltidy_status == 'pass') {
				$data[] = $str_to_process;

			} else if ($tidy->htmltidy_status == 'fail') {
				$data['error'] = 'HTML Tidy failed to process the given content';

			} else if ($tidy->htmltidy_status == 'wait') {
				$data['error'] = 'HTML Tidy wasn\'t ready to process content. Try again';

			} else if ($tidy->htmltidy_status == 'disabled') {
				$data['error'] = 'HTML Tidy is disabled on the System';
			}

		} else {
			$data['error'] = 'No string passes to process HTMLTidy on';
		}

		return $data;

	}// end _executeHTMLTidy


	/**
	* gets general information (and if configured) attributes of the assets
	*
	* @param array		$args			bundled array of arguments do be used while performing operations
	* @param object		$asset			the asset we are executing this function on
	* @param boolean	$read_access	read access check for the asset we
	*									are dealing with with respect to current user
	* @param boolean	$write_access	write access check for the asset we
	*									are dealing with with respect to current user
	*
	* @return array
	* @access private
	*/
	private function _getGeneral($args=Array(), $asset=NULL, $read_access=FALSE, $write_access=FALSE)
	{
		if ($read_access) {
			$get_attributes = isset($args['get_attributes']) ? $args['get_attributes'] : FALSE;
			$data = $this->getGeneralInfo($asset, $get_attributes);
		} else {
			$data['error'] = 'You do not have permissions to access this asset';
		}//end else

		return $data;

	}// end _getGeneral


	/**
	* get all the attributes for the passed asset
	*
	* @param array		$args			bundled array of arguments do be used while performing operations
	* @param object		$asset			the asset we are executing this function on
	* @param boolean	$read_access	read access check for the asset we
	*									are dealing with with respect to current user
	* @param boolean	$write_access	write access check for the asset we
	*									are dealing with with respect to current user
	*
	* @return array
	* @access private
	*/
	private function _getAttributes($args=Array(), $asset=NULL, $read_access=FALSE, $write_access=FALSE)
	{
		if ($read_access) {
			// Get available attributes
			$attributes = $GLOBALS['SQ_SYSTEM']->am->getAssetTypeAttributes($asset->type());
			foreach ($attributes as $key => $val) {
				if ($val['type'] == 'password') continue;
				if ($key == 'api_key') {
					// Added security to make sure no one can get API Keys from other API Assets
					$data[$key] = '';
				} else {
					$data[$key] = $asset->attr($key);
				}
			}//end foreach
		} else {
			$data['error'] = 'You do not have permissions to access this asset';
		}//end else

		return $data;

	}// end _getAttributes


	/**
	* sets multipls attributes passed for asset in a single call.
	* Basically a wrapper function to _setAttribute
	*
	* @param array		$args			bundled array of arguments do be used while performing operations
	* @param object		$asset			the asset we are executing this function on
	* @param boolean	$read_access	read access check for the asset we
	*									are dealing with with respect to current user
	* @param boolean	$write_access	write access check for the asset we
	*									are dealing with with respect to current user
	*
	* @return array
	* @access private
	*/
	private function _setMultipleAttributes($args=Array(), $asset=NULL, $read_access=FALSE, $write_access=FALSE)
	{
		return $this->_setAttribute($args, $asset, $read_access, $write_access);

	}// end _setMultipleAttributes


	/**
	* set the passed value of attribute for the given asset
	*
	* @param array		$args			bundled array of arguments do be used while performing operations
	* @param object		$asset			the asset we are executing this function on
	* @param boolean	$read_access	read access check for the asset we
	*									are dealing with with respect to current user
	* @param boolean	$write_access	write access check for the asset we
	*									are dealing with with respect to current user
	*
	* @return array
	* @access private
	*/
	private function _setAttribute($args=Array(), $asset=NULL, $read_access=FALSE, $write_access=FALSE)
	{
		if ($write_access) {
			if (isset($args['field_info']) || (isset($args['attr_name']) && isset($args['attr_val']))) {
					$attr_names = Array();
				$attr_vals = Array();

				if(isset($args['field_info'])) {
					foreach($args['field_info'] as $key => $value){
					$attr_names[] = $key;
					$attr_vals[] = $value;
					}
				}
				else {
					$attr_names = explode('\\,', $args['attr_name']);
					$attr_vals = explode('\\,', $args['attr_val']);
				}

				// Set our new values
				$success = $this->setAttributeValue($asset->id, $attr_names, $attr_vals);
				if($success) {
					$data = $success;
				} else {
					$data = 'Attribute for Asset #'.$asset->id.' were not set. Check Error Log for more information';
				}
			} else {
				$data['error'] = 'Please enter both the attribute name and value';
			}//end else
		} else {
			$data['error'] = 'You do not have permissions to access this asset';
		}//end else

		return $data;

	}// end _setAttribute


	/**
	* gets all the metadata on the asset we have passed in
	*
	* @param array		$args			bundled array of arguments do be used while performing operations
	* @param object		$asset			the asset we are executing this function on
	* @param boolean	$read_access	read access check for the asset we
	*									are dealing with with respect to current user
	* @param boolean	$write_access	write access check for the asset we
	*									are dealing with with respect to current user
	*
	* @return array
	* @access private
	*/
	private function _getMetadata($args=Array(), $asset=NULL, $read_access=FALSE, $write_access=FALSE)
	{
		if ($read_access) {
			$mm = $GLOBALS['SQ_SYSTEM']->getMetadataManager();
			$schema = $mm->getSchemas($asset->id, TRUE);
			if (!empty($schema)) {
				$data = $mm->getMetadataFieldValues($asset->id);
			} else {
				$data['error'] = 'No Metadata Schema applied to Asset "'.$asset->name.'(#'.$asset->id.')"';
			}
		} else {
			$data['error'] = 'You do not have permissions to access this asset';
		}//end else

		return $data;

	}//end _getMetadata


	/**
	* sets Multiple metadata for field in one go.
	* basically the wrapper function for _setMetadata
	*
	* @param array		$args			bundled array of arguments do be used while performing operations
	* @param object		$asset			the asset we are executing this function on
	* @param boolean	$read_access	read access check for the asset we
	*									are dealing with with respect to current user
	* @param boolean	$write_access	write access check for the asset we
	*									are dealing with with respect to current user
	*
	* @return array
	* @access private
	*/
	private function _setMetadataAllFields($args=Array(), $asset=NULL, $read_access=FALSE, $write_access=FALSE)
	{
		return $this->_setMetadata($args, $asset, $read_access, $write_access);

	}// end _setMetadataAllFields


	/**
	* set the metadata values to the fields provided for the asset
	*
	* @param array		$args			bundled array of arguments do be used while performing operations
	* @param object		$asset			the asset we are executing this function on
	* @param boolean	$read_access	read access check for the asset we
	*									are dealing with with respect to current user
	* @param boolean	$write_access	write access check for the asset we
	*									are dealing with with respect to current user
	*
	* @return array
	* @access private
	*/
	private function _setMetadata($args=Array(), $asset=NULL, $read_access=FALSE, $write_access=FALSE)
	{
		if ($write_access) {
			if (isset($args['field_info']) || (isset($args['field_id']) && isset($args['field_val']))) {
				require_once SQ_CORE_PACKAGE_PATH.'/metadata/metadata_field/metadata_field.inc';
				$field_ids = Array();
				$field_vals = Array();

				if(isset($args['field_info'])) {
					foreach($args['field_info'] as $key => $value){
					$field_ids[] = $key;
					$field_vals[] = $value;
					}
				}
				else {
					$field_ids = explode('\\,', $args['field_id']);
					$field_vals = explode('\\,', $args['field_val']);
				}

				foreach($field_vals as $index =>  $fieldVal) {
					$field_vals[$index] = Metadata_Field::encodeValueString($fieldVal, Array());
				}

				$result = $this->setAssetMetadata($asset->id, $field_ids, $field_vals);
				if($result) {
					$i = 0;
					foreach($field_vals as $index =>  $fieldVal) {
						$value = $fieldVal;
						$to_decode = Array('\=', '\;', '\\\\');
						$decoded_str = Array('=', ';', '\\');
						$value = str_replace($to_decode, $decoded_str, $value);
						$data['success'][] = 'Metadata field #'.$field_ids[$i].' has been successfully set to "'.$value.'" for Asset "'.$asset->name.'" (#'.$asset->id.')';
						$i++;
					}
				}
			} else {
				$data['error'] = 'Please enter both the field id and value';
			}//end else
		} else {
			$data['error'] = 'You do not have permissions to access this asset';
		}//end else

		return $data;

	}// end _setMetadata


	/**
	* get children assets for the passed asset based on type_code, link_type, link_values
	* optionally we can also get back all the attributes for the returned children
	*
	* @param array		$args			bundled array of arguments do be used while performing operations
	* @param object		$asset			the asset we are executing this function on
	* @param boolean	$read_access	read access check for the asset we
	*									are dealing with with respect to current user
	* @param boolean	$write_access	write access check for the asset we
	*									are dealing with with respect to current user
	*
	* @return array
	* @access private
	*/
	private function _getChildren($args=Array(), $asset=NULL, $read_access=FALSE, $write_access=FALSE)
	{

		if ($read_access) {
			if (empty($args['levels']) && $args['levels'] != '0') {
				$data['error'] = 'Please set a depth(levels) limit';
				return $data;
			}
			// Check to see how deep we should go
			if ($args['levels'] == '0') {
				$level = NULL;
			} else {
				$level = $args['levels'];
			}

			$type_codes  = Array();
			$link_types  = Array();
			$link_values = Array();

			// lets check for the type_codes, link_types, and link_values
			if(isset($args['type_codes']) && !empty($args['type_codes'])) {
				$type_codes = is_array($args['type_codes']) ? $args['type_codes'] : explode('\\,', $args['type_codes']);
			}
			if(isset($args['link_types']) && !empty($args['link_types'])) {
				$link_types = is_array($args['link_types']) ? $args['link_types'] : explode('\\,', $args['link_types']);
			}
			if(isset($args['link_values']) && !empty($args['link_values'])) {
				$link_values = is_array($args['link_values']) ? $args['link_values'] : explode('\\,', $args['link_values']);
			}
			$get_attributes	= isset($args['get_attributes']) ? $args['get_attributes'] : FALSE;

			$data = $this->getChildren($asset->id, $level, $type_codes, $link_types, $link_values, $get_attributes);

			if (empty($data)) {
				$data['error'] = 'Asset #'.$asset->id.' has no children';
			}
		} else {
			$data['error'] = 'You do not have permissions to access this asset';
		}//end else

		return $data;

	}// end _getChildren


	/**
	* gets parents for the passed asset.
	* results can be filtered by type_code, link_types, link_values.
	* optionally we can also get all the attributes for the result assets we get
	*
	* @param array		$args			bundled array of arguments do be used while performing operations
	* @param object		$asset			the asset we are executing this function on
	* @param boolean	$read_access	read access check for the asset we
	*									are dealing with with respect to current user
	* @param boolean	$write_access	write access check for the asset we
	*									are dealing with with respect to current user
	*
	* @return array
	* @access private
	*/
	private function _getParents($args=Array(), $asset=NULL, $read_access=FALSE, $write_access=FALSE)
	{
		if ($read_access) {
			if ($asset->id == 1) {
				$data['error'] = 'The root asset does not have any parents';
				return $data;
			}
			if (empty($args['levels']) && $args['levels'] != '0') {
				$data['error'] = 'Please set a depth(levels) limit';
				return $data;
			}
			// Check to see how deep we should go
			if ($args['levels'] == '0') {
				$level = NULL;
			} else {
				$level = $args['levels'];
			}

			$type_codes  = Array();
			$link_types  = Array();
			$link_values = Array();

			// lets check for the type_codes, link_types, and link_values
			if(isset($args['type_codes']) && !empty($args['type_codes'])) {
				$type_codes = is_array($args['type_codes']) ? $args['type_codes'] : explode('\\,', $args['type_codes']);
			}
			if(isset($args['link_types']) && !empty($args['link_types'])) {
				$link_types = is_array($args['link_types']) ? $args['link_types'] : explode('\\,', $args['link_types']);
			}
			if(isset($args['link_values']) && !empty($args['link_values'])) {
				$link_values = is_array($args['link_values']) ? $args['link_values'] : explode('\\,', $args['link_values']);
			}
			$get_attributes	= isset($args['get_attributes']) ? $args['get_attributes'] : FALSE;

			$data = $this->getParents($asset->id, $level, $type_codes, $link_types, $link_values, $get_attributes);
		} else {
			$data['error'] = 'You do not have permissions to access this asset';
		}//end else

		return $data;

	}// end _getParents


	/**
	* returns the defined user for the particular permission for the passed asset
	*
	* @param array		$args			bundled array of arguments do be used while performing operations
	* @param object		$asset			the asset we are executing this function on
	* @param boolean	$read_access	read access check for the asset we
	*									are dealing with with respect to current user
	* @param boolean	$write_access	write access check for the asset we
	*									are dealing with with respect to current user
	*
	* @return array
	* @access private
	*/
	private function _getPermissions($args=Array(), $asset=NULL, $read_access=FALSE, $write_access=FALSE)
	{
		if ($read_access) {
			$level = $args['level'];
			if (empty($level)) {
				$data['error'] = 'Please set a permissions level';
				return $data;
			}
			if (!is_numeric($level)) {
				$data['error'] = 'Permissions level must be numeric';
				return $data;
			}
			if ($level > 3) {
				$data['error'] = $level.' is not a valid permissions level';
				return $data;
			}

			$data = $this->getPermission($asset->id, $level);
			if (!$data) return FALSE;

			return $data;
		} else {
			$data['error'] = 'You do not have permissions to access this asset';
		}//end else

		return $data;

	}// end _getPermissions


	/**
	* creates a new asset with the passed in information
	*
	* @param array		$args			bundled array of arguments do be used while performing operations
	* @param object		$asset			the asset we are executing this function on
	* @param boolean	$read_access	read access check for the asset we
	*									are dealing with with respect to current user
	* @param boolean	$write_access	write access check for the asset we
	*									are dealing with with respect to current user
	*
	* @return array
	* @access private
	*/
	private function _createAsset($args=Array(), $asset=NULL, $read_access=FALSE, $write_access=FALSE)
	{
		if ($write_access || $this->attr('ignore_permissions')) {
			// These must be set, so we don't need to check these
			if (isset($args['type_code']) && isset($args['asset_name'])) {
				$type_code			= isset($args['type_code']) ? $args['type_code'] : '';
				$asset_name			= isset($args['asset_name']) ? $args['asset_name'] : '';
				$link_type			= isset($args['link_type']) ? $args['link_type'] : 1;
				$link_value			= isset($args['link_value']) ? $args['link_value'] : '';
				$sort_order			= isset($args['sort_order']) ? $args['sort_order'] : '';
				$is_dependant		= isset($args['is_dependant']) ? $args['is_dependant'] : 0;
				$is_exclusive		= isset($args['is_exclusive']) ? $args['is_exclusive'] : 0;
				$extra_attributes	= (isset($args['extra_attributes']) && $args['extra_attributes'] == '1') ? TRUE : FALSE;

				// Create our asset
				$GLOBALS['SQ_SYSTEM']->setRunLevel(SQ_RUN_LEVEL_FORCED|SQ_SECURITY_LINK_INTEGRITY);
					$data = $this->createAsset($asset->id, $type_code, $asset_name, $link_type, $link_value, $sort_order, $is_dependant, $is_exclusive, $extra_attributes, $args);
				$GLOBALS['SQ_SYSTEM']->restoreRunLevel();
			} else {
				$data['error'] = 'You must set the parent id, type code and asset name to create an asset';
			}
		} else {
			$data['error'] = 'You do not have permissions to create an asset';
		}//end else

		return $data;

	}// end _createAsset


	/**
	* gets the lock info about the asset passed in with respect to the screen we want
	*
	* @param array		$args			bundled array of arguments do be used while performing operations
	* @param object		$asset			the asset we are executing this function on
	* @param boolean	$read_access	read access check for the asset we
	*									are dealing with with respect to current user
	* @param boolean	$write_access	write access check for the asset we
	*									are dealing with with respect to current user
	*
	* @return array
	* @access private
	*/
	private function _getLocksInfo($args=Array(), $asset=NULL, $read_access=FALSE, $write_access=FALSE)
	{
		if ($read_access) {
			$data = Array();
			$screen = $args['screen'];
			$response = $this->getLocksInfo($asset->id, $screen);
			if (empty($response)) {
				$data[] = 'No lock are held by anyone on Asset "'.$asset->name.'" (#'.$asset->id.')';
			} else {
				require_once SQ_FUDGE_PATH.'/general/datetime.inc';
				foreach ($response as $screen_lock => $lock_info) {
					if ($screen_lock == '0') $screen_lock = $screen;
					$user = $GLOBALS['SQ_SYSTEM']->am->getAsset($lock_info['userid']);
					$expires_in = easy_time_total(($lock_info['expires'] - time()), TRUE);
					if (!$expires_in) $expires_in = '1 second';
					$data[] = '"'.$screen_lock.'" locks are held by User "'.$user->name.'" (#'.$lock_info['userid'].') for Asset "'.$asset->name.'" (#'.$asset->id.'). This is due to expire in '.$expires_in;
				}
			}// end if
		} else {
			$data['error'] = 'You do not have permission to access this asset';
		}//end else

		return $data;

	}// end _getLocksInfo


	/**
	* acquires lock on the specified asset for the screen mentioned.
	* forcibly if flag is set
	* if the locks are already acquired then they are updated and extended
	*
	* @param array		$args			bundled array of arguments do be used while performing operations
	* @param object		$asset			the asset we are executing this function on
	* @param boolean	$read_access	read access check for the asset we
	*									are dealing with with respect to current user
	* @param boolean	$write_access	write access check for the asset we
	*									are dealing with with respect to current user
	*
	* @return array
	* @access private
	*/
	private function _acquireLock($args=Array(), $asset=NULL, $read_access=FALSE, $write_access=FALSE)
	{
		if ($write_access) {
			if (isset($args['screen']) && !empty($args['screen'])) {
				$screen = $args['screen'];
			} else if (isset($args['screen_name']) && !empty($args['screen_name'])) {
				$screen = $args['screen_name'];
			} else {
				$screen = 'all';
			}
			$dependants_only = isset($args['dependants_only']) ? $args['dependants_only'] : TRUE ;
			$force_acquire = isset($args['force_acquire']) ? $args['force_acquire'] : FALSE;

			$response = $this->acquireLocks($asset, $screen, $dependants_only, $force_acquire);
			if (empty($response)) {
				$data[] = '"'.$screen.'" locks are now acquired for Asset "'.$asset->name.'" (#'.$asset->id.')';
			} else {
				$data = $response;
			}// end if
		} else {
			$data['error'] = 'You do not have permission to access this asset';
		}//end else

		return $data;

	}// end _acquireLock


	/**
	* releases the locks for mentioned screen for the given asset
	*
	* @param array		$args			bundled array of arguments do be used while performing operations
	* @param object		$asset			the asset we are executing this function on
	* @param boolean	$read_access	read access check for the asset we
	*									are dealing with with respect to current user
	* @param boolean	$write_access	write access check for the asset we
	*									are dealing with with respect to current user
	*
	* @return array
	* @access private
	*/
	private function _releaseLock($args=Array(), $asset=NULL, $read_access=FALSE, $write_access=FALSE)
	{
		$screen = $args['screen'];
		$response = $this->getLocksInfo($asset->id, $screen);

		// we might not have write access but this might be caused because
		// the status of the asset has changed (say from UC to pending approval)
		// but still we should be able to release locks if we hold any
		if ($write_access || (!empty($response) && $asset->writeAccess('', Array(), FALSE))) {
			if (empty($response)) {
				$data[] =  'No lock are held by anyone on Asset "'.$asset->name.'" (#'.$asset->id.')';
			} else {
				$response = $GLOBALS['SQ_SYSTEM']->am->releaseLock($asset->id, $screen);
				if ($response) {
					$data[] ='"'.$screen.'" locks are now released for Asset "'.$asset->name.'" (#'.$asset->id.')';
				} else {
					$data['error'] = 'You cannot release locks on this asset';
				}// end if
			}//end else
		} else {
			$data['error'] = 'You do not have permission to access this asset';
		}//end else

		return $data;

	}// end _releaseLock


	/**
	* create a new link of the passed asset to the new parent provided with the options given
	*
	* @param array		$args			bundled array of arguments do be used while performing operations
	* @param object		$asset			the asset we are executing this function on
	* @param boolean	$read_access	read access check for the asset we
	*									are dealing with with respect to current user
	* @param boolean	$write_access	write access check for the asset we
	*									are dealing with with respect to current user
	*
	* @return array
	* @access private
	*
	* @see createAssociation
	*/
	private function _createLink($args=Array(), $asset=NULL, $read_access=FALSE, $write_access=FALSE)
	{
		if ($write_access) {
			if (isset($args['parent_id'])) {
				// Check to see if we are using an id or a URL
				$parent = $this->setAsset($args['parent_id']);
				if (!$parent) return FALSE;
			} else {
				$data['error'] = 'A parent asset is required';
				return FALSE;
			}

			$link_type		= isset($args['link_type']) ? $args['link_type'] :  1;
			$link_value		= isset($args['link_value']) ? $args['link_value'] : '';
			$sort_order		= isset($args['sort_order']) ? $args['sort_order'] : '';
			$is_dependant	= isset($args['is_dependant']) ? $args['is_dependant'] : 0;
			$is_exclusive	= isset($args['is_exclusive']) ? $args['is_exclusive'] : 0;

			$data = $this->createAssociation($parent->id, $asset->id, $link_type, $link_value, $sort_order, $is_dependant, $is_exclusive);
		} else {
			$data['error'] = 'You do not have permission to access this asset';
		}//end else

		return $data;

	}// end _createLink


	/**
	* removes the link between the 2 assets specified which matches with the provided details
	*
	* @param array		$args			bundled array of arguments do be used while performing operations
	* @param object		$asset			the asset we are executing this function on
	* @param boolean	$read_access	read access check for the asset we
	*									are dealing with with respect to current user
	* @param boolean	$write_access	write access check for the asset we
	*									are dealing with with respect to current user
	*
	* @return array
	* @access private
	* see this::removeLink
	*/
	private function _removeLink($args=Array(), $asset=NULL, $read_access=FALSE, $write_access=FALSE)
	{
		if ($write_access) {
			if (isset($args['parent_id'])) {
				// Check to see if we are using an id or a URL
				$parent = $this->setAsset($args['parent_id']);
				if (!$parent) return FALSE;
			} else {
				$data['error'] = 'A parent asset is required';
				return $data;
			}
			$link_type = defined($args['link_type']) ? constant($args['link_type']) : SQ_LINK_TYPE_1;
			$link_value = $args['link_value'];

			// Remove our link
			$data = $this->removeLink($parent->id, $asset->id, $link_type, $link_value);
			if (!$data) return FALSE;
		} else {
			$data['error'] = 'You do not have permission to access this asset';
		}//end else

		return $data;

	}// end _removeLink


	/**
	* moves links between 2 assets. Also provides option to change link_typ, link_value on move
	*
	* @param object		$asset			the asset we are executing this function on
	* @param boolean	$read_access	read access check for the asset we
	*									are dealing with with respect to current user
	* @param boolean	$write_access	write access check for the asset we
	*									are dealing with with respect to current user
	* @param array		$args			bundled array of arguments do be used while performing operations
	*
	* @return array
	* @access private
	*/
	private function _moveLink($args=Array(), $asset=NULL, $read_access=FALSE, $write_access=FALSE)
	{
		if ($write_access) {
			if (!empty($args['old_parent_id']) && !empty($args['new_parent_id'])) {
				// Check to see if we are using an id or a URL
				$old_parent = $this->setAsset($args['old_parent_id']);
				$new_parent = $this->setAsset($args['new_parent_id']);
				if (is_null($old_parent) || is_null($new_parent)) {
					$data['error'] = 'Either Old Parent or the New Parent was not a valid assetid.';
					return $data;
				}
			} else {
				$data['error'] = 'A parent asset is required';
				return $data;
			}//end else

			$old_link_type	= defined($args['old_link_type']) ? constant($args['old_link_type']) : SQ_LINK_TYPE_1 ;
			$old_link_value	= $args['old_link_value'];
			$new_link_type	= defined($args['new_link_type']) ? constant($args['new_link_type']) : SQ_LINK_TYPE_1 ;
			$new_link_value	= $args['new_link_value'];
			$new_position	= isset($args['new_position']) ? $args['new_position'] : -1;

			// Remove our link
			$data = $this->moveLink($old_parent->id, $asset->id, $old_link_type, $old_link_value, $new_parent->id, $new_link_type, $new_link_value, $new_position);

			if (!$data) return FALSE;
		} else {
			$data['error'] = 'You do not have permission to access this asset';
		}//end else

		return $data;

	}// end _moveLink


	/**
	* updates a link between 2 assets for the linktype/ link_value/ sort_order/ locked option provided:
	*
	* @param array		$args			bundled array of arguments do be used while performing operations
	* @param object		$asset			the asset we are executing this function on
	* @param boolean	$read_access	read access check for the asset we
	*									are dealing with with respect to current user
	* @param boolean	$write_access	write access check for the asset we
	*									are dealing with with respect to current user
	*
	* @return array
	* @access private
	*/
	private function _updateLink($args=Array(), $asset=NULL, $read_access=FALSE, $write_access=FALSE)
	{
		if ($write_access) {
			if (isset($args['parent_id'])) {
				// Check to see if we are using an id or a URL
				$parent = $this->setAsset($args['parent_id']);
				if (!$parent) return FALSE;
			} else {
				$data['error'] = 'A parent asset is required';
				return $data;
			}

			$old_link_type	= defined($args['existing_link_type']) ? constant($args['existing_link_type']) : SQ_LINK_TYPE_1 ;
			$old_link_value	= empty($args['existing_link_value']) ? NULL : $args['existing_link_value'];
			$new_link_type	= defined($args['link_type']) ? constant($args['link_type']) : SQ_LINK_TYPE_1 ;
			$new_link_value	= $args['link_value'];
			$sort_order		= (isset($args['sort_order']) && $args['sort_order'] != '' ) ?$args['sort_order'] : NULL;
			$locked			= (isset($args['locked']) && $args['locked'] != '' ) ? $args['locked'] : NULL;

			// Remove our link
			$data = $this->updateLink($parent->id, $asset->id, $old_link_type, $old_link_value, $new_link_type, $new_link_value, $sort_order, $locked);

			if (!$data) return FALSE;
		} else {
			$data['error'] = 'You do not have permission to access this asset';
		}//end else

		return $data;

	}// end _updateLink


	/**
	* see this::getLinkId
	*
	* @param array		$args			bundled array of arguments do be used while performing operations
	* @param object		$asset			the asset we are executing this function on
	* @param boolean	$read_access	read access check for the asset we
	*									are dealing with with respect to current user
	* @param boolean	$write_access	write access check for the asset we
	*									are dealing with with respect to current user
	*
	* @return array
	* @access private
	*/
	private function _getLinkId($args=Array(), $asset=NULL, $read_access=FALSE, $write_access=FALSE)
	{
		if ($read_access) {
			if (isset($args['parent_id'])) {
				// Check to see if we are using an id or a URL
				$parent = $this->setAsset($args['parent_id']);
				if (!$parent) return FALSE;
			} else {
				$data['error'] = 'A parent asset is required';
				return $data;
			}
			$link_type = defined($args['link_type']) ? constant($args['link_type']) : SQ_LINK_TYPE_1 ;
			$link_value = $args['link_value'];
			$all_info = (boolean)$args['all_info'];

			// Get our link id
			$data = $this->getLinkId($parent->id, $asset->id, $link_type, $link_value, $all_info);

			if (!$data) {
				$data['error'] = 'Parent Asset #'.$parent->id.' and Child Asset #'.$asset->id.' do not have a valid link for given Link Type and Link Value';
			}
		} else {
			$data['error'] = 'You do not have permissions to access this asset';
		}//end if

		return $data;

	}// end _getLinkId


	/**
	* get the asset tree of the passed asset
	*
	* @param array		$args			bundled array of arguments do be used while performing operations
	* @param object		$asset			the asset we are executing this function on
	* @param boolean	$read_access	read access check for the asset we
	*									are dealing with with respect to current user
	* @param boolean	$write_access	write access check for the asset we
	*									are dealing with with respect to current user
	*
	* @return array
	* @access private
	*/
	private function _getAssetTree($args=Array(), $asset=NULL, $read_access=FALSE, $write_access=FALSE)
	{
		if ($read_access) {
			// Check to see how deep we should go
			if (empty($args['levels'])) {
				$levels = NULL;
			} else {
				$levels = $args['levels'];
			}//end if

			$data = $GLOBALS['SQ_SYSTEM']->am->getAssetTree($asset->id, $levels);

			if (empty($data)) $data['error'] = 'Asset #'.$asset->id.' has no children';
		} else {
			$data['error'] = 'You do not have permissions to access this asset';
		}//end else

		return $data;

	}// end _getAssetTree


	/**
	* get the replacements for the passed keywords against the current asset
	*
	* @param array		$args			bundled array of arguments do be used while performing operations
	* @param object		$asset			the asset we are executing this function on
	* @param boolean	$read_access	read access check for the asset we
	*									are dealing with with respect to current user
	* @param boolean	$write_access	write access check for the asset we
	*									are dealing with with respect to current user
	*
	* @return array
	* @access private
	*/
	private function _getKeywordsReplacements($args=Array(), $asset=NULL, $read_access=FALSE, $write_access=FALSE)
	{
		if (!isset($arg['keywords']) && isset($args['keywords_array'])) {
			// if this IF statement is true that means the batchRequest() is used to call
			// getKeywordReplacement(). Make sure the keywords index expected is always populated.
			$args['keywords'] = $args['keywords_array'];
		}

		if ($read_access) {
			if (isset($args['keywords']) && !empty ($args['keywords']) ){
				$keywords = is_array($args['keywords']) ? $args['keywords'] : explode('\\,', $args['keywords']);
				$data = $this->getKeywordsReplacements($asset, $keywords);
			} else {
				$data['error'] = 'Please provide keywords to get replacement for';
			}
		} else {
			$data['error'] = 'You do not have permissions to access this asset';
		}//end else

		return $data;

	}// end _getKeywordsReplacements


	/**
	* sets asset to the passed status for the current asset
	*
	* @param array		$args			bundled array of arguments do be used while performing operations
	* @param object		$asset			the asset we are executing this function on
	* @param boolean	$read_access	read access check for the asset we
	*									are dealing with with respect to current user
	* @param boolean	$write_access	write access check for the asset we
	*									are dealing with with respect to current user
	*
	* @return array
	* @access private
	*/
	private function _setAssetStatus($args=Array(), $asset=NULL, $read_access=FALSE, $write_access=FALSE)
	{
		if ($write_access) {
			$status				= (int) $args['status'];
			$cascade			= isset($args['cascade']) ? $args['cascade'] : FALSE;
			$workflow_stream	= (isset($args['workflow_stream']) && ($args['workflow_stream'] != '') )? $args['workflow_stream'] : 'SQ_USE_DEFAULT';

			//let check if the status supplied is valid one
			$desc = get_status_description($status);
			$current_desc = ($status != $asset->status) ? get_status_description($asset->status) : $desc;

			$response = $this->setAssetStatus($asset, $status, $cascade, $current_desc, $desc, $workflow_stream);
			if (empty($response)) {
				if (!$cascade) {
					$data[] = 'Status for Asset "'.$asset->name.'" (#'.$asset->id.') has been changed successfully to '.$desc;
				} else {
					$data[] = 'Status for Asset "'.$asset->name.'" (#'.$asset->id.') and its non-dependant children has been changed successfully to '.$desc;
				}
			} else {
				$data = $response;
			}
		} else {
			$data['error'] = 'You do not have permissions to access this change status for this asset';
		}//end else

		return $data;

	}// end _setAssetStatus


	/**
	* gets all the webpaths for the asset passed
	*
	* @param array		$args			bundled array of arguments do be used while performing operations
	* @param object		$asset			the asset we are executing this function on
	* @param boolean	$read_access	read access check for the asset we
	*									are dealing with with respect to current user
	* @param boolean	$write_access	write access check for the asset we
	*									are dealing with with respect to current user
	*
	* @return array
	* @access private
	*/
	private function _getWebPath($args=Array(), $asset=NULL, $read_access=FALSE, $write_access=FALSE)
	{
		if($read_access) {
			$webpaths = $asset->getWebPaths();
			if(!empty($webpaths)) {
				$i = 0;
				foreach($webpaths as $webpath) {
					$data['webpath'][$i] = $webpath;
					$i++;
				}
			} else {
				$data ['webpath'] = Array();
			}
		} else {
			$data['error'] = 'You do not have permission to access this asset';
		}

		return $data;

	}// end _getWebPath


	/**
	* removes the current webpath to the passed asset and set the supplied one
	* optionally lets the decide user to set the remap too. wow!
	*
	* @param array		$args			bundled array of arguments do be used while performing operations
	* @param object		$asset			the asset we are executing this function on
	* @param boolean	$read_access	read access check for the asset we
	*									are dealing with with respect to current user
	* @param boolean	$write_access	write access check for the asset we
	*									are dealing with with respect to current user
	*
	* @return array
	* @access private
	*/
	private function _setWebPath($args=Array(), $asset=NULL, $read_access=FALSE, $write_access=FALSE)
	{
		if($write_access) {
			$webpaths = isset($args['paths']) && is_array($args['paths']) ? $args['paths'] : explode('\\,', $args['webpath']);
			if (!empty($webpaths)) {
				$auto_remap = (isset($args['auto_remap']) && $args['auto_remap'] != '') ? $args['auto_remap'] : TRUE;
				$response = $asset->saveWebpaths($webpaths, $auto_remap);

				if($response) {
					$this->updateLookupsforAsset($asset->id);
					$data[] = 'Webpath for Asset "'.$asset->name.'" (# '.$asset->id.') had been updated sucessfully';
				} else {
					$data[] = 'Could not update webpath for Asset "'.$asset->name.'" (# '.$asset->id.') ';
				}
			} else {
				$data['error'] = 'No webpaths provided';
			}
		} else {
			$data['error'] = 'You do not have permission to access this asset';
		}

		return $data;

	}// end _setWebPath


	/**
	* get the applied/denied/running information about the workflow schema on the passed asset
	*
	* @param array		$args			bundled array of arguments do be used while performing operations
	* @param object		$asset			the asset we are executing this function on
	* @param boolean	$read_access	read access check for the asset we
	*									are dealing with with respect to current user
	* @param boolean	$write_access	write access check for the asset we
	*									are dealing with with respect to current user
	*
	* @return array
	* @access private
	*/
	private function _getWorkflowSchema($args=Array(), $asset=NULL, $read_access=FALSE, $write_access=FALSE)
	{
		if($read_access) {
			$wfm = $GLOBALS['SQ_SYSTEM']->getWorkflowManager();
			$granted = isset($args['granted']) && $args['granted'] != '' ? $args['granted'] : NULL;
			$running = isset($args['running']) && $args['running'] != '' ? $args['running'] : FALSE;
			$schemas = $wfm->getSchemas($asset->id, $granted, $running);

			if(empty($schemas)) {
				$message = 'No Workflow Schema ';
				if ($running) {
					$message .= 'currently running for ';
				} else {
					if (is_null($granted)) {
						$message .= 'currently applied to';
					} else {
						$message .= $granted ? 'granted on ' : 'denied on ';
					}
				}
				$data[] = $message.'Asset "'.$asset->name.'" (# '.$asset->id.')';
			} else {
				foreach ($schemas as $index => $info) {
					if ($running && $granted) {
						$data['running'][] = $info;
					} else if (is_null($granted) && $running) {
						$data['running'][] = $index;
					} else {
						if (is_null($granted)) {
							$status = $info ? 'granted' : 'denied';
							$data[$status][] = $index;
						} else {
							$status = $granted ? 'granted' : 'denied';
							$data[$status][] = $info;
						}
					}
				}
			}
		} else {
			$data['error'] = 'You do not have permission to access this asset';
		}

		return $data;

	}// end _getWorkflowSchema


	/**
	* server side function to create a file asset.
	* note : it will create an asset container for the file type specified. File needs to be uploaded
	*		 once the asset is created.
	*
	* @param array		$args			bundled array of arguments do be used while performing operations
	* @param object		$asset			the asset we are executing this function on
	* @param boolean	$read_access	read access check for the asset we
	*									are dealing with with respect to current user
	* @param boolean	$write_access	write access check for the asset we
	*									are dealing with with respect to current user
	*
	* @return array
	* @access private
	*/
	private function _createFileAsset($args=Array(), $asset=NULL, $read_access=FALSE, $write_access=FALSE)
	{
		if ($write_access || $this->attr('ignore_permissions')) {

			if ((isset($args['friendly_name']) && $args['friendly_name'] != '' )) {
				$filename		= $args['friendly_name'];
				$type_code		= $args['type_code'];
				$link_type		= defined($args['link_type']) ? constant($args['link_type']) : SQ_LINK_TYPE_1;
				$link_value		= $args['link_value'];
				$valid_type_code= $GLOBALS['SQ_SYSTEM']->am->getTypeDescendants('file', TRUE);

				if(!in_array(Array($type_code), $valid_type_code)) {

					$GLOBALS['SQ_SYSTEM']->am->includeAsset($type_code);
					$new_asset = new $type_code();

					require_once SQ_FUDGE_PATH.'/general/file_system.inc';
					$file_ext = get_file_type($filename);

					// bug fix #5749 JS API createFileAsset warns of file extension on generic file asset
					// if we are creaed generic File asset the allowed_extensions property is empty for
					// this get all the allowed file extensions by matrix and let the user go nuts !!!
					if (strtolower($type_code) == 'file' || strtolower($type_code) == 'text_file') {
						// get our standard list of MIME type allowed
						include SQ_FUDGE_PATH.'/standards_lists/mime_types.inc';
						$allowed_extensions = array_keys($standards_lists_mime_types);
					} else {
						$allowed_extensions = $new_asset->allowed_extensions;
					}

					if (!in_array($file_ext, $allowed_extensions)) {
						return Array('error' => 'The supplied file extension "'.$file_ext.'" is not allowed for file type "'.$type_code.'"');
					}
					//We are going to write to the Matrix Data tmp dir
					$tmp_file_name  = $this->getRandomFilename($filename);
					$destination_file   = SQ_DATA_PATH.'/temp/'.$tmp_file_name.'.'.$file_ext;

					while (file_exists($destination_file)) {
						$tmp_file_name  = $this->getRandomFilename($filename);
						$destination_file   = SQ_DATA_PATH.'/temp/'.$tmp_file_name.'.'.$file_ext;
					}//end while

					//create an file with empty content, coz we are getting rid of it anyways
					if ($type_code != 'image' && $type_code != 'thumbnail') {
						file_put_contents($destination_file, '');
					} else {
						// is a image do it differently
						$img = imagecreate('10','10');
						$background_color = imagecolorallocate($img, 0, 0, 0);
						imagepng($img, $destination_file);
					}

					// construct the link array
					$link = Array (
								'asset'		=> $asset,
								'link_type'	=> $link_type,
								'value'		=> $link_value,
								'sort_order'=> -1,
							);

					$file_info = Array (
									'name'				=> $filename,
									'tmp_name'			=> $destination_file,
									'type'				=> $file_ext,
									'error'				=> 0,
									'size'				=> filesize($destination_file),
									'filename'			=> $filename,
									'non_uploaded_file' => TRUE,
								 );

					//fake it
					$new_asset->_tmp['uploading_file'] = TRUE;
					if ($type_code == 'data_source_graph') $new_asset->setAttrValue('title', $filename);

					$success = $new_asset->create($link, $file_info);

					if (!$success) {
						$data['error'] = 'Error occured while creating new asset of type \''.$type_code.'\'';
					} else {
						// done creating the asset ? let remove the temp file we had uploaded and make way for new one
						$fv = $GLOBALS['SQ_SYSTEM']->getFileVersioning();
						$existing = $new_asset->getExistingFile();

						$new_asset_edit_fns = $new_asset->getEditFns();
						$new_asset_edit_fns->removeOldFile($new_asset, $existing, $fv);
						$data[$new_asset->id] = 'New File Asset (#'.$new_asset->id.') \''.$new_asset->name.'\'of type_code \''.$type_code.'\' created successfully';
					}
				} else {
					$data['error'] = 'Passed type code is not File Type asset or its Descendant Type';
				}
			} else {
				$data['error'] = 'You must set the Name of the Asset and provide a Base 64 encoded file to upload';
			}
		} else {
			$data['error'] = 'You do not have permissions to create an asset';
		}

		return $data;

	}// end _createFileAsset


	/**
	* updates the file content of the editable file types viz: xml_file, xsl_file, css_file, text_file and js_file
	*
	* @param array		$args			bundled array of arguments do be used while performing operations
	* @param object		$asset			the asset we are executing this function on
	* @param boolean	$read_access	read access check for the asset we
	*									are dealing with with respect to current user
	* @param boolean	$write_access	write access check for the asset we
	*									are dealing with with respect to current user
	*
	* @return array
	* @access private
	*/
	private function _setContentOfEditableFileAsset($args=Array(), $asset=NULL, $read_access=FALSE, $write_access=FALSE)
	{
		if ($write_access) {
			$valid_file_type = Array('xml_file', 'xsl_file', 'css_file', 'text_file', 'js_file');
			if(in_array($asset->type(), $valid_file_type)) {
				if($asset->writeAccess('attributes')) {
					$content = urldecode($args['content']);
					if($content == 'no_value_provided') {
						$data[] = 'No file content provided';
					} else {
						require_once(SQ_INCLUDE_PATH.'/backend_outputter.inc');
						$o			= new Backend_Outputter();
						$prefix 	= $asset->getPrefix();
						$edit_fns 	= $asset->getEditFns();
						$_POST[$prefix.'_new_file'] = urldecode($content);

						$success 	= $edit_fns->processEditFile($asset, $o, $prefix);
						if ($success) {
							$data['success'] = 'Content of Asset '.$asset->short_name.'(#'.$asset->id.') successfully updated';
						} else {
							$data['error'] = 'Error occured while updating Content of Asset '.$asset->short_name.'(#'.$asset->id.')';
						}
					}
				} else {
					$data['error'] = 'Locks are not acquired on Asset '.$asset->short_name.'(#'.$asset->id.'), content cannot be updated';
				}
			} else {
				$data['error'] = 'Content for Asset of type code \''.$asset->type().'\' cannot be updated';
			}
		} else {
			$data['error'] = 'You do not have permissions to Edit this asset';
		}

		return $data;

	}// end _setContentOfEditableFileAsset


	/**
	* get the lineage of the passed asset
	*
	* @param array		$args			bundled array of arguments do be used while performing operations
	* @param object		$asset			the asset we are executing this function on
	* @param boolean	$read_access	read access check for the asset we
	*									are dealing with with respect to current user
	* @param boolean	$write_access	write access check for the asset we
	*									are dealing with with respect to current user
	*
	* @return array
	* @access private
	*/
	private function _getLineage($args=Array(), $asset=NULL, $read_access=FALSE, $write_access=FALSE)
	{
		// we have the url in the args, lets get it and get the asset are dealing with
		$url = trim(array_get_index($args, 'asset_url', ''));
		if (empty($url)) {
			return Array(
					'error' => 'No URL supplied'
					);
		}
		$significant_link_only = array_get_index($args, 'significant_link_only', TRUE);

		// cant get lineage of root_folder, DUHH !
		if ($url == '1') return Array();

		// now if we have url we have a ready made function in asset_manager.inc we can use
		// although we manipulate the results as it not the elements/format we want
		$url_bits = parse_url($url);
		if (!empty($url_bits['scheme']) && !empty($url_bits['host'])) {
			$asset = $GLOBALS['SQ_SYSTEM']->am->getAssetFromURL($url_bits['scheme'], $url_bits['host'].$url_bits['path']);
			if (is_null($asset)) {
				return Array(
						'error' => 'URL supplied does not point to asset in Matrix'
						);
			}
		} else if (preg_match('|^[0-9]+(?:\:.+)?|', $url)) {
			if (!$GLOBALS['SQ_SYSTEM']->am->getAssetInfo($url)) {
				return Array(
						'error' => 'Supplied assetid #'.$url.' does not exist'
						);
			}
			$asset = $GLOBALS['SQ_SYSTEM']->am->getAsset($url);
		} else {
			return Array(
					'error' => 'URL supplied is invalid'
					);
		}

		if(!$asset->readAccess()) return Array('error' => 'You do not have permissions to access this asset');

		$raw_lineages = $GLOBALS['SQ_SYSTEM']->am->getLinkLineages($asset->id, 0, NULL, Array('name', 'type_code'), $significant_link_only);
		$root_folder = $GLOBALS['SQ_SYSTEM']->am->getSystemAsset('root_folder');

		$result_lineages = Array();
		$linkid_done = Array();
		$link_types = $significant_link_only ? SQ_SC_LINK_SIGNIFICANT : SQ_SC_LINK_ALL;

		foreach ($raw_lineages as $index => $val) {
			$data = Array();
			$minorid = $asset->id;
			$parent_lineage_assetids = array_merge(array_reverse(array_keys($val['lineage'])), Array($root_folder->id));
			foreach($parent_lineage_assetids as $majorid) {
				$major_asset = $GLOBALS['SQ_SYSTEM']->am->getAsset($majorid);
				$minor_asset = $GLOBALS['SQ_SYSTEM']->am->getAsset($minorid);

				$links = $GLOBALS['SQ_SYSTEM']->am->getLinkByAsset($major_asset->id, $minor_asset->id, $link_types, NULL, 'major', TRUE);
				foreach($links as $link) {
					if (!in_array($link['linkid'], $linkid_done)) {
						if ($link['link_type'] & SQ_SC_LINK_SIGNIFICANT) {
							if (strpos($link['linkid'], ':') === FALSE) {
								$treeid_data = $GLOBALS['SQ_SYSTEM']->am->getLinkTreeid($link['linkid']);
							} else {
								// No tree ID for shadow links (yet).
								$treeids = $GLOBALS['SQ_SYSTEM']->am->getAssetTreeids($major_asset->id);
								$treeid_data = Array($link['linkid'] => Array(Array()));
								foreach ($treeids as $treeid) {
									$treeid_data[$link['linkid']][0][] = $treeid.str_repeat('*', SQ_CONF_ASSET_TREE_SIZE);
								}
							}
						} else {
							$treeid_data = Array($link['linkid'] => Array(Array(FALSE)));
						}
						foreach($treeid_data[$link['linkid']] as $treeid) {
							$info = Array();
							$info['tree_id']			= $treeid[0];
							$info['assetid']			= $minor_asset->id;
							$info['name']				= $minor_asset->name;
							$info['type_code']			= $minor_asset->type();
							$info['link_type']			= $link['link_type'];
							$info['link_value']			= $link['value'];
							$info['linkid']				= $link['linkid'];
							$info['parent_assetid']		= $major_asset->id;
							$info['parent_type_code']	= $major_asset->type();

							$data[] = $info;
						}
						$linkid_done[] = $link['linkid'];
					}
				}//end foreach

				// Get the link info for the next lineage, one level up, if any
				$GLOBALS['SQ_SYSTEM']->am->forgetAsset($minorid);
				$minorid = $majorid;
			}//end foreach

			$result_lineages = array_merge($result_lineages, array_reverse($data));
		}//end foreach

		$GLOBALS['SQ_SYSTEM']->am->forgetAsset($root_folder);

		return $result_lineages;

	}// end _getLineage()


	/**
	* get a single lineage of the passed url
	*
	* @param array		$args			bundled array of arguments do be used while performing operations
	* @param object		$asset			the asset we are executing this function on
	* @param boolean	$read_access	read access check for the asset we
	*									are dealing with with respect to current user
	* @param boolean	$write_access	write access check for the asset we
	*									are dealing with with respect to current user
	*
	* @return array
	* @access private
	*/
	private function _getLineageFromUrl($args=Array(), $asset=NULL, $read_access=FALSE, $write_access=FALSE)
	{
		// we have the url in the args, lets get it and get the asset are dealing with
		$url = trim(array_get_index($args, 'asset_url', ''));
		if (empty($url)) return Array('error' => 'No URL supplied');

		$url_bits = parse_url($url);

		$asset = $GLOBALS['SQ_SYSTEM']->am->getAssetFromURL($url_bits['scheme'], $url_bits['host'].$url_bits['path']);

		if (is_null($asset)) return Array('error' => 'URL supplied does not point to asset in Matrix');

		if(!$asset->readAccess()) return Array('error' => 'You do not have permissions to access this asset');


		$result_lineage = $GLOBALS['SQ_SYSTEM']->am->getLineageFromURL($url_bits['scheme'], $url_bits['host'].$url_bits['path']);
		if(empty($result_lineage)) return Array('error' => 'No lineage found');

		return $result_lineage;

	}// end _getLineageFromUrl()


	/**
	* get url from passed lineage array (asset ids)
	*
	* @param array		$args			bundled array of arguments do be used while performing operations
	* @param object		$asset			the asset we are executing this function on
	* @param boolean	$read_access	read access check for the asset we
	*									are dealing with with respect to current user
	* @param boolean	$write_access	write access check for the asset we
	*									are dealing with with respect to current user
	*
	* @return array
	* @access private
	*/
	private function _getUrlFromLineage($args=Array(), $asset=NULL, $read_access=FALSE, $write_access=FALSE)
	{

		$lineage = array_get_index($args, 'lineage', NULL);
		$root_url = trim(array_get_index($args, 'root_url', NULL));
		$protocol = trim(array_get_index($args, 'protocol', NULL));

		if(empty($root_url)) $root_url = NULL;
		if (empty($lineage)) return Array('error' => 'No lineage supplied');

		if(!is_array($lineage)) {
			$lineage = explode('\\,', trim($lineage));
		}

		$current_asset_id = trim(array_pop($lineage));
		$current_asset = $this->setAsset($current_asset_id);
		If(!$current_asset) return FALSE;

		if(!$current_asset->readAccess()) return Array('error' => 'You do not have permissions to access this asset');

		$candidate_urls = $GLOBALS['SQ_SYSTEM']->am->getURLs($current_asset_id);
		if(empty($candidate_urls)) return Array('error' => 'No URL found for current asset');

		$lineage[] = $current_asset_id;

		$result = $GLOBALS['SQ_SYSTEM']->am->getUrlFromLineage($lineage, $root_url, $protocol);

		if (empty($result)) return Array('error' => 'No URL found for supplied lineage');

		return $result;

	}// end _getUrlFromLineage()


	/**
	* clone the given asset under a new parent
	*
	* @param array		$args			bundled array of arguments do be used while performing operations
	* @param object		$asset			the asset we are executing this function on
	* @param boolean	$read_access	read access check for the asset we
	*									are dealing with with respect to current user
	* @param boolean	$write_access	write access check for the asset we
	*									are dealing with with respect to current user
	*
	* @return array
	* @access private
	*/
	private function _cloneAsset($args=Array(), $asset=NULL, $read_access=FALSE, $write_access=FALSE)
	{
		if ($write_access || $this->attr('ignore_permissions')) {
			$data = Array();
			$new_parent = array_get_index($args, 'new_parent', '');

			// check if the parent asset is provided
			if (empty($new_parent)) {
				$data['error'] = 'No location to clone the asset was provided';
				return $data;
			}

			// check to see if we have write permission to the new parent
			if (!($new_parent_asset = $GLOBALS['SQ_SYSTEM']->am->getAsset($new_parent, '', TRUE))) {
				$data['error'] = 'Assetid "'.$new_parent.'" provided is invalid or doesn\'t exist on system';
				return $data;
			}

			if (!$new_parent_asset->writeAccess() && !$this->attr('ignore_permissions')) {
				$data['error'] = 'You do not have write access to the Asset under which you are tying to clone';
				return $data;
			}

			$clone_num	= array_get_index($args, 'clone_num', 1);
			$position	= array_get_index($args, 'new_position', -1);
			$value		= array_get_index($args, 'link_value', '');
			$link_type	= defined($args['link_type']) ? constant($args['link_type']) : SQ_LINK_TYPE_1;

			if (!is_null($asset)) {
				$hh	= $GLOBALS['SQ_SYSTEM']->getHipoHerder();
				$running_vars	= Array (
									'assets'			=> Array (
															$asset->id			=> Array (
																					'number_of_clones'	=> 1,
																				   ),
														   ),
									'to_parent_assetid'	=> $new_parent,
									'to_parent_pos'		=> $position,
									'link_type'			=> $link_type,
									'value'				=> $value,
								  );

				$all_errors	= Array();
				$cloned_assets = Array();
				for ($i = 0; $i < $clone_num; $i++) {
					// Need to reset the running vars again as it is passed by reference to the hipo job
					$running_vars_copy	= $running_vars;
					$errors	= $hh->freestyleHipo('hipo_job_clone_assets', $running_vars_copy);
					if(isset($running_vars_copy['cloned_asset'])) {
						$cloned_asset_id = array_pop(array_keys($running_vars_copy['cloned_asset']));
						$cloned_asset_id = '#'.$cloned_asset_id;
						$cloned_assets[] = $cloned_asset_id;
					}
					$all_errors	+= $errors;
				}//end for
				if (empty($all_errors)) {
					$cloned_assets_string = implode($cloned_assets, ', ');
					$data['success'] = $asset->name.'(#'.$asset->id.') has been successfully cloned to '.$cloned_assets_string;
				} else {
					$data['error'] = $all_errors;
				}
			} else {
				$data['error'] = 'Asset to clone was not provided';
			}
		} else {
			$data['error'] = 'You do not have permissions to create an asset';
		}//end else

		return $data;

	}// end _cloneAsset


	/**
	* takes in 2 assets and checks for the difference in their content
	* optionally lets user to use the paint layout to generate the difference
	*
	* @param array		$args			bundled array of arguments do be used while performing operations
	* @param object		$asset			the asset we are executing this function on
	* @param boolean	$read_access	read access check for the asset we
	*									are dealing with with respect to current user
	* @param boolean	$write_access	write access check for the asset we
	*									are dealing with with respect to current user
	*
	* @return array
	* @access private
	*/
	private function _showDifference($args=Array(), $asset=NULL, $read_access=FALSE, $write_access=FALSE)
	{
		$asset_2_id = array_get_index($args, 'assetid_2', '');
		if (empty($asset_2_id)) {
			$data['error'] = 'Only one asset was provided. Need 2 assets to check the difference';
			return $data;
		}

		if (!($asset_2 = $GLOBALS['SQ_SYSTEM']->am->getAsset($asset_2_id, '', TRUE))) {
			$data['error'] = 'Assetid "'.$asset_2_id.'", provided is invalid or doesn\'t exist on system';
			return $data;
		}

		if (($read_access) && $asset_2->readAccess()) {
			$data = Array();
			$paint_layout_1 = array_get_index($args, 'paint_layout_1', '');
			$paint_layout_2 = array_get_index($args, 'paint_layout_2', '');

			if (empty($paint_layout_1) || empty($paint_layout_2)) {
				// we dont care about the paint layout
				// get the content without them
				ob_start();
					$asset->printBody();
					$content1 = ob_get_contents();
				ob_end_clean();

				ob_start();
					$asset_2->printBody();
					$content2 = ob_get_contents();
				ob_end_clean();

			} else {
				// we are considering the paint layout lets see if we are passed the layouts
				// to use or just using the defined layouts
				// now the values for $paint_layout_1 and $paint_layout_2 are either assetids
				// or empty string, both are valid argument we can pass into printBodyWithPaintLayout
				foreach (Array($paint_layout_1, $paint_layout_2) as $paint_layout_assetid) {
					if (!$paint_layout = $GLOBALS['SQ_SYSTEM']->am->getAsset($paint_layout_assetid, 'paint_layout_page', 'TRUE')) {
						$data['error'] = 'Invalid assetid (#'.$paint_layout_assetid.') provided for Paint Layout to use';
					}
					$paint_layout = NULL;
					if (isset($data['error'])) return $data;
				}

				ob_start();
					$asset->printBodyWithPaintLayout($paint_layout_1);
					$content1 = ob_get_contents();
				ob_end_clean();

				ob_start();
					$asset_2->printBodyWithPaintLayout($paint_layout_2);
					$content2 = ob_get_contents();
				ob_end_clean();
			}

			// Show the differences
			require_once SQ_CORE_PACKAGE_PATH.'/designs/design_areas/design_area_body/matrix_diff_highlighter.inc';
			$highlighter = new Matrix_Diff_Highlighter();
			$contents = Array(&$content1, &$content2);
			foreach ($contents as $index => $content) {
				$pattern = '/(<div)([^>]*)( id="content_div_[0-9]*")([^>]*)(>)/';
				$replacement = '$1$2$4$5';
				// we need to remove the div id's as they are the assetid
				// of the bodycopy containers ..this will always be different while comparing 2 assets
				// and this will cause the output to be malformed even if the content was same
				$contents[$index] = preg_replace($pattern, $replacement, $contents[$index]);
				// replace &nbsp; with proper space. this is to prevent html_entity_decode
				// in matrix_diff_highlighter.inc to convert it to a "\xa0"
				// which is break the putput when were are json_encode'ing the results
				// to be sent back to API
				$contents[$index] = str_replace('&nbsp;', ' ', $contents[$index]);
			}
			$difference =  $highlighter->process($content1, $content2, FALSE);
			$data['success'] = $difference;
		} else {
			$data['error'] = 'You do not have permissions to read content of one or either of the assets passed in';
		}//end else

		return $data;

	}// end _showDifference()


	/**
	* Returns the context we should be using based on the
	* settings on the Context Configuration screen
	*
	* @param array		$args			bundled array of arguments do be used while performing operations
	* @param object		$asset			the asset we are executing this function on
	* @param boolean	$read_access	read access check for the asset we
	*									are dealing with with respect to current user
	* @param boolean	$write_access	write access check for the asset we
	*									are dealing with with respect to current user
	*
	* @return array
	* @access private
	*/
	private function _getAlternateContext($args=Array(), $asset=NULL, $read_access=FALSE, $write_access=FALSE)
	{
		$all_info	 = array_get_index($args, 'all_info', FALSE);

		$system_contexts   = $GLOBALS['SQ_SYSTEM']->getAllContexts();
		$current_contextid = $GLOBALS['SQ_SYSTEM']->getAlternateContext();
		$context_info      = Array();

		$currnet_context   = $system_contexts[$current_contextid];
		if ($all_info) {
			$context_info[$current_contextid] = $currnet_context;
		} else {
			$context_info[$current_contextid] = $currnet_context['name'];
		}

		$data['success'] = $context_info;

		return $data;

	}// end _getAlternateContext()


	/**
	* Returns the current context active on the system
	*
	* @param array		$args			bundled array of arguments do be used while performing operations
	* @param object		$asset			the asset we are executing this function on
	* @param boolean	$read_access	read access check for the asset we
	*									are dealing with with respect to current user
	* @param boolean	$write_access	write access check for the asset we
	*									are dealing with with respect to current user
	*
	* @return array
	* @access private
	*/
	private function _getCurrentContext($args=Array(), $asset=NULL, $read_access=FALSE, $write_access=FALSE)
	{
		$all_info	 = array_get_index($args, 'all_info', FALSE);

		// check to se if we can read the contextid from the session first
		if ($this->attr('sync_context') && isset($_SESSION[SQ_SESSION_SANDBOX_INDEX]['SQ_BACKEND_ALTERNATE_CONTEXT_ID'])) {
			$current_contextid = $_SESSION[SQ_SESSION_SANDBOX_INDEX]['SQ_BACKEND_ALTERNATE_CONTEXT_ID'];
			if (!isset($_SESSION[SQ_SESSION_SANDBOX_INDEX]['SQ_ALTERNATE_CONTEXT_ID']) ||
					$_SESSION[SQ_SESSION_SANDBOX_INDEX]['SQ_ALTERNATE_CONTEXT_ID'] != $current_contextid) {
				$_SESSION[SQ_SESSION_SANDBOX_INDEX]['SQ_ALTERNATE_CONTEXT_ID'] = $current_contextid;
			}
		} else if (isset($_SESSION[SQ_SESSION_SANDBOX_INDEX]['SQ_ALTERNATE_CONTEXT_ID'])) {
			$current_contextid = $_SESSION[SQ_SESSION_SANDBOX_INDEX]['SQ_ALTERNATE_CONTEXT_ID'];
		} else {
			$current_contextid = $GLOBALS['SQ_SYSTEM']->getContextId();
		}

		$system_contexts   = $GLOBALS['SQ_SYSTEM']->getAllContexts();
		$context_info      = Array();

		$currnet_context   = $system_contexts[$current_contextid];
		if ($all_info) {
			$context_info[$current_contextid] = $currnet_context;
		} else {
			$context_info[$current_contextid] = $currnet_context['name'];
		}

		$data['success'] = $context_info;

		return $data;

	}// end _getCurrentContext()


	/**
	* Returns details of all the contexts in a system
	*
	* @param array		$args			bundled array of arguments do be used while performing operations
	* @param object		$asset			the asset we are executing this function on
	* @param boolean	$read_access	read access check for the asset we
	*									are dealing with with respect to current user
	* @param boolean	$write_access	write access check for the asset we
	*									are dealing with with respect to current user
	*
	* @return array
	* @access private
	*/
	private function _getAllContexts($args=Array(), $asset=NULL, $read_access=FALSE, $write_access=FALSE)
	{
		$all_info	 = array_get_index($args, 'all_info', FALSE);

		$system_contexts   = $GLOBALS['SQ_SYSTEM']->getAllContexts();
		$context_info      = Array();

		foreach ($system_contexts as $context_id => $context_data) {
			if ($all_info) {
				$context_info[$context_id] = $context_data;
			} else {
				$context_info[$context_id] = $context_data['name'];
			}
		}//end foreach

		$data['success'] = $context_info;

		return $data;

	}// end _getAllContexts()


	/**
	* Set the system context to the id provided
	*
	* @param array		$args			bundled array of arguments do be used while performing operations
	* @param object		$asset			the asset we are executing this function on
	* @param boolean	$read_access	read access check for the asset we
	*									are dealing with with respect to current user
	* @param boolean	$write_access	write access check for the asset we
	*									are dealing with with respect to current user
	*
	* @return array
	* @access private
	*/
	private function _setContext($args=Array(), $asset=NULL, $read_access=FALSE, $write_access=FALSE)
	{
		$context_id = array_get_index($args, 'context_id', NULL);

		if (!is_null($context_id)) {
			$all_contexts = $GLOBALS['SQ_SYSTEM']->getAllContexts();
			if (isset($all_contexts[$context_id])) {
				// valid context id is provided
				$GLOBALS['SQ_SYSTEM']->changeContext($context_id);
				$_SESSION[SQ_SESSION_SANDBOX_INDEX]['SQ_ALTERNATE_CONTEXT_ID'] = $context_id;
				if ($this->attr('sync_context')) {
					$_SESSION[SQ_SESSION_SANDBOX_INDEX]['SQ_BACKEND_ALTERNATE_CONTEXT_ID'] = $context_id;
				}
				$data['success'] = "Context successfully set to '$context_id'.";
			} else {
				$data['error'] = 'Context Id Provided doesn\'t exist on the system.';
			}
		} else {
			$data['error'] = 'No Context Id Provided.';
		}

		return $data;

	}// end _setContext()


	/**
	* restores the previous context
	*
	* @param array		$args			bundled array of arguments do be used while performing operations
	* @param object		$asset			the asset we are executing this function on
	* @param boolean	$read_access	read access check for the asset we
	*									are dealing with with respect to current user
	* @param boolean	$write_access	write access check for the asset we
	*									are dealing with with respect to current user
	*
	* @return array
	* @access private
	*/
	private function _restoreContext($args=Array(), $asset=NULL, $read_access=FALSE, $write_access=FALSE)
	{
		// valid context id is provided
		$restored_context = $GLOBALS['SQ_SYSTEM']->restoreContext(TRUE);
		$_SESSION[SQ_SESSION_SANDBOX_INDEX]['SQ_ALTERNATE_CONTEXT_ID'] = $restored_context;
		if ($this->attr('sync_context')) {
			$_SESSION[SQ_SESSION_SANDBOX_INDEX]['SQ_BACKEND_ALTERNATE_CONTEXT_ID'] = $restored_context;
		}

		$data['success'] = "Context successfully restored back to '$restored_context'.";

		return $data;

	}// end _restoreContext()


	/**
	* Set the metadata schema info applied to the given assetid
	*
	* @param array		$args			bundled array of arguments do be used while performing operations
	* @param object		$asset			the asset we are executing this function on
	* @param boolean	$read_access	read access check for the asset we
	*									are dealing with with respect to current user
	* @param boolean	$write_access	write access check for the asset we
	*									are dealing with with respect to current user
	*
	* @return array
	* @access private
	*/
	private function _editMetadataSchema($args=Array(), $asset=NULL, $read_access=FALSE, $write_access=FALSE)
	{
		if ($write_access) {
			if (!is_null($asset)) {

				$set = array_get_index($args, 'set', '');

				if (empty($set) && $set != 0) return $data['error'] = 'Please specify if the Metadata Scheme needs to be set or unset';

				$granted  = array_get_index($args, 'granted', TRUE);
				$cascades = array_get_index($args, 'cascades', NULL);
				$schemaid = array_get_index($args, 'schemaid', '');

				if (!empty($schemaid)) {

					$mm = $GLOBALS['SQ_SYSTEM']->getMetadataManager();
					$GLOBALS['SQ_SYSTEM']->am->acquireLock($asset->id, 'metadata');
					if ($set) {
						$schemas = $mm->setSchema($asset->id, $schemaid, $granted, $cascades);

						if ($schemas) {
							$data['success'] = 'Metadata Schema #'.$schemaid.' successfully '.($granted ? 'applied' : 'denied').' to Asset #'.$asset->id;
						} else {
							$data['error'] = 'Could not '.($granted ? 'apply' : 'deny').' Metadata Schema #'.$schemaid.' to Asset #'.$asset->id;
						}
					} else {
						$schemas = $mm->deleteSchema($asset->id, $schemaid);
						if ($schemas) {
							$data['success'] = 'Metadata Schema #'.$schemaid.' successfully removed from Asset #'.$asset->id;
						} else {
							$data['error'] = 'Could not remove Metadata Schema #'.$schemaid.' from Asset #'.$asset->id;
						}
					}
					$GLOBALS['SQ_SYSTEM']->am->releaseLock($asset->id, 'metadata');

				} else {
					$data['error'] = 'Assetid for the Metadata Schema to set on the Asset not provided';
				}
			} else {
				$data['error'] = 'Asset to set the Metadata Schema to was not provided';
			}
		} else {
			$data['error'] = 'You do not have permissions to access this asset';
		}//end else

		return $data;

	}// end _editMetadataSchema()


	/**
	* Get the metadata schema info applied to the given assetid
	*
	* @param array		$args			bundled array of arguments do be used while performing operations
	* @param object		$asset			the asset we are executing this function on
	* @param boolean	$read_access	read access check for the asset we
	*									are dealing with with respect to current user
	* @param boolean	$write_access	write access check for the asset we
	*									are dealing with with respect to current user
	*
	* @return array
	* @access private
	*/
	private function _getMetadataSchema($args=Array(), $asset=NULL, $read_access=FALSE, $write_access=FALSE)
	{
		if ($write_access) {
			if (!is_null($asset)) {
				$granted = array_get_index($args, 'granted', TRUE);
				$cascades = array_get_index($args, 'cascades', NULL);

				$data = $this->getMetadataSchema($asset, $granted, $cascades);
			} else {
				$data['error'] = 'Asset to get the Metadata Schema applied to it was not provided';
			}
		} else {
			$data['error'] = 'You do not have permissions to access this asset';
		}//end else

		return $data;

	}// end _getMetadataSchema()


	/**
	* Gets general information for the passed in Asset
	*
	* @param object		$asset					The asset we want to get information about
	* @param boolean	$add_more_attributes	if we are getting additional attributes and its values for the asset(FALSE by default)
	*
	* @return array
	* @access public
	*/
	public function getGeneralInfo(Asset $asset, $add_more_attributes=FALSE)
	{
		if (is_null($asset)) return Array();

		//root folder?
		if ($asset->id == '1') {
			$data['name'] = 'Root Folder';
			return $data;
		}

		// Set some shortcuts
		$am = $GLOBALS['SQ_SYSTEM']->am;

		$user_to_check = Array(
							'created_user'		=> Array(
														'id'		=> 'created_userid',
														'default'	=> 'Undefined',
												   ),
							'updated_user'		=> Array(
														'id'		=> 'updated_userid',
														'default'	=> 'Never Updated',
												   ),
							'published_user'	=> Array(
														'id'		=> 'published_userid',
														'default'	=> 'Never Published',
												   ),
							'status_user'		=> Array(
														'id'		=> 'status_changed_userid',
														'default'	=> 'Never Changed',
												   ),
						 );

		$created_user = $updated_user = $published_user = $status_user = NULL;
		foreach ($user_to_check as $user => $user_info) {
			$user_id = $user_info['id'];
			if (isset($asset->$user_id) && $asset->$user_id != '' && (int)$asset->$user_id != 0 ) {
				if(!(${$user} = $am->getAsset($asset->$user_id, '', TRUE))) {
					${$user} = 'Unknown User';
				}
			} else if (isset($asset->$user_id) && (int)$asset->$user_id == 0) {
				${$user} = 'Root User';
			} else {
				${$user} = $user_info['default'];
			}
		}

		$user = $am->getAsset($GLOBALS['SQ_SYSTEM']->currentUserId());
		$array_to_check_in = array_merge(Array($user->id), $user->getUserGroups());
		$effective_permission = '';

		if ($GLOBALS['SQ_SYSTEM']->userRoot($user) || $GLOBALS['SQ_SYSTEM']->userSystemAdmin($user)) {
			$effective_permission = 'Admin Access';
		}

		//if we havent got max permissions yet try the user id and all the user groups it belongs in
		if($effective_permission === '' ) {
			foreach($array_to_check_in as $user_group_id) {
				if ($asset->adminAccess('', Array($user_group_id))) {
					$effective_permission = 'Admin Access';
					// if admin permission is granted
					// no need to check on other groups
					break;

				} else if ($effective_permission != 'Write Access' && $asset->writeAccess('', Array($user_group_id), FALSE)) {
					$effective_permission = 'Write Access';
					// do not break out here as other usergroup
					// might have admin perm

					// go in the following block only if the effective permission isn't determined
					// if the write permission is already found no need to change the perm to read access
				} else if ($effective_permission == '' && $asset->readAccess(Array($user_group_id))) {
					$effective_permission = 'Read Access';
					// do not break out here as other usergroup
					// might have admin/write perm
				}
			}
		}

		// Get general info about asset
		$data = Array(
					'name'						=>	$asset->name,
					'short_name'				=>	$asset->short_name,
					'asset_id'					=>	$asset->id,
					'id'						=>	$asset->id,
					'type_code'					=>	$asset->type(),
					'type'						=>	str_replace('_', ' ', get_class($asset)),
					'icon_path'					=>	$am->getAssetIconURL($asset->type()),
					'data_path'					=>	$asset->data_path,
					'web_path'					=>	$asset->getURL(),
					'status'					=>	$asset->getStatusDescription(),
					'created'					=>	$asset->created,
					'created_userid'			=>	$asset->created_userid,
					'created_username'			=>	($created_user === 'Root User' || $created_user === 'Undefined' || $created_user === 'Unknown User') ? $created_user : $created_user->name,
					'updated'					=>	($updated_user === 'Never Updated' || $updated_user === 'Unknown User') ? $updated_user : $asset->updated,
					'updated_userid'			=>	($updated_user === 'Never Updated' || $updated_user === 'Unknown User') ? $updated_user : $updated_user->id,
					'updated_username'			=>	($updated_user === 'Never Updated' || $updated_user === 'Unknown User') ? $updated_user : $updated_user->name,
					'published'					=>	($published_user === 'Never Published' || $published_user === 'Unknown User') ? $published_user : $asset->published,
					'published_userid'			=>	($published_user === 'Never Published' || $published_user === 'Unknown User') ? $published_user : $published_user->id,
					'published_username'		=>	($published_user === 'Never Published' || $published_user === 'Unknown User') ? $published_user : $published_user->name,
					'status_changed'			=>	($status_user === 'Never Changed' || $status_user === 'Unknown User') ? $status_user : $asset->status_changed,
					'status_changed_userid'		=>	($status_user === 'Never Changed' || $status_user === 'Unknown User') ? $status_user : $status_user->id,
					'status_changed_username'	=>	($status_user === 'Never Changed' || $status_user === 'Unknown User') ? $status_user : $status_user->name,
					'maximum_perm_on_asset'		=>	$effective_permission,
				);

		foreach($am->getAssetTypeAttributes($asset->type()) as $name => $type) {
			if(($add_more_attributes || isset($data[$name])) && $name != 'api_key' && $type['type'] != 'password') {
				$value = $asset->attr($name);
				$data[$name] = $value;
			}
		}

		return $data;

	}//end getGeneralInfo()


	/**
	* Description: This will create a link between two assets
	*
	* @param integer	$parent			Major asset id we are linking from
	* @param integer	$child			Minor asset id we are linking to
	* @param integer	$link_type		Type of link to create
	* @param string		$link_value		Value of the link
	* @param integer	$sort_order		Order in the tree
	* @param integer	$is_dependant	Dependant to parent
	* @param integer	$is_exclusive	Exclusive to parent
	*
	* @return array
	* @access public
	*/
	function createAssociation($parent, $child, $link_type, $link_value, $sort_order, $is_dependant, $is_exclusive)
	{
		if (!empty($parent) && !empty($child)) {
			// Set some shortcuts
			$am = $GLOBALS['SQ_SYSTEM']->am;

			$parent	= $am->getAsset($parent);
			$child	= $am->getAsset($child);
			$new_id = $am->createAssetLink($parent, $child, $link_type, $link_value, $sort_order, $is_dependant, $is_exclusive);
			//done creating the link, lets update lookup
			$result = $this->updateLookupsforAsset($child->id);
			if (!is_null($result)) return Array('error' => 'Unable to Update Lookups on the Asset #'.$child->id.' and its Childrens');

			if ($new_id == 0) {
				return Array('error' => 'Unable to create link');
			}//end if
			return Array (
					'link_id'	=>	$new_id,
				   );
		} else {
			$this->returnError('Missing information to create link. Make sure parent id and child are provided');
		}//end else

	}//end createAssociation:()


	/**
	* Description: This operation will delete a link based on the LinkID
	*
	* @param string		$parent			Major asset id we are linking from
	* @param string		$child			Minor asset id we are linking to
	* @param string		$link_type		Link type between parent and child asset
	* @param string		$link_value		Link value of link between parent and child asset
	*
	* @return array
	* @access public
	*/
	function removeLink($parent, $child, $link_type=NULL, $link_value=NULL)
	{
		$am = $GLOBALS['SQ_SYSTEM']->am;

		// Set our link id
		$linkid = $this->getLinkId($parent, $child, $link_type, $link_value, TRUE);

		if (!$linkid) {
			$data['error'] = 'Parent Asset #'.$parent.' and Child Asset #'.$child.' do not have a valid link for given Link Type and Link Value';
			return $data;
		}

		$parent_asset = $am->getAsset($parent);
		$child_asset = $am->getAsset($child);

		if ($linkid['locked']) {
			$data['error'] = 'Link #"'.$linkid['link_id'].'" between Asset "'.$child_asset->name.'" (#'.$child_asset->id.') and Asset "'.$parent_asset->name.'" (#'.$parent_asset->id.') " cannot be deleted, link is locked';
			return $data;
		}

		// Set some shortcuts
		$am = $GLOBALS['SQ_SYSTEM']->am;

		if ($linkid['link_id'] != '') {
			$link = $am->getLinkById($linkid['link_id']);
			// Make sure our array contains some items
			if (!empty($link)) {
				// Get all our locks
				$am->acquireLock($link['majorid'], 'all');
				$am->acquireLock($link['minorid'], 'all');
				$parents = $am->getLinks($link['minorid'], SQ_SC_LINK_SIGNIFICANT, '', TRUE, 'minor');

				// See if we should delete a link or move the asset to the trash
				// but do this only if we are deleting a SIGNIFICANT LINK and not NOTICE link
				if (count($parents) > 1 || !($link_type & SQ_SC_LINK_SIGNIFICANT)) {
					// More than one parent link, we can delete the link
					$result	= $am->deleteAssetLink($linkid['link_id']);
				} else {
					// Only one link holly cow, lets trash it instead, otherwise we'l have an orphan asset in the system
					$result	= $am->trashAsset($link['minorid'], TRUE);
				}//end if
				//no matter if link is deleted or the whole asset is moved to trash, we will update lookups :)
				$result_from_update = $this->updateLookupsforAsset($child);
				if (!is_null($result_from_update)) $this->returnError('Unable to Update Lookups on the Asset "'.$child_asset->name.'" (#'.$child.') and its Childrens');

				// Release all our locks
				$am->releaseLock($link['minorid'], 'all');
				$am->releaseLock($link['majorid'], 'all');

				if ($result) {
					$data['success'] = 'Link #"'.$linkid['link_id'].'" between Asset "'.$child_asset->name.'" (#'.$child_asset->id.') and Asset "'.$parent_asset->name.'" (#'.$parent_asset->id.') " has been sucessfully removed';
					return $data;
				} else {
					$data['error'] = 'Unable to delete link';
					return $data;
				}//end else

			} else {
				$data['error'] = 'The link id that was provided is invalid';
				return $data;
			}//end else

		} else {
			$data['error'] = 'The link id provided is empty';
			return $data;
		}//end else

		return FALSE;

	}//end removeLink


	/**
	* Description: This operation will move a link under a new parent asset
	*
	* @param integer	$old_parent_id		Old parent id
	* @param integer	$asset_id_to_move	Asset id to move
	* @param constant	$old_link_type		Link type between asset and existing parent
	* @param string		$old_link_value		Link value between asset and existing parent
	* @param integer	$new_parent_id		New parent id
	* @param integer	$old_link_type		Link type to create on moving the asset
	* @param integer	$old_link_value		Link value to create on moving the asset
	* @param integer	$new_position		Position under the new parent
	*
	* @return void
	* @access public
	*/
	function moveLink($old_parent_id, $asset_id_to_move, $old_link_type=SQ_LINK_TYPE_1, $old_link_value='', $new_parent_id, $new_link_type=SQ_LINK_TYPE_1, $new_link_value='', $new_position)
	{
		// lets make sure the link type passed in for both, old link and new link are significant
		if (!($new_link_type & SQ_SC_LINK_SIGNIFICANT)) $new_link_type = SQ_LINK_TYPE_1;
		if (!($old_link_type & SQ_SC_LINK_SIGNIFICANT)) $old_link_type = SQ_LINK_TYPE_1;

		if (!empty($old_parent_id) && !empty($asset_id_to_move) && !empty($new_parent_id)) {
			// Get our link id!!
			$link_id = $this->getLinkId($old_parent_id, $asset_id_to_move, $old_link_type, $old_link_value, TRUE);

			if (!$link_id) {
				$data['error'] = 'Parent Asset #'.$old_parent_id.' and Child Asset #'.$asset_id_to_move.' do not have a valid link for given Link Type and Link Value';
				return $data;
			}

			if ($link_id['locked']) {
				$this->returnError('Link "'.$link_id['link_id'].'" cannot be moved, link is locked');
				return FALSE;
			}
			$new_link_id = $GLOBALS['SQ_SYSTEM']->am->moveLink($link_id['link_id'], $new_parent_id, $new_link_type, $new_position, $new_link_value);

			if (!empty($new_link_id)) {
				//wow we moved the asset successfully, now lets try and update lookups
				$result = $this->updateLookupsforAsset($asset_id_to_move);
				if (!is_null($result)) $this->returnError('Unable to Update Lookups on the Asset #'.$asset_id_to_move.' and its Childrens');
				return Array (
						'link_id'	=> $new_link_id,
					   );
			} else {
				$this->returnError('Unable to move link');
			}//end else
		} else {
			$this->returnError('Please provide a valid current parent id, asset id to move, and new parent id');
		}//end else

		return FALSE;

	}//end moveLink


	/**
	* Description: This operation will update an existing link
	*
	* @param integer	$parent			Major asset id we are linking from
	* @param integer	$child			Minor asset id we are linking to
	* @param integer	$link_type		Type of link to set
	* @param string		$link_value		Value of the link
	* @param integer	$sort_order		Sort order in the asset tree
	* @param integer	$locked			the asset's link lock status
	*
	* @return array
	* @access public
	*/
	function updateLink($parent, $child, $existing_link_type=SQ_LINK_TYPE_1, $existing_link_value= NULL, $link_type=SQ_LINK_TYPE_1, $link_value='', $sort_order, $locked=NULL)
	{
		$am = $GLOBALS['SQ_SYSTEM']->am;

		if (!empty($parent) && !empty($child)) {

			$link_id = $this->getLinkId($parent, $child, $existing_link_type, $existing_link_value, TRUE);
			$parent_asset = $am->getAsset($parent);
			$child_asset = $am->getAsset($child);

			if (!$link_id) {
				$data['error'] = 'Parent Asset #'.$parent.' and Child Asset #'.$child.' do not have a valid link for given Link Type and Link Value';
				return $data;
			}

			// we cannot change a link from being a significant link to a non-significant link or vice versa
			// so if we are try to do that, do what a backend does, delete the old one and then create a new
			// one with all same properties except for changed linked value
			$current_is_sig = (bool) ((int) $link_id['link_type'] & SQ_SC_LINK_SIGNIFICANT);
			$new_is_sig     = (bool) ($link_type & SQ_SC_LINK_SIGNIFICANT);
			if ($current_is_sig !== $new_is_sig) {
				if ($am->deleteAssetLink($link_id['link_id'], FALSE)) {
					// irrespective of what is passed value of link lock, if the link isnt siginificant, then dont bother locking it
					if (!$new_is_sig) {
						$lock = 0;
					} else {
						$lock = $locked;
					}
					$new_link_id = $am->createAssetLink($parent_asset, $child_asset, $link_type, $link_value, $sort_order, $link_id['is_dependant'], $link_id['is_exclusive'], FALSE, $lock);
				}

			} else {
				$new_link_id = $am->updateLink($link_id['link_id'], $link_type, $link_value, $sort_order, $locked);
			}

			if (!empty($new_link_id)) {
				$response['success'] = 'Link #'.$link_id['link_id'].' between Asset "'.$child_asset->name.'" (#'.$child_asset->id.') and Asset "'.$parent_asset->name.'" (#'.$parent_asset->id.') has been updated';
				return $response;
			} else {
				$response['error'] = 'Unable to update Link #'.$link_id['link_id'].' between Asset "'.$child_asset->name.'" (#'.$child_asset->id.') and Asset "'.$parent_asset->name.'" (#'.$parent_asset->id.')';
				return $response;
			}//end else
		} else {
			$this->returnError('Please enter a valid parent id and child id');
			return FALSE;
		}//end else

	}//end updateLink


	/**
	* Description: This operation will return the permission set for on an asset
	*
	* @param integer	$id				Id of the asset to get permissions for
	* @param string		$level			Read, Write, Admin
	*
	* @return void
	* @access public
	*/
	function getPermission($id, $level)
	{
		if (!empty($id) && !empty($level)) {
			// Shortcuts
			$am = $GLOBALS['SQ_SYSTEM']->am;

			// Set our level
			if ($level == 1) {
				$level = SQ_PERMISSION_READ;
				$level_name = 'read';
			} else if ($level == 2) {
				$level = SQ_PERMISSION_WRITE;
				$level_name = 'write';
			} else if ($level == 3) {
				$level = SQ_PERMISSION_ADMIN;
				$level_name = 'admin';
			}//end else if

			$permissions = $am->getPermission($id, $level, NULL, FALSE, FALSE, TRUE);
			if (empty($permissions)) {
				$this->returnError('There are no '.$level_name.' permissions set for id #'.$id);
				return FALSE;
			}

			foreach ($permissions as $key => $access) {

				$granted = ($access == 0) ? 'denied' : 'granted';
				$asset = $am->getAsset($key);

				$data[$granted][$key]['id'] = $key;
				$data[$granted][$key]['name'] = $asset->name;
				if (isset($asset->vars['username'])) {
					$data[$granted][$key]['user_name'] = $asset->attr('username');
				}
				$data[$granted][$key]['type'] = $asset->type();

				// Expand our user groups
				if ($asset->type() == 'user_group') {

					$data[$granted][$key]['users'] = $this->getGroupChildren($key);

				}//end if

			}//end foreach

			return $data;

		} else {
			$this->returnError('Asset ID or Access Level is not valid. Please provide a valid AssetID and Access Level');
			return FALSE;
		}//end else

	}//end getPermission


	/**
	* Description: Returns arrays of user assets from a parent user group
	*
	* @param integer	$key			Asset id of the user group
	*
	* @return void
	* @access public
	*/
	function getGroupChildren($key)
	{
		// Shortcuts
		$am = $GLOBALS['SQ_SYSTEM']->am;

		$group_children = $am->getChildren($key, '', TRUE, NULL, NULL, NULL, TRUE, NULL, 1);
		foreach ($group_children as $children => $child_id) {
			$child = $am->getAsset($children);
			$children_arr[$child->id] = Array(
							'id'		=>	$child->id,
							'name'		=>	$child->name,
							'type'		=>	$child->type(),
							'username'	=>	$child->attr('username'),
						);
		}//end foreach

		return $children_arr;

	}//end getGroupChildren


	/**
	* Description: Gets parent asset ids
	*
	* @param integer	$id					Asset id of the child
	* @param integer    $level				How deep to go
	* @param array		$type_codes			type_code of assets to return
	* @param array		$link_types			type of link we are looking to return
	* @param array		$link_values		link values we want on the assets returned
	* @param boolean	$also_attributes	if we are getting additional attributes and its values(FALSE by default)
	*
	* NOTE : link_types and link_values works only if the level of gettign the child assets is set to 1
	*
	* @return void
	* @access public
	*/
	function getParents($id, $level, $type_codes=Array(), $link_types=Array(), $link_values=Array(), $also_attributes=FALSE)
	{
		// Shortcuts
		$am = $GLOBALS['SQ_SYSTEM']->am;
		$parent_arr = Array();
		$parents = $am->getParents($id, '', TRUE, NULL, NULL, TRUE, NULL, $level);

		foreach ($parents as $parent => $parent_type_code) {
			// not in the type_code we want, remove it
			if(!empty($type_codes) && !in_array($parent_type_code, $type_codes)) {
				continue;
			}
			$parent_asset = $am->getAsset($parent);

			// Levels greater than 1 won't have a link id with the parent
			if (!$this->hasLink($parent, $id)) {
				$show['link_id']		= NULL;
				$show['link_type']		= NULL;
				$show['is_dependant']	= NULL;
				$show['is_exclusive']	= NULL;
				$show['sort_order']		= NULL;
				$show['link_value']		= NULL;
				$show['locked']			= NULL;
			} else {
				$show_link = $this->getLinkId($parent, $id, NULL, NULL, TRUE);
				if (!$show_link) {
					$data['error'] = 'Parent Asset #'.$parent.' and Child Asset #'.$id.' do not have a valid link';
					return $data;
				}

				// abra-ka-dabra all except for the link_values we are interested in
				if (!empty($link_values) && !in_array($show_link['value'], $link_values)) {
					continue;
				}

				if(!empty($link_types)) {
					$found = FALSE;
					foreach($link_types as $link_type) {
						if(defined($link_type) && (constant($link_type) == $show_link['link_type'])) {
							$found = TRUE;
						} else if (!defined($link_type)) {
							$found = TRUE;
						}
					}
					if(!$found) {
						continue;
					}
				}

				$show['link_id']		= $show_link['link_id'];
				$show['link_type']		= $show_link['link_type'];
				$show['is_dependant']	= $show_link['is_dependant'];
				$show['is_exclusive']	= $show_link['is_exclusive'];
				$show['sort_order']		= $show_link['sort_order'];
				$show['link_value']		= $show_link['value'];
				$show['locked']			= $show_link['locked'];
			}

			$parent_arr[] = array_merge($this->getGeneralInfo($parent_asset, $also_attributes), $show);

		}//end foreach

		// see if there are any parents or not
		// if not then stop matrix from throwing a JSON.parse: unexpected end of data error
		if (empty($parent_arr)) {
			$parent_arr['error'] = 'No parents found for asset #'.$id.' for the given filters';
		}

		return $parent_arr;

	}//end getParents


	/**
	* Description: Gets child asset ids
	*
	* @param integer	$id					Asset id of the parent
	* @param integer	$level				How deep to go
	* @param array		$type_codes			type_code of assets to return
	* @param array		$link_types			type of link we are looking to return
	* @param array		$link_values		link values we want on the assets returned
	* @param boolean	$also_attributes	if we are getting additional attributes and its values(FALSE by default)
	*
	* NOTE : link_types, type_codes and link_values works only if the level of gettign the child assets is set to 1
	*
	* @return void
	* @access public
	*/
	function getChildren($id, $level, $type_codes=Array(), $link_types=Array(), $link_values=Array(), $also_attributes=FALSE)
	{
		// Shortcuts
		$am = $GLOBALS['SQ_SYSTEM']->am;
		$children_arr =Array();
		if (empty($type_codes)) $type_codes = '';
		foreach ($link_types as $index => $link_type) {
			 if(defined($link_type)) $link_types[$index] = constant($link_type);
		}

		$children = $am->getChildren($id, $type_codes, TRUE, NULL, NULL, NULL, TRUE, NULL, $level, TRUE, NULL, $link_types);

		foreach ($children as $child_id => $child_info) {

			// Levels lower than 1 won't have a link id with the parent
			if (!$this->hasLink($id, $child_id)) {
				$show['link_id']		= NULL;
				$show['link_type']		= NULL;
				$show['is_dependant']	= NULL;
				$show['is_exclusive']	= NULL;
				$show['sort_order']		= NULL;
				$show['link_value']		= NULL;
				$show['locked']			= NULL;
			} else {
				$show_link = $this->getLinkId($id, $child_id, NULL, NULL, TRUE);

				// avada kedavra all except fo the link_values we are interested in
				if (!empty($link_values) && !in_array($show_link['value'], $link_values)) {
					unset($children[$child_id]);
					continue;
				}

				$show['link_id']		= $show_link['link_id'];
				$show['link_type']		= $show_link['link_type'];
				$show['is_dependant']	= $show_link['is_dependant'];
				$show['is_exclusive']	= $show_link['is_exclusive'];
				$show['sort_order']		= $show_link['sort_order'];
				$show['link_value']		= $show_link['value'];
				$show['locked']			= $show_link['locked'];
			}

			$sub_children = $am->getLinks($child_id, SQ_SC_LINK_ALL);
			$sub_dependent_count = $sub_non_dependent_count = $sub_shadow_child_count = 0;

			foreach ($sub_children as $idx => $sub_child) {

				if ($sub_child['link_type'] == SQ_LINK_NOTICE)	$sub_shadow_child_count++;
				else if ($sub_child['is_dependant']) 			$sub_dependent_count++;
				else if ($sub_child['is_dependant'] == 0) 		$sub_non_dependent_count++;
			}

			$show['direct_dependant_children']		= $sub_dependent_count;
			$show['direct_non_dependant_children']	= $sub_non_dependent_count;
			$show['direct_notice_linked_children']	= $sub_shadow_child_count;

			$child = $am->getAsset($child_id);
			$children_arr[] = array_merge($this->getGeneralInfo($child, $also_attributes), $show);

		}//end foreach

		return $children_arr;

	}//end getChildren


	/**
	* Description: This operation will send an asset to the trash
	*
	* @param array  $ids  The id of the assets we want to delete
	*
	*
	* @return array
	* @access public
	*/
	function trashAsset($ids)
	{
		// Set some shortcuts
		$am = $GLOBALS['SQ_SYSTEM']->am;
		$response = Array();

		if (!empty($ids)) {
			foreach($ids as $id) {
				$already_in_trash = $am->assetInTrash($id, TRUE);
				$asset = $am->getAsset($id);
				//need to make sure that links are not locked
				$parents = $this->getParents($id, 1);
				$locked = FALSE;
				//loop through each parents and see if any of the links are locked
				foreach ($parents as $parentid => $parent_link_info) {
					if($parent_link_info['locked'] == '1') $locked = TRUE;
				}

				if ($already_in_trash) {
					$response[] = 'Asset (#'.$asset->id.') is already in the trash';
				} else if ($locked && $asset->writeAccess()){
					// few or all the links are locked so lets go through each one of them and try to delete the links
					foreach ($parents as $parentid => $parent_link_info) {
						$parent_asset = $am->getAsset($parentid);
						if ($parent_link_info['locked'] == '1') {
							// found the link locked....
							$response[] = 'Link between Asset "'.$asset->name.'" (#'.$asset->id.') and Asset "'.$parent_asset->name.'" (#'.$parent_asset->id.') is locked';
						} else {
							//link not locked....lets get it done with
							if($am->deleteAssetLink($parent_link_info['link_id'])) {
								$response[] = 'Successfully unlinked link between Asset "'.$asset->name.'" (#'.$asset->id.') and Asset "'.$parent_asset->name.'" (#'.$parent_asset->id.')';
							}
						}
					}
				} else if ($asset->writeAccess()) {
					// asset isnt in trash and none of the links are locked, Hooorrrrray !!!!
					$am->acquireLock($id, 'all');
					$result	= $am->trashAsset($id, TRUE);
					if (!$result) {
						$response[] = 'Could not move Asset "'.$asset->name.'" (#'.$asset->id.') to Trash';
					} else {
						//done trashing the asset? update lookups then
						$result_from_update = $this->updateLookupsforAsset($id);
						if (!is_null($result_from_update)) {
							$response[] = 'Asset deleted but unable to Update Lookups on the Asset "'.$asset->name.'" (#'.$asset->id.')';
						} else {
							//all good? let the user know what we did
							$response[] = 'Asset "'.$asset->name.'" (#'.$asset->id.') successfully moved to Trash';
						}
					}
					$am->releaseLock($id, 'all');
				} else {
					$response[] = 'Not enough permission to delete Asset (#'.$asset->id.')';
				}

			}
		} else {
			$this->returnError('Please provide a valid Asset ID');
			return FALSE;
		}//end else

		return $response;

	}//end trashAsset()


	/**
	* Description: This operation will return set the attribute value of an asset based on assetid and attribute name
	*
	* @param string		$assetid	The ID of the asset in query
	* @param array		$attr_name	The names of the attributes
	* @param array		$attr_val	The new values of the attributes
	*
	* @return array
	* @access public
	*/
	function setAttributeValue($assetid, $attr_name, $attr_val)
	{
		if (!empty($assetid) && !empty($attr_name) && !empty($attr_val)) {
			if (count($attr_name) != count($attr_val) ) $this->returnError('Please provide Attributes and corresponding values correctly');
			// Shortcuts
			$asset	= $GLOBALS['SQ_SYSTEM']->am->getAsset($assetid);
			$am = $GLOBALS['SQ_SYSTEM']->am;

			// Change our values
			$am->acquireLock($assetid, 'attributes');
			for ($i = 0; $i < count($attr_name); $i++ ) {
				$attributes = $am->getAssetTypeAttributes($asset->type());
				$attr_type = $attributes[$attr_name[$i]]['type'];
				if ($attr_name[$i] != '' && array_key_exists($attr_name[$i], $attributes)) {
					//set our new attribute
					$attr_set = $asset->setAttrValue($attr_name[$i], $attr_val[$i]);

					if(!$attr_set) {
						if ($$attr_type == 'password') {
							$data[] = 'Attribute "'.$attr_name[$i].'"(of type "'.$attr_type.'") could not be set for Asset "'.$asset->name.'" (#'.$assetid.')';
						} else {
							$data[] = 'Attribute "'.$attr_name[$i].'"(of type "'.$attr_type.'") could not be set to "'.$attr_val[$i].'" for Asset "'.$asset->name.'" (#'.$assetid.')';
						}
						continue;
					}
				} else if (!array_key_exists($attr_name[$i], $attributes)){
					$data[] = 'Attribute "'.$attr_name[$i].'" does not exist for Asset "'.$asset->name.'" (#'.$assetid.')';
					continue;
				}
				if (!$asset->saveAttributes()) {
					// Do not show the attribute value in the response message for the "password" attr
					if ($attr_type == 'password') {
						$data[] = 'Attribute "'.$attr_name[$i].'" could not be set for Asset "'.$asset->name.'" (#'.$assetid.')';
					} else {
						$data[] = 'Attribute "'.$attr_name[$i].'" could not be set to "'.$attr_val[$i].'" for Asset "'.$asset->name.'" (#'.$assetid.')';
					}
				} else {
					if ($attr_type == 'password') {
						$data[] = 'Attribute "'.$attr_name[$i].'" has been successfully set for Asset "'.$asset->name.'" (#'.$assetid.')';
					} else {
						$data[] = 'Attribute "'.$attr_name[$i].'" has been successfully set to "'.$attr_val[$i].'" for Asset "'.$asset->name.'" (#'.$assetid.')';
					}
				}
			}//end for
			$am->releaseLock($assetid, 'attributes');
			$am->forgetAsset($asset);

			return $data;
		} else {
			$this->returnError('Please make sure to provide an Asset ID, Attribute Name and Attribute Value');
		}//end else

	}//end setAttributeValue()


	/**
	* Description: This operation will set the value for a metadata field(s) of an asset
	*
	* @param string		$assetid	The asset to regenerate metadata for
	* @param array		$fieldid	Array of the metadata field ids
	* @param array		$new_value	Array of the new value corresponding to metadata field
	*
	* @return void
	* @access public
	*/
	function setAssetMetadata($assetid, $fieldid, $new_value)
	{
		if (!empty($assetid) && !empty($fieldid)) {
			if (count($fieldid) != count($new_value) ) $this->returnError('Please provide Metadata fieldids and corresponding values correctly');
			$s_result = TRUE;
			$g_result = TRUE;

			// Shortucts
			$mm	= $GLOBALS['SQ_SYSTEM']->getMetadataManager();
			$am = $GLOBALS['SQ_SYSTEM']->am;

			$am->acquireLock($assetid, 'metadata');
			// All context metadata files need to be regenerated if there is a non-contextable field changed
			$regen_all_contexts = FALSE;
			for ($i = 0; $i < count($fieldid); $i++ ) {

				$field	= $am->getAsset($fieldid[$i]);
				if (!($field instanceof Metadata_Field)) {
					$this->returnError('The Field ID provided does not belong to a metadata field');
					return FALSE;
				}//end if

				if (!$regen_all_contexts && (!$field->attr('is_contextable') || $field instanceof Metadata_Field_Select)) {
					$regen_all_contexts = TRUE;
				}

				$field_name		= $field->attr('name');
				$metadata_info	= Array (
									$fieldid[$i]	=> Array (
														Array (
															'name'	=> $field_name,
															'value'	=> $new_value[$i],
														),
													   ),
								  );
				// Set metadata
				if (!$mm->setMetadata($assetid, $metadata_info)) $s_result = FALSE;

			}//end for
			$am->releaseLock($assetid, 'metadata');

			// if the Metadata wasnt set correctly, dont even worry about regenerating the metadata file
			// just return from here telling the user it was a big time fail ;)
			if(!$s_result) {
				$this->returnError('Unable to set Matadata for Asset #'.$assetid);
				return FALSE;
			}

			// instead for doing it for every field, lets regen content file just once here
			if ($regen_all_contexts) {
				$all_contexts = $GLOBALS['SQ_SYSTEM']->getAllContexts();
				foreach ($all_contexts as $contextid => $context_data) {
					if (!$mm->generateContentFile($assetid, FALSE, $contextid)) {
						$g_result = FALSE;
						break;
					}
				}
			} else {
				if (!$mm->generateContentFile($assetid)) $g_result = FALSE;
			}

			if ($s_result && $g_result) {
				return TRUE;
			} else {
				$this->returnError('Unable To Regenerate Metadata For Asset #'.$assetid);
				return FALSE;
			}//end else
		} else {
			$this->returnError('Please provide both AssetID and FieldID of the metadata field');
			return FALSE;
		}//end else

	}//end setAssetMetadata()


	/**
	* Description: Returns the link id between a parent and a child
	*
	* @param string		$parent			Parent id
	* @param string		$child			Child id
	* @param string		$link_type		Type of link we are looking for (default to TYPE 1 links)
	* @param string		$link_value		Value of the link we are looking for(Default to NULL)
	* @param boolean	$all_info		If to return just LinkId or all the information
	*
	* @return array
	* @access public
	*/
	function getLinkId($parent, $child, $link_type=SQ_LINK_TYPE_1, $link_value=NULL, $all_info = FALSE)
	{
		if (!empty($parent) && !empty($child)) {
			$link = $GLOBALS['SQ_SYSTEM']->am->getLinkByAsset($parent, $child, $link_type, $link_value, 'major', TRUE);
			if (empty($link)) {
				return FALSE;
			} else if (count($link) > 1) {
				$this->returnError('More than one link found between Parent Asset #'.$parent.' and Child Asset #'.$child.', please filter out with help of Link value and Link Type');
				return FALSE;
			}

			$link = $link[0];
			if (!$all_info) {
				$data_array['link_id'] = array_get_index($link, 'linkid', 0);
				return $data_array;
			} else {
				// we are returning more information
				$data_array = Array (
								'link_id'		=> isset($link['linkid']) ? $link['linkid'] : '',
								'minorid'		=> isset($link['minorid']) ? $link['minorid'] : '',
								'majorid'		=> $parent,
								'value'			=> isset($link['value']) ? $link['value'] : '',
								'link_type'		=> isset($link['link_type']) ? $link['link_type'] : '',
								'sort_order'	=> isset($link['sort_order']) ? $link['sort_order'] : '',
								'is_dependant'	=> isset($link['is_dependant']) ? $link['is_dependant'] : '',
								'is_exclusive'	=> isset($link['is_exclusive']) ? $link['is_exclusive'] : '',
								'locked'		=> isset($link['locked']) ? $link['locked'] : '',
							  );
				return $data_array;
			}
		} else {
			$this->returnError('Please provide a parent id and child id');
			return FALSE;
		}//end else

	}//end getLinkId


	/**
	* Description: Check to see if there is a link id between a parent and child
	*
	* @param string		$parent			Parent id
	* @param string		$child			Child id
	*
	* @return void
	* @access public
	*/
	function hasLink($parent, $child)
	{
		if (!empty($parent) && !empty($child)) {
			$link = $GLOBALS['SQ_SYSTEM']->am->getLinkByAsset($parent, $child);
			$linkid = array_get_index($link, 'linkid', 0);
			if (!empty($linkid)) {
				// Found a link
				return TRUE;
			}//end if
		}//end if

		return FALSE;

	}//end hasLink


	/**
	* Description: This operation will create an asset of a specific type under a specific location
	*
	* @param integer	$id					Parentid of the new parent
	* @param string		$type_code			Type code of new asset
	* @param string		$asset_name			Name for new asset
	* @param integer	$link_type			Type of link to create
	* @param string		$link_value			Value of the link
	* @param integer	$sort_order			Order in the tree
	* @param integer	$is_dependant		Dependant to parent
	* @param integer	$is_exclusive		Exclusive to parent
	* @param boolean	$extra_attributes	Any extra attributes to create
	* @param array		$attribute_values	array of name => value of the extra attributes that needs to be set
	*
	* @return array
	* @access public
	*/
	function createAsset($id, $type_code, $asset_name, $link_type=1, $link_value, $sort_order, $is_dependant=0, $is_exclusive=0, $extra_attributes=FALSE, $attribute_values)
	{
		// Shortucts
		$mm	= $GLOBALS['SQ_SYSTEM']->getMetadataManager();
		$am = $GLOBALS['SQ_SYSTEM']->am;
		// Get our parent asset
		$pa = $am->getAsset($id);
		// Set our asset type to create
		$type_code = strtolower($type_code);
		if (!$am->installed($type_code)) {
			return Array('error' => 'Asset type requested to create not installed on the system');
		}//end if

		// Are we allowed to create this?
		$types = $this->attr('types');
		if (!empty($types)) {
			if (!isset($types[$type_code])) {
				return Array('error' => 'Asset type requested to create is not allowed via API');
			}
		}

		// Are we allowed to create this asset under this parent type?
		$restrict = $this->attr('types_restriction');
		if (!empty($restrict)) {
			if (!$this->checkParentType($pa)) {
				return Array('error' => 'Parent asset passed does not match the restricted Parents type configured');
			}
		}

		$am->includeAsset($type_code);

		$object_name = str_replace(' ', '_', ucwords(str_replace('_', ' ', $type_code)));
		$asset = new $object_name();
		$asset->setAttrValue('name', $asset_name);
		$request_info['asset']	= $pa;
		// Set our asset info
		$link_info	= Array (
						'asset'			=> $pa,
						'name'			=> $asset_name,
						'link_type'		=> $link_type,
						'value'			=> $link_value,
						'sort_order'	=> $sort_order,
						'is_dependant'	=> $is_dependant,
						'is_exclusive'	=> $is_exclusive,
					  );

		// Should we add some attributes to our new asset?
		if ($extra_attributes && $this->attr('create_attributes')) {
			// Check our post values for any correct attibutes
			$attributes = $am->getAssetTypeAttributes($type_code);
			foreach($attribute_values as $name => $value) {
				if (array_key_exists($name, $attributes) && $name !== $link_info[$name]) {
					// Set our new values
					$asset->setAttrValue($name, $value);
				}//end if
			}//end foreach
			
			// for batch request we still have to look for the 'attributes' url param, because it won't be auto set in array
			if(isset($attribute_values['attributes'])) {
			    $all_attributes = explode('&', $attribute_values['attributes']);
			    foreach ($all_attributes as $attribute_data) {
				$attribute_data_parts = explode('=', $attribute_data);
				if(isset($attribute_data_parts[0]) && !empty($attribute_data_parts[0]) && isset($attribute_data_parts[1])) {
				    $asset->setAttrValue(trim($attribute_data_parts[0]), $attribute_data_parts[1]);
				}
			    }
			}
		}//end if

		$link_id = $asset->create($link_info);

		if ($link_id) {
			// Save and quit
			$am->acquireLock($asset->id, 'attributes');
			$asset->saveAttributes();
			$am->releaseLock($asset->id, 'attributes');
			return Array (
					'name'		=>	$asset_name,
					'id'		=>	$asset->id,
					'link_id'	=>	$link_id,
				   );
		}//end if

		return Array('error' => 'Error occured while asset creation');

	}//end createAsset


	/**
	* Description: Check out see if the parent passed matches the allowed types
	*
	* @param object	 $parent	The asset to check
	*
	* @return TRUE
	* @access public
	*/
	function checkParentType($parent)
	{
		// Are we allowed to create this asset under this parent type?
		if (array_key_exists($parent->type(), $this->attr('types_restriction'))) {
			return TRUE;
		}

	}//end checkParentType


	/**
	* Description: Check out see if the passed asset is under the root restriction
	*
	* @param object	 &$asset	The asset to check
	*
	* @return TRUE
	* @access public
	*/
	function checkRoot(&$asset)
	{
		// This is the root node, will allow it
		$root_nodes = $this->getRootNodes();

		// check to see if we ARE the root node OR
		// one of the root node is ROOT_FOLDER
		// that way we can cut down the queries
		if (in_array($asset->id, $root_nodes) || in_array('1', $root_nodes)) return TRUE;

		// If a shadow link test the bridge instead
		if (strpos($asset->id, ':') !== FALSE) {
			$matches = explode(':', $asset->id);
			$test_asset = $GLOBALS['SQ_SYSTEM']->am->getAsset($matches[0]);
		} else {
			$test_asset = $asset;
		}//end if

		$is_under = $GLOBALS['SQ_SYSTEM']->am->isUnderRootNodes($test_asset->id, $root_nodes);

		return $is_under;

	}//end checkRoot


	/**
	 * Return a list of the root nodes
	 *
	 * @return array
	 * @access public
	 */
	public function getRootNodes()
	{
		$root_nodes   = $GLOBALS['SQ_SYSTEM']->am->getLinks($this->id, SQ_LINK_NOTICE, '', TRUE, 'major', 'root');
		$nodes = Array();
		foreach ($root_nodes as $node) {
			$node_id = array_get_index($node, 'minorid', 0);
			if (!empty($node_id)) $nodes[] = $node_id;
		}//end foreach

		return $nodes;

	}//end getRootNodes()


	/**
	* updates the lookup of the assetid passed and all its children, (runs freestyle HIPO)
	*
	* @return assetid
	* @access public
	*/
	public function updateLookupsforAsset($assetid)
	{
		require_once SQ_SYSTEM_ROOT.'/core/hipo/jobs/hipo_job_update_lookups.inc';
		$hh = $GLOBALS['SQ_SYSTEM']->getHipoHerder();

		$vars = Array('assetids' => Array($assetid));
		$hh->freestyleHipo('hipo_job_update_lookups', $vars);

	}//end updateLookupsforAsset()


	/**
	* Description: get the asset/global/simple_edit level keyword replacement for the passed in asset
	*
	* @param object	 $asset	The asset to fetch replacements for
	* @param array	 $keywords	array of keywords to get replacement for
	*
	* @return array
	* @access public
	*/
	public function getKeywordsReplacements($asset, $keywords)
	{
		require_once SQ_INCLUDE_PATH.'/general.inc';
		$return_array = Array();
		$se_level_keywords = Array();
		//lets split asset/global level keywords as opposed to simple edit layout keywords
		foreach ($keywords as $index => $keyword) {
			if(!(strpos($keyword, '%asset_') === 0 || strpos($keyword, '%globals_') === 0) ) {
				//seems like its a simple edit screen keywords
				$se_level_keywords[] = $keyword;
				unset($keywords[$index]);
			}
		}

		foreach ($keywords as $keyword) {
			$trimmed_keyword = trim($keyword,'%');
			if (strpos($keyword, '%globals_') === 0) {
				$return_array[$trimmed_keyword] = replace_global_keywords($keyword);
			} else if (strpos($keyword, '%asset_') === 0 ) {
				$return_array[$trimmed_keyword] = $asset->getKeywordReplacement($trimmed_keyword);
			}
			if (empty($return_array[$trimmed_keyword]) || $return_array[$trimmed_keyword] == '%'.$trimmed_keyword.'%') {
				$return_array[$trimmed_keyword] = 'Could not get replacement for keyword : '.$trimmed_keyword ;
			}
		}

		if(!empty($se_level_keywords)) {
			// create layout on the fly
			$GLOBALS['SQ_SYSTEM']->am->includeAsset('layout');
			$layout = new Layout();
			// create limbo outputter on fly
			require_once SQ_INCLUDE_PATH.'/limbo_outputter.inc';
			$limbo_outputter = new Limbo_Outputter();
			// get asset_edit_interface
			require_once SQ_SYSTEM_ROOT.'/core/include/asset_edit_interface.inc';
			$ei = new Asset_Edit_Interface($asset->type());

			// determine and set the preview url
			// we don't care about the returned value, it's not used here
			$ei->_determinePreviewUrl($asset, $limbo_outputter);

			foreach ($se_level_keywords as $index => $se_keyword) {
				$se_keyword = trim($se_keyword,  '%');
				$layout->layout_keywords = $layout->getLayoutKeywordsArray(Array ($se_keyword));
				//sort order isnt really important here, so we always use like follows
				$screen_sort_order = Array ('0' => 'custom', '1' => 'screen');
				$return_array[$se_keyword] = $ei->getKeywordsReplacementsForPaint($asset, $limbo_outputter, $layout, $screen_sort_order, FALSE, TRUE);
				// lets replace % with &#37; so that while rendering on frontend %asset_name% (keywords)
				// are not replaced and the raw keyword is spat out. but also make sure to do this only to keywords
				$pattern = '/(.)(%)([asset_|globals_]?[a-zA-Z0-9_:\.\^]*)(%)(.)/';
				$replacement = '$1&#37;$3&#37;$5';
				$return_array[$se_keyword] = preg_replace($pattern, $replacement ,$return_array[$se_keyword]);
				$pattern_2 = '/([\"|\']{0}%)([asset_|globals_]?[a-zA-Z0-9_:\.\^]*)(%[\"|\']{0})/';
				$replacement_2 = '&#37;$2&#37;';
				$return_array[$se_keyword] = preg_replace($pattern_2, $replacement_2 ,$return_array[$se_keyword]);

				if (empty($return_array[$se_keyword])) $return_array[$se_keyword] = 'Could not get replacement for keyword : '.$se_keyword ;
			}
		}

		return $return_array;

	}//end getKeywordsReplacements()


	/**
	* Description: trys to acquire locks on the asset
	* if the locks are already acquired then it will update them
	*
	* @param object	 $asset				the asset to acquire locks for
	* @param string	 $screen			the screen for which to acquire locks for
	* @param boolean $dependants_only	whether dependants only or all children, defaults to true
	* @param boolean $force_acquire		whether to attempt to forceably acquire the lock or not,  defaults to false
	*
	* @return array
	* @access public
	*/
	public function acquireLocks(Asset $asset, $screen, $dependants_only , $force_acquire)
	{
		$return_array = Array();

		//if the we arent trying to get all locks and if the locks name specified doesnt exist for this asset then start complaining
		if(!in_array($screen, $GLOBALS['SQ_SYSTEM']->am->getLockTypes($asset->id, 'all')) && $screen != 'all') {
			$return_array[] = '"'.$screen.'" type lock doesnt exist for Asset "'.$asset->name.'" (#'.$asset->id.').';
			return $return_array;
		}

		//lets prepare a hipo running vars
		$res = NULL;
		$locks = $this->getLocksInfo($asset->id, $screen);

		if (empty($locks) || (!empty($locks) && $force_acquire) || (!empty($locks) && $screen == 'all')) {
			$job_dir = SQ_SYSTEM_ROOT.'/core/hipo/jobs';
			require_once $job_dir.'/hipo_job_acquire_locks.inc';
			$init_hipo = new Hipo_Job_Acquire_Locks();
			$running_vars = Array(
								'assetids'          => Array($asset->id),
								'lock_type'         => $screen,
								'forceably_acquire' => $force_acquire,
								'dependants_only'   => $dependants_only,
							);
			$init_hipo->setRunningVars($running_vars);
			$init_hipo->_steps = $init_hipo->getInitialStepData();
			if (!$init_hipo->prepare()) {
				$return_array['error'] = 'Could not initialise HIPO job';
				return $return_array;
			}
			set_error_handler(Array(&$init_hipo, '_errorHandler'));
				$init_hipo->freestyle();
			restore_error_handler();
			$return_array = $init_hipo->getErrors();
		} else {
			// the locks are already acquired on the asset
			// if we are asked again to do it mean we have
			// to update the locks
			if (!$GLOBALS['SQ_SYSTEM']->am->updateLock($asset->id, $screen)) {
				$return_array['error'] = 'Unable to update locks for Asset '.$asset->name.'('.$asset->id.')';
			} else {
				$return_array[] = 'Locks for \''.$screen.'\' screen for Asset '.$asset->name.'('.$asset->id.') successfully updated';
			}
		}

		return array_unique($return_array);

	}//end acquireLocks()


	/**
	* Returns an array of information about the current lock on this asset (if any)
	*
	* @param string		$assetid		the assetid of the asset that we want lock info for
	* @param string		$lock_type		any valid lock type for the passed asset's type, plus 'all' which will return all locks that this asset has -
	*
	* @return array
	* @access public
	* @see Asset_Manager::getLockInfo()
	*/
	public function getLocksInfo($assetid, $lock_types)
	{
		$return_array =  $GLOBALS['SQ_SYSTEM']->am->getLockInfo($assetid, $lock_types);
		foreach ($return_array as $screen => $lock_info){
			if (empty($lock_info)){
				unset($return_array[$screen]);
			}
		}
		if($lock_types != 'all' && !empty($return_array) && !(array_key_exists('attributes', $return_array) || array_key_exists('links', $return_array))) $return_array = Array ($return_array);

		return $return_array;

	}//end getLocksInfo()


	/**
	* sets the asset status that is passed
	*
	* @param string		$assetid				the assetid of the asset that we want lock info for
	* @param int		$status					status we want to set to
	* @param boolean	$cascade				if the status change should be cascaded to non dependant children assets, default to false
	* @param string		$current_status_desc	description of the current status
	* @param string		$to_be_status_desc		description of the status to be changed to
	*
	* @return array
	* @access public
	*/
	public function setAssetStatus(Asset $asset, $status, $cascade, $current_status_desc, $to_be_status_desc, $workflow_stream)
	{

		$return_array = Array();
		//if status is being set to something we are not allowed to set then start complaining
		if(!(in_array($status, array_keys($asset->getAvailableStatii()))) && !($status == $asset->status && $cascade) ) {
			$return_array[] = '"'.$to_be_status_desc.'" is not a valid option for the "'.$asset->name.'" asset (#'.$asset->id.') in its current "'.$current_status_desc.'" state';
		return $return_array;
		}

		//lets prepare a hipo running vars
		$job_dir = SQ_SYSTEM_ROOT.'/core/hipo/jobs';
		require_once $job_dir.'/hipo_job_edit_status.inc';
		$init_hipo = new HIPO_Job_Edit_Status();
		$running_vars = Array(
							'assetid'           => $asset->id,
							'new_status'        => $status,
							'dependants_only'   => !$cascade,
						);
		if ($workflow_stream !== 'SQ_USE_DEFAULT') {
			$running_vars['workflow_stream'] = $workflow_stream;
		}

		$init_hipo->setRunningVars($running_vars);
		$init_hipo->_steps = $init_hipo->getInitialStepData();
		if (!$init_hipo->prepare()) {
			$return_array[] = 'Could not initialise HIPO job';
			return $return_array;
		}
		set_error_handler(Array(&$init_hipo, '_errorHandler'));
			$init_hipo->freestyle();
		restore_error_handler();
		$return_array = $init_hipo->getErrors();

		return $return_array;

	}//end setAssetStatus()


	/**
	* Get the info of the Metadata Schema applied to the asset
	*
	* @param string		$assetid	The asset to get the info of the schemas applied to
	* @param boolean    $granted    type of access : TRUE = applied, FALSE = denied
	* @param boolean    $cascades   does this schema cascade to newly created assets?
	*
	* @return array
	* @access public
	*/
	function getMetadataSchema($asset, $granted, $cascades)
	{
		$return_array = Array();

		if (is_null($granted)) {
			$granted = TRUE;
		}

		$mm = $GLOBALS['SQ_SYSTEM']->getMetadataManager();
		$schemas = $mm->getSchemas($asset->id, $granted, $cascades, TRUE);
		if (!empty($schemas)) {
			foreach($schemas as $schema_id) {
				$return_array[$schema_id] = Array();
				$field_children = $GLOBALS['SQ_SYSTEM']->am->getChildren($schema_id, 'metadata_field', FALSE);
				foreach($field_children as $field_id => $field_type) {
					$return_array[$schema_id][$field_id] = $GLOBALS['SQ_SYSTEM']->am->getAllAttributeValues($field_id, $field_type[0]['type_code']);
				}//end foreach
			}//end foreach
		} else {
			$return_array['error'] = 'Asset #'.$asset->id.' does not has any Metadata Schema '.($granted ? 'applied' : 'denied') .' to it';
		}

		return $return_array;

	}//end getMetadataSchema()


	/**
	* Returns an error in JSON format
	*
	* @param string	 $error	The error to send back as JSON
	*
	* @access public
	*/
	function returnError($error)
	{
		// Sanitise the error message
		$error = htmlspecialchars($error);

		// Set our error
		trigger_error($error, E_USER_WARNING);

		// Send error as JSON so they are not confused why the function quit
		$data['error'] = $error;

		// if we are batching  request then  don't send the error to console yet, wait
		// wait for all the remaining request to execute and provide feedback together
		if (isset($this->tmp['batched_request']) && $this->tmp['batched_request']) return $data;

		// Send our data as JSON
		if (isset($data)) {
			$this->returnJSON($data);
		}//end if

	}//end returnError()


	/**
	* Returns the JSON output
	*
	* @param array	 $data	The array of information to return as JSON
	*
	* @access public
	*/
	function returnJSON($data, $type='')
	{
		// Are we allowed to send JSON? OR return JSON when the function is a 'get' OR on error which will be empty type
		if ($this->attr('allow_json') || (!empty($type) && (strpos($type, 'get') === 0))) {
			// Send our data as JSON
			if (!empty($data)) {
				if (!function_exists('json_encode')) {
					require_once 'Services/JSON.php';
					$json = new Services_JSON();
					$output = $json->encode($data);
					if (Services_JSON::isError($output)) {
						$this->returnError($output);
					}
				} else {
					// all ISO characters have to be converted to UTF8 encoding
					if(SQ_CONF_DEFAULT_CHARACTER_SET == 'iso-8859-1')
						$data = $this->array_utf8_encode_recursive($data);
					$output = json_encode($data);
					if (function_exists('json_last_error') && (json_last_error() != JSON_ERROR_NONE)) {
						switch (json_last_error()) {
							case JSON_ERROR_DEPTH:
								$error = 'Maximum stack depth exceeded';
								break;
							case JSON_ERROR_STATE_MISMATCH:
								$error = 'Underflow or the modes mismatch';
								break;
							case JSON_ERROR_CTRL_CHAR:
								$error = 'Unexpected control character found';
								break;
							case JSON_ERROR_SYNTAX:
								$error = 'Syntax error, malformed JSON';
								break;
							case JSON_ERROR_UTF8:
								$error = 'Malformed UTF-8 characters, possibly incorrectly encoded';
								break;
							default:
								$error = 'Unknown error';
								break;
						}
						$this->returnError($error);
					}
				}//end else

				// Send our JSON
				$GLOBALS['SQ_SYSTEM']->setGlobalDefine('SQ_REPLACE_MYSOURCE_LEVEL_KEYWORDS', FALSE);

				echo $output;

			}//end if

		}//end if

	}//end returnJSON()


	/**
	* getRandomFileName get a random filename for the file being uploaded
	*
	* @param string $file_name  The original name of the file
	*
	* @return string
	* @access public
	*/
	function getRandomFilename($file_name)
	{
		$hash_file_name     = hash('md5', $file_name.time());
		$rand_position      = rand(0, strlen($hash_file_name)-6);
		$tmp_file_name      = substr($hash_file_name, $rand_position, 6);

		return $tmp_file_name;

	}//end getRandomFileName()


	/**
	* Purpose : converts our stdClass object to an array
	*
	* @param object	$obj	the stdClass object we need to convert
	*
	* @return array
	* @access public
	*/
	function object2Array($obj)
	{
		$array = Array();
		if (is_object($obj)) {
			foreach ($obj as $key => $value) {
				if (is_object($value)) {
					$array[$key] = $this->object2Array($value);
				} else {
					$array[$key] = $value;
				}
			}
		} else {
			$array = $obj;
		}

		return $array;

	}// end object2Array


	/**
	* Purpose : converts the passed multidimensional array to urf8_encoded
	*
	* @param array	$data	the array whos content we want to utf8 encode
	*
	* @return array
	* @access public
	*/
	function array_utf8_encode_recursive($dat)
	{
		if (is_string($dat)) {
			return utf8_encode($dat);
		}

		if (is_object($dat)) {
			$ovs= get_object_vars($dat);
			$new=$dat;
			foreach ($ovs as $k =>$v) {
				$new->$k= $this->array_utf8_encode_recursive($new->$k);
			}
			return $new;
		}

		if (!is_array($dat)) return $dat;
		$ret = array();
		foreach($dat as $i=>$d) {
			$ret[$i] = $this->array_utf8_encode_recursive($d);
		}

		return $ret;

	}//end array_utf8_encode_recursive


}//end class


?>
