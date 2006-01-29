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
* $Id: asset.inc,v 1.499 2006/01/29 22:41:26 lwright Exp $
*
*/


/**
* Asset
*
*	Basic data item in MySource Matrix, from which all pages, users and other items inherit
*
*
* @author  Blair Robertson <blair@squiz.net>
* @version $Revision: 1.499 $
* @package MySource_Matrix
*/
class Asset extends MySource_Object
{

	/**
	* The unique ID for this asset
	* @var int
	*/
	var $id = 0;

	/**
	* The version number of the asset (x.x.x)
	* @var string
	*/
	var $version = '';

	/**
	* String representation of this asset
	* @var string
	*/
	var $name = '';

	/**
	* Short String representation of this asset
	* @var string
	*/
	var $short_name = '';

	/**
	* Current status of the asset (live, under contruction etc)
	* @var int
	*/
	var $status;

	/**
	* Current languages set for the asset
	* @var string
	*/
	var $languages = '';

	/**
	* Current character set of the asset
	* @var string
	*/
	var $charset = '';

	/**
	* Whether this asset should only have secure URL lookups
	* @var boolean
	*/
	var $force_secure = 0;

	/**
	* Timestamp of when this asset was created
	* @var int
	*/
	var $created;

	/**
	* Assetid for the user that created this asset
	* @var int
	*/
	var $created_userid;

	/**
	* Timestamp of when this asset was last updated
	* @var int
	*/
	var $updated;

	/**
	* Assetid for the last user that updated this asset
	* @var int
	*/
	var $updated_userid;

	/**
	* Timestamp of when this asset was last published (ie. made LIVE)
	* @var int
	*/
	var $published;

	/**
	* Assetid for the user that last published this asset
	* @var int
	*/
	var $published_userid;

	/**
	* Timestamp of when this asset was last published (ie. made LIVE)
	* @var int
	*/
	var $status_changed;

	/**
	* Assetid for the user that last published this asset
	* @var int
	*/
	var $status_changed_userid;

	/**
	* Is this asset type able to be cached automatically by the system?
	* @var boolean
	*/
	var $_is_cacheable = false;

	/**
	* The Vars that belong to this asset
	* @var Array('var_name' => Array('attributeid' => $attributeid, 'value' = $value))
	*/
	var $vars = Array();

	/**
	* An array of keywords that this asset makes available for use along with a default replacement
	* @var Array(keyword => name)
	*/
	var $_available_keywords = Array();

	/**
	* The suffix for the data path to be used for public, private, and file versioning
	*
	* @var string
	*/
	var $data_path_suffix = '';

	/**
	* The current data path for this asset
	* This always points to the restricted directory but may also
	* point to the .sq_system directory within it
	*
	* @var string
	*/
	var $data_path = '';

	/**
	* The current unrestrcited data path for this asset
	* This always points to the unrestricted directory
	* Only use this path for storage if the web server should serve the file
	*
	* @var string
	*/
	var $data_path_public = '';

	/**
	* Whether this asset has serialise attributes or not, basically a performance var
	* when true the vars a looped over in _loadVars() and if they are type 'serialise' unserialised
	*
	* @var bool
	*/
	var $_ser_attrs = false;


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
	* <PRE>
	* Array ('asset'         => [ref major asset to create link under],
	*        'link_type'     => SQ_LINK_?,
	*        'value'         => [link value],
	*        'sort_order'    => [link sort order],
	*        'is_dependant'  => [0|1],
	*        'is_exclusive'  => [0|1]
	*        )
	* </PRE>
	*
	* @return mixed int|boolean
	* @access public
	*/
	function create(&$link)
	{
		$GLOBALS['SQ_SYSTEM']->changeDatabaseConnection('db2');
		$GLOBALS['SQ_SYSTEM']->doTransaction('BEGIN');
		$db =& $GLOBALS['SQ_SYSTEM']->db;

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
		$trigger_level_changed = false;
		if ($GLOBALS['SQ_SYSTEM']->runLevelEnables(SQ_SECURITY_TRIGGERS)) {
			$GLOBALS['SQ_SYSTEM']->setRunLevel($GLOBALS['SQ_SYSTEM']->getRunLevel() - SQ_SECURITY_TRIGGERS);
			$trigger_level_changed = true;
		}

		require_once SQ_FUDGE_PATH.'/db_extras/db_extras.inc';

		// let the system know that we are creating this asset so we dont
		// do any updating of versions etc. until we are done
		$this->_tmp['__creating__'] = true;

		$assetid = $db->nextId('sq_ast');
		assert_valid_db_result($assetid);

		$name   = ucwords(str_replace('_', ' ', $this->type())).' #'.$assetid;
		$now    = time();
		$userid = $GLOBALS['SQ_SYSTEM']->currentUserId();
		$initial_version = '0.0.1';
		$initial_status = SQ_STATUS_UNDER_CONSTRUCTION;
		if (array_get_index($link, 'is_dependant') && ($link['asset']->status != SQ_STATUS_UNDER_CONSTRUCTION)) {
			$wfm =& $GLOBALS['SQ_SYSTEM']->getWorkflowManager();
			if (0 == count($wfm->getSchemas($link['asset']->id, true))) {
				// no workflow for parent
				 $initial_status = $link['asset']->status;
			}
		}

		$now_sql = db_extras_todate($db, ts_iso8601($now));
		$sql = 'INSERT INTO
					sq_ast
				(
					assetid,
					version,
					type_code,
					name,
					short_name,
					status,
					created,
					created_userid,
					updated,
					updated_userid,
					published,
					published_userid,
					status_changed,
					status_changed_userid
				)
				VALUES
				(
					'.$db->quoteSmart($assetid).',
					'.$db->quoteSmart($initial_version).',
					'.$db->quoteSmart($this->type()).',
					'.$db->quoteSmart($name).',
					'.$db->quoteSmart($name).',
					'.$db->quoteSmart($initial_status).',
					'.$now_sql.',
					'.$db->quoteSmart($userid).',
					'.$now_sql.',
					'.$db->quoteSmart($userid).',
					  null,
					  null,
					'.$now_sql.',
					'.$db->quoteSmart($userid).')';

		$result = $db->query($sql);
		assert_valid_db_result($result);

		$this->id                    = $assetid;
		$this->version               = $initial_version;
		$this->name                  = $name;
		$this->short_name            = $name;
		$this->status                = $initial_status;
		$this->created               = $now;
		$this->created_userid        = $userid;
		$this->updated               = $now;
		$this->updated_userid        = $userid;
		$this->published             = null;
		$this->published_userid      = null;
		$this->status_changed        = $now;
		$this->status_changed_userid = $userid;

		// log what happened
		$ms = &$GLOBALS['SQ_SYSTEM']->getMessagingService();
		$ms->openLog();

		// do this so that all calls to getAsset() get a reference to us, and don't load a new instance from the DB
		$GLOBALS['SQ_SYSTEM']->am->rememberAsset($this);

		if ($GLOBALS['SQ_SYSTEM']->runLevelEnables(SQ_SECURITY_INTEGRITY)) {
			// OK, what we need to do here is set write access for this asset to the current user
			// so that they can actually do anything
			// NOW normally this would require admin access but this is a very special case
			$this->_tmp[__CLASS__.'_in_create_cascading'] = true;

			if ($GLOBALS['SQ_SYSTEM']->runLevelEnables(SQ_SECURITY_LOCKING)) {
				if (!$GLOBALS['SQ_SYSTEM']->am->acquireLock($this->id, 'permissions')) {
					return $this->_abortCreate($trigger_level_changed);
				}
			}

			// only set the permission if there is a userid to set it for - otherwise we are going
			// to be setting public write access
			if (!$GLOBALS['SQ_SYSTEM']->userPublic()) {
				if (!$GLOBALS['SQ_SYSTEM']->am->setPermission($this->id, $GLOBALS['SQ_SYSTEM']->currentUserId(), SQ_PERMISSION_WRITE, true)) {
					return $this->_abortCreate($trigger_level_changed);
				}
			}

			unset($this->_tmp[__CLASS__.'_in_create_cascading']);
		}

		$message_body = 'New '.$this->type().' "'.$this->name.'" created';
		$msg_reps = Array(
						'type_code'		=> $this->type(),
						'asset_name'	=> $this->name,
					);
		$message = $ms->newMessage(Array(), 'asset', $msg_reps);
		$message->parameters['assetid'] = $this->id;
		$message->send();

		// set the attribute values that have been stored temporarily
		if ($GLOBALS['SQ_SYSTEM']->runLevelEnables(SQ_SECURITY_LOCKING)) {
			if (!$GLOBALS['SQ_SYSTEM']->am->acquireLock($this->id, 'attributes')) {
				return $this->_abortCreate($trigger_level_changed);
			}
		}
		if (!$this->saveAttributes(true)) {
			return $this->_abortCreate($trigger_level_changed);
		}

		if ($GLOBALS['SQ_SYSTEM']->runLevelEnables(SQ_SECURITY_LOCKING)) {
			$GLOBALS['SQ_SYSTEM']->am->releaseLock($this->id, 'attributes');
		}

		$this->_loadVars();

		if (!empty($link)) {
			// create the initial link
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

			if ($GLOBALS['SQ_SYSTEM']->runLevelEnables(SQ_SECURITY_LOCKING)) {
				$lock_info = @$GLOBALS['SQ_SYSTEM']->am->getLockInfo($link['asset']->id, 'links');
				$parent_was_locked = !empty($lock_info);
				if (!$GLOBALS['SQ_SYSTEM']->am->acquireLock($link['asset']->id, 'links')) {
					return $this->_abortCreate($trigger_level_changed);
				}

				// if we are creating a dependant link, we need to make sure this new
				// asset is locked in the same chain as the parent (ie, has the same source_assetid)
				$source_assetid = 0;
				if ($link['is_dependant']) {
					$lock = $GLOBALS['SQ_SYSTEM']->am->getLockInfo($link['asset']->id, 'links');
					$source_assetid = $lock['source_assetid'];
				}
				if (!$GLOBALS['SQ_SYSTEM']->am->acquireLock($this->id, 'links', $source_assetid)) {
					return $this->_abortCreate($trigger_level_changed);
				}
			}
			$linkid = $link['asset']->createLink($this, $link['link_type'], $link['value'], $link['sort_order'], $link['is_dependant'], $link['is_exclusive']);

			// get the link so we can find out if it was dependant (cant rely on the info passed in
			// because other assets may have changed what happens in createLink)
			$new_link = $GLOBALS['SQ_SYSTEM']->am->getLinkById($linkid, $this->id, 'minor');

			if ($GLOBALS['SQ_SYSTEM']->runLevelEnables(SQ_SECURITY_INTEGRITY)) {
				// cascade permissions, and schemas on this asset that the parent
				// has if the parent is not the root folder
				$this->_tmp[__CLASS__.'_in_create_cascading'] = true;

				if (!$link['asset']->cloneComponents($this, Array('permissions', 'metadata_schemas', 'workflow', 'content_tags'))) {
					return $this->_abortCreate($trigger_level_changed);
				}
				unset($this->_tmp[__CLASS__.'_in_create_cascading']);
			}

			if ($GLOBALS['SQ_SYSTEM']->runLevelEnables(SQ_SECURITY_LOCKING)) {
				if (!$GLOBALS['SQ_SYSTEM']->am->releaseLock($this->id, 'permissions')) {
					return $this->_abortCreate($trigger_level_changed);
				}

				// release locks depending on if the parent was locked before we were created
				if (!$parent_was_locked) {
					// parent was not locked, so unlock both the parent and us
					if (!$GLOBALS['SQ_SYSTEM']->am->releaseLock($link['asset']->id, 'links')) {
						return $this->_abortCreate($trigger_level_changed);
					}
				} else if ($new_link['is_dependant']) {
					// the parent was locked before we created this asset and the new asset
					// is dependantly linked, so make sure we have all the locks of our parent
					$parent_lock_info = $GLOBALS['SQ_SYSTEM']->am->getLockInfo($link['asset']->id, 'all');
					foreach (array_values($parent_lock_info) as $lock_details) {
						if (empty($lock_details)) continue;
						if (!$GLOBALS['SQ_SYSTEM']->am->acquireLock($this->id, $lock_details['lock_type'], $lock_details['source_assetid'])) {
							return $this->_abortCreate($trigger_level_changed);
						}
					}
				}

				if (!$new_link['is_dependant']) {
					if (!$GLOBALS['SQ_SYSTEM']->am->releaseLock($this->id, 'links')) {
						return $this->_abortCreate($trigger_level_changed);
					}
				}
			}

			if (!$linkid) {
				return $this->_abortCreate($trigger_level_changed);
			}

		}//end if creating link

		// re-enable the triggers, if they were disabled here
		if ($trigger_level_changed) {
			$GLOBALS['SQ_SYSTEM']->restoreRunLevel();
		}

		// before firing the "Before Asset Created" event, turn off locking checks
		$modified_runlevel = false;
		if ($GLOBALS['SQ_SYSTEM']->runLevelEnables(SQ_SECURITY_LOCKING)) {
			$GLOBALS['SQ_SYSTEM']->setRunLevel($GLOBALS['SQ_SYSTEM']->getRunLevel() - SQ_SECURITY_LOCKING);
			$modified_runlevel = true;
		}

		// fire the event, abort if the event fails
		$event_result = $GLOBALS['SQ_SYSTEM']->broadcastTriggerEvent('trigger_event_before_asset_created', $this);

		if ($modified_runlevel) {
			$GLOBALS['SQ_SYSTEM']->restoreRunLevel();
		}

		if (!$event_result) return $this->_abortCreate();


		if ($GLOBALS['SQ_SYSTEM']->runLevelEnables(SQ_SECURITY_INTEGRITY)) {
			// perform any additional processing during the create - such as creating other assets
			if (!$this->_createAdditional($link)) {
				return $this->_abortCreate();
			}
		}

		unset($this->_tmp['__creating__']);
		if (!$this->_updated()) return $this->_abortCreate();
		$GLOBALS['SQ_SYSTEM']->doTransaction('COMMIT');
		$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();

		// because we are inheriting statuses, perhaps we are in safe edit now?
		if ($this->status == SQ_STATUS_EDITING) {
			$this->saveSystemVersion();
		}

		// fire the 'Asset Created' event
		$GLOBALS['SQ_SYSTEM']->broadcastTriggerEvent('trigger_event_asset_created', $this);
		$em = &$GLOBALS['SQ_SYSTEM']->getEventManager();

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
			return true;
		}

	}//end create()


	/**
	* Perform any pre-validation required during the creation of this asset
	*
	* For example, checking that attributes are set should bee performed in this method
	*
	* @param array	&$link	information used to create the initial link<br/>
	* <PRE>
	* Array (
	*   'asset'        => [ref major asset to create link under],
	*   'link_type'    => SQ_LINK_?,
	*   'value'        => [link value],
	*   'sort_order'   => [link sort order],
	*   'is_dependant' => [0|1],
	*   'is_exclusive' => [0|1],
	* )
	* </PRE>
	*
	* @return boolean
	* @access private
	*/
	function _preCreateCheck(&$link)
	{
		return true;

	}//end _preCreateCheck()


	/**
	* Perform any additional processing required during the creation of this asset
	*
	* For example, creating other child assets should be performed in this method
	*
	* @param array	&$link	information used to create the initial link<br/>
	* <PRE>
	* Array (
	*   'asset'        => [ref major asset to create link under],
	*   'link_type'    => SQ_LINK_?,
	*   'value'        => [link value],
	*   'sort_order'   => [link sort order],
	*   'is_dependant' => [0|1],
	*   'is_exclusive' => [0|1],
	* )
	* </PRE>
	*
	* @return boolean
	* @access private
	*/
	function _createAdditional(&$link)
	{
		return true;

	}//end _createAdditional()


	/**
	* Abort the creation of this asset
	*
	* The create log and DB transaction will also be aborted
	*
	* @param boolean	$trigger_level_changed	if it's false, restore runlevel
	*
	* @return boolean
	* @access private
	*/
	function _abortCreate($trigger_level_changed=false)
	{
		if ($trigger_level_changed) {
			$GLOBALS['SQ_SYSTEM']->restoreRunLevel();
		}
		$ms = &$GLOBALS['SQ_SYSTEM']->getMessagingService();
		$ms->abortLog();
		$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
		$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();
		$GLOBALS['SQ_SYSTEM']->am->forgetAsset($this);
		unset($this->_tmp['__creating__']);
		$this->id = 0;
		return false;

	}//end _abortCreate()


	/**
	* Load the asset represented by the passed asset ID
	*
	* @param int	$assetid	the asset ID to be loaded
	*
	* @return void
	* @access public
	*/
	function load($assetid)
	{
		$db = &$GLOBALS['SQ_SYSTEM']->db;

		// OK, the first thing to do is check we actually exist :)
		$sql = 'SELECT assetid, type_code, version, name, short_name, status, languages,
				charset, force_secure, created, created_userid, updated, updated_userid,
				published, published_userid, status_changed, status_changed_userid
				FROM '.SQ_TABLE_RUNNING_PREFIX.'ast ';
		$where = 'assetid = '.$db->quote($assetid);
		$where = $GLOBALS['SQ_SYSTEM']->constructRollbackWhereClause($where);

		$result = $db->getRow($sql.$where, null, DB_FETCHMODE_ORDERED);
		assert_valid_db_result($result);
		if (is_null($result)) {
			trigger_localised_error('SYS0087', E_USER_WARNING, $assetid);
			return;
		}

		list(
		$this->id,
		$type_code,
		$this->version,
		$this->name,
		$this->short_name,
		$this->status,
		$this->languages,
		$this->charset,
		$this->force_secure,
		$this->created,
		$this->created_userid,
		$this->updated,
		$this->updated_userid,
		$this->published,
		$this->published_userid,
		$this->status_changed,
		$this->status_changed_userid
		) = $result;

		$this->created = iso8601_ts($this->created);
		$this->updated = iso8601_ts($this->updated);
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
			$this->created               = null;
			$this->created_userid        = null;
			$this->updated               = null;
			$this->updated_userid        = null;
			$this->published             = null;
			$this->published_userid      = null;
			$this->status_changed        = null;
			$this->status_changed_userid = null;
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
	function _loadDataPaths()
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
	function _loadVars()
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
			$sql = 'SELECT atr.name, atr.attrid, atr.type, COALESCE(v.custom_val, atr.default_val) AS value
					FROM sq_ast_attr atr
					  LEFT OUTER JOIN '.SQ_TABLE_RUNNING_PREFIX.'ast_attr_val v
						ON (atr.attrid = v.attrid AND v.assetid = '.$GLOBALS['SQ_SYSTEM']->db->quote($this->id)
						.$GLOBALS['SQ_SYSTEM']->constructRollbackWhereClause('', 'v', 'AND').')
					WHERE atr.type_code = '.$GLOBALS['SQ_SYSTEM']->db->quote($this->type());
		} else {
			// else just load all defaults
			$sql = 'SELECT atr.name, atr.attrid, atr.type, atr.default_val AS value
					FROM sq_ast_attr atr
					WHERE atr.type_code = '.$GLOBALS['SQ_SYSTEM']->db->quote($this->type());
		}// end if

		$this->vars = $GLOBALS['SQ_SYSTEM']->db->getAssoc($sql, false, Array(), DB_FETCHMODE_ASSOC);
		assert_valid_db_result($this->vars);

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
	* Determine whether this asset can have its last significant link removed
	*
	* This function is most commonly called when moving the asset to the trash.
	*
	* @return boolean
	* @access public
	*/
	function canDelete()
	{
		return true;

	}//end canDelete()


	/**
	* Delete this asset from the trash
	*
	* Note that the asset <i>MUST</i> be in the trash to delete it <i>FROM</i> the trash.
	*
	* @param boolean	$release_lock	should we realease the lock after deleting
	*
	* @return boolean
	* @access public
	*/
	function delete($release_lock=true)
	{
		// check that we are in the trash
		if (!$GLOBALS['SQ_SYSTEM']->am->assetInTrash($this->id)) {
			trigger_localised_error('SYS0103', E_USER_WARNING, $this->name, $this->id);
			return false;
		}

		$GLOBALS['SQ_SYSTEM']->changeDatabaseConnection('db2');
		$db = &$GLOBALS['SQ_SYSTEM']->db;
		$GLOBALS['SQ_SYSTEM']->doTransaction('BEGIN');

		// say to other assets: 'Delete me bitch cause I'm outa here'
		$links = $GLOBALS['SQ_SYSTEM']->am->getLinks($this->id, SQ_SC_LINK_ALL, '', true, 'minor');
		foreach ($links as $link) {
			$major = &$GLOBALS['SQ_SYSTEM']->am->getAsset($link['majorid'], $link['major_type_code']);
			if (!is_null($major)) {
				$major->deleteLink($link['linkid']);
			} else {
				trigger_localised_error('SYS0128', E_USER_WARNING, $link['majorid']);
				$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
				$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();
				return false;
			}
		}

		if ($release_lock) {
			// we are about to go to the big asset manager in the sky,
			// so we wont be needing worldly things such as locks
			if (!$GLOBALS['SQ_SYSTEM']->am->releaseLock($this->id, 'all')) {
				trigger_localised_error('SYS0131', E_USER_WARNING, $this->name);
				$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
				$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();
				return false;
			}
		}

		// remove web paths
		if (!$this->saveWebPaths(Array())) {
			trigger_localised_error('SYS0133', E_USER_WARNING, $this->name);
			$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
			$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();
			return false;
		}

		// update the lookups to clear them; figures that if there are no web paths there cannot
		// be any lookups
		if (!$this->updateLookups()) {
			trigger_localised_error('SYS0132', E_USER_WARNING, $this->name);
			$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
			$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();
			return false;
		}

		// delete from the asset table
		$sql = 'DELETE FROM
					sq_ast
				WHERE
					assetid	= '.$db->quoteSmart($this->id);

		$result = $db->query($sql);
		assert_valid_db_result($result);

		// delete the attributes
		$sql = 'DELETE FROM
					sq_ast_attr_val
				WHERE
					assetid	= '.$db->quoteSmart($this->id);

		$result = $db->query($sql);
		assert_valid_db_result($result);

		$sql = 'DELETE FROM
					sq_ast_attr_uniq_val
				WHERE
					assetid	= '.$db->quoteSmart($this->id);

		$result = $db->query($sql);
		assert_valid_db_result($result);

		$sql = 'DELETE FROM
					sq_ast_perm
				WHERE
						assetid	= '.$db->quoteSmart($this->id).'
					OR	userid	= '.$db->quoteSmart($this->id);

		$result = $db->query($sql);
		assert_valid_db_result($result);

		// delete any metadata values this asset might have
		$mm = &$GLOBALS['SQ_SYSTEM']->getMetadataManager();
		if (!$mm->purgeMetadata($this->id)) {
			$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
			$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();
			return false;
		}

		// delete any workflow values that this asset might have
		$wm = &$GLOBALS['SQ_SYSTEM']->getWorkflowManager();
		if (!$wm->purgeWorkflow($this->id)) {
			$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
			$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();
			return false;
		}

		// now try and clear the data directories for this asset
		require_once SQ_FUDGE_PATH.'/general/file_system.inc';
		if (is_dir($this->data_path)) {
			if (!delete_directory($this->data_path)) {
				trigger_localised_error('SYS0151', E_USER_WARNING, $this->name);
				$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
				$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();
				return false;
			}
		}

		if (is_dir($this->data_path_public)) {
			if (!delete_directory($this->data_path_public)) {
				trigger_localised_error('SYS0151', E_USER_WARNING, $this->name);
				$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
				$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();
				return false;
			}
		}

		$GLOBALS['SQ_SYSTEM']->doTransaction('COMMIT');
		$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();

		$em = &$GLOBALS['SQ_SYSTEM']->getEventManager();
		$em->broadcastEvent($this, 'assetDeleted', Array());
		$GLOBALS['SQ_SYSTEM']->am->forgetAsset($this, true);

		return true;

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
	function useSystemVersion()
	{
		if (!isset($this->_tmp['use_system_version'])) {
			$usv = (($this->status & SQ_SC_STATUS_SAFE_EDITING) && !$this->writeAccess(''));
			if ($usv && !$GLOBALS['SQ_SYSTEM']->userPublic()) {
				$wfm = &$GLOBALS['SQ_SYSTEM']->getWorkflowManager();
				$publishers = $wfm->whoCanPublish($this->id);
				if (!empty($publishers) && in_array($GLOBALS['SQ_SYSTEM']->currentUserId(), $publishers)) {
					$usv = false;
				}
			}
			$this->_tmp['use_system_version'] = $usv;
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
	function loadSystemVersion()
	{
		require_once SQ_FUDGE_PATH.'/general/file_system.inc';

		// make sure our data directory exists
		if (!file_exists($this->data_path.'/.object_data')) {
			trigger_localised_error('SYS0159', E_USER_WARNING, $this->id, $this->data_path);
			return false;
		}

		$real_status = $this->status;
		$this = unserialize(file_to_string($this->data_path.'/.object_data'));
		$this->status = $real_status;
		return true;

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
			return false;
		}

		// make sure our system directories exists
		if (!create_directory($this->data_path.'/.sq_system')) {
			trigger_localised_error('CORE0050', E_USER_WARNING, $this->name);
			return false;
		}

		// make sure there is nothing in the system directories
		if (!clear_directory($this->data_path.'/.sq_system')) {
			trigger_localised_error('CORE0050', E_USER_WARNING, $this->name);
			return false;
		}

		// save the object for later (in the restricted directory)
		if (!string_to_file(serialize($this), $this->data_path.'/.sq_system/.object_data')) {
			trigger_localised_error('CORE0051', E_USER_WARNING, $this->name);
			return false;
		}

		// move all the other files in our data directories
		// into the new system directories for later use
		$files_to_copy = list_files($this->data_path);
		foreach ($files_to_copy as $filename) {
			if (!copy_file($this->data_path.'/'.$filename, $this->data_path.'/.sq_system/'.$filename)) {
				trigger_localised_error('SYS0166', E_USER_WARNING, $this->name, $filename);
				return false;
			}
		}

		return true;

	}//end saveSystemVersion()


	/**
	* Clear the directory for saving the system version
	*
	* @return boolean
	* @access public
	*/
	function clearSystemVersion()
	{
		require_once SQ_FUDGE_PATH.'/general/file_system.inc';

		// make sure our data directory exists
		if (!create_directory($this->data_path)) {
			trigger_localised_error('SYS0148', E_USER_WARNING, $this->name);
			return false;
		}

		// make sure our system directory exists
		if (!create_directory($this->data_path.'/.sq_system')) {
			trigger_localised_error('SYS0299', E_USER_WARNING, $this->name);
			return false;
		}

		if (!clear_directory($this->data_path.'/.sq_system')) {
			trigger_localised_error('SYS0147', E_USER_WARNING, $this->name);
			return false;
		}

		return true;

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
	function revertToSystemVersion()
	{
		$GLOBALS['SQ_REVERT_TO_SYSTEM_VERSION'] = true;
		require_once SQ_FUDGE_PATH.'/general/file_system.inc';

		// load our object data in as it was before
		if (is_file($this->data_path.'/.sq_system/.object_data')) {
			$old_version = unserialize(file_to_string($this->data_path.'/.sq_system/.object_data'));
		} else {
			trigger_localised_error('SYS0163', E_USER_WARNING, $this->name);
			$GLOBALS['SQ_REVERT_TO_SYSTEM_VERSION'] = false;
			return false;
		}

		// copy over the old files we stored in the system directory
		$files_to_copy = list_files($this->data_path.'/.sq_system');
		foreach ($files_to_copy as $filename) {
			// skip hidden files
			if (strpos($filename, '.') === 0) continue;

			if (!copy_file($this->data_path.'/.sq_system/'.$filename, $this->data_path.'/'.$filename)) {
				trigger_localised_error('SYS0162', E_USER_WARNING, $this->name, $filename);
				$GLOBALS['SQ_REVERT_TO_SYSTEM_VERSION'] = false;
				return false;
			}
		}

		// update all the vars
		foreach ($old_version->vars as $var_name => $var_data) {
			if ($var_data['type'] == 'serialise') {
				$var_data['value'] = serialize($var_data['value']);
			}
			if (!$this->setAttrValue($var_name, $var_data['value'], true)) {
				trigger_localised_error('SYS0165', E_USER_WARNING, $this->name, $var_name);
				$GLOBALS['SQ_REVERT_TO_SYSTEM_VERSION'] = false;
				return false;
			}
		}
		if (!$this->saveAttributes()) {
			trigger_localised_error('SYS0164', E_USER_WARNING, $this->name);
			$GLOBALS['SQ_REVERT_TO_SYSTEM_VERSION'] = false;
			return false;
		}

		if (!$this->clearSystemVersion()) {
			trigger_localised_error('SYS0161', E_USER_WARNING, $this->name);
			$GLOBALS['SQ_REVERT_TO_SYSTEM_VERSION'] = false;
			return false;
		}

		// ITS ALIVE... ITS ALIVE...
		$this = $old_version;

		// save this old version to the db
		$this->_updated();

		$GLOBALS['SQ_REVERT_TO_SYSTEM_VERSION'] = false;
		return true;

	}//end revertToSystemVersion()


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
	function _updated($update_parents=true)
	{
		require_once SQ_FUDGE_PATH.'/db_extras/db_extras.inc';

		if (isset($this->_tmp['__creating__']) && $this->_tmp['__creating__']) {
			return true;
		}

		$updated = time();
		$updated_userid  = $GLOBALS['SQ_SYSTEM']->currentUserId();
		$name            = substr($this->_getName(), 0, 255);
		$short_name      = substr($this->_getName(true), 0, 255);

		// if we have an ID hit the DB
		if ($this->id) {

			$GLOBALS['SQ_SYSTEM']->changeDatabaseConnection('db2');
			$db = &$GLOBALS['SQ_SYSTEM']->db;
			$GLOBALS['SQ_SYSTEM']->doTransaction('BEGIN');

			$sql = 'UPDATE
						sq_ast
					SET ';

			if (!empty($name)) {
				$sql .= 'name = '.$db->quoteSmart($name).',';
			}
			if (!empty($short_name)) {
				$sql .= 'short_name = '.$db->quoteSmart($short_name).',';
			}

			$sql .= '
						languages		= '.$db->quoteSmart($this->languages).',
						charset			= '.$db->quoteSmart($this->charset).',
						force_secure	= '.$db->quoteSmart($this->force_secure).',
						updated			= '.db_extras_todate($db, ts_iso8601($updated)).',
						updated_userid	= '.$db->quoteSmart($updated_userid).'
					WHERE
						assetid	= '.$db->quoteSmart($this->id);

			$result = $db->query($sql);
			assert_valid_db_result($result);

			$em = &$GLOBALS['SQ_SYSTEM']->getEventManager();

			$fields = Array(
						'name',
						'short_name',
						'updated',
						'updated_userid',
					  );

			$changed_data = Array();

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

			$em->broadcastEvent($this, 'AssetUpdate', $changed_data);

			// increment the micro version number
			if (!$this->incrementVersion('micro', $update_parents)) {
				trigger_localised_error('SYS0183', E_USER_WARNING, $this->name);
				$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
				$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();
				return false;
			}

			$GLOBALS['SQ_SYSTEM']->doTransaction('COMMIT');
			$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();

		}//end if $this->id

		return true;

	}//end _updated()


	/**
	* Return a name to use to describe this instance of the asset
	*
	* This function should be overridden by children as it is used to set the <i>name</i> and <i>short_name</i>
	* values for this asset when updating.
	*
	* @param boolean	$short_name	whether or not we are after the shortname or the full name
	*
	* @return string
	* @access private
	* @see _updated
	*/
	function _getName($short_name=false)
	{
		return $GLOBALS['SQ_SYSTEM']->am->getTypeInfo($this->type(), 'name').' #'.$this->id;

	}//end _getName()


	/**
	* Get the type of the asset
	*
	* @return string
	* @access public
	*/
	function type()
	{
		return get_class($this);

	}//end type()


	/**
	* Get an array of all assets types that the asset extends from
	*
	* @param boolean	$include_asset	when true adds the "Asset" type to the parent list,
	*									even though it's an unistantiable object
	*
	* @return array
	* @access public
	* @see Asset_Manager::getTypeAncestors()
	*/
	function getTypeAncestors($include_asset=true)
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
	function incrementVersion($number='micro', $update_parents=true)
	{
		if (!$this->id) return false;

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
				return false;
			break;
		}

		// work out the new version number based on the part of the
		// version that we have been asked to update (major|minor|micro)
		$version_parts = explode('.', $this->version);
		for (reset($version_parts); null !== ($k = key($version_parts)); next($version_parts)) {
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
		$db = &$GLOBALS['SQ_SYSTEM']->db;

		$sql = 'UPDATE
					sq_ast
				SET
					version	= '.$db->quoteSmart($new_version).'
				WHERE
					assetid	= '.$db->quoteSmart($this->id);

		$result = $db->query($sql);
		assert_valid_db_result($result);

		if ($update_parents) {
			// increment the micro version of all dependant parents because a 'part' of them
			// has been updated - even though they havnt been updated directly
			$dependant_parents = $GLOBALS['SQ_SYSTEM']->am->getDependantParents($this->id);
			foreach ($dependant_parents as $parentid) {
				$dep_parent = &$GLOBALS['SQ_SYSTEM']->am->getAsset($parentid);
				if (!is_null($dep_parent)) {
					if (!$dep_parent->_updated(false)) {
						$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
						$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();
						$GLOBALS['SQ_SYSTEM']->am->forgetAsset($dep_parent);
						trigger_localised_error('SYS0158', E_USER_WARNING, $this->name, $this->id, $dep_parent->name, $dep_parent->id);
						return false;
					}
				}
				$GLOBALS['SQ_SYSTEM']->am->forgetAsset($dep_parent);
			}
		}

		$em = &$GLOBALS['SQ_SYSTEM']->getEventManager();
		$em->broadcastEvent($this, 'AssetVersionUpdate', Array(
															'old_version'	=> $this->version,
															'new_version'	=> $new_version,
														 )
		);

		$GLOBALS['SQ_SYSTEM']->doTransaction('COMMIT');
		$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();

		$this->version = $new_version;
		return true;

	}//end incrementVersion()


	/**
	* Gets the default languages of the asset as an array
	*
	* @return array
	* @access public
	*/
	function getLanguages()
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
	function setLanguages($languages)
	{
		$set_languages = implode(',', $languages);
		if ($set_languages == $this->languages) return false;

		$old_languages = $this->languages;
		$this->languages = $set_languages;
		if (!$this->_updated()) {
			trigger_localised_error('SYS0172', E_USER_WARNING, $this->name);
			$this->languages = $old_languages;
			return false;
		}

		$em = &$GLOBALS['SQ_SYSTEM']->getEventManager();
		$em->broadcastEvent($this, 'AssetLanguageUpdate', Array(
															'old_languages'	=> $old_languages,
															'new_languages'	=> $this->languages,
														  )
		);
		return true;

	}//end setLanguages()


	/**
	* Set the default character set of the asset
	*
	* @param string	$charset	the character set to set for the asset
	*
	* @return boolean
	* @access public
	*/
	function setCharset($charset)
	{
		if ($charset == $this->charset) return false;
		$old_charset = $this->charset;
		$this->charset = $charset;
		if (!$this->_updated()) {
			trigger_localised_error('SYS0170', E_USER_WARNING, $this->name);
			$this->charset = $old_charset;
			return false;
		}

		$em = &$GLOBALS['SQ_SYSTEM']->getEventManager();
		$em->broadcastEvent($this, 'AssetCharsetUpdate', Array(
															'old_charset'	=> $old_charset,
															'new_charset'	=> $this->charset,
														 )
		);
		return true;

	}//end setCharset()


	/**
	* Set the status of the Force Secure Flag
	*
	* @param boolean	$force_secure	whether to force the use of SSL on URLs
	*
	* @return boolean
	* @access public
	*/
	function setForceSecure($force_secure)
	{
		if (!$this->adminAccess('settings')) {
			trigger_localised_error('SYS0252', E_USER_WARNING);
			return false;
		}

		$force_secure = ($force_secure) ? '1' : '0';
		if ($force_secure == $this->force_secure) return false;
		$old_force_secure = $this->force_secure;
		$this->force_secure = $force_secure;
		if (!$this->_updated()) {
			trigger_localised_error('SYS0169', E_USER_WARNING, $this->name, $this->id);
			$this->force_secure = $old_force_secure;
			return false;
		}

		$em = &$GLOBALS['SQ_SYSTEM']->getEventManager();
		$em->broadcastEvent($this, 'AssetForceSecureUpdate', Array(
																'old_force_secure'	=> $old_force_secure,
																'old_force_secure'	=> $this->force_secure,
															 )
		);
		return true;

	}//end setForceSecure()


	/**
	* Remap existing assetids used to new ones for ourselves and all our dependants
	*
	* @param array	$map	a map of the form Array([old asset id] => [new asset id]);
	*
	* @return boolean
	* @access public
	*/
	function remapAssetids($map)
	{
		$GLOBALS['SQ_SYSTEM']->changeDatabaseConnection('db2');
		$GLOBALS['SQ_SYSTEM']->doTransaction('BEGIN');
		// try and acquire locks on our dependants
		$dependant_links = $GLOBALS['SQ_SYSTEM']->am->getLinks($this->id, SQ_SC_LINK_SIGNIFICANT, '', true, 'major', null, 1);
		if (!empty($dependant_links)) {
			$am = &$GLOBALS['SQ_SYSTEM']->am;
			foreach ($dependant_links as $link) {
				$asset = &$am->getAsset($link['minorid'], $link['minor_type_code']);
				if (!$asset->remapAssetids($map)) {
					$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
					$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();
					return false;
				}
			}
		}

		$GLOBALS['SQ_SYSTEM']->doTransaction('COMMIT');
		$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();
		return true;

	}//end remapAssetids()


	/**
	* Morph the asset to either one of its parent types or one of its children types
	*
	* Only succeeds if $type_code assets are allowed to link to all children that this asset has.
	*
	* @param string	$new_type_code	the new type_code to attempt to cast this asset to
	*
	* @return boolean
	* @access public
	*/
	function morph($new_type_code)
	{
		$new_type_code = strtolower($new_type_code);
		$old_type_code = $this->type();

		if ($this->type() == $new_type_code) return true;

		if (!$GLOBALS['SQ_SYSTEM']->am->installed($new_type_code)) {
			trigger_localised_error('SYS0085', E_USER_WARNING, $new_type_code);
			return false;
		}

		$am = &$GLOBALS['SQ_SYSTEM']->am;

		// open the transaction
		$GLOBALS['SQ_SYSTEM']->changeDatabaseConnection('db2');
		$GLOBALS['SQ_SYSTEM']->doTransaction('BEGIN');
		$db = &$GLOBALS['SQ_SYSTEM']->db;

		// is this new type one of our decendents or ancestors?
		$sql = 'SELECT COUNT(*)
				FROM sq_ast_typ_inhd
				WHERE (inhd_type_code = '.$db->quote($this->type()).'  AND type_code = '.$db->quote($new_type_code).')
				   OR (inhd_type_code = '.$db->quote($new_type_code).' AND type_code = '.$db->quote($this->type()).')';
		$count = $db->getOne($sql);
		assert_valid_db_result($count);
		if ($count == 0) {
			trigger_localised_error('SYS0234', E_USER_WARNING, $this->name, $new_type_code, $this->type());
			$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
			$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();
			return false;
		}

		$am->includeAsset($new_type_code);
		$tmp =& new $new_type_code();

		//// FIRST we need to check that all our links are allowed for the new type ////

		// if there any children links
		if ($num_links = $GLOBALS['SQ_SYSTEM']->am->countLinks($this->id, 'major')) {

			$links = $GLOBALS['SQ_SYSTEM']->am->getLinks($this->id, SQ_SC_LINK_ALL);
			foreach ($links as $link) {
				if ($GLOBALS['SQ_SYSTEM']->am->canLinkToType($tmp, $link['minor_type_code'], $link['link_type'], 0, $link['is_exclusive']) !== true) {
					require_once SQ_INCLUDE_PATH.'/general_occasional.inc';
					trigger_localised_error('SYS0236', E_USER_WARNING, $this->name, $new_type_code, $link['minor_type_code'], link_type_name($link['link_type']), $new_type_code);
					$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
					$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();
					return false;
				}
			}// end foreach

		}// end if child links

		// if there any parent links
		$num_links = $GLOBALS['SQ_SYSTEM']->am->countLinks($this->id, 'minor');
		if ($num_links) {

			$links = $GLOBALS['SQ_SYSTEM']->am->getLinks($this->id, SQ_SC_LINK_ALL, '', true, 'minor');
			foreach ($links as $link) {
				$parent = &$am->getAsset($link['majorid'], $link['major_type_code']);
				if (is_null($parent)) continue;
				if (($err_msg = $GLOBALS['SQ_SYSTEM']->am->canLinkToType($parent, $new_type_code, $link['link_type'], $link['linkid'], $link['is_exclusive'])) !== true) {
					require_once SQ_INCLUDE_PATH.'/general_occasional.inc';
					trigger_localised_error('SYS0235', E_USER_WARNING, $this->name, $new_type_code, $parent->name, $parent->id, $err_msg);
					$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
					$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();
					return false;
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

				$sql = 'UPDATE
							sq_ast_attr_val
						SET
							attrid	= '.$db->quoteSmart($new_id).'
						WHERE
								assetid	= '.$db->quoteSmart($this->id).'
							AND	attrid	= '.$db->quoteSmart($current_id);

				$result = $db->query($sql);
				assert_valid_db_result($result);
			}
		}//end if common var list

		// Get all the vars that aren't available in the new type and delete them
		$deletes_var_list = array_diff($current_var_list, $new_var_list);

		if ($deletes_var_list) {
			$deletes_attributeids = '';
			foreach ($deletes_var_list as $var_name) {
				$deletes_attributeids .= (($deletes_attributeids) ? ',' : '').$db->quoteSmart($this->vars[$var_name]['attrid']);
			}

			$sql = 'DELETE FROM
						sq_ast_attr_val
					WHERE
							assetid	= '.$db->quoteSmart($this->id).'
						AND attrid IN
						(
							'.$deletes_attributeids.'
						)';

			$result = $db->query($sql);
			assert_valid_db_result($result);

		}// end if delete var list

		$sql = 'UPDATE
					sq_ast
				SET
					type_code = '.$db->quote($new_type_code).'
				WHERE
					assetid = '.$db->quote($this->id);

		$result = $db->query($sql);
		assert_valid_db_result($result);

		// now load this new asset into the temporary
		$tmp->load($this->id);
		$all_ok = false;
		if ($tmp->id) {
			// OK if we got this far let's move the directory (if it exists)
			if (is_dir($this->data_path)) {
				require_once SQ_FUDGE_PATH.'/general/file_system.inc';
				// make sure the parent directory exists, then move the our directory to it's new home
				if (create_directory(dirname($tmp->data_path)) && rename($this->data_path, $tmp->data_path)) {
					$all_ok = true;
				}
			} else {
				// if there isn't a directory then everything is fine
				$all_ok = true;
			}

		}// end if

		// before we override ourselves, do any cleaning up that we might need
		if ($all_ok && !$this->_morphCleanup($new_type_code)) {
			$all_ok = false;
		}

		// all is OK so override ourselves with the temporary
		if ($all_ok) {
			$this = $tmp;
			$GLOBALS['SQ_SYSTEM']->doTransaction('COMMIT');
			$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();

			$em = &$GLOBALS['SQ_SYSTEM']->getEventManager();
			$em->broadcastEvent($this, 'AssetTypeUpdate', Array(
															'old_type'	=> $old_type_code,
															'new_type'	=> $new_type_code,
														  )
			);

			return true;

		} else {
			// bugger something went wrong190
			$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
			$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();
			return false;

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
	function _morphCleanup($new_type_code)
	{
		return true;

	}//end _morphCleanup()


	/**
	* Handle a request for available asset-type keywords
	*
	* We set $vars['my_keyword'] = 'my keyword desc' to tell the broadcaster
	* what keywords are available for this asset type.
	*
	* @param object	&$broadcaster	the asset that triggered the event
	* @param array	$vars			the vars that get submitted by the broadcaster
	*								we add keywords to the $vars['keywords'] array
	*
	* @return void
	* @access private
	*/
	function onRequestKeywords(&$broadcaster, $vars=Array())
	{

		if (!isset($vars['keywords'])) return;
		$vars['keywords'] = array_merge($vars['keywords'], $this->_available_keywords);

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
	function canClone()
	{
		return true;

	}//end canClone()


	/**
	* Takes a clone shell and clones the sources required components to the clone shell
	*
	* @param object	&$clone		the clone shell
	* @param array	$components	the wanted components to clone eg.
	* <PRE>
	*                       Array(
	*                         'attributes',
	*                         'metadata_shemas',
	*                         'metadata',
	*                         'workflow',
	*                         'permissions',
	*                         'data',
	*						  'content_tags'
	*                       );
	*                       or alternately
	*                       Array('all');
	* </PRE>
	*
	* @return boolean
	* @access public
	*/
	function cloneComponents(&$clone, $components)
	{
		if (!is_array($components) || empty($components)) {
			return false;
		}
		if (is_null($clone)) return false;

		$GLOBALS['SQ_SYSTEM']->changeDatabaseConnection('db2');
		$GLOBALS['SQ_SYSTEM']->doTransaction('BEGIN');


		  ///////////////////
		 /// PERMISSIONS ///
		///////////////////
		// we need to clone permissions first (if we are cloning permissions) so that the user has
		// access to save attributes, schemas etc later on

		if (in_array('permissions', $components) || in_array('all', $components)) {
			$GLOBALS['SQ_SYSTEM']->setRunLevel(SQ_RUN_LEVEL_FORCED);

			$this->_tmp[__CLASS__.'_in_create_cascading'] = true;
			foreach (Array(SQ_PERMISSION_READ, SQ_PERMISSION_WRITE, SQ_PERMISSION_ADMIN) as $perm) {
				$set_perms = $GLOBALS['SQ_SYSTEM']->am->getPermission($this->id, $perm, null, false, false, true);
				foreach ($set_perms as $userid => $granted) {
					if (!$GLOBALS['SQ_SYSTEM']->am->setPermission($clone->id, $userid, $perm, $granted)) {
						$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
						$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();
						unset($this->_tmp[__CLASS__.'_in_create_cascading']);
						$GLOBALS['SQ_SYSTEM']->restoreRunLevel();
						return false;
					}
				}
			}//end foreach
			unset($this->_tmp[__CLASS__.'_in_create_cascading']);

			$GLOBALS['SQ_SYSTEM']->restoreRunLevel();
		}//end if permissions


		  //////////////////
		 /// ATTRIBUTES ///
		//////////////////

		if (in_array('attributes', $components) || in_array('all', $components)) {
			// Now we set all the attributes
			foreach ($this->vars as $name => $data) {
				if (!$clone->setAttrValue($name, $data['value'])) {
					$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
					$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();
					return false;
				}
			}
			$clone->saveAttributes();
		}//end if attributes


		  ////////////////
		 /// METADATA ///
		////////////////

		if (in_array('metadata_schemas', $components) || in_array('all', $components)) {
			$mm = &$GLOBALS['SQ_SYSTEM']->getMetadataManager();
			// apply the schemas from the cloner to the clonee
			$schemas = $mm->getSchemas($this->id);
			$metadata = Array();
			foreach ($schemas as $schemaid => $granted) {
				if (!$GLOBALS['SQ_SYSTEM']->am->assetExists($schemaid)) {
					continue;
				}
				if (!$mm->setSchema($clone->id, $schemaid, $granted)) {
					$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
					$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();
					return false;
				}
			}//end foreach

			// apply the actual metadata from the cloner to the clonee
			if (in_array('metadata', $components) || in_array('all', $components)) {
				$metadata = $mm->getMetadata($this->id);
				if (!empty($metadata)) {
					if (!$mm->setMetadata($clone->id, $metadata)) {
						$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
						$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();
						return false;
					}
				}
			}


		}//end if metadata


		  ////////////////
		 /// WORKFLOW ///
		////////////////

		if (in_array('workflow', $components) || in_array('all', $components)) {
			$wfm = &$GLOBALS['SQ_SYSTEM']->getWorkflowManager();
			$schemas = $wfm->getSchemas($this->id);
			foreach ($schemas as $schemaid => $granted) {
				if (!$wfm->setSchema($clone->id, $schemaid, $granted)) {
					$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
					$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();
					return false;
				}
			}//end foreach
		}//end if workflow


		  ////////////////
		 /// DATA DIR ///
		////////////////

		if (in_array('data', $components) || in_array('all', $components)) {
			// OK if we got this far let's copy the directory (if it exists)
			if (is_dir($this->data_path)) {
				require_once SQ_FUDGE_PATH.'/general/file_system.inc';
				if (!copy_directory($this->data_path, $clone->data_path)) {
					$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
					$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();
					return false;
				}
			}
		}//end if data


		  ////////////////////
		 /// CONTENT TAGS ///
		////////////////////

		if (in_array('content_tags', $components) || in_array('all', $components)) {
			$tag_manager = &$GLOBALS['SQ_SYSTEM']->getTagManager();
			$current_tag_links = $tag_manager->getTagLinks($this->id);

			foreach ($current_tag_links as $link) {
				$tag_manager->setTag($clone->id, $link['minorid'], $link['value']);
			}
		}//end if content tags

		$GLOBALS['SQ_SYSTEM']->doTransaction('COMMIT');
		$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();
		return true;

	}//end cloneComponents()


//--        STATUS        --//


	/**
	* Returns a status object for the current asset to be used for processing
	*
	* @return object
	* @access public
	*/
	function &getStatus()
	{
		if (!isset($this->_tmp['status_object'])) {
			// work out the name of our status file
			require_once SQ_INCLUDE_PATH.'/general_occasional.inc';
			$status_code = get_bit_names('SQ_STATUS_', $this->status);
			$status_code = 'asset_status_'.strtolower($status_code);
			require_once SQ_INCLUDE_PATH.'/asset_status/'.$status_code.'.inc';
			$this->_tmp['status_object'] =& new $status_code($this);
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
	function getAvailableStatii()
	{
		$status = &$this->getStatus();
		$statii = $status->getAvailableStatii();
		ksort($statii, SORT_NUMERIC);
		return array_reverse($statii, true);

	}//end getAvailableStatii()


	/**
	* Returns a description representing the current status
	*
	* @return string
	* @access public
	*/
	function getStatusDescription()
	{
		$status = &$this->getStatus();
		return $status->getDescription();

	}//end getStatusDescription()


	/**
	* Processes the changes required when a new status is set
	*
	* @param int		$new_status		the new status to set for this asset
	* @param boolean	$update_parents	Specify if we want to update the parent as well
	*
	* @return boolean
	* @access public
	*/
	function processStatusChange($new_status, $update_parents=true)
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
				$dependant_parents = $GLOBALS['SQ_SYSTEM']->am->getLinks($this->id, SQ_SC_LINK_SIGNIFICANT, '', true, 'minor', null, 1);
				if (!empty($dependant_parents)) {
					$am = &$GLOBALS['SQ_SYSTEM']->am;
					foreach ($dependant_parents as $link) {
						$asset = &$am->getAsset($link['majorid'], $link['major_type_code']);
						if ($asset->status > $new_status) {
							// we dont want to rollback, but we dont
							// want to change our status for real
							return true;
						}
					}
				}
			}
		}

		$status = &$this->getStatus();
		if (!$status->processStatusChange($new_status)) {
			return false;
		}

		$GLOBALS['SQ_SYSTEM']->changeDatabaseConnection('db2');
		$GLOBALS['SQ_SYSTEM']->doTransaction('BEGIN');
		require_once SQ_FUDGE_PATH.'/db_extras/db_extras.inc';

		if ($new_status != $this->status) {
			// the status has actually changed to a different one

			$db = &$GLOBALS['SQ_SYSTEM']->db;

			$sql = 'UPDATE
						sq_ast
					SET
						status					= '.$db->quoteSmart($new_status).',
						status_changed			= '.db_extras_todate($db, ts_iso8601(time())).',
						status_changed_userid	= '.$db->quoteSmart($GLOBALS['SQ_SYSTEM']->user->id).'
					WHERE
						assetid = '.$db->quoteSmart($this->id);

			$result = $db->query($sql);
			assert_valid_db_result($result);

			// broadcast event notifying of status change
			$em = &$GLOBALS['SQ_SYSTEM']->getEventManager();
			$data = Array(
						'old_status'	=> $this->status,
						'new_status'	=> $new_status,
					);
			$em->broadcastEvent($this, 'AssetStatusUpdate', $data);

			// send message notifying of status change
			require_once SQ_INCLUDE_PATH.'/general_occasional.inc';
			$ms = &$GLOBALS['SQ_SYSTEM']->getMessagingService();
			$msg_reps = Array(
							'asset_name'	=> $this->name,
							'old_status'	=> get_status_description($this->status),
							'new_status'	=> get_status_description($new_status),
						);
			$message = $ms->newMessage(Array(), 'asset.status', $msg_reps);
			$message->parameters['assetid'] = $this->id;
			$message->send();

			$this->status = $new_status;
			unset($this->_tmp['status_object']);



			$this->_updated($update_parents);

			// if we have just made this asset live, we update its minor version
			if ($new_status == SQ_STATUS_LIVE) {
				if (!$this->incrementVersion('minor', false)) {
					trigger_localised_error('SYS0271', E_USER_WARNING, $this->name);
					$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
					$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();
					return false;
				}
			}

			// fire trigger_event_status_changed
			$event_data = Array(
							'old_status'	=> $old_status,
						  );
			$GLOBALS['SQ_SYSTEM']->broadcastTriggerEvent('trigger_event_status_changed', $this, $event_data);
		}

		$GLOBALS['SQ_SYSTEM']->doTransaction('COMMIT');
		$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();

		return true;

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
	function setDate($date_type, $time=null, $userid=null)
	{
		if (!in_array($date_type, Array('published' ,'created'))) {
			trigger_localised_error('SYS0198', E_USER_WARNING);
			return false;
		}

		if (is_null($userid)) {
			$userid = $GLOBALS['SQ_SYSTEM']->currentUserId();
		}
		if (is_null($time)) {
			if ($date_type == 'created') {
				$time = time();
			} else {
				// a null published date means that the asset has never been published
				// so we also set the userid to null
				$userid = null;
			}
		}

		require_once SQ_FUDGE_PATH.'/general/datetime.inc';
		require_once SQ_FUDGE_PATH.'/db_extras/db_extras.inc';

		// construct a string to represent the current date value
		if ($this->$date_type != null) {
			$old_date_string = readable_datetime($this->$date_type);
			$current_user = $date_type.'_userid';
			if ($this->$current_user != null) {
				$old_date_string .= ' by user #'.$this->$current_user;
			} else {
				$old_date_string .= ' by [unknown user]';
			}
		} else {
			$old_date_string = 'never '.$date_type;
		}

		// construct a string to represent the new date value
		if ($time != null) {
			$new_date_string = readable_datetime($time);
			if ($userid != null) {
				$new_date_string .= ' by user #'.$userid;
			} else {
				$new_date_string .= ' by [unknown user]';
			}
		} else {
			$new_date_string = 'never '.$date_type;
		}

		// if the date and user has not changed, dont process anything
		if ($old_date_string == $new_date_string) return true;

		// begin a transaction
		$GLOBALS['SQ_SYSTEM']->changeDatabaseConnection('db2');
		$GLOBALS['SQ_SYSTEM']->doTransaction('BEGIN');
		$db = &$GLOBALS['SQ_SYSTEM']->db;

		$sql = 'UPDATE
					sq_ast
				SET
					'.$date_type.'			= '.db_extras_todate($db, ts_iso8601($time)).','.
					  $date_type.'_userid	= '.$db->quoteSmart($userid).'
				WHERE
					assetid	= '.$db->quoteSmart($this->id);

		$result = $db->query($sql);
		assert_valid_db_result($result);

		$GLOBALS['SQ_SYSTEM']->doTransaction('COMMIT');
		$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();

		$ms = &$GLOBALS['SQ_SYSTEM']->getMessagingService();
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

		$em = &$GLOBALS['SQ_SYSTEM']->getEventManager();
		$em->broadcastEvent($this, 'AssetUpdate', Array($date_type));


		return true;

	}//end setDate()


//--        LOCKING        --//


	/**
	* Returns a list of lock types available for this asset type
	*
	* @return array
	* @access public
	*/
	function lockTypes()
	{
		$lock_types =  Array(
						'settings'		=> 1,
						'attributes'	=> 2,
						'permissions'	=> 4,
						'links'			=> 8,
						'workflow'		=> 16,
						'metadata'		=> 32,
						'lookups'		=> 64,
						'weights'		=> 128,
					   );

		// a menu lock allows editing of both attributes (such as name etc)
		// and links (such as position in menu)
		$lock_types['menu'] = $lock_types['attributes'] | $lock_types['links'];

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
	function canForceablyAcquireLock($lock_type)
	{
		$current_locks = $GLOBALS['SQ_SYSTEM']->am->getLockInfo($this->id, $lock_type, true, false);
		// lock type not known ? bugger off
		if (empty($current_locks)) return false;

		foreach ($current_locks as $lock) {
			if (empty($lock)) continue;
			$user = null;
			if ($lock['userid']) {
				$user = &$GLOBALS['SQ_SYSTEM']->am->getAsset($lock['userid']);
			}
			// lets work out if the current user has a high
			// enough level of access to forceably acquire the lock
			if (!is_null($user)) {
				// locked by root, no-one can force acquire...
				if ($GLOBALS['SQ_SYSTEM']->userRoot($user)) {
					return false;
				}

				// locked by a sysadmin? only root can force acquire...
				if ($GLOBALS['SQ_SYSTEM']->userSystemAdmin($user) && !$GLOBALS['SQ_SYSTEM']->userRoot()) {
					return false;
				}
			}

			// locked by someone else? need to be a system admin to acquire this lock
			if (!$GLOBALS['SQ_SYSTEM']->userRoot() && !$GLOBALS['SQ_SYSTEM']->userSystemAdmin()) {
				return false;
			}

		}//end foreach
		return true;

	}//end canForceablyAcquireLock()


	/**
	* Returns any locks that are needed for editing, based on a set of custom keywords
	*
	* @param array	$keywords	the keywords to govern what locks are required
	*
	* @return array
	* @access public
	*/
	function getEditingLocks($keywords)
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
	*   exclusive   = true|false
	*
	* @return array
	* @access private
	*/
	function _getAllowedLinks()
	{
		return Array(
				SQ_LINK_TYPE_1	=> Array(),
				SQ_LINK_TYPE_2	=> Array(),
				SQ_LINK_TYPE_3	=> Array(),
				SQ_LINK_NOTICE	=> Array(
									'image'		=> Array('card' => 1, 'exclusive' => false),
									'design'	=> Array('card' => 'M', 'exclusive' => false),
								   ),
			   );

	}//end _getAllowedLinks()


	/**
	* Creates a link between this asset and the passed asset, of a certain type
	* and returns the linkid of the created link
	*
	* @param object		&$minor		the minor asset that we are linking to
	* @param string		$link_type	the type of link this is
	* @param string		$value		the value that is to be associated with this link
	* @param string		$sort_order	the position in the links list that this link should take,
	*								if null or less than zero places at end of list
	* @param string		$dependant	'0' / '1' on whether the this asset is dependant on the
	*								asset that will be linked by this new link
	* @param string		$exclusive	'0' / '1' on whether the this asset is linked exclusivly
	*								to the asset that will be linked by this new link
	* @param boolean	$moving		moving
	*
	* @return int
	* @access public
	* @see Asset_Manager::createAssetLink()
	*/
	function createLink(&$minor, $link_type, $value='', $sort_order=null, $dependant='0', $exclusive='0', $moving=false)
	{
		if (!$this->id) return 0;
		return $GLOBALS['SQ_SYSTEM']->am->createAssetLink($this, $minor, $link_type, $value, $sort_order, $dependant, $exclusive, $moving);

	}//end createLink()


	/**
	* Prepares for linking by checking that the link values are valid for this link
	*
	* This function will be called if this asset is the major or minor party in the link, so the side_of_link flag
	* should be checked to work out what side of the link this asset is on. The return value should indicate
	* if any of the link fields were changed.
	*
	* @param object	&$asset			the major or minor asset that we are linking to
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
	function prepareLink(&$asset, $side_of_link, &$link_type, &$value, &$sort_order, &$dependant, &$exclusive)
	{
		return false;

	}//end prepareLink()


	/**
	* Returns TRUE if the passed minor asset can be linked to this asset, or a string with the error msg
	*
	* @param object	&$minor		the minor asset that we are linking to
	* @param string	$link_type	the type of link this is
	* @param int	$exclusive	the exclusive status of the link (gets passed to canLinkToType)
	*
	* @return mixed boolean|string
	* @access public
	*/
	function canCreateLink(&$minor, $link_type, $exclusive)
	{
		if (!$this->id) return 'Asset Not Created';
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
	* @param object	&$minor		the minor asset that we are linking to
	* @param object	&$old_major	the major asset that we are being moved from
	* @param string	$link_type	the type of link this is
	*
	* @return mixed boolean|string
	* @access public
	*/
	function canMoveLink(&$minor, &$old_major, $link_type)
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
	function describeLink($linkid)
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
	function isDeletableLink($linkid)
	{
		return true;

	}//end isDeletableLink()


	/**
	* Determine if we can delete the passed link right now
	*
	* This function returns TRUE or error message
	*
	* @param int	$linkid	the link id of the link to remove
	*
	* @return mixed boolean|string
	* @access public
	*/
	function canDeleteLink($linkid)
	{
		if (!$this->id) return 'Asset Not Created';
		if (($err_msg = $this->isDeletableLink($linkid)) !== true) {
			return $err_msg;
		}
		if (!$this->writeAccess('links')) {
			return 'Cannot delete link, permission denied';
		}
		return true;

	}//end canDeleteLink()


	/**
	* Determine if we COULD delete the passed link IF we acquired the lock on this asset
	*
	* This function return TRUE or error message
	*
	* @param int	$linkid	the link id of the link to remove
	*
	* @return mixed boolean|string
	* @access public
	*/
	function couldDeleteLink($linkid)
	{
		if (!$this->id) return 'Asset Not Created';
		if (($err_msg = $this->isDeletableLink($linkid)) !== true) {
			return $err_msg;
		}
		if (!$this->writeAccess()) {
			return 'Cannot delete link, permission denied';
		}
		return true;

	}//end couldDeleteLink()


	/**
	* Remove a link where this asset is the major player
	*
	* @param int		$linkid	the link id of the link to remove
	* @param boolean	$moving	TRUE if this is part of a move operation
	*
	* @return boolean
	* @access public
	* @see Asset_Manager::deleteAssetLink()
	*/
	function deleteLink($linkid, $moving=false)
	{
		$link = $GLOBALS['SQ_SYSTEM']->am->getLinkById($linkid, $this->id);
		if (empty($link)) {
			trigger_localised_error('SYS0243', E_USER_NOTICE, $linkid);
			return false;
		} else {
			return $GLOBALS['SQ_SYSTEM']->am->deleteAssetLink($linkid, $moving);
		}

	}//end deleteLink()


	/**
	* Allow any required processesing to occur when any link is updated for the asset
	*
	* Called whenever any type of link is changed. Allows caching of objects, links and other stuff
	* to occur but not cause integrity issues during script execution.
	* ONLY TO BE CALLED FROM THE ASSET MANAGER
	*
	* @return boolean
	* @access protected
	*/
	function linksUpdated()
	{
		return true;

	}//end linksUpdated()


//--        ATTRIBUTES        --//


	/**
	* Save attributes set with setAttrValue to the database
	*
	* @param boolean	$dont_run_updated	if true this->_updated() won't be run
	*										(THIS IS ONLY EVER CALLED FROM Asset::create())
	*
	* @return boolean
	* @access public
	* @see setAttrValue()
	*/
	function saveAttributes($dont_run_updated=false)
	{
		if (!$this->id) return true;
		if (empty($this->_tmp['vars_set'])) return true;
		if (!$GLOBALS['SQ_REVERT_TO_SYSTEM_VERSION'] && !$this->writeAccess('attributes')) {
			trigger_localised_error('CORE0121', E_USER_WARNING, $this->name, $this->id);
			return false;
		}

		$save_vars = Array();
		$attr_ids  = Array();

		$db = &$GLOBALS['SQ_SYSTEM']->db;

		// open a queue for all messages we are going to be logging
		$ms = &$GLOBALS['SQ_SYSTEM']->getMessagingService();
		$ms->openLog();

		$changed_array = Array();
		foreach ($this->_tmp['vars_set'] as $var_name => $var_data) {
			$attr_id = $this->vars[$var_name]['attrid'];

			// if this is a unique attribute then let's make sure that this isn't already set
			$owning_attributeid = 0;
			$attribute = &$this->getAttribute($var_name);
			if ($attribute->uniq) {

				$sql = 'SELECT oa.attrid
						FROM sq_ast_attr a
						  INNER JOIN sq_ast_attr oa ON (a.owning_type_code = oa.type_code AND a.name = oa.name)
						WHERE a.name      = '.$db->quote($var_name).'
						  AND a.type_code = '.$db->quote($this->type());

				$owning_attributeid = $db->getOne($sql);
				assert_valid_db_result($owning_attributeid);

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

			// add attribute to the event broadcaster array (for notification of what has changed)
			// this allows event listener to act only on what has changed

			$changed_array[] = $attribute->name;
		}

		// open the transaction
		$GLOBALS['SQ_SYSTEM']->changeDatabaseConnection('db2');
		$GLOBALS['SQ_SYSTEM']->doTransaction('BEGIN');
		$db =& $GLOBALS['SQ_SYSTEM']->db;

		// find any previous entries
		$atrr_id_cond = 'IN ('.implode(', ', $attr_ids).')';
		$sql   = 'SELECT attrid FROM '.SQ_TABLE_RUNNING_PREFIX.'ast_attr_val ';
		$where = 'assetid = '.$db->quote($this->id).'
					AND attrid '.$atrr_id_cond;
		$where = $GLOBALS['SQ_SYSTEM']->constructRollbackWhereClause($where);

		$existing = $db->getCol($sql.$where);
		assert_valid_db_result($existing);
		$new = array_diff($attr_ids, $existing);


		// update existing custom attribute values
		if (!empty($existing)) {

			$prepared = $db->prepare('UPDATE sq_ast_attr_val SET custom_val = ? WHERE attrid = ? AND assetid = '.$db->quote($this->id));
			assert_valid_db_result($prepared);

			$prepared_uniq = $db->prepare('UPDATE sq_ast_attr_uniq_val SET custom_val = ? WHERE owning_attrid = ? AND assetid = '.$db->quote($this->id));
			assert_valid_db_result($prepared_uniq);

			foreach ($existing as $attr_id) {
				$attr_id = (int) $attr_id;
				$attr_value = $save_vars[$attr_id]['value'];

				// if this is a unique attribute then let's make sure we register that in the unique value table
				$attribute = &$this->getAttribute($save_vars[$attr_id]['name']);
				if ($attribute->uniq) {
					// update existing entry
					$result = $db->execute($prepared_uniq, Array($attr_value, $save_vars[$attr_id]['owning_attrid']));
					assert_valid_db_result($result);
				}

				$result = $db->execute($prepared, Array($attr_value, $attr_id));
				assert_valid_db_result($result);

			}//end foreach

			$db->freePrepared($prepared);
			$db->freePrepared($prepared_uniq);

		}//end updating existing custom attribute values

		// insert new custom attribute values
		if (!empty($new)) {

			$prepared = $db->prepare('INSERT INTO sq_ast_attr_val (attrid, custom_val, assetid) VALUES (?, ?, '.$db->quote($this->id).')');
			assert_valid_db_result($prepared);

			$prepared_uniq = $db->prepare('INSERT INTO sq_ast_attr_uniq_val (owning_attrid, custom_val, assetid) VALUES (?, ?, '.$db->quote($this->id).')');
			assert_valid_db_result($prepared_uniq);

			foreach ($new as $attr_id) {
				$attr_id = (int) $attr_id;
				$attr_value = $save_vars[$attr_id]['value'];

				// if this is a unique attribute then let's make sure we register that in the unique value table
				$attribute = &$this->getAttribute($save_vars[$attr_id]['name']);
				if ($attribute->uniq) {
					$result = $db->execute($prepared_uniq, Array($save_vars[$attr_id]['owning_attrid'], $attr_value));
					assert_valid_db_result($result);
				}

				$result = $db->execute($prepared, Array($attr_id, $attr_value));
				assert_valid_db_result($result);

			}//end foreach

			$db->freePrepared($prepared);
			$db->freePrepared($prepared_uniq);

		}//end new custom attribute values

		// tell, the asset it has updated
		if (!$dont_run_updated && !$this->_updated()) {
			$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
			$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();
			$ms->abortLog();
			return false;
		}

		unset($this->_tmp['vars_set']);

		// if we get this far, then it's all OK
		$GLOBALS['SQ_SYSTEM']->doTransaction('COMMIT');
		$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();
		$ms->closeLog();

		// notify anyone interested that attributes changed
		$em =& $GLOBALS['SQ_SYSTEM']->getEventManager();
		$em->broadcastEvent($this, 'attributeChange', $changed_array);
		$GLOBALS['SQ_SYSTEM']->broadcastTriggerEvent('trigger_event_attributes_saved', $this);

		return true;

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
	function setAttrValue($name, $value)
	{
		if (empty($this->vars[$name])) {
			trigger_localised_error('SYS0093', E_USER_WARNING, $name, $this->type());
			return false;
		}
		$attribute = &$this->getAttribute($name);
		if (!$attribute->setValue($value)) {
			trigger_localised_error('SYS0073', E_USER_WARNING, $value, $name);
			$attribute->setValue($this->vars[$name]['value']);
			return false;
		}

		if ($this->vars[$name]['type'] == 'serialise') {
			$value = unserialize($value);
		}

		// if the value being set is the same as the current value - dont do anything
		if ($this->vars[$name]['value'] === $value) {
			return true;
		}

		// if this is a uniq attribute then let's make sure that this isn't already set
		if ($attribute->uniq) {
			$db = &$GLOBALS['SQ_SYSTEM']->db;

			$sql = 'SELECT COUNT(*)
					  FROM '.SQ_TABLE_RUNNING_PREFIX.'ast_attr_uniq_val uv
						INNER JOIN '.SQ_TABLE_RUNNING_PREFIX.'ast_attr oa ON uv.owning_attrid = oa.attrid
						INNER JOIN sq_ast_attr a ON (oa.type_code = a.owning_type_code AND oa.name = a.name)
					  ';
			$where = ' uv.custom_val = '.$db->quote($value).'
						AND a.name = '.$db->quote($name).'
						AND a.type_code = '.$db->quote($this->type());
			$where = $GLOBALS['SQ_SYSTEM']->constructRollbackWhereClause($where, 'uv');

			$result = $db->getOne($sql.$where);
			assert_valid_db_result($result);

			if ((int) $result > 0) {
				trigger_localised_error('SYS0251', E_USER_WARNING, $name, $value);
				$attribute->setValue($this->vars[$name]['value']);
				return false;
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

		return true;

	}//end setAttrValue()


	/**
	* Get the value of an attribute
	*
	* @param string	$name	the name of the attribute whose value you want to get
	*
	* @return mixed string|null
	* @access public
	*/
	function attr($name)
	{
		if (!isset($this->vars[$name])) {
			trigger_localised_error('SYS0092', E_USER_WARNING, $name, $this->name, $this->id);
			return null;
		}
		return $this->vars[$name]['value'];

	}//end attr()


	/**
	* Get the value of an attribute by reference
	*
	* @param string	$name	the name of the attribute whose value you want to get
	*
	* @return mixed object|null
	* @access public
	*/
	function &attrByRef($name)
	{
		if (!isset($this->vars[$name])) {
			trigger_localised_error('SYS0092', E_USER_WARNING, $name, $this->name, $this->id);
			return null;
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
	* @return mixed object|null
	* @access public
	*/
	function &getAttribute($name, $mute_errors=false)
	{
		if (!isset($this->_tmp['attributes'][$name]) || !is_object($this->_tmp['attributes'][$name])) {

			if (empty($this->_tmp['attributes'])) {
				$this->_tmp['attributes'] = Array();
			}

			if (empty($this->vars[$name])) {
				if (!$mute_errors) {
					trigger_localised_error('SYS0094', E_USER_WARNING, $name, $this->type());
				}

				$this->_tmp['attributes'][$name] = null;
			} else {

				require_once SQ_ATTRIBUTES_PATH.'/'.$this->vars[$name]['type'].'/'.$this->vars[$name]['type'].'.inc';
				$attr_class = 'Asset_Attribute_'.$this->vars[$name]['type'];
				$this->_tmp['attributes'][$name] =& new $attr_class($this->vars[$name]['attrid'], $this->vars[$name]['value']);
				// make sure of the asset is correct
				if (empty($this->_tmp['attributes'][$name]->id)) {
					$this->_tmp['attributes'][$name] = null;
				}

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
	function getAssetKeywords($descriptions=false)
	{

		$keywords = Array();
		$default_keywords = Array(
								'asset_name',
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
							);

		$mm = &$GLOBALS['SQ_SYSTEM']->getMetadataManager();
		$mm_keywords = $mm->generateKeywordReplacements($this, $default_keywords, false);
		foreach ($default_keywords as $keyword) {
			$keywords[$keyword]['value'] = $mm_keywords[$keyword];
		}

		foreach ($this->vars as $name => $info) {
			if ($info['type'] == 'serialise') continue;
			$keywords['asset_attribute_'.$name]['value'] = $info['value'];

			if ($descriptions) {
				$attr = &$this->getAttribute($name);
				$keywords['asset_attribute_'.$name]['description'] = $attr->description;
			}
		}

		$keywords['asset_assetid']['value'] = $this->id;

		if ($descriptions) {
			$keywords['asset_assetid']['description']                     = 'The ID of the asset';
			$keywords['asset_name']['description']                        = 'Full name of the asset';
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
		}

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
	function getAvailableKeywords()
	{

		$keywords = Array();
		$default_keywords = Array(
								'asset_assetid',
								'asset_name',
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
								'asset_status_description',
								'asset_status_colour',
							);

		// add default keywords first
		foreach ($default_keywords as $keyword) {
			$keywords[$keyword] = translate($keyword);
		}

		// add attribute
		foreach ($this->vars as $name => $info) {
			$attr = &$this->getAttribute($name);
			if (!$attr->is_admin) {
				$keywords['asset_attribute_'.$name] = empty($attr->description)?'Asset Attribute: '.$name:$attr->description;
			}
		}

		return $keywords;

	}//end getAvailableKeywords()


	/**
	* Get bodycopy keyword replacement
	*
	* Provides a mechanism for providing keyword replacement functions by
	* looking up the function named 'getKeywordNameKeywordReplacement'
	* so an asset wishing to add support for 'best_ever' custom keyword
	* should implement a function getBestEverKeywordReplacement()
	* which returns a replacement string
	*
	* @param string	$keyword	Keyword to find a replacement for
	*
	* @return string
	* @access public
	*/
	function getKeywordReplacement($keyword)
	{
		if (empty($keyword)) return '';

		$replacement = "%$keyword%";
		$tmp_keyword = strtr($keyword, '_', ' ');
		$tmp_keyword = ucwords($tmp_keyword);
		$tmp_keyword = preg_replace('/\s+/', '', $tmp_keyword);
		$func_name = 'get'.$tmp_keyword.'KeywordReplacement';
		if (method_exists($this, $func_name)) {
			$replacement = $this->$func_name();
		} else if (strcmp('asset_assetid', $keyword) == 0) {
			$replacement = $this->id;
		} else if (0 === strpos($keyword, 'asset_attribute_')) {
			$attr_name = substr($keyword, strlen('asset_attribute_'));
			preg_match('/(_([^_]*)){0,1}$/', $attr_name, $matches);
			if (array_key_exists($attr_name, $this->vars)) {
				$attr = &$this->getAttribute($attr_name);
				if (!$attr->is_admin) {
					$replacement = $this->attr($attr_name);
				}
			} else if (!empty($matches[1])) {
				// we might have a datetime attribute with a date formatting suffix (readable, iso8601 etc)
				$date_formats = Array(
									'short'				=> 'Y-m-d',
									'readable'			=> 'd M Y g:ia',
									'readabledate'		=> 'd M Y',
									'readabletime'		=> 'g:ia',
									'iso8601'			=> 'Y-m-d\TH:i:s',
									'rfc2822'			=> 'r',
									'rfc2822-dateonly'	=> 'D, d M Y',
									'ical'				=> 'Ymd\THis',
								);

				$suffix = $matches[1];
				$date_format = $matches[2];
				$attr_name_cropped = substr($attr_name, 0, (strlen($attr_name) - strlen($matches[1])));
				if (array_key_exists($attr_name_cropped, $this->vars)) {
					$attr = &$this->getAttribute($attr_name_cropped);
					if (!$attr->is_admin && ($attr->type() == 'datetime')) {
						if (array_key_exists($date_format, $date_formats)) {
							$attr_value = $this->attr($attr_name_cropped);
							$format_value = (empty($attr_value)) ? '' : date($date_formats[$date_format], iso8601_ts($attr_value));
							$replacement = $format_value;
						}
					}
				}
			}
		} else if (substr($keyword, 0, 15) == 'asset_metadata_') {
			$metadata_keyword = substr($keyword, 15);
			$mm = &$GLOBALS['SQ_SYSTEM']->getMetadataManager();
			$mm_replacements = $mm->getMetadataFieldValues($this->id, Array($metadata_keyword));
			if (isset($mm_replacements[$metadata_keyword])) {
				$replacement = $mm_replacements[$metadata_keyword];
			} else {
				$replacement = '';
			}
		} else {
			$mm = &$GLOBALS['SQ_SYSTEM']->getMetadataManager();
			$mm_keyword = $mm->generateKeywordReplacements($this, Array($keyword), false);
			if (isset($mm_keyword[$keyword])) {
				$replacement = $mm_keyword[$keyword];
			}
		}
		return $replacement;

	}//end getKeywordReplacement()


	/**
	* Gets replacements for keywords that are specific to this asset for LIMBO
	*
	* @param array	$keywords	the keywords to get replacements for
	*
	* @return array
	* @access public
	*/
	function getCustomKeywordReplacements($keywords=Array())
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
	function processCustomKeywords($keywords=Array())
	{

		return true;

	}//end processCustomKeywords()


	/**
	* Returns the editable content of this asset
	*
	* @return string
	* @access public
	*/
	function getContent()
	{
		return '';

	}//end getContent()


	/**
	* Sets the editable content of this asset
	*
	* @return boolean
	* @access public
	*/
	function setContent()
	{
		return false;

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
	function getEffectiveLastUpdatedTime($assetids)
	{
		$assetids[] = $this->id; // add self

		// do not include shadow assets
		foreach ($assetids as $key => $assetid) {
			$id_parts = explode(':', $assetid);
			if (isset($id_parts[1])) {
				unset($assetids[$key]);
			}
		}

		// get all updated time from DB
		$db =& $GLOBALS['SQ_SYSTEM']->db;
		$sql = 'SELECT assetid, updated
				FROM '.SQ_TABLE_RUNNING_PREFIX.'ast
				WHERE assetid in ('.implode(',', $assetids).')
				ORDER BY updated DESC';
		$result =& $db->getAssoc($sql);
		assert_valid_db_result($result);

		// get the effective (most recent) updated time
		//echo key($result);
		$most_recent = array_shift($result);
		return (!is_null($most_recent)) ? strtotime($most_recent) : 0;

	}//end getEffectiveLastUpdatedTime()


//--        PERMISSIONS/ACCESS        --//


	/**
	* Returns true if at least one of the passed assetids has read access to this asset
	*
	* @param array	$assetids	An array of user_group and/or user ids to check for read access
	*							If the array is empty, the current user will be checked
	*
	* @return boolean
	* @access public
	*/
	function readAccess($assetids=Array())
	{
		if (!$this->id) return true;

		// if the asset is in the middle of workflow and the current user
		// can approve the asset, we really need to give them read access to it
		$wfm = &$GLOBALS['SQ_SYSTEM']->getWorkflowManager();
		$publishers = $wfm->whoCanPublish($this->id);
		if (!empty($publishers) && in_array($GLOBALS['SQ_SYSTEM']->currentUserId(), $publishers)) {
			return true;
		}

		// if asset is not live we really need to check for write access
		if ($this->status < SQ_STATUS_LIVE) {
			// if we are in rollback view mode, bypass extra write access checks or we
			// we always be returning false here (writeAccess returns false)
			if (SQ_ROLLBACK_VIEW) {
				return $this->_checkPermissionAccess(SQ_PERMISSION_WRITE, $assetids, false);
			}
			return $this->writeAccess('', $assetids, false);
		}

		return $this->_checkPermissionAccess(SQ_PERMISSION_READ, $assetids);

	}//end readAccess()


	/**
	* Returns true if at least one of the passed assetids has write access to this asset
	*
	* The extra parameter, $only_workflow, is currently only be used in
	* determining read access for an asset before it goes live.
	*
	* @param string		$lock_type		the type of write access required - if blank returns a plain check against
	*									permissions set for this asset, otherwise returns effective access based
	*									on locking and status
	* @param array		$assetids		An array of user_group and/or user ids to check for write access
	*									If the array is empty, the current user will be checked
	* @param boolean	$only_workflow	If true (default), users with write access normally will be denied write access
	*									if outside the current step of the workflow. If false, both those in the workflow
	*									and those with normal write access will be granted write access.
	*
	* @return boolean
	* @access public
	*/
	function writeAccess($lock_type='', $assetids=Array(), $only_workflow=true)
	{
		if (SQ_ROLLBACK_VIEW) return false;

		if (!$this->id) return true;
		if (!$GLOBALS['SQ_SYSTEM']->runLevelEnables(SQ_SECURITY_PERMISSIONS)) {
			return true;
		}

		if ($this->status == SQ_STATUS_LIVE) {
			return $this->liveEditAccess($lock_type);
		}
		return $this->checkAccess(SQ_PERMISSION_WRITE, $lock_type, $assetids, $only_workflow);

	}//end writeAccess()


	/**
	* Returns true if at least one of the passed assetids has admin access to this asset
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
	function adminAccess($lock_type='', $assetids=Array())
	{
		if (SQ_ROLLBACK_VIEW) return false;

		if (!$this->id || !empty($this->_tmp[__CLASS__.'_in_create_cascading'])) {
			return true;
		}
		if (!$GLOBALS['SQ_SYSTEM']->runLevelEnables(SQ_SECURITY_PERMISSIONS)) {
			return true;
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
	function backendAccess()
	{
		return is_a($GLOBALS['SQ_SYSTEM']->user, 'backend_user');

	}//end backendAccess()


	/**
	* Returns true if the current user can live edit this asset
	* Live edit means the asset is currently SQ_STATUS_LIVE but this
	* user can edit the asset without going through a safe edit
	* NOTE: will return false if asset is not currently live
	*
	* @param string	$lock_type	the type of lock
	*
	* @return boolean
	* @access public
	* @see canliveEdit()
	*/
	function liveEditAccess($lock_type)
	{
		if (empty($lock_type) || $this->canLiveEdit($lock_type)) {
			return $this->checkAccess(SQ_PERMISSION_WRITE, $lock_type);
		}
		return false;

	}//end liveEditAccess()


	/**
	* Returns true if the current user could live edit this asset if they had the lock
	*
	* Live edit means the asset is currently SQ_STATUS_LIVE but this
	* user can edit the asset without going through a safe edit
	* NOTE: will return false if asset is not currently live
	*
	* @param int	$lock_type	the type of lock
	*
	* @return boolean
	* @access public
	*/
	function canliveEdit($lock_type)
	{
		if (SQ_ROLLBACK_VIEW) return false;
		if (!$this->id) return true;
		if ($this->status != SQ_STATUS_LIVE) return true;

		if (!empty($lock_type)) {
			if ($lock_type != 'attributes') {
				$locks = $GLOBALS['SQ_SYSTEM']->am->getLockInfo($this->id, $lock_type, true);
				// if we are getting a lock type that cant be safe edited (eg permissions), you can live edit
				// if you dont have the attributes lock also (which must be safe edited)
				if (!in_array('attributes', array_keys($locks))) {
					return true;
				}
			}
		}

		$wfm = &$GLOBALS['SQ_SYSTEM']->getWorkflowManager();
		$schemas = $wfm->getSchemas($this->id, true);

		if (empty($schemas)) return true;

		// if there is workflow defined for this asset, the current user
		// must be the only concerned user to be able to live edit
		$wf_complete = $wfm->testPublish($this->id, $GLOBALS['SQ_SYSTEM']->currentUserId());
		if ($wf_complete) return true;

		return false;

	}//end canliveEdit()


	/**
	* Returns true if this asset is effectivly readable by everyone
	*
	* @return boolean
	* @access public
	*/
	function effectiveUnrestricted()
	{
		// This is not readAccess() because if this is false we don't refer to the writeAccess() setting
		$public_userid = $GLOBALS['SQ_SYSTEM']->am->getSystemAssetid('public_user');
		return ($this->status >= SQ_STATUS_LIVE && $this->_checkPermissionAccess(SQ_PERMISSION_READ, Array($public_userid)));

	}//end effectiveUnrestricted()


	/**
	* Returns true if at least one of the passed assetids has the passed permission
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
	function checkAccess($perm, $lock_type, $assetids=Array(), $only_workflow=true)
	{
		if ($perm == SQ_PERMISSION_READ) {
			trigger_localised_error('SYS0172', E_USER_ERROR, __CLASS__, __FUNCTION__);
		}
		if ($lock_type == '') {
			return $this->_checkPermissionAccess($perm, $assetids, $only_workflow);

		} else {
			// effective access is only valid for the current user
			if (!empty($assetids)) return false;

			$locks = $GLOBALS['SQ_SYSTEM']->am->getLockInfo($this->id, $lock_type, true);
			// if there is no lock or
			if (empty($locks)) return false;

			foreach ($locks as $lock) {
				// if any one of these locks is not owned by the current user they can't edit
				if (empty($lock) || $lock['userid'] != $GLOBALS['SQ_SYSTEM']->currentUserId()) {
					return false;
				}
			}

			// if we are in a state of total approval no editing allowed
			if ($this->status & SQ_SC_STATUS_ALL_APPROVED) {
				return false;
			}

			// if the asset is archived no editing allowed
			if ($this->status & SQ_STATUS_ARCHIVED) {
				return false;
			}

			// otherwise they can edit if they have permission access
			return $this->_checkPermissionAccess($perm, Array(), $only_workflow);

		}// end if

	}//end checkAccess()


	/**
	* Returns true if at least one of the passed assetids has the passed permission
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
	function _checkPermissionAccess($perm, $assetids=Array(), $only_workflow=true)
	{
		if (empty($assetids)) {

			if (!$GLOBALS['SQ_SYSTEM']->userPublic()) {
				// if we are the root user, we can do anything
				if ($GLOBALS['SQ_SYSTEM']->userRoot()) return true;

				// if we are a system administrator, we can do anything
				// unless an asset overwrites this function
				if ($GLOBALS['SQ_SYSTEM']->userSystemAdmin()) {
					return true;
				}
			}

			// get the current user and groups we are not restricted from
			$user = &$GLOBALS['SQ_SYSTEM']->am->getAsset($GLOBALS['SQ_SYSTEM']->currentUserId());
			$assetids = $user->getUserGroups();
			$GLOBALS['SQ_SYSTEM']->am->forgetAsset($user);

			$assetids[] = $GLOBALS['SQ_SYSTEM']->currentUserId();
		}

		if (!$GLOBALS['SQ_PURGING_TRASH']) {
			// if we are in workflow no editing allowed
			// except by those users who can currently approve the asset
			if ($this->status & SQ_SC_STATUS_PENDING) {
				if ($perm == SQ_PERMISSION_WRITE) {
					$wfm = &$GLOBALS['SQ_SYSTEM']->getWorkflowManager();
					$publishers = $wfm->whoCanPublish($this->id);
					if (in_array($GLOBALS['SQ_SYSTEM']->currentUserId(), $publishers)) {
						return true;
					}

					// Those with admin permissions are now allowed to have a
					// crack at write permission throughout the workflow
					if ($this->_checkPermissionAccess(SQ_PERMISSION_ADMIN, $assetids)) {
						return true;
					}

					if ($only_workflow) return false;
				}
			}
		}

		$grants  = $GLOBALS['SQ_SYSTEM']->am->getPermission($this->id, $perm, true);
		$revokes = $GLOBALS['SQ_SYSTEM']->am->getPermission($this->id, $perm, false, false);
		// has their access been revoked at all?
		if (!empty($revokes)) {
			$revoked = array_intersect($assetids, $revokes);
			if (!empty($revoked)) return false;
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
	* It returns true if everything is OK.
	*
	* @return boolean
	* @access public
	*/
	function permissionsUpdated()
	{
		return true;

	}//end permissionsUpdated()


//--        PAINTING        --//


	/**
	* Prints out the Frontend for this asset
	* it's up to the kids to override
	*
	* @return void
	* @access public
	*/
	function printFrontend()
	{
		$this->printFrontendAsset($this);

	}//end printFrontend()


	/**
	* Paint the passed asset on the frontend
	* Override printFrontend() if you wish to pass a different asset to print
	*
	* @param object	&$asset	the asset to print in frontend
	* @param object	$design	the design to print the asset with
	*
	* @return void
	* @access public
	*/
	function printFrontendAsset(&$asset, $design=null)
	{
		if (!headers_sent()) {
			if (!$asset->readAccess()) {
				$GLOBALS['SQ_SYSTEM']->paintLogin(translate('login'), translate('cannot_access_asset', $asset->name));
				return;
			}

			if ($asset->charset) {
				header('Content-type: text/html; charset='.$asset->charset);
			} else {
				header('Content-type: text/html; charset='.SQ_CONF_DEFAULT_CHARACTER_SET);
			}
		}

		$original_asset = null;
		if ($asset->id != $GLOBALS['SQ_SYSTEM']->frontend_asset->id) {
			$original_asset = $GLOBALS['SQ_SYSTEM']->frontend_asset;
			$GLOBALS['SQ_SYSTEM']->frontend_asset = &$asset;
		}

		if ($asset->languages) {
			header('Content-language: '.$asset->languages);
		}

		$db = &$GLOBALS['SQ_SYSTEM']->db;
		$url = strip_url(current_url(false, true));

		//No design has been passed in, so we need to find one
		if (is_null($design)) {
			// Have they set a design name that they want to view ??
			if (!empty($_GET['SQ_DESIGN_NAME'])) {
				$design_name = 'design::user::'.$_GET['SQ_DESIGN_NAME'];
			} else if (!empty($_SESSION['SQ_DESIGN_NAME'])) {
				$design_name = 'design::user::'.$_SESSION['SQ_DESIGN_NAME'];
			} else {
				$design_name = 'design::system::frontend';
			}

			$result = $GLOBALS['SQ_SYSTEM']->am->getDesignFromURL($url, $design_name);

			// if no design was found, try and get the default frontend
			// design if we were not already trying to get it
			if (empty($result) && $design_name != 'design::system::frontend') {
				$result = $GLOBALS['SQ_SYSTEM']->am->getDesignFromURL($url, 'design::system::frontend');
			}

			// if no design was found, try using the asset's URL from the DB
			if (empty($result)) {
				$url = strip_url($asset->getUrl(), true);
				$result = $GLOBALS['SQ_SYSTEM']->am->getDesignFromURL($url, $design_name);
				if (empty($result) && $design_name != 'design::system::frontend') {
					$result = $GLOBALS['SQ_SYSTEM']->am->getDesignFromURL($url, 'design::system::frontend');
				}
			}
		} else {
			$result = true;
		}

		if (!is_null($design) || $result) {
			if (is_null($design)) {
				// we have found the design to use
				$design = &$GLOBALS['SQ_SYSTEM']->am->getAsset($result['designid'], $result['type_code']);
			}
			$design->paint($asset);
		} else {
			// we can't find a design, oh well let's just print out our body
			$asset->printBody();
		}

		if (!is_null($original_asset)) {
			$GLOBALS['SQ_SYSTEM']->frontend_asset = $original_asset;
		}

	}//end printFrontendAsset()


	/**
	* Called by the design to print extra material in the head of the HTML document for this asset
	* it's up to the kids to override
	* TODO: empty function
	*
	* @return void
	* @access public
	*/
	function printHead()
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
	function printBody()
	{

	}//end printBody()


	/**
	* Called by the design to print the body of this asset, WITH the paint layout applied
	*
	* @return void
	* @access public
	*/
	function printBodyWithPaintLayout()
	{
		$url = strip_url(current_url(false, true));

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

		$result = $GLOBALS['SQ_SYSTEM']->am->getValueFromURL($url, $layout_name);

		// if no layout was found, try and get the default frontend
		// layout if we were not already trying to get it
		if (empty($result) && $layout_name != 'paint_layout::system::frontend') {
			$result = $GLOBALS['SQ_SYSTEM']->am->getValueFromURL($url, 'paint_layout::system::frontend');
		}

		$global_def_layout_id = $GLOBALS['SQ_SYSTEM']->getGlobalDefine('SQ_PAINT_LAYOUT_ID', 0);
		if (!$result && $global_def_layout_id) {
			$result = $global_def_layout_id;
		}

		if ($result) {
			// we have found the design to use... we might need to fool the system
			// into thinking we are printing the frontend of that asset
			$switched_frontend = false;
			if ($GLOBALS['SQ_SYSTEM']->frontend_asset->id !== $this->id) {
				$current_frontend_asset =& $GLOBALS['SQ_SYSTEM']->frontend_asset;
				$GLOBALS['SQ_SYSTEM']->frontend_asset =& $this;
				$switched_frontend = true;
			}
			$layout = &$GLOBALS['SQ_SYSTEM']->am->getAsset($result);
			$layout->paint($this);

			if ($switched_frontend) {
				$GLOBALS['SQ_SYSTEM']->frontend_asset =& $current_frontend_asset;
			}
		} else {
			// we can't find a layout, oh well let's just print out our body
			$this->printBody();
		}

	}//end printBodyWithPaintLayout()


	/**
	* Called by the design to print the body of this asset when in limbo
	*
	* By default, the normal backend is printed for the default screen
	*
	* @return void
	* @access public
	*/
	function printLimbo()
	{
		$o = &$GLOBALS['SQ_SYSTEM']->backend->out;
		$o->addHiddenField('asset_action', 'limbo');
		$o->openRaw();
		$this->paintBackend($o);
		$o->closeRaw();
		$o->paint();

	}//end printLimbo()


	/**
	* Prints out the backend interface for the asset
	* it's up to the kids to override
	*
	* @param object	&$o	Backend_Outputter object
	*
	* @return void
	* @access public
	*/
	function paintBackend(&$o)
	{
		$o->addHiddenField('asset_action', (($this->id) ? 'edit' : 'create'));
		if ($this->charset) $o->setCharset($this->charset);
		$ei = &$this->getEI();
		$ei->paint($this, $o, empty($this->id));

	}//end paintBackend()


	/**
	* Generate a prefix to be used by paint() and process()
	*
	* @return string
	* @access public
	*/
	function getPrefix()
	{
		return str_replace(' ', '_', $this->type().'_'.$this->id);

	}//end getPrefix()


	/**
	* Processes a backend submission from this asset, returns true if all OK
	* Put's the array of processed attributes into $this->_tmp['process_attributes']
	*
	* @param object	&$o		Backend_Outputter object
	* @param array	&$link	information used to create the initial link
	*
	* @return boolean
	* @access public
	*/
	function processBackend(&$o, &$link)
	{
		$ei = &$this->getEI();

		switch ($_POST['asset_action']) {
			case 'create' :
				if ($ei->process($this, $o, true)) {
					return (bool) $this->create($link);
				} else {
					return false;
				}
			break;

			default :
				return $ei->process($this, $o, false);
			break;
		}

	}//end processBackend()


	/**
	* Returns a reference to the Asset_Edit_Interface object for this asset type
	*
	* @return object
	* @access public
	*/
	function &getEI()
	{
		if (empty($this->_tmp['ei']) || get_class($this->_tmp['ei']) != 'asset_edit_interface') {
			require_once SQ_INCLUDE_PATH.'/asset_edit_interface.inc';
			$this->_tmp['ei'] =& new Asset_Edit_Interface($this->type());
		}
		return $this->_tmp['ei'];

	}//end getEI()


	/**
	* Returns the Edit_Fns object appropriate for whatever asset type we are
	*
	* @return object
	* @access public
	*/
	function &getEditFns()
	{
		$class_name = $this->type().'_edit_fns';
		require_once SQ_SYSTEM_ROOT.'/'.$GLOBALS['SQ_SYSTEM']->am->getTypeInfo($this->type(), 'dir').'/'.$class_name.'.inc';
		$edit_fns =& new $class_name();

		return $edit_fns;

	}//end getEditFns()


//--        URL's & HREF's        --//


	/**
	* Returns the href for getting to the nearest backend for this asset
	*
	* @param string	$screen	the code name of the screen that you want displayed when the asset is loaded
	*
	* @return string
	* @access public
	*/
	function getBackendHref($screen='')
	{
		$href = $GLOBALS['SQ_SYSTEM']->am->getAssetBackendHref(Array($this->id => $screen));
		return $href[$this->id];

	}//end getBackendHref()


	/**
	* Returns the URL for this asset, closest to the $base_url
	*
	* @param string		$base_url			the url that we are want to match closest to
	* @param boolean	$ignore_rollback	ignore rollback checks in asset manager?
	*										this should only get set by the asset manager itself
	*
	* @return string
	* @access public
	*/
	function getURL($base_url=null, $ignore_rollback=false)
	{
		return $GLOBALS['SQ_SYSTEM']->am->getAssetURL($this->id, $base_url, $ignore_rollback);

	}//end getURL()


	/**
	* Returns an array of URLs of this asset, including the protocol information
	*
	* @return array
	* @access public
	*/
	function getURLs()
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
	function getHref($base_url=null, $ignore_rollback=false)
	{
		return $GLOBALS['SQ_SYSTEM']->am->getAssetHref($this->id, $base_url, $ignore_rollback);

	}//end getHref()


	/**
	* Returns the href for this asset closest to the $base_url, but relative to the current url
	*
	* @return string
	* @access public
	*/
	function getWebDataPath()
	{
		return sq_web_path('data').'/assets/'.$this->type().'/'.$this->id;

	}//end getWebDataPath()


	/**
	* Returns a list of paths that this asset has
	*
	* @return array
	* @access public
	*/
	function getWebPaths()
	{
		if (!isset($this->_tmp['paths'])) {
			$db = &$GLOBALS['SQ_SYSTEM']->db;
			$where = 'assetid = '.$db->quote($this->id);
			$where = $GLOBALS['SQ_SYSTEM']->constructRollbackWhereClause($where);
			$sql = 'SELECT path
					FROM '.SQ_TABLE_RUNNING_PREFIX.'ast_path '.$where.'
					ORDER BY sort_order';

			$result = $db->getCol($sql);
			assert_valid_db_result($result);
			$this->_tmp['paths'] = $result;

		}// end if

		return $this->_tmp['paths'];

	}//end getWebPaths()


	/**
	* Attempts to override all current paths for this asset with those passed in the array
	*
	* @param array	$paths	array of paths to give this asset
	*
	* @return boolean
	* @access public
	*/
	function saveWebPaths($paths)
	{
		$current_paths = $this->getWebPaths();
		$save_paths = Array();

		// make sure these paths are compliant
		require_once SQ_INCLUDE_PATH.'/general_occasional.inc';
		$save_paths = make_valid_web_paths($paths);

		// if there is no difference in the arrays (including in the sort order - the index),
		// then there is nothing to do
		if (!array_diff_assoc($save_paths, $current_paths) && !array_diff_assoc($current_paths, $save_paths)) {
			return true;
		}

		//// CHECK INSERTED PATHS ////
		// to make sure that they path name is not in use by any of our siblings
		$inserts = array_diff($save_paths, $current_paths);
		if (!empty($inserts)) {
			// now we need to check that none of our parents have
			// any nav kids that are also using this path name
			$parents = $GLOBALS['SQ_SYSTEM']->am->getLinks($this->id, SQ_SC_LINK_WEB_PATHS, '', null, 'minor');
			for ($i = 0; $i < count($parents); $i++) {
				$parent = &$GLOBALS['SQ_SYSTEM']->am->getAsset($parents[$i]['majorid'], $parents[$i]['major_type_code']);
				if (is_null($parent)) continue;
				$bad_paths = $GLOBALS['SQ_SYSTEM']->am->webPathsInUse($parent, $inserts, $this->id);
				if (!empty($bad_paths)) {
					trigger_localised_error('SYS0201', E_USER_WARNING, implode('", "', $bad_paths), $parent->name);
					return false;
				}
			}
		}// end if inserts

		$GLOBALS['SQ_SYSTEM']->changeDatabaseConnection('db2');
		$GLOBALS['SQ_SYSTEM']->doTransaction('BEGIN');

		$db = &$GLOBALS['SQ_SYSTEM']->db;

		$sql = 'DELETE FROM
					sq_ast_path
				WHERE
					assetid	= '.$db->quoteSmart($this->id);

		$result = $db->query($sql);
		assert_valid_db_result($result);

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
						'.$db->quoteSmart($path).',
						'.$db->quoteSmart($this->id).',
						'.$db->quoteSmart($sort_order).'
					)';

			$result = $db->query($sql);
			assert_valid_db_result($result);
		}

		unset($this->_tmp['paths']);
		unset($this->_tmp['lookups']);

		if ($this->updateLookups()) {
			$GLOBALS['SQ_SYSTEM']->doTransaction('COMMIT');
			$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();
			return true;
		} else {
			$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
			$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();
			return false;
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
	function getLookups($field='')
	{
		if (!isset($this->_tmp['lookups'])) {
			$db = &$GLOBALS['SQ_SYSTEM']->db;

			$sql = 'SELECT url, http, https, root_urlid
					FROM '.SQ_TABLE_RUNNING_PREFIX.'ast_lookup ';
			$where = 'assetid = '.$db->quote($this->id);
			$where = $GLOBALS['SQ_SYSTEM']->constructRollbackWhereClause($where);

			$result = $db->getAll($sql.$where);
			assert_valid_db_result($result);
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
	function getDesignLookups()
	{
		if (!isset($this->_tmp['design_lookups'])) {
			$this->_tmp['design_lookups'] = Array();

			$db = &$GLOBALS['SQ_SYSTEM']->db;

			$sql = 'SELECT l.url, lv.name, lv.value as designid
					FROM '.SQ_TABLE_RUNNING_PREFIX.'ast_lookup l
							INNER JOIN '.SQ_TABLE_RUNNING_PREFIX.'ast_lookup_value lv ON l.url = lv.url
					';
			$where = 'l.assetid = '.$db->quote($this->id).'
					  AND lv.name LIKE '.$db->quote('design::%');
			$where = $GLOBALS['SQ_SYSTEM']->constructRollbackWhereClause($where, 'l');
			$where = $GLOBALS['SQ_SYSTEM']->constructRollbackWhereClause($where, 'lv');

			$result = $db->query($sql.$where);
			assert_valid_db_result($result);

			while (null !== ($row = $result->fetchRow())) {
				if (!isset($this->_tmp['design_lookups'][$row['url']])) {
					$this->_tmp['design_lookups'][$row['url']] = Array();
				}
				$this->_tmp['design_lookups'][$row['url']][$row['name']] = $row['designid'];
			}
			$result->free();

		}// end if

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
	* @param mixed		$inherited		it takes a mixed argument, boolean or null
	*									include inherited values (NULL = don't care, TRUE = Yes, FALSE = no)
	* @param string		$prefix			a prefix to search for in the name field of ast_lookup_value
	* @param boolean	$like_search	TRUE = LIKE $prefix, FALSE = NOT LIKE $prefix
	*
	* @return array
	* @access public
	*/
	function getLookupValues($inherited=null, $prefix='', $like_search=true)
	{
		$db = &$GLOBALS['SQ_SYSTEM']->db;

		$sql = 'SELECT l.url, lv.name, lv.value, lv.inhd
				FROM '.SQ_TABLE_RUNNING_PREFIX.'ast_lookup l
						INNER JOIN '.SQ_TABLE_RUNNING_PREFIX.'ast_lookup_value lv ON l.url = lv.url
				';
		$where = 'l.assetid = '.$db->quote($this->id);
		if (!is_null($inherited)) {
			$where .= ' AND lv.inhd = '.$db->quote((int) $inherited);
		}
		if ($prefix != '') {
			$where .= ' AND lv.name '.((!$like_search) ? 'NOT' : '').' LIKE '.$db->quote('design::%');
		}

		$where = $GLOBALS['SQ_SYSTEM']->constructRollbackWhereClause($where, 'l');
		$where = $GLOBALS['SQ_SYSTEM']->constructRollbackWhereClause($where, 'lv');

		$result = $db->query($sql.$where);
		assert_valid_db_result($result);

		$ret_val = Array();
		while (null !== ($row = $result->fetchRow())) {
			if (!isset($ret_val[$row['url']])) {
				$ret_val[$row['url']] = Array();
			}
			$ret_val[$row['url']][$row['name']] = Array('value' => $row['value'], 'inhd' => $row['inhd']);
		}
		$result->free();

		return $ret_val;

	}//end getLookupValues()


	/**
	* Called to force an update of this assets lookup information
	*
	* @return boolean
	* @access private
	* @see saveWebPaths()
	*/
	function updateLookups()
	{
		$paths = $this->getWebPaths();

		unset($this->_tmp['lookups']);
		unset($this->_tmp['design_lookups']);
		$GLOBALS['SQ_SYSTEM']->am->clearLookupsCache($this->id);

		require_once SQ_FUDGE_PATH.'/db_extras/db_extras.inc';
		$GLOBALS['SQ_SYSTEM']->changeDatabaseConnection('db2');
		$GLOBALS['SQ_SYSTEM']->doTransaction('BEGIN');
		$db = &$GLOBALS['SQ_SYSTEM']->db;

		// Remove all the old URLs
		$sql = 'SELECT url, http, https
				FROM sq_ast_lookup l
				WHERE assetid = '.$db->quote($this->id);

		$old_urls = $db->getAll($sql);
		assert_valid_db_result($old_urls);
		// get all the lookup information that we have set, ignoring designs as they are set on every url
		$our_lookup_values = $this->getLookupValues(false, $prefix='design::', false);

		$tmp_old_urls = Array();

		$current_urls = Array();
		$old_web_paths = Array();
		foreach ($old_urls as $url_data) {
			$url = $url_data['url'];
			$current_urls[] = $db->quoteSmart($url);
			if (strpos($url, '__data') === false) {
				// get the asset lineage and use that as the basis for indexing old URLs
				$url_lineage = $GLOBALS['SQ_SYSTEM']->am->getLineageFromURL(NULL, $url);
				$url_assetids = Array();
				foreach($url_lineage as $url_lineage_item) {
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
		// and reindex them to they both start from zero
		$old_to_new = array_values(array_diff($old_web_paths, $paths));
		$new_to_old = array_values(array_diff($paths, $old_web_paths));

		$web_path_changes = Array();

		if ((count($old_to_new) == 1) && (count($new_to_old) == 1)) {
			$web_path_changes[$old_to_new[0]] = $new_to_old[0];
		} else if ((count($old_to_new) > 1) && (count($new_to_old) > 1)) {
			// If more than one web path has changed...
			// If the number of web paths changing are the same, we should
			for ($i = 0; $i < min(count($old_to_new), count($new_to_old)); $i++) {
				$web_path_changes[$old_to_new[$i]] = $new_to_old[$i];
			}
		}

		if (!empty($current_urls)) {

			$sql = 'DELETE FROM
						sq_ast_lookup_value
					WHERE
						url IN ('.implode(',', $current_urls).')';

			$result = $db->query($sql);
			assert_valid_db_result($result);

			$sql = 'DELETE FROM
						sq_ast_lookup
					WHERE
						assetid	= '.$db->quoteSmart($this->id);

			$result = $db->query($sql);
			assert_valid_db_result($result);
		}

		// if we have paths then do some url inserting
		if (!empty($paths)) {

			$num_paths = count($paths);

			$our_design_links = $GLOBALS['SQ_SYSTEM']->am->getLinks($this->id, SQ_LINK_NOTICE, 'design', false);
			$our_designs = Array();
			foreach ($our_design_links as $link) {
				if (preg_match('/^design::(system|user)::.*$/', $link['value'])) {
					$our_designs[$link['value']] = Array(
													'value'	=> $link['minorid'],
													'inhd'	=> '0',
												   );
				}
			}

			// override designs take priority over normal, inheritable designs
			foreach ($our_design_links as $link) {
				if (preg_match('/^override::design::(system|user)::.*$/', $link['value'])) {
					$name = preg_replace('/^override::(.*)$/', '$1', $link['value']);
					$our_designs[$name] = Array(
													'value'	=> $link['minorid'],
													'inhd'	=> '0',
												 );
				}
			}
			unset($link);
			unset($our_design_links);

			$parents = $GLOBALS['SQ_SYSTEM']->am->getLinks($this->id, SQ_SC_LINK_WEB_PATHS, '', null, 'minor');

			$done_urls    = Array();
			$done_parents = Array();

			for ($i = 0; $i < count($parents); $i++) {
				if (in_array($parents[$i]['majorid'], $done_parents)) {
					continue;
				}
				$parent = &$GLOBALS['SQ_SYSTEM']->am->getAsset($parents[$i]['majorid'], $parents[$i]['major_type_code']);
				if (is_null($parent)) continue;
				$done_parents[] = $parent->id;

				$parent_lookup_values = $parent->getLookupValues();

				$parent_urls    = $parent->getLookups();

				for ($j = 0, $num_parent_urls = count($parent_urls); $j < $num_parent_urls; $j++) {
					for ($k = 0; $k < $num_paths; $k++) {
						$new_url = $parent_urls[$j]['url'].'/'.$paths[$k];
						if (isset($done_urls[$new_url])) continue;

						$http  = ($this->force_secure) ? '0' : $parent_urls[$j]['http'];
						$https = $parent_urls[$j]['https'];
						// if we have no protocol then we can just ignore this URL
						if (!$http && !$https) {
							continue;
						}

						$sql = 'INSERT INTO
									sq_ast_lookup
									(
										url,
										assetid,
										http,
										https,
										root_urlid
									)
									VALUES
									(
										'.$db->quoteSmart($new_url).',
										'.$db->quoteSmart($this->id).',
										'.$db->quoteSmart($http).',
										'.$db->quoteSmart($https).',
										'.$db->quoteSmart($parent_urls[$j]['root_urlid']).'
									)';

						$done_urls[] = $new_url;
						$result = $db->query($sql);
						assert_valid_db_result($result);

						// get the asset lineage and use that as the index
						$url_lineage = $GLOBALS['SQ_SYSTEM']->am->getLineageFromURL(NULL, $parent_urls[$j]['url']);
						$new_url_assetids = Array();
						foreach($url_lineage as $url_lineage_item) {
							$new_url_assetids[] = $url_lineage_item['assetid'];
						}
						$new_url_assetids[] = $this->id;

						foreach($tmp_old_urls as $tmp_old_url) {
							if ($tmp_old_url['url'] == $new_url) {
								// exact same URL as before
								$lookup_values = isset($our_lookup_values[$tmp_old_url['url']]) ? $our_lookup_values[$tmp_old_url['url']] : Array();
								break;
							} else if (($tmp_old_url['parent_url'] == $parent_urls[$j]['url']) && ($tmp_old_url['web_path'] == $paths[$k])) {
								// the same parent URL and the same path
								$lookup_values = isset($our_lookup_values[$tmp_old_url['url']]) ? $our_lookup_values[$tmp_old_url['url']] : Array();
								break;
							} else if ($tmp_old_url['parent_url'] == $parent_urls[$j]['url']) {
								// If the same parent URL, not the same path, but it's in our change list
								if (isset($web_path_changes[$tmp_old_url['web_path']]) && $web_path_changes[$tmp_old_url['web_path']] == $paths[$k]) {
									$lookup_values = isset($our_lookup_values[$tmp_old_url['url']]) ? $our_lookup_values[$tmp_old_url['url']] : Array();
									break;
								}
							}
						}

						if (!isset($lookup_values)) $lookup_values = Array();

						// all designs go to all urls (at the moment anyway), so merge them all in
						foreach ($our_designs as $name => $data) {
							$lookup_values[$name] = $data;
						}

						// get any of our parent's lookup values that we haven't overridden
						if (!empty($parent_lookup_values[$parent_urls[$j]['url']])) {
							foreach ($parent_lookup_values[$parent_urls[$j]['url']] as $name => $value_data) {
								if (isset($lookup_values[$name])) continue;

								if (preg_match('/^design::(system|user)::.*$/', $name)) {
									$inherited_lookup_value = $GLOBALS['SQ_SYSTEM']->am->getInheritedDesignValueFromURL($parent_urls[$j]['url'], $name);
									if (!empty($inherited_lookup_value)) {
										$lookup_values[$name] = Array(
																	'value'	=> $inherited_lookup_value['designid'],
																	'inhd'	=> '1',
																);
									}
								} else {
									$lookup_values[$name] = Array(
																'value'	=> $value_data['value'],
																'inhd'	=> '1',
															);
								}
							}
						}

						foreach ($lookup_values as $name => $data) {
							$sql = 'SELECT
										url
									FROM
										sq_ast_lookup_value
									WHERE
										url = '.$db->quoteSmart($new_url).'
										AND name = '.$db->quoteSmart($name);
							$result = $db->getRow($sql);
							assert_valid_db_result($result);

							// if the lookup value already exists, re-use it (this could happen
							// when a file changes back from a __data url)
							if (empty($result)) {
								$sql = 'INSERT INTO
											sq_ast_lookup_value
											(
												url,
												name,
												value,
												inhd
											)
											VALUES
											(
												'.$db->quoteSmart($new_url).',
												'.$db->quoteSmart($name).',
												'.$db->quoteSmart($data['value']).',
												'.$db->quoteSmart($data['inhd']).'
											)';
							} else {
								$sql = 'UPDATE
											sq_ast_lookup_value
										SET
											value = '.$db->quoteSmart($data['value']).',
											inhd = '.$db->quoteSmart($data['inhd']).'
										WHERE
											url = '.$db->quoteSmart($new_url).'
											AND name = '.$db->quoteSmart($name);
							}

							$result = $db->query($sql);
							assert_valid_db_result($result);
						}

					}//end for paths
				}//end for parent_urls
			}//end for parents

			if (!empty($old_urls)) {
				$rm = &$GLOBALS['SQ_SYSTEM']->am->getSystemAsset('remap_manager');
				// get the first new URL
				reset($done_urls);
				$new_url = current($done_urls);
				$old_url = $url_data['url'];
				// don't add a remap if we don't have a URL to map to
				if ($new_url != '') {
					foreach ($old_urls as $url_data) {
						$old_url = $url_data['url'];
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
							if ($url_data['http']) {
								$rm->addRemapURL('http://'.$old_url,  'http://'.$new_url);
							}
							if ($url_data['https']) {
								$rm->addRemapURL('https://'.$old_url, 'https://'.$new_url);
							}
						}
					}
				}
			}//end if !empty($old_urls)

		}//end if count paths

		$GLOBALS['SQ_SYSTEM']->doTransaction('COMMIT');
		$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();
		$GLOBALS['SQ_SYSTEM']->broadcastTriggerEvent('trigger_event_lookups_updated', $this);

		return true;

	}//end updateLookups()


	/**
	* Set's the lookup values for this asset
	*
	* @param array	$values	array of the values that we want to save
	* <PRE>
	*							"[url]" => array (
	*								"[name]" => array (
	*									"value" => "[value]",
	*									"inhd" => "[inherited]",
	*								),
	*							),
	* </PRE>
	*						NOTE: 'inhd' is ignored - it exists only because it makes it
	*						easier to just modify the return value of Asset::getLookupValues()
	*						all values in this array will be saved with inhd set to 0 (false)
	*
	* @return boolean
	* @access public
	* @see updateLookups()
	*/
	function setLookupValues($values)
	{
		if (!$this->writeAccess('lookups')) {
			trigger_localised_error('SYS0265', E_USER_WARNING, $this->name, $this->id);
			return false;
		}

		$db = &$GLOBALS['SQ_SYSTEM']->db;
		$GLOBALS['SQ_SYSTEM']->changeDatabaseConnection('db2');
		$GLOBALS['SQ_SYSTEM']->doTransaction('BEGIN');

		$sql = 'DELETE FROM
					sq_ast_lookup_value
				WHERE
						url IN
					(
						SELECT
							url
						FROM
							sq_ast_lookup
						WHERE
							assetid = '.$db->quoteSmart($this->id).'
					)';
		$result = $db->query($sql);
		assert_valid_db_result($result);

		$lookups = $this->getLookups('url');

		foreach ($values as $url => $values_data) {
			if (!in_array($url, $lookups)) continue;
			if (empty($values_data)) continue;

			foreach ($values_data as $value_name => $value_data) {

				$sql = 'INSERT INTO
							sq_ast_lookup_value
							(
								url,
								name,
								value,
								inhd
							)
							VALUES
							(
								'.$db->quoteSmart($url).',
								'.$db->quoteSmart($value_name).',
								'.$db->quoteSmart($value_data['value']).',
								'.$db->quoteSmart('0').'
							)';

				$result = $db->query($sql);
				assert_valid_db_result($result);
			}
		}//end foreach

		$GLOBALS['SQ_SYSTEM']->doTransaction('COMMIT');
		$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();
		return true;

	}//end setLookupValues()


	/**
	* Returns a replacement for the asset_status_description keyword
	* Will return 'Live', 'Under Construction' etc
	*
	* @return boolean
	* @access public
	*/
	function getAssetStatusDescriptionKeywordReplacement()
	{
		return get_status_description($this->status);
	}//end getAssetStatusDescriptionKeywordReplacement()


	/**
	* Returns a replacement for the asset_status_colour keyword
	* Returns it in '#RRGGBB' Hex form, and the colour corresponds
	* to that seen in the asset map for statuses.
	*
	* @return boolean
	* @access public
	*/
	function getAssetStatusColourKeywordReplacement()
	{
		return '#'.get_status_colour($this->status);
	}//end getAssetStatusColourKeywordReplacement()


}//end class


?>
