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
* $Id: asset.inc,v 1.789 2013/07/04 04:45:08 akarelia Exp $
*
*/

/**
 * A lot of functions use this, so we'll load it once.
 */
require_once SQ_FUDGE_PATH.'/db_extras/db_extras.inc';


/**
* Asset
*
*	Basic data item in MySource Matrix, from which all pages, users and other items inherit
*
*
* @author  Blair Robertson <blair@squiz.net>
* @version $Revision: 1.789 $
* @package MySource_Matrix
*/
class Asset extends MySource_Object
{

	/**
	* The unique ID for this asset
	* @var int
	*/
	public $id = 0;

	/**
	* The version number of the asset (x.x.x)
	* @var string
	*/
	public $version = '';

	/**
	* String representation of this asset
	* @var string
	*/
	public $name = '';

	/**
	* Short String representation of this asset
	* @var string
	*/
	public $short_name = '';

	/**
	* Current status of the asset (live, under contruction etc)
	* @var int
	*/
	public $status;

	/**
	* Current languages set for the asset
	* @var string
	*/
	public $languages = '';

	/**
	* Current character set of the asset
	* @var string
	*/
	public $charset = '';

	/**
	* Whether this asset is forced to only use one protocol:
	*   '0' = not forced at all
	*   '1' = force secure URLs
	*   '-' = block secure URLs (think '-1')
	* @var boolean
	*/
	public $force_secure = '0';

	/**
	* Timestamp of when this asset was created
	* @var int
	*/
	public $created;

	/**
	* Assetid for the user that created this asset
	* @var int
	*/
	public $created_userid;

	/**
	* Timestamp of when this asset was last updated
	* @var int
	*/
	public $updated;

	/**
	* Assetid for the last user that updated this asset
	* @var int
	*/
	public $updated_userid;

	/**
	* Timestamp of when this asset was last published (ie. made LIVE)
	* @var int
	*/
	public $published;

	/**
	* Assetid for the user that last published this asset
	* @var int
	*/
	public $published_userid;

	/**
	* Timestamp of when this asset was last published (ie. made LIVE)
	* @var int
	*/
	public $status_changed;

	/**
	* Assetid for the user that last published this asset
	* @var int
	*/
	public $status_changed_userid;

	/**
	* Is this asset type able to be cached automatically by the system?
	* @var boolean
	*/
	public $_is_cacheable = FALSE;

	/**
	* The Vars that belong to this asset
	* @var Array('var_name' => Array('attributeid' => $attributeid, 'value' = $value))
	*/
	public $vars = Array();

	/**
	* An array of keywords that this asset makes available for use along with a default replacement
	* @var Array(keyword => name)
	*/
	public $_available_keywords = Array();

	/**
	* The suffix for the data path to be used for public, private, and file versioning
	*
	* @var string
	*/
	public $data_path_suffix = '';

	/**
	* The current data path for this asset
	* This always points to the restricted directory but may also
	* point to the .sq_system directory within it
	*
	* @var string
	*/
	public $data_path = '';

	/**
	* The current unrestrcited data path for this asset
	* This always points to the unrestricted directory
	* Only use this path for storage if the web server should serve the file
	*
	* @var string
	*/
	public $data_path_public = '';

	/**
	* Whether this asset has serialise attributes or not, basically a performance var
	* when TRUE the vars a looped over in _loadVars() and if they are type 'serialise' unserialised
	*
	* @var bool
	*/
	public $_ser_attrs = FALSE;


	/**
	* Constructor
	*
	* @param int	$assetid	the asset id to be loaded
	*
	* @return object
	* @access public
	*/
	function Asset($assetid=0)
	{
		$this->MySource_Object();

		if ($assetid) {
			$this->load($assetid);
		} else {
			$this->_loadVars();
		}

	}//end constructor


	/**
	* Create this asset
	*
	* The return value will be:<br/>
	* <ul>
	*   <li>FALSE if the asset was not created</li>
	*   <li>the ID of the newly created link if the asset and intital link were created</li>
	* </ul>
	*
	* @param array	&$link	information used to create the initial link<br/>
	* <pre>
	* Array ('asset'         => [ref major asset to create link under],
	*        'link_type'     => SQ_LINK_?,
	*        'value'         => [link value],
	*        'sort_order'    => [link sort order],
	*        'is_dependant'  => [0|1],
	*        'is_exclusive'  => [0|1],
	*		 'is_locked'	 => [0|1]
	*        )
	* </pre>
	*
	* @return mixed int|boolean
	* @access public
	*/
	public function create(Array &$link)
	{
		if ($this->id) {
			trigger_localised_error('CORE0268', E_USER_WARNING, $this->type());
			return;
		}

		$GLOBALS['SQ_SYSTEM']->changeDatabaseConnection('db2');
		$GLOBALS['SQ_SYSTEM']->doTransaction('BEGIN');
		$db = MatrixDAL::getDb();

		if ($GLOBALS['SQ_SYSTEM']->runLevelEnables(SQ_SECURITY_DATA_VALIDATION)) {
			// perform any validation before starting the create process
			if (!$this->_preCreateCheck($link)) {
				return $this->_abortCreate();
			}

			if (!empty($link)) {
				// make sure the initial link information is passed in
				assert_isset_array_index($link, 'asset', 'Cannot create asset without an asset to link to');
				assert_isset_array_index($link, 'link_type', 'Cannot create asset without a link type');
				assert_not_empty(($link['link_type'] & SQ_SC_LINK_SIGNIFICANT), 'Cannot create asset with an insignificant link type');
			}
		}

		// disable the triggering system. we do not want to broadcast any nested events, only our own
		$trigger_level_changed = FALSE;
		if ($GLOBALS['SQ_SYSTEM']->runLevelEnables(SQ_SECURITY_TRIGGERS)) {
			$GLOBALS['SQ_SYSTEM']->setRunLevel($GLOBALS['SQ_SYSTEM']->getRunLevel() - SQ_SECURITY_TRIGGERS);
			$trigger_level_changed = TRUE;
		}

		// let the system know that we are creating this asset so we dont
		// do any updating of versions etc. until we are done
		$this->_tmp['__creating__'] = TRUE;

		$assetid = MatrixDAL::executeOne('core', 'seqNextVal', Array('seqName' => 'sq_ast_seq'));
		$name   = ucwords(str_replace('_', ' ', $this->type())).' #'.$assetid;
		$now    = time();
		$userid = $GLOBALS['SQ_SYSTEM']->currentUserId();
		$initial_version = '0.0.1';
		$initial_status = SQ_STATUS_UNDER_CONSTRUCTION;
		if (array_get_index($link, 'is_dependant') && ($link['asset']->status != SQ_STATUS_UNDER_CONSTRUCTION)) {
			$wfm = $GLOBALS['SQ_SYSTEM']->getWorkflowManager();
			if (0 == count($wfm->getSchemas($link['asset']->id, TRUE))) {
				// no workflow for parent
				 $initial_status = $link['asset']->status;
			}
		}

		try {
			$now_iso = ts_iso8601($now);
			$bind_vars = Array(
							'assetid'				=> $assetid,
							'version'				=> $initial_version,
							'type_code'				=> $this->type(),
							'name'					=> $name,
							'short_name'			=> $name,
							'status'				=> $initial_status,
							'created'				=> $now_iso,
							'created_userid'		=> $userid,
							'updated'				=> $now_iso,
							'updated_userid'		=> $userid,
							'published'				=> NULL,
							'published_userid'		=> NULL,
							'status_changed'		=> $now_iso,
							'status_changed_userid'	=> $userid,
						 );
			MatrixDAL::executeQuery('core', 'createAsset', $bind_vars);
		} catch (Exception $e) {
			throw new Exception('Unable to create new asset  "'.$name.'" (# "'.$assetid.'") due to database error: '.$e->getMessage());
		}

		$this->id                    = $assetid;
		$this->version               = $initial_version;
		$this->name                  = $name;
		$this->short_name            = $name;
		$this->status                = $initial_status;
		$this->created               = $now;
		$this->created_userid        = $userid;
		$this->updated               = $now;
		$this->updated_userid        = $userid;
		$this->published             = NULL;
		$this->published_userid      = NULL;
		$this->status_changed        = $now;
		$this->status_changed_userid = $userid;

		// Get ready to log what happens
		$ms = $GLOBALS['SQ_SYSTEM']->getMessagingService();
		$ms->openLog();

		// Register ourselves with asset mgr so getAsset doesn't load a us again from the DB
		$GLOBALS['SQ_SYSTEM']->am->rememberAsset($this);

		// Acquire permissions lock on ourselves so permissions can be cascaded
		if ($GLOBALS['SQ_SYSTEM']->runLevelEnables(SQ_SECURITY_LOCKING)) {
			if (!$GLOBALS['SQ_SYSTEM']->am->acquireLock($this->id, 'permissions')) {
				return $this->_abortCreate($trigger_level_changed);
			}
		}

		// Send a message to tell everyone the good news
		$message_body = 'New '.$this->type().' "'.$this->name.'" created';
		$msg_reps = Array(
						'type_code'		=> $this->type(),
						'asset_name'	=> $this->name,
					);
		$message = $ms->newMessage(Array(), 'asset', $msg_reps);
		$message->parameters['assetid'] = $this->id;
		$message->send();

		// Save the attribute values that have been stored temporarily
		$GLOBALS['SQ_SYSTEM']->setRunLevel($GLOBALS['SQ_SYSTEM']->getRunLevel() & SQ_RUN_LEVEL_FORCED);
		$ok = $this->saveAttributes(TRUE);
		$GLOBALS['SQ_SYSTEM']->restoreRunLevel();
		if (!$ok) {
			return $this->_abortCreate($trigger_level_changed);
		}

		$this->_loadVars();

		if (!empty($link)) {
			if (!isset($link['value'])) $link['value'] = '';
			if (!isset($link['sort_order'])) {
				$link['sort_order'] = -1;
			}
			if (!isset($link['is_dependant'])) {
				$link['is_dependant'] = 0;
			}
			if (!isset($link['is_exclusive'])) {
				$link['is_exclusive'] = 0;
			}
			if (!isset($link['is_locked'])) {
				$link['is_locked'] = 0;
			}

			if ($GLOBALS['SQ_SYSTEM']->runLevelEnables(SQ_SECURITY_LOCKING)) {
				// Acquire the links lock on the new parent
				$lock_result = $GLOBALS['SQ_SYSTEM']->am->acquireLock($link['asset']->id, 'links');
				if (!$lock_result) {
					return $this->_abortCreate($trigger_level_changed);
				}
				$parent_was_locked = ($lock_result == 2);

				// Acquire links lock on ourselves
				// If we are creating a dependant link, we need to make sure this new
				// asset is locked in the same chain as the parent
				// (ie. has the same source_assetid)
				$source_assetid = 0;
				if ($link['is_dependant']) {
					$lock = $GLOBALS['SQ_SYSTEM']->am->getLockInfo($link['asset']->id, 'links');
					$source_assetid = $lock['source_assetid'];
				}
				if (!$GLOBALS['SQ_SYSTEM']->am->acquireLock($this->id, 'links', $source_assetid)) {
					return $this->_abortCreate($trigger_level_changed);
				}
			}

			if ($GLOBALS['SQ_SYSTEM']->runLevelEnables(SQ_SECURITY_INTEGRITY)) {
				// Cascade various components from parent asset to new child
				// unless the parent is the root folder or system management folder
				$this->_tmp[__CLASS__.'_in_create_cascading'] = TRUE;
				$GLOBALS['SQ_SYSTEM']->setRunLevel($GLOBALS['SQ_SYSTEM']->getRunLevel() & SQ_RUN_LEVEL_FORCED);

					if (!$link['asset']->cloneComponents($this, Array('permissions', 'metadata_schemas', 'workflow', 'content_tags', 'roles'))) {
							$GLOBALS['SQ_SYSTEM']->restoreRunLevel();
							return $this->_abortCreate($trigger_level_changed);
					}

				$GLOBALS['SQ_SYSTEM']->restoreRunLevel();
				unset($this->_tmp[__CLASS__.'_in_create_cascading']);
			}

			// Create the link to the parent asset
			$linkid = $link['asset']->createLink($this, $link['link_type'], $link['value'], $link['sort_order'], $link['is_dependant'], $link['is_exclusive'], FALSE, $link['is_locked']);
			if (!$linkid) {
				if (!$parent_was_locked) {
					$GLOBALS['SQ_SYSTEM']->am->releaseLock($link['asset']->id, 'links');
				}
				return $this->_abortCreate($trigger_level_changed);
			}
			// bug fix #3877 Matrix will create and retain links in DB if asset fails to get created
			// by this point ^ we have linkid available since the link has been created.
			// any further calls to _abortCreate() should include linkid so the we are not left
			// with unwanted rubissh in sq_ast_lnk and sq_ast_lnk_tree tables

			if ($GLOBALS['SQ_SYSTEM']->runLevelEnables(SQ_SECURITY_LOCKING)) {

				if (!$GLOBALS['SQ_SYSTEM']->am->releaseLock($this->id, 'permissions')) {
					return $this->_abortCreate($trigger_level_changed, $linkid);
				}

				// Get the link so we can find out if it was dependent
				// (link details may have changed during createLink)
				$new_link = $GLOBALS['SQ_SYSTEM']->am->getLinkById($linkid, $this->id, 'minor');

				// Release locks, unless the parent was locked before we were created
				if (!$parent_was_locked) {
					if (!$GLOBALS['SQ_SYSTEM']->am->releaseLock($link['asset']->id, 'links')) {
						return $this->_abortCreate($trigger_level_changed, $linkid);
					}
				} else if ($new_link['is_dependant']) {
					// The parent was locked before we created this asset and the new asset
					// is dependantly linked, so make sure we have all the locks of our parent
					$parent_lock_info = $GLOBALS['SQ_SYSTEM']->am->getLockInfo($link['asset']->id, 'all');
					foreach (array_values($parent_lock_info) as $lock_details) {
						if (empty($lock_details)) continue;
						if (!$GLOBALS['SQ_SYSTEM']->am->acquireLock($this->id, $lock_details['lock_type'], $lock_details['source_assetid'])) {
							return $this->_abortCreate($trigger_level_changed, $linkid);
						}
					}
				}
				if (!$new_link['is_dependant']) {
					if (!$GLOBALS['SQ_SYSTEM']->am->releaseLock($this->id, 'links')) {
						return $this->_abortCreate($trigger_level_changed, $linkid);
					}
				}

			}//end if locking subsystem enabled

		}//end if creating link

		// Re-enable the triggers, if they were disabled here
		if ($trigger_level_changed) {
			$GLOBALS['SQ_SYSTEM']->restoreRunLevel();
		}

		// Before firing the "Before Asset Created" event, turn off locking checks
		$modified_runlevel = FALSE;
		if ($GLOBALS['SQ_SYSTEM']->runLevelEnables(SQ_SECURITY_LOCKING)) {
			$GLOBALS['SQ_SYSTEM']->setRunLevel($GLOBALS['SQ_SYSTEM']->getRunLevel() - SQ_SECURITY_LOCKING);
			$modified_runlevel = TRUE;
		}

		// Fire the event, abort if the event fails
		$event_result = $GLOBALS['SQ_SYSTEM']->broadcastTriggerEvent('trigger_event_before_asset_created', $this);

		if ($modified_runlevel) {
			$GLOBALS['SQ_SYSTEM']->restoreRunLevel();
		}

		if (!$event_result) return $this->_abortCreate(FALSE, $linkid);


		if ($GLOBALS['SQ_SYSTEM']->runLevelEnables(SQ_SECURITY_INTEGRITY)) {
			// Perform any additional processing required, such as creating other assets
			if (!$this->_createAdditional($link)) {
				return $this->_abortCreate(FALSE, $linkid);
			}
		}

		// If for some unknown reason they don't have write access to the asset
		// from the cascaded permissions of the parent (goodness knows why - you
		// shouldn't be here in the first place!), add an explicit permission
		if (!$this->writeAccess()) {
			// note, if the user is unknown (NULL or 0), the permissions WILL NOT BE SET, and the creation will not fail
			$current_user = $GLOBALS['SQ_SYSTEM']->currentUserId();
			$GLOBALS['SQ_SYSTEM']->setRunLevel(SQ_RUN_LEVEL_FORCED);
			if (!empty($current_user) && !$GLOBALS['SQ_SYSTEM']->am->setPermission($this->id, $current_user, SQ_PERMISSION_WRITE, TRUE)) {
				$GLOBALS['SQ_SYSTEM']->restoreRunLevel();
				return $this->_abortCreate(FALSE, $linkid);
			}
			$GLOBALS['SQ_SYSTEM']->restoreRunLevel();
		}

		unset($this->_tmp['__creating__']);
		if (!$this->_updated()) return $this->_abortCreate(FALSE, $linkid);
		$GLOBALS['SQ_SYSTEM']->doTransaction('COMMIT');
		$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();

		// because we are inheriting statuses, perhaps we are in safe edit now?
		if ($this->status == SQ_STATUS_EDITING) {
			$this->saveSystemVersion();
		}

		// fire the 'Asset Created' event
		$GLOBALS['SQ_SYSTEM']->broadcastTriggerEvent('trigger_event_asset_created', $this);
		$em = $GLOBALS['SQ_SYSTEM']->getEventManager();

		// create a list of asset vars that have changed
		// only published and published_userid is
		$vars = Array(
					'assetid',
					'version',
					'type_code',
					'name',
					'short_name',
					'status',
					'created',
					'created_userid',
					'updated',
					'updated_userid',
					'status_changed',
					'status_changed_userid',
				);

		$em->broadcastEvent($this, 'AssetCreate', $vars);
		$ms->closeLog();

		if (!empty($link)) {
			return (int) $linkid;
		} else {
			return TRUE;
		}

	}//end create()


	/**
	* Perform any pre-validation required during the creation of this asset
	*
	* For example, checking that attributes are set should bee performed in this method
	*
	* @param array	&$link	information used to create the initial link<br/>
	* <pre>
	* Array (
	*   'asset'        => [ref major asset to create link under],
	*   'link_type'    => SQ_LINK_?,
	*   'value'        => [link value],
	*   'sort_order'   => [link sort order],
	*   'is_dependant' => [0|1],
	*   'is_exclusive' => [0|1],
	* )
	* </pre>
	*
	* @return boolean
	* @access private
	*/
	protected function _preCreateCheck(Array &$link)
	{
		return TRUE;

	}//end _preCreateCheck()


	/**
	* Perform any additional processing required during the creation of this asset
	*
	* For example, creating other child assets should be performed in this method
	*
	* @param array	&$link	information used to create the initial link<br/>
	* <pre>
	* Array (
	*   'asset'        => [ref major asset to create link under],
	*   'link_type'    => SQ_LINK_?,
	*   'value'        => [link value],
	*   'sort_order'   => [link sort order],
	*   'is_dependant' => [0|1],
	*   'is_exclusive' => [0|1],
	* )
	* </pre>
	*
	* @return boolean
	* @access private
	*/
	protected function _createAdditional(Array &$link)
	{
		return TRUE;

	}//end _createAdditional()


	/**
	* Abort the creation of this asset
	*
	* The create log and DB transaction will also be aborted
	*
	* @param boolean	$trigger_level_changed	if it's FALSE, restore runlevel
	*
	* @return boolean
	* @access private
	*/
	protected function _abortCreate($trigger_level_changed=FALSE, $linkid = 0)
	{
		if ($trigger_level_changed) {
			$GLOBALS['SQ_SYSTEM']->restoreRunLevel();
		}

		$ms = $GLOBALS['SQ_SYSTEM']->getMessagingService();
		$ms->abortLog();
		if ($linkid != 0) {
			// Get the asset's link and its immediate children info before we rollback
			$link = $GLOBALS['SQ_SYSTEM']->am->getLinkById($linkid);

			$children_assets = Array();
			if (isset($link['minorid'])) {
				$children_assets = $GLOBALS['SQ_SYSTEM']->am->getChildren($link['minorid'], '', TRUE, NULL, NULL, NULL, TRUE, NULL, 1);
			}
		}

		$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
		$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();

		if (!empty($link) && isset($link['linkid']) && isset($link['majorid'])) {
			// Clear the asset's link left on the link table
			$GLOBALS['SQ_SYSTEM']->am->deleteAssetLinkByLink($link, TRUE, TRUE);

			// If there were children created by this asset successfully (that didn't got aborted),
			// even though those childern will be rolledback when parent asset aborts itself,
			// however those children will leave links in link table
			foreach($children_assets as $child_assetid => $val) {
				$child_link = $GLOBALS['SQ_SYSTEM']->am->getLinkByAsset($link['minorid'], $child_assetid);

				if (isset($child_link['linkid'])) {
					$child_link['majorid'] = $link['minorid'];
					$child_link['major_type_code'] = $this->type();
					$GLOBALS['SQ_SYSTEM']->am->deleteAssetLinkByLink($child_link, TRUE, TRUE);
				}
			}//end foreach

		}//end if

		$GLOBALS['SQ_SYSTEM']->am->forgetAsset($this);
		unset($this->_tmp['__creating__']);
		if ($this->id) {
			@$GLOBALS['SQ_SYSTEM']->am->releaseLock($this->id, 'all');
		}
		$this->id = 0;
		return FALSE;

	}//end _abortCreate()


	/**
	* Load the asset represented by the passed asset ID
	*
	* @param int	$assetid	the asset ID to be loaded
	*
	* @return void
	* @access public
	*/
	public function load($assetid)
	{
		$db = MatrixDAL::getDb();

		// OK, the first thing to do is check we actually exist :)
		$sql = 'SELECT assetid, type_code, version, name, short_name, status, languages,
				charset, force_secure, created, created_userid, updated, updated_userid,
				published, published_userid, status_changed, status_changed_userid
				FROM '.SQ_TABLE_RUNNING_PREFIX.'ast ';
		$where = 'assetid = :assetid';
		$where = $GLOBALS['SQ_SYSTEM']->constructRollbackWhereClause($where);

		try {
			$query = MatrixDAL::preparePdoQuery($sql.$where);
			MatrixDAL::bindValueToPdo($query, 'assetid', $assetid);
			$result = MatrixDAL::executePdoAssoc($query);
			if (!empty($result)) {
				$result = $result[0];
			}
		} catch (Exception $e) {
			throw new Exception('Unable to get asset info for asset ID #'.$assetid.' due to database error: '.$e->getMessage());
		}

		if (empty($result)) {
			trigger_localised_error('SYS0087', E_USER_WARNING, $assetid);
			return;
		}

		$this->id                    = $result['assetid'];
		$type_code                   = $result['type_code'];
		$this->version               = $result['version'];
		$this->name                  = $result['name'];
		$this->short_name            = $result['short_name'];
		$this->status                = $result['status'];
		$this->languages             = $result['languages'];
		$this->charset               = $result['charset'];
		$this->force_secure          = $result['force_secure'];
		$this->created               = iso8601_ts($result['created']);
		$this->created_userid        = $result['created_userid'];
		$this->updated               = iso8601_ts($result['updated']);
		$this->updated_userid        = $result['updated_userid'];
		$this->published             = $result['published'];
		$this->published_userid      = $result['published_userid'];
		$this->status_changed        = $result['status_changed'];
		$this->status_changed_userid = $result['status_changed_userid'];

		if (!is_null($this->status_changed)) {
			$this->status_changed = iso8601_ts($this->status_changed);
		}
		if (!is_null($this->published)) {
			$this->published = iso8601_ts($this->published);
		}
		unset($result);

		// make sure the asset we are loading is of the same type as our class
		if ($type_code != $this->type()) {
			trigger_localised_error('SYS0089', E_USER_WARNING, $assetid, $this->type());
			$this->id                    = 0;
			$this->created               = NULL;
			$this->created_userid        = NULL;
			$this->updated               = NULL;
			$this->updated_userid        = NULL;
			$this->published             = NULL;
			$this->published_userid      = NULL;
			$this->status_changed        = NULL;
			$this->status_changed_userid = NULL;
			return;
		}

		if ($this->useSystemVersion()) {
			$this->_loadDataPaths();
			if (!$this->loadSystemVersion()) {
				trigger_localised_error('SYS0088', E_USER_WARNING, $assetid);
				return;
			}
			$this->_loadDataPaths();
		} else {
			$this->_loadVars();
		}

	}//end load()


	/**
	* Setup the data paths for the asset
	*
	* Also takes into account if we are using a system version
	*
	* @return void
	* @access private
	*/
	public function _loadDataPaths()
	{
		$this->data_path_suffix = asset_data_path_suffix($this->type(), $this->id);
		$this->data_path = SQ_DATA_PATH.'/private/'.$this->data_path_suffix;
		$this->data_path_public = SQ_DATA_PATH.'/public/'.$this->data_path_suffix;
		if ($this->useSystemVersion()) {
			$this->data_path .= '/.sq_system';
		}

	}//end _loadDataPaths()


	/**
	* Load the vars for this asset/ asset type into the vars array
	*
	* If the asset has no asset ID, this function just loads the default values for the asset type.
	*
	* @return void
	* @access private
	*/
	protected function _loadVars()
	{
		// let's setup the data path
		$this->_loadDataPaths();

		// Right, now we need to get any values that this asset has customised
		$this->vars = Array();

		$sql = '';
		// if we have an id then we need to load with current vars
		if ($this->id) {
			// PURPOSLY DONT ADD EXTRA CLAUSES FOR ASSET_ATTRIBUTE_VALUE BECAUSE WE WONT GET
			// DEFAULT VALUES IF WE DO
			$sql = 'SELECT atr.name, atr.attrid, atr.type, COALESCE(v.custom_val, atr.default_val) AS value, atr.is_contextable, v.use_default
					FROM (sq_ast_attr atr
					  LEFT OUTER JOIN (SELECT * FROM '.SQ_TABLE_RUNNING_PREFIX.'ast_attr_val WHERE contextid = :contextid) v
						ON (atr.attrid = v.attrid AND v.assetid = :assetid'
						.$GLOBALS['SQ_SYSTEM']->constructRollbackWhereClause('', 'v', 'AND').'))
						WHERE atr.type_code = :type_code';
		} else {
			// else just load all defaults
			$sql = 'SELECT atr.name, atr.attrid, atr.type, atr.default_val AS value, atr.is_contextable, \'1\' as use_default
					FROM sq_ast_attr atr
					WHERE atr.type_code = :type_code';
		}// end if

		try {
			$query = MatrixDAL::preparePdoQuery($sql);
			MatrixDAL::bindValueToPdo($query, 'type_code', $this->type(), PDO::PARAM_STR);
			// Only bind it if we are using the first query, not binding when going in the else above
			if ($this->id) {
				MatrixDAL::bindValueToPdo($query, 'contextid', $GLOBALS['SQ_SYSTEM']->getContextId(), PDO::PARAM_INT);
				MatrixDAL::bindValueToPdo($query, 'assetid',   $this->id, PDO::PARAM_STR);
			}//end if
			$result = MatrixDAL::executePdoGroupedAssoc($query);

			if (empty($result)) {
				$this->vars = Array();
			} else {
				$this->vars = $result;
				foreach (array_keys($this->vars) as $name) {
					$this->vars[$name] = $this->vars[$name][0];
					unset($this->vars[$name][0]);
				}
			}

			unset($result);
		} catch (Exception $e) {
			throw new Exception('Unable to load variables of asset #'.$this->id.' ('.$this->type().') due to database error: '.$e->getMessage());
		}

		if ($this->_ser_attrs && $this->vars) {
			for (reset($this->vars); NULL !== ($name = key($this->vars)); next($this->vars)) {
				if ($this->vars[$name]['type'] != 'serialise') {
					continue;
				}
				$this->vars[$name]['value'] = @unserialize($this->vars[$name]['value']);
			}
		}

	}//end _loadVars()


	/**
	* Reload this asset from the DB, clearing all its local caches
	*
	* @return void
	* @access public
	*/
	public function reload()
	{
		$this->_tmp = Array();
		$this->load($this->id);

	}//end reload()


	/**
	* Determine whether this asset can have its last significant link removed
	*
	* This function is most commonly called when moving the asset to the trash.
	*
	* @return boolean
	* @access public
	*/
	public function canDelete()
	{
		return TRUE;

	}//end canDelete()


	/**
	* Delete this asset from the trash
	*
	* Note that the asset <i>MUST</i> be in the trash to delete it <i>FROM</i> the trash.
	*
	* @param boolean	$release_lock	should we realease the lock after deleting
	* @param boolean	$check_locked	Check if the link is locked or not
	*
	* @return boolean
	* @access public
	*/
	public function delete($release_lock=TRUE, $check_locked=TRUE)
	{
		// check that we are in the trash
		if (!$GLOBALS['SQ_SYSTEM']->am->assetInTrash($this->id)) {
			trigger_localised_error('SYS0103', E_USER_WARNING, $this->name, $this->id);
			return FALSE;
		}

		$GLOBALS['SQ_SYSTEM']->changeDatabaseConnection('db2');
		$db = MatrixDAL::getDb();
		$GLOBALS['SQ_SYSTEM']->doTransaction('BEGIN');

		$links = $GLOBALS['SQ_SYSTEM']->am->getLinks($this->id, SQ_SC_LINK_ALL, '', TRUE, 'minor');
		foreach ($links as $link) {
			$major = $GLOBALS['SQ_SYSTEM']->am->getAsset($link['majorid'], $link['major_type_code']);

			// We've got almost everything we need to delete the link,
			// just need to add the current minor code and id to the array.
			// This saves deleteLink from having to get it from the database again.
			$link['minor_type_code'] = $this->type();
			$link['minorid'] = $this->id;

			if (!is_null($major)) {
				$major->deleteLink($link['linkid'], $check_locked, $link);
			} else {
				trigger_localised_error('SYS0128', E_USER_WARNING, $link['majorid']);
				$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
				$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();
				return FALSE;
			}
		}

		if ($release_lock) {
			// we are about to go to the big asset manager in the sky,
			// so we wont be needing worldly things such as locks
			if (!$GLOBALS['SQ_SYSTEM']->am->releaseLock($this->id, 'all')) {
				trigger_localised_error('SYS0131', E_USER_WARNING, $this->name);
				$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
				$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();
				return FALSE;
			}
		}

		// remove web paths.
		// saveWebPaths() calls updateLookups() for us, we don't need to
		// do it again after this.
		if (!$this->saveWebPaths(Array())) {
			trigger_localised_error('SYS0133', E_USER_WARNING, $this->name);
			$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
			$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();
			return FALSE;
		}

		$bind_vars['assetid'] = $this->id;
		try {
			// delete related asset data from the DB tables
			$result = MatrixDAL::executeQuery('core', 'assetDeleteAsset', $bind_vars);
			$result = MatrixDAL::executeQuery('core', 'assetDeleteAssetAttrVal', $bind_vars);
			$result = MatrixDAL::executeQuery('core', 'assetDeleteAssetAttrUniqVal', $bind_vars);
			$result = MatrixDAL::executeQuery('core', 'assetDeleteAssetPerm', $bind_vars);
			$result = MatrixDAL::executeQuery('core', 'assetDeleteAssetRole', $bind_vars);
		} catch (Exception $e) {
			throw new Exception("Failed to delete asset (#$this->id): ".$e->getMessage());
		}

		// delete any metadata values this asset might have
		$mm = $GLOBALS['SQ_SYSTEM']->getMetadataManager();
		if (!$mm->purgeMetadata($this->id)) {
			$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
			$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();
			return FALSE;
		}

		// delete any workflow values that this asset might have
		$wm = $GLOBALS['SQ_SYSTEM']->getWorkflowManager();
		if (!$wm->purgeWorkflow($this->id)) {
			$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
			$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();
			return FALSE;
		}

		// now try and clear the data directories for this asset
		require_once SQ_FUDGE_PATH.'/general/file_system.inc';
		if (is_dir($this->data_path)) {
			if (!delete_directory($this->data_path)) {
				trigger_localised_error('SYS0151', E_USER_WARNING, $this->name);
				$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
				$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();
				return FALSE;
			}
		}

		if (is_dir($this->data_path_public)) {
			if (!delete_directory($this->data_path_public)) {
				trigger_localised_error('SYS0151', E_USER_WARNING, $this->name);
				$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
				$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();
				return FALSE;
			}
		}

		$GLOBALS['SQ_SYSTEM']->doTransaction('COMMIT');
		$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();

		$em = $GLOBALS['SQ_SYSTEM']->getEventManager();
		$em->broadcastEvent($this, 'assetDeleted', Array());
		$GLOBALS['SQ_SYSTEM']->am->forgetAsset($this, TRUE);

		return TRUE;

	}//end delete()


	/**
	* Determine if the current user be shown the system version or not
	*
	* You would want to show the system version if the asset is in safe edit and the
	* user does not have write access
	*
	* @return boolean
	* @access public
	*/
	public function useSystemVersion()
	{
		if (!isset($this->_tmp['use_system_version'])) {
			if (!($this->status & SQ_SC_STATUS_SAFE_EDITING)) {
				$this->_tmp['use_system_version'] = FALSE;
			} else {
				if ($GLOBALS['SQ_SYSTEM']->getGlobalDefine('force_system_version')) {
					$this->_tmp['use_system_version'] = TRUE;
				} else {
					if (SQ_ROLLBACK_VIEW) {
						$usv = (!$this->checkAccess(SQ_PERMISSION_WRITE, ''));
					} else {
						$usv = (!$this->writeAccess(''));
					}//end if
					if ($usv && !$GLOBALS['SQ_SYSTEM']->userPublic()) {
						$wfm = $GLOBALS['SQ_SYSTEM']->getWorkflowManager();
						$publishers = $wfm->whoCanPublish($this->id);
						if (!empty($publishers) && in_array($GLOBALS['SQ_SYSTEM']->currentUserId(), $publishers)) {
							$usv = FALSE;
						}
					}
					$this->_tmp['use_system_version'] = $usv;
				}
			}
		}
		return $this->_tmp['use_system_version'];

	}//end useSystemVersion()


	/**
	* Load the asset from the system version
	*
	* The system version is saved when the asset status becomes SQ_STATUS_LIVE.
	*
	* @return boolean
	* @access public
	*/
	public function loadSystemVersion()
	{
		require_once SQ_FUDGE_PATH.'/general/file_system.inc';

		// make sure our data directory exists
		$contextid = $GLOBALS['SQ_SYSTEM']->getContextId();
		$filename = '.object_data'.(($contextid === 0) ? '' : '.'.$contextid);
		if (file_exists($this->data_path.'/'.$filename) === TRUE) {
			$use_filename = $this->data_path.'/'.$filename;
		} else if (file_exists($this->data_path.'/.object_data') === TRUE) {
			// The context mustn't have existed when we saved the system
			// version, so use the default context version instead...?
			$use_filename = $this->data_path.'/.object_data';
		} else {
			trigger_localised_error('SYS0159', E_USER_WARNING, $this->id, $this->data_path);
			return FALSE;
		}

		$real_status = $this->status;

		foreach (get_object_vars(unserialize(file_to_string($use_filename))) as $key => $value) {
			$this->$key = $value;
		}
		$this->status = $real_status;
		return TRUE;

	}//end loadSystemVersion()


	/**
	* Save a copy of this asset at this point in time
	*
	* This copy gets used when the asset is live and the user does not have write access
	*
	* @return boolean
	* @access public
	*/
	function saveSystemVersion()
	{
		require_once SQ_FUDGE_PATH.'/general/file_system.inc';

		// make sure our data directories exists
		if (!create_directory($this->data_path)) {
			trigger_localised_error('CORE0049', E_USER_WARNING, $this->name);
			return FALSE;
		}

		// make sure our system directories exists
		if (!create_directory($this->data_path.'/.sq_system')) {
			trigger_localised_error('CORE0050', E_USER_WARNING, $this->name);
			return FALSE;
		}

		// make sure there is nothing in the system directories
		if (!clear_directory($this->data_path.'/.sq_system')) {
			trigger_localised_error('CORE0050', E_USER_WARNING, $this->name);
			return FALSE;
		}

		// save the object for later (in the restricted directory)
		// The only thing that varies at the moment is attributes, so we
		// can reload the vars in each context instead of completely reloading
		// the asset
		$current_vars = $this->vars;
		$contextids = array_keys($GLOBALS['SQ_SYSTEM']->getAllContexts());
		foreach ($contextids as $contextid) {
			// Reload the vars for this context
			$GLOBALS['SQ_SYSTEM']->changeContext($contextid);
			$this->_loadVars();

			$filename = '.object_data'.(($contextid === 0) ? '' : '.'.$contextid);
			if (!string_to_file(serialize($this), $this->data_path.'/.sq_system/'.$filename)) {
				trigger_localised_error('CORE0051', E_USER_WARNING, $this->name);
				return FALSE;
			}

			$GLOBALS['SQ_SYSTEM']->restoreContext();
		}//end foreach

		// Put it back how it was
		$this->vars = $current_vars;

		// move all the other files in our data directories
		// into the new system directories for later use
		$files_to_copy = list_files($this->data_path);
		foreach ($files_to_copy as $filename) {
			if (!copy_file($this->data_path.'/'.$filename, $this->data_path.'/.sq_system/'.$filename)) {
				trigger_localised_error('SYS0166', E_USER_WARNING, $this->name, $filename);
				return FALSE;
			}
		}

		//  we need to save the current notice links
		if(!$this->saveNoticeLinksForSystemVersion()) return FALSE;

		return TRUE;

	}//end saveSystemVersion()


	/**
	* Clear the directory for saving the system version
	*
	* @return boolean
	* @access public
	*/
	public function clearSystemVersion()
	{
		require_once SQ_FUDGE_PATH.'/general/file_system.inc';

		// make sure our data directory exists
		if (!create_directory($this->data_path)) {
			trigger_localised_error('SYS0148', E_USER_WARNING, $this->name);
			return FALSE;
		}

		// make sure our system directory exists
		if (!create_directory($this->data_path.'/.sq_system')) {
			trigger_localised_error('SYS0299', E_USER_WARNING, $this->name);
			return FALSE;
		}

		if (!clear_directory($this->data_path.'/.sq_system')) {
			trigger_localised_error('SYS0147', E_USER_WARNING, $this->name);
			return FALSE;
		}

		return TRUE;

	}//end clearSystemVersion()


	/**
	* Replace the editing version with the system (live) version
	*
	* This is what happens when you cancel a safe edit. The database version (latest and greatest) is replaced
	* with the system version that is currently being show to non-editors.
	*
	* @return boolean
	* @access public
	*/
	public function revertToSystemVersion()
	{
		$GLOBALS['SQ_REVERT_TO_SYSTEM_VERSION'] = TRUE;
		require_once SQ_FUDGE_PATH.'/general/file_system.inc';

		// copy over the old files we stored in the system directory
		$files_to_copy = list_files($this->data_path.'/.sq_system');
		foreach ($files_to_copy as $filename) {
			// skip hidden files
			if (strpos($filename, '.') === 0) continue;

			if (!copy_file($this->data_path.'/.sq_system/'.$filename, $this->data_path.'/'.$filename)) {
				trigger_localised_error('SYS0162', E_USER_WARNING, $this->name, $filename);
				$GLOBALS['SQ_REVERT_TO_SYSTEM_VERSION'] = FALSE;
				return FALSE;
			}
		}

		// load our object data in as it was before
		$current_contextid = $GLOBALS['SQ_SYSTEM']->getContextId();
		$contextids = array_keys($GLOBALS['SQ_SYSTEM']->getAllContexts());
		foreach ($contextids as $contextid) {
			if ($contextid !== $current_contextid) {
				$GLOBALS['SQ_SYSTEM']->changeContext($contextid);
				$contexted_asset = $GLOBALS['SQ_SYSTEM']->am->getAsset($this->id);
			} else {
				$contexted_asset = $this;
			}


			$filename = '.object_data'.(($contextid === 0) ? '' : '.'.$contextid);
			if (is_file($this->data_path.'/.sq_system/'.$filename)) {
				$old_version = unserialize(file_to_string($this->data_path.'/.sq_system/'.$filename));
			} else if (is_file($this->data_path.'/.sq_system/.object_data')) {
				// The context mustn't have existed when we saved the system
				// version, so use the default context version instead...?
				$old_version = unserialize(file_to_string($this->data_path.'/.sq_system/.object_data'));
			} else {
				trigger_localised_error('SYS0163', E_USER_WARNING, $this->name);
				$GLOBALS['SQ_REVERT_TO_SYSTEM_VERSION'] = FALSE;
				return FALSE;
			}

			// update all the vars
			foreach ($old_version->vars as $var_name => $var_data) {
				if (!$contexted_asset->setAttrValue($var_name, $var_data['value'], TRUE)) {
					trigger_localised_error('SYS0165', E_USER_WARNING, $this->name, $var_name);
					$GLOBALS['SQ_REVERT_TO_SYSTEM_VERSION'] = FALSE;
					return FALSE;
				}
			}

			if (!$contexted_asset->saveAttributes()) {
				trigger_localised_error('SYS0164', E_USER_WARNING, $this->name);
				$GLOBALS['SQ_REVERT_TO_SYSTEM_VERSION'] = FALSE;
				return FALSE;
			}

			if ($contextid !== $current_contextid) {
				$GLOBALS['SQ_SYSTEM']->restoreContext();
				$GLOBALS['SQ_SYSTEM']->am->forgetAsset($contexted_asset);
				unset($contexted_asset);
			}

		}//end foreach

		if (!$this->revertNoticeLinksToSystemVersion()) return FALSE;

		// If we have metadata, load the old version so we can revert the values
		$metadata_filename = $this->data_path.'/.sq_system/metadata_field_values.php';
		if (is_file($metadata_filename)) {
			$old_metadata = unserialize(file_to_string($metadata_filename));

			// Acquire the "metadata" lock to set values
			$GLOBALS['SQ_SYSTEM']->am->acquireLock($this->id, 'metadata');

			$mm = $GLOBALS['SQ_SYSTEM']->getMetadataManager();

			// Set the defaults and any customised values
			if (!$mm->setMetadata($this->id, $old_metadata)) {
				trigger_localised_error('SYS0331', E_USER_WARNING, $this->name);
				$GLOBALS['SQ_REVERT_TO_SYSTEM_VERSION'] = FALSE;
				return FALSE;
			}

			// ...and we're done
			$GLOBALS['SQ_SYSTEM']->am->releaseLock($this->id, 'metadata');
		}

		if (!$this->clearSystemVersion()) {
			trigger_localised_error('SYS0161', E_USER_WARNING, $this->name);
			$GLOBALS['SQ_REVERT_TO_SYSTEM_VERSION'] = FALSE;
			return FALSE;
		}

		// ITS ALIVE... ITS ALIVE...
		foreach (get_object_vars($old_version) as $key => $value) {
			$this->$key = $value;
		}

		// also keep the data in asset to notify other parts of matrix
		// that we are reverting back to system version
		$this->_tmp['reverting_to_system_version'] = TRUE;

		// save this old version to the db
		$this->_updated();

		$GLOBALS['SQ_REVERT_TO_SYSTEM_VERSION'] = FALSE;
		return TRUE;

	}//end revertToSystemVersion()


	/**
	* Save the  notice links in system (live) version so it can be reverted later
	*
	* @return boolean
	* @access public
	*/
	public function saveNoticeLinksForSystemVersion()
	{
		$replace_assetids = Array();
		$notice_links = $GLOBALS['SQ_SYSTEM']->am->getLinks($this->id, SQ_LINK_NOTICE);

		// preserve the nest content related notice links so that we can recreate the links when reverting to system version
		$links_to_preserve = Array(
								'nested_asset',
								'paint_with_layout',
								'root',
								'redirect_asset',
								'thumbnail',
							 );
		foreach ($notice_links as $link) {
			if (in_array($link['value'], $links_to_preserve)) {
				$replace_assetids[] = $link;
			}
		}

		// save the notice links we currently have in .sq_notice_links file
		if (!string_to_file(serialize($replace_assetids), $this->data_path.'/.sq_system/.sq_notice_links')) {
			trigger_localised_error('CORE0048', E_USER_WARNING, $this->name);
			return FALSE;
		}
		
		return TRUE;
		
	}//end saveNoticeLinksForSystemVersion
	
	
	/**
	* Replaces the  notice links in editing version with the system (live) version
	*
	* @return boolean
	* @access public
	*/
	public function revertNoticeLinksToSystemVersion()
	{
		// we need to reconstruct the notice links (for nested contents) before the parent function removes the files in .sq_system!
		// so, read the notice links information from the system version file
		require_once SQ_FUDGE_PATH.'/general/file_system.inc';
		$notice_links_file = file_to_string($this->data_path.'/.sq_system/.sq_notice_links');

		// if we have any notice links preserved in the system version file, remove the current notice links
		// and create new ones based on the information from the system version file
		if (!empty($notice_links_file)) {
			$system_ver_notice_links = unserialize($notice_links_file);
			$notice_links = $GLOBALS['SQ_SYSTEM']->am->getLinks($this->id, SQ_LINK_NOTICE);

			$links_to_preserve = Array(
									'nested_asset',
									'paint_with_layout',
									'root',
									'redirect_asset',
									'thumbnail',
								 );
			foreach ($notice_links as $index => $info) {
				if (in_array($info['value'], $links_to_preserve)) {
					foreach($system_ver_notice_links as $prev_index => $prev_info) {
						if (array_diff($info, $prev_info) == Array()) {
							unset($system_ver_notice_links[$prev_index]); //not going to create
							unset($notice_links[$index]); //not going to delete
						}
					}
				} else {
					unset($notice_links[$index]); //not going to delete
				}
			}
			if (empty($system_ver_notice_links) && empty($notice_links)) return TRUE;

			$GLOBALS['SQ_SYSTEM']->changeDatabaseConnection('db2');
			$GLOBALS['SQ_SYSTEM']->doTransaction('BEGIN');

			$run_level_changed = FALSE;
			if ($GLOBALS['SQ_SYSTEM']->runLevelEnables(SQ_SECURITY_LOCKING)) {
				$GLOBALS['SQ_SYSTEM']->setRunLevel($GLOBALS['SQ_SYSTEM']->getRunLevel() - SQ_SECURITY_LOCKING);
				$run_level_changed = TRUE;
			}

			// delete existing notice links that are no longer being used.
			// the system version file contains only nest content related notice links with a link value, 'nested_asset' or 'paint_with_layout'
			// so, what we are deleting here is just nest content related notice links
			foreach ($notice_links as $link_data) {
				if (!$this->deleteLink($link_data['linkid'])) {
					if ($run_level_changed) {
						$GLOBALS['SQ_SYSTEM']->restoreRunLevel();
					}
					$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
					$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();
					return FALSE;
				}
			}

			// now re-create new nest content related notice links based on the information preserved in the system version file
			foreach ($system_ver_notice_links as $link_info) {
				// the first element of link info array needs to be an assetid (the minorid)
				if (!isset($link_info['minorid']) || !assert_valid_assetid($link_info['minorid'], '', FALSE, FALSE)) {
					continue;
				}
				$minor_asset = $GLOBALS['SQ_SYSTEM']->am->getAsset($link_info['minorid']);
				if (is_null($minor_asset)) continue;

				// notice link the minor asset to this asset with a link value kept in the 2nd element of link info array
				$link_val = '';
				if (isset($link_info['value'])) {
					$link_val = $link_info['value'];
				}
				$this->createLink($minor_asset, SQ_LINK_NOTICE, $link_val);

				// save attributes to fill in any missing information
				if (!$this->saveAttributes()) {
					if ($run_level_changed) {
						$GLOBALS['SQ_SYSTEM']->restoreRunLevel();
					}
					$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
					$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();
					return FALSE;
				}

			}//end foreach

			if ($run_level_changed) {
				$GLOBALS['SQ_SYSTEM']->restoreRunLevel();
			}

			$GLOBALS['SQ_SYSTEM']->doTransaction('COMMIT');
			$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();

		}//end if

		return TRUE;

	}//end revertNoticeLinksToSystemVersion()



	/**
	* Set the last updated info for this asset
	*
	* Call this function when the asset has been changed in some way so we can indicate in the
	* asset table when the asset was last changed and who by. This function will also increment the micro
	* version number for the asset.
	*
	* @param boolean	$update_parents	should we go through an update all our dependant parents as well ?
	*									NOTE: this is passed to incrementVersion to let it know that
	*									the parents should have their versions updated as well.
	*
	* @return boolean
	* @access private
	*/
	public function _updated($update_parents=TRUE)
	{
		require_once SQ_FUDGE_PATH.'/general/general.inc';

		if (isset($this->_tmp['__creating__']) && $this->_tmp['__creating__']) {
			return TRUE;
		}

		$updated = time();
		$updated_userid  = $GLOBALS['SQ_SYSTEM']->currentUserId();
		$default_context = 0;
		$name            = check_char_encoding(substr($this->_getName(FALSE, $default_context), 0, 255));
		$short_name      = check_char_encoding(substr($this->_getName(TRUE, $default_context), 0, 255));



		// if we have an ID hit the DB
		if ($this->id) {

			$GLOBALS['SQ_SYSTEM']->changeDatabaseConnection('db2');
			$GLOBALS['SQ_SYSTEM']->doTransaction('BEGIN');
			$bind_vars = Array();

			$sql = 'UPDATE
						sq_ast
					SET ';

			if (($name === '0') || (!empty($name))) {
				$bind_vars['name'] = $name;
				$sql .= 'name = :name,';
			}
			if (($short_name === '0') || (!empty($short_name))) {
				$bind_vars['short_name'] = $short_name;
				$sql .= 'short_name = :short_name,';
			}

			$sql .= '
						languages		= :languages,
						charset			= :charset,
						force_secure	= :force_secure,
						updated			= :updated,
						updated_userid	= :updated_userid
					WHERE
						assetid	= :assetid';

			$bind_vars['languages']      = $this->languages;
			$bind_vars['charset']        = $this->charset;
			$bind_vars['force_secure']   = $this->force_secure;
			$bind_vars['updated']        = ts_iso8601($updated);
			$bind_vars['updated_userid'] = $updated_userid;
			$bind_vars['assetid']        = $this->id;

			try {
				$query = MatrixDAL::preparePdoQuery($sql);
				foreach ($bind_vars as $bind_var => $bind_value) {
					MatrixDAL::bindValueToPdo($query, $bind_var, $bind_value);
				}
				MatrixDAL::execPdoQuery($query);
			} catch (Exception $e) {
				throw new Exception('Unable to update asset "'.$this->_getName().'" (#'.$this->id.') due to database error: '.$e->getMessage());
			}

			$em = $GLOBALS['SQ_SYSTEM']->getEventManager();

			$fields = Array(
						'name',
						'short_name',
						'updated_userid',
					  );

			// We know that 'updated' field ALWAYS changes, no need to check the value changes
			$changed_data = Array('updated');
			foreach ($fields as $field) {
				if ($this->$field != $$field) {
					$changed_data[] = $field;
				}
			}

			// set these components before broadcasting the event
			$this->name           = $name;
			$this->short_name     = $short_name;
			$this->updated        = $updated;
			$this->updated_userid = $updated_userid;

			// increment the micro version number
			if (!$this->incrementVersion('micro', $update_parents)) {
				trigger_localised_error('SYS0183', E_USER_WARNING, $this->name);
				$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
				$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();
				return FALSE;
			}

			$GLOBALS['SQ_SYSTEM']->doTransaction('COMMIT');
			$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();

			$em->broadcastEvent($this, 'AssetUpdate', $changed_data);

			$GLOBALS['SQ_SYSTEM']->broadcastTriggerEvent('trigger_event_asset_updated', $this);

		}//end if $this->id

		return TRUE;

	}//end _updated()


	/**
	* Return a name to use to describe this instance of the asset
	*
	* This function should be overridden by children as it is used to set the <i>name</i> and <i>short_name</i>
	* values for this asset when updating.
	*
	* @param boolean	$short_name	whether or not we are after the shortname or the full name
	* @param int		$contextid	what context to get the name in (override in kiddies)
	*
	* @return string
	* @access private
	* @see _updated
	*/
	protected function _getName($short_name=FALSE, $contextid=NULL)
	{
		return $GLOBALS['SQ_SYSTEM']->am->getTypeInfo($this->type(), 'name').' #'.$this->id;

	}//end _getName()


	/**
	* Get the type of the asset
	*
	* Note that as of PHP 5.x, return values from get_class() are in mixed case
	* (eg. "Asset" instead of "asset"). This function ensures that asset types
	* it returns are lower case, as in other parts of Matrix.
	*
	* @return string
	* @access public
	*/
	public function type()
	{
		return strtolower(get_class($this));

	}//end type()


	/**
	* Get an array of all assets types that the asset extends from
	*
	* @param boolean	$include_asset	when TRUE adds the "Asset" type to the parent list,
	*									even though it's an unistantiable object
	*
	* @return array
	* @access public
	* @see Asset_Manager::getTypeAncestors()
	*/
	public function getTypeAncestors($include_asset=TRUE)
	{
		return $GLOBALS['SQ_SYSTEM']->am->getTypeAncestors($this->type(), $include_asset);

	}//end getTypeAncestors()


	/**
	* Increments the version number of the asset based on the part of the version passed in
	*
	* @param string		$number			the part of the version to update (major|minor|micro)
	* @param boolean	$update_parents	should we go through an update all our dependant parents as well ?
	*									NOTE: this is passed to incrementVersion to let it know that the parents
	*									should have their versions updated as well.
	*
	* @return boolean
	* @access public
	*/
	public function incrementVersion($number='micro', $update_parents=TRUE)
	{
		if (!$this->id) return FALSE;

		switch ($number) {
			case 'major' :
				$part_num = 0;
			break;
			case 'minor' :
				$part_num = 1;
			break;
			case 'micro' :
				$part_num = 2;
			break;
			default :
				trigger_localised_error('SYS0157', E_USER_WARNING, $this->name, $this->id, $number);
				return FALSE;
			break;
		}

		// work out the new version number based on the part of the
		// version that we have been asked to update (major|minor|micro)
		$version_parts = explode('.', $this->version);
		for (reset($version_parts); NULL !== ($k = key($version_parts)); next($version_parts)) {
			$value =& $version_parts[$k];
			$value = (int)$value;
			if ($k == $part_num) {
				$value++;
			} else if ($k > $part_num) {
				$value = 0;
			}
		}


		$new_version = implode('.', $version_parts);

		$GLOBALS['SQ_SYSTEM']->changeDatabaseConnection('db2');
		$GLOBALS['SQ_SYSTEM']->doTransaction('BEGIN');
		$db = MatrixDAL::getDb();

		$sql = 'UPDATE
					sq_ast
				SET
					version	= :version
				WHERE
					assetid	= :assetid';

		try {
			$query = MatrixDAL::preparePdoQuery($sql);
			MatrixDAL::bindValueToPdo($query, 'version', $new_version, PDO::PARAM_STR);
			MatrixDAL::bindValueToPdo($query, 'assetid', $this->id,    PDO::PARAM_STR);
			MatrixDAL::execPdoQuery($query);
		} catch (Exception $e) {
			throw new Exception('Unable to increment version of "'.$this->_getName().'" (#'.$this->id.') due to database error: '.$e->getMessage());
		}

		if ($update_parents) {
			// increment the micro version of all dependant parents because a 'part' of them
			// has been updated - even though they havnt been updated directly
			$dependant_parents = $GLOBALS['SQ_SYSTEM']->am->getDependantParents($this->id);
			foreach ($dependant_parents as $parentid) {
				$dep_parent = $GLOBALS['SQ_SYSTEM']->am->getAsset($parentid);
				if (!is_null($dep_parent)) {
					if (!$dep_parent->_updated(FALSE)) {
						$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
						$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();
						$GLOBALS['SQ_SYSTEM']->am->forgetAsset($dep_parent);
						trigger_localised_error('SYS0158', E_USER_WARNING, $this->name, $this->id, $dep_parent->name, $dep_parent->id);
						return FALSE;
					}
				}
				$GLOBALS['SQ_SYSTEM']->am->forgetAsset($dep_parent);
			}
		}

		$em = $GLOBALS['SQ_SYSTEM']->getEventManager();
		$em->broadcastEvent($this, 'AssetVersionUpdate', Array(
															'old_version'	=> $this->version,
															'new_version'	=> $new_version,
														 )
		);

		$GLOBALS['SQ_SYSTEM']->doTransaction('COMMIT');
		$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();

		$this->version = $new_version;
		return TRUE;

	}//end incrementVersion()


	/**
	* Gets the default languages of the asset as an array
	*
	* @return array
	* @access public
	*/
	public function getLanguages()
	{
		$langs = trim($this->languages, ', ');
		if ($langs == '') return Array();
		return explode(',', $langs);

	}//end getLanguages()


	/**
	* Set the default languages of the asset
	*
	* @param array	$languages	an array of languages to set for the asset
	*
	* @return boolean
	* @access public
	*/
	public function setLanguages(Array $languages)
	{
		$set_languages = implode(',', $languages);
		if ($set_languages == $this->languages) return FALSE;

		$old_languages = $this->languages;
		$this->languages = $set_languages;
		if (!$this->_updated()) {
			trigger_localised_error('SYS0172', E_USER_WARNING, $this->name);
			$this->languages = $old_languages;
			return FALSE;
		}

		$em = $GLOBALS['SQ_SYSTEM']->getEventManager();
		$em->broadcastEvent($this, 'AssetLanguageUpdate', Array(
															'old_languages'	=> $old_languages,
															'new_languages'	=> $this->languages,
														  )
		);
		return TRUE;

	}//end setLanguages()


	/**
	* Set the default character set of the asset
	*
	* @param string	$charset	the character set to set for the asset
	*
	* @return boolean
	* @access public
	*/
	public function setCharset($charset)
	{
		if ($charset == $this->charset) return FALSE;
		$old_charset = $this->charset;
		$this->charset = $charset;
		if (!$this->_updated()) {
			trigger_localised_error('SYS0170', E_USER_WARNING, $this->name);
			$this->charset = $old_charset;
			return FALSE;
		}

		$em = $GLOBALS['SQ_SYSTEM']->getEventManager();
		$em->broadcastEvent($this, 'AssetCharsetUpdate', Array(
															'old_charset'	=> $old_charset,
															'new_charset'	=> $this->charset,
														 )
		);
		return TRUE;

	}//end setCharset()


	/**
	* Set the status of the Force Secure Flag
	*
	* Force Secure now takes on three possible options:
	*  '0' => don't force any protocol.
	*  '1' => force secure (HTTPS only)
	*  '-' => force insecure (HTTP only)
	*
	* It will still also accept boolean TRUE|FALSE, which corresponds with
	* '1' and '0' respectively.
	*
	* Returns TRUE if the setting is successfully changed, FALSE otherwise.
	* It is NOT considered a successful change if the force secure setting
	* wouldn't actually change (in which case this function does nothing).
	*
	* @param mixed	$force_secure	whether to force the use of SSL on URLs
	*
	* @return boolean
	* @access public
	*/
	public function setForceSecure($force_secure)
	{
		if (!$this->adminAccess('settings')) {
			trigger_localised_error('SYS0252', E_USER_WARNING);
			return FALSE;
		}

		if ($force_secure !== '-') {
			$force_secure = ($force_secure) ? '1' : '0';
		}
		if ($force_secure == $this->force_secure) return FALSE;
		$old_force_secure = $this->force_secure;
		$this->force_secure = $force_secure;
		if (!$this->_updated()) {
			trigger_localised_error('SYS0169', E_USER_WARNING, $this->name, $this->id);
			$this->force_secure = $old_force_secure;
			return FALSE;
		}

		$em = $GLOBALS['SQ_SYSTEM']->getEventManager();
		$em->broadcastEvent($this, 'AssetForceSecureUpdate', Array(
																'old_force_secure'	=> $old_force_secure,
																'old_force_secure'	=> $this->force_secure,
															 )
		);

		return TRUE;

	}//end setForceSecure()


	/**
	* Remap existing assetids used to new ones for ourselves and all our dependants
	*
	* @param array	$map	a map of the form Array([old asset id] => [new asset id]);
	*
	* @return boolean
	* @access public
	*/
	public function remapAssetids(Array $map)
	{
		$GLOBALS['SQ_SYSTEM']->changeDatabaseConnection('db2');
		$GLOBALS['SQ_SYSTEM']->doTransaction('BEGIN');
		// try and acquire locks on our dependants
		$dependant_links = $GLOBALS['SQ_SYSTEM']->am->getLinks($this->id, SQ_SC_LINK_SIGNIFICANT, '', TRUE, 'major', NULL, 1);
		if (!empty($dependant_links)) {
			$am = $GLOBALS['SQ_SYSTEM']->am;
			foreach ($dependant_links as $link) {
				$asset = $am->getAsset($link['minorid'], $link['minor_type_code']);
				if (!$asset->remapAssetids($map)) {
					$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
					$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();
					return FALSE;
				}
			}
		}

		$GLOBALS['SQ_SYSTEM']->doTransaction('COMMIT');
		$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();
		return TRUE;

	}//end remapAssetids()


	/**
	* Morph the asset to either one of its parent types or one of its children types
	*
	* Only succeeds if $type_code assets are allowed to link to all children that this asset has.
	* Since 3.18, we cannot do $this = $morphed, so we return the morphed obj if successful
	* We also call forgetAsset and getAsset again to refresh the cache.
	*
	* @param string	$new_type_code	the new type_code to attempt to cast this asset to
	*
	* @return object|null
	* @access public
	*/
	public function morph($new_type_code)
	{
		$new_type_code = strtolower($new_type_code);
		$old_type_code = $this->type();

		if ($this->type() == $new_type_code) return $this;

		if (!$GLOBALS['SQ_SYSTEM']->am->installed($new_type_code)) {
			trigger_localised_error('SYS0085', E_USER_WARNING, $new_type_code);
			return NULL;
		}

		$am = $GLOBALS['SQ_SYSTEM']->am;

		// open the transaction
		$GLOBALS['SQ_SYSTEM']->changeDatabaseConnection('db2');
		$GLOBALS['SQ_SYSTEM']->doTransaction('BEGIN');
		$db = MatrixDAL::getDb();

		// is this new type one of our decendents or ancestors?
		$sql = 'SELECT COUNT(*)
				FROM sq_ast_typ_inhd
				WHERE (inhd_type_code = :type1 AND type_code = :type2)
				   OR (inhd_type_code = :type2 AND type_code = :type1)';
		try {
			$query = MatrixDAL::preparePdoQuery($sql);
			MatrixDAL::bindValueToPdo($query, 'type1', $this->type());
			MatrixDAL::bindValueToPdo($query, 'type2', $new_type_code);
			$count = MatrixDAL::executePdoOne($query);
		} catch (Exception $e) {
			throw new Exception('Unable to get number of inherited type codes for this inherited type code: '.$this->type().' and this type code: '.$new_type_code.' due to database error: '.$e->getMessage());
		}

		if ($count == 0) {
			trigger_localised_error('SYS0234', E_USER_WARNING, $this->name, $new_type_code, $this->type());
			$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
			$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();
			return NULL;
		}

		$am->includeAsset($new_type_code);
		$tmp = new $new_type_code();

		//// FIRST we need to check that all our links are allowed for the new type ////

		// if there any children links
		if ($num_links = $GLOBALS['SQ_SYSTEM']->am->countLinks($this->id, 'major')) {

			$links = $GLOBALS['SQ_SYSTEM']->am->getLinks($this->id, SQ_SC_LINK_ALL);
			foreach ($links as $link) {
				// Shadow asset links (such as tags) are not encumbered by
				// type checks in createAssetLink(), so no restrictions here.
				if (strpos($link['minorid'], ':') !== FALSE) {
					continue;
				}
				if ($GLOBALS['SQ_SYSTEM']->am->canLinkToType($tmp, $link['minor_type_code'], $link['link_type'], 0, $link['is_exclusive']) !== TRUE) {
					require_once SQ_INCLUDE_PATH.'/general_occasional.inc';
					trigger_localised_error('SYS0236', E_USER_WARNING, $this->name, $new_type_code, $link['minor_type_code'], link_type_name($link['link_type']), $new_type_code);
					$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
					$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();
					return NULL;
				}
			}// end foreach

		}// end if child links

		// if there any parent links
		$num_links = $GLOBALS['SQ_SYSTEM']->am->countLinks($this->id, 'minor');
		if ($num_links) {

			$links = $GLOBALS['SQ_SYSTEM']->am->getLinks($this->id, SQ_SC_LINK_ALL, '', TRUE, 'minor');
			foreach ($links as $link) {
				$parent = $am->getAsset($link['majorid'], $link['major_type_code']);
				if (is_null($parent)) continue;
				if (($err_msg = $GLOBALS['SQ_SYSTEM']->am->canLinkToType($parent, $new_type_code, $link['link_type'], $link['linkid'], $link['is_exclusive'])) !== TRUE) {
					require_once SQ_INCLUDE_PATH.'/general_occasional.inc';
					trigger_localised_error('SYS0235', E_USER_WARNING, $this->name, $new_type_code, $parent->name, $parent->id, $err_msg);
					$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
					$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();
					return NULL;
				}
			}// end foreach

		}// end if parent links


		$current_var_list = array_keys($this->vars);
		$new_var_list     = array_keys($tmp->vars);

		// Get all the common vars and update the attribute ids to the new values
		// so the asset gets to keep the common values already set
		$common_var_list = array_intersect($current_var_list, $new_var_list);

		if ($common_var_list) {
			foreach ($common_var_list as $var_name) {

				$current_id = $this->vars[$var_name]['attrid'];
				$new_id     = $tmp->vars[$var_name]['attrid'];


				try {
					$bind_vars	= Array (
									'newattrid'			=> $new_id,
									'assetid'			=> $this->id,
									'currentattrid'		=> $current_id,
								  );
					$result = MatrixDAL::executeQuery('core', 'updateAttrId', $bind_vars);
				} catch (Exception $e) {
					throw new Exception('Unable to update attribute id for asset: '.$this->id.' from current attribute id: '.$current_id.' to new attribute id: '.$new_id.' due to database error: '.$e->getMessage());
				}
			}
		}//end if common var list

		// Get all the vars that aren't available in the new type and delete them
		$deletes_var_list = array_diff($current_var_list, $new_var_list);

		if ($deletes_var_list) {
			$deletes_attributeids = '';
			foreach ($deletes_var_list as $var_name) {
				$deletes_attributeids .= (($deletes_attributeids) ? ',' : '').MatrixDAL::quote($this->vars[$var_name]['attrid']);
			}

			$sql = 'DELETE FROM
						sq_ast_attr_val
					WHERE
							assetid	= :assetid
						AND attrid IN
						(
							'.$deletes_attributeids.'
						)';

			try {
				$query = MatrixDAL::preparePdoQuery($sql);
				MatrixDAL::bindValueToPdo($query, 'assetid', $this->id);
				$result = MatrixDAL::execPdoQuery($query);
			} catch (Exception $e) {
				throw new Exception('Unable to delete attribute values for asset: '.$this->id.' due to database error: '.$e->getMessage());
			}

		}// end if delete var list

		try {
			$bind_vars	= Array (
							'type_code'			=> $new_type_code,
							'assetid'			=> $this->id,
						  );
			$result = MatrixDAL::executeQuery('core', 'updateAstTypeCode', $bind_vars);
		} catch (Exception $e) {
			throw new Exception('Unable to update type code for asset: '.$this->id.' due to database error: '.$e->getMessage());
		}

		// now load this new asset into the temporary
		$tmp->load($this->id);
		$all_ok = FALSE;
		if ($tmp->id) {
			// OK if we got this far let's move the directory (if it exists)
			if (is_dir($this->data_path)) {
				require_once SQ_FUDGE_PATH.'/general/file_system.inc';
				// make sure the parent directory exists, then move the our directory to it's new home
				if (create_directory(dirname($tmp->data_path)) && rename($this->data_path, $tmp->data_path)) {
					$all_ok = TRUE;
				}
			} else {
				// if there isn't a directory then everything is fine
				$all_ok = TRUE;
			}

		}// end if

		// before we override ourselves, do any cleaning up that we might need
		if ($all_ok && !$this->_morphCleanup($new_type_code)) {
			$all_ok = FALSE;
		}

		// all is OK so override ourselves with the temporary
		if ($all_ok) {
			$GLOBALS['SQ_SYSTEM']->doTransaction('COMMIT');
			$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();

			$em = $GLOBALS['SQ_SYSTEM']->getEventManager();
			$em->broadcastEvent($this, 'AssetTypeUpdate', Array(
															'old_type'	=> $old_type_code,
															'new_type'	=> $new_type_code,
														  )
			);

			// We beed to forget the asset because we are changed the
			// type_code, and it does not get updated in the asset cache
			// because of the way it does references. Then we get a fresh copy
			// from asset manager which will repopoulate the cache.
			$GLOBALS['SQ_SYSTEM']->am->forgetAsset($tmp, TRUE);
			return $GLOBALS['SQ_SYSTEM']->am->getAsset($tmp->id, $tmp->type());

		} else {
			$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
			$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();
			return NULL;

		}// end if

	}//end morph()


	/**
	* Clean up any sloppy seconds that are left over before we cast this asset for good
	*
	* @param string	$new_type_code	the new type_code that this asset is being cast to
	*
	* @return boolean
	* @access private
	*/
	public function _morphCleanup($new_type_code)
	{
		return TRUE;

	}//end _morphCleanup()


	/**
	* Handle a request for available asset-type keywords
	*
	* We set $vars['my_keyword'] = 'my keyword desc' to tell the broadcaster
	* what keywords are available for this asset type.
	*
	* @param Asset	$broadcaster	the asset that triggered the event
	* @param array	$vars			the vars that get submitted by the broadcaster
	*								we add keywords to the $vars['keywords'] array
	*
	* @return void
	* @access private
	*/
	public function onRequestKeywords(Asset $broadcaster, Array $vars=Array())
	{
		if (!isset($vars['keywords'])) return;
		$vars['keywords'] = array_merge($vars['keywords'], $this->getAvailableKeywords());

	}//end onRequestKeywords()


//--        CLONING        --//


	/**
	* Returns TRUE if this type of asset can be cloned
	*
	* Override this function if you do not want a particular type of asset to be cloned
	*
	* @return boolean
	* @access public
	*/
	public function canClone()
	{
		return TRUE;

	}//end canClone()


	/**
	* Takes a clone shell and clones the sources required components to the clone shell
	* (if the current user has the permissions on the clone shell to do so)
	*
	* If $fail_on_error is TRUE, then cloneComponents will bail on the first error (default
	* behaviour) - if not, however, cloneComponents will report the error and attempt to
	* continue (returning FALSE in exceptional circumstances only)
	*
	* @param Asset		$clone		the asset object that we are turning into a clone of this one
	* @param array		$components	the wanted components to clone eg.
	* <pre>
	* 						Array(
	* 							'attributes',
	* 							'metadata_schemas',
	* 							'metadata',
	* 							'workflow',
	* 							'permissions',
	* 							'data',
	* 							'content_tags',
	* 							'roles',
	* 						);
	* 						or alternately
	* 						Array('all');
	* </pre>
	* @param boolean	$override	if TRUE, $this' permissions, metadata schemas, workflow schemas will override any existing ones already set on $clone
	*
	* @return boolean
	* @access public
	*/
	public function cloneComponents(Asset $clone, Array $components, $override=FALSE)
	{
		if (!is_array($components) || empty($components)) {
			return FALSE;
		}
		if (is_null($clone)) return FALSE;

		$GLOBALS['SQ_SYSTEM']->changeDatabaseConnection('db2');
		$GLOBALS['SQ_SYSTEM']->doTransaction('BEGIN');

		$GLOBALS['SQ_CLONE_COMPONENTS'] = TRUE;

		$all_contexts = $GLOBALS['SQ_SYSTEM']->getAllContexts();

		  //////////////////
		 /// ATTRIBUTES ///
		//////////////////
		if (in_array('attributes', $components) || in_array('all', $components)) {
			if (!$GLOBALS['SQ_SYSTEM']->runLevelEnables(SQ_SECURITY_PERMISSIONS) || $clone->writeAccess('attributes')) {
				// Save the current variables so we can restore them afterwards
				$current_vars = $this->vars;

				foreach ($all_contexts as $contextid => $context_data) {
					// Change the context, then reload the attributes
					// (we don't need to change the clone, because saveAttributes()
					// already works with the current context)
					$GLOBALS['SQ_SYSTEM']->changeContext($contextid);
					$this->_loadVars();

					// Now we set all the attributes
					foreach ($this->vars as $name => $data) {
						if (($contextid === 0) || ((boolean)$data['use_default'] === FALSE)) {
							if (!$clone->setAttrValue($name, $data['value'])) {
								$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
								$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();
								$GLOBALS['SQ_CLONE_COMPONENTS'] = FALSE;
								return FALSE;
							}
						}
					}
					if (!$clone->saveAttributes()) {
						$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
						$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();
						$GLOBALS['SQ_CLONE_COMPONENTS'] = FALSE;
						return FALSE;
					}

					$GLOBALS['SQ_SYSTEM']->restoreContext();
				}//end foreach

				// Move the original context's vars back
				$this->vars = $current_vars;
			} else {
				trigger_localised_error('SYS0328', E_USER_WARNING, $clone->name);
			}//end if (permissions)
		}//end if attributes


		  ///////////////////
		 /// PERMISSIONS ///
		///////////////////

		if (in_array('permissions', $components) || in_array('all', $components)) {
			if (!$GLOBALS['SQ_SYSTEM']->runLevelEnables(SQ_SECURITY_PERMISSIONS) || $clone->adminAccess('permissions')) {
				$this->_tmp[__CLASS__.'_in_create_cascading'] = TRUE;
				foreach (Array(SQ_PERMISSION_READ, SQ_PERMISSION_WRITE, SQ_PERMISSION_ADMIN) as $perm) {
					$set_perms = $GLOBALS['SQ_SYSTEM']->am->getAssetPermissionByCascade($this->id, $perm, NULL, TRUE);
					foreach ($set_perms as $perm_info) {
						if (!$GLOBALS['SQ_SYSTEM']->am->setPermission($clone->id, $perm_info['userid'], $perm, $perm_info['granted'], TRUE, $override)) {
							$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
							$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();
							unset($this->_tmp[__CLASS__.'_in_create_cascading']);
							$GLOBALS['SQ_CLONE_COMPONENTS'] = FALSE;
							return FALSE;
						}
					}
				}//end foreach
				unset($this->_tmp[__CLASS__.'_in_create_cascading']);
			} else {
				trigger_localised_error('SYS0322', E_USER_WARNING, $clone->name);
			}
		}//end if permissions


		  /////////////
		 /// ROLES ///
		/////////////

		if (in_array('roles', $components) || in_array('all', $components)) {
			if (!$GLOBALS['SQ_SYSTEM']->runLevelEnables(SQ_SECURITY_PERMISSIONS) || $clone->adminAccess('roles')) {
				$roles = $GLOBALS['SQ_SYSTEM']->am->getRole($this->id);
				foreach ($roles as $roleid => $ids) {
					foreach ($ids as $row => $userid) {
						if (!$GLOBALS['SQ_SYSTEM']->am->setRole($clone->id, $roleid, $userid)) {
							$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
							$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();
							$GLOBALS['SQ_CLONE_COMPONENTS'] = FALSE;
							return FALSE;
						}
					}
				}
			} else {
				trigger_localised_error('SYS0325', E_USER_WARNING, $clone->name);
			}
		}//end if roles


		  ////////////////
		 /// METADATA ///
		////////////////

		// first up, clone the schemas
		if (in_array('metadata_schemas', $components) || in_array('all', $components)) {
			if (!$GLOBALS['SQ_SYSTEM']->runLevelEnables(SQ_SECURITY_PERMISSIONS) || $clone->adminAccess('metadata')) {
				$mm = $GLOBALS['SQ_SYSTEM']->getMetadataManager();
				if ($mm->allowsMetadata($clone->id)) {
					// apply the schemas from the cloner to the clonee
					// But only if they're set to cascade to new assets
					$schemas = $mm->getSchemas($this->id, NULL, TRUE);

					$schema_count = 0;
					$metadata = Array();
					foreach ($schemas as $schemaid => $granted) {
						if (!$GLOBALS['SQ_SYSTEM']->am->assetExists($schemaid)) {
							continue;
						}

						// FastTrack the "metadata content file generation" task for all schemas except the last one
						if (++$schema_count == count($schemas)) {
							$clone->unFastTrack('metadata_manager_generate_content_file');
						} else {
							$clone->fastTrack('metadata_manager_generate_content_file');
						}

						if (!$mm->setSchema($clone->id, $schemaid, $granted, TRUE, $override)) {
							$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
							$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();
							$GLOBALS['SQ_CLONE_COMPONENTS'] = FALSE;
							return FALSE;
						}
					}//end foreach
				}
			} else {
				trigger_localised_error('SYS0323', E_USER_WARNING, $clone->name);
			}//end if access
		}//end if metadata schemas

		// apply the actual metadata from the cloner to the clonee
		if (in_array('metadata', $components) || in_array('all', $components)) {
			$mm = $GLOBALS['SQ_SYSTEM']->getMetadataManager();
			if ($mm->allowsMetadata($clone->id)) {
				if (!$GLOBALS['SQ_SYSTEM']->runLevelEnables(SQ_SECURITY_PERMISSIONS) || $clone->writeAccess('metadata')) {
					$ok = TRUE;
					$context_count = 0;

					foreach ($all_contexts as $contextid => $context_data) {
						// Change the context, then reload the attributes
						// (we don't need to change the clone, because saveAttributes()
						// already works with the current context)
						$GLOBALS['SQ_SYSTEM']->changeContext($contextid);

						$metadata = $mm->getMetadata($this->id);
						if (!empty($metadata)) {
							// apply the metadata from $this to the clone - setMetadata() will ignore any values
							// belonging to fields that don't exist on the clone, so we're fine even if the
							// schemas weren't cloned
							if (!$mm->setMetadata($clone->id, $metadata) || !$mm->regenerateMetadata($clone->id)) {
								$ok = FALSE;
							}
						}

						$GLOBALS['SQ_SYSTEM']->restoreContext();

						if ($ok === FALSE) break;

					}//end foreach


					if ($ok === FALSE) {
						$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
						$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();
						$GLOBALS['SQ_CLONE_COMPONENTS'] = FALSE;
						return FALSE;
					}

				} else {
					trigger_localised_error('SYS0327', E_USER_WARNING, $clone->name);
				}//end if ($clone->writeAccess('metadata'))
			}//end if ($mm->allowsMetadata($clone->id))
		}//end if metadata


		  ////////////////
		 /// WORKFLOW ///
		////////////////

		if (in_array('workflow', $components) || in_array('all', $components)) {
			if (!$GLOBALS['SQ_SYSTEM']->runLevelEnables(SQ_SECURITY_PERMISSIONS) || $clone->adminAccess('workflow')) {
				$wfm = $GLOBALS['SQ_SYSTEM']->getWorkflowManager();
				// Get all schemas that are set to cascade down to new assets
				$schemas = $wfm->getSchemas($this->id, NULL, FALSE, TRUE);
				foreach ($schemas as $schemaid => $granted) {
					if (!$wfm->setSchema($clone->id, $schemaid, $granted, TRUE, $override)) {
						$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
						$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();
						$GLOBALS['SQ_CLONE_COMPONENTS'] = FALSE;
						return FALSE;
					}
				}//end foreach
			} else {
				trigger_localised_error('SYS0324', E_USER_WARNING, $clone->name);
			}
		}//end if workflow


		  ////////////////
		 /// DATA DIR ///
		////////////////

		if (in_array('data', $components) || in_array('all', $components)) {
			// make sure we have write access before we start mucking around with files
			if ($GLOBALS['SQ_SYSTEM']->runLevelEnables(SQ_SECURITY_PERMISSIONS) && !$clone->writeAccess('attributes')) {
				// if permission check is on, and user does not have permission
				trigger_localised_error('SYS0321', E_USER_WARNING, $clone->name);
			} else {
				// copy the directory (if it exists)
				if (is_dir($this->data_path)) {
					require_once SQ_FUDGE_PATH.'/general/file_system.inc';
					// something slightly more catastrophic happened
					if (!copy_directory($this->data_path, $clone->data_path)) {
						$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
						$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();
						$GLOBALS['SQ_CLONE_COMPONENTS'] = FALSE;
						return FALSE;
					}
				}
			}
		}//end if data


		  ////////////////////
		 /// CONTENT TAGS ///
		////////////////////

		if (in_array('content_tags', $components) || in_array('all', $components)) {
			if (!$GLOBALS['SQ_SYSTEM']->runLevelEnables(SQ_SECURITY_PERMISSIONS) || ($clone->writeAccess('attributes') && $clone->writeAccess('links'))) {
				$tag_manager = $GLOBALS['SQ_SYSTEM']->getTagManager();

				// don't clone content tags if there's no tag manager yet
				if (!is_null($tag_manager)) {
					$current_tag_links = $tag_manager->getTagLinks($this->id);
					foreach ($current_tag_links as $link) {
						$tag_manager->setTag($clone->id, $link['minorid'], $link['value']);
					}
				}
			} else {
				trigger_localised_error('SYS0326', E_USER_WARNING, $clone->name);
			}
		}//end if content tags

		$GLOBALS['SQ_SYSTEM']->doTransaction('COMMIT');
		$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();
		$GLOBALS['SQ_CLONE_COMPONENTS'] = FALSE;
		return TRUE;

	}//end cloneComponents()


	/**
	* Perform any additional processing required during the cloning of this asset
	*
	* This function is being called by asset_manager after the dependants of asset
	* have been cloned.
	*
	* @param Asset	$clone		the clone asset shell
	* @param array	$components	the wanted components to clone eg.
	*							Array(
	*							'attributes',
	*							'metadata_schemas',
	*							'metadata',
	*							'workflow',
	*							'permissions',
	*							'data',
	*							);
	*							or alternately
	*							Array('all');
	*
	* @return boolean
	* @access public
	*/
	public function cloneComponentsAdditional(Asset $clone, Array $components)
	{
		return TRUE;

	}//end cloneComponentsAdditional()


	/**
	* Link the assets that are linked under this asset under the supplied clone shell too
	*
	* Under the default cloning policy, this is done for notice and type 3 links only.  Notice
	* that the linked assets are not cloned - they end up linked to both this asset and the clone.
	*
	* @param Asset:OO	$clone	The asset object that we are turning into a clone of this one
	*
	* @return boolean
	* @access public
	*/
	public function cloneLinks(Asset $clone)
	{
		// we do not want to clone any assets that are type_3 or type_notice linked to the asset we are
		// cloning. Instead, we will just link them up to the cloner's original link counter parts
		$orig_links = $GLOBALS['SQ_SYSTEM']->am->getLinks($this->id, SQ_LINK_TYPE_3 | SQ_LINK_NOTICE, '', TRUE, 'major', NULL, FALSE, NULL);
		foreach ($orig_links as $orig_link) {
			$asset = $GLOBALS['SQ_SYSTEM']->am->getAsset($orig_link['minorid'], $orig_link['minor_type_code']);
			if ($asset->canCloneLink()) {
				$linkid = $clone->createLink($asset, $orig_link['link_type'], $orig_link['value'], $orig_link['sort_order'], $orig_link['is_dependant'], $orig_link['is_exclusive']);
				if (!$linkid) return FALSE;
			}

		}//end foreach
		return TRUE;

	}//end cloneLinks()


//--        STATUS        --//


	/**
	* Returns a status object for the current asset to be used for processing
	*
	* @return Asset_Status
	* @access public
	*/
	public function getStatus()
	{
		if (!isset($this->_tmp['status_object'])) {
			// work out the name of our status file
			require_once SQ_INCLUDE_PATH.'/general_occasional.inc';
			$status_code = get_bit_names('SQ_STATUS_', $this->status);
			$status_code = 'asset_status_'.strtolower($status_code);
			require_once SQ_INCLUDE_PATH.'/asset_status/'.$status_code.'.inc';
			$this->_tmp['status_object'] = new $status_code($this);
		}
		return $this->_tmp['status_object'];

	}//end getStatus()


	/**
	* Returns an array of statii that the current user can set for this asset
	* Value returned is a key => value array for a drop down
	*
	* @return array
	* @access public
	*/
	public function getAvailableStatii()
	{
		$status = $this->getStatus();
		$statii = $status->getAvailableStatii();
		ksort($statii, SORT_NUMERIC);
		return array_reverse($statii, TRUE);

	}//end getAvailableStatii()


	/**
	* Returns a description representing the current status
	*
	* @return string
	* @access public
	*/
	public function getStatusDescription()
	{
		$status = $this->getStatus();
		return $status->getDescription();

	}//end getStatusDescription()


	/**
	* Processes the changes required when a new status is set
	*
	* @param int		$new_status		the new status to set for this asset
	* @param boolean	$update_parents	Specify if we want to update the parent as well
	* @param boolean	$run_updated	specify it we want to run _upated
	*
	* @return boolean
	* @access public
	*/
	public function processStatusChange($new_status, $update_parents=TRUE, $run_updated=TRUE)
	{
		$old_status = $this->status;

		// if we are dependant minor assets, we can only change our status
		// to a status higher than or equal to all the statii of our parents -
		// BUT only if we are going down status - we are allowed to play
		// 'catch-up' to a parent status but not retreat from it
		if ($this->status > $new_status) {
			// Allow downgrades anyway if in a workflow status - we still want
			// people to reject a workflow in this situation
			if ($this->status & ~(SQ_SC_STATUS_PENDING | SQ_SC_STATUS_ALL_APPROVED)) {
				$dependant_parents = $GLOBALS['SQ_SYSTEM']->am->getLinks($this->id, SQ_SC_LINK_SIGNIFICANT, '', TRUE, 'minor', NULL, 1);
				if (!empty($dependant_parents)) {
					$am = $GLOBALS['SQ_SYSTEM']->am;
					foreach ($dependant_parents as $link) {
						$asset = $am->getAsset($link['majorid'], $link['major_type_code']);
						if ($asset->status > $new_status) {
							// we dont want to rollback, but we dont
							// want to change our status for real
							return TRUE;
						}
					}
				}
			}
		}

		// Place the workflow and status change in a transaction block
		$GLOBALS['SQ_SYSTEM']->changeDatabaseConnection('db2');
		$GLOBALS['SQ_SYSTEM']->doTransaction('BEGIN');

		$status = $this->getStatus();
		if (!$status->processStatusChange($new_status)) {
			$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
			$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();
			return FALSE;
		}

		if ($new_status != $old_status) {
			// the status has actually changed to a different one
			$db = MatrixDAL::getDb();

			$sql = 'UPDATE
						sq_ast
					SET
						status					= :status,
						status_changed			= :status_changed_date,
						status_changed_userid	= :userid
					WHERE
						assetid = :assetid';

			try {
				$query = MatrixDAL::preparePdoQuery($sql);
				MatrixDAL::bindValueToPdo($query, 'status',  $new_status);
				MatrixDAL::bindValueToPdo($query, 'status_changed_date',    ts_iso8601(time()));
				MatrixDAL::bindValueToPdo($query, 'userid',  $GLOBALS['SQ_SYSTEM']->currentUserId());
				MatrixDAL::bindValueToPdo($query, 'assetid', $this->id);
				MatrixDAL::execPdoQuery($query);
			} catch (Exception $e) {
				throw new Exception('Unable to update status of asset "'.$this->name.'" (#'.$this->id.'), due to database error: '.$e->getMessage());
			}

			$this->status = $new_status;
			unset($this->_tmp['status_object']);

			if ($run_updated) $this->_updated($update_parents);

			// if we have just made this asset live, we update its minor version
			if ($new_status == SQ_STATUS_LIVE) {
				if (!$this->incrementVersion('minor', FALSE)) {
					trigger_localised_error('SYS0271', E_USER_WARNING, $this->name);
					$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
					$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();
					return FALSE;
				}
			}
		}

		// Commit before we broadcast the status change event (bug #5232)
		$GLOBALS['SQ_SYSTEM']->doTransaction('COMMIT');
		$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();

		if ($new_status != $old_status) {
			// broadcast event notifying of status change
			$em = $GLOBALS['SQ_SYSTEM']->getEventManager();
			$data = Array(
						'old_status'	=> $old_status,
						'new_status'	=> $new_status,
					);
			$em->broadcastEvent($this, 'AssetStatusUpdate', $data);

			// send message notifying of status change
			require_once SQ_INCLUDE_PATH.'/general_occasional.inc';
			$ms = $GLOBALS['SQ_SYSTEM']->getMessagingService();
			$msg_reps = Array(
							'asset_name'	=> $this->name,
							'old_status'	=> get_status_description($old_status),
							'new_status'	=> get_status_description($new_status),
						);
			$message = $ms->newMessage(Array(), 'asset.status', $msg_reps);
			$message->parameters['assetid'] = $this->id;
			$message->send();

			// prepare the event data and fire trigger_event_status_changed
			$event_data['old_status'] = $old_status;
			$event_data['new_status'] = $new_status;
			if (isset($this->_tmp['old_urls'])) {
				$event_data['old_urls'] = $this->_tmp['old_urls'];
			}
			if (isset($this->_tmp['allow_unrestricted'])) {
				$event_data['allow_unrestricted'] = $this->_tmp['allow_unrestricted'];
			}
			$GLOBALS['SQ_SYSTEM']->broadcastTriggerEvent('trigger_event_status_changed', $this, $event_data);
		}//end if

		return TRUE;

	}//end processStatusChange()


	/**
	* Modifies the specified date to the specified timestamp
	*
	* The user field for the specified date is set to the userid specified. If
	* none is specified, then the current user is used
	*
	* @param string	$date_type	the date to update, which must be one of 'published' or 'created'
	* @param int	$time		the time to set the date to. If NULL then use current time
	* @param int	$userid		the userid that modified the specified date
	*
	* @return boolean
	* @access public
	*/
	public function setDate($date_type, $time=NULL, $userid=NULL)
	{
		if (!in_array($date_type, Array('published' ,'created'))) {
			trigger_localised_error('SYS0198', E_USER_WARNING);
			return FALSE;
		}

		if (is_null($userid)) {
			$userid = $GLOBALS['SQ_SYSTEM']->currentUserId();
		}
		if (is_null($time)) {
			if ($date_type == 'created') {
				$time = time();
			} else {
				// a NULL published date means that the asset has never been published
				// so we also set the userid to NULL
				$userid = NULL;
			}
		}

		require_once SQ_FUDGE_PATH.'/general/datetime.inc';

		// construct a string to represent the current date value
		if ($this->$date_type != NULL) {
			$old_date_string = readable_datetime($this->$date_type);
			$current_user = $date_type.'_userid';
			if ($this->$current_user != NULL) {
				$old_date_string .= ' by user #'.$this->$current_user;
			} else {
				$old_date_string .= ' by [unknown user]';
			}
		} else {
			$old_date_string = 'never '.$date_type;
		}

		// construct a string to represent the new date value
		if ($time != NULL) {
			$new_date_string = readable_datetime($time);
			if ($userid != NULL) {
				$new_date_string .= ' by user #'.$userid;
			} else {
				$new_date_string .= ' by [unknown user]';
			}
		} else {
			$new_date_string = 'never '.$date_type;
		}

		// if the date and user has not changed, dont process anything
		if ($this->$date_type == $time) {
			// if we are not changing dates - check if we are changing users
			$user_type = $date_type.'_userid';
			if ($this->$user_type == $userid) return TRUE;
		}

		// begin a transaction
		$GLOBALS['SQ_SYSTEM']->changeDatabaseConnection('db2');
		$GLOBALS['SQ_SYSTEM']->doTransaction('BEGIN');
		$db = MatrixDAL::getDb();

		$sql = 'UPDATE
					sq_ast
				SET
					'.$date_type.'			= :'.$date_type.'_date,
					'.$date_type.'_userid	= :userid
				WHERE
				assetid	= :assetid';

		try {
			$query = MatrixDAL::preparePdoQuery($sql);
			MatrixDAL::bindValueToPdo($query, $date_type.'_date',    ts_iso8601($time));
			MatrixDAL::bindValueToPdo($query, 'userid',  $userid);
			MatrixDAL::bindValueToPdo($query, 'assetid', $this->id);
			MatrixDAL::execPdoQuery($query);
		} catch (Exception $e) {
			throw new Exception('Unable to update '.str_replace('_', ' ', $date_type).' date of asset "'.$this->name.'" (#'.$this->id.'), due to database error: '.$e->getMessage());
		}

		$GLOBALS['SQ_SYSTEM']->doTransaction('COMMIT');
		$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();

		$ms = $GLOBALS['SQ_SYSTEM']->getMessagingService();
		$msg_reps = Array(
						'date_type'		=> $date_type,
						'asset_name'	=> $this->name,
						'old_date'		=> $old_date_string,
						'new_date'		=> $new_date_string,
					);
		$message = $ms->newMessage(Array(), 'asset.dates', $msg_reps);
		$message->parameters['assetid'] = $this->id;
		$message->send();

		eval('$this->'.$date_type.' = $time;');
		eval('$this->'.$date_type.'_userid = $userid;');

		$em = $GLOBALS['SQ_SYSTEM']->getEventManager();
		$em->broadcastEvent($this, 'AssetUpdate', Array($date_type));


		return TRUE;

	}//end setDate()


//--        LOCKING        --//


	/**
	* Returns a list of lock types available for this asset type
	*
	* @return array
	* @access public
	*/
	public function lockTypes()
	{
		$lock_types =  Array(
						'settings'		=> 1,
						'attributes'	=> 2,
						'permissions'	=> 4,
						'links'			=> 8,
						'workflow'		=> 16,
						'metadata'		=> 32,
						'lookups'		=> 64,
						'roles'			=> 256,
					   );

		// all locks
		$all = 0;
		foreach ($lock_types as $lock_type) {
			$all = $all | $lock_type;
		}
		$lock_types['all'] = $all;

		// a menu lock allows editing of both attributes (such as name etc)
		// and links (such as position in menu)
		$lock_types['menu']			= $lock_types['attributes'] | $lock_types['links'];
		$lock_types['lookupValues']	= $lock_types['attributes'] | $lock_types['links'] | $lock_types['lookups'];

		return $lock_types;

	}//end lockTypes()


	/**
	* Can the current user forceably acquire the current lock on this asset?
	*
	* @param string	$lock_type	any valid lock type for this asset's type,
	*							plus 'all' which will attempt to acquire all locks that this asset has
	*
	* @return boolean
	* @access public
	*/
	public function canForceablyAcquireLock($lock_type)
	{
		$current_locks = $GLOBALS['SQ_SYSTEM']->am->getLockInfo($this->id, $lock_type, TRUE, FALSE);

		// lock type not known
		if (empty($current_locks)) return FALSE;

		foreach ($current_locks as $lock) {
			if (empty($lock)) continue;
			$user = NULL;
			if ($lock['userid']) {
				$user = $GLOBALS['SQ_SYSTEM']->am->getAsset($lock['userid']);
			}
			// lets work out if the current user has a high
			// enough level of access to forceably acquire the lock
			if (!is_null($user)) {
				// locked by root, no-one can force acquire...
				if ($GLOBALS['SQ_SYSTEM']->userRoot($user)) {
					return FALSE;
				}

				// locked by a sysadmin? only root can force acquire...
				if ($GLOBALS['SQ_SYSTEM']->userSystemAdmin($user) && !$GLOBALS['SQ_SYSTEM']->userRoot()) {
					return FALSE;
				}
			}

			// locked by someone else? need to be a system admin to acquire this lock
			if (!$GLOBALS['SQ_SYSTEM']->userRoot() && !$GLOBALS['SQ_SYSTEM']->userSystemAdmin()) {
				return FALSE;
			}

		}//end foreach
		return TRUE;

	}//end canForceablyAcquireLock()


	/**
	* Returns any locks that are needed for editing, based on a set of custom keywords
	*
	* @param array	$keywords	the keywords to govern what locks are required
	*
	* @return array
	* @access public
	*/
	public function getEditingLocks($keywords)
	{
		return Array();

	}//end getEditingLocks()


//--        LINKING        --//


	/**
	* Returns an array of all the permitted links type, the type asset and the cardinality
	* In the form
	*
	*   Array('[link_type]' => Array('[type_code]' => Array('card' => [cardinality], 'exclusive' => [exclusive])));
	* Where:
	*   link_type   = SQ_LINK_TYPE_1|SQ_LINK_TYPE_2|SQ_LINK_TYPE_3|SQ_LINK_NOTICE
	*   cardinality = [max number of links]|'M'
	*   exclusive   = TRUE|FALSE
	*
	* @return array
	* @access private
	*/
	public function _getAllowedLinks()
	{
		return Array(
				SQ_LINK_TYPE_1	=> Array(),
				SQ_LINK_TYPE_2	=> Array(),
				SQ_LINK_TYPE_3	=> Array(),
				SQ_LINK_NOTICE	=> Array(
									'image'				=> Array('card' => 'M', 'exclusive' => FALSE),
									'design'			=> Array('card' => 'M', 'exclusive' => FALSE),
									'paint_layout_page'	=> Array('card' => 'M', 'exclusive' => FALSE),
								   ),
			   );

	}//end _getAllowedLinks()


	/**
	* Creates a link between this asset and the passed asset, of a certain type
	* and returns the linkid of the created link
	*
	* @param Asset		$minor		the minor asset that we are linking to
	* @param string		$link_type	the type of link this is
	* @param string		$value		the value that is to be associated with this link
	* @param string		$sort_order	the position in the links list that this link should take,
	*								if NULL or less than zero places at end of list
	* @param string		$dependant	'0' / '1' on whether the this asset is dependant on the
	*								asset that will be linked by this new link
	* @param string		$exclusive	'0' / '1' on whether the this asset is linked exclusivly
	*								to the asset that will be linked by this new link
	* @param boolean	$moving		moving
	* @param string     $locked     whether or not this asset's link is locked
	*
	* @return int
	* @access public
	* @see Asset_Manager::createAssetLink()
	*/
	public function createLink(Asset $minor, $link_type, $value='', $sort_order=NULL, $dependant='0', $exclusive='0', $moving=FALSE, $locked=0)
	{
		if (!$this->id) return 0;
		return $GLOBALS['SQ_SYSTEM']->am->createAssetLink($this, $minor, $link_type, $value, $sort_order, $dependant, $exclusive, $moving, $locked);

	}//end createLink()


	/**
	* Prepares for linking by checking that the link values are valid for this link
	*
	* This function will be called if this asset is the major or minor party in the link, so the side_of_link flag
	* should be checked to work out what side of the link this asset is on. The return value should indicate
	* if any of the link fields were changed.
	*
	* @param Asset	$asset			the major or minor asset that we are linking to
	* @param string	$side_of_link	the side of the link we are on (major or minor)
	* @param string	&$link_type		the type of link we are creating
	* @param string	&$value			the value that is to be associated with the link
	* @param string	&$sort_order	the position in the links list that this link should take,
	* @param string	&$dependant		'0' / '1' on whether the this asset is dependant on the
	*								asset that will be linked by the new link
	* @param string	&$exclusive		'0' / '1' on whether the this asset is linked exclusivly
	*								to the asset that will be linked by the new link
	*
	* @return boolean
	* @access public
	*/
	public function prepareLink(Asset $asset, $side_of_link, &$link_type, &$value, &$sort_order, &$dependant, &$exclusive)
	{
		return FALSE;

	}//end prepareLink()


	/**
	* Returns TRUE if the passed minor asset can be linked to this asset, or a string with the error msg
	*
	* @param Asset	$minor		the minor asset that we are linking to
	* @param string	$link_type	the type of link this is
	* @param int	$exclusive	the exclusive status of the link (gets passed to canLinkToType)
	*
	* @return mixed boolean|string
	* @access public
	*/
	public function canCreateLink(Asset $minor, $link_type, $exclusive)
	{
		if (!$this->id) return translate('asset_not_created');
		return $GLOBALS['SQ_SYSTEM']->am->canCreateLink($this, $minor, $link_type, $exclusive);

	}//end canCreateLink()


	/**
	* Returns TRUE if the passed minor asset can be move-linked to this asset, or a string with the error msg
	*
	* By default, use the same tests as being newly linked with a non-exclusive link
	* (as all moved links are non-exclusive by moveLink() specification) - that is,
	* call canCreateLink().
	* However this may not be appropriate for moves in some circumstances (cf. Sites
	* within a Site Network) so this can be used to provide a different check for them.
	* Remember that this is called for the asset we are moving TO - as it would
	* have with canCreateLink().
	*
	* @param Asset	$minor		the minor asset that we are linking to
	* @param Asset	$old_major	the major asset that we are being moved from
	* @param string	$link_type	the type of link this is
	*
	* @return mixed boolean|string
	* @access public
	*/
	public function canMoveLink(Asset $minor, Asset$old_major, $link_type)
	{
		return $this->canCreateLink($minor, $link_type, 0);

	}//end canMoveLink()


	/**
	* Return a human readable description of the passed link
	*
	* @param int	$linkid	the link ID of the link to describe
	*
	* @return string
	* @access public
	*/
	public function describeLink($linkid)
	{
		return '';

	}//end describeLink()


	/**
	* Return TRUE if this link is deletable
	*
	* @param int	$linkid	the link id of the link to remove
	*
	* @return boolean
	* @access public
	*/
	public function isDeletableLink($linkid)
	{
		return TRUE;

	}//end isDeletableLink()


	/**
	* Determine if we can delete the passed link
	*
	* This function ignores locks, and returns TRUE or error message
	*
	* @param int	$linkid	the link id of the link to remove
	*
	* @return mixed boolean|string
	* @access public
	*/
	public function canDeleteLink($linkid)
	{
		if (!$this->id) return translate('asset_not_created');
		if (($err_msg = $this->isDeletableLink($linkid)) !== TRUE) {
			return $err_msg;
		}
		// we only need permissions, not effective access, to delete a significant link
		if (!$this->writeAccess()) {
			return translate('permission_denied');
		}
		return TRUE;

	}//end canDeleteLink()


	/**
	* Remove a link where this asset is the major player
	*
	* @param int		$linkid			the link id of the link to remove
	* @param boolean	$check_locked	Check if the link is locked or not
	* @param array      $link           The link (if we've already worked it out).
	*
	* @return boolean
	* @access public
	* @see Asset_Manager::deleteAssetLink()
	*/
	public function deleteLink($linkid, $check_locked=TRUE, $link=NULL)
	{
		if ($link === NULL || empty($link)) {
			// Don't pass the current asset id to getLinkById(), so we can pass
			// info about the whole link (both major and minor sides) to
			// deleteAssetLink().
			$link = $GLOBALS['SQ_SYSTEM']->am->getLinkById($linkid);
		}

		if (empty($link)) {
			trigger_localised_error('SYS0243', E_USER_NOTICE, $linkid);
			return FALSE;
		} else {
			return $GLOBALS['SQ_SYSTEM']->am->deleteAssetLink($linkid, $check_locked, $link);
		}

	}//end deleteLink()


	/**
	* Allow any required processesing to occur when any link is updated for the asset
	*
	* Called whenever any type of link is changed. Allows caching of objects, links and other stuff
	* to occur but not cause integrity issues during script execution.
	* ONLY TO BE CALLED FROM THE ASSET MANAGER
	*
	* @return void
	* @access protected
	*/
	public function linksUpdated()
	{


	}//end linksUpdated()


	/**
	* Return TRUE if this link can be cloned
	*
	* @return boolean
	* @access public
	*/
	public function canCloneLink()
	{
		return TRUE;

	}//end isDeletableLink()


//--        ATTRIBUTES        --//


	/**
	* Save attributes set with setAttrValue to the database
	*
	* @param boolean	$dont_run_updated	if TRUE this->_updated() won't be run
	* @param boolean	$log_message	if FALSE attrubutes change won't be logged in system log
	*
	*
	* @return boolean
	* @access public
	* @see setAttrValue()
	*/
	public function saveAttributes($dont_run_updated=FALSE, $log_message=TRUE)
	{
		if (!$this->id) return TRUE;
		if (empty($this->_tmp['vars_set'])) return TRUE;

		if (!$GLOBALS['SQ_REVERT_TO_SYSTEM_VERSION'] && !$this->writeAccess('attributes')) {
			trigger_localised_error('CORE0121', E_USER_WARNING, $this->name, $this->id);
			return FALSE;
		}

		$save_vars = Array();
		$attr_ids  = Array();
		$contextid = $GLOBALS['SQ_SYSTEM']->getContextId();

		$db = MatrixDAL::getDb();

		// open a queue for all messages we are going to be logging
		$ms = $GLOBALS['SQ_SYSTEM']->getMessagingService();
		$ms->openLog();

		$changed_array = Array();

		foreach ($this->_tmp['vars_set'] as $var_name => $var_data) {
			$attr_id = $this->vars[$var_name]['attrid'];
			$is_contextable = array_get_index($this->vars[$var_name], 'is_contextable', FALSE) ? TRUE : FALSE;

			// if this is a unique attribute then let's make sure that this isn't already set
			$owning_attributeid = 0;
			$attribute = $this->getAttribute($var_name);
			if ($attribute->uniq) {

				$sql = 'SELECT oa.attrid
						FROM sq_ast_attr a
						  INNER JOIN sq_ast_attr oa ON (a.owning_type_code = oa.type_code AND a.name = oa.name)
						WHERE a.name      = :name
							AND a.type_code = :type_code';

				try {
					$query = MatrixDAL::preparePdoQuery($sql);
					MatrixDAL::bindValueToPdo($query, 'name',      $var_name);
					MatrixDAL::bindValueToPdo($query, 'type_code', $this->type());
					$owning_attributeid = MatrixDAL::executePdoOne($query);
				} catch (Exception $e) {
					throw new Exception('Cannot check for attribute value "'.$name.'" of type code "'.$this->type().'" due to database error: '.$e->getMessage());
				}

			}// end if

			$attr_value = $this->vars[$var_name]['value'];
			if ($this->vars[$var_name]['type'] == 'serialise') {
				$attr_value = serialize($attr_value);
			}
			$save_vars[$attr_id]['value'] = $attr_value;
			$save_vars[$attr_id]['name']  = $var_name;
			$save_vars[$attr_id]['owning_attrid']  = $owning_attributeid;
			$attr_ids[] = $attr_id;

			// log a message for the asset - this wont get logged until we close the queue
			if($log_message) {
			    $msg_reps = Array(
							    'asset_name'	=> $this->name,
							    'attr_name'		=> $attribute->name,
						    );
			    $msg_type = 'asset.attributes.fulllog';
			    if (is_scalar($var_data['old_value']) && is_scalar($attr_value)) {
				    $msg_reps['old_value'] = $var_data['old_value'];
				    $msg_reps['new_value'] = $attr_value;
				    $msg_type .= '.scalar';
			    }
			    $message = $ms->newMessage(Array(), $msg_type, $msg_reps);
			    $message->parameters['assetid'] = $this->id;
			    $ms->logMessage($message);

			    // log a smaller message for users to view - this wont get logged until we close the queue
			    require_once SQ_FUDGE_PATH.'/general/general.inc';
			    $msg_type = 'asset.attributes';
			    if (is_scalar($var_data['old_value']) && is_scalar($attr_value)) {
				    $msg_reps['old_value'] = ellipsisize($var_data['old_value'],30);
				    $msg_reps['new_value'] = ellipsisize($attr_value,30);
				    $msg_type .= '.scalar';
			    }

			    $sml_message = $ms->newMessage(Array(), $msg_type, $msg_reps);
			    $sml_message->parameters['assetid'] = $this->id;
			    $ms->logMessage($sml_message);
			}

			// add attribute to the event broadcaster array (for notification of what has changed)
			// this allows event listener to act only on what has changed

			$changed_array[] = $attribute->name;

		}//end foreach

		$GLOBALS['SQ_SYSTEM']->changeDatabaseConnection('db2');
		$GLOBALS['SQ_SYSTEM']->doTransaction('BEGIN');
		$db = MatrixDAL::getDb();

		// find any previous entries
		$atrr_id_cond = 'IN ('.implode(', ', $attr_ids).')';
		$sql   = 'SELECT attrid FROM '.SQ_TABLE_RUNNING_PREFIX.'ast_attr_val ';
		$where = 'assetid = :assetid
					AND attrid '.$atrr_id_cond.'
					AND contextid = :contextid';
		$where = $GLOBALS['SQ_SYSTEM']->constructRollbackWhereClause($where);

		try {
			$query = MatrixDAL::preparePdoQuery($sql.$where);
			MatrixDAL::bindValueToPdo($query, 'assetid', $this->id);
			MatrixDAL::bindValueToPdo($query, 'contextid', $contextid);
			$existing = MatrixDAL::executePdoAssoc($query, 0);
		} catch (Exception $e) {
			throw new Exception('Unable to get attributes for asset "'.$this->name.'" (#'.$this->id.') due to database error: '.$e->getMessage());
		}

		$new = array_diff($attr_ids, $existing);

		// update existing custom attribute values
		if (!empty($existing)) {

			foreach ($existing as $attr_id) {
				$attr_id = (int) $attr_id;
				$attr_value = $save_vars[$attr_id]['value'];

				// if this is a unique attribute then let's make sure we register that in the unique value table
				$attribute = $this->getAttribute($save_vars[$attr_id]['name']);
				$is_contextable = array_get_index($this->vars[$save_vars[$attr_id]['name']], 'is_contextable', FALSE) ? TRUE : FALSE;

				try {
					if (($contextid === 0) || ($is_contextable === FALSE) || (array_key_exists('__creating__', $this->_tmp) === TRUE)) {
						// If updating in the default context, we should be updating all
						// context values that are still using the default
						// If not contextable, just keep them linked together as well
						$unique_query_name = 'updateDefaultContextUniqueAttrValue';
						$query_name = 'updateDefaultContextAttrValue';
						$bind_vars = Array(
										'custom_val'	=> $attr_value,
										'assetid'		=> $this->id,
									 );
					} else {
						// Updating in alternate context. Set the value and mark that
						// we have now broken away from the default
						$unique_query_name = 'updateAlternateContextUniqueAttrValue';
						$query_name = 'updateAlternateContextAttrValue';
						$bind_vars = Array(
										'custom_val'	=> $attr_value,
										'contextid'		=> $contextid,
										'assetid'		=> $this->id,
									 );
						$this->vars[$save_vars[$attr_id]['name']]['use_default'] = 0;
					}
					if ((boolean)$attribute->uniq === TRUE) {
						$bind_vars['attrid'] = $save_vars[$attr_id]['owning_attrid'];
						MatrixDAL::executeQuery('core', $unique_query_name, $bind_vars);
					}
					$bind_vars['attrid'] = $attr_id;
					MatrixDAL::executeQuery('core', $query_name, $bind_vars);
				} catch (Exception $e) {
					throw new Exception('Unable to update attribute values for asset "'.$this->name.'" (#'.$this->id.') due to database error: '.$e->getMessage());
				}

			}//end foreach

		}//end updating existing custom attribute values

		// insert new custom attribute values
		if (!empty($new)) {

			foreach ($new as $attr_id) {
				$attr_id = (int) $attr_id;
				$attr_value = $save_vars[$attr_id]['value'];

				// if this is a unique attribute then let's make sure we register that in the unique value table
				$attribute = $this->getAttribute($save_vars[$attr_id]['name']);
				$is_contextable = array_get_index($this->vars[$save_vars[$attr_id]['name']], 'is_contextable', FALSE) ? TRUE : FALSE;

				try {
					if (($contextid === 0) || ($is_contextable === FALSE) || (array_key_exists('__creating__', $this->_tmp)  === TRUE)) {
						// Inserting into the default context. Insert into all other
						// existing contexts that don't yet have a value set, and
						// mark them as using the default
						$unique_query_name = 'insertDefaultContextUniqueAttrValue';
						$query_name = 'insertDefaultContextAttrValue';
						$bind_vars = Array(
										'custom_val'	=> $attr_value,
										'assetid'		=> $this->id,
									 );
						// Oracle has problems handling INSERT..SELECT, because we
						// can't make the DAL know whether it's a LOB or not.
						// Changes have been made to MatrixDAL so we can pass LOB
						// objects to bindings DAL has not assigned to a field.
						// Two LOBs are needed if we are inserting unique values,
						// since one is invalidated after each query is run.
						if (MatrixDAL::getDbType() === 'oci') {
							if ((boolean)$attribute->uniq === TRUE) {
								$lob1 = oci_new_descriptor(MatrixDAL::getDb(), OCI_D_LOB);
								$lob1->writeTemporary($bind_vars['custom_val'], OCI_TEMP_CLOB);
							}
							$lob2 = oci_new_descriptor(MatrixDAL::getDb(), OCI_D_LOB);
							$lob2->writeTemporary($bind_vars['custom_val'], OCI_TEMP_CLOB);
						}
					} else {
						// Inseting into an alternate context. (Strange...)
						// Set up the row, set as broken away from default
						$unique_query_name = 'insertAlternateContextUniqueAttrValue';
						$query_name = 'insertAlternateContextAttrValue';
						$bind_vars = Array(
										'custom_val'	=> $attr_value,
										'contextid'		=> $contextid,
										'assetid'		=> $this->id,
									 );
						$this->vars[$save_vars[$attr_id]['name']]['use_default'] = 0;
					}
					if ((boolean)$attribute->uniq === TRUE) {
						if (isset($lob1) === TRUE) $bind_vars['custom_val'] = $lob1;
						$bind_vars['attrid'] = $save_vars[$attr_id]['owning_attrid'];
						MatrixDAL::executeQuery('core', $unique_query_name, $bind_vars);
					}
					if (isset($lob2) === TRUE) $bind_vars['custom_val'] = $lob2;
					$bind_vars['attrid'] = $attr_id;
					MatrixDAL::executeQuery('core', $query_name, $bind_vars);
				} catch (Exception $e) {
					throw new Exception('Unable to insert new attribute values for asset "'.$this->name.'" (#'.$this->id.') due to database error: '.$e->getMessage());
				}

			}//end foreach

		}//end new custom attribute values

		// tell, the asset it has updated
		if (!$dont_run_updated && !$this->_updated()) {
			$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
			$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();
			$ms->abortLog();
			return FALSE;
		}

		// if we get this far, then it's all OK
		$GLOBALS['SQ_SYSTEM']->doTransaction('COMMIT');
		$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();
		$ms->closeLog();

		if (!empty($this->_tmp['vars_set'])) {
			// prepare to broadcast the attributes_changed
			foreach ($this->_tmp['vars_set'] as $attr => $details) {
				$changed_vars[$attr] = Array(
										'type'		=> $this->vars[$attr]['type'],
										'old_value'	=> $details['old_value'],
										'new_value'	=> $this->attr($attr),
									   );
			}
			$GLOBALS['SQ_SYSTEM']->broadcastTriggerEvent('trigger_event_attributes_changed', $this, $changed_vars);
		}

		unset($this->_tmp['vars_set']);

		// notify anyone interested that attributes changed
		$em = $GLOBALS['SQ_SYSTEM']->getEventManager();
		$em->broadcastEvent($this, 'attributeChange', $changed_array);
		$GLOBALS['SQ_SYSTEM']->broadcastTriggerEvent('trigger_event_attributes_saved', $this);

		return TRUE;

	}//end saveAttributes()


	/**
	* Queue the setting of the value of an attribute
	*
	* This function sets the value of the attribute locally, but does not database updating.
	* The change is queued awaiting the call of saveAttributes()
	*
	* @param string	$name	the name of the attribute
	* @param string	$value	the new value of the attribute
	*
	* @return boolean
	* @access public
	* @see saveAttributes()
	*/
	public function setAttrValue($name, $value)
	{
		if (empty($this->vars[$name])) {
			trigger_localised_error('SYS0093', E_USER_WARNING, $name, $this->type());
			return FALSE;
		}

		if(!SQ_IN_BACKEND && !SQ_IN_LIMBO && !SQ_IN_LOGIN && !SQ_IN_CRON && !SQ_PHP_CLI) {
			$filter_enabled = $GLOBALS['SQ_SYSTEM']->getUserPrefs('user', 'SQ_USER_FILTER_FRONT_END_INPUT');
			if(isset($GLOBALS['SQ_REVERT_TO_SYSTEM_VERSION'] ) && $GLOBALS['SQ_REVERT_TO_SYSTEM_VERSION'])
			    $filter_enabled = FALSE;
			else if (isset($GLOBALS['SQ_CLONE_COMPONENTS'] ) && $GLOBALS['SQ_CLONE_COMPONENTS'])
			    $filter_enabled = FALSE;

			//Any identified attributes that shouldn't be filtered
			$type_excl = Array ('form_submission');
			$attr_excl = Array ('xml');
			if ($filter_enabled && !(in_array($this->type(), $type_excl) && in_array($name, $attr_excl))){
				if (!empty($value)) {
					$value = filter_content($value);
					if ($value === FALSE){
						trigger_localised_error('SYS0346', E_USER_WARNING, $name, $this->type());
						return FALSE;
					}
				}
			}
		}

		$attribute = $this->getAttribute($name);
		if (!$attribute->setValue($value)) {
			trigger_localised_error('SYS0073', E_USER_WARNING, $value, $name);
			$attribute->setValue($this->vars[$name]['value']);
			return FALSE;
		}

		if ($this->vars[$name]['type'] == 'serialise') {
			$value = unserialize($value);
		}

		// if the value being set is the same as the current value - don't do anything
		// take into consideration Oracle's treatment of empty strings - they are retrieved as NULL values
		// this condition is satisfied when assigning an empty string in place of a pre-existing NULL value in the case of Oracle systems
		if (($this->vars[$name]['value'] === $value) || (is_null($this->vars[$name]['value']) && $value === '')) {
			return TRUE;
		}

		// if this is a uniq attribute then let's make sure that this isn't already set
		if ($attribute->uniq) {
			$db = MatrixDAL::getDb();

			$sql = 'SELECT COUNT(*)
					  FROM '.SQ_TABLE_RUNNING_PREFIX.'ast_attr_uniq_val uv
						INNER JOIN '.SQ_TABLE_RUNNING_PREFIX.'ast_attr oa ON uv.owning_attrid = oa.attrid
						INNER JOIN sq_ast_attr a ON (oa.type_code = a.owning_type_code AND oa.name = a.name)
					  ';
			$where = ' uv.custom_val =    :custom_val
						AND a.name =      :name
						AND a.type_code = :type_code
						AND uv.contextid = :contextid';
			$where = $GLOBALS['SQ_SYSTEM']->constructRollbackWhereClause($where, 'uv');

			try {
				$query = MatrixDAL::preparePdoQuery($sql.$where);
				MatrixDAL::bindValueToPdo($query, 'custom_val', $value);
				MatrixDAL::bindValueToPdo($query, 'name',       $name);
				MatrixDAL::bindValueToPdo($query, 'type_code',  $this->type());
				MatrixDAL::bindValueToPdo($query, 'contextid',  $GLOBALS['SQ_SYSTEM']->getContextId());
				$result = MatrixDAL::executePdoOne($query);
			} catch (Exception $e) {
				throw new Exception('Cannot check for unique attribute value "'.$name.'" of type code "'.$this->type().'" due to database error: '.$e->getMessage());
			}

			if ((int) $result > 0) {
				trigger_localised_error('SYS0251', E_USER_WARNING, $name, $value);
				$attribute->setValue($this->vars[$name]['value']);
				return FALSE;
			}

		}// end if

		if (!isset($this->_tmp['vars_set'])) {
			$this->_tmp['vars_set'] = Array();
		}
		if (is_scalar($this->vars[$name]['value'])) {
			$this->_tmp['vars_set'][$name]['old_value'] = $this->vars[$name]['value'];
		} else {
			$this->_tmp['vars_set'][$name]['old_value'] = '__(old value)__';
		}

		$this->vars[$name]['value'] = $value;



		return TRUE;

	}//end setAttrValue()


	/**
	* Get the value of an attribute
	*
	* @param string	$name	the name of the attribute whose value you want to get
	*
	* @return mixed string|NULL
	* @access public
	*/
	public function attr($name)
	{
		if (!isset($this->vars[$name])) {
			trigger_localised_error('SYS0092', E_USER_WARNING, $name, $this->name, $this->id);
			return NULL;
		}

		$return_value = $this->vars[$name]['value'];

		// remove any sq_wysiwyg_* tags used as placeholders for plugins
		// such as Embed Movie/Youtube but only on frontend
		if ($this->vars[$name]['type'] == 'wysiwyg' && !SQ_IN_BACKEND && !SQ_IN_LIMBO) {
			$return_value = preg_replace('|<sq_wysiwyg_([^ >]*)([^>]*)>(.*)</sq_wysiwyg_\1>|si', '$3', $return_value);
		}

		return $return_value;

	}//end attr()


	/**
	* Get the value of an attribute by reference
	*
	* @param string	$name	the name of the attribute whose value you want to get
	*
	* @return mixed object|NULL
	* @access public
	*/
	public function &attrByRef($name)
	{
		if (!isset($this->vars[$name])) {
			trigger_localised_error('SYS0092', E_USER_WARNING, $name, $this->name, $this->id);
			return NULL;
		}
		return $this->vars[$name]['value'];

	}//end attrByRef()


	/**
	* Returns a reference to the actual attribute object, used for manipulating the object
	*
	* @param string		$name			the name of the attribute
	* @param boolean	$mute_errors	stops the outputting of errors in this fn, needed
	*									because you can't use the '@' operator when
	*									returning by reference
	*
	* @return mixed object|NULL
	* @access public
	*/
	public function getAttribute($name, $mute_errors=FALSE)
	{
		if (!isset($this->_tmp['attributes'][$name]) || !is_object($this->_tmp['attributes'][$name])) {

			if (empty($this->_tmp['attributes'])) {
				$this->_tmp['attributes'] = Array();
			}

			if (empty($this->vars[$name])) {
				if (!$mute_errors) {
					trigger_localised_error('SYS0094', E_USER_WARNING, $name, $this->type());
				}

				$this->_tmp['attributes'][$name] = NULL;
			} else {

				require_once SQ_ATTRIBUTES_PATH.'/'.$this->vars[$name]['type'].'/'.$this->vars[$name]['type'].'.inc';
				$attr_class = 'Asset_Attribute_'.$this->vars[$name]['type'];
				$this->_tmp['attributes'][$name] = new $attr_class($this->vars[$name]['attrid'], $this->vars[$name]['value']);

			}//end if

		}//end if

		return $this->_tmp['attributes'][$name];

	}//end getAttribute()


	/**
	* Return the keywords for this asset type with their replacements for this asset instance
	*
	* This function is called in asset-listing situations like the asset listing, search page etc
	* The return value is in the form:
	* <PRE>
	* Array(
	*   'name' => Array (
	*               'value'       => 'page',
	*               'description' => 'the asset name',
	*             ),
	* );
	* </PRE>
	*
	* @param boolean	$descriptions	if TRUE descriptions will be included in the array
	*
	* @deprecated
	* @return array
	* @access public
	*/
	public function getAssetKeywords($descriptions=FALSE)
	{

		$keywords = Array();
		$default_keywords = Array(
								'asset_name',
								'asset_type',
								'asset_short_name',
								'asset_version',
								'asset_version_major',
								'asset_version_minor',
								'asset_version_micro',
								'asset_created',
								'asset_updated',
								'asset_created_short',
								'asset_updated_short',
								'asset_created_readable',
								'asset_updated_readable',
								'asset_created_iso8601',
								'asset_updated_iso8601',
								'asset_created_rfc2822',
								'asset_updated_rfc2822',
								'asset_created_ical',
								'asset_updated_ical',
								'asset_created_by_name',
								'asset_updated_by_name',
								'asset_published',
								'asset_published_short',
								'asset_published_readable',
								'asset_published_iso8601',
								'asset_published_rfc2822',
								'asset_published_ical',
								'asset_published_by_name',
								'asset_status_changed',
								'asset_status_changed_short',
								'asset_status_changed_readable',
								'asset_status_changed_iso8601',
								'asset_status_changed_rfc2822',
								'asset_status_changed_ical',
								'asset_status_changed_by_name',
								'asset_url',
								'asset_href',
								'asset_thumbnail',
								'asset_thumbnail_url',
								'asset_thumbnail_caption',
								'asset_thumbnail_assetid',
								'asset_thumbnail_title',
								'asset_thumbnail_name',
							);

		$mm = $GLOBALS['SQ_SYSTEM']->getMetadataManager();
		$mm_keywords = $mm->generateKeywordReplacements($this, $default_keywords, FALSE);
		foreach ($default_keywords as $keyword) {
			// we need to check. otherwise we get a undefined index error if mm_keywords for
			// one of the default keywords is not set
			if(isset($mm_keywords[$keyword])) $keywords[$keyword]['value'] = $mm_keywords[$keyword];
		}

		foreach ($this->vars as $name => $info) {
			if ($info['type'] == 'serialise') continue;
			$keywords['asset_attribute_'.$name]['value'] = $info['value'];

			if ($descriptions) {
				$attr = $this->getAttribute($name);
				$keywords['asset_attribute_'.$name]['description'] = $attr->description;
			}
		}

		$keywords['asset_assetid']['value'] = $this->id;

		if ($descriptions) {
			$keywords['asset_assetid']['description']                     = 'The ID of the asset';
			$keywords['asset_name']['description']                        = 'Full name of the asset';
			$keywords['asset_type']['description']                        = 'Asset\'s type';
			$keywords['asset_short_name']['description']                  = 'Short name of the asset';
			$keywords['asset_version']['description']                     = 'Version of the asset being displayed';
			$keywords['asset_version_major']['description']               = 'Major version number of the asset being displayed';
			$keywords['asset_version_minor']['description']               = 'Minor version number of the asset being displayed';
			$keywords['asset_version_micro']['description']               = 'Micro version number of the asset being displayed';
			$keywords['asset_created']['description']                     = 'The date and time the asset was created (yyyy-mm-dd hh:mm:ss)';
			$keywords['asset_updated']['description']                     = 'The date and time the asset was last updated (yyyy-mm-dd hh:mm:ss)';
			$keywords['asset_status_updated']['description']              = 'The date and time the asset\'s status was last updated (yyyy-mm-dd hh:mm:ss)';
			$keywords['asset_published']['description']                   = 'The date and time the asset was last published (yyyy-mm-dd hh:mm:ss)';
			$keywords['asset_created_short']['description']               = 'The date when the asset was created (yyyy-mm-dd)';
			$keywords['asset_updated_short']['description']               = 'The date when the asset was last updated (yyyy-mm-dd)';
			$keywords['asset_published_short']['description']             = 'The date when the asset was last published (yyyy-mm-dd)';
			$keywords['asset_status_updated_short']['description']        = 'The date when the asset\'s status was last changed (yyyy-mm-dd)';
			$keywords['asset_created_readable']['description']            = 'The date when the asset was created (dd m yyyy h:mm[am pm])';
			$keywords['asset_updated_readable']['description']            = 'The date when the asset was last updated (dd m yyyy h:mm[am pm])';
			$keywords['asset_created_readabledate']['description']        = 'The date when the asset was created (date only - dd m yyyy)';
			$keywords['asset_updated_readabledate']['description']        = 'The date when the asset was last updated (date only - dd m yyyy)';
			$keywords['asset_created_readabletime']['description']        = 'The date when the asset was created (time only - h:mm[am pm])';
			$keywords['asset_updated_readabletime']['description']        = 'The date when the asset was last updated (time only - h:mm[am pm])';
			$keywords['asset_created_iso8601']['description']             = 'The date when the asset was created (ISO8601 format)';
			$keywords['asset_updated_iso8601']['description']             = 'The date when the asset was last updated (ISO8601 format)';
			$keywords['asset_published_iso8601']['description']           = 'The date when the asset was last published (ISO8601 format)';
			$keywords['asset_created_rfc2822']['description']             = 'The date when the asset was created (RFC2822 format)';
			$keywords['asset_updated_rfc2822']['description']             = 'The date when the asset was last updated (RFC2822 format)';
			$keywords['asset_published_rfc2822']['description']           = 'The date when the asset was last published (RFC2822 format)';
			$keywords['asset_created_ical']['description']                = 'The date when the asset was created (iCalendar format)';
			$keywords['asset_updated_ical']['description']                = 'The date when the asset was last updated (iCalendar format)';
			$keywords['asset_published_ical']['description']              = 'The date when the asset was last published (iCalendar format)';
			$keywords['asset_published_readable']['description']          = 'The date when the asset was last published (dd m yyyy h:mm[am pm])';
			$keywords['asset_status_changed_readable']['description']     = 'The date when the asset\'s status was last changed (dd m yyyy h:mm[am pm])';
			$keywords['asset_published_readabledate']['description']      = 'The date when the asset was last published (date only - dd m yyyy)';
			$keywords['asset_status_changed_readabledate']['description'] = 'The date when the asset\'s status was last changed (date only - dd m yyyy)';
			$keywords['asset_published_readabletime']['description']      = 'The date when the asset was last published (time only - h:mm[am pm])';
			$keywords['asset_status_changed_readabletime']['description'] = 'The date when the asset\'s status was last changed (time only - h:mm[am pm])';
			$keywords['asset_created_by_name']['description']             = 'The name of the user who created this asset';
			$keywords['asset_updated_by_name']['description']             = 'The name of the user who last updated this asset';
			$keywords['asset_published_by_name']['description']           = 'The name of the user who last published this asset';
			$keywords['asset_status_changed_by_name']['description']      = 'The name of the user who last changed the status of this asset';
			$keywords['asset_url']['description']                         = 'The absolute url to the asset';
			$keywords['asset_href']['description']                        = 'The relative href to the asset';
			$keywords['asset_thumbnail']['description']                   = 'The img tag for the thumbnail attached to the asset';
			$keywords['asset_thumbnail_url']['description']               = 'The URL only for the thumbnail attached to the asset';
			$keywords['asset_thumbnail_caption']['description']           = 'The caption for the thumbnail attached to the asset';
			$keywords['asset_thumbnail_assetid']['description']           = 'The asset id for the thumbnail attached to the asset';
			$keywords['asset_thumbnail_title']['description']             = 'The friendly name for the thumbnail attached to the asset';
			$keywords['asset_thumbnail_name']['description']              = 'The file name for the thumbnail attached to the asset';

		}//end if

		return $keywords;

	}//end getAssetKeywords()


	/**
	* Return the keywords for this asset type.
	*
	* This function is called in asset-listing situations like the asset listing, search page etc
	* The return value is in the form:
	* <PRE>
	* Array(
	*   'name' => 'description' => 'the asset name',
	* );
	* </PRE>
	*
	* @return array
	* @access public
	*/
	public function getAvailableKeywords()
	{

		$keywords = Array();
		$default_keywords = Array(
								'asset_assetid',
								'asset_name',
								'asset_type',
								'asset_short_name',
								'asset_version',
								'asset_version_major',
								'asset_version_minor',
								'asset_version_micro',
								'asset_created',
								'asset_updated',
								'asset_created_short',
								'asset_updated_short',
								'asset_created_readable',
								'asset_updated_readable',
								'asset_created_readabledate',
								'asset_updated_readabledate',
								'asset_created_readabletime',
								'asset_updated_readabletime',
								'asset_created_iso8601',
								'asset_updated_iso8601',
								'asset_created_rfc2822',
								'asset_updated_rfc2822',
								'asset_created_ical',
								'asset_updated_ical',
								'asset_created_by_name',
								'asset_updated_by_name',
								'asset_published',
								'asset_published_short',
								'asset_published_readable',
								'asset_published_readabledate',
								'asset_published_readabletime',
								'asset_published_iso8601',
								'asset_published_rfc2822',
								'asset_published_ical',
								'asset_published_by_name',
								'asset_status_changed',
								'asset_status_changed_short',
								'asset_status_changed_readable',
								'asset_status_changed_readabledate',
								'asset_status_changed_readabletime',
								'asset_status_changed_iso8601',
								'asset_status_changed_rfc2822',
								'asset_status_changed_ical',
								'asset_status_changed_by_name',
								'asset_url',
								'asset_href',
								'asset_thumbnail',
								'asset_thumbnail_url',
								'asset_thumbnail_caption',
								'asset_thumbnail_assetid',
								'asset_thumbnail_title',
								'asset_thumbnail_name',
								'asset_status_description',
								'asset_status_colour',
							);

		// add default keywords first
		foreach ($default_keywords as $keyword) {
			$keywords[$keyword] = translate($keyword);
		}

		// add attribute
		foreach ($this->vars as $name => $info) {
			$attr = $this->getAttribute($name);
			if (!$attr->is_admin) {
				$keywords['asset_attribute_'.$name] = empty($attr->description) ? 'Asset Attribute: '.$name : $attr->description;
			}
		}

		return $keywords;

	}//end getAvailableKeywords()


	/**
	* Get bodycopy keyword replacement
	* Provides a mechanism for providing keyword replacement functions by
	* looking up the function named 'getKeywordNameKeywordReplacement'
	* so an asset wishing to add support for 'best_ever' custom keyword
	* should implement a function getBestEverKeywordReplacement()
	* which returns a replacement string
	*
	* Note that this function returns NULL for any keywords it doesn't
	* recognise, unless the keyword begins with %globals_
	*
	* @param string	$keyword	Keyword to find a replacement for
	*
	* @return string
	* @access public
	* @see _getKeywordReplacement
	*/
	public function getKeywordReplacement($keyword)
	{
		$replacement = $this->_getKeywordReplacement($keyword);
		return $replacement;

	}//end getKeywordReplacement()


	/**
	* Get bodycopy keyword replacement
	* Provides a mechanism for providing keyword replacement functions by
	* looking up the function named 'getKeywordNameKeywordReplacement'
	* so an asset wishing to add support for 'best_ever' custom keyword
	* should implement a function getBestEverKeywordReplacement()
	* which returns a replacement string
	*
	* Note that this function returns NULL for any keywords it doesn't
	* recognise, unless the keyword begins with %globals_
	*
	* @param string	$keyword	Keyword to find a replacement for
	*
	* @return string
	* @access private
	*/
	protected function _getKeywordReplacement($keyword)
	{
		if (empty($keyword)) return '';

		if (($keyword == 'asset_contents') || (0 === strpos($keyword, 'globals_'))) {
			return "%$keyword%";
		}
		// The full keyword with the modifier(s), if any
		$full_keyword = $keyword;

		// Remove any modifiers from keyword, and keep them for later.
		$keyword = parse_keyword($keyword, $modifiers);
		$contextid = extract_context_modifier($modifiers);

		if ($contextid !== NULL) {
			// If we were able to extract a context ID to change to, and it's
			// different to our current one, then change and then reload a copy
			// of the asset in that context (as we would need to do anyway)

			if ((int)$contextid !== $GLOBALS['SQ_SYSTEM']->getContextId()) {
				$GLOBALS['SQ_SYSTEM']->changeContext($contextid);
				$contexted_asset = $GLOBALS['SQ_SYSTEM']->am->getAsset($this->id);

				// Get the keyword without any modifiers
				$replacement = $contexted_asset->getKeywordReplacement($keyword);

				// Then apply the ones we had remaining, then return it
				apply_keyword_modifiers($replacement, $modifiers, Array('assetid' => $contexted_asset->id));

				unset($contexted_asset);
				$GLOBALS['SQ_SYSTEM']->restoreContext();
				return $replacement;

			}//end if contextid is not the currently active context

		}//end if contextid is not NULL

		$replacement = NULL;
		$tmp_keyword = strtr($keyword, '_', ' ');
		$tmp_keyword = ucwords($tmp_keyword);
		$tmp_keyword = preg_replace('/\s+/', '', $tmp_keyword);
		$func_name = 'get'.$tmp_keyword.'KeywordReplacement';

		//check if the keyword starts with asset_ because the global asset keyword %globals_asset_% would invoke this
		//method through the replace_global_keywords() function (general.inc) with the keyword starts with asset_
		$possible_func_name = '';
		if (0 === strpos($keyword, 'asset_')) {
			//get the method name without the start "Asset"
			$possible_tmp_keyword = substr($tmp_keyword, 5);
			if ($possible_tmp_keyword != '') {
				$possible_func_name = 'get'.$possible_tmp_keyword.'KeywordReplacement';
			}
		}

		if (method_exists($this, $func_name)) {
			$replacement = $this->$func_name();
		} else if (($possible_func_name != '') && method_exists($this, $possible_func_name)) {
			//allow the use of %globals_asset_<keyword>:<assetid>% if the %<keyword>% is available for
			//the <assetid> asset through the get<Keyword>KeywordReplacement() method
			$replacement = $this->$possible_func_name();
		} else if (strcmp('asset_assetid', $keyword) == 0) {
			$replacement = $this->id;
		} else if (strcmp('asset_name', $keyword) == 0) {
			$replacement = $this->name;
			$replacement = str_replace('&', '&amp;', $replacement);
		} else if (strcmp('asset_name_linked', $keyword) == 0) {
			$name = str_replace('&', '&amp;', $this->name);
			$replacement = '<a href="'.$this->getURL().'">'.$name.'</a>';
		} else if (strcmp('asset_short_name', $keyword) == 0) {
			$replacement = $this->short_name;
			$replacement = str_replace('&', '&amp;', $replacement);
		} else if (strcmp('asset_type', $keyword) == 0) {
			$replacement = $GLOBALS['SQ_SYSTEM']->am->getTypeInfo($this->type(), 'name');
		} else if (strcmp('asset_type_icon', $keyword) == 0) {
			$replacement = sq_get_icon($GLOBALS['SQ_SYSTEM']->am->getAssetIconURL($this->type()), 16, 16, '');
		} else if (strcmp('asset_type_icon_url', $keyword) == 0) {
			$replacement = $GLOBALS['SQ_SYSTEM']->am->getAssetIconURL($this->type());
		} else if (0 === strpos($keyword, 'asset_attribute_')) {
			$attr_name = substr($keyword, strlen('asset_attribute_'));
			if (array_key_exists($attr_name, $this->vars)) {
				$attr = $this->getAttribute($attr_name);
				if (!is_null($attr) && !$attr->is_admin) {
					$replacement = $attr->getKeywordValue();
				}
			} else {
				preg_match('/(_([^_]*)){0,1}$/', $attr_name, $matches);
				if (!empty($matches[1])) {
					$attr_name = substr($attr_name, 0, (strlen($attr_name) - strlen($matches[1])));
					$format = $matches[2];
					$attr = $this->getAttribute($attr_name);
					if (!is_null($attr)) {
						$replacement = $attr->getKeywordValue($format);
					}
				}
			}
		} else if (substr($keyword, 0, 15) == 'asset_metadata_') {
			$metadata_keyword = substr($keyword, 15);
			$mm = $GLOBALS['SQ_SYSTEM']->getMetadataManager();

			$current_context = $GLOBALS['SQ_SYSTEM']->getContextId();
			if ($current_context === NULL) {
				$current_context = 0;
			}

			if (!isset($this->_tmp['mm_replacements'][$current_context])) {
				// generate all metadata keywords in one go
				$this->_tmp['mm_replacements'][$current_context] = $mm->getMetadataFieldValues($this->id);
			}
			$additional_keyword = '';
			$replacement = NULL;

			// With "additional" keywords, we don't know where our metadata
			// field name ends and the additional keyword begins...so we will
			// try and split it up by underscores
			while (strlen($metadata_keyword) > 0) {
				$spaced_metadata_keyword = str_replace('_', ' ', $metadata_keyword);

				if (isset($this->_tmp['mm_replacements'][$current_context][$metadata_keyword]) === TRUE) {
					if ($additional_keyword !== '') {
						$mm = $GLOBALS['SQ_SYSTEM']->getMetadataManager();
						$field_assetid = $mm->getFieldAssetIdFromName($this->id, $metadata_keyword);

						if ($field_assetid !== FALSE) {
							$field = $GLOBALS['SQ_SYSTEM']->am->getAsset($field_assetid);
							$replacement = $field->getAdditionalKeywordReplacement($this->id, $additional_keyword, NULL);
							$GLOBALS['SQ_SYSTEM']->am->forgetAsset($field);
							unset($field);
						}
					} else {
						$replacement = $this->_tmp['mm_replacements'][$current_context][$metadata_keyword];
					}

				} else if (isset($this->_tmp['mm_replacements'][$current_context][$spaced_metadata_keyword]) === TRUE) {

					// Try it with metadata keyword underscores replaced with spaces
					// (additional keyword never has its spaces removed when
					// sent to the metadata field)
					if ($additional_keyword !== '') {
						$mm = $GLOBALS['SQ_SYSTEM']->getMetadataManager();
						$field_assetid = $mm->getFieldAssetIdFromName($this->id, $spaced_metadata_keyword);
						if ($field_assetid !== FALSE) {
							$field = $GLOBALS['SQ_SYSTEM']->am->getAsset($field_assetid);
							$replacement = $field->getAdditionalKeywordReplacement($this->id, $additional_keyword, NULL);
							$GLOBALS['SQ_SYSTEM']->am->forgetAsset($field);
							unset($field);
						}
					} else {
						$replacement = $this->_tmp['mm_replacements'][$current_context][$spaced_metadata_keyword];
					}

				}//end if

				// Do we have a replacement?
				if ($replacement !== NULL) break;

				// There are no underscores to break apart, so we've run out of
				// possibilities
				$underscore_pos = strrpos($metadata_keyword, '_');
				if ($underscore_pos === FALSE) break;

				if (strlen($additional_keyword) > 0) {
					$additional_keyword = '_'.$additional_keyword;
				}

				$additional_keyword = substr($metadata_keyword, $underscore_pos + 1).$additional_keyword;
				$metadata_keyword = substr($metadata_keyword, 0, $underscore_pos);
			}

			// If no replacement yet, then perhaps its a related metadata field keyword
			if ($replacement === NULL) {
				$metadata_keyword = substr($keyword, 15);
				// Related metadata fields names, if any, needs to be sorted out before doing the replacement
				$related_assetids = $this->_tmp['mm_replacements'][$current_context];
				array_sort_by_length($related_assetids, FALSE, FALSE);

				foreach($related_assetids as $field_name => $related_assetid) {
					if (strpos($metadata_keyword, $field_name.'_') !== FALSE) {
						$asset_keyword = substr($metadata_keyword, strlen($field_name)+1, strlen($metadata_keyword) - strlen($field_name));
						if ($GLOBALS['SQ_SYSTEM']->am->getAssetInfo($related_assetid)) {
							$related_asset = $GLOBALS['SQ_SYSTEM']->am->getAsset($related_assetid);
							$replacement = $related_asset->getKeywordReplacement($asset_keyword);
							$this->_tmp['mm_replacements'][$current_context][$metadata_keyword] = $replacement;
						}
						break;
					}//end if
				}//end foreach
			}//end if

			// No replacement? Oh well, back to square one
			// EDIT : not sure why the variable needs to be unset here
			// looking at the fact that we are running apply_keyword_modifier
			// on $replacement further down and then returning $replacement var
			//if ($replacement === NULL) {
			//	unset($replacement);
			//}

		} else if (0 === strpos($keyword, 'asset_charset')) {
			$replacement = ($this->charset) ? $this->charset : SQ_CONF_DEFAULT_CHARACTER_SET;

		} else if (0 === strpos($keyword, 'asset_languages')) {
			$replacement = ($this->languages) ? $this->languages : SQ_CONF_DEFAULT_FRONTEND_LANGUAGE;

		} else if (0 === strpos($keyword, 'asset_summary')) {
			$temp_assetid = substr($keyword, 14);
			$temp_assetid = trim($temp_assetid);
			$temp_asset_info = $GLOBALS['SQ_SYSTEM']->am->getAssetInfo($temp_assetid);
			if (!empty($temp_asset_info)) {
				$temp_asset = $GLOBALS['SQ_SYSTEM']->am->getAsset($temp_assetid);
				if (method_exists($temp_asset, 'getAssetSummary')) {
					$replacement = $temp_asset->getAssetSummary();
				}
				$GLOBALS['SQ_SYSTEM']->am->forgetAsset($temp_asset);
			}
		} else if (0 === strpos($keyword, 'asset_paint_layout_id')) {
			$replacement = $GLOBALS['SQ_SYSTEM']->getGlobalDefine('SQ_PAINT_LAYOUT_ID', 0);
			if (empty($replacement)) {
				$url = strip_url(current_url(FALSE, TRUE));
				$layout_name = $this->getCurrentPaintLayoutName();
				$replacement = $GLOBALS['SQ_SYSTEM']->am->getValueFromURL($url, $layout_name);

				// if no layout was found, try and get the default frontend
				// layout if we were not already trying to get it
				if (empty($replacement) && $layout_name != 'paint_layout::system::frontend') {
					$replacement = $GLOBALS['SQ_SYSTEM']->am->getValueFromURL($url, 'paint_layout::system::frontend');
				}
			}
		} else if (0 === strpos($keyword, 'asset_workflow_')) {
			$wfm = $GLOBALS['SQ_SYSTEM']->getWorkflowManager();

			if (!isset($this->_tmp['wfm_schema_workflows'])) {
				$schema_workflows = $wfm->getSchemaWorkflows($this->id);
				$this->_tmp['wfm_schema_workflows'] = $schema_workflows;
			} else {
				$schema_workflows = $this->_tmp['wfm_schema_workflows'];
			}

			if (0 === strpos($keyword, 'asset_workflow_current_')){
				$workflow_keyword = substr($keyword, strlen('asset_workflow_current_'));
				$workflow_keyword_parts = explode(':', $workflow_keyword);

				$schema_ids = Array();
				$wf_replacements = Array();

				if (isset($workflow_keyword_parts[1])) {
					$workflow_keyword = $workflow_keyword_parts[0];
					$schema_ids[] = $workflow_keyword_parts[1];
				} else {
					$schema_ids = array_keys($schema_workflows);
				}

				foreach($schema_ids as $schema_id) {
					//filter out non-running schemas
					if (empty($schema_workflows[$schema_id])) continue;

					if (0 === strpos($workflow_keyword, 'step_')){
						//filter out schemas with no steps/no current step(complete)
						if (empty($schema_workflows[$schema_id]['steps']) || empty($schema_workflows[$schema_id]['current_step'])) continue;

						$current_step = $wfm->getCurrentStep($schema_workflows[$schema_id]);
						switch ($workflow_keyword) {
							case 'step_name':
								$wf_replacements[] = $current_step['step_name'];
								break;
							case 'step_start_time':
								$wf_replacements[] = date('d-m-Y H:i:s', $current_step['started']);
								break;
							case 'step_expiry_time':
								$wf_replacements[] = (!empty($current_step['expiry_time'])) ? date('d-m-Y H:i:s', $current_step['started'] + $current_step['expiry_time']) : '';
								break;
							case 'step_conditions':
								$asset_names = Array();
								foreach (array_keys($current_step['conditions']) as $assetid){
									$asset = $GLOBALS['SQ_SYSTEM']->am->getAsset($assetid);
									if ($asset instanceof User) {
										$asset_names[] = $asset->_getName();
									} else {
										$asset_names[] = $asset->name;
									}
									$GLOBALS['SQ_SYSTEM']->am->forgetAsset($asset);
									unset($asset);
								}
								$wf_replacements[] = implode(', ', $asset_names);
								break;
							case 'step_condition_ids':
								$wf_replacements[] = implode(', ', array_keys($current_step['conditions']));
								break;
						}
					} else {
						switch ($workflow_keyword) {
						    case 'start_time':
						        $wf_replacements[] = date('d-m-Y H:i:s', $schema_workflows[$schema_id]['started']);
						        break;
						    case 'stream_name':
						         $wf_replacements[] = $schema_workflows[$schema_id]['stream_name'];
						        break;
						}
					}
				}

				$replacement = implode(', ', $wf_replacements);
				if (empty($replacement)) return NULL;

			} else 	{
				$workflow_keyword = substr($keyword, strlen('asset_workflow_'));
				switch ($workflow_keyword) {
					case 'applied_schemas':
						$replacement = implode(', ', array_keys($schema_workflows));
						break;
					default:
						return $replacement;
				}
			}

		} else if (0 === strpos($keyword, 'asset_frontend_metadata')) {
			// Returns the asset's frontend metadata content
			$contextid = $GLOBALS['SQ_SYSTEM']->getContextId();
			$metadata_default_name = 'metadata.php';
			$metadata_basename = ($contextid === 0) ? 'metadata.php' : 'metadata.'.$contextid.'.php';

			ob_start();
				if (file_exists($this->data_path.'/'.$metadata_basename)) {
					require $this->data_path.'/'.$metadata_basename;
				} else if (file_exists($this->data_path.'/'.$metadata_default_name)) {
					require $this->data_path.'/'.$metadata_default_name;
				}
			$replacement = ob_get_clean();

		} else if (strpos($keyword, 'asset_data_source_csv_content') !== FALSE) {
			$row_col = explode('-', str_replace('asset_data_source_csv_content_', '', $keyword));
			$replacement = $this->getDataSourceCsvContentKeywordReplacement($keyword, $row_col);
		} else {
			$mm = $GLOBALS['SQ_SYSTEM']->getMetadataManager();
			$mm_keyword = $mm->generateKeywordReplacements($this, Array($keyword), FALSE);
			if (isset($mm_keyword[$keyword])) {
				$replacement = $mm_keyword[$keyword];
			} else {
				// If we got here, we couldn't do anything with the keyword.
				// If this is because of a modifier on a keyword that's
				// supposed to be dealt with higher up the inheritance chain..
				if (count($modifiers) > 1) {
					// Go back up the chain trying to call with just the keyword part.
					$replacement = $this->getKeywordReplacement($keyword);
				} else {
					// replace at higher level
					return "%$full_keyword%";
				}//end if
			}
		}

		if (!is_null($replacement) && trim($replacement, '%') != $keyword) {
			apply_keyword_modifiers($replacement, $modifiers, Array('assetid' => $this->id));
		}

		return $replacement;

	}//end _getKeywordReplacement()


	/**
	* Gets replacements for asset_web_path keyword this asset
	* If the asset has multiple webpath, shortest one (alphabetically) is returned
	*
	* @return null|string
	* @access public
	*/
	public function getAssetWebPathKeywordReplacement()
	{
		$webpaths = $this->getWebPaths();

		if (empty($webpaths)) {
			return NULL;
		}

		$shortest_wp = $webpaths[0];

		foreach($webpaths as $webpath) {

			$wp_len = strlen($webpath);
			$shortest_wp_len = strlen($shortest_wp);

			if ($wp_len <= $shortest_wp_len) {
				$shortest_wp = ($wp_len == $shortest_wp_len) ?  min($shortest_wp, $webpath) : $webpath;
			}
		}

		return $shortest_wp;

	}//end getAssetWebPathKeywordReplacement()


	/**
	* Gets replacements for keywords that are specific to this asset for LIMBO
	*
	* @param array  $keywords   the keywords to get replacements for
	*
	* @return array
	* @access public
	*/
	public function getCustomKeywordReplacements($keywords=Array(), $invoke_backend = FALSE)
	{
		return Array();

	}//end getCustomKeywordReplacements()


	/**
	* Processes anything that was previously printed for a custom keyword for LIMBO
	*
	* An example of this might be a keyword replacement for a bodycopy. When submitted, we need to process
	* The bodycopy here
	*
	* @param array	$keywords	the keywords that were originally printed
	*
	* @return boolean
	* @access public
	*/
	public function processCustomKeywords($keywords=Array())
	{
		return TRUE;

	}//end processCustomKeywords()


	/**
	* Returns the editable content of this asset
	*
	* @return string
	* @access public
	*/
	public function getContent()
	{
		return '';

	}//end getContent()


	/**
	* Sets the editable content of this asset
	*
	* @return boolean
	* @access public
	*/
	public function setContent()
	{
		return FALSE;

	}//end setContent()


	/**
	* Returns the most recent 'last modified time' among all component parts
	*
	* used in conjunction with printFrontend to get a list of loaded asset ids
	* see  _sendHTTPHeaders() in mysource.inc for more information
	*
	* @param array	$assetids	id of all assets that get loaded during printFrontend
	*
	* @return int
	* @access public
	*/
	public function getEffectiveLastUpdatedTime($assetids)
	{
		$db = MatrixDAL::getDb();

		$assetids[] = $this->id; // add self

		// do not include shadow assets
		foreach ($assetids as $key => $assetid) {
			$id_parts = explode(':', $assetid);
			if (isset($id_parts[1])) {
				unset($assetids[$key]);
			} else {
				$assetids[$key] = MatrixDAL::quote((string) $assetid);
			}
		}

		// if only shadow assets are loaded, ignore
		if (empty($assetids)) return 0;

		// break up the assets into chunks of 1000 so that oracle does not complain
		$in_clauses = Array();
		foreach (array_chunk($assetids, 999) as $chunk) {
			$in_clauses[] = ' (assetid IN ('.implode(', ', $chunk).'))';
		}
		$where = '('.implode(' OR ', $in_clauses).')';

		try {
			// get all updated time from DB
			$sql = 'SELECT MAX(updated)
					FROM '.SQ_TABLE_RUNNING_PREFIX.'ast
					WHERE '.$where;
			$most_recent = MatrixDAL::executeSqlOne($sql);
		} catch (Exception $e) {
			throw new Exception('Can not get updated time for asset due to the following database error:'.$e->getMessage());
		}//end try catch

		return (!is_null($most_recent)) ? strtotime($most_recent) : 0;

	}//end getEffectiveLastUpdatedTime()


//--        PERMISSIONS/ACCESS        --//


	/**
	* Returns TRUE if at least one of the passed assetids has read access to this asset
	*
	* @param array	$assetids	An array of user_group and/or user ids to check for read access
	*							If the array is empty, the current user will be checked
	*
	* @return boolean
	* @access public
	*/
	public function readAccess(Array $assetids=Array())
	{
		if (!$this->id) return TRUE;
		if (!$GLOBALS['SQ_SYSTEM']->runLevelEnables(SQ_SECURITY_PERMISSIONS)) {
			return TRUE;
		}

		// if asset is not live we really need to check for write access
		if ($this->status < SQ_STATUS_LIVE) {
			// if we are in rollback view mode, bypass extra write access checks or we
			// we always be returning FALSE here (writeAccess returns FALSE)
			if (SQ_ROLLBACK_VIEW) {
				return $this->_checkPermissionAccess(SQ_PERMISSION_WRITE, $assetids, FALSE);
			}
			return $this->writeAccess('', $assetids, FALSE);
		}

		if ($this->_checkPermissionAccess(SQ_PERMISSION_READ, $assetids)){
			return TRUE;
		} else if ($this->status & SQ_SC_STATUS_PENDING) {
			// if the asset is in the middle of workflow and the current user
			// can approve the asset, we really need to give them read access to it
			$wfm = $GLOBALS['SQ_SYSTEM']->getWorkflowManager();
			$publishers = $wfm->whoCanPublish($this->id);
			if (!empty($publishers) && in_array($GLOBALS['SQ_SYSTEM']->currentUserId(), $publishers)) {
				return TRUE;
			}
		}

		return FALSE;

	}//end readAccess()


	/**
	* Returns TRUE if at least one of the passed assetids has write access to this asset
	*
	* The extra parameter, $only_workflow, is currently only be used in
	* determining read access for an asset before it goes live.
	*
	* @param string		$lock_type		the type of write access required - if blank returns a plain check against
	*									permissions set for this asset, otherwise returns effective access based
	*									on locking and status
	* @param array		$assetids		An array of user_group and/or user ids to check for write access
	*									If the array is empty, the current user will be checked
	* @param boolean	$only_workflow	If TRUE (default), users with write access normally will be denied write access
	*									if outside the current step of the workflow. If FALSE, both those in the workflow
	*									and those with normal write access will be granted write access.
	*
	* @return boolean
	* @access public
	*/
	public function writeAccess($lock_type='', Array $assetids=Array(), $only_workflow=TRUE)
	{
		if (SQ_ROLLBACK_VIEW) return FALSE;

		if (!$this->id) return TRUE;
		if (!$GLOBALS['SQ_SYSTEM']->runLevelEnables(SQ_SECURITY_PERMISSIONS)) {
			return TRUE;
		}

		if ($this->status == SQ_STATUS_LIVE) {
			return $this->liveEditAccess($lock_type);
		}
		return $this->checkAccess(SQ_PERMISSION_WRITE, $lock_type, $assetids, $only_workflow);

	}//end writeAccess()


	/**
	* Returns TRUE if at least one of the passed assetids has admin access to this asset
	*
	* @param string	$lock_type	the type of write access required - if blank returns a plain check against
	*							permissions set for this asset, otherwise returns effective access based on locking
	*							and status
	* @param array	$assetids	an array of user_group and/or user ids to check for write access
	*							If the array is empty, the current user will be checked
	*
	* @return boolean
	* @access public
	*/
	public function adminAccess($lock_type='', Array $assetids=Array())
	{
		if (SQ_ROLLBACK_VIEW) return FALSE;

		if (!$this->id || !empty($this->_tmp[__CLASS__.'_in_create_cascading'])) {
			return TRUE;
		}
		if (!$GLOBALS['SQ_SYSTEM']->runLevelEnables(SQ_SECURITY_PERMISSIONS)) {
			return TRUE;
		}

		$access = $this->checkAccess(SQ_PERMISSION_ADMIN, $lock_type, $assetids);
		if ($access && $this->status == SQ_STATUS_LIVE) {
			return $this->liveEditAccess($lock_type);
		}
		return $access;

	}//end adminAccess()


	/**
	* Determine if the current user is allowed into this asset's backend interface
	*
	* @return boolean
	* @access public
	*/
	public function backendAccess()
	{
		return $GLOBALS['SQ_SYSTEM']->user->canAccessBackend();

	}//end backendAccess()


	/**
	* Returns TRUE if the current user can live edit this asset
	* Live edit means the asset is currently SQ_STATUS_LIVE but this
	* user can edit the asset without going through a safe edit
	* NOTE: will return FALSE if asset is not currently live
	*
	* @param string	$lock_type	the type of lock
	*
	* @return boolean
	* @access public
	* @see canliveEdit()
	*/
	public function liveEditAccess($lock_type)
	{
		if (empty($lock_type) || $this->canLiveEdit($lock_type)) {
			return $this->checkAccess(SQ_PERMISSION_WRITE, $lock_type);
		}
		return FALSE;

	}//end liveEditAccess()


	/**
	* Returns TRUE if the current user could live edit this asset if they had the lock
	*
	* Live edit means the asset is currently SQ_STATUS_LIVE but this
	* user can edit the asset without going through a safe edit
	* NOTE: will return FALSE if asset is not currently live
	*
	* @param int	$lock_type	the type of lock
	*
	* @return boolean
	* @access public
	*/
	public function canliveEdit($lock_type)
	{
		if (SQ_ROLLBACK_VIEW) return FALSE;
		if (!$this->id) return TRUE;
		if (!($this->status & SQ_SC_STATUS_LIVE_EDIT)) {
			return TRUE;
		}

		if (!empty($lock_type)) {
			// make sure user can't live edit attributes and metadata if workflow is used.
			if ($lock_type !== 'attributes' && $lock_type !== 'metadata') {
				$locks = $GLOBALS['SQ_SYSTEM']->am->getLockInfo($this->id, $lock_type, TRUE);
				// if we are getting a lock type that cant be safe edited (eg permissions), you can live edit
				if (!in_array('attributes', array_keys($locks)) && !in_array('metadata', array_keys($locks))) {
					return TRUE;
				}
			}
			
		}

		$wfm = $GLOBALS['SQ_SYSTEM']->getWorkflowManager();
		$schemas = $wfm->getSchemas($this->id, TRUE);

		if (empty($schemas)) return TRUE;

		// if there is workflow defined for this asset, the current user
		// must be the only concerned user to be able to live edit
		$wf_complete = $wfm->testPublish($this->id, $GLOBALS['SQ_SYSTEM']->currentUserId());
		if ($wf_complete) return TRUE;

		return FALSE;

	}//end canliveEdit()


	/**
	* Returns TRUE if this asset is effectivly readable by everyone
	*
	* @return boolean
	* @access public
	*/
	public function effectiveUnrestricted()
	{
		// This is not readAccess() because if this is FALSE we don't refer to the writeAccess() setting
		$public_userid = $GLOBALS['SQ_SYSTEM']->am->getSystemAssetid('public_user');
		return ($this->status >= SQ_STATUS_LIVE && $this->_checkPermissionAccess(SQ_PERMISSION_READ, Array($public_userid)));

	}//end effectiveUnrestricted()


	/**
	* Returns TRUE if at least one of the passed assetids has the passed permission
	*
	* Can perform effective access check based on current status, lock, workflow, and metadata
	* NOTE: Only for write and above access
	*
	* @param int		$perm			the permission to check access for
	* @param string		$lock_type		the type of write access required - if blank returns a plain check against
	*									permissions set for this asset, otherwise returns effective access based on locking
	*									and status
	* @param array		$assetids		An array of user_group and/or user ids to check for permission
	*									If the array is empty, the current user will be checked
	* @param boolean	$only_workflow	check only workflow
	*
	* @return boolean
	* @access public
	*/
	public function checkAccess($perm, $lock_type, Array $assetids=Array(), $only_workflow=TRUE)
	{
		if ($perm == SQ_PERMISSION_READ) {
			trigger_localised_error('SYS0272', E_USER_ERROR, __CLASS__, __FUNCTION__);
		}
		if ($lock_type == '') {
			return $this->_checkPermissionAccess($perm, $assetids, $only_workflow);

		} else {
			// effective access is only valid for the current user
			if (!empty($assetids)) return FALSE;

			// don't check locks if not required
			if ($GLOBALS['SQ_SYSTEM']->runLevelEnables(SQ_SECURITY_LOCKING)) {
			    $locks = $GLOBALS['SQ_SYSTEM']->am->getLockInfo($this->id, $lock_type, TRUE);
			    // if there is no lock or
			    if (empty($locks)) return FALSE;

			    foreach ($locks as $lock) {
				    // if any one of these locks is not owned by the current user they can't edit
				    if (empty($lock) || $lock['userid'] != $GLOBALS['SQ_SYSTEM']->currentUserId()) {
					    return FALSE;
				    }
			    }
			}

			// If we are purging trash, don't bother about statuses
			// Links are not part of workflow either, so don't bother checking
			// on statuses if we are checking links access
			if (!($GLOBALS['SQ_PURGING_TRASH'] || $lock_type == 'links')) {
				if (!$this->accessEffective()) return FALSE;
			}

			// otherwise they can edit if they have permission access
			return $this->_checkPermissionAccess($perm, Array(), $only_workflow);

		}//end else

	}//end checkAccess()


	/**
	* Returns true if permissions on this asset equal effective access
	*
	* NB this function doesn't actually check permissions, but indicates
	* whether having write or admin permissions actually allows you to edit
	* the asset (which it doesn't if the status is all-approved or archived)
	*
	* @return boolean
	* @access public
	*/
	public function accessEffective()
	{
		// if we are in a state of total approval no editing allowed
		if ($this->status & SQ_SC_STATUS_ALL_APPROVED) {
			return FALSE;
		}

		// if the asset is archived no editing allowed
		if ($this->status & SQ_STATUS_ARCHIVED) {
			return FALSE;
		}
		return TRUE;

	}//end accessEffective()


	/**
	* Returns TRUE if at least one of the passed assetids has the passed permission
	* Does not check current status of lock
	*
	* @param int		$perm			the permission to check access for
	* @param array		$assetids		An array of user_group and/or user ids to check for permission
	*									If the array is empty, the current user will be checked
	* @param boolean	$only_workflow	check only workflow
	*
	* @return boolean
	* @access public
	*/
	protected function _checkPermissionAccess($perm, $assetids=Array(), $only_workflow=TRUE)
	{
		if (empty($assetids)) {
			if (!$GLOBALS['SQ_SYSTEM']->userPublic()) {
				// if we are the root user, we can do anything
				if ($GLOBALS['SQ_SYSTEM']->userRoot()) return TRUE;

				// if we are a system administrator, we can do anything
				// unless an asset overwrites this function
				if ($GLOBALS['SQ_SYSTEM']->userSystemAdmin()) {
					return TRUE;
				}
			}

			// get the current user and groups we are not restricted from
			$user = $GLOBALS['SQ_SYSTEM']->am->getAsset($GLOBALS['SQ_SYSTEM']->currentUserId());
			$assetids = $user->getUserGroups();
			$GLOBALS['SQ_SYSTEM']->am->forgetAsset($user);

			$assetids[] = $GLOBALS['SQ_SYSTEM']->currentUserId();
		}

		if (!$GLOBALS['SQ_PURGING_TRASH']) {
			// if we are in workflow no editing allowed
			// except by those users who can currently approve the asset
			if ($this->status & SQ_SC_STATUS_PENDING) {
				if ($perm == SQ_PERMISSION_WRITE) {
					$wfm = $GLOBALS['SQ_SYSTEM']->getWorkflowManager();
					$publishers = $wfm->whoCanPublish($this->id);
					if (in_array($GLOBALS['SQ_SYSTEM']->currentUserId(), $publishers)) {
						return TRUE;
					}

					// Those with admin permissions are now allowed to have a
					// crack at write permission throughout the workflow
					if ($this->_checkPermissionAccess(SQ_PERMISSION_ADMIN, $assetids)) {
						return TRUE;
					}

					if ($only_workflow) return FALSE;
				}
			}
		}

		$grants  = $GLOBALS['SQ_SYSTEM']->am->getPermission($this->id, $perm, TRUE, TRUE, FALSE, FALSE, FALSE);
		$revokes = $GLOBALS['SQ_SYSTEM']->am->getPermission($this->id, $perm, FALSE, FALSE, FALSE, FALSE, FALSE);

		// has their access been revoked at all?
		if (!empty($revokes)) {
			$revoked = array_intersect($assetids, $revokes);
			if (!empty($revoked)) return FALSE;
		}

		// Add the public user to the mix, because if the public can see it then everyone should see it.
		// we add it after the revoke check because we otherwise if the publc is revoked then everyone bar a sysadmin or Root
		// will be blocked
		$assetids[] = $GLOBALS['SQ_SYSTEM']->am->getSystemAssetid('public_user');
		// have they been granted access?
		$common = array_intersect($assetids, $grants);
		return (!empty($common));

	}//end _checkPermissionAccess()


	/**
	* Called when the any permission is updated for this asset
	*
	* It returns TRUE if everything is OK.
	*
	* @return boolean
	* @access public
	*/
	public function permissionsUpdated()
	{
		return TRUE;

	}//end permissionsUpdated()


//--        PAINTING        --//


	/**
	* Prints out the Frontend for this asset
	* it's up to the kids to override
	*
	* @return void
	* @access public
	*/
	public function printFrontend()
	{
		$this->printFrontendAsset($this);

	}//end printFrontend()


	/**
	* Paint the passed asset on the frontend
	* Override printFrontend() if you wish to pass a different asset to print
	*
	* @param Asset	&$asset	the asset to print in frontend
	* @param mixed	$design	the design to print the asset with
	*
	* @return void
	* @access public
	*/
	public function printFrontendAsset(Asset $asset, $design=NULL)
	{
		if (!is_null($design)) {
			assert_is_a($design, 'design');
		}

		if (!$asset->readAccess()) {
			$GLOBALS['SQ_SYSTEM']->paintLogin(translate('login'), translate('cannot_access_asset', $asset->name));
			return;
		}

		$current_protocol = current_protocol();
		$valid_protocols = $this->getValidProtocols();
		if (!empty($valid_protocols) && !in_array($current_protocol, $valid_protocols)) {
			$new_url = (($current_protocol == 'http') ? 'https' : 'http').'://'.current_url(FALSE);
			do_redirect($new_url); // exits
		}

		if (!headers_sent()) {
			if ($asset->charset) {
				header('Content-type: text/html; charset='.$asset->charset);
			} else {
				header('Content-type: text/html; charset='.SQ_CONF_DEFAULT_CHARACTER_SET);
			}

			if ($asset->languages) {
				header('Content-language: '.$asset->languages);
			}
		}

		$original_asset = NULL;
		if (is_null($GLOBALS['SQ_SYSTEM']->frontend_asset) || $asset->id != $GLOBALS['SQ_SYSTEM']->frontend_asset->id) {
			$original_asset = $GLOBALS['SQ_SYSTEM']->frontend_asset;
			$GLOBALS['SQ_SYSTEM']->frontend_asset = &$asset;
		}

		$db = MatrixDAL::getDb();
		$url = strip_url(current_url(FALSE, TRUE));

		// No design has been passed in, so we need to find one
		if (is_null($design)) {
			$design_name = $this->getCurrentDesignName();
			$result = $GLOBALS['SQ_SYSTEM']->am->getDesignFromURL($url, $design_name);

			// if no design was found, try and get the default frontend
			// design if we were not already trying to get it
			if (empty($result) && $design_name != 'design::system::frontend') {
				$result = $GLOBALS['SQ_SYSTEM']->am->getDesignFromURL($url, 'design::system::frontend');
			}

			// if no design was found, try using the asset's URL from the DB
			if (empty($result)) {
				$url = strip_url($asset->getUrl(), TRUE);
				$result = $GLOBALS['SQ_SYSTEM']->am->getDesignFromURL($url, $design_name);
				if (empty($result) && $design_name != 'design::system::frontend') {
					$result = $GLOBALS['SQ_SYSTEM']->am->getDesignFromURL($url, 'design::system::frontend');
				}
			}
		} else {
			$result = TRUE;
		}

		if (!is_null($design) || $result) {
			if (is_null($design)) {
				// we have found the design to use
				$design = $GLOBALS['SQ_SYSTEM']->am->getAsset($result['designid'], $result['type_code']);
			}
			$design->paint($asset);
		} else {
			// we can't find a design, oh well let's just print out our body
			$asset->printBody();
		}

		if (!is_null($original_asset)) {
			$GLOBALS['SQ_SYSTEM']->frontend_asset = &$original_asset;
		}

	}//end printFrontendAsset()


	/**
	* Get the valid protocols for this asset for the current URL
	*
	* @return boolean
	* @access private
	*/
	public function getValidProtocols()
	{
		if (!isset($this->_tmp['valid_protocols'])) {
			$protocol = current_protocol();
			$db = MatrixDAL::getDb();
			try {
				$bind_vars	= Array (
								'assetid'	=> $this->id,
								'url'		=> strip_url(current_url(FALSE)),
							  );
				$result = MatrixDAL::executeAll('core', 'getValidProtocols', $bind_vars);
				if (isset($result[0])) {
					$res = $result[0];
				} else
					$res = $result;
			} catch (Exception $e) {
				throw new Exception('Unable to get valid protocols for asset: '.$this->id.' with this URL: '.strip_url(current_url(FALSE)).' due to database error: '.$e->getMessage());
			}

			$this->_tmp['valid_protocols'] = Array();
			if (is_array($res)) {
				foreach ($res as $protocol => $valid) {
					// Force secure
					if (($this->force_secure === '1') && ($protocol == 'http')) {
						continue;
					}
					// Force INsecure
					if (($this->force_secure === '-') && ($protocol == 'https')) {
						continue;
					}
					if ($valid) {
						$this->_tmp['valid_protocols'][] = $protocol;
					}
				}
			}
		}
		return $this->_tmp['valid_protocols'];

	}//end getValidProtocols()


	/**
	* Called by the design to print extra material in the head of the HTML document for this asset
	* it's up to the kids to override
	*
	* @return void
	* @access public
	*/
	public function printHead()
	{

	}//end printHead()


	/**
	* Called by the design to print the body of this asset
	* it's up to the kids to override
	* TODO: empty function
	*
	* @return void
	* @access public
	*/
	public function printBody()
	{

	}//end printBody()


	/**
	* Get the current paint layout code-name from GET or SESSION as appropriate
	*
	* @return string
	* @access protected
	*/
	public function getCurrentPaintLayoutName()
	{
		// Have they set a paint layout name that they want to view ??
		if (!empty($_GET['SQ_PAINT_LAYOUT_NAME'])) {
			$layout_name = 'paint_layout::user::'.$_GET['SQ_PAINT_LAYOUT_NAME'];
		} else if (!empty($_SESSION['SQ_PAINT_LAYOUT_NAME'])) {
			// is it in the session ?
			$layout_name = 'paint_layout::user::'.$_SESSION['SQ_PAINT_LAYOUT_NAME'];
		} else if (!empty($_GET['SQ_DESIGN_NAME'])) {
			// Maybe they have a design name that we can use to match against
			$layout_name = 'paint_layout::user::'.$_GET['SQ_DESIGN_NAME'];
		} else if (!empty($_SESSION['SQ_DESIGN_NAME'])) {
			$layout_name = 'paint_layout::user::'.$_SESSION['SQ_DESIGN_NAME'];
		} else {
			$layout_name = 'paint_layout::system::frontend';
		}
		return $layout_name;

	}//end getCurrentPaintLayoutName()


	/**
	* Get the current design code-name from GET or SESSION as appropriate
	*
	* @return string
	* @access protected
	*/
	public function getCurrentDesignName()
	{
		// Have they set a design name that they want to view ??
		if (!empty($_GET['SQ_DESIGN_NAME'])) {
			$design_name = 'design::user::'.$_GET['SQ_DESIGN_NAME'];
		} else if (!empty($_SESSION['SQ_DESIGN_NAME'])) {
			$design_name = 'design::user::'.$_SESSION['SQ_DESIGN_NAME'];
		} else {
			$design_name = 'design::system::frontend';
		}
		return $design_name;

	}//end getCurrentDesignName()


	/**
	* Called by the design to print the body of this asset, WITH the paint layout applied
	*
	* @param string	$layout_id	the assetid of the paint layout (leave empty to guess it from URL/Global define)
	*
	* @return void
	* @access public
	*/
	public function printBodyWithPaintLayout($layout_id='')
	{
		if (empty($layout_id)) {
			$layout_id = $GLOBALS['SQ_SYSTEM']->getGlobalDefine('SQ_PAINT_LAYOUT_ID', 0);
		}
		if (empty($layout_id)) {
			$url = strip_url(current_url(FALSE, TRUE));
			$layout_name = $this->getCurrentPaintLayoutName();
			$layout_id = $GLOBALS['SQ_SYSTEM']->am->getValueFromURL($url, $layout_name);

			// if no layout was found, try and get the default frontend
			// layout if we were not already trying to get it
			if (empty($layout_id) && $layout_name != 'paint_layout::system::frontend') {
				$layout_id = $GLOBALS['SQ_SYSTEM']->am->getValueFromURL($url, 'paint_layout::system::frontend');
			}
		}

		if ($layout_id) {
			$replacement = $GLOBALS['SQ_SYSTEM']->setGlobalDefine('SQ_PAINT_LAYOUT_ID', $layout_id);
			// we have found the design to use... we might need to fool the system
			// into thinking we are printing the frontend of that asset
			$switched_frontend = FALSE;

			// Firstly check if the "frontend_asset" is set, and then try to extract the id from it
			if (isset($GLOBALS['SQ_SYSTEM']->frontend_asset) && $GLOBALS['SQ_SYSTEM']->frontend_asset->id !== $this->id) {
				$current_frontend_asset = &$GLOBALS['SQ_SYSTEM']->frontend_asset;
				$GLOBALS['SQ_SYSTEM']->frontend_asset = &$this;
				$switched_frontend = TRUE;
			}

			$layout = $GLOBALS['SQ_SYSTEM']->am->getAsset($layout_id);
			$layout->paint($this);

			if ($switched_frontend) {
				$GLOBALS['SQ_SYSTEM']->frontend_asset = &$current_frontend_asset;
			}

			if (!is_null($replacement)) {
				$GLOBALS['SQ_SYSTEM']->setGlobalDefine('SQ_PAINT_LAYOUT_ID', $replacement);
			} else {
				$GLOBALS['SQ_SYSTEM']->unsetGlobalDefine('SQ_PAINT_LAYOUT_ID');
			}
		} else {
			// we can't find a layout, oh well let's just print out our body
			$this->printBody();
		}

	}//end printBodyWithPaintLayout()


	/**
	* Initialise the limbo (simple-edit) interface by painting to the limbo outputter
	*
	* This function should be overridden if assets want to expose another asset's interface
	* instead of their own.  By default, the normal backend is printed for the default screen.
	*
	* @return void
	* @access public
	* @see printLimbo
	*/
	public function initLimbo()
	{
		$o = $GLOBALS['SQ_SYSTEM']->backend->out;
		$o->addHiddenField('asset_action', 'limbo');
		$o->openRaw();
		$this->paintBackend($o);
		$o->closeRaw();

	}//end initLimbo()


	/**
	* Print the body of the limbo (simple-edit) interface
	*
	* Asset types should only override this if they want to bypass or supplement the output
	* of the backend outputter.  If the backend outputter is not painted then initLimbo should
	* be overridden too to avoid wasting effort populating the backend outputter.
	*
	* @return void
	* @access public
	* @see initLimbo
	*/
	public function printLimbo()
	{
		$o = $GLOBALS['SQ_SYSTEM']->backend->out;
		if (is_null($o)) {
			trigger_localised_error('SYS0245', E_USER_ERROR, current_url());
			return;
		}
		$o->paint();

	}//end printLimbo()


	/**
	* Prints out the backend interface for the asset
	*
	* @param Backend_Outputter	&$o	Backend_Outputter object
	*
	* @return void
	* @access public
	*/
	public function paintBackend(Backend_Outputter $o)
	{
		$o->addHiddenField('asset_action', (($this->id) ? 'edit' : 'create'));
		if ($this->charset) $o->setCharset($this->charset);
		$ei = $this->getEI();
		if ($this->id !== 0) {
			$owner = $GLOBALS['SQ_SYSTEM']->am->getAsset($this->id);
		} else {
			$owner = $this;
		}
		$ei->paint($owner, $o, empty($this->id));

	}//end paintBackend()


	/**
	* Generate a prefix to be used by paint() and process()
	*
	* @return string
	* @access public
	*/
	public function getPrefix()
	{
		return str_replace(':', '_', str_replace(' ', '_', $this->type().'_'.$this->id));

	}//end getPrefix()


	/**
	* Processes a backend submission from this asset, returns TRUE if all OK
	* Put's the array of processed attributes into $this->_tmp['process_attributes']
	*
	* @param Backend_Outputter	$o		Backend_Outputter object
	* @param array				&$link	information used to create the initial link
	*
	* @return boolean
	* @access public
	*/
	public function processBackend(Backend_Outputter $o, Array &$link)
	{
		$ei = $this->getEI();

		switch ($_POST['asset_action']) {
			case 'create' :
				if ($ei->process($this, $o, TRUE)) {
					return (bool) $this->create($link);
				} else {
					return FALSE;
				}
			break;

			default :
				if ($this->id !== 0) {
					$owner = $GLOBALS['SQ_SYSTEM']->am->getAsset($this->id);
				} else {
					$owner = $this;
				}
				return $ei->process($owner, $o, FALSE);
			break;
		}

	}//end processBackend()


	/**
	* Returns a reference to the Asset_Edit_Interface object for this asset type
	*
	* @return object
	* @access public
	*/
	public function getEI()
	{
		if (empty($this->_tmp['ei']) || get_class($this->_tmp['ei']) != 'asset_edit_interface') {
			require_once SQ_INCLUDE_PATH.'/asset_edit_interface.inc';
			$this->_tmp['ei'] = new Asset_Edit_Interface($this->type());
		}
		return $this->_tmp['ei'];

	}//end getEI()


	/**
	* Returns the Edit_Fns object appropriate for whatever asset type we are
	*
	* @return object
	* @access public
	*/
	public function getEditFns()
	{
		if (!isset($this->_tmp['edit_fns'])) {
			$class_name = $this->type().'_edit_fns';
			require_once SQ_SYSTEM_ROOT.'/'.$GLOBALS['SQ_SYSTEM']->am->getTypeInfo($this->type(), 'dir').'/'.$class_name.'.inc';
			$this->_tmp['edit_fns'] = new $class_name();
		}

		return $this->_tmp['edit_fns'];

	}//end getEditFns()


//--        URL's & HREF's        --//


	/**
	* Returns the href for getting to the nearest backend for this asset
	*
	* @param string		$screen			the code name of the screen that you want displayed when the asset is loaded
	* @param boolean	$backend_page	whether to include the 'SQ_BACKEND_PAGE=main' query string
	*									include this bit when we are switching screens in the backend
	*									do not include when generating URL (e.g. for workflow emails)
	*
	* @return string
	* @access public
	*/
	public function getBackendHref($screen='', $backend_page=TRUE)
	{
		$href = $GLOBALS['SQ_SYSTEM']->am->getAssetBackendHref(Array($this->id => $screen), $backend_page);
		return $href[$this->id];

	}//end getBackendHref()


	/**
	* Returns the URL for this asset, closest to the $base_url
	*
	* @param string		$base_url			the url that we are want to match closest to
	* @param boolean	$ignore_rollback	ignore rollback checks in asset manager?
	*										this should only get set by the asset manager itself
	* @param int		$base_contextid		the id of the context that we want the url from
	*
	* @return string
	* @access public
	*/
	public function getURL($base_url=NULL, $ignore_rollback=FALSE, $base_contextid = NULL)
	{
		return $GLOBALS['SQ_SYSTEM']->am->getAssetURL($this->id, $base_url, $ignore_rollback, $base_contextid);

	}//end getURL()


	/**
	* Returns an array of URLs of this asset, including the protocol information
	*
	* @return array
	* @access public
	*/
	public function getURLs()
	{
		return $GLOBALS['SQ_SYSTEM']->am->getURLs($this->id);

	}//end getURLs()


	/**
	* Returns the href for this asset closest to the $base_url, but relative to the current url
	*
	* @param string		$base_url			the url that we are want to match closest to
	* @param boolean	$ignore_rollback	ignore rollback checks in asset manager?
	*										this should only get set by the asset manager itself
	*
	* @return string
	* @access public
	*/
	public function getHref($base_url=NULL, $ignore_rollback=FALSE)
	{
		return $GLOBALS['SQ_SYSTEM']->am->getAssetHref($this->id, $base_url, $ignore_rollback);

	}//end getHref()


	/**
	* Returns the href for this asset closest to the $base_url, but relative to the current url
	*
	* @return string
	* @access public
	*/
	public function getWebDataPath()
	{
		return sq_web_path('data').'/assets/'.$this->type().'/'.get_asset_hash($this->id).'/'.$this->id;

	}//end getWebDataPath()


	/**
	* Returns a list of paths that this asset has
	*
	* @return array
	* @access public
	*/
	public function getWebPaths()
	{
		if (!isset($this->_tmp['paths'])) {
			$db = MatrixDAL::getDb();
			$where = 'assetid = :assetid';
			$where = $GLOBALS['SQ_SYSTEM']->constructRollbackWhereClause($where);
			$sql = 'SELECT path
					FROM '.SQ_TABLE_RUNNING_PREFIX.'ast_path '.$where.'
					ORDER BY sort_order';

			try {
				$query = MatrixDAL::preparePdoQuery($sql);
				MatrixDAL::bindValueToPdo($query, 'assetid', $this->id);
				$result = MatrixDAL::executePdoAssoc($query, 0);
			} catch (Exception $e) {
				throw new Exception('Unable to get web paths for asset "'.$this->name.'" (#'.$this->id.') due to database error: '.$e->getMessage());
			}

			$this->_tmp['paths'] = $result;

		}// end if

		return $this->_tmp['paths'];

	}//end getWebPaths()


	/**
	* Create an initial webpath for an asset
	* This function is typically called by _createAdditional()
	* Returns FALSE only if a web path cannot be saved
	*
	* @param string	$path			an initial web path
	* @param Asset	$parent_asset	asset under which this asset is being created (NULL is none)
	*
	* @return boolean
	* @access protected
	*/
	protected function makeAndSaveInitialWebPath($path, $parent_link=NULL)
	{
		require_once SQ_INCLUDE_PATH.'/general_occasional.inc';
		$valid_paths = make_valid_web_paths(Array($path), $this->id);

		// don't check for path conflicts if we have no parent
		if (!empty($parent_link['asset'])) {
			$valid_paths = $GLOBALS['SQ_SYSTEM']->am->webPathsInUse($parent_link['asset'], $valid_paths, $this->id, TRUE);
		}

		return $this->saveWebPaths($valid_paths);

	}//end makeAndSaveInitialWebPath($path)


	/**
	* Attempts to override all current paths for this asset with those passed in the array
	*
	* @param array		$paths			array of paths to give this asset
	* @param boolean	$auto_add_remaps	whether or not to add remaps for the asset when updating lookups
	*
	* @return boolean
	* @access public
	*/
	public function saveWebPaths($paths, $auto_add_remaps = TRUE)
	{
		$save_paths = Array();

		if (!empty($paths)) {
			$current_paths = $this->getWebPaths();

			// make sure these paths are compliant
			require_once SQ_INCLUDE_PATH.'/general_occasional.inc';
			$save_paths = make_valid_web_paths($paths, $this->id ? $this->id : '');

			// do not run hipo if any of the paths start with __ or is reserved by Matrix
			foreach ($paths as $orig_path) {
				if (preg_match('/^__/', $orig_path) || $orig_path == SQ_CONF_BACKEND_SUFFIX || $orig_path == SQ_CONF_LIMBO_SUFFIX || $orig_path == SQ_CONF_NOCACHE_SUFFIX) {
					return FALSE;
				}
			}

			// if there is no difference in the arrays (including in the sort order - the index),
			// then there is nothing to do
			if (!array_diff_assoc($save_paths, $current_paths) && !array_diff_assoc($current_paths, $save_paths)) {
				return TRUE;
			}

			//// CHECK INSERTED PATHS ////

			// change db connection because in a nested db transaction lookup on db1 may not produce a valid result
			$GLOBALS['SQ_SYSTEM']->changeDatabaseConnection('db2');

			// to make sure that they path name is not in use by any of our siblings
			$inserts = array_diff($save_paths, $current_paths);
			if (!empty($inserts)) {
				// now we need to check that none of our parents have
				// any nav kids that are also using this path name
				$parents = $GLOBALS['SQ_SYSTEM']->am->getLinks($this->id, SQ_SC_LINK_WEB_PATHS, '', NULL, 'minor');
				for ($i = 0; $i < count($parents); $i++) {
					$parent = $GLOBALS['SQ_SYSTEM']->am->getAsset($parents[$i]['majorid'], $parents[$i]['major_type_code']);
					if (is_null($parent)) continue;
					$bad_paths = $GLOBALS['SQ_SYSTEM']->am->webPathsInUse($parent, $inserts, $this->id);
					if (!empty($bad_paths)) {
						trigger_localised_error('SYS0201', E_USER_WARNING, implode('", "', $bad_paths), $parent->name);
						$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();
						return FALSE;
					}
				}
			}// end if inserts
		} else {
			// change db connection because in a nested db transaction lookup on db1 may not produce a valid result
			// We did it if there are paths to add, but if there aren't, we need to switch
			// so the updateLookups() below works in the right connection.
			$GLOBALS['SQ_SYSTEM']->changeDatabaseConnection('db2');
		}

		$GLOBALS['SQ_SYSTEM']->doTransaction('BEGIN');

		$db = MatrixDAL::getDb();

		$sql = 'DELETE FROM
					sq_ast_path
				WHERE
					assetid	= :assetid';

		try {
			$query = MatrixDAL::preparePdoQuery($sql);
			MatrixDAL::bindValueToPdo($query, 'assetid', $this->id);
			MatrixDAL::execPdoQuery($query);
		} catch (Exception $e) {
			throw new Exception('Unable to delete web paths for asset "'.$this->name.'" (#'.$this->id.') due to database error: '.$e->getMessage());
		}

		foreach ($save_paths as $sort_order => $path) {
			$sql = 'INSERT INTO
						sq_ast_path
					(
						path,
						assetid,
						sort_order
					)
					VALUES
					(
						:path,
						:assetid,
						:sort_order
					)';

			try {
				$query = MatrixDAL::preparePdoQuery($sql);
				MatrixDAL::bindValueToPdo($query, 'path',       $path);
				MatrixDAL::bindValueToPdo($query, 'assetid',    $this->id);
				MatrixDAL::bindValueToPdo($query, 'sort_order', $sort_order);
				MatrixDAL::execPdoQuery($query);
			} catch (Exception $e) {
				throw new Exception('Unable to insert web paths for asset "'.$this->name.'" (#'.$this->id.') due to database error: '.$e->getMessage());
			}
		}

		unset($this->_tmp['paths']);
		unset($this->_tmp['lookups']);

		if ($this->updateLookups($auto_add_remaps)) {
			$GLOBALS['SQ_SYSTEM']->doTransaction('COMMIT');
			$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();
			return TRUE;
		} else {
			$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
			$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();
			return FALSE;
		}

	}//end saveWebPaths()


//--        LOOKUPS        --//


	/**
	* Returns a list of lookup information associated with this asset
	*
	* @param string	$field	if only one field is required pass it's name through
	*						(url, http, https, root_urlid)
	*
	* @return array
	* @access public
	*/
	public function getLookups($field='')
	{
		if (!isset($this->_tmp['lookups'])) {
			$db = MatrixDAL::getDb();

			$sql = 'SELECT url, http, https, root_urlid
					FROM '.SQ_TABLE_RUNNING_PREFIX.'ast_lookup ';
			$where = 'assetid = :assetid';
			$where = $GLOBALS['SQ_SYSTEM']->constructRollbackWhereClause($where);

			try {
				$query = MatrixDAL::preparePdoQuery($sql.$where);
				MatrixDAL::bindValueToPdo($query, 'assetid', $this->id);
				$result = MatrixDAL::executePdoAll($query);
			} catch (Exception $e) {
				throw new Exception('Unable to get lookups for asset "'.$this->name.'" (#'.$this->id.') due to database error: '.$e->getMessage());
			}

			$this->_tmp['lookups'] = $result;

		}//end if

		if (!$field) {
			return $this->_tmp['lookups'];
		} else {
			$ret_val = Array();
			foreach ($this->_tmp['lookups'] as $data) {
				$ret_val[] = $data[$field];
			}
			return $ret_val;
		}

	}//end getLookups()


	/**
	* Returns a list of design lookup information associated with this asset
	*
	* @return array
	* @access public
	*/
	public function getDesignLookups()
	{
		if (!isset($this->_tmp['design_lookups'])) {
			$this->_tmp['design_lookups'] = Array();

			$db = MatrixDAL::getDb();

			$sql = 'SELECT l.url, lv.name, lv.value as designid
					FROM '.SQ_TABLE_RUNNING_PREFIX.'ast_lookup l
							INNER JOIN '.SQ_TABLE_RUNNING_PREFIX.'ast_lookup_value lv ON ((l.url LIKE lv.url || \'/%\')
							OR (l.url = lv.url)
							OR (l.url || \'/\' = lv.url))
					';
			$where = 'l.assetid = :assetid
						AND lv.name LIKE '.MatrixDAL::quote('design::%');

			// sort ascending so the override designs overwrite the array index for the normal design in the loop below
			$order_by = ' ORDER BY lv.depth ASC';
			$where = $GLOBALS['SQ_SYSTEM']->constructRollbackWhereClause($where, 'l');
			$where = $GLOBALS['SQ_SYSTEM']->constructRollbackWhereClause($where, 'lv');

			try {
				$query = MatrixDAL::preparePdoQuery($sql.$where.$order_by);
				MatrixDAL::bindValueToPdo($query, 'assetid', $this->id);
				$result = MatrixDAL::executePdoAssoc($query);
			} catch (Exception $e) {
				throw new Exception('Unable to get design lookup information for asset: '.$this->id.' due to database error: '.$e->getMessage());
			}

			foreach ($result as $row) {
				if (!isset($this->_tmp['design_lookups'][$row['url']])) {
					$this->_tmp['design_lookups'][$row['url']] = Array();
				}
				$this->_tmp['design_lookups'][$row['url']][$row['name']] = $row['designid'];
			}

		}//end if

		return $this->_tmp['design_lookups'];

	}//end getDesignLookups()


	/**
	* Returns a list of lookup value information associated with this asset matching the criteria
	*
	* Return value is in the form<br />
	* <PRE>
	* Array(
	*   [url] => Array(
	*   [name] => Array('value' => [value], 'inhd' => [inhd]),
	*   ....
	* )
	* </PRE>
	*
	* @param mixed		$inherited					it takes a mixed argument, boolean or NULL
	* 												NULL = FALSE = return only parent values
	* 												TRUE = return parents and children values
	* @param string		$prefix						a prefix to search for in the name field of ast_lookup_value
	* @param boolean	$like_search				TRUE = LIKE $prefix, FALSE = NOT LIKE $prefix
	* @param boolean	$ignore_override			TRUE = ignore URLs with '/' appended to the end, i.e. URLs with override designs/layouts
	* 												applied
	* 												FALSE = return the URL with all its current design/layout information, i.e. the fact
	* 												that a URL has an override design/layout applied will be transparent
	*
	* @return array
	* @access public
	*/
	public function getLookupValues($inherited=NULL, $prefix='', $like_search=TRUE, $ignore_override=FALSE)
	{
		$db = MatrixDAL::getDb();

		// return normal and regular values in sq_lookup_value corresponding exactly to this asset's URL(s), i.e. the parents
		// called by updateLookups() and by the lookup settings screen
		// 'inhd' column no longer exists, legacy value kept because other classes depend on seeing 'inhd'.
		// select the lookup_value's URL column because it will indicate whether the value is an override
		$sql_default = 'SELECT lv.url, lv.name, lv.value, \'0\' as inhd, lv.depth
								FROM '.SQ_TABLE_RUNNING_PREFIX.'ast_lookup l
								INNER JOIN '.SQ_TABLE_RUNNING_PREFIX.'ast_lookup_value lv ON (l.url = lv.url)
						';

		$include_override = ' OR (l.url || \'/\' = lv.url)';

		$where = ' l.assetid = :assetid';

		// return all rows which are children, i.e. they inherit, override values by definition are not inherited
		// called by lookupValues.inc to paint the lookup settings screen
		$sql_inherited = 'SELECT l.url, lv.name, lv.value, \'1\' as inhd, lv.depth
								FROM '.SQ_TABLE_RUNNING_PREFIX.'ast_lookup l
								INNER JOIN '.SQ_TABLE_RUNNING_PREFIX.'ast_lookup_value lv ON (l.url LIKE lv.url || \'/%\')
						';

		if ($prefix !== '') {
			$where .= ' AND lv.name '.((!$like_search) ? 'NOT' : '').' LIKE :lookup_value';
		}

		$where = $GLOBALS['SQ_SYSTEM']->constructRollbackWhereClause($where, 'l');
		$where = $GLOBALS['SQ_SYSTEM']->constructRollbackWhereClause($where, 'lv');

		$sql_order_by = ' ORDER BY depth ASC ';

		try {
			if (!is_null($inherited) && $inherited) {
				$full_sql = $sql_inherited.$where.' UNION '.$sql_default.$include_override.$where.$sql_order_by;
			}
			else if ($ignore_override) {
				$full_sql = $sql_default.$where;
			}
			else {
				$full_sql = $sql_default.$include_override.$where;
			}

			$query = MatrixDAL::preparePdoQuery($full_sql);
			MatrixDAL::bindValueToPdo($query, 'assetid', $this->id);
			if ($prefix !== '') {
				MatrixDAL::bindValueToPdo($query, 'lookup_value', $prefix.'%');
			}
			$result = MatrixDAL::executePdoAssoc($query);
		} catch (Exception $e) {
			throw new Exception('Unable to get lookup values for asset: '.$this->id.' due to database error: '.$e->getMessage());
		}

		$ret_val = Array();
		foreach ($result as $row) {
			if (!isset($ret_val[$row['url']])) {
				$ret_val[$row['url']] = Array();
			}
			$ret_val[$row['url']][$row['name']] = Array('value' => $row['value'], 'inhd' => $row['inhd']);
		}

		return $ret_val;

	}//end getLookupValues()


	/**
	* Removes a lookup value from this asset and returns the list of values after modification
	*
	* @param string	$layout_name	The name of the layout, can also be in notice link style,
	*								e.g. override::paint_layout::system::frontend
	* @param string	$layout_value	The value that the layout has in the sq_lookup_values table
	*
	* @return mixed array|boolean
	* @access public
	*/
	public function deleteLookupValue($layout_name, $layout_value)
	{
		$lookup_values = $this->getLookupValues(FALSE);

		foreach ($lookup_values as $resource => $data) {
			foreach ($data as $name => $value) {
				if (strstr($layout_name, 'override::')) {
					if (preg_match('/.+\/$/', $resource)) {
						// override layout
						if ((str_replace('override::', '', $layout_name) === $name) && ($layout_value === $value['value'])) {
							unset($lookup_values[$resource][$name]);
						}
					}
				} else {
					if (preg_match('/.+\/{0}$/', $resource)) {
						// normal layout
						if (($layout_name === $name) && ($layout_value === $value['value'])) {
							unset($lookup_values[$resource][$name]);
						}
					}
				}
			}
		}

		$GLOBALS['SQ_SYSTEM']->changeDatabaseConnection('db2');
		$GLOBALS['SQ_SYSTEM']->doTransaction('BEGIN');

		if (!$this->setLookupValues($lookup_values)) {
			$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
			$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();
			return FALSE;
		}

		$GLOBALS['SQ_SYSTEM']->doTransaction('COMMIT');
		$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();

		return $lookup_values;

	}//end deleteLookupValue()


	/**
	* Sets asset based paint layouts which apply to all of this asset's URLs
	*
	* @param array	$layouts	Array of the paint layouts that we wish to apply.
	*							There are 2 types you can apply and both are optional.
	* <pre>
	*							Array(
	*								'paint_layout::system::frontend' => [assetid of paint layout],
	*								'override::paint_layout::system::frontend' => [assetid of paint layout],
	*							)
	* </pre>
	*
	* @return boolean
	* @access public
	*/
	public function setPaintLayouts($layouts)
	{
		if (!is_array($layouts)) return FALSE;
		$GLOBALS['SQ_SYSTEM']->changeDatabaseConnection('db2');
		$GLOBALS['SQ_SYSTEM']->doTransaction('BEGIN');

		$modified = FALSE;
		// work out what links we have currently and create new ones if they do not exist
		foreach ($layouts as $layout_name => $assetid) {
			if (!empty($assetid)) {
				$layout = $GLOBALS['SQ_SYSTEM']->am->getAsset($assetid);
				if (!($layout instanceof Paint_Layout_Page)) return FALSE;

				$create_link = FALSE;
				$link = $GLOBALS['SQ_SYSTEM']->am->getLink($this->id, SQ_LINK_NOTICE, 'paint_layout_page', TRUE, $layout_name);
				if (isset($link['minorid'])) {
					// this asset already has an asset based layout
					if ($link['minorid'] !== $assetid) {
						// different so delete existing link so we can create a new one
						if (!$this->deleteLink($link['linkid'])) {
							trigger_localised_error('CORE0283', E_USER_WARNING, $link['linkid'], $layout_name, $this->id);
							$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
							$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();
							return FALSE;
						}

						// also need to delete any other URLs which use the same value
						// deleting an asset based value means all other URLs which use the same value are also deleted
						if (!$this->deleteLookupValue($layout_name, $link['minorid'])) {
							trigger_localised_error('CORE0284', E_USER_WARNING, $layout_name, $this->id);
							$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
							$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();
							return FALSE;
						}

						$create_link = TRUE;
					}
				} else {
					// no existing link so let's just create one
					$create_link = TRUE;
				}

				if ($create_link) {
					$modified = TRUE;
					if (!$GLOBALS['SQ_SYSTEM']->am->createAssetLink($this, $layout, SQ_LINK_NOTICE, $layout_name)) {
						trigger_localised_error('CORE0285', E_USER_WARNING, $layout->id, $layout_name, $this->id);
						$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
						$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();
						return FALSE;
					}
				}
			}
		}

		if ($modified && !$this->updateLookups()) {
			$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
			$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();
			return FALSE;
		}

		$GLOBALS['SQ_SYSTEM']->doTransaction('COMMIT');
		$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();

		return TRUE;

	}//end setPaintLayouts()


	/**
	 * Starts the update lookup process.
	 *
	 * Clears some _tmp vars,
	 * connects to the right database,
	 * also starts a transaction,
	 * and finally opens the messaging service log.
	 *
	 * This is separated so the updateLookup() process can (possibly) be returned quickly.
	 *
	 * @return array Returns an array of the messaging service handler, and
	 *               the database handler.
	 */
	private function _beginUpdateLookups()
	{
		// get ready to log what happens
		$ms = $GLOBALS['SQ_SYSTEM']->getMessagingService();
		$ms->openLog();

		unset($this->_tmp['lookups']);
		unset($this->_tmp['design_lookups']);
		$GLOBALS['SQ_SYSTEM']->am->clearLookupsCache($this->id);

		$GLOBALS['SQ_SYSTEM']->changeDatabaseConnection('db2');
		$GLOBALS['SQ_SYSTEM']->doTransaction('BEGIN');
		$db = MatrixDAL::getDb();

		return array($ms, $db);
	}


	/**
	 * Finishes the update lookup process.
	 * Commits the transaction started by _beginUpdateLookups(),
	 * broadcasts the LookupsUpdated event and also closes the message log
	 * that was started before.
	 *
	 * This is separated so the updateLookup() process can (possibly) be returned quickly.
	 *
	 * @param object $ms The messaging service from _beginUpdateLookups()
	 * @param object $db The database handler from _beginUpdateLookups()
	 * @param array  $parameter The parameters to pass through to the broadcast event.
	 *
	 * @return void
	 */
	private function _finishUpdateLookups($ms, $db, $parameter)
	{
		$GLOBALS['SQ_SYSTEM']->doTransaction('COMMIT');
		$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();

		// notify anyone interested that lookups have been updated
		$em = $GLOBALS['SQ_SYSTEM']->getEventManager();
		$em->broadcastEvent($this, 'LookupsUpdated', Array('asset_name' => $this->name));
		$GLOBALS['SQ_SYSTEM']->broadcastTriggerEvent('trigger_event_lookups_updated', $this, $parameter);
		$ms->closeLog();
	}


	/**
	* Called to force an update of this assets lookup information
	*
	* @param boolean	$auto_add_remaps	whether to add  remaps for this asset
	*
	* @return boolean
	* @access private
	* @see saveWebPaths()
	*/
	public function updateLookups($auto_add_remaps = TRUE)
	{
		$paths = $this->getWebPaths();

		list($ms, $db) = $this->_beginUpdateLookups();

		// retrieve all existing URLs for this asset
		// include sort order so that we can figure out which one is which
		$sql = 'SELECT l.url, l.http, l.https, u.urlid
				FROM sq_ast_lookup l
					LEFT OUTER JOIN sq_ast_url u ON l.root_urlid = u.urlid
					LEFT OUTER JOIN sq_ast_path p ON l.assetid = p.assetid
				WHERE l.assetid = :assetid
				ORDER BY p.sort_order DESC';

		try {
			$query = MatrixDAL::preparePdoQuery($sql);
			MatrixDAL::bindValueToPdo($query, 'assetid', $this->id);
			$old_urls = MatrixDAL::executePdoAssoc($query);
		} catch (Exception $e) {
			throw new Exception('Unable to get old lookups for asset "'.$this->name.'" (#'.$this->id.') due to database error: '.$e->getMessage());
		}

		// prepare trigger event data
		$this->_tmp['old_urls'] = $old_urls;
		$parameter['old_urls'] = $old_urls;

		/**
		 * Since there are no paths left, we can clear out the
		 * lookup and lookup_value tables.
		 */
		if (empty($paths)) {
			try {
				$bind_vars = Array('assetid' => $this->id);
				$result = MatrixDAL::executeQuery('core', 'deleteLookupValueByAssetid', $bind_vars);

				$bind_vars = Array('assetid' => $this->id);
				$result = MatrixDAL::executeQuery('core', 'deleteLookup', $bind_vars);
			} catch (Exception $e) {
				throw new Exception('Unable to delete lookup information for asset: '.$this->id.' due to database error: '.$e->getMessage());
			}

			$this->_finishUpdateLookups($ms, $db, $parameter);
			return TRUE;
		}

		// get all the lookup information that we have set (i.e. no inherited values), ignoring designs as they are set on every url
		$our_lookup_values = $this->getLookupValues(FALSE, $prefix='design::', FALSE);
		$our_edit_layouts = $this->getLookupValues(FALSE, 'layout::');

		$tmp_old_urls = Array();

		$current_urls = Array();
		$old_web_paths = Array();
		foreach ($old_urls as $url_data) {
			$url = $url_data['url'];
			// Keep the current url
			$current_urls[] = $url;
			// and the override url
			$current_urls[] = $url.'/';

			if ((strpos($url, '__data') === FALSE) && ($url_data['urlid'])) {
				// get the asset lineage and use that as the basis for indexing old URLs
				$url_lineage = $GLOBALS['SQ_SYSTEM']->am->getLineageFromURL(NULL, $url);
				$url_assetids = Array();
				foreach ($url_lineage as $url_lineage_item) {
					$url_assetids[] = $url_lineage_item['assetid'];
				}

				$parent_url = substr($url, 0, strrpos($url, '/'));
				$web_path = substr($url, strrpos($url, '/') + 1);

				$tmp_old_urls[] = Array(
									'url'			=> $url,
									'parent_url'	=> $parent_url,
									'web_path'		=> $web_path,
									'lineage'		=> $url_assetids,
								  );
				$old_web_paths[] = $web_path;
			}
		}
		$old_web_paths = array_unique($old_web_paths);

		// find the differences between the two - from both directions -
		// and reindex them so they both start from zero
		$old_to_new = array_values(array_diff($old_web_paths, $paths));
		$new_to_old = array_values(array_diff($paths, $old_web_paths));

		$web_path_changes = Array();

		if ((count($old_to_new) == 1) && (count($new_to_old) == 1)) {
			$web_path_changes[$old_to_new[0]] = $new_to_old[0];
		} else if ((count($old_to_new) > 1) && (count($new_to_old) > 1)) {
			// if more than one web path has changed...
			// if the number of web paths changing are the same, we should
			for ($i = 0; $i < min(count($old_to_new), count($new_to_old)); $i++) {
				$web_path_changes[$old_to_new[$i]] = $new_to_old[$i];
			}
		}

		if (!empty($current_urls)) {
			// delete rows which have normal values applied
			// and rows which have override values applied
			// We added both sets of rows when looking at the old urls.
			$url_chunks = array_chunk($current_urls, 1000, TRUE);
			foreach ($url_chunks as $url_chunk) {
				try {
					$bind_vars = Array(
									'urls'	=> $url_chunk,
								 );
					MatrixDAL::executeQuery('core', 'deleteLookupValueByUrls', $bind_vars);
				} catch (Exception $e) {
					throw new Exception('Unable to delete URLs with override values from lookup value table due to database error: '.$e->getMessage());
				}
			}//end foreach

			try {
				$bind_vars = Array('assetid' => $this->id);
				$result = MatrixDAL::executeQuery('core', 'deleteLookup', $bind_vars);
			} catch (Exception $e) {
				throw new Exception('Unable to delete lookup information for asset: '.$this->id.' due to database error: '.$e->getMessage());
			}
		}//end if

		$num_paths = count($paths);

		$our_design_links = $GLOBALS['SQ_SYSTEM']->am->getLinks($this->id, SQ_LINK_NOTICE, 'design', FALSE);
		$our_designs = Array();
		foreach ($our_design_links as $link) {
			if (preg_match('/^design::(system|user)::.*$/', $link['value'])) {
				$our_designs[$link['value']] = Array(
												'value'	=> $link['minorid'],
											   );
			}
		}

		// override designs take priority over normal, inheritable designs
		$override_designs = Array();
		foreach ($our_design_links as $link) {
			if (preg_match('/^override::design::(system|user)::.*$/', $link['value'])) {
				$name = preg_replace('/^override::(.*)$/', '$1', $link['value']);
				$override_designs[$name] = Array(
											'value'	=> $link['minorid'],
										   );
			}
		}
		unset($our_design_links);

		$our_layout_links = $GLOBALS['SQ_SYSTEM']->am->getLinks($this->id, SQ_LINK_NOTICE, 'paint_layout_page', FALSE);
		$layouts = Array();
		foreach ($our_layout_links as $link) {
			if (preg_match('/^paint_layout::(system|user)::.*$/', $link['value'])) {
				$layouts[$link['value']] = Array(
												'value'	=> $link['minorid'],
											   );
			}
		}

		// override layouts take priority over normal, inheritable layouts
		$override_layouts = Array();
		foreach ($our_layout_links as $link) {
			if (preg_match('/^override::paint_layout::system::.*$/', $link['value'])) {
				$name = preg_replace('/^override::(.*)$/', '$1', $link['value']);
				$override_layouts[$name] = Array(
											'value'	=> $link['minorid'],
										   );
			}
		}
		unset($link);
		unset($our_layout_links);

		// edit layouts applied to the asset
		$edit_layouts = Array();
		foreach($our_edit_layouts as $layout_url => $layouts) {
			foreach($layouts as $name => $layout_data) {
				if (!isset($edit_layouts[$name])) {
					$edit_layouts[$name] = Array(
											'value' => $layout_data['value'],
											);
				}//end if
			}//end foreach
		}//end foreach
		unset($our_edit_layouts);

		$parents = $GLOBALS['SQ_SYSTEM']->am->getLinks($this->id, SQ_SC_LINK_WEB_PATHS, '', NULL, 'minor');

		$done_urls    = Array();
		$done_parents = Array();
		for ($i = 0; $i < count($parents); $i++) {
			if (in_array($parents[$i]['majorid'], $done_parents)) {
				continue;
			}
			$parent = $GLOBALS['SQ_SYSTEM']->am->getAsset($parents[$i]['majorid'], $parents[$i]['major_type_code']);
			if (is_null($parent) || !is_object($parent)) {
				continue;
			}
			$done_parents[] = $parent->id;

			$parent_urls = $parent->getLookups();

			for ($j = 0, $num_parent_urls = count($parent_urls); $j < $num_parent_urls; $j++) {

				for ($k = 0; $k < $num_paths; $k++) {
					$new_url = $parent_urls[$j]['url'].'/'.$paths[$k];
					if (isset($done_urls[$new_url])) continue;

					// Get the protocol info for this new url
					$site_url_info = $GLOBALS['SQ_SYSTEM']->am->getRootURL($new_url);

					$site_https = isset($site_url_info['https']) ? $site_url_info['https'] : 0;
					$site_http = isset($site_url_info['http']) ? $site_url_info['http'] : 0;

					$https = ($this->force_secure === '-') ? '0' : $site_https;
					$http  = ($this->force_secure === '1') ? '0' : $site_http;

					if (!$http && !$https) {
						$https = $site_https;
						$http = $site_http;
					}

					// if we have no protocol then we can just ignore this URL
					if (!$http && !$https) {
						continue;
					}

					// check if another asset is using the new URL
					$sql = 'SELECT assetid FROM sq_ast_lookup WHERE url = :new_url';
					$existing_asset_with_url_id = NULL;
					try {
						$query = MatrixDAL::preparePdoQuery($sql);
						MatrixDAL::bindValueToPdo($query, 'new_url', $new_url);
						$existing_asset_with_url_id = MatrixDAL::executePdoAssoc($query, 0);
					} catch (Exception $e) {
						throw new Exception('Cannot check if another asset is already using URL "'.$new_url.'": '.$e->getMessage());
					}
					if (!empty($existing_asset_with_url_id)) {
						// if an existing asset has the same url as the new URL throw an error and rollback
						$existing_asset_with_url_info = $GLOBALS['SQ_SYSTEM']->am->getAssetInfo($existing_asset_with_url_id);
						trigger_localised_error('CORE0081', E_USER_WARNING, $new_url, $existing_asset_with_url_info[$existing_asset_with_url_id[0]]['name'], $existing_asset_with_url_id[0]);
						$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
						$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();
						return FALSE;
					}

					try {
						$bind_vars	= Array (
										'url'			=> $new_url,
										'assetid'		=> $this->id,
										'http'			=> $http,
										'https'			=> $https,
										'root_urlid'	=> $parent_urls[$j]['root_urlid'],
									  );
						$result = MatrixDAL::executeQuery('core', 'insertLookup', $bind_vars);
					} catch (Exception $e) {
						throw new Exception('Unable to insert lookups for asset "'.$this->name.'" (#'.$this->id.') due to database error: '.$e->getMessage());
					}

					$done_urls[] = $new_url;

					// get the asset lineage and use that as the index
					$url_lineage = $GLOBALS['SQ_SYSTEM']->am->getLineageFromURL(NULL, $parent_urls[$j]['url']);
					$new_url_assetids = Array();
					foreach ($url_lineage as $url_lineage_item) {
						$new_url_assetids[] = $url_lineage_item['assetid'];
					}
					$new_url_assetids[] = $this->id;

					// holds paint layout values and design values
					$lookup_values = Array();
					// holds override paint layout values and override design values
					$override_lookup_values = Array();

					// look for valid paint layouts to insert or update
					foreach ($tmp_old_urls as $tmp_old_url) {
						if ($tmp_old_url['url'] == $new_url) {
							// exact same URL as before
							$lookup_values = isset($our_lookup_values[$tmp_old_url['url']]) ? $our_lookup_values[$tmp_old_url['url']] : Array();
							// look for an override paint layout value
							$override_lookup_values = isset($our_lookup_values[$tmp_old_url['url'].'/']) ? $our_lookup_values[$tmp_old_url['url'].'/'] : Array();
							break;
						} else if (($tmp_old_url['parent_url'] == $parent_urls[$j]['url']) && ($tmp_old_url['web_path'] == $paths[$k])) {
							// the same parent URL and the same web path
							$lookup_values = isset($our_lookup_values[$tmp_old_url['url']]) ? $our_lookup_values[$tmp_old_url['url']] : Array();
							// look for an override paint layout value
							$override_lookup_values = isset($our_lookup_values[$tmp_old_url['url'].'/']) ? $our_lookup_values[$tmp_old_url['url'].'/'] : Array();
							break;
						} else if ($tmp_old_url['parent_url'] == $parent_urls[$j]['url']) {
							// same parent URL but different web path so change the affected paint layout entries

							// ensure web path changes are valid
							if (isset($web_path_changes[$tmp_old_url['web_path']]) && $web_path_changes[$tmp_old_url['web_path']] == $paths[$k]) {
								// retrieve normal paint layout lookup value
								$lookup_values = isset($our_lookup_values[$tmp_old_url['url']]) ? $our_lookup_values[$tmp_old_url['url']] : Array();
								// look for an override paint layout value
								$override_lookup_values = isset($our_lookup_values[$tmp_old_url['url'].'/']) ? $our_lookup_values[$tmp_old_url['url'].'/'] : Array();

								// update each affected paint layout's URL
								try {
									$bind_vars	= Array (
													'url'	=> $tmp_old_url['url'].'%',
													'name'	=> 'paint_layout::%',
												  );
									$applied_layout = MatrixDAL::executeAssoc('core', 'getLookupValueUrlsByPattern', 0, $bind_vars);
								} catch (Exception $e) {
									throw new Exception('Unable to get paint layouts for URL pattern: '.$tmp_old_url['url'].'%'.' due to database error: '.$e->getMessage());
								}

								foreach ($applied_layout as $key => $old_layout_url) {
									$new_layout_url = str_replace($tmp_old_url['url'], $new_url, $old_layout_url);
									try {
										$bind_vars	= Array (
														'seturl'	=> $new_layout_url,
														'whereurl'	=> $old_layout_url,
													  );
										$result = MatrixDAL::executeQuery('core', 'updateLookupValueUrl', $bind_vars);
									} catch (Exception $e) {
										throw new Exception('Unable to update lookup value table for existing URL: '.$old_layout_url.' with new URL: '.$new_layout_url.' due to database error: '.$e->getMessage());
									}
								}
								break;
							}
						}//end if
					}//end foreach

					// merge in design lookup values with our paint layout lookup values
					foreach ($our_designs as $name => $data) {
						$lookup_values[$name] = $data;
					}
					foreach ($override_designs as $name => $data) {
						$override_lookup_values[$name] = $data;
					}

					// merge in asset based paint layout values if they don't exist currently for this URL
					foreach ($layouts as $name => $data) {
						if (!in_array($name, array_keys($lookup_values))) {
							$lookup_values[$name] = $data;
						}
					}
					foreach ($override_layouts as $name => $data) {
						if (!in_array($name, array_keys($override_lookup_values))) {
							$override_lookup_values[$name] = $data;
						}
					}

					// merge in edit layout lookup values
					foreach($edit_layouts as $name => $data) {
						if (!in_array($name, array_keys($lookup_values))) {
							$lookup_values[$name] = $data;
						}
					}

					// calculate the depth of the url
					$exploded_url = explode('/', $new_url);
					$page_depth = count($exploded_url)-1;

					foreach ($lookup_values as $name => $data) {
						try {
							$bind_vars	= Array (
											'url'	=> $new_url,
											'name'	=> $name,
										  );
							$result = MatrixDAL::executeAssoc('core', 'getLookupValueUrls', $bind_vars);
							if (isset($result[0])) {
								$result = $result[0];
							}
						} catch (Exception $e) {
							throw new Exception('Unable to get lookup value URL for specified URL: '.$new_url.' due to database error: '.$e->getMessage());
						}

						// if the lookup value already exists, re-use it (this could happen
						// when a file changes back from a __data url)
						if (empty($result)) {
							try {
								$bind_vars	= Array (
												'url'	=> $new_url,
												'name'	=> $name,
												'value'	=> $data['value'],
												'depth'	=> $page_depth,
											  );
								$result = MatrixDAL::executeQuery('core', 'insertLookupValue', $bind_vars);
							} catch (Exception $e) {
								throw new Exception('Unable to insert new lookup value for specified URL: '.$new_url.' due to database error: '.$e->getMessage());
							}
						} else {
							try {
								$bind_vars	= Array (
												'value'	=> $data['value'],
												'url'	=> $new_url,
												'name'	=> $name,
											  );
								$result = MatrixDAL::executeQuery('core', 'updateLookupValue', $bind_vars);
							} catch (Exception $e) {
								throw new Exception('Unable to update lookup value for specified URL: '.$new_url.' due to database error: '.$e->getMessage());
							}
						}

					}//end foreach

					// for override values
					foreach ($override_lookup_values as $name => $data) {
						try {
							$bind_vars	= Array (
											'url'	=> $new_url.'/',
											'name'	=> $name,
										  );
							$result = MatrixDAL::executeAssoc('core', 'getLookupValueUrls', $bind_vars);
							if (isset($result[0])) {
								$result = $result[0];
							}
						} catch (Exception $e) {
							throw new Exception('Unable to get lookup value URL for specified URL: '.$new_url.' due to database error: '.$e->getMessage());
						}

						// if the lookup value already exists, re-use it (this could happen
						// when a file changes back from a __data url)
						if (empty($result)) {
							try {
								$bind_vars	= Array (
												'url'	=> $new_url.'/',
												'name'	=> $name,
												'value'	=> $data['value'],
												'depth'	=> $page_depth+1,
											  );
								$result = MatrixDAL::executeQuery('core', 'insertLookupValue', $bind_vars);
							} catch (Exception $e) {
								throw new Exception('Unable to insert new lookup value for specified URL: '.$new_url.' due to database error: '.$e->getMessage());
							}
						} else {
							try {
								$bind_vars	= Array (
												'value'	=> $data['value'],
												'url'	=> $new_url,
												'name'	=> $name,
											  );
								$result = MatrixDAL::executeQuery('core', 'updateLookupValue', $bind_vars);
							} catch (Exception $e) {
								throw new Exception('Unable to update lookup value for specified URL: '.$new_url.' due to database error: '.$e->getMessage());
							}
						}

					}//end foreach

				}//end for paths

			}//end for parent_urls

		}//end for parents

		// send a message to tell everyone the good news
		$msg = $ms->newMessage();
		$msg->subject = 'Asset Lookups Updated';
		$msg->type = 'asset.lookups.updated';

		$msg_reps = Array(
						'asset_name'	=> $this->name,
					);
		$msg->replacements = $msg_reps;
		$msg->parameters['assetid'] = $this->id;
		$msg->send();

		if (!empty($old_urls)) {
			if ($auto_add_remaps) {
				$rm = $GLOBALS['SQ_SYSTEM']->am->getSystemAsset('remap_manager');

				// get the first new URL
				// don't add a remap if we don't have a URL to map to
				if (count($done_urls) > 0) {
					foreach ($old_urls as $url_data) {
						$old_url = $url_data['url'];

						// Search the new URLs for the one closest to an old
						// URL..
						$closest_url = NULL;
						$most_bits_matched = -1;

						foreach ($done_urls as $new_url) {
							$new_url = strip_url($new_url, TRUE);
							$new_url_bits = explode('/', $new_url);
							$old_url_bits = explode('/', $old_url);

							for ($bits = 0; $bits < count($old_url_bits); $bits++) {
								if ((string)$old_url_bits[$bits] !== (string)$new_url_bits[$bits]) {
									break;
								}
							}//end for

							if ($bits > $most_bits_matched) {
								$closest_url = $new_url;
								$most_bits_matched = $bits;
							}

						}//end foreach URL

						// this URL is already in the list of new URLs, therefore we don't need a remap
						// because we're only making a remap to ourselves
						if (!in_array($old_url, $done_urls)) {
							// don't remap __data URLs, because they aren't served by Matrix
							foreach (explode("\n", SQ_CONF_SYSTEM_ROOT_URLS) as $root_url) {
								$data_url = $root_url.'/__data';
								if (substr($old_url, 0, strlen($data_url)) == $data_url) {
									continue 2;
								}
							}
							$static_url = SQ_CONF_STATIC_ROOT_URL;
							if (!empty($static_url) && strpos($old_url, $static_url) !== FALSE) {
								continue;
							}
							if ($url_data['http']) {
								$rm->addRemapURL('http://'.$old_url,  'http://'.$closest_url);
							}
							if ($url_data['https']) {
								$rm->addRemapURL('https://'.$old_url, 'https://'.$closest_url);
							}
						}
					}
				}
			}
		}//end if !empty($old_urls)

		$this->_finishUpdateLookups($ms, $db, $parameter);

		return TRUE;

	}//end updateLookups()


	/**
	* Set's the lookup values for this asset
	*
	* @param array	$values	array of the values that we want to save
	* <pre>
	*							"[url]" => array (
	*								"[name]" => array (
	*									"value" => "[value]",
	*								),
	*							),
	* </pre>
	*
	* @return boolean
	* @access public
	* @see updateLookups()
	*/
	public function setLookupValues(Array $values)
	{
		if (!$this->writeAccess('lookups')) {
			trigger_localised_error('SYS0265', E_USER_WARNING, $this->name, $this->id);
			return FALSE;
		}

		$db = MatrixDAL::getDb();
		$GLOBALS['SQ_SYSTEM']->changeDatabaseConnection('db2');
		$GLOBALS['SQ_SYSTEM']->doTransaction('BEGIN');

		// delete all lookup value URLs for this asset, including override values
		$sql = 'DELETE FROM
					sq_ast_lookup_value
				WHERE
						url IN
					(
						(SELECT
							url
						FROM
							sq_ast_lookup l
						WHERE
							assetid = :assetid_1)
						UNION
						(SELECT
							url || \'/\'
						FROM
							sq_ast_lookup l
						WHERE
							assetid = :assetid_2)
					)';
		try {
			$query = MatrixDAL::preparePdoQuery($sql);
			MatrixDAL::bindValueToPdo($query, 'assetid_1', $this->id);
			MatrixDAL::bindValueToPdo($query, 'assetid_2', $this->id);
			$result = MatrixDAL::execPdoQuery($query);
		} catch (Exception $e) {
			throw new Exception('Unable to delete lookup values for asset: '.$this->id.' due to database error: '.$e->getMessage());
		}

		$lookups = $this->getLookups('url');

		foreach ($values as $url => $values_data) {
			// check if this is a valid URL reflected in our lookups table
			$valid_url = FALSE;
			foreach ($lookups as $lookup_url) {
				if ($url === $lookup_url) {
					// normal value
					$valid_url = TRUE;
				} else if ($url === $lookup_url.'/') {
					// override value
					$valid_url = TRUE;
				}
			}
			if (!$valid_url) continue;
			if (empty($values_data)) continue;

			// calculate the depth of the url
			$exploded_url = explode('/', $url);
			$page_depth = count($exploded_url)-1;

			foreach ($values_data as $value_name => $value_data) {
				try {
					$bind_vars	= Array (
									'url'	=> $url,
									'name'	=> $value_name,
									'value'	=> $value_data['value'],
									'depth'	=> $page_depth,
								  );
					$result = MatrixDAL::executeQuery('core', 'insertLookupValue', $bind_vars);
				} catch (Exception $e) {
					throw new Exception('Unable to insert new lookup value for specified URL: '.$url.' due to database error: '.$e->getMessage());
				}
			}
		}//end foreach

		$GLOBALS['SQ_SYSTEM']->doTransaction('COMMIT');
		$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();
		return TRUE;

	}//end setLookupValues()


	/**
	* Returns a replacement for the asset_status_description keyword
	* Will return 'Live', 'Under Construction' etc
	*
	* @return boolean
	* @access public
	*/
	public function getAssetStatusDescriptionKeywordReplacement()
	{
		return get_status_description($this->status);

	}//end getAssetStatusDescriptionKeywordReplacement()


	/**
	* Returns a replacement for the asset_status_colour keyword
	* Returns it in '#RRGGBB' Hex form, and the colour corresponds
	* to that seen in the asset map for statuses.
	*
	* @return string
	* @access public
	*/
	public function getAssetStatusColourKeywordReplacement()
	{
		return '#'.get_status_colour($this->status);

	}//end getAssetStatusColourKeywordReplacement()


	/**
	* Replaces all keywords in a string
	*
	* This function shouldn't handle global keywords
	*
	* @param string	$string	containing keywords
	*
	* @return string
	* @access public
	*/
	public function replaceKeywordsInString($string)
	{
		$replacements = Array();
		$keyword_list = extract_keywords($string);
		if (empty($keyword_list) || !is_array($keyword_list)) {
			return $string;
		}

		foreach ($keyword_list as $keyword) {
			$replacements[$keyword] = $this->getKeywordReplacement($keyword);
		}

		replace_keywords($string, $replacements);

		return $string;

	}//end replaceKeywordsInString()


	/**
	* Fast Track is way of flagging on this asset, "it's currently
	* safe to skip certain expensive tasks". This is useful when
	* trying to avoid things like search reindexing from happening
	* multiple times when it only needs to happen once (during asset
	* creation for example).
	*
	* As such, it's a less than ideal yet pragmatic way to find a
	* "fast path" for the spaghetti that is asset builder / applying
	* schemas / regenerating metadata / reindexing search in multiple
	* context sauce.
	*
	* e.g.
	*
	* $asset->fastTrack(Array(	'metadata_manager_generate_content_file',
	*							'search_manager_reindex_metadata',
	*							'trigger_event_metadata_updated'));
	*
	* // Do stuff that may trigger events in loops for those tasks..
	*
	* $asset->unFastTrack(Array(	'metadata_manager_generate_content_file',
	*								'search_manager_reindex_metadata',
	*								'trigger_event_metadata_updated'));
	*
	* // Now do those tasks once.
	*
	* @see CMS::Page_Asset_Builder
	* @see Metadata_Manager
	* @param Mixed $tasks Identifiers to be fast tracked.
	* @return void
	* @access public
	*/
	public function fastTrack($tasks)
	{
		if (!is_array($tasks)) $tasks = Array($tasks);
		if (!isset($this->_tmp['fast_track'])) $this->_tmp['fast_track'] = Array();

		foreach($tasks as $task) {
			// Initialise the "fast tracked assetids" array only if we have not fast tracked this task before,
			// or if we have removed it from being fast tracked
			if (!isset($this->_tmp['fast_track'][$task.'_assetids']) || !isset($this->_tmp['fast_track'][$task]) || !$this->_tmp['fast_track'][$task]) {
				$this->_tmp['fast_track'][$task.'_assetids'] = Array();
			}

			$this->_tmp['fast_track'][$task] = TRUE;
		}

	}//end fastTrack()


	/**
	* Remove the given task from being fast tracked, usually so that
	* the caller can finally perform the given tasks sanely (once).
	*
	* @see CMS::Page_Asset_Builder
	* @see Metadata_Manager
	* @param Mixed $tasks Identifiers to be removed from fast track.
	* @return void
	* @access public
	*/
	public function unFastTrack($tasks)
	{
		if (!is_array($tasks)) $tasks = Array($tasks);
		if (!isset($this->_tmp['fast_track'])) $this->_tmp['fast_track'] = Array();

		foreach($tasks as $task) {
			$this->_tmp['fast_track'][$task] = FALSE;
		}

	}//end unFastTrack()


	/**
	* Determine if the given task should be fast tracked. Generally called from the
	* place doing the expensive task.
	*
	* @param $task 		string 	Task to fasttrack
	* @param $assetid	int		Asset associated with the task

	* @see Trigger_Manager
	* @see Search_Manger
	* @see Metadata_Manager
	* @access public
	*/
	public function shouldFastTrack($task, $assetid=NULL)
	{
		if (isset($this->_tmp['fast_track'][$task]) && $this->_tmp['fast_track'][$task]) {
			// Keep track of assetids associated with the task that has been fast tracked
			if (!is_null($assetid)) {
				$this->_tmp['fast_track'][$task.'_assetids'][] = $assetid;
			}
			return TRUE;
		}

		return FALSE;

	}//end shouldFastTrack()


	/**
	* Returns a list of assetids associated with the given task
	* that has been fast tracked
	*
	* @param $task	string Fasktracked task name
	*
	* @return Array
	* @access public
	*/
	public function getFaskTrackedTaskAssetids($task)
	{
		return !empty($this->_tmp['fast_track'][$task.'_assetids']) ? $this->_tmp['fast_track'][$task.'_assetids'] : Array();

	}//end getFaskTrackedTaskAssetids()


	/**
	* If the asset doesn't has URL, get it from the dependant parent, if any
	*
	* @return string
	* @access public
	*/
	public function getDependantParentsURL()
	{
		$asset_url = $this->getURL();
		if (empty($asset_url)) {
			$dependant_parents = $GLOBALS['SQ_SYSTEM']->am->getDependantParents($this->id, '', TRUE, FALSE);

			$dependant_parent = isset($dependant_parents[0]) ? $GLOBALS['SQ_SYSTEM']->am->getAsset($dependant_parents[0]) : NULL;
			if (!is_null($dependant_parent)) {
				$asset_url = $dependant_parent->getURL();
				$GLOBALS['SQ_SYSTEM']->am->forgetAsset($dependant_parent);
			}
		}//end if

		return $asset_url;

	}//end getDependantParentURL()

}//end class

?>
