<?php

/**
* Asset
*
* Purpose
*
*
* @author  Blair Robertson <blair@squiz.net>
* @version $Version$ - 1.0
* @package Resolve
*/
class Asset extends Resolve_Object
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
	* String representation of this attribute
	* @var string
	*/
	var $name = '';

	/**
	* Short String representation of this attribute
	* @var string
	*/
	var $short_name = '';

	/**
	* Current status of the asset (live, under contrustion etc)
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
	* Timestamp of when this asset was last updated
	* @var int
	*/
	var $last_updated;

	/**
	* Assetid for the last user that updated this asset
	* @var int
	*/
	var $last_userid;

	/**
	* The Vars that belong to this asset
	* @var Array('var_name' => Array('attributeid' => $attributeid, 'value' = $value))
	*/
	var $vars = Array();

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
	*/
	function Asset($assetid=0)
	{
		$this->Resolve_Object();
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
	*   <li>TRUE if the asset was created but $link was empty</li>
	* </ul>
	*
	* @param Array	&$link	information used to create the initial link<br/>
	* <PRE>
	* Array ('asset'      => [ref major asset to create link under],
	*        'link_type'  => SQ_LINK_?,
	*        'value'      => [link value],
	*        'sort_order' => [link sort order],
	*        'dependant'  => [0|1],
	*        'exclusive'  => [0|1]
	*        )
	* </PRE>
	*
	* @return mixed int or false
	* @access public
	*/
	function create(&$link)
	{
		if (!empty($link)) {
			// make sure the initial link information is passed in
			if (!isset($link['asset']))     trigger_error('Cant create asset without an asset to link to', E_USER_ERROR);
			if (!isset($link['link_type'])) trigger_error('Cant create asset without a link type', E_USER_ERROR);
		}

		$db = &$GLOBALS['SQ_SYSTEM']->db;
		$GLOBALS['SQ_SYSTEM']->doTransaction('BEGIN');

		// welcome to sequences, they are cool, see the PEAR::DB docs for more info
		$assetid = $db->nextId('sq_sequence_asset');
		if (DB::isError($assetid)) trigger_error($assetid->getMessage().'<br/>'.$assetid->getUserInfo(), E_USER_ERROR);

		$name   = ucwords(str_replace('_', ' ', $this->type())).' #'.$assetid;
		$now    = time();
		$userid = $GLOBALS['SQ_SYSTEM']->currentUserId();
		$initial_version = '0.0.0';

		$values = Array('assetid'      => $assetid,
						'version'      => $initial_version,
						'type_code'    => $this->type(),
						'name'         => $name,
						'short_name'   => $name,
						'status'       => SQ_STATUS_UNDER_CONSTRUCTION,
						'last_updated' => ts_iso8601($now),
						'last_userid'  => $userid
						);
		if (!$GLOBALS['SQ_SYSTEM']->rollbackInsert('asset', $values)) {
			$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
			return false;
		}

		$this->id           = $assetid;
		$this->version      = $initial_version;
		$this->name         = $name;
		$this->short_name   = $name;
		$this->status       = SQ_STATUS_UNDER_CONSTRUCTION;
		$this->last_updated = $now;
		$this->last_userid  = $userid;

		// set the attribute values that have been stored temporarily
		if (!$this->saveAttributes(true)) {
			$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
			$this->id = 0;
			return false;
		}

		$this->_loadVars();

		if (!$this->_updated()) {
			$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
			return false;
		}

		if (!empty($link)) {
			// create the initial link
			if (!isset($link['value']))      $link['value']      = '';
			if (!isset($link['sort_order'])) $link['sort_order'] = -1;
			if (!isset($link['dependant']))  $link['dependant']  = 0;
			if (!isset($link['exclusive']))  $link['exclusive']  = 0;

			$lock_info = $link['asset']->getLockInfo();
			$parent_was_locked = !empty($lock_info);
			if (!$link['asset']->acquireLock()) {
				$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
				$this->id = 0;
				return false;
			}

			// if we are creating a dependant link, we need to make sure this new
			// asset is locked in the same chain as the parent (ie, has the same source_assetid)
			$source_assetid = 0;
			if ($link['dependant']) {
				$lock = $link['asset']->getLockInfo();
				$source_assetid = $lock['source_asset'];
			}
			if (!$this->acquireLock($source_assetid)) {
				$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
				$this->id = 0;
				return false;
			}

			$linkid = $link['asset']->createLink($this, $link['link_type'], $link['value'], $link['sort_order'], $link['dependant'], $link['exclusive']);

			// get the link so we can find out if it was dependant (cant rely on the info passed in
			// because other assets may have changed what happens in createLink)
			$new_link = $this->getLinkById($linkid, 'minor');

			// set permission on this asset that the parent has
			foreach (Array(SQ_PERMISSION_READ, SQ_PERMISSION_WRITE, SQ_PERMISSION_ADMIN) as $perm) {
				$set_perms = $link['asset']->getPermission($perm, null, false, false, true);
				foreach ($set_perms as $userid => $access) {
					if (!$this->setPermission($userid, $perm, $access)) {
						$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
						$this->id = 0;
						return false;
					}
				}
			}

			// release locks depending on if the parent was locked before we were created
			if (!$parent_was_locked) {
				// parent was not locked, so unlock both the parent and us
				if (!$link['asset']->releaseLock()) {
					$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
					$this->id = 0;
					return false;
				}
			}
			if (!$new_link['dependant']) {
				if (!$this->releaseLock()) {
					$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
					$this->id = 0;
					return false;
				}
			}

			if (!$linkid) {
				$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
				$this->id = 0;
				return false;
			}
			$GLOBALS['SQ_SYSTEM']->doTransaction('COMMIT');
			return (int) $linkid;
		} else {
			$GLOBALS['SQ_SYSTEM']->doTransaction('COMMIT');
			return true;
		}

	}//end create()


	/**
	* Load the asset represented by the passed asset ID
	*
	* @param int	$assetid	the asset ID to be loaded
	*
	* @access public
	*/
	function load($assetid)
	{
		$db = &$GLOBALS['SQ_SYSTEM']->db;

		// OK, the first thing to do is check we actually exist :)
		$sql = 'SELECT assetid, version, name, short_name, status, languages, charset, type_code, last_updated, last_userid
				FROM '.SQ_TABLE_RUNNING_PREFIX.'asset ';
		$where = 'assetid = '.$db->quote($assetid);
		$where = $GLOBALS['SQ_SYSTEM']->constructRollbackWhereClause($where);

		$result = $db->getRow($sql.$where, null, DB_FETCHMODE_ORDERED);
		if (DB::isError($result)) trigger_error($result->getMessage().'<br/>'.$result->getUserInfo(), E_USER_ERROR);

		if (is_null($result)) return;

		list($this->id,
			 $this->version,
			 $this->name,
			 $this->short_name,
			 $this->status,
			 $this->languages,
			 $this->charset,
			 $type_code,
			 $this->last_updated,
			 $this->last_userid) = $result;
		$this->last_updated = iso8601_ts($this->last_updated);
		unset($result);

		// make sure the asset we are loading is of the same type as our class
		if ($type_code != $this->type()) {
			trigger_error('Asset #'.$assetid.' is not a '.$this->type(), E_USER_WARNING);
			$this->id = null;
			$this->last_updated = null;
			$this->last_userid  = null;
			return;
		}

		if ($this->useSystemVersion()) {
			$this->_loadDataPaths();
			if (!$this->loadSystemVersion()) {
				trigger_error('Asset #'.$assetid.' failed loading from cache', E_USER_WARNING);
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
	* @access private
	* @return void
	*/
	function _loadDataPaths()
	{
		$this->data_path_suffix = 'assets/'.$this->type().'/'.$this->id;
		$this->data_path = SQ_DATA_PATH.'/private/'.$this->data_path_suffix;
		$this->data_path_public = SQ_DATA_PATH.'/public/'.$this->data_path_suffix;
		if ($this->useSystemVersion()) $this->data_path .= '/.sq_system';

	}//end _loadDataPaths()


	/**
	* Load the vars for this asset/ asset type into the vars array
	*
	* If the asset has no asset ID, this function just loads the default values for the asset type.
	*
	* @access private
	* @return void
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
			$where = 'a.assetid = '.$GLOBALS['SQ_SYSTEM']->db->quote($this->id);
			$where = $GLOBALS['SQ_SYSTEM']->constructRollbackWhereClause($where, 'a');
			// PURPOSLY DONT ADD EXTRA CLAUSES FOR ASSET_ATTRIBUTE_VALUE BECAUSE WE WONT GET
			// DEFAULT VALUES IF WE DO
			$sql = 'SELECT atr.name, atr.attributeid, atr.type, COALESCE(v.custom_value, atr.default_value) AS value
					FROM '.SQ_TABLE_RUNNING_PREFIX.'asset a
							INNER JOIN '.SQ_TABLE_PREFIX.'asset_attribute atr ON a.type_code = atr.type_code
							LEFT OUTER JOIN '.SQ_TABLE_RUNNING_PREFIX.'asset_attribute_value v
								ON (atr.attributeid = v.attributeid AND a.assetid = v.assetid
									'.$GLOBALS['SQ_SYSTEM']->constructRollbackWhereClause('', 'v', 'AND').')
					'.$where;
		// else just load all defaults
		} else {
			$sql = 'SELECT atr.name, atr.attributeid, atr.type, atr.default_value AS value
					FROM '.SQ_TABLE_PREFIX.'asset_attribute atr
					WHERE atr.type_code = '.$GLOBALS['SQ_SYSTEM']->db->quote($this->type());
		}// end if

		$this->vars = $GLOBALS['SQ_SYSTEM']->db->getAssoc($sql, false, Array(), DB_FETCHMODE_ASSOC); // need all this because of DB API
		if (DB::isError($this->vars)) trigger_error($this->vars->getMessage().'<br/>'.$this->vars->getUserInfo(), E_USER_ERROR);

		if ($this->_ser_attrs && $this->vars) {
			for(reset($this->vars); NULL !== ($name = key($this->vars)); next($this->vars)) {
				if ($this->vars[$name]['type'] != 'serialise') continue;
				$this->vars[$name]['value'] = @unserialize($this->vars[$name]['value']);
			}
		}

	}//end _loadVars()


	/**
	* Create a duplicate of this asset
	*
	* The only child assets that are duplicated are the ones that are <i>DEPENDANTLY</i> linked to this asset.
	* <i>NO OTHER</i> linked assets are duplicated.
	*
	* @param Array	&$link				information used to create the initial link,
	* <PRE>
	* Array ('asset'      => [ref major asset to create link under],
	*        'link_type'  => SQ_LINK_?,
	*        'value'      => [link value],
	*        'sort_order' => [link sort order],
	*        'dependant'  => [0|1],
	*        'exclusive'  => [0|1]
	*        )
	* </PRE>
	* @param Array	&$dupe_map			a map for that can be used for re-indexing data held in
	*									assets of the form Array([old asset id] => [new asset id])
	* @param bool	$dupe_dependants	whether or not to duplicate any assets that are dependantly
	*									linked to this asset, only works if $link is not empty
	* @param bool	$dupe_directory		whether or not to duplicate this assets data directory
	*
	* @return mixed reference to the new object or NULL if an error occured
	* @access public
	*/
	function &duplicate(&$link, &$dupe_map, $dupe_dependants=true, $dupe_directory=true)
	{
		if (!empty($link)) {
			// make sure the initial link information is passed in
			if (!isset($link['asset']))     trigger_error('Cant create asset without an asset to link to', E_USER_ERROR);
			if (!isset($link['link_type'])) trigger_error('Cant create asset without a link type', E_USER_ERROR);
		}

		$null = null; // needed because we return by reference
		if (!$this->id) return $null;

		$lock = $this->getLockInfo();
		if (empty($lock)) {
			trigger_error('You need to acquire a lock on this asset before it can be duplicated', E_USER_WARNING);
			return $null;
		}

		$db = &$GLOBALS['SQ_SYSTEM']->db;
		$GLOBALS['SQ_SYSTEM']->doTransaction('BEGIN');

		// welcome to sequences, they are cool, see the PEAR::DB docs for more info
		$assetid = $db->nextId('sq_sequence_asset');
		if (DB::isError($assetid)) {
			$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
			trigger_error($assetid->getMessage().'<br/>'.$assetid->getUserInfo(), E_USER_ERROR);
		}

		$now    = time();
		$userid = $GLOBALS['SQ_SYSTEM']->currentUserId();
		// version number starts at 0.0.0 again because the dupe is a brand new asset
		$initial_version = '0.0.0';

		$values = Array('assetid'      => $assetid,
						'version'      => $initial_version,
						'type_code'    => $this->type(),
						'name'         => $this->name.' - Duplicate',
						'short_name'   => $this->short_name.' - Duplicate',
						'status'       => SQ_STATUS_UNDER_CONSTRUCTION,
						'last_updated' => ts_iso8601($now),
						'last_userid'  => $userid
						);
		if (!$GLOBALS['SQ_SYSTEM']->rollbackInsert('asset', $values)) {
			$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
			return $null;
		}

		$dupe = &$GLOBALS['SQ_SYSTEM']->am->getAsset($assetid, $this->type());
		if (is_null($dupe)) {
			$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
			return $null;
		}

		// Lock the new dupe in the same chain as we are locked
		if (!$dupe->acquireLock($lock['source_asset'])) {
			$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
			return $null;
		}

		// Now we set all the attributes
		foreach($this->vars as $name => $data) {
			if (!$dupe->setAttrValue($name, $data['value'])) {
				$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
				return $null;
			}
		}

		if (!empty($link)) {
			// create the initial link
			if (!isset($link['value']))      $link['value']      = '';
			if (!isset($link['sort_order'])) $link['sort_order'] = -1;
			if (!isset($link['dependant']))  $link['dependant']  = 0;
			if (!isset($link['exclusive']))  $link['exclusive']  = 0;

			// OK what we are going to do is make sure that the new parents lock is
			// in the same chain as our current lock
			$parent_lock = $link['asset']->getLockInfo();
			if (empty($parent_lock) || $parent_lock['source_asset'] != $lock['source_asset']) {

				if (!empty($parent_lock)) {
					if (!$link['asset']->releaseLock()) {
						$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
						return $null;
					}
				}

				if (!$link['asset']->acquireLock($lock['source_asset'])) {
					$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
					return $null;
				}
			}

			$linkid = $link['asset']->createLink($dupe, $link['link_type'], $link['value'], $link['sort_order'], $link['dependant'], $link['exclusive']);
			if(empty($linkid)) {
				$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
				return $null;
			}

			// OK now that we are linked up let's our dependants
			if ($dupe_dependants) {
				$dependant_links = $this->getLinks(SQ_SC_LINK_SIGNIFICANT, '', true, 'major', null, 1);

				if (!empty($dependant_links)) {
					$create_link = Array('asset'      => &$dupe,
										 'link_type'  => null,
										 'value'      => null,
										 'sort_order' => -1,
										 'dependant'  => '1',
										 'exclusive'  => '0');
					$am = &$GLOBALS['SQ_SYSTEM']->am;
					foreach($dependant_links as $data) {
						// If this asset has already been duplicated in this duplication run
						// then just link it to the new duplicate of ourselves
						if (isset($dupe_map[$data['minorid']])) {

							$duped_child = &$GLOBALS['SQ_SYSTEM']->am->getAsset($dupe_map[$data['minorid']], $data['type_code']);
							if (is_null($duped_child)) {
								$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
								return $null;
							}// end if

							$linkid = $dupe->createLink($duped_child, $data['link_type'], $data['value'], $data['sort_order'], '1', $data['exclusive']);
							if (!$linkid) {
								$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
								return $null;
							}// end if

						// otherwise duplicate and link
						} else {
							$child = &$am->getAsset($data['minorid'], $data['type_code']);
							if (is_null($child)) continue;
							$create_link['link_type']	= $data['link_type'];
							$create_link['value']		= $data['value'];
							$create_link['sort_order']	= $data['sort_order'];
							$create_link['exclusive']	= $data['exclusive'];
							$duped_child = &$child->duplicate($create_link, $dupe_map, true, $dupe_directory);
							if (is_null($duped_child)) {
								$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
								return $null;
							}
						}// end if
					}// end fpr
				}// end if empty
			}// end if dupe dependants

		}// end if !empty(link)

		// OK if we got this far let's copy the directory (if it exists)
		if ($dupe_directory && is_dir($this->data_path)) {
			require_once SQ_FUDGE_PATH.'/general/file_system.inc';
			if (!copy_directory($this->data_path, $dupe->data_path)) {
				$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
				return $null;
			}
		}

		// if we got this far all is well,
		// so add to the map and return the duplicate
		$dupe_map[$this->id] = $dupe->id;
		$GLOBALS['SQ_SYSTEM']->doTransaction('COMMIT');
		return $dupe;

	}//end duplicate()


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
			trigger_error('Cannot delete asset, asset is not in the trash', E_USER_WARNING);
			return false;
		}

		// check that the trash link is the only significant link left
		$links = $this->getLinks(SQ_SC_LINK_SIGNIFICANT, '', true, 'minor');
		if (count($links) != 1) {
			trigger_error('Cannot delete asset, asset must only be linked in the trash', E_USER_WARNING);
			return false;
		}

		$db = &$GLOBALS['SQ_SYSTEM']->db;
		$GLOBALS['SQ_SYSTEM']->doTransaction('BEGIN');

		// say to other assets: 'Delete me bitch cause I'm outa here'
		$links = $this->getLinks(SQ_SC_LINK_ALL, '', true, 'minor');
		foreach ($links as $link) {
			$major = &$GLOBALS['SQ_SYSTEM']->am->getAsset($link['majorid']);
			$major->deleteLink($link['linkid']);
		}

		if ($release_lock) {
			// we are about to go to the big asset manager in the sky,
			// so we wont be needing worldly things such as locks
			if (!$this->releaseLock()) {
				trigger_error('Delete failed, the lock on "'.$this->name.'" could not be released', E_USER_WARNING);
				$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
				return false;
			}
		}

		// remove web paths
		if (!$this->saveWebPaths(Array())) {
			$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
			return false;
		}

		// delete from the asset table
		$where_cond = 'assetid = '.$db->quote($this->id);
		if (!$GLOBALS['SQ_SYSTEM']->rollbackDelete('asset', $where_cond)) {
			$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
			return false;
		}

		// now try and clear the data directories for this asset
		require_once SQ_FUDGE_PATH.'/general/file_system.inc';
		if (is_dir($this->data_path)) {
			if (!delete_directory($this->data_path)) {
				trigger_error('Failed deleteing asset "'.$this->name.'", could not delete restricted data directory', E_USER_WARNING);
				$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
				return false;
			}
		}

		if (is_dir($this->data_path_public)) {
			if (!delete_directory($this->data_path_public)) {
				trigger_error('Failed deleteing asset "'.$this->name.'", could not delete restricted data directory', E_USER_WARNING);
				$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
				return false;
			}
		}

		$GLOBALS['SQ_SYSTEM']->doTransaction('COMMIT');

		return true;

	}//end delete()


	/**
	* Determine if the current user be shown the system version or not
	*
	* You would want to show the system version if the asset is in safe edit and the user does not have write access.
	*
	* @return boolean
	* @access public
	*/
	function useSystemVersion()
	{
		return (($this->status & SQ_SC_STATUS_SAFE_EDITING) && !$this->writeAccess(Array(), false));

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
			trigger_error('Failed loading asset "'.$this->id.'" from cache, object data file "'.$this->data_path.'/.object_data'.'" not found', E_USER_WARNING);
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
	* This copy gets used when the asset is live and the user does not have write access.
	*
	* @return boolean
	* @access public
	*/
	function saveSystemVersion()
	{
		require_once SQ_FUDGE_PATH.'/general/file_system.inc';

		// make sure our data directories exists
		if (!create_directory($this->data_path)) {
			trigger_error('Failed saving asset "'.$this->name.'" to cache, could not create restricted data directory', E_USER_WARNING);
			return false;
		}

		// make sure our system directories exists
		if (!create_directory($this->data_path.'/.sq_system')) {
			trigger_error('Failed saving asset "'.$this->name.'" to cache, could not create restricted system directory', E_USER_WARNING);
			return false;
		}

		// make sure there is nothing in the system directories
		if (!clear_directory($this->data_path.'/.sq_system')) {
			trigger_error('Failed saving asset "'.$this->name.'" to cache, could not clear restricted system directory', E_USER_WARNING);
			return false;
		}

		// save the object for later (in the restricted directory)
		if (!string_to_file(serialize($this), $this->data_path.'/.sq_system/.object_data')) {
			trigger_error('Failed saving asset "'.$this->name.'" to cache, could not write object data file', E_USER_WARNING);
			return false;
		}

		// move all the other files in our data directories
		// into the new system directories for later use
		$files_to_copy = list_files($this->data_path);
		foreach ($files_to_copy as $filename) {
			if (!copy_file($this->data_path.'/'.$filename, $this->data_path.'/.sq_system/'.$filename)) {
				trigger_error('Failed saving asset "'.$this->name.'" to cache, could not copy file "'.$filename.'" to restricted system directory', E_USER_WARNING);
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
			trigger_error('Failed clearing system version of asset "'.$this->name.'", could not create data directory', E_USER_WARNING);
			return false;
		}

		// make sure our system directory exists
		if (!create_directory($this->data_path.'/.sq_system')) {
			trigger_error('Failed clearing system version of asset "'.$this->name.'", could not create system directory', E_USER_WARNING);
			return false;
		}

		if (!clear_directory($this->data_path.'/.sq_system')) {
			trigger_error('Failed clearing system version of asset "'.$this->name.'"', E_USER_WARNING);
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
		require_once SQ_FUDGE_PATH.'/general/file_system.inc';

		// load our object data in as it was before
		if (is_file($this->data_path.'/.sq_system/.object_data')) {
			$old_version = unserialize(file_to_string($this->data_path.'/.sq_system/.object_data'));
		} else {
			trigger_error('Failed reverting asset "'.$this->name.'" to system version, could not locate object data file', E_USER_WARNING);
			return false;
		}

		// copy over the old files we stored in the system directory
		$files_to_copy = list_files($this->data_path.'/.sq_system');
		foreach ($files_to_copy as $filename) {
			// skip hidden files
			if (strpos($filename, '.') === 0) continue;

			if (!copy_file($this->data_path.'/.sq_system/'.$filename, $this->data_path.'/'.$filename)) {
				trigger_error('Failed reverting asset "'.$this->name.'" to system version, could not copy file "'.$filename.'" to restricted data directory', E_USER_WARNING);
				return false;
			}
		}

		if (!$this->clearSystemVersion()) {
			trigger_error('Failed reverting asset "'.$this->name.'" to system version, could not clear system version', E_USER_WARNING);
			return false;
		}

		// update all the vars
		foreach ($old_version->vars as $var_name => $var_data) {
			if ($var_data['type'] == 'serialise') $var_data['value'] = serialize($var_data['value']);
			if (!$this->setAttrValue($var_name, $var_data['value'], true)) {
				trigger_error('Failed reverting asset "'.$this->name.'" to system version, could not save value of attribute "'.$var_name.'"', E_USER_WARNING);
				return false;
			}
		}

		// ITS ALIVE... ITS ALIVE...
		$this = $old_version;

		// save this old version to the db
		$this->_updated();

		return true;

	}//end revertToSystemVersion()


	/**
	* Set the last updated info for this asset
	*
	* Call this function when the asset has been changed in some way so we can indicated in the
	* asset table when the asset was last changed and who by. This function will also increment the micro
	* version number for the asset.
	*
	* @return boolean
	* @access private
	*/
	function _updated()
	{
		$last_updated = time();
		$last_userid  = $GLOBALS['SQ_SYSTEM']->currentUserId();
		$name         = $this->_getName();
		$short_name   = $this->_getName(true);

		// if we have an ID hit the DB
		if ($this->id) {

			$db = &$GLOBALS['SQ_SYSTEM']->db;

			$GLOBALS['SQ_SYSTEM']->doTransaction('BEGIN');

			$where = 'assetid = '.$db->quote($this->id);
			$values = Array('name'         => $db->quote($name),
							'short_name'   => $db->quote($short_name),
							'languages'    => $db->quote($this->languages),
							'charset'      => $db->quote($this->charset),
							'last_updated' => $db->quote(ts_iso8601($last_updated)),
							'last_userid'  => $db->quote($last_userid)
							);
			if (!$GLOBALS['SQ_SYSTEM']->rollbackUpdate('asset', $values, $where)) {
				$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
				return false;
			}

			$GLOBALS['SQ_SYSTEM']->broadcastEvent('AssetUpdate',
												  Array( 'assetid'          => $this->id,
														 'old_name'         => $this->name,
														 'new_name'         => $name,
														 'old_short_name'   => $this->short_name,
														 'new_short_name'   => $short_name,
														 'old_last_updated' => $this->last_updated,
														 'new_last_updated' => $last_updated,
														 'old_last_userid'  => $this->last_userid,
														 'new_last_userid'  => $last_userid
														));

			// increment the micro version number
			if (!$this->incrementVersion()) {
				trigger_error('Failed updating "'.$this->name.'", version not incremented', E_USER_WARNING);
				$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
				return false;
			}

			$GLOBALS['SQ_SYSTEM']->doTransaction('COMMIT');

		}// end if $this->id

		$this->name         = $name;
		$this->short_name   = $short_name;
		$this->last_updated = $last_updated;
		$this->last_userid  = $last_userid;

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
	* Generate a prefix to be used by paint() and process()
	*
	* @param object	Asset	$owner	the asset whose interface we are painting
	*
	* @return string
	* @access public
	*/
	function getPrefix()
	{
		return $this->type().'_'.$this->id;

	}//end getPrefix()


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
	* @return array()
	* @access public
	* @see Asset_Manager::getTypeAncestors()
	*/
	function getTypeAncestors($include_asset=true)
	{
		return $GLOBALS['SQ_SYSTEM']->am->getTypeAncestors($this->type(), $include_asset);

	}//end getTypeAncestors()


	/**
	* Get all asset IDs that are above the asset in the various trees in which it exists
	*
	* @param string|array	$type_code			the type of asset that is linked
	*											(eg 'Page', 'File', etc)
	*											if an array returns link if matches any
	*											of the array values
	* @param bool			$strict_type_code	whether we are finding assets that are just a $type_code
	*											or $type_code and any of it's sub-classes
	*
	* @return array(int)
	* @access public
	*/
	function getParents($type_code='', $strict_type_code=true)
	{
		return $GLOBALS['SQ_SYSTEM']->am->getParents($this->id, $type_code, $strict_type_code);

	}//end getParents()


	/**
	* Get all asset IDs that are below the asset in the various trees in which it exists
	*
	* @param string|array	$type_code			the type of asset that is linked
	*											(eg 'Page', 'File', etc)
	*											if an array returns link if matches any
	*											of the array values
	* @param bool			$strict_type_code	whether we are finding assets that are just a $type_code
	*											or $type_code and any of it's sub-classes
	*
	* @return array(int)
	* @access public
	*/
	function getChildren($type_code='', $strict_type_code=true)
	{
		return $GLOBALS['SQ_SYSTEM']->am->getChildren($this->id, $type_code, $strict_type_code);

	}//end getChildren()


	/**
	* Get all dependant children of an asset in the various tress in which it exists
	*
	* @param string|array	$type_code			the type of asset that is linked
	*											(eg 'Page', 'File', etc)
	*											if an array returns link if matches any
	*											of the array values
	* @param bool			$strict_type_code	whether we are finding assets that are just a $type_code
	*											or $type_code and any of it's sub-classes
	*
	* @return array(int)
	* @access public
	*/
	function getDependantChildren($type_code='', $strict_type_code=true)
	{
		return $GLOBALS['SQ_SYSTEM']->am->getDependantChildren($this->id, $type_code, $strict_type_code);

	}//end getChildren()


	/**
	* Increments the version number of the asset based on the part of the version passed in
	*
	* @param string	$number	the part of the version to update (major|minor|micro)
	*
	* @return boolean
	* @access public
	*/
	function incrementVersion($number='micro')
	{
		if (!$this->id) return false;

		if      ($number == 'major') $part_num = 0;
		else if ($number == 'minor') $part_num = 1;
		else if ($number == 'micro') $part_num = 2;

		// work out the new version number based on the part of the
		// version that we have been asked to update (major|minor|micro)
		$version_parts = explode('.', $this->version);
		for (reset($version_parts); null !== ($k = key($version_parts)); next($version_parts)) {
			$value =& $version_parts[$k];
			$value = (int)$value;
			if ($k == $part_num) $value++;
			else if ($k > $part_num) $value = 0;
		}

		$new_version = implode('.', $version_parts);

		$GLOBALS['SQ_SYSTEM']->doTransaction('BEGIN');
		$db = &$GLOBALS['SQ_SYSTEM']->db;

		$where = 'assetid = '.$db->quote($this->id);
		$values = Array('version' => $db->quote($new_version));
		if (!$GLOBALS['SQ_SYSTEM']->rollbackUpdate('asset', $values, $where)) {
			$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
			return false;
		}

		$GLOBALS['SQ_SYSTEM']->broadcastEvent('AssetVersionUpdate',
											  Array( 'assetid'          => $this->id,
													 'old_version'      => $this->version,
													 'new_version'      => $new_version
													));

		$GLOBALS['SQ_SYSTEM']->doTransaction('COMMIT');

		$this->version = $new_version;
		return true;

	}//end incrementVersion()


	/**
	* Returns a status object for the current asset to be used for processing
	*
	* @return object Asset_Status
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
			$this->_tmp['status_object'] = new $status_code($this);
		}
		return $this->_tmp['status_object'];

	}//end &getStatus()


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
	* @param int	$new_status	the new status to set for this asset
	*
	* @return boolean
	* @access public
	*/
	function processStatusChange($new_status)
	{
		if ($this->status != $new_status) {
			// if we are dependant minor assets, we can only change
			// our status to a status higher than or equal to all
			// the statii of our parents
			$dependant_parents = $this->getLinks(SQ_SC_LINK_SIGNIFICANT, '', true, 'minor', null, 1);
			if (!empty($dependant_parents)) {
				$am = &$GLOBALS['SQ_SYSTEM']->am;
				foreach ($dependant_parents as $link) {
					$asset = &$am->getAsset($link['majorid']);
					if ($asset->status > $new_status) {
						// we dont want to rollback, but we dont
						// want to change our status for real,
						// and we dont want to change our childrens statii
						return true;
					}
				}
			}
		}

		// we are probably going to be sending some internal messages during this
		// process, so we'll open a new queue here
		$ms = &$GLOBALS['SQ_SYSTEM']->getMessagingService();
		$ms->openQueue();

		$status = &$this->getStatus();
		if (!$status->processStatusChange($new_status)) {
			$ms->abortQueue();
			return false;
		}

		$GLOBALS['SQ_SYSTEM']->doTransaction('BEGIN');

		if ($new_status != $this->status) {

			// if we have just made this asset live, we update its minor version
			if ($new_status == SQ_STATUS_LIVE) {
				if (!$this->incrementVersion('minor')) {
					trigger_error('processStatusChange failed for asset "'.$this->name.'", version not updated', E_USER_WARNING);
					$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
					$ms->abortQueue();
					return false;
				}
			}

			$db = &$GLOBALS['SQ_SYSTEM']->db;

			$where = 'assetid = '.$db->quote($this->id);
			$values = Array('status' => $db->quote($new_status));
			if (!$GLOBALS['SQ_SYSTEM']->rollbackUpdate('asset', $values, $where)) {
				$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
				$ms->abortQueue();
				return false;
			}

			$GLOBALS['SQ_SYSTEM']->broadcastEvent('AssetStatusUpdate',
												  Array('assetid'    => $this->id,
														'old_status' => $this->status,
														'new_status' => $new_status,
														));

			$this->status = $new_status;
			unset($this->_tmp['status_object']);
			$this->_updated();
		}

		// try and process the change on our dependants
		$dependant_links = $this->getLinks(SQ_SC_LINK_SIGNIFICANT, '', true, 'major', null, 1);
		if (!empty($dependant_links)) {
			$am = &$GLOBALS['SQ_SYSTEM']->am;
			foreach ($dependant_links as $link) {
				$asset = &$am->getAsset($link['minorid']);
				if (!$asset->processStatusChange($new_status)) {
					trigger_error('processStatusChange failed for asset "'.$this->name.'" trying to process dependant child "'.$asset->name.'"', E_USER_WARNING);
					$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
					$ms->abortQueue();
					return false;
				}
			}
		}

		$GLOBALS['SQ_SYSTEM']->doTransaction('COMMIT');

		// close the internal message queue and send the messages that
		// were generated during this status change
		$ms->closeQueue();

		return true;

	}//end processStatusChange()


	/**
	* Gets the default languages of the asset as an array
	*
	* @return array(string)
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
		if($set_languages == $this->languages) return false;

		$old_languages = $this->languages;
		$this->languages = $set_languages;
		if (!$this->_updated()) {
			trigger_error('Failed setting languages for "'.$this->name.'"', E_USER_WARNING);
			$this->languages = $old_languages;
			return false;
		}

		$GLOBALS['SQ_SYSTEM']->broadcastEvent('AssetLanguageUpdate',
											   Array( 'assetid'       => $this->id,
													  'old_languages' => $old_languages,
													  'new_languages' => $this->languages,
													));
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
		if($charset == $this->charset) return false;
		$old_charset = $this->charset;
		$this->charset = $charset;
		if (!$this->_updated()) {
			trigger_error('Failed setting character set for "'.$this->name.'"', E_USER_WARNING);
			$this->charset = $old_charset;
			return false;
		}
		$GLOBALS['SQ_SYSTEM']->broadcastEvent('AssetCharsetUpdate',
											   Array( 'assetid'     => $this->id,
													  'old_charset' => $old_charset,
													  'new_charset' => $this->charset,
													));
		return true;

	}//end setCharset()


	/**
	* Remap existing assetids used to new ones for ourselves and all our dependants
	*
	* @param array	&$map	a map of the form Array([old asset id] => [new asset id]);
	*
	* @return boolean
	* @access public
	*/
	function remapAssetids($map)
	{
		$GLOBALS['SQ_SYSTEM']->doTransaction('BEGIN');
		// try and acquire locks on our dependants
		$dependant_links = $this->getLinks(SQ_SC_LINK_SIGNIFICANT, '', true, 'major', null, 1);
		if (!empty($dependant_links)) {
			$am = &$GLOBALS['SQ_SYSTEM']->am;
			foreach ($dependant_links as $link) {
				$asset = &$am->getAsset($link['minorid'], $link['type_code']);
				if (!$asset->remapAssetids($map)) {
					$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
					return false;
				}
			}
		}

		$GLOBALS['SQ_SYSTEM']->doTransaction('COMMIT');
		return true;

	}// end remapAssetids()


	/**
	* Morph the asset to either one of its parent types or one of its children types
	*
	* Only succeeds if $type_code assets are allowed to link to all children that this asset has.
	*
	* @param string	$new_type_code	the new type_code to attempt to cast this asset to
	*
	* @return bool
	* @access public
	*/
	function morph($new_type_code)
	{
		$new_type_code = strtolower($new_type_code);
		$old_type_code = $this->type();

		// dickhead check
		if ($this->type() == $new_type_code) return true;

		if (!$GLOBALS['SQ_SYSTEM']->am->installed($new_type_code)) {
			trigger_error('Asset "'.$new_type_code.'" is not installed on the system', E_USER_WARNING);
			return false;
		}

		$db = &$GLOBALS['SQ_SYSTEM']->db;
		$am = &$GLOBALS['SQ_SYSTEM']->am;

		// open the transaction
		$GLOBALS['SQ_SYSTEM']->doTransaction('BEGIN');

		// is this new type one of our decendents or ancestors?
		$sql = 'SELECT COUNT(*)
				FROM '.SQ_TABLE_PREFIX.'asset_type_inherited
				WHERE (inherited_type_code = '.$db->quote($this->type()).'  AND type_code = '.$db->quote($new_type_code).')
				   OR (inherited_type_code = '.$db->quote($new_type_code).' AND type_code = '.$db->quote($this->type()).')';
		if ($db->getOne($sql) == 0) {
			trigger_error('Unable to cast "'.$this->name.'" to "'.$new_type_code.'", "'.$new_type_code.'" is not in the "'.$this->type().'" asset type heirarchy.', E_USER_WARNING);
			return false;
		}

		$am->includeAsset($new_type_code);
		$tmp = new $new_type_code();

		//// FIRST we need to check that all our links are allowed for the new type ////

		// if there any children links
		if ($num_links = $this->countLinks('major')) {

			$links = $this->getLinks(SQ_SC_LINK_ALL);
			foreach($links as $link) {
				if ($tmp->canLinkToType($link['type_code'], $link['link_type'], 0, $link['exclusive']) !== true) {
					require_once SQ_INCLUDE_PATH.'/general_occasional.inc';
					trigger_error('Unable to cast "'.$this->name.'" to "'.$new_type_code.'",<br>"'.$link['type_code'].'" assets are not allowed to be "'.link_type_name($link['link_type']).'" linked to "'.$new_type_code.'" assets.', E_USER_WARNING);
					$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
					return false;
				}
			}// end foreach

		}// end if child links

		// if there any parent links
		if ($num_links = $this->countLinks('minor')) {

			$links = $this->getLinks(SQ_SC_LINK_ALL, '', true, 'minor');
			foreach($links as $link) {
				$parent = &$am->getAsset($link['majorid'], $link['type_code']);
				if (is_null($parent)) continue;
				if (($err_msg = $parent->canLinkToType($new_type_code, $link['link_type'], $link['linkid'])) !== true) {
					require_once SQ_INCLUDE_PATH.'/general_occasional.inc';
					trigger_error('Unable to cast "'.$this->name.'" to "'.$new_type_code.'", "'.$parent->name.'" (Asset # '.$parent->id.') cannot be linked to the new Asset - ('.$err_msg.')', E_USER_WARNING);
					$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
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
			foreach($common_var_list as $var_name) {
				$current_id = $this->vars[$var_name]['attributeid'];
				$new_id = $tmp->vars[$var_name]['attributeid'];

				// now update attributeid
				$where = 'assetid = '.$db->quote($this->id).'
						  AND attributeid = '.$current_id;
				$values = Array('attributeid' => $db->quote($new_id));
				if (!$GLOBALS['SQ_SYSTEM']->rollbackUpdate('asset_attribute_value', $values, $where)) {
					$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
					return false;
				}
			}
		}// end if common var list

		// Get all the vars that aren't available in the new type and delete them
		$deletes_var_list = array_diff($current_var_list, $new_var_list);

		if ($deletes_var_list) {
			$deletes_attributeids = '';
			foreach($deletes_var_list as $var_name) $deletes_attributeids .= (($deletes_attributeids) ? ',' : '').$db->quote($this->vars[$var_name]['attributeid']);

			$where_cond = ' assetid = '.$db->quote($this->id).'
							AND attributeid IN ('.$deletes_attributeids.')';
			if (!$GLOBALS['SQ_SYSTEM']->rollbackDelete('asset_attribute_value', $where_cond)) {
				$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
				return false;
			}
		}// end if delete var list

		$where = 'assetid = '.$db->quote($this->id);
		$values = Array('type_code' => $db->quote($new_type_code));
		if (!$GLOBALS['SQ_SYSTEM']->rollbackUpdate('asset', $values, $where)) {
			$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
			return false;
		}

		// Now load this new asset into the temporary
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
			// if there isn't a directory then everything is fine
			} else {
				$all_ok = true;
			}// end if

		}// end if

		// before we override ourselves, do any cleaning up that we might need
		if ($all_ok && !$this->_morphCleanup($new_type_code)) $all_ok = false;

		// All is OK so override ourselves with the temporary
		if ($all_ok) {
			$this = $tmp;
			$GLOBALS['SQ_SYSTEM']->doTransaction('COMMIT');

			$GLOBALS['SQ_SYSTEM']->broadcastEvent('AssetTypeUpdate',
												  Array( 'assetid'  => $this->id,
														 'old_type' => $old_type_code,
														 'new_type' => $new_type_code
														));

			return true;

		// bugger something went wrong
		} else {
			$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
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
	* Can the current user forceably acquire the current lock on this asset?
	*
	* @return boolean
	* @access public
	*/
	function canForceablyAcquireLock()
	{
		$current_lock = $this->getLockInfo();
		if (empty($current_lock)) return false;

		$user = &$GLOBALS['SQ_SYSTEM']->am->getAsset($current_lock['userid']);

		$can_acquire_lock = false;

		// lets work out if the current user has a high
		// enough level of access to forceably acquire the lock
		if (!$GLOBALS['SQ_SYSTEM']->userRoot($user)) {
			if ($GLOBALS['SQ_SYSTEM']->userSystemAdmin($user)) {
				// lock is held by a system admin, so need
				// to be root to acquire this lock
				if ($GLOBALS['SQ_SYSTEM']->userRoot()) $can_acquire_lock = true;
			} else {
				// need to be a system admin to acquire this lock
				if ($GLOBALS['SQ_SYSTEM']->userRoot()) $can_acquire_lock = true;
				else if ($GLOBALS['SQ_SYSTEM']->userSystemAdmin()) $can_acquire_lock = true;
			}
		}

		return $can_acquire_lock;

	}//end canForceablyAcquireLock()


	/**
	* Acquires a lock on this asset
	*
	* @param int		$source		the id of the asset that started this locking process
	* @param boolean	$force		attempt to forcibly acquire (if allowed)
	* @param int		$expires	when the lock expires (timestamp)
	*
	* @return boolean
	* @access public
	*/
	function acquireLock($source=0, $force=false, $expires=null)
	{
		$current_lock = $this->getLockInfo();
		if (!empty($current_lock) && $current_lock['userid'] == $GLOBALS['SQ_SYSTEM']->currentUserid()) {
			// the user is asking to acquire a lock they already had
			// so just update the lock expiry date
			$this->updateLock($expires);
			return true;
		}

		$db = &$GLOBALS['SQ_SYSTEM']->db;

		if($force && $this->canForceablyAcquireLock()) {
			// special case where the current user is actually
			// removing the current lock and taking it for themselves
			$where = 'source_asset = '.$db->quote($current_lock['source_asset']);
			$where = $GLOBALS['SQ_SYSTEM']->constructRollbackWhereClause($where);
			$sql = 'SELECT assetid FROM '.SQ_TABLE_RUNNING_PREFIX.'asset_lock '.$where;
			$locked_ids = $db->getCol($sql);
			if (DB::isError($locked_ids)) {
				$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
				trigger_error($locked_ids->getMessage().'<br/>'.$locked_ids->getUserInfo(), E_USER_WARNING);
				return false;
			}

			if (!$this->releaseLock()) return false;

			// send an internal message
			$ms = &$GLOBALS['SQ_SYSTEM']->getMessagingService();
			$ms->openQueue();
			$current_user = &$GLOBALS['SQ_SYSTEM']->am->getAsset($GLOBALS['SQ_SYSTEM']->currentUserid());
			$user = &$GLOBALS['SQ_SYSTEM']->am->getAsset($current_lock['userid']);

			foreach ($locked_ids as $locked) {
				$locked_asset = &$GLOBALS['SQ_SYSTEM']->am->getAsset($locked);

				// create a new message object and populate it, before adding to
				// the message queue in the internal messaging system to sending later
				$message = $ms->newMessage();
				$message->subject = 'Asset Lock Forceably Acquired';
				$message->body = 'User "'.$current_user->name.'" has forceably acquired the lock on '.$locked_asset->type().' "'.$locked_asset->name.'", formally held by user "'.$user->name.'".';
				$message->to = $locked_asset->getPermission(SQ_PERMISSION_ADMIN, true);
				$message->to[] = $user->id;
				$message->from = 0; // a system message

				$ms->enqueueMessage($message);
				$GLOBALS['SQ_SYSTEM']->am->unregisterAsset($locked_asset);
			}

			// close the queue of messages we opened, which sends all the messages in the queue
			$ms->closeQueue();

			// refresh the lock info
			$current_lock = $this->getLockInfo();

		}// end if

		// is this asset already locked
		if (!empty($current_lock)) {
			$user = &$GLOBALS['SQ_SYSTEM']->am->getAsset($current_lock['userid']);
			trigger_error('Cannot acquire lock on "'.$this->name.'", lock already held by "'.$user->name.'"', E_USER_NOTICE);
			return false;
		}

		// if we have no source, we are the asset aquiring the locks
		if (empty($source)) $source = $this->id;
		$expires = (is_null($expires)) ? (time() + SQ_CONF_LOCK_LENGTH) : (int) $expires;

		$GLOBALS['SQ_SYSTEM']->doTransaction('BEGIN');

		// now try and get the lock on ourself
		$values = Array('assetid'      => $this->id,
						'source_asset' => $source,
						'userid'       => $GLOBALS['SQ_SYSTEM']->currentUserid(),
						'expires'      => ts_iso8601($expires)
						);
		if (!$GLOBALS['SQ_SYSTEM']->rollbackInsert('asset_lock', $values)) {
			$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
			return false;
		}

		$GLOBALS['SQ_SYSTEM']->doTransaction('COMMIT');
		return true;

	}//end acquireLock()


	/**
	* Release the locks that this asset holds
	*
	* @return boolean
	* @access public
	*/
	function releaseLock()
	{
		$current_lock = $this->getLockInfo(false, false);
		if (empty($current_lock)) return true;

		// is this asset already locked by someone else and we can't forceably acquire it, piss off
		if ((int) $current_lock['userid'] != $GLOBALS['SQ_SYSTEM']->currentUserid() && !$this->canForceablyAcquireLock()) {
			$user = &$GLOBALS['SQ_SYSTEM']->am->getAsset($current_lock['userid']);
			trigger_error('You cannot release a lock that you do not own, the lock on "'.$this->name.'" is held by "'.$user->name.'"', E_USER_WARNING);
			return false;
		}

		$GLOBALS['SQ_SYSTEM']->doTransaction('BEGIN');

		$db = &$GLOBALS['SQ_SYSTEM']->db;

		$where_cond = 'source_asset = '.$db->quote($current_lock['source_asset']);
		if (!$GLOBALS['SQ_SYSTEM']->rollbackDelete('asset_lock', $where_cond)) {
			$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
			return false;
		}

		$GLOBALS['SQ_SYSTEM']->doTransaction('COMMIT');
		return true;

	}//end releaseLock()


	/**
	* Updates the expiry date of the lock on this asset
	*
	* @param int	$source		the id of the asset that started this locking process
	* @param int	$expires	when the lock expires (timestamp)
	*
	* @return boolean
	* @access public
	*/
	function updateLock($expires=null)
	{
		$current_lock = $this->getLockInfo();
		if (empty($current_lock)) return true;

		$db = &$GLOBALS['SQ_SYSTEM']->db;

		$expires = (is_null($expires)) ? (time() + SQ_CONF_LOCK_LENGTH) : (int) $expires;

		$GLOBALS['SQ_SYSTEM']->doTransaction('BEGIN');

		$where = 'source_asset = '.$db->quote($current_lock['source_asset']);
		$values = Array('expires' => $db->quote(ts_iso8601($expires)));
		if (!$GLOBALS['SQ_SYSTEM']->rollbackUpdate('asset_lock', $values, $where)) {
			$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
			return false;
		}

		$GLOBALS['SQ_SYSTEM']->doTransaction('COMMIT');
		return true;

	}//end updateLock()


	/**
	* Returns an array of information about the current lock on this asset (if any)
	*
	* @param boolean	$full_chain		return the info for every asset in the lock chain
	* @param boolean	$check_expires	check the expiry date and cleanup the lock if it has expired
	*
	* @return array
	* @access public
	*/
	function getLockInfo($full_chain=false, $check_expires=true)
	{
		$db = &$GLOBALS['SQ_SYSTEM']->db;

		// just get the lock for this asset
		$where = 'assetid = '.$db->quote($this->id);
		$where = $GLOBALS['SQ_SYSTEM']->constructRollbackWhereClause($where);
		$sql = 'SELECT assetid, source_asset, userid, expires
				FROM '.SQ_TABLE_RUNNING_PREFIX.'asset_lock '.$where;

		$result = $db->getRow($sql);
		if (DB::isError($result)) {
			trigger_error($result->getMessage().'<br/>'.$result->getUserInfo(), E_USER_WARNING);
			return Array();
		}

		if (!empty($result)) {
			$result['expires'] = iso8601_ts($result['expires']);
			if ($check_expires && !empty($result) && $result['expires'] < time()) {
				// this lock has expired
				if ($this->cleanupLocks()) return Array();
			}
		}

		if ($full_chain && !empty($result)) {
			$result['chained_assets'] = Array();
			$source_assetid = $result['source_asset'];
			$where = 'source_asset = '.$db->quote($source_assetid).'
					  AND assetid <> '.$db->quote($this->id);
			$where = $GLOBALS['SQ_SYSTEM']->constructRollbackWhereClause($where);
			$sql = 'SELECT assetid, source_asset, userid, expires
					FROM '.SQ_TABLE_RUNNING_PREFIX.'asset_lock '.$where;

			$to_do = $db->getAll($sql);
			if (DB::isError($to_do)) {
				trigger_error($to_do->getMessage().'<br/>'.$to_do->getUserInfo(), E_USER_WARNING);
				return Array();
			}

			foreach ($to_do as $lock_row) {
				$lock_row['expires'] = iso8601_ts($lock_row['expires']);
				$result['chained_assets'][$lock_row['assetid']] = $lock_row;
			}
		}

		return $result;

	}//end getLockInfo()


	/**
	* Deletes the lock on this asset if it is expired
	*
	* @return array
	* @access public
	*/
	function cleanupLocks()
	{
		$current_lock = $this->getLockInfo(false, false);
		if (empty($current_lock)) return true;

		$now = date('Y-m-d H:i:s');
		$db = &$GLOBALS['SQ_SYSTEM']->db;

		$where_cond = ' source_asset = '.$db->quote($current_lock['source_asset']).'
						AND expires < '.$db->quote($now);
		if (!$GLOBALS['SQ_SYSTEM']->rollbackDelete('asset_lock', $where_cond)) {
			return false;
		}

		return true;

	}//end cleanupLock()


	/**
	* Returns information for a link that this asset is the major asset in
	*
	* @param int			$link_type			integer that should be a single integer of
	*											the SQ_LINK_* constants
	* @param string|array	$type_code			the type of asset that is linked
	*											(eg 'Page', 'File', etc)
	*											if an array returns link if matches any
	*											of the array values
	* @param bool			$strict_type_code	whether we are finding an asset that
	*											is just a $type_code or potentially an inherited type
	*											or $type_code and any of it's sub-classes
	* @param string			$value				the value that is associated with this link
	* @param string			$side_of_link		Which side of the link this (the current) asset
	*											is on ('major' or 'minor')
	* @param boolean		$exclusive			The exclusive status for the link must be
	*											this (if not null)
	*
	* @return Array()
	* @access public
	*/
	function getLink($link_type, $type_code='', $strict_type_code=true, $value=null, $side_of_link='major', $exclusive=null)
	{
		if ($side_of_link != 'major' && $side_of_link != 'minor')
			trigger_error('Unknown Side of Link "'.$side_of_link.'"', E_USER_ERROR);

		if (!$this->id) return Array();

		$other_side = ($side_of_link == 'major') ? 'minor' : 'major';

		$db = &$GLOBALS['SQ_SYSTEM']->db;
		$sql = 'SELECT  l.linkid, l.'.$other_side.'id, l.value, l.link_type,
						a.type_code, l.sort_order, l.dependant, l.exclusive
				FROM '.SQ_TABLE_RUNNING_PREFIX.'asset_link l
						INNER JOIN '.SQ_TABLE_RUNNING_PREFIX.'asset a ON l.'.$other_side.'id  = a.assetid
				';
		$where = 'l.'.$side_of_link.'id = '.$db->quote($this->id).'
				  AND l.link_type = '.$db->quote($link_type);
		if (!is_null($exclusive)) $where .= ' AND l.exclusive = '.$db->quote(($exclusive) ? '1' : '0');

		if ($type_code) {

			$type_code_cond = '';
			if (is_array($type_code)) {
				for($i = 0; $i < count($type_code); $i++) $type_code[$i] = $db->quote($type_code[$i]);
				$type_code_cond = 'IN ('.implode(', ', $type_code).')';
			} else {
				$type_code_cond = '= '.$db->quote($type_code);
			}

			if ($strict_type_code) {
				$where .= ' AND a.type_code '.$type_code_cond;
			} else {
				require_once SQ_FUDGE_PATH.'/db_extras/db_extras.inc';
				$where .= ' AND a.type_code IN (~SQ0~)';
				$subs = Array( 'SELECT type_code
								FROM '.SQ_TABLE_PREFIX.'asset_type_inherited
								WHERE inherited_type_code '.$type_code_cond );
				$where = db_extras_subquery($db, $where, $subs);
				if (DB::isError($where))
					trigger_error($where->getMessage().'<br/>'.$where->getUserInfo(), E_USER_ERROR);
			}
		}

		if (!is_null($value)) $where .= '  AND l.value = '.$db->quote($value);

		$where = $GLOBALS['SQ_SYSTEM']->constructRollbackWhereClause($where, 'l');
		$where = $GLOBALS['SQ_SYSTEM']->constructRollbackWhereClause($where, 'a');

		$sql .= $where.' ORDER BY l.sort_order';

		$result = $db->getRow($sql);
		if (DB::isError($result)) {
			trigger_error($result->getMessage().'<br/>'.$result->getUserInfo(), E_USER_ERROR);
			return Array();
		}

		return $result;

	}//end getLink()


	/**
	* Returns the link information for a specific link that this asset is the major asset in
	*
	* @param int	$linkid
	* @param string	$side_of_link	Which side of the link this (the current)
	*								asset is on ('major' or 'minor')
	* @param bool	$ignore_this	if true we don't care whether this link is ours or not
	*
	* @return Array
	* @access public
	*/
	function getLinkById($linkid, $side_of_link='major', $ignore_this=false)
	{
		if ($side_of_link != 'major' && $side_of_link != 'minor')
			trigger_error('Unknown Side of Link "'.$side_of_link.'"', E_USER_ERROR);

		if (!$this->id) return Array();

		$other_side = ($side_of_link == 'major') ? 'minor' : 'major';

		$db = &$GLOBALS['SQ_SYSTEM']->db;
		$where = 'l.linkid = '.$db->quote($linkid).'
				  AND l.'.$other_side.'id = a.assetid';
		if (!$ignore_this) $where .= ' AND l.'.$side_of_link.'id = '.$db->quote($this->id);
		$where = $GLOBALS['SQ_SYSTEM']->constructRollbackWhereClause($where, 'l');
		$where = $GLOBALS['SQ_SYSTEM']->constructRollbackWhereClause($where, 'a');

		$sql = 'SELECT  l.linkid, l.'.$other_side.'id, l.value, l.link_type,
						a.type_code, l.sort_order, l.dependant, l.exclusive
				FROM '.SQ_TABLE_RUNNING_PREFIX.'asset_link l, '.SQ_TABLE_RUNNING_PREFIX.'asset a '.$where;

		$result = $db->getRow($sql);
		if (DB::isError($result))
			trigger_error($result->getMessage().'<br/>'.$result->getUserInfo(), E_USER_ERROR);

		return $result;

	}//end getLinkById()


	/**
	* Returns information for a link or links that is/are between this asset and the passed
	*
	*
	* @param object Asset	$asset				the asset to find a link to/from (major/minor)
	* @param int			$link_type			integer that should be a single integer of
	*											the SQ_LINK_* constants
	* @param string			$value				the value that is associated with this link
	* @param string			$side_of_link		Which side of the link this (the current) asset
	*											is on ('major' or 'minor')
	* @param boolean		$force_array		force the results to return and array of links
	*											even if there is only one link
	*
	* @return Array()	array of link info if only one link and force_array=false,
	*                   array of arrays of link info otherwise
	* @access public
	*/
	function getLinkByAsset(&$asset, $link_type=null, $value=null, $side_of_link='major', $force_array=false)
	{
		if ($side_of_link != 'major' && $side_of_link != 'minor')
			trigger_error('Unknown Side of Link "'.$side_of_link.'"', E_USER_ERROR);

		if (!$this->id) return Array();

		$other_side = ($side_of_link == 'major') ? 'minor' : 'major';

		$db = &$GLOBALS['SQ_SYSTEM']->db;
		$sql = 'SELECT  l.linkid, l.'.$other_side.'id, l.value, l.link_type,
						a.type_code, l.sort_order, l.dependant, l.exclusive
				FROM '.SQ_TABLE_RUNNING_PREFIX.'asset_link l
						INNER JOIN '.SQ_TABLE_RUNNING_PREFIX.'asset a ON l.'.$other_side.'id  = a.assetid
				';

		$where = 'l.'.$side_of_link.'id = '.$db->quote($this->id).'
				  AND l.'.$other_side.'id  = '.$db->quote($asset->id);
		if (!is_null($link_type)) $where .= '  AND l.link_type = '.$db->quote($link_type);
		if (!is_null($value))     $where .= '  AND l.value = '.$db->quote($value);

		$where = $GLOBALS['SQ_SYSTEM']->constructRollbackWhereClause($where, 'l');
		$where = $GLOBALS['SQ_SYSTEM']->constructRollbackWhereClause($where, 'a');

		$sql .= $where.'
				ORDER BY l.sort_order';

		$result = $db->getAll($sql);
		if (DB::isError($result)) {
			trigger_error($result->getMessage().'<br/>'.$result->getUserInfo(), E_USER_ERROR);
			return Array();
		}

		if (!$force_array && count($result) == 1) return $result[0];
		else return $result;

	}//end getLinkByAsset()


	/**
	* Return all links that this asset has to other assets
	*
	* @param int			$link_types			integer that can be the product of bitwise operations
	*											on the SQ_LINK_* constants
	* @param string|array	$type_code			the type of asset that is linked
	*											(eg 'Page', 'File', etc)
	*											if an array returns link if matches any
	*											of the array values
	* @param bool			$strict_type_code	whether we are finding assets that are just a $type_code
	*											or $type_code and any of it's sub-classes
	* @param string			$side_of_link		Which side of the link this (the current) asset
	*											is on ('major' or 'minor')
	* @param string			$value				The value for all the links must equal this (if not null)
	* @param boolean		$dependant			The dependant status for all the links must be
	*											this (if not null)
	* @param boolean		$exclusive			The exclusive status for all the links must be
	*											this (if not null)
	*
	* @return Array()
	* @access public
	*/
	function getLinks($link_types, $type_code='', $strict_type_code=true, $side_of_link='major', $value=null, $dependant=null, $exclusive=null)
	{
		if ($side_of_link != 'major' && $side_of_link != 'minor')
			trigger_error('Unknown Side of Link "'.$side_of_link.'"', E_USER_ERROR);

		if (!$this->id) return Array();

		$other_side = ($side_of_link == 'major') ? 'minor' : 'major';

		$db = &$GLOBALS['SQ_SYSTEM']->db;

		$where = 'l.'.$other_side.'id = a.assetid
				  AND l.'.$side_of_link.'id = '.$db->quote($this->id).'
				  AND (l.link_type & '.$db->quote($link_types).') > 0 ';
		if (!is_null($value))		$where .= ' AND l.value     = '.$db->quote($value);
		if (!is_null($dependant))	$where .= ' AND l.dependant = '.$db->quote(($dependant) ? '1' : '0');
		if (!is_null($exclusive))	$where .= ' AND l.exclusive = '.$db->quote(($exclusive) ? '1' : '0');

		if (!empty($type_code)) {

			$type_code_cond = '';
			if (is_array($type_code)) {
				for($i = 0; $i < count($type_code); $i++) $type_code[$i] = $db->quote($type_code[$i]);
				$type_code_cond = 'IN ('.implode(', ', $type_code).')';
			} else {
				$type_code_cond = '= '.$db->quote($type_code);
			}

			if ($strict_type_code) {
				$where .= ' AND a.type_code '.$type_code_cond;
			} else {
				require_once SQ_FUDGE_PATH.'/db_extras/db_extras.inc';
				$where .= ' AND a.type_code IN (~SQ0~)';
				$subs = Array( 'SELECT type_code
								FROM '.SQ_TABLE_PREFIX.'asset_type_inherited
								WHERE inherited_type_code '.$type_code_cond );
				$where = db_extras_subquery($db, $where, $subs);
				if (DB::isError($where))
					trigger_error($where->getMessage().'<br/>'.$where->getUserInfo(), E_USER_ERROR);
			}
		}// end if

		$where = $GLOBALS['SQ_SYSTEM']->constructRollbackWhereClause($where, 'l');
		$where = $GLOBALS['SQ_SYSTEM']->constructRollbackWhereClause($where, 'a');

		$sql = 'SELECT  l.linkid, l.'.$other_side.'id, l.value, l.link_type,
						a.type_code, l.sort_order, l.dependant, l.exclusive
				FROM '.SQ_TABLE_RUNNING_PREFIX.'asset_link l, '.SQ_TABLE_RUNNING_PREFIX.'asset a '.
				$where.' ORDER BY l.sort_order, a.type_code';

		$result = $db->getAll($sql);
		if (DB::isError($result))
			trigger_error($result->getMessage().'<br/>'.$result->getUserInfo(), E_USER_ERROR);

		return $result;

	}//end getLinks()


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
	* @return Array()
	* @access private
	*/
	function _getAllowedLinks()
	{
		return Array(SQ_LINK_TYPE_1 => Array(),
					 SQ_LINK_TYPE_2 => Array(),
					 SQ_LINK_TYPE_3 => Array('design' => Array('card' => 'M', 'exclusive' => false)),
					 SQ_LINK_NOTICE => Array()
					);

	}//end _getAllowedLinks()


	/**
	* Returns the number of links that this asset has as either a major or minor party,
	* can be restricted by either link type and/or asset type code
	*
	* @param string			$side_of_link		Which side of the link this (the current) asset
	*											is on ('major' or 'minor')
	* @param int			$link_types			integer that can be the product of bitwise operations
	*											on the SQ_LINK_* constants
	* @param string|array	$type_code			the type of asset that is linked (eg 'Page', 'File', etc)
	*											if an array returns link if matches any
	*											of the array values
	* @param bool			$strict_type_code	whether we are finding assets that are just a $type_code
	*											or $type_code and any of it's sub-classes
	* @param int			$ignore_linkid		ignore the link represented by this link
	*											id when returning the count
	*
	* @return int
	* @access public
	*/
	function countLinks($side_of_link, $link_types=0, $type_code='', $strict_type_code=true, $ignore_linkid=0)
	{
		if ($side_of_link != 'major' && $side_of_link != 'minor')
			trigger_error('Unknown Side of Link "'.$side_of_link.'"', E_USER_ERROR);

		if (!$this->id) return 0;

		$db = &$GLOBALS['SQ_SYSTEM']->db;

		$extra_table = '';
		$where = 'l.'.$side_of_link.'id = '.$db->quote($this->id);
		if ($link_types) {
			$where .= ' AND (link_type & '.$db->quote($link_types).') > 0';
		}
		if ($type_code) {
			$extra_table .= ', '.SQ_TABLE_RUNNING_PREFIX.'asset a';
			$where .= ' AND l.minorid = a.assetid ';
			$where  = $GLOBALS['SQ_SYSTEM']->constructRollbackWhereClause($where, 'a');

			$type_code_cond = '';
			if (is_array($type_code)) {
				for($i = 0; $i < count($type_code); $i++) $type_code[$i] = $db->quote($type_code[$i]);
				$type_code_cond = 'IN ('.implode(', ', $type_code).')';
			} else {
				$type_code_cond = '= '.$db->quote($type_code);
			}

			if ($strict_type_code) {
				$where .= ' AND a.type_code '.$type_code_cond;
			} else {
				require_once SQ_FUDGE_PATH.'/db_extras/db_extras.inc';
				$where .= ' AND a.type_code IN (~SQ0~)';
				$subs = Array( 'SELECT type_code
								FROM '.SQ_TABLE_PREFIX.'asset_type_inherited
								WHERE inherited_type_code '.$type_code_cond );
				$where = db_extras_subquery($db, $where, $subs);
				if (DB::isError($sql))
					trigger_error($sql->getMessage().'<br/>'.$sql->getUserInfo(), E_USER_ERROR);
			}// end if
		}// end if
		if ($ignore_linkid) {
			$where .= ' AND l.linkid <> '.$db->quote($ignore_linkid);
		}


		$where = $GLOBALS['SQ_SYSTEM']->constructRollbackWhereClause($where, 'l');
		$sql = 'SELECT COUNT(*)
				FROM '.SQ_TABLE_RUNNING_PREFIX.'asset_link l'.$extra_table.' '.$where;
		$result = $db->getOne($sql);

		if (DB::isError($result))
			trigger_error($result->getMessage().'<br/>'.$result->getUserInfo(), E_USER_ERROR);

		return $result;

	}//end countLinks()


	/**
	* Returns TRUE if the passed minor asset can be linked to this asset, or a string with the error msg
	*
	* @param object Asset	$minor		the minor asset that we are linking to
	* @param string			$link_type	the type of link this is
	* @param int			$exclusive	the exclusive status of the link (gets passed to canLinkToType)
	*
	* @return mixed TRUE or error message string
	* @access public
	*/
	function canCreateLink(&$minor, $link_type, $exclusive)
	{
		if (!$this->id) return false;

		// if you dont have write access to this asset you cant create any links
		if (!$GLOBALS['SQ_INSTALL'] && !$this->writeAccess(Array(), true, false)) {
			return 'Cannot create link from '.$this->name.' (# '.$this->id.') to '.$minor->name.' (# '.$minor->id.'), permission denied (no write access to "'.$this->name.'")';
		}

		require_once SQ_FUDGE_PATH.'/db_extras/db_extras.inc';

		if (!is_a($minor, 'Asset')) {
			return 'Minor is not an asset';
		}

		// check if we are allowed to link to these type of assets
		if (($err_msg = $this->canLinkToType($minor->type(), $link_type, 0, $exclusive)) !== true) {
			return $err_msg;
		}

		$db = &$GLOBALS['SQ_SYSTEM']->db;

		// check for web path conflicts with the new parents children
		if ($link_type & SQ_SC_LINK_WEB_PATHS) {
			$paths = $minor->getWebPaths();
			$bad_paths = $this->_webPathsInUse($paths, $minor->id);
			if (!empty($bad_paths)) {
				return 'Cannot create link from '.$this->name.' (# '.$this->id.') to '.$minor->name.' (# '.$minor->id.'), path(s) "'.implode('", "', $bad_paths).'" already in use by one of the siblings of "'.$this->name.'"';
			}
		}

		// if this link is a significant link, then we need to make sure that this
		// minor asset doesn't already have an exclusive link from anything
		// and that we aren't moving under ourselves
		if ($link_type & SQ_SC_LINK_SIGNIFICANT) {
			//// Check for exclusive ////
			$sql = 'SELECT majorid FROM '.SQ_TABLE_RUNNING_PREFIX.'asset_link ';
			$where = 'minorid       = '.$db->quote($minor->id).'
					  AND exclusive = '.$db->quote('1');
			$where = $GLOBALS['SQ_SYSTEM']->constructRollbackWhereClause($where);
			$sql .= $where;

			$majorid = $db->getOne($sql);
			if (DB::isError($majorid)) {
				trigger_error($majorid->getMessage().'<br/>'.$majorid->getUserInfo(), E_USER_WARNING);
				return 'Database Error';
			}

			if ($majorid) {
				$major = &$GLOBALS['SQ_SYSTEM']->am->getAsset($majorid);
				return 'Asset "'.$minor->name.'" (Asset #'.$minor->id.') cannot be linked to "'.$this->name.'" (Asset #'.$this->id.') it is exclusively linked to Asset #'.$majorid.' ("'.$major->name.'")';
			}

			//// Check we aren't moving the minor asset under itself ////
			// get all the links where the minor asset is a parent
			// and check to see if our id is under it

			// so much for SQL standards...
			$concat = ($db->phptype == 'mysql') ? 'CONCAT(pt.treeid, '.$db->quote('%').')' : '(pt.treeid || '.$db->quote('%').')';
			$where = 'ct.treeid LIKE '.$concat.'
					  AND ct.treeid >= pt.treeid
					  AND pt.linkid IN (~SQ0~)
					  AND cl.minorid = '.$db->quote($this->id);

			$sub_where = 'minorid = '.$db->quote($minor->id);
			$sub_where = $GLOBALS['SQ_SYSTEM']->constructRollbackWhereClause($sub_where);
			$subs = Array('SELECT linkid FROM '.SQ_TABLE_RUNNING_PREFIX.'asset_link '.$sub_where);

			$where = db_extras_subquery($db, $where, $subs);
			if (DB::isError($where)) {
				trigger_error($where->getMessage().'<br/>'.$where->getUserInfo(), E_USER_WARNING);
				return 'Database Error';
			}

			$where = $GLOBALS['SQ_SYSTEM']->constructRollbackWhereClause($where, 'pt');
			$where = $GLOBALS['SQ_SYSTEM']->constructRollbackWhereClause($where, 'ct');
			$where = $GLOBALS['SQ_SYSTEM']->constructRollbackWhereClause($where, 'cl');

			$sql = 'SELECT COUNT(DISTINCT ct.linkid)
					FROM '.SQ_TABLE_RUNNING_PREFIX.'asset_link_tree pt,
						'.SQ_TABLE_RUNNING_PREFIX.'asset_link_tree ct INNER JOIN '.SQ_TABLE_RUNNING_PREFIX.'asset_link cl ON ct.linkid = cl.linkid '.$where;
			$moving_under = $db->getOne($sql);
			if (DB::isError($moving_under)) {
				trigger_error($moving_under->getMessage().'<br/>'.$moving_under->getUserInfo(), E_USER_WARNING);
				return 'Database Error';
			} else if ($moving_under) {
				return 'You are not allowed to move asset "'.$minor->name.'" (ID : #'.$minor->id.') under  "'.$this->name.'" (ID : #'.$this->id.') because this will cause the asset to be nested under itself';
			}// end if
		}// end if

		return true;

	}// end canCreateLink()


	/**
	* Checks whether a link of a certain type can be made to this asset
	* Returns TRUE if link allowed and a string containing the error if it is not
	*
	* @param string $type_code		the type code that we want to check
	* @param string $link_type		the type of link to check
	* @param int	$ignore_linkid	if it gets down to the checking of the cardinality,
	*								ignore the link represented by this link id
	* @param int	$exclusive		the exclusive status of the link
	*
	* @return mixed TRUE or error message string
	* @access public
	*/
	function canLinkToType($type_code, $link_type, $ignore_linkid=0, $exclusive=0)
	{
		// get the minor assets parents and add it's type to the front of the indexed array
		$types = $GLOBALS['SQ_SYSTEM']->am->getTypeAncestors($type_code);
		array_unshift($types, $type_code);

		$allowed_links = $this->_getAllowedLinks();

		// we will be ascending up the parent tree from the current asset type.
		// that way the major asset can have specific cardinality for different assets types
		// EG, assets => 'M', user => '1' -> this means that many assets can be linked to this asset
		//     and only one user, but because 'user' is an 'asset' we need to check for any 'user' references
		//     before we check for any 'asset' references
		$type = '';
		for($i = 0; $i < count($types); $i++) {
			$type = $types[$i];
			if (!empty($allowed_links[$link_type][$type])) break;
		}

		if (empty($allowed_links[$link_type][$type])) {
			require_once SQ_INCLUDE_PATH.'/general_occasional.inc';
			return 'Assets of Type "'.$type_code.'" cannot be "'.link_type_name($link_type).'" linked to a "'.$this->type().'"';
		}

		if (!($link_type & SQ_SC_LINK_SIGNIFICANT) && $exclusive) {
			return 'In order for a link to be exclusive it must also be a significant link';
		}

		if (!$exclusive && !empty($allowed_links[$link_type][$type]['exclusive'])) {
			require_once SQ_INCLUDE_PATH.'/general_occasional.inc';
			return 'Assets of Type "'.$type_code.'" must be "'.link_type_name($link_type).'" exclusivly linked to a "'.$this->type().'"';
		}

		//// Now we need to check that the cardinality is kept valid ////

		// if we are only allowed up to a certain number of these links
		if ($allowed_links[$link_type][$type]['card'] != 'M') {
			$num_curr_links = $this->countLinks('major', $link_type, $type, true, $ignore_linkid);
			//  and we already have our quota of links, bugger off
			if ($num_curr_links >= (int) $allowed_links[$link_type][$type]['card']) {
				require_once SQ_INCLUDE_PATH.'/general_occasional.inc';
				return 'Assets of Type "'.$this->type().'" can only have '.((int) $allowed_links[$link_type][$type]['card']).' "'.link_type_name($link_type).'" link to a "'.$type.'"';
			}
		}//end if

		// if we get this far all is OK
		return true;

	}// canLinkToType()


	/**
	* Creates a link between this asset and the passed asset, of a certain type
	* and returns the linkid of the created link
	*
	* @param object Asset	$minor		the minor asset that we are linking to
	* @param string			$link_type	the type of link this is
	* @param string			$value		the value that is to be associated with this link
	* @param string			$sort_order	the position in the links list that this link should take,
	*									if null or less than zero places at end of list
	* @param string			$dependant	'0' / '1' on whether the this asset is dependant on the
	*									asset that will be linked by this new link
	* @param string			$exclusive	'0' / '1' on whether the this asset is linked exclusivly
	*									to the asset that will be linked by this new link
	*
	* @return int
	* @access public
	*/
	function createLink(&$minor, $link_type, $value='', $sort_order=null, $dependant='0', $exclusive='0')
	{
		if (!$this->id) return 0;
		require_once SQ_FUDGE_PATH.'/db_extras/db_extras.inc';

		if (!is_a($minor, 'Asset')) {
			trigger_error('Minor is not an asset, unable to create link', E_USER_WARNING);
			return 0;
		}

		$link_type = (int) $link_type;

		if (!($link_type & SQ_SC_LINK_SIGNIFICANT) && $dependant) {
			trigger_error('In order for a link to be dependant it must also be a significant link', E_USER_WARNING);
			return 0;
		}

		// First, we should check that we don't already have a link of this type
		$current_links = $this->getLinks($link_type, $minor->type());
		$num_curr_links = count($current_links);
		for($i = 0; $i < $num_curr_links; $i++) {
			// it already exists, so we don't need to do anything
			if ($current_links[$i]['minorid'] == $minor->id && $current_links[$i]['value'] == $value) {
				// if we are trashing the asset and it is already in the trash, just
				// pretend that we created the link
				$trash = $GLOBALS['SQ_SYSTEM']->am->getSystemAsset('trash_folder');
				if ($trash->id == $this->id) return true;

				trigger_error('Link Already Exists', E_USER_WARNING);
				return $current_links[$i]['linkid'];
			}
		}//end for


		// check if we are allowed to link to these type of assets
		if (($err_msg = $this->canCreateLink($minor, $link_type, $exclusive)) !== true) {
			trigger_error($err_msg, E_USER_WARNING);
			return 0;
		}

		$GLOBALS['SQ_SYSTEM']->doTransaction('BEGIN');
		$db = &$GLOBALS['SQ_SYSTEM']->db;

		// make sure the sort order is in a valid range
		$sql   = 'SELECT COUNT(*) as count, MAX(sort_order) as max
				  FROM '.SQ_TABLE_RUNNING_PREFIX.'asset_link ';
		$where = 'majorid = '.$db->quote($this->id);
		$where = $GLOBALS['SQ_SYSTEM']->constructRollbackWhereClause($where);

		$row = $db->getRow($sql.$where);
		if (DB::isError($row)) {
			trigger_error($row->getMessage().'<br/>'.$row->getUserInfo(), E_USER_WARNING);
			$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
			return 0;
		}

		$max = ($row['count'] > 0) ? (int) $row['max'] + 1 : 0;
		if (is_null($sort_order) || (int) $sort_order > (int) $max || (int) $sort_order < 0) $sort_order = (int) $max;

		// sequences are cool, see the PEAR::DB docs for more info
		$linkid = $db->nextId('sq_sequence_asset_link');
		if (DB::isError($linkid)) {
			trigger_error($linkid->getMessage().'<br/>'.$linkid->getUserInfo(), E_USER_WARNING);
			$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
			return 0;
		}

		// if this is a significant link, then do stuff with the tree
		if ($link_type & SQ_SC_LINK_SIGNIFICANT) {
			require_once SQ_INCLUDE_PATH.'/general_occasional.inc';
			//// UPDATE THE TREE TABLE ////

			// Get one of the treeids that we have
			$sql   = 'SELECT t.treeid
					  FROM '.SQ_TABLE_RUNNING_PREFIX.'asset_link_tree t
					  INNER JOIN '.SQ_TABLE_RUNNING_PREFIX.'asset_link l ON t.linkid = l.linkid ';
			$where = 'l.minorid = '.$db->quote($this->id);
			$where = $GLOBALS['SQ_SYSTEM']->constructRollbackWhereClause($where, 't');
			$where = $GLOBALS['SQ_SYSTEM']->constructRollbackWhereClause($where, 'l');
			$sql .= $where.' LIMIT 1';

			$existing_treeid = $db->getOne($sql);
			if (DB::isError($existing_treeid)) {
				trigger_error($existing_treeid->getMessage().'<br/>'.$existing_treeid->getUserInfo(), E_USER_WARNING);
				$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
				return 0;
			}

			// if we aren't linked anywhere we can't be linked to, sorry
			// NOTE: exception to rule is root folder
			if ((string) $existing_treeid == '' && $this->id != 1) {
				trigger_error('Unable to create link, Asset "'.$this->name.'" (Asset #'.$this->id.') is not linked to anything itself', E_USER_WARNING);
				$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
				return 0;
			}

			// OK, what we are going to get a treeid and then do a "INSERT INTO ... SELECT FROM"
			// into the tree table of all the links that are under the minor asset
			$sql   = 'SELECT t.treeid, t.num_immediate_kids
					  FROM '.SQ_TABLE_RUNNING_PREFIX.'asset_link_tree t
					  INNER JOIN '.SQ_TABLE_RUNNING_PREFIX.'asset_link l ON t.linkid = l.linkid ';
			$where = 'l.minorid = '.$db->quote($minor->id);
			$where = $GLOBALS['SQ_SYSTEM']->constructRollbackWhereClause($where, 't');
			$where = $GLOBALS['SQ_SYSTEM']->constructRollbackWhereClause($where, 'l');
			$sql .= $where.' LIMIT 1';

			$minor_tree = $db->getRow($sql);
			if (DB::isError($minor_tree)) {
				trigger_error($minor_tree->getMessage().'<br/>'.$minor_tree->getUserInfo(), E_USER_WARNING);
				$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
				return 0;
			}

			$existing_treeid = (string) $existing_treeid;
			if (empty($minor_tree)) {
				$minor_tree = Array('treeid' => '', 'num_immediate_kids' => 0);
			} else {
				$minor_tree['treeid'] = (string) $minor_tree['treeid'];
				$minor_tree['num_immediate_kids'] = (int) $minor_tree['num_immediate_kids'];
			}

			// when links are deleted their entries in the tree table are set to have a linkid of zero
			// (see deleteLink()) so we can attempt to find unused treeid's that are forming gaps
			$sql   = 'SELECT ct.treeid
					  FROM '.SQ_TABLE_RUNNING_PREFIX.'asset_link_tree ct ';
			$where = 'ct.treeid LIKE '.$db->quote($existing_treeid.'%').'
					  AND ct.treeid > '.$db->quote($existing_treeid).'
					  AND CHARACTER_LENGTH(ct.treeid) = '.(strlen($existing_treeid) + SQ_CONF_ASSET_TREE_SIZE).'
					  AND ct.linkid = 0';
			$where = $GLOBALS['SQ_SYSTEM']->constructRollbackWhereClause($where, 'ct');
			$sql .= $where.' LIMIT 1';

			$free_childid = $db->getOne($sql);
			if (DB::isError($free_childid)) {
				trigger_error($free_childid->getMessage().'<br/>'.$free_childid->getUserInfo(), E_USER_WARNING);
				$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
				return 0;
			}

			// No free children ? generate one from the max value
			if ((string) $free_childid == '') {
				$sql   = 'SELECT MAX(ct.treeid) FROM '.SQ_TABLE_RUNNING_PREFIX.'asset_link_tree ct ';
				$where = 'ct.treeid LIKE '.$db->quote($existing_treeid.'%').'
						  AND ct.treeid > '.$db->quote($existing_treeid).'
						  AND CHARACTER_LENGTH(ct.treeid) = '.(strlen($existing_treeid) + SQ_CONF_ASSET_TREE_SIZE);
				$where = $GLOBALS['SQ_SYSTEM']->constructRollbackWhereClause($where, 'ct');

				$free_childid = $db->getOne($sql.$where);
				if (DB::isError($free_childid)) {
					trigger_error($free_childid->getMessage().'<br/>'.$free_childid->getUserInfo(), E_USER_WARNING);
					$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
					return 0;
				}

				// No max ? we must be the first
				if ((string) $free_childid == '') {
					if (($free_childid = asset_link_treeid_convert(0, true)) === false) {
						$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
						return 0;
					}

				// we found a max so increment it
				} else {
					// get only the child part
					$free_childid = substr($free_childid, -1 * SQ_CONF_ASSET_TREE_SIZE);

					if (($child_num    = asset_link_treeid_convert($free_childid, false)) === false) {
						$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
						return 0;
					}
					if (($free_childid = asset_link_treeid_convert($child_num + 1, true)) === false)  {
						$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
						return 0;
					}
				}// end if

			// else there was a free entry
			} else {

				// get only the child part
				$free_childid = substr($free_childid, -1 * SQ_CONF_ASSET_TREE_SIZE);

				// remove the current zeroed entries in the tree table,
				// as we will be inserting over the top of them

				// because the root folder is not a minor party in any links we have
				// to something special for it
				$where_cond = '';
				if ($this->id == 1) {
					$where_cond = 'treeid = '.$db->quote($free_childid);
				} else {
					$where_cond = 'treeid IN (~SQ0~)';

					$concat = ($db->phptype == 'mysql')
								? 'CONCAT(t.treeid, '.$db->quote($free_childid).')'
								: 't.treeid || '.$db->quote($free_childid);
					$sub_where = 'l.minorid = '.$db->quote($this->id);
					$sub_where = $GLOBALS['SQ_SYSTEM']->constructRollbackWhereClause($sub_where, 't');
					$sub_where = $GLOBALS['SQ_SYSTEM']->constructRollbackWhereClause($sub_where, 'l');
					$subs = Array(' SELECT '.$concat.'
									FROM '.SQ_TABLE_RUNNING_PREFIX.'asset_link_tree t INNER JOIN '.SQ_TABLE_RUNNING_PREFIX.'asset_link l ON t.linkid = l.linkid '.$sub_where);
					$where_cond = db_extras_subquery($db, $where_cond, $subs);
					if (DB::isError($where_cond)) {
						trigger_error($where_cond->getMessage().'<br/>'.$where_cond->getUserInfo(), E_USER_WARNING);
						$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
						return 0;
					}

				}

				if (!$GLOBALS['SQ_SYSTEM']->rollbackDelete('asset_link_tree', $where_cond)) {
					$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
					return 0;
				}

			}// end if

			// if we don't have any existing tree entries then
			// we are the root folder, so do a simple insert
			if ((string) $existing_treeid == '') {
				$values = Array('treeid'             => $free_childid,
								'linkid'             => $linkid,
								'num_immediate_kids' => $minor_tree['num_immediate_kids']
								);
				if (!$GLOBALS['SQ_SYSTEM']->rollbackInsert('asset_link_tree', $values)) {
					$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
					return 0;
				}

			// we have existing tree entries, do a insert..select to create entries for them all
			} else {
				// so much for SQL standards...
				$concat = ($db->phptype == 'mysql')
							? 'CONCAT(t.treeid, '.$db->quote($free_childid).')'
							: '(t.treeid || '.$db->quote($free_childid).')';

				$where = 'l.minorid = '.$db->quote($this->id);
				// note that we dont prep this query for rollback because its getting run
				// by the rollback query functions already
				$select_from = SQ_TABLE_PREFIX.'asset_link_tree t INNER JOIN '.SQ_TABLE_PREFIX.'asset_link l ON t.linkid = l.linkid';
				$values = Array('treeid'             => $concat,
								'linkid'             => $db->quote($linkid),
								'num_immediate_kids' => $db->quote($minor_tree['num_immediate_kids'])
								);
				if (!$GLOBALS['SQ_SYSTEM']->rollbackInsertSelect('asset_link_tree', $select_from, $values, $where)) {
					$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
					return 0;
				}
			}// end if

			// Update the parents to tell them that they are have one kid more
			$where_cond = 'treeid IN (~SQ0~)';
			$sub_where = 't.linkid = '.$db->quote($linkid);
			$sub_where = $GLOBALS['SQ_SYSTEM']->constructRollbackWhereClause($sub_where, 't');
			$subs = Array(' SELECT SUBSTRING(t.treeid FROM 1 FOR (CHARACTER_LENGTH(t.treeid) - '.SQ_CONF_ASSET_TREE_SIZE.'))
							FROM '.SQ_TABLE_RUNNING_PREFIX.'asset_link_tree t '.$sub_where);

			$where_cond = db_extras_subquery($db, $where_cond, $subs);
			if (DB::isError($where_cond)) {
				trigger_error($where_cond->getMessage().'<br/>'.$where_cond->getUserInfo(), E_USER_WARNING);
				$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
				return 0;
			}

			$values = Array('num_immediate_kids' => 'num_immediate_kids + 1');
			if (!$GLOBALS['SQ_SYSTEM']->rollbackUpdate('asset_link_tree', $values, $where_cond)) {
				$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
				return 0;
			}

			// if this minor has already been linked so do a select into
			if ($minor_tree['treeid'] != '') {

				// so much for SQL standards...
				$concat = ($db->phptype == 'mysql')
							? 'CONCAT(pt.treeid, '.$db->quote($free_childid).', SUBSTRING(ct.treeid FROM '.$db->quote(strlen($minor_tree['treeid']) + 1).'))'
							: 'pt.treeid || '.$db->quote($free_childid).' || SUBSTRING(ct.treeid FROM '.$db->quote(strlen($minor_tree['treeid']) + 1).')';

				$where = 'pl.minorid = '.$db->quote($this->id).'
						  AND ct.treeid LIKE '.$db->quote($minor_tree['treeid'].'%').'
						  AND ct.treeid > '.$db->quote($minor_tree['treeid']);
				// note that we dont prep this query for rollback because its getting run
				// by the rollback query functions already
				$select_from = SQ_TABLE_PREFIX.'asset_link_tree pt INNER JOIN '.SQ_TABLE_PREFIX.'asset_link pl ON pt.linkid = pl.linkid,
								'.SQ_TABLE_PREFIX.'asset_link_tree ct';
				$values = Array('treeid'             => $concat,
								'linkid'             => 'ct.linkid',
								'num_immediate_kids' => 'ct.num_immediate_kids'
								);
				if (!$GLOBALS['SQ_SYSTEM']->rollbackInsertSelect('asset_link_tree', $select_from, $values, $where)) {
					$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
					return 0;
				}

			}// end if

		}// end if significant link

		// move the sort_order for all the other assets up one
		$where_cond = ' majorid         = '.$db->quote($this->id).'
						AND sort_order >= '.$db->quote($sort_order);
		$values = Array('sort_order' => 'sort_order + 1');
		if (!$GLOBALS['SQ_SYSTEM']->rollbackUpdate('asset_link', $values, $where_cond)) {
			$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
			return 0;
		}

		// and finally insert the link
		$values = Array('linkid'       => $linkid,
						'majorid'      => $this->id,
						'minorid'      => $minor->id,
						'link_type'    => $link_type,
						'value'        => $value,
						'sort_order'   => $sort_order,
						'dependant'    => (($dependant) ? '1' : '0'),
						'exclusive'    => (($exclusive) ? '1' : '0'),
						'last_updated' => ts_iso8601(time()),
						'last_userid'  => $GLOBALS['SQ_SYSTEM']->currentUserId()
						);
		if (!$GLOBALS['SQ_SYSTEM']->rollbackInsert('asset_link', $values)) {
			$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
			return 0;
		}

		// tell, the asset it has updated
		if (!$this->_linksUpdated()) {
			$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
			return 0;
		}
		if (!$minor->_linksUpdated()) {
			$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
			return 0;
		}

		$GLOBALS['SQ_SYSTEM']->doTransaction('COMMIT');
		$GLOBALS['SQ_SYSTEM']->broadcastEvent('CreateLink', Array('assetid' => $this->id, 'linkid' => $linkid));

		return $linkid;

	}//end createLink()


	/**
	* Determine if we can delete the passed link
	*
	* @param int	$linkid	the link id of the link to remove
	*
	* @return mixed TRUE or error msg string
	* @access public
	*/
	function canDeleteLink($linkid)
	{
		return true;

	}//end canDeleteLink()


	/**
	* Remove a link where this asset is the major player
	*
	* @param int	$linkid	the link id of the link to remove
	*
	* @return boolean
	* @access public
	*/
	function deleteLink($linkid)
	{
		// first, we should try and find the link
		$link = $this->getLinkById($linkid);
		if (empty($link)) {
			trigger_error('Existing Link #'.$linkid.' not found, unable to delete link', E_USER_WARNING);
			return false;
		}

		// if you dont have write access to this asset you cant delete any links
		if (!$this->writeAccess(Array(), false)) {
			trigger_error('Cannot delete link, permission denied', E_USER_WARNING);
			return false;
		}

		// check if we are allowed to delete this link
		if (($err_msg = $this->canDeleteLink($linkid)) !== true) {
			trigger_error($err_msg, E_USER_WARNING);
			return 0;
		}

		require_once SQ_FUDGE_PATH.'/db_extras/db_extras.inc';

		$db = &$GLOBALS['SQ_SYSTEM']->db;

		// open the transaction
		$GLOBALS['SQ_SYSTEM']->doTransaction('BEGIN');

		$minor = &$GLOBALS['SQ_SYSTEM']->am->getAsset($link['minorid'], $link['type_code']);
		if (is_null($minor)) {
			$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
			return false;
		}

		// if this is a significant link
		if ($link['link_type'] & SQ_SC_LINK_SIGNIFICANT) {

			$num_other_links = $minor->countLinks('minor', SQ_SC_LINK_SIGNIFICANT, '', true, $linkid);

			// we create a new link to the trash if this is the last significant link
			// being deleted and if we are not purging the trash
			if (!$num_other_links && !$GLOBALS['SQ_PURGING_TRASH']) {
				// some assets may not be able to have their last significant link
				// deleted, so lets check first before going ahead
				if (!$minor->canDelete()) {
					trigger_error($minor->name.' can not have its last significant link removed' , E_USER_WARNING);
					$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
					return false;
				}

				$trash_folder = &$GLOBALS['SQ_SYSTEM']->am->getSystemAsset('trash_folder');
				if (is_null($trash_folder)) {
					$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
					return false;
				}

				// this asset is destined for the trash, so we need to set its permissions to
				// whatever it is inheriting right now so it keeps them while in the trash
				$perms = Array(SQ_PERMISSION_ADMIN, SQ_PERMISSION_WRITE, SQ_PERMISSION_READ);
				foreach ($perms as $perm) {
					$all_permissions = $minor->getPermission($perm, null, false, true, false, true);
					foreach($all_permissions as $userid => $data) {
						if (empty($data)) continue;
						foreach($data as $assetid => $access) {
							if($assetid == $minor->id) continue;
							// we are deliberatly not checking for the return value here
							// because if for some reason the permission can't be set that is really just
							// bad luck
							@$minor->setPermission($userid, $perm, $access);
						}
					}
				}

				if (!$trash_folder->createLink($minor, SQ_LINK_TYPE_2)) {
					$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
					return false;
				}
			}

			//// UPDATE THE TREE ////

			// Update the parents to tell them that they are going to be one kid less
			$where_cond_string = 'treeid in (~SQ0~)';
			$sub_where = 't.linkid = '.$db->quote($linkid);
			$sub_where = $GLOBALS['SQ_SYSTEM']->constructRollbackWhereClause($sub_where, 't');
			$subs = Array(' SELECT SUBSTRING(t.treeid FROM 1 FOR (CHARACTER_LENGTH(t.treeid) - '.SQ_CONF_ASSET_TREE_SIZE.'))
							FROM '.SQ_TABLE_RUNNING_PREFIX.'asset_link_tree t '.$sub_where);

			$where_cond = db_extras_subquery($db, $where_cond_string, $subs);
			if (DB::isError($where_cond)) {
				$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
				trigger_error($sql->getMessage().'<br/>'.$where_cond->getUserInfo(), E_USER_WARNING);
				return false;
			}

			// add a rollback entry for the tree
			$values = Array('num_immediate_kids' => 'num_immediate_kids - 1');
			if (!$GLOBALS['SQ_SYSTEM']->rollbackUpdate('asset_link_tree', $values, $where_cond)) {
				$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
				return false;
			}

			// We can delete all the links under these nodes because it will be a clean start
			// when we insert into the gap's we create below
			$where_cond_string = 'treeid in (~SQ0~)';
			$concat = ($db->phptype == 'mysql') ? 'CONCAT(pt.treeid, '.$db->quote('%').')' : 'pt.treeid || '.$db->quote('%');
			$sub_where = 'pt.linkid = '.$db->quote($linkid).'
						  AND ct.treeid LIKE '.$concat.'
						  AND ct.treeid > pt.treeid';
			$sub_where = $GLOBALS['SQ_SYSTEM']->constructRollbackWhereClause($sub_where, 'pt');
			$sub_where = $GLOBALS['SQ_SYSTEM']->constructRollbackWhereClause($sub_where, 'ct');
			$subs = Array('SELECT ct.treeid
							FROM '.SQ_TABLE_RUNNING_PREFIX.'asset_link_tree pt, '.SQ_TABLE_RUNNING_PREFIX.'asset_link_tree ct
							'.$sub_where);

			$where_cond = db_extras_subquery($db, $where_cond_string, $subs);
			if (DB::isError($where_cond)) {
				$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
				trigger_error($where_cond->getMessage().'<br/>'.$where_cond->getUserInfo(), E_USER_WARNING);
				return false;
			}

			if (!$GLOBALS['SQ_SYSTEM']->rollbackDelete('asset_link_tree', $where_cond)) {
				$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
				return false;
			}

			// We are going to set the treeid nodes that this link is associated
			// with to zero so that we can find it as a gap when we createLink() later on
			$where = 'linkid = '.$db->quote($linkid);
			$values = Array('linkid' => $db->quote('0'),
							'num_immediate_kids' => $db->quote('0')
							);
			if (!$GLOBALS['SQ_SYSTEM']->rollbackUpdate('asset_link_tree', $values, $where)) {
				$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
				return false;
			}

		}// end if significant link

		// move 'em up, higher
		$where_cond = ' majorid        = '.$db->quote($this->id).'
						AND sort_order > '.$db->quote($link['sort_order']);
		$values = Array('sort_order' => 'sort_order - 1');
		if (!$GLOBALS['SQ_SYSTEM']->rollbackUpdate('asset_link', $values, $where_cond)) {
			$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
			return false;
		}

		$where_cond = ' linkid  = '.$db->quote($linkid).'
						AND majorid = '.$db->quote($this->id);
		if (!$GLOBALS['SQ_SYSTEM']->rollbackDelete('asset_link', $where_cond)) {
			$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
			return false;
		}

		// tell, the asset it has updated
		if (!$this->_linksUpdated()) {
			$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
			return false;
		}
		if (!$minor->_linksUpdated()) {
			$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
			return 0;
		}

		// if this is a web path link, then we need to inform the asset to update it's lookups
		if ($link['link_type'] & SQ_SC_LINK_WEB_PATHS) {
			if (!$minor->updateLookups()) {
				$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
				return 0;
			}
		}

		$GLOBALS['SQ_SYSTEM']->doTransaction('COMMIT');
		$GLOBALS['SQ_SYSTEM']->broadcastEvent('DeleteLink', Array('assetid' => $this->id, 'linkid' => $linkid));

		return true;

	}//end deleteLink()


	/**
	* Move a link to a new parent
	*
	* @param int	$linkid			the link id of the link to move
	* @param int	$to_parentid	the asset id of the new parent asset
	* @param int	$link_type		the type of the new link
	* @param int	$to_parent_pos	the position under the new parent
	*
	* @return boolean
	* @access public
	*/
	function moveLink($linkid, $to_parentid, $link_type, $to_parent_pos)
	{
		// first, we should try and find the link
		$link = $this->getLinkById($linkid);
		if (empty($link)) {
			trigger_error('Existing Link #'.$linkid.' not found, unable to move link', E_USER_WARNING);
			return false;
		}

		// now check that the new parent exists
		$new_parent = &$GLOBALS['SQ_SYSTEM']->am->getAsset($to_parentid);
		if (!$new_parent->id) {
			trigger_error('Unable to Move Asset, new parent (Asset #"'.$to_parentid.'") not found', E_USER_WARNING);
			return false;
		}

		// cant link to the new parent if it is in the trash
		if ($GLOBALS['SQ_SYSTEM']->am->assetInTrash($new_parent->id, true)) {
			trigger_error('Unable to Move Asset, new parent (Asset #"'.$new_parent->id.'") is in the trash');
			return;
		}

		// if you dont have write access to this asset you cant delete any links
		if (!$this->writeAccess(Array(), false)) {
			trigger_error('Unable to Move Asset, permission denied to old parent "'.$this->name.'"', E_USER_WARNING);
			return false;
		}

		// if you dont have write access to the new parent asset you cant create any links
		if (!$new_parent->writeAccess(Array(), false)) {
			trigger_error('Unable to Move Asset, permission denied to new parent "'.$new_parent->name.'"', E_USER_WARNING);
			return false;
		}

		// this is the asset we are moving
		$minor = &$GLOBALS['SQ_SYSTEM']->am->getAsset($link['minorid']);
		if (!$minor->id) {
			trigger_error('Unable to Move Asset, minor (Asset #"'.$link['minorid'].'") not found', E_USER_WARNING);
			return false;
		}

		$db = &$GLOBALS['SQ_SYSTEM']->db;

		// open the transaction
		$GLOBALS['SQ_SYSTEM']->doTransaction('BEGIN');

		if (!$new_parent->acquireLock()) {
			$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
			return false;
		}
		$new_linkid = $new_parent->createLink($minor, $link_type, '', $to_parent_pos);
		$new_parent->releaseLock();
		if (!$new_linkid) {
			$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
			return false;
		}

		// Now let's try and delete the old link
		if (!$this->acquireLock()) {
			$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
			return false;
		}
		$deleted = $this->deleteLink($linkid);
		$this->releaseLock();
		if (!$deleted) {
			$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
			return false;
		}

		$GLOBALS['SQ_SYSTEM']->doTransaction('COMMIT');

		return true;

	}//end moveLink()


	/**
	* Move a link from one position in the assets links order to another
	*
	* @param int	$linkid		the link id of the link to move about
	* @param int	$sort_order	the position in the links list that this link should take,
	*							if less than zero places at end of list
	*
	* @return boolean
	* @access public
	*/
	function moveLinkPos($linkid, $sort_order=-1)
	{
		// open the transaction
		$GLOBALS['SQ_SYSTEM']->doTransaction('BEGIN');
		if ($this->_updateLink($linkid, null, null, $sort_order) && $this->_linksUpdated()) {
			$GLOBALS['SQ_SYSTEM']->doTransaction('COMMIT');
			return true;
		} else {
			$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
			return false;
		}

	}//end moveLinkPos()


	/**
	* Allow any required processesing to occur when any link is updated for the asset
	*
	* Called whenever any type of link is changed. Allows caching of objects, links and other stuff
	* to occur but not cause integrity issues during script execution.
	*
	* @return boolean
	* @access private
	*/
	function _linksUpdated()
	{
		if (!$this->_updated()) return false;
		unset($this->_tmp['dependant_links']);
		return true;

	}//end _linksUpdated()


	/**
	* Update the details of an existing link
	*
	* If any of the detail vars are NULL they are not updated.<br/>
	* <b>NOTE:</b> No transaction stuff is put around the SQL calls in the fn. That sould be done externally.
	*
	* @param int	$linkid		the link id of the link to update
	* @param int	$link_type
	* @param string $value
	* @param int	$sort_order	the position in the links list that this link should take,
	*							if less than zero places at end of list
	*
	* @return boolean
	* @access public
	*/
	function _updateLink($linkid, $link_type=null, $value=null, $sort_order=null)
	{
		// First, we should try and find the link
		$link = $this->getLinkById($linkid);
		if (empty($link)) {
			trigger_error('Existing Link #'.$linkid.' not found, unable to update link', E_USER_WARNING);
			return false;
		}

		$db = &$GLOBALS['SQ_SYSTEM']->db;

		$set_clauses = Array();

		$link_type_changed	= (!is_null($link_type)		&& $link['link_type']	!= $link_type	);
		$value_changed		= (!is_null($value)			&& $link['value']		!= $value		);
		$sort_order_changed	= (!is_null($sort_order)	&& $link['sort_order']	!= $sort_order	);

		$link_type  = (int) $link_type;
		$sort_order = (int) $sort_order;

		$minor = &$GLOBALS['SQ_SYSTEM']->am->getAsset((int) $link['minorid']);

		if ($link_type_changed) {

			// We need to make sure that we aren't going to have to be fucking
			// about with tree to get the update working
			// so because the tree only contains significant links if the
			// significant state has changed... barf
			$current_is_sig = (bool) ((int) $link['link_type'] & SQ_SC_LINK_SIGNIFICANT);
			$new_is_sig     = (bool) ($link_type & SQ_SC_LINK_SIGNIFICANT);
			if ($current_is_sig !== $new_is_sig) {
				trigger_error('Unable to update link #'.$linkid.' , you cannot change a link from being a significant link to a non-significant link or vice versa', E_USER_WARNING);
				return false;
			}

			if (($err_msg = $this->canLinkToType($minor->type(), $link_type)) !== true) {
				trigger_error('Unable to update link #'.$linkid.' ('.$err_msg.')', E_USER_WARNING);
				return false;
			}

			// check for web path conflicts with the new parents children
			// if the old link is not a web path link but the new one is
			if (!($link['link_type'] & SQ_SC_LINK_WEB_PATHS) && ($link_type & SQ_SC_LINK_WEB_PATHS)) {
				$paths = $minor->getWebPaths();
				$bad_paths = $this->_webPathsInUse($paths, $minor->id);
				if (!empty($bad_paths)) {
					trigger_error('Cannot update link #'.$linkid.', path(s) "'.implode('", "', $bad_paths).'" already in use by one of the siblings of "'.$this->name.'"', U_USER_WARNING);
					return false;
				}
			}

			$set_clauses['link_type'] = $db->quote($link_type);

		}// end if link_type_changed

		if ($value_changed) $set_clauses['value'] = $db->quote($value);

		if ($sort_order_changed) {
			$where = 'majorid = '.$db->quote($this->id);
			$where = $GLOBALS['SQ_SYSTEM']->constructRollbackWhereClause($where);
			$sql = 'SELECT COUNT(*) AS count, MAX(sort_order) AS max
					FROM '.SQ_TABLE_RUNNING_PREFIX.'asset_link '.$where;
			$row = $db->getRow($sql);
			if (DB::isError($row)) {
				trigger_error($row->getMessage().'<br/>'.$row->getUserInfo(), E_USER_WARNING);
				return false;
			}
			$max = ($row['count'] > 0) ? (int) $row['max'] : 0;
			if ($sort_order > $max || $sort_order < 0) $sort_order = $max;

			$set_clauses['sort_order'] = $db->quote($sort_order);
		}

		// dickhead check
		if (empty($set_clauses)) return true;

		$set_clauses['last_userid']  = $db->quote($GLOBALS['SQ_SYSTEM']->currentUserId());
		$set_clauses['last_updated'] = $db->quote(ts_iso8601(time()));
		$where = '  linkid      = '.$db->quote($linkid).'
					AND majorid = '.$db->quote($this->id);
		if (!$GLOBALS['SQ_SYSTEM']->rollbackUpdate('asset_link', $set_clauses, $where)) {
			return false;
		}

		if ($link_type_changed) {
			// if this is a web path link or if the old link type was,
			// then we need to inform the asset to update it's lookups
			if (($link_type & SQ_SC_LINK_WEB_PATHS) || ((int) $link['link_type'] & SQ_SC_LINK_WEB_PATHS)) {
				if (!$minor->updateLookups()) {
					return false;
				}
			}
		}

		if ($sort_order_changed) {
			// move 'em up, higher
			if ($link['sort_order'] > $sort_order) {
				$where = '  majorid = '.$db->quote($this->id).'
							AND linkid <> '.$db->quote($linkid).'
							AND sort_order >= '.$db->quote($sort_order).'
							AND sort_order <= '.$db->quote($link['sort_order']);
				$values = Array('sort_order' => 'sort_order + 1');
				if (!$GLOBALS['SQ_SYSTEM']->rollbackUpdate('asset_link', $values, $where)) {
					return false;
				}

			} else {
				$where = '  majorid = '.$db->quote($this->id).'
							AND linkid <> '.$db->quote($linkid).'
							AND sort_order >= '.$db->quote($link['sort_order']).'
							AND sort_order <= '.$db->quote($sort_order);
				$values = Array('sort_order' => 'sort_order - 1');
				if (!$GLOBALS['SQ_SYSTEM']->rollbackUpdate('asset_link', $values, $where)) {
					return false;
				}
			}

		}// end if sort order changed

		return $minor->_updated();

	}//end _updateLink()


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

		$save_vars = Array();
		$attr_ids  = Array();

		$db = &$GLOBALS['SQ_SYSTEM']->db;

		foreach($this->_tmp['vars_set'] as $var_name => $var_data) {
			$attr_id = $this->vars[$var_name]['attributeid'];

			// if this is a unique attribute then let's make sure that this isn't already set
			$owning_attributeid = 0;
			$attribute = &$this->getAttribute($var_name);
			if ($attribute->uniq) {

				$sql = 'SELECT oa.attributeid
						FROM '.SQ_TABLE_PREFIX.'asset_attribute a
						  INNER JOIN '.SQ_TABLE_PREFIX.'asset_attribute oa ON (a.owning_type_code = oa.type_code AND a.name = oa.name)
						WHERE a.name      = '.$db->quote($var_name).'
						  AND a.type_code = '.$db->quote($this->type());

				$owning_attributeid = $db->getOne($sql);
				if (DB::isError($owning_attributeid)) {
					trigger_error($owning_attributeid->getMessage().'<br/>'.$owning_attributeid->getUserInfo(), E_USER_ERROR);
				}

			}// end if

			$attr_value = $this->vars[$var_name]['value'];
			if ($this->vars[$var_name]['type'] == 'serialise') $attr_value = serialize($attr_value);
			$save_vars[$attr_id]['value'] = $attr_value;
			$save_vars[$attr_id]['name']  = $var_name;
			$save_vars[$attr_id]['owning_attrid']  = $owning_attributeid;
			$attr_ids[] = $attr_id;
		}

		// open the transaction
		$GLOBALS['SQ_SYSTEM']->doTransaction('BEGIN');

		// find any previous entries
		$atrr_id_cond = 'IN ('.implode(', ', $attr_ids).')';
		$sql   = 'SELECT attributeid FROM '.SQ_TABLE_RUNNING_PREFIX.'asset_attribute_value ';
		$where = 'assetid = '.$db->quote($this->id).'
					AND attributeid '.$atrr_id_cond;
		$where = $GLOBALS['SQ_SYSTEM']->constructRollbackWhereClause($where);

		$existing = $db->getCol($sql.$where);
		if (DB::isError($existing)) {
			$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
			trigger_error($existing->getMessage().'<br/>'.$existing->getUserInfo(), E_USER_WARNING);
			return false;
		}
		$new = array_diff($attr_ids, $existing);


		// update existing custom attribute values
		if (!empty($existing)) {

			$attr_id_cond = 'IN ('.implode(', ', $existing).')';
			$value_cases = '';
			foreach ($existing as $attr_id) {

				$attr_value = $save_vars[$attr_id]['value'];

				// if this is a unique attribute then let's make sure we register that in the unique value table
				$attribute = &$this->getAttribute($save_vars[$attr_id]['name']);
				if ($attribute->uniq) {

					// update existing entry
					$where = 'owning_attributeid = '.$db->quote($save_vars[$attr_id]['owning_attrid']).'
							    AND assetid = '.$db->quote($this->id);

					$values = Array('custom_value' => $db->quote($attr_value));
					if (!$GLOBALS['SQ_SYSTEM']->rollbackUpdate('asset_attribute_unique_value', $values, $where)) {
						$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
						$attribute->setValue($this->_tmp['vars_set'][$save_vars[$attr_id]['name']]['old_value']);
						return false;
					}

				}// end if

				$value_cases .= '
								WHEN '.$db->quote($attr_id).' THEN '.$db->quote((string)$attr_value);

			}//end foreach

			$value_cases = 'CASE attributeid '.$value_cases.' END';

			$where = 'assetid = '.$db->quote($this->id).' AND attributeid '.$attr_id_cond;
			$values = Array('custom_value' => $value_cases);
			if (!$GLOBALS['SQ_SYSTEM']->rollbackUpdate('asset_attribute_value', $values, $where)) {
				$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
				return false;
			}

		}//end updating existing custom attribute values


		// insert new custom attribute values
		if (!empty($new)) {

			$attr_id_cond = 'IN ('.implode(', ', $new).')';

			$value_cases = '';
			foreach ($new as $attr_id) {

				$attr_value = $save_vars[$attr_id]['value'];

				// if this is a unique attribute then let's make sure we register that in the unique value table
				$attribute = &$this->getAttribute($save_vars[$attr_id]['name']);
				if ($attribute->uniq) {

					// now insert new unique entry
					$values = Array(  'owning_attributeid' => $save_vars[$attr_id]['owning_attrid'],
									  'custom_value'       => $attr_value,
									  'assetid'            => $this->id,
									);
						if (!$GLOBALS['SQ_SYSTEM']->rollbackInsert('asset_attribute_unique_value', $values)) {
							$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
							$attribute->setValue($this->_tmp['vars_set'][$save_vars[$attr_id]['name']]['old_value']);
							return false;
						}

				}// end if

				$value_cases .= ' WHEN '.$db->quote($attr_id).' THEN '.$db->quote((string)$attr_value);
			}//end foreach

			$value_cases = 'CASE attributeid '.$value_cases.' END';

			$where = 'attributeid '.$attr_id_cond;
			$select_from = SQ_TABLE_PREFIX.'asset_attribute';
			$values = Array('assetid'      => $this->id,
							'attributeid'  => 'attributeid',
							'custom_value' => $value_cases
							);
			if (!$GLOBALS['SQ_SYSTEM']->rollbackInsertSelect('asset_attribute_value', $select_from, $values, $where)) {
				$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
				return false;
			}

		}//end new custom attribute values


		// tell, the asset it has updated
		if (!$dont_run_updated && !$this->_updated()) {
			$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
			return false;
		}

		unset($this->_tmp['vars_set']);

		// If we get this far, then it's all OK
		$GLOBALS['SQ_SYSTEM']->doTransaction('COMMIT');

		return true;

	}//end saveAttributes()


	/**
	* Queue the setting of the value of an attribute
	*
	* This function sets the value of the attribute locally, but does not database updating.
	* The change is queued awaiting the call of saveAttributes()
	*
	* @param string		$name	the name of the attribute
	* @param string		$value	the new value of the attribute
	*
	* @return boolean
	* @access public
	* @see saveAttributes()
	*/
	function setAttrValue($name, $value)
	{
		if (empty($this->vars[$name])) {
			trigger_error('Attribute "'.$name.'" not found for Asset of type "'.$this->type().'"', E_USER_WARNING);
			return false;
		}

		$attribute = &$this->getAttribute($name);
		if (!$attribute->setValue($value)) {
			trigger_error('"'.$value.'" is not a valid value for attribute "'.$name.'"', E_USER_WARNING);
			$attribute->setValue($this->vars[$name]['value']);
			return false;
		}

		// dickhead check
		if ($this->vars[$name]['value'] == $value) return true;

		if ($this->vars[$name]['type'] == 'serialise') $value = unserialize($value);

		// if this is a uniq attribute then let's make sure that this isn't already set
		if ($attribute->uniq) {
			$db = &$GLOBALS['SQ_SYSTEM']->db;

			$sql = 'SELECT COUNT(*)
					  FROM '.SQ_TABLE_RUNNING_PREFIX.'asset_attribute_unique_value uv
						INNER JOIN '.SQ_TABLE_RUNNING_PREFIX.'asset_attribute oa ON uv.owning_attributeid = oa.attributeid
						INNER JOIN '.SQ_TABLE_PREFIX.'asset_attribute a ON (oa.type_code = a.owning_type_code AND oa.name = a.name)
					  ';
			$where = ' uv.custom_value = '.$db->quote($value).'
						AND a.name = '.$db->quote($name).'
						AND a.type_code = '.$db->quote($this->type());
			$where = $GLOBALS['SQ_SYSTEM']->constructRollbackWhereClause($where, 'uv');

			$result = $db->getOne($sql.$where);
			if (DB::isError($result)) {
				trigger_error($result->getMessage().'<br/>'.$result->getUserInfo(), E_USER_ERROR);
			}

			if ((int) $result > 0) {
				trigger_error('Unable to set "'.$name.'" it is a unique attribute and another asset already uses "'.$value.'"', E_USER_WARNING);
				$attribute->setValue($this->vars[$name]['value']);
				return false;
			}

		}// end if

		if (!isset($this->_tmp['vars_set'])) $this->_tmp['vars_set'] = Array();
		$this->_tmp['vars_set'][$name]['old_value'] = $this->vars[$name]['value'];

		$this->vars[$name]['value'] = $value;

		return true;

	}//end setAttrValue()


	/**
	* Get the value of an attribute
	*
	* @param string	$name the name of the attribute whos value you want to get
	*
	* @return mixed
	* @access public
	*/
	function attr($name)
	{
		if (!isset($this->vars[$name])) {
			trigger_error('Attribute "'.$name.'" not found for Asset "'.$this->name.'" (# '.$this->id.')', E_USER_WARNING);
			return null;
		}
		return $this->vars[$name]['value'];

	}//end attr()


	/**
	* Returns a reference to the actual attribute object, used for manipulting the object
	*
	* @param string		$name			the name of the attribute
	* @param boolean	$mute_errors	stops the outputting of errors in this fn, needed
	*									because you can't use the '@' operator when
	*									returning by reference
	*
	* @return mixed object Asset_Attribute or NULL
	* @access public
	*/
	function &getAttribute($name, $mute_errors=false)
	{
		if (!isset($this->_tmp['attributes'][$name]) || !is_object($this->_tmp['attributes'][$name])) {

			if (empty($this->_tmp['attributes'])) $this->_tmp['attributes'] = Array();

			if (empty($this->vars[$name])) {
				if (!$mute_errors) trigger_error('Attribute "'.$name.'" not found for Asset"'.$this->type().'"', E_USER_WARNING);
				$this->_tmp['attributes'][$name] = null;
			} else {

				require_once SQ_ATTRIBUTES_PATH.'/'.$this->vars[$name]['type'].'/'.$this->vars[$name]['type'].'.inc';
				$attr_class = 'Asset_Attribute_'.$this->vars[$name]['type'];
				$this->_tmp['attributes'][$name] = new $attr_class($this->vars[$name]['attributeid'], $this->vars[$name]['value']);
				// make sure of the asset is correct
				if (empty($this->_tmp['attributes'][$name]->id)) $this->_tmp['attributes'][$name] = null;

			}//end if

		}//end if

		return $this->_tmp['attributes'][$name];

	}//end getAttribute()


	/**
	* Returns an array of all user or user_group assets that have the passed permission for this asset
	*
	* @param string		$permission		the permission code you are getting
	* @param boolean	$access			Type of Access : null = all, true = granted, false = denied
	* @param boolean	$and_greater	get effective permission (eg read access = read
	*									or write or admin)
	* @param boolean	$expand_groups	expand user groups so only user ids get returned
	*									(NOTE: only valid if $all_info is false)
	* @param boolean	$all_info		when false fn just returns an array of userids for
	*									those that have permission
	*									When true fn returns all info about the permission in the form of
	*									Array(userid => Array(assetid => access))
	*									(NOTE: TRUE is only valid if $and_greater and
	*									$expand_groups are false)
	*
	* @return array()
	* @access public
	*/
	function getPermission($permission, $access=null, $and_greater=true, $expand_groups=false, $all_info=false)
	{
		$permission = (int) $permission;
		// can't be and_greater with workflow
		if ($permission == SQ_PERMISSION_WORKFLOW) $and_greater = false;
		if (!is_null($access)) $access = (bool)$access;

		if (($and_greater || $expand_groups) && $all_info) {
			trigger_error(__CLASS__.'::'.__FUNCTION__.'() - You cannot have either and_greater or expand_groups arguments set to true if you want to have all_info set to true', E_USER_NOTICE);
			$all_info = false;
		}

		// return cached version if we can
		$storage_name =(($and_greater) ? 'effective_' : '').'permission_'.$permission;
		if (!isset($this->_tmp['permission_cache'][$storage_name])) {

			$db = &$GLOBALS['SQ_SYSTEM']->db;


			$sql = '  SELECT DISTINCT assetid, userid, access
					  FROM '.SQ_TABLE_RUNNING_PREFIX.'asset_permission ';
			$where = 'assetid = '.$db->quote($this->id).'
					  AND permission '.(($and_greater) ? '>= ' : '= ').$db->quote($permission);
			$where = $GLOBALS['SQ_SYSTEM']->constructRollbackWhereClause($where);

			$result = $db->getAll($sql.$where);
			if (DB::isError($result)) {
				trigger_error($result->getMessage().'<br/>'.$result->getUserInfo(), E_USER_WARNING);
				return Array();
			}

			// cache the result for next time
			$this->_tmp['permission_cache'][$storage_name] = $result;

		}// end if

		$ret_val = Array();
		foreach($this->_tmp['permission_cache'][$storage_name] as $data) {

			if      ($access === false && $data['access'] != '0') continue;
			else if ($access === true  && $data['access'] != '1') continue;

			// return all info
			if ($all_info) {
				$ret_val[$data['userid']] = $data['access'];

			// else we just want user ids
			} else {

				// if we are expanding user groups and we aren't public access
				if ($expand_groups && $data['userid']) {
					$user = &$GLOBALS['SQ_SYSTEM']->am->getAsset($data['userid']);
					if (!is_null($user)) {
						if (is_a($user, 'user_group')) {
							$ret_val = array_merge($ret_val, $user->getChildren(Array('user'), false));
						} else {
							$ret_val[] = $user->id;
						}
					}
				} else {
					$ret_val[] = $data['userid'];
				}

			}// end if all info

		}// end foreach

		if ($all_info) return $ret_val;
		else           return array_unique($ret_val);

	}//end getPermission()


	/**
	* Sets a permission for the passed user or user_group
	*
	* @param int		$userid		the id of the user or user_group to add the permission for
	* @param string		$permission	the permission code you are adding
	* @param boolean	$access		whether this permission grants access or not
	*
	* @return boolean
	* @access public
	*/
	function setPermission($userid, $permission, $access)
	{
		if (!$this->adminAccess()) {
			trigger_error('Cannot grant permission for "'.$this->name.'", permission denied', E_USER_WARNING);
			return false;
		}

		$userid     = (int)  $userid;
		$permission = (int)  $permission;
		$access     = (bool) $access;

		// check if this permission is already set
		$current = $this->getPermission($permission, $access, false);
		if (in_array($userid, array_values($current))) return true;

		// if we dont have an userid, we are granting public access
		if ($permission != SQ_PERMISSION_WORKFLOW && !empty($userid)) {
			// check that the passed userid is a user or user_group
			$user = &$GLOBALS['SQ_SYSTEM']->am->getAsset($userid, '', true);
			if (!$user->id) {
				trigger_error('Cannot grant permission for an asset that does not exist', E_USER_WARNING);
				return false;
			} else if (!is_a($user, 'user') && !is_a($user, 'user_group')) {
				trigger_error('Cannot grant permission for asset type "'.$user->type().'", not a user or user group', E_USER_WARNING);
				return false;
			}
			$user_name = $user->name;
		} else if ($permission != SQ_PERMISSION_WORKFLOW) {
			$user_name = 'Asset Workflow';
		} else {
			$user_name = 'General Public';
		}

		// check if the reverse access of this permission is set
		$current = $this->getPermission($permission, !$access, false);
		if (in_array($userid, array_values($current))) {
			require_once SQ_INCLUDE_PATH.'/general_occasional.inc';
			$perm_name = permission_type_name($permission);

			$new_access     = ($access) ? 'grant'   : 'revoke';
			$current_access = ($access) ? 'revoked' : 'granted';

			trigger_error('Could not '.$new_access.' '.$perm_name.' access for "'.$user_name.'" on "'.$this->name.'" because this asset already has '.$current_access.' '.$perm_name.' access to "'.$user_name.'"', E_USER_WARNING);
			return false;
		}

		$db = &$GLOBALS['SQ_SYSTEM']->db;
		$GLOBALS['SQ_SYSTEM']->doTransaction('BEGIN');

		// add the permission into the database
		$values = Array('assetid'      => $this->id,
						'userid'       => $userid,
						'permission'   => $permission,
						'access'       => (($access) ? '1' : '0')
						);
		if (!$GLOBALS['SQ_SYSTEM']->rollbackInsert('asset_permission', $values)) {
			$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
			return false;
		}

		// clear the permission cache
		unset($this->_tmp['permission_cache']);
		if (!$this->_updated()) {
			$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
			return false;
		}

		$GLOBALS['SQ_SYSTEM']->doTransaction('COMMIT');
		return true;

	}//end setPermission()


	/**
	* Deletes a permission with the passed user or user_group
	*
	* @param int	$userid			the id of the user or user_group to remove the permission from
	* @param string	$permission		the permission code you are deleting
	*
	* @return boolean
	* @access public
	*/
	function deletePermission($userid, $permission)
	{
		$userid     = (int) $userid;
		$permission = (int) $permission;

		$db = &$GLOBALS['SQ_SYSTEM']->db;

		// if we dont have an userid, we are deleting public access
		if ($permission != SQ_PERMISSION_WORKFLOW && !empty($userid)) {
			// check that the passed userid is a user or user_group
			$user = &$GLOBALS['SQ_SYSTEM']->am->getAsset($userid);
			if (!is_a($user, 'user') && !is_a($user, 'user_group')) {
				trigger_error('Cannot delete permission for asset type "'.$user->type().'", not a user or user group', E_USER_WARNING);
				return false;
			}
		}

		$GLOBALS['SQ_SYSTEM']->doTransaction('BEGIN');

		// delete and add rollback entries
		$where = 'userid         = '.$db->quote($userid).'
				  AND permission = '.$db->quote($permission).'
				  AND assetid    = '.$db->quote($this->id);
		if (!$GLOBALS['SQ_SYSTEM']->rollbackDelete('asset_permission', $where)) {
			$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
			return false;
		}

		// clear the permission cache
		unset($this->_tmp['permission_cache']);
		if (!$this->_updated()) {
			$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
			return false;
		}

		$GLOBALS['SQ_SYSTEM']->doTransaction('COMMIT');
		return true;

	}//end deletePermission()


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

		// if asset is not live or asset does not give public read access
		// we really need to check for write access
		if ($this->status < SQ_STATUS_LIVE ||
			!$this->_checkPermissionAccess(SQ_PERMISSION_READ, Array('0'))) {
			// if we are in rollback vie wmode, bypass extra write access checks or we
			// we always be returning false here (writeAccess returns false)
			if (SQ_ROLLBACK_VIEW) return $this->_checkPermissionAccess(SQ_PERMISSION_WRITE, $assetids);
			return $this->writeAccess($assetids, false);
		}

		return $this->_checkPermissionAccess(SQ_PERMISSION_READ, $assetids);

	}//end readAccess()


	/**
	* Returns true if at least one of the passed assetids has write access to this asset
	*
	* @param array		$assetids	an array of user_group and/or user ids to check for write access
	*								If the array is empty, the current user will be checked
	* @param boolean	$effective	effective access based on locking and status
	* @param boolean	$editing	effective access based on live edit access
	*
	* @return boolean
	* @access public
	*/
	function writeAccess($assetids=Array(), $effective=true, $editing=true)
	{
		if (SQ_ROLLBACK_VIEW) return false;
		if (!$this->id) return true;
		if ($effective && $editing && $this->status == SQ_STATUS_LIVE) return $this->liveEditAccess();
		return $this->checkAccess(SQ_PERMISSION_WRITE, $assetids, $effective);

	}//end writeAccess()


	/**
	* Returns true if at least one of the passed assetids has admin access to this asset
	*
	* @param array		$assetids	an array of user_group and/or user ids to check for write access
	*								If the array is empty, the current user will be checked
	* @param boolean	$effective	effective access based on locking and status
	* @param boolean	$editing	effective access based on live edit access
	*
	* @return boolean
	* @access public
	*/
	function adminAccess($assetids=Array(), $effective=true, $editing=true)
	{
		if (SQ_ROLLBACK_VIEW) return false;
		if (!$this->id) return true;
		if ($effective && $editing && $this->status == SQ_STATUS_LIVE) return $this->liveEditAccess();
		return $this->checkAccess(SQ_PERMISSION_ADMIN, $assetids, $effective);

	}//end adminAccess()


	/**
	* Returns true if the current user can live edit this asset
	* Live edit means the asset is currently SQ_STATUS_LIVE but this
	* user can edit the asset without going through a safe edit
	* NOTE: will return false if asset is not currently live
	*
	* @param boolean	$effective	effective access based on locking and status
	*
	* @return boolean
	* @access public
	*/
	function liveEditAccess($effective=true)
	{
		if (SQ_ROLLBACK_VIEW) return false;
		if (!$this->id) return true;
		if ($this->status != SQ_STATUS_LIVE) return true;

		require_once SQ_INCLUDE_PATH.'/workflow_manager.inc';
		$wfm = new Workflow_Manager($this);

		if (empty($wfm->steps) && empty($wfm->inherited_workflows)) {
			// no workflow set for this asset at all, so anyone with admin
			// access can live edit the asset
			return $this->checkAccess(SQ_PERMISSION_ADMIN, Array(), $effective);
		}

		// if there is workflow defined for this asset, the current user
		// must be the only concerned user to be able to live edit
		$status = $wfm->testPublish($GLOBALS['SQ_SYSTEM']->currentUserId());
		if ($status == 'complete') return $this->checkAccess(SQ_PERMISSION_WRITE, Array(), $effective);

		return false;

	}//end liveEditAccess()


	/**
	* Returns true if this asset is effectivly readable by everyone
	* This function is really just shorthand for getting public read access
	*
	* @return boolean
	* @access public
	*/
	function effectiveUnrestricted()
	{
		return $this->readAccess(Array(0));

	}//end effectiveUnrestricted()


	/**
	* Returns true if at least one of the passed assetids has the passed permission
	* Can perform effective access check based on current status and lock
	* NOTE: Only for write and above access
	*
	* @param int		$perm		the permission to check access for
	* @param array		$assetids	An array of user_group and/or user ids to check for permission
	*								If the array is empty, the current user will be checked
	* @param boolean	$effective	effective access baed on locking and status
	*
	* @return boolean
	* @access public
	*/
	function checkAccess($perm, $assetids=Array(), $effective=true)
	{
		if ($perm == SQ_PERMISSION_READ) {
			trigger_error(__CLASS__.'::'.__FUNCTION__.' is not to be used for read access', E_USER_ERROR);
		}
		if ($effective) {
			// if we are in workflow, no editing allowed
			if ($this->status & SQ_SC_STATUS_PENDING) return false;

			// if we are in a state of total approval, no editing allowed
			if ($this->status & SQ_SC_STATUS_ALL_APPROVED) return false;

			// if the asset is archived, no editing allowed
			if ($this->status & SQ_STATUS_ARCHIVED) return false;

			// effective access is only valid for the current user
			if (!empty($assetids)) return false;

			$lock = $this->getLockInfo();
			// if there is no lock you can't edit
			if (empty($lock)) return false;

			$user = &$GLOBALS['SQ_SYSTEM']->am->getAsset($lock['userid']);
			// if the lock is not owned by the current user they can't edit
			if ($user->id != $GLOBALS['SQ_SYSTEM']->currentUserid()) return false;

			// otherwise they can edit if they have permission access
			return $this->_checkPermissionAccess($perm);

		} else {
			return $this->_checkPermissionAccess($perm, $assetids);
		}

	}//end checkAccess()


	/**
	* Returns true if at least one of the passed assetids has the passed permission
	* Does not check current status of lock
	*
	* @param int	$perm		the permission to check access for
	* @param array	$assetids	An array of user_group and/or user ids to check for permission
	*							If the array is empty, the current user will be checked
	*
	* @return boolean
	* @access public
	*/
	function _checkPermissionAccess($perm, $assetids=Array())
	{
		if (empty($assetids) && $GLOBALS['SQ_SYSTEM']->currentUserId()) {
			// if we are the root user, we can do anything
			if ($GLOBALS['SQ_SYSTEM']->userRoot()) return true;

			// if we are a system administrator, we can do anything
			// unless an asset overwrites this function
			if ($GLOBALS['SQ_SYSTEM']->userSystemAdmin()) return true;

			// use the current user and their groups
			$assetids = $GLOBALS['SQ_SYSTEM']->user->getParents(Array('user', 'user_group'), false);
			$assetids[] = $GLOBALS['SQ_SYSTEM']->currentUserId();
		}

		$grants  = $this->getPermission($perm, true);
		$revokes = $this->getPermission($perm, false);

		// has their access been revoked at all?
		if (!empty($revokes)) {
			$revoked = array_intersect($assetids, $revokes);
			if (!empty($revoked)) return false;
		}

		// is public access granted?
		if (in_array('0', $grants)) return true;

		// have they been granted access?
		$common = array_intersect($assetids, $grants);
		return (!empty($common));

	}//end _checkDbAccess()


	/**
	* Prints out the Frontend for this asset
	* it's up to the kids to override
	*
	* @return void
	* @access public
	*/
	function printFrontend()
	{
		if (!$this->readAccess()) {
			$GLOBALS['SQ_SYSTEM']->paintLogin('Login', 'You do not have permission to access <i>'.$this->name.'</i>');
			return;
		}

		if($this->charset) header("Content-type: text/html; charset=$this->charset");
		if($this->languages) header("Content-language: $this->languages");

		$db = &$GLOBALS['SQ_SYSTEM']->db;
		$url = strip_url(current_url(false));

		$sql = 'SELECT l.frontend_designid, a.type_code
				FROM '.SQ_TABLE_RUNNING_PREFIX.'asset_lookup l, '.SQ_TABLE_RUNNING_PREFIX.'asset a ';
		$where = 'l.frontend_designid = a.assetid
				  AND l.url        = '.$db->quote($url).'
				  AND l.assetid    = '.$db->quote($this->id);
		$where = $GLOBALS['SQ_SYSTEM']->constructRollbackWhereClause($where, 'l');
		$where = $GLOBALS['SQ_SYSTEM']->constructRollbackWhereClause($where, 'a');
		$sql .= $where;

		$result = $db->getRow($sql);
		if (DB::isError($result)) {
			trigger_error($result->getMessage().'<br/>'.$result->getUserInfo(), E_USER_ERROR);
		}

		// we have found the design to use
		if ($result) {
			$design = &$GLOBALS['SQ_SYSTEM']->am->getAsset($result['frontend_designid'], $result['type_code']);
			$design->paint($this);
		// shit we can't find a design, oh well let's just print out our body
		} else {
			$this->printBody();
		}

	}// end printFrontend()


	/**
	* Called by the design to print the body of this asset
	* it's up to the kids to override
	*
	* @access public
	*/
	function printBody()
	{

	}// end printBody()


	/**
	* Prints out the backend interface for the asset
	* it's up to the kids to override
	*
	* @param object Backend_Outputter $o
	*
	* @access public
	*/
	function paintBackend(&$o)
	{
		$o->addHiddenField('asset_action', (($this->id) ? 'edit' : 'create'));
		if ($this->charset) $o->setCharset($this->charset);
		$ei = &$this->getEI();
		$ei->paint($this, $o, empty($this->id));

	}// end paintBackend()


	/**
	* Processes a backend submission from this asset, returns true if all OK
	* Put's the array of processed attributes into $this->_tmp['process_attributes']
	*
	* @param object Backend_Outputter	$o
	* @param array(string)				&$link	information used to create the initial link
	*
	* @return boolean
	* @access public
	*/
	function processBackend(&$o, &$link)
	{
		$ei = &$this->getEI();

		switch($_POST['asset_action']) {
			case 'create' :
				if ($ei->process($this, $o, true)) {
					return (bool) $this->create($link);
				} else {
					return false;
				}
				break;

			default :
				return $ei->process($this, $o, false);

		}// end switch

	}// end processBackend()


	/**
	* Returns the href for getting to the nearest backend for this asset
	*
	* @param string	$screen	the code name of the screen that you want displayed when the asset is loaded
	*
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
	* @param string	$base_url	the url that we are want to match closest to
	*
	* @return string
	* @access public
	*/
	function getWebDataPath()
	{
		return '__data/assets/'.$this->type().'/'.$this->id;

	}//end getWebDataPath()


	/**
	* Returns a list of paths that this asset has
	*
	* @return Array()
	* @access public
	*/
	function getWebPaths()
	{
		if (!isset($this->_tmp['paths'])) {
			$db = &$GLOBALS['SQ_SYSTEM']->db;
			$where = 'assetid = '.$db->quote($this->id);
			$where = $GLOBALS['SQ_SYSTEM']->constructRollbackWhereClause($where);
			$sql = 'SELECT path
					FROM '.SQ_TABLE_RUNNING_PREFIX.'asset_path '.$where.'
					ORDER BY sort_order';

			$result = $db->getCol($sql);
			if (DB::isError($result)) {
				trigger_error($result->getMessage().'<br/>'.$result->getUserInfo(), E_USER_ERROR);
				$this->_tmp['paths'] = Array();
			} else {
				$this->_tmp['paths'] = $result;
			}

		}// end if

		return $this->_tmp['paths'];

	}// end getWebPaths()


	/**
	* Attempts to override all current paths for this asset with those passed in the array
	*
	* @param Array	$paths	array of paths to give this asset
	*
	* @return boolean
	* @access public
	*/
	function saveWebPaths($paths)
	{
		$db = &$GLOBALS['SQ_SYSTEM']->db;

		$current_paths = $this->getWebPaths();
		$save_paths = Array();

		// make sure these paths are compliant
		require_once SQ_INCLUDE_PATH.'/general_occasional.inc';
		$save_paths = make_valid_web_paths($paths);

		// if there is no difference in the arrays (including in the sort order - the index),
		// then there is nothing to do
		if (!array_diff_assoc($save_paths, $current_paths) &&
			!array_diff_assoc($current_paths, $save_paths)) return true;

		//// CHECK INSERTED PATHS ////
		// to make sure that they path name is not in use by any of our siblings
		$inserts = array_diff($save_paths, $current_paths);
		if (!empty($inserts)) {
			// now we need to check that none of our parents have
			// any nav kids that are also using this path name
			$siblingids = Array();
			$parents = $this->getLinks(SQ_SC_LINK_WEB_PATHS, '', null, 'minor');
			for($i = 0; $i < count($parents); $i++) {
				$parent = &$GLOBALS['SQ_SYSTEM']->am->getAsset($parents[$i]['majorid'], $parents[$i]['type_code']);
				if (is_null($parent)) continue;
				$bad_paths = $parent->_webPathsInUse($inserts, $this->id);
				if (!empty($bad_paths)) {
					trigger_error('Path(s) "'.implode('", "', $bad_paths).'" already in use by one of the siblings of "'.$parent->name.'"', E_USER_WARNING);
					return false;
				}
			}
		}// end if inserts

		$GLOBALS['SQ_SYSTEM']->doTransaction('BEGIN');

		// add a rollback entries for the old paths
		$where = 'assetid = '.$db->quote($this->id);
		if (!$GLOBALS['SQ_SYSTEM']->rollbackDelete('asset_path', $where)) {
			$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
			return false;
		}

		// Now insert the paths
		foreach($save_paths as $sort_order => $path) {

			$values = Array('path'       => $path,
							'assetid'    => $this->id,
							'sort_order' => $sort_order);
			if (!$GLOBALS['SQ_SYSTEM']->rollbackInsert('asset_path', $values)) {
				$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
				return false;
			}

		}// end foreach

		unset($this->_tmp['paths']);
		unset($this->_tmp['lookups']);
		unset($this->_tmp['url']);
		unset($this->_tmp['href']);

		if ($this->updateLookups()) {
			$GLOBALS['SQ_SYSTEM']->doTransaction('COMMIT');
			return true;
		} else {
			$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
			return false;
		}

	}// end saveWebPaths()


	/**
	* Returns an array of the passed web paths that are currently in use by any of our siblings
	*
	* @param array		$paths					array of web paths to check
	* @param int		$ignore_assetid			ignore this assetid when checking
	* @param boolean	$return_alternatives	return all paths but replace the bad paths
	*											with a valid alternative
	*
	* @return array(string)
	* @access protected
	*/
	function _webPathsInUse($paths=Array(), $ignore_assetid=0, $return_alternatives=false)
	{
		if (empty($paths)) return Array();
		$return_paths = Array();

		$db = &$GLOBALS['SQ_SYSTEM']->db;

		// get all our nav siblings
		$siblingids = Array();
		$my_kids = $this->getLinks(SQ_SC_LINK_WEB_PATHS, '', null, 'major');
		for($j = 0; $j < count($my_kids); $j++) {
			if ($my_kids[$j]['minorid'] == $ignore_assetid) continue;
			$siblingids[] = $db->quote($my_kids[$j]['minorid']);
		}

		$sibling_paths = Array();

		if (count($siblingids)) {
			$where = 'assetid IN ('.implode(',', $siblingids).')';
			$where = $GLOBALS['SQ_SYSTEM']->constructRollbackWhereClause($where);
			$sql = 'SELECT path FROM '.SQ_TABLE_RUNNING_PREFIX.'asset_path '.$where;

			$sibling_paths = $db->getCol($sql);
			if (DB::isError($sibling_paths)) {
				trigger_error($sibling_paths->getMessage().'<br/>'.$sibling_paths->getUserInfo(), E_USER_ERROR);
				return false;
			}
		}

		foreach ($paths as $path) {
			if (in_array($path, $sibling_paths)) {
				if ($return_alternatives) {
					// find a good path to use
					require_once SQ_FUDGE_PATH.'/general/file_system.inc';
					$new_path = $path;
					while (in_array($new_path, $sibling_paths)) {
						// increment the path using numerials
						$new_path = increment_filename($new_path);
					}
					$path = $new_path;
				}
				$return_paths[] = $path;
			} else if ($return_alternatives) {
				$return_paths[] = $path;
			}
		}

		return $return_paths;

	}//end _webPathsInUse()


	/**
	* Returns a list of lookup information associated with this asset
	*
	* @param string	$field	if only one field is required pass it's name through
	*						(url, root_urlid, frontend_designid)
	*
	* @return Array()
	* @access public
	*/
	function getLookups($field='')
	{
		if (!isset($this->_tmp['lookups'])) {
			$db = &$GLOBALS['SQ_SYSTEM']->db;

			$sql = 'SELECT url, root_urlid, frontend_designid, login_designid
					FROM '.SQ_TABLE_RUNNING_PREFIX.'asset_lookup ';
			$where = 'assetid = '.$db->quote($this->id);
			$where = $GLOBALS['SQ_SYSTEM']->constructRollbackWhereClause($where);

			$result = $db->getAll($sql.$where);
			if (DB::isError($result)) {
				trigger_error($result->getMessage().'<br/>'.$result->getUserInfo(), E_USER_ERROR);
				$this->_tmp['lookups'] = Array();
			} else {
				$this->_tmp['lookups'] = $result;
			}

		}// end if

		if (!$field) {
			return $this->_tmp['lookups'];
		} else {
			$ret_val = Array();
			foreach($this->_tmp['lookups'] as $data) $ret_val[] = $data[$field];
			return $ret_val;
		}

	}// end getLookups()


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
		$db = &$GLOBALS['SQ_SYSTEM']->db;

		unset($this->_tmp['lookups']);
		unset($this->_tmp['url']);
		unset($this->_tmp['href']);

		// Remove all the old URLs
		$where = 'assetid = '.$db->quote($this->id);
		if (!$GLOBALS['SQ_SYSTEM']->rollbackDelete('asset_lookup', $where)) {
			return false;
		}

		// if we have paths then do some url inserting
		if (count($paths)) {

			$design_link = $this->getLink(SQ_LINK_TYPE_3, 'design', false, 'frontend_design');
			$designid = (is_null($design_link)) ? 0 : $design_link['minorid'];
			$login_design_link = $this->getLink(SQ_LINK_TYPE_3, 'design', false, 'login_design');
			$login_designid = (is_null($login_design_link)) ? 0 : $login_design_link['minorid'];
			$parents = $this->getLinks(SQ_SC_LINK_WEB_PATHS, '', null, 'minor');

			$done_urls    = Array();
			$done_parents = Array();

			for($i = 0; $i < count($parents); $i++) {
				if (in_array($parents[$i]['majorid'], $done_parents)) continue;
				$parent = &$GLOBALS['SQ_SYSTEM']->am->getAsset($parents[$i]['majorid'], $parents[$i]['type_code']);
				if (is_null($parent)) continue;

				$parent_urls = $parent->getLookups();
				for($j = 0; $j < count($parent_urls); $j++) {
					for($k = 0; $k < count($paths); $k++) {
						$new_url = $parent_urls[$j]['url'].'/'.$paths[$k];
						if (in_array($new_url, $done_urls)) continue;

						$values = Array('url'              => $new_url,
										'assetid'           => $this->id,
										'root_urlid'        => $parent_urls[$j]['root_urlid'],
										'frontend_designid' => (($designid) ? $designid : $parent_urls[$j]['frontend_designid']),
										'login_designid'    => (($login_designid) ? $login_designid : $parent_urls[$j]['login_designid'])
										);
						if (!$GLOBALS['SQ_SYSTEM']->rollbackInsert('asset_lookup', $values)) {
							return false;
						}

						$done_urls[] = $new_url;
					}// end for paths
				}// end for parent_urls

				$done_parents[] = $parent->id;

			}// end for parents

		}// end if count paths

		return true;

	}// end updateLookups()


	/**
	* Returns a reference to the edit interface for this asset type
	*
	* @return object Asset_Edit_Interface
	* @access public
	*/
	function &getEI()
	{
		if (empty($this->_tmp['ei']) || get_class($this->_tmp['ei']) != 'asset_edit_interface') {
			require_once SQ_INCLUDE_PATH.'/asset_edit_interface.inc';
			$this->_tmp['ei'] = new Asset_Edit_Interface($this->type());
		}
		return $this->_tmp['ei'];

	}// end getEI()


	/**
	* Returns the Edit_Fns object appropriate for whatever asset type we are
	*
	* @return object Asset_Edit_Fns
	* @access public
	*/
	function getEditFns()
	{
		$class_name = $this->type().'_edit_fns';
		require_once SQ_SYSTEM_ROOT.'/'.$GLOBALS['SQ_SYSTEM']->am->getTypeInfo($this->type(), 'dir').'/'.$class_name.'.inc';
		return new $class_name();

	}// end getEditFns()


}//end class

?>