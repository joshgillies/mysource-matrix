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
	* Current status of the asset as an object - for processing
	* @var int
	*/
	var $status_object = null;

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
	* The return value will be:
	*    FALSE if the asset was not created
	*    the ID of the newly created link if the asset and intital link were created
	*    TRUE if the asset was created but $link was empty
	*
	* @param Array	&$link	information used to create the initial link,
	*						Array ('asset' => [ref major asset to create link under],
	*						'link_type' => SQ_LINK_?, 'value' => [link value],
	*						'sort_order' => [link sort order], 'dependant' = [0|1],
	*						'exclusive' = [0|1])
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

		$sql = 'INSERT INTO sq_asset
				(assetid, type_code, name, short_name, status, last_updated, last_userid)
				VALUES
				('.$db->quote($assetid).','.$db->quote($this->type()).', '.$db->quote($name).', '.$db->quote($name).', '.$db->quote(SQ_STATUS_UNDER_CONSTRUCTION).', '.$db->quote(ts_iso8601($now)).', '.$db->quote($userid).')';

		$result = $db->query($sql);
		if (DB::isError($result)) {
			trigger_error($result->getMessage().'<br/>'.$result->getUserInfo(), E_USER_WARNING);
			$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
			return false;
		}

		$this->id           = $assetid;
		$this->name         = $name;
		$this->short_name   = $name;
		$this->status       = SQ_STATUS_UNDER_CONSTRUCTION;
		$this->last_updated = $now;
		$this->last_userid  = $userid;

		if (!empty($this->_tmp['vars_set'])) {
			$save_vars = Array();
			foreach($this->_tmp['vars_set'] as $var_name) {
				$save_vars[$var_name] = $this->vars[$var_name]['value'];
			}
		}

		$this->_loadVars();

		if (!empty($this->_tmp['vars_set'])) {
			foreach($save_vars as $var_name => $var_value) {
				if (!$this->setAttrValue($var_name, $var_value, true)) {
					$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
					$this->id = 0;
					return false;
				}
			}
			unset($this->_tmp['vars_set']);
		}// end if

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

			// dont need to release the lock when creating
			// because chances are they want to edit it anyway

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
	* Creates a duplicate of this asset.
	* The only child assetst that are duplicated are the ones that are DEPENDANTLY linked to this asset
	* NO OTHER linked assets are duplicated
	* Returns a reference to the new object or if an error occured NULL
	*
	* @param Array	&$link				information used to create the initial link,
	*									Array ('asset' => [ref major asset to create link under],
	*									'link_type' => SQ_LINK_?, 'value' => [link value],
	*									'sort_order' => [link sort order], 'dependant' = [boolean],
										'exclusive' = [boolean])
	* @param Array	&$dupe_map			a map for that can be used for re-indexing data held in
	*									assets of the form Array([old asset id] => [new asset id]);
	* @param bool	$dupe_dependants	whether or not to duplicate any assets that are dependantly
	*									linked to this asset, only works if $link is not empty
	* @param bool	$dupe_directory		whether or not to duplicate this assets data directory
	*
	* @return mixed object Asset or NULL
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

		$sql = 'INSERT INTO sq_asset
				(assetid, type_code, name, short_name, status, last_updated, last_userid)
				VALUES
				('.$db->quote($assetid).','.$db->quote($this->type()).', '.$db->quote($this->name.' - Duplicate').', '.$db->quote($this->short_name.' - Duplicate').', '.$db->quote(SQ_STATUS_UNDER_CONSTRUCTION).', '.$db->quote(ts_iso8601($now)).', '.$db->quote($userid).')';

		$result = $db->query($sql);
		if (DB::isError($result)) {
			$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
			trigger_error($result->getMessage().'<br/>'.$result->getUserInfo(), E_USER_WARNING);
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

#			pre_echo("DUPE LINK : ".$this->name);

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
				$dependant_links = $this->getLinks(SQ_LINK_SIGNIFICANT, '', true, 'major', null, 1);

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
	* Can this asset have its last significant link removed (putting it in the trash)?
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
		$links = $this->getLinks(SQ_LINK_SIGNIFICANT, '', true, 'minor');
		if (count($links) != 1) {
			trigger_error('Cannot delete asset, asset must only be linked in the trash', E_USER_WARNING);
			return false;
		}

		$db = &$GLOBALS['SQ_SYSTEM']->db;
		$GLOBALS['SQ_SYSTEM']->doTransaction('BEGIN');

		// say to other assets: 'Delete me bitch cause I'm outa here'
		$links = $this->getLinks(SQ_LINK_ALL, '', true, 'minor');
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

		$sql = 'DELETE FROM sq_asset WHERE assetid = '.$db->quote($this->id);

		$result = $db->query($sql);
		if (DB::isError($result)) {
			trigger_error($result->getMessage().'<br/>'.$result->getUserInfo(), E_USER_WARNING);
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
	* Returns a name to use to describe this instance of this asset
	* should be overridden, used to set $this->name and $this->short_name
	*
	* @param boolean	$short_name	whether or not we are after the shortname or the full name
	*
	* @access private
	*/
	function _getName($short_name=false)
	{
		return ucwords(str_replace('_', ' ', $this->type())).' #'.$this->id;

	}//end _getName()


	/**
	* The type that this asset is
	*
	* @return string
	* @access public
	*/
	function type()
	{
		return get_class($this);

	}//end type()


	/**
	* Returns an array of all assets types that this asset extends from
	*
	* @param boolean	$include_asset	when true adds the "Asset" type to the parent list,
	*									even though it's an unistantiable object
	*
	* @see Asset_Manager::getTypeAncestors()
	* @access public
	*/
	function getTypeAncestors($include_asset=true)
	{
		return $GLOBALS['SQ_SYSTEM']->am->getTypeAncestors($this->type(), $include_asset);

	}//end getTypeAncestors()


	/**
	* Get all asset ids that are above this asset in the various trees in which it exists
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
	* Get all asset ids that are below this asset in the various trees in which it exists
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
	* Should the current user be shown the cached version or not
	*
	* @return boolean
	* @access public
	*/
	function useSystemVersion()
	{
		return (($this->status & SQ_STATUS_SAFE_EDITING) && !$this->writeAccess(Array(), false));

	}//end useSystemVersion()


	/**
	* Loads the asset from the cached version
	* The cached version is saved when the asset status becomes SQ_STATUS_LIVE
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
	* Saves a copy of this asset at this point in time
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
	* Clears the directory for saving the system version
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
	* Replaces the editing version with the system (live) version
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
	* Load the asset represented by the passed asset id
	*
	* @param int	$assetid	the asset id to be loaded
	*
	* @access public
	*/
	function load($assetid)
	{
		$db = &$GLOBALS['SQ_SYSTEM']->db;

		// OK, the first thing to do is check we actually exist :)
		$sql = 'SELECT assetid, name, short_name, status, languages, charset, type_code, last_updated, last_userid
				FROM sq_asset
				WHERE assetid = '.$db->quote($assetid);
		$result = $db->getRow($sql, null, DB_FETCHMODE_ORDERED);
		if (DB::isError($result)) trigger_error($result->getMessage().'<br/>'.$result->getUserInfo(), E_USER_ERROR);

		if (is_null($result)) return;

		list($this->id,
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
	* Sets up the data paths for this asset
	* Also takes into account if we are using a cached version
	*
	* @access private
	*/
	function _loadDataPaths()
	{
		$this->data_path = SQ_DATA_PATH.'/private/assets/'.$this->type().'/'.$this->id;
		$this->data_path_public = SQ_DATA_PATH.'/public/assets/'.$this->type().'/'.$this->id;
		if ($this->useSystemVersion()) $this->data_path .= '/.sq_system';

	}//end _loadDataPaths()


	/**
	* Loads the vars for this asset/ asset type into the vars array
	*
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
			$sql = 'SELECT atr.name, atr.attributeid, atr.type, COALESCE(v.custom_value, atr.default_value) AS value
					FROM (sq_asset a INNER JOIN sq_asset_attribute atr ON a.type_code = atr.type_code)
					  LEFT OUTER JOIN sq_asset_attribute_value v ON atr.attributeid = v.attributeid AND a.assetid = v.assetid
					WHERE a.assetid   = '.$GLOBALS['SQ_SYSTEM']->db->quote($this->id);
		// else just load all defaults
		} else {
			$sql = 'SELECT atr.name, atr.attributeid, atr.type, atr.default_value AS value
					FROM sq_asset_attribute atr
					WHERE atr.type_code   = '.$GLOBALS['SQ_SYSTEM']->db->quote($this->type());
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
	* Set's the last updated info for this asset
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
			$sql = 'UPDATE sq_asset
					SET name         = '.$db->quote($name).',
						short_name   = '.$db->quote($short_name).',
						languages    = '.$db->quote($this->languages).',
						charset      = '.$db->quote($this->charset).',
						last_updated = '.$db->quote(ts_iso8601($last_updated)).',
						last_userid  = '.$db->quote($last_userid).'
					WHERE assetid = '.$db->quote($this->id);

			$result = $db->query($sql);
			if (DB::isError($result)) {
				trigger_error($result->getMessage().'<br/>'.$result->getUserInfo(), E_USER_WARNING);
				return false;
			}

			$old_name         = $this->name;
			$old_short_name   = $this->short_name;
			$old_last_updated = $this->last_updated;
			$old_last_userid  = $this->last_userid;

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

		}// end if $this->id

		$this->name         = $name;
		$this->short_name   = $short_name;
		$this->last_updated = $last_updated;
		$this->last_userid  = $last_userid;

		return true;

	}//end _updated();


	/**
	* Returns a status object for the current asset to be used for processing
	*
	* @return object Asset_Status
	* @access public
	*/
	function &getStatus()
	{
		if (!is_null($this->status_object)) return $status_object;
		
		// work out the name of our status file
		require_once SQ_INCLUDE_PATH.'/general_occasional.inc';
		$status_code = get_bit_names('SQ_STATUS_', $this->status);
		$status_code = 'asset_status_'.strtolower($status_code);
		include_once SQ_INCLUDE_PATH.'/asset_status/'.$status_code.'.inc';
		return new $status_code($this);

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
		if ($this->status == $new_status) {
			// if we are dependant minor assets, we can only change
			// our status to a status higher than or equal to all
			// the statii of our parents
			$dependant_parents = $this->getLinks(SQ_LINK_SIGNIFICANT, '', true, 'minor', null, 1);
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
			trigger_error('processStatusChange failed for asset "'.$this->name.'"', E_USER_WARNING);
			return false;
		}

		$GLOBALS['SQ_SYSTEM']->doTransaction('BEGIN');

		if ($new_status != $this->status) {
			$db = &$GLOBALS['SQ_SYSTEM']->db;
			$sql = 'UPDATE sq_asset
					SET status    = '.$db->quote($new_status).'
					WHERE assetid = '.$db->quote($this->id);

			$result = $db->query($sql);
			if (DB::isError($result)) {
				trigger_error($result->getMessage().'<br/>'.$result->getUserInfo(), E_USER_WARNING);
				$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
				return false;
			}

			$GLOBALS['SQ_SYSTEM']->broadcastEvent('AssetStatusUpdate', 
												  Array('assetid'    => $this->id,
														'old_status' => $this->status,
														'new_status' => $new_status,
														));

			$this->status = $new_status;
			$this->_updated();
		}

		// try and process the change on our dependants
		$dependant_links = $this->getLinks(SQ_LINK_SIGNIFICANT, '', true, 'major', null, 1);
		if (!empty($dependant_links)) {
			$am = &$GLOBALS['SQ_SYSTEM']->am;
			foreach ($dependant_links as $link) {
				$asset = &$am->getAsset($link['minorid']);
				if (!$asset->processStatusChange($new_status)) {
					trigger_error('processStatusChange failed for asset "'.$this->name.'" trying to process dependant child "'.$asset->name.'"', E_USER_WARNING);
					$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
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
	* Remaps existing assetids used to new ones, for ourselves and all our dependants
	*
	* @param array	&$map	a map of the form Array([old asset id] => [new asset id]);
	*
	* @return bool
	* @access public
	*/
	function remapAssetids($map)
	{
		$GLOBALS['SQ_SYSTEM']->doTransaction('BEGIN');
		// try and acquire locks on our dependants
		$dependant_links = $this->getLinks(SQ_LINK_SIGNIFICANT, '', true, 'major', null, 1);
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
	* Morph's this asset to either one of it's parent type or one of it's children types
	* Only succeeds if $type_code asset's is allowed to link to all children that this asset has
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
				FROM sq_asset_type_inherited
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

			$links = $this->getLinks(SQ_LINK_ALL);
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

			$links = $this->getLinks(SQ_LINK_ALL, '', true, 'minor');
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
				$sql = 'UPDATE sq_asset_attribute_value
						SET attributeid = '.$new_id.'
						WHERE assetid = '.$db->quote($this->id).'
						  AND attributeid = '.$current_id;
				$result = $db->query($sql);
				if (DB::isError($result)) {
					trigger_error($result->getMessage().'<br/>'.$result->getUserInfo(), E_USER_WARNING);
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

			// now insert new entry
			$sql = 'DELETE FROM sq_asset_attribute_value
					WHERE assetid = '.$db->quote($this->id).'
					  AND attributeid IN ('.$deletes_attributeids.')';

			$result = $db->query($sql);
			if (DB::isError($result)) {
				trigger_error($result->getMessage().'<br/>'.$result->getUserInfo(), E_USER_WARNING);
				$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
				return false;
			}
		}// end if delete var list

		$sql = 'UPDATE sq_asset
				SET type_code    = '.$db->quote($new_type_code).'
				WHERE assetid = '.$db->quote($this->id);

		$result = $db->query($sql);
		if (DB::isError($result)) {
			trigger_error($result->getMessage().'<br/>'.$result->getUserInfo(), E_USER_WARNING);
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
	* Acquires a lock on this asset and its dependant assets
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
			$sql = 'SELECT assetid FROM sq_asset_lock
					WHERE source_asset = '.$db->quote($current_lock['source_asset']);

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
			}

			// close the queue of messages we opened, which sends all the messages in the queue
			$ms->closeQueue();

			// refresh the lock info
			$current_lock = $this->getLockInfo();

		}// end if

		// is this asset already locked
		if (!empty($current_lock)) {
			$user = &$GLOBALS['SQ_SYSTEM']->am->getAsset($current_lock['userid']);

			require_once SQ_FUDGE_PATH.'/general/datetime.inc';
			$expires_in = easy_time_total(($current_lock['expires'] - time()), true);
			if (!$expires_in) $expires_in = '1 second';

			trigger_error('Cannot acquire lock on "'.$this->name.'", lock already held by "'.$user->name.'"', E_USER_WARNING);
			return false;
		}

		// if we have no source, we are the asset aquiring the locks
		if (empty($source)) $source = $this->id;
		$expires = (is_null($expires)) ? (time() + SQ_CONF_LOCK_LENGTH) : (int) $expires;

		$GLOBALS['SQ_SYSTEM']->doTransaction('BEGIN');

		// try and acquire locks on our dependants
		$dependant_links = $this->getLinks(SQ_LINK_SIGNIFICANT, '', true, 'major', null, 1);
		if (!empty($dependant_links)) {
			$am = &$GLOBALS['SQ_SYSTEM']->am;
			foreach ($dependant_links as $link) {
				$asset = &$am->getAsset($link['minorid']);
				if (!$asset->acquireLock($source, $force, $expires)) {
					$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
					return false;
				}
			}
		}

		// now try and get the lock on ourself
		$sql = 'INSERT INTO sq_asset_lock
				(assetid, source_asset, userid, expires)
				VALUES
				('.$db->quote($this->id).', '.$db->quote($source).',
				 '.$db->quote($GLOBALS['SQ_SYSTEM']->currentUserid()).',
				 '.$db->quote(ts_iso8601($expires)).')';
		$result = $db->query($sql);
		if (DB::isError($result)) {
			$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
			trigger_error($result->getMessage().'<br/>'.$result->getUserInfo(), E_USER_WARNING);
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
		$sql = 'DELETE FROM sq_asset_lock
				WHERE source_asset = '.$db->quote($current_lock['source_asset']);

		$result = $db->query($sql);
		if (DB::isError($result)) {
			$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
			trigger_error($result->getMessage().'<br/>'.$result->getUserInfo(), E_USER_WARNING);
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

		$sql = 'UPDATE sq_asset_lock
				SET expires        = '.$db->quote(ts_iso8601($expires)).'
				WHERE source_asset = '.$db->quote($current_lock['source_asset']);
		$result = $db->query($sql);
		if (DB::isError($result)) {
			$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
			trigger_error($result->getMessage().'<br/>'.$result->getUserInfo(), E_USER_WARNING);
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
		$sql = 'SELECT assetid, source_asset, userid, expires
				FROM sq_asset_lock
				WHERE assetid = '.$db->quote($this->id);

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
			// just get the permissions set for this asset
			$sql = 'SELECT assetid, source_asset, userid, expires
					FROM sq_asset_lock
					WHERE source_asset = '.$db->quote($source_assetid).'
					  AND assetid <> '.$db->quote($this->id);

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

		// just get the permissions set for this asset
		$sql = 'DELETE FROM sq_asset_lock
				WHERE source_asset = '.$db->quote($current_lock['source_asset']).'
				  AND expires < '.$db->quote($now);

		$result = $db->query($sql);
		if (DB::isError($result)) {
			trigger_error($result->getMessage().'<br/>'.$result->getUserInfo(), E_USER_WARNING);
			return Array();
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
	*											are just a $type_code
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
				FROM sq_asset_link l, sq_asset a
				WHERE l.'.$side_of_link.'id = '.$db->quote($this->id).'
				  AND l.'.$other_side.'id  = a.assetid
				  AND l.link_type = '.$db->quote($link_type);

		if (!is_null($exclusive)) $sql .= '  AND l.exclusive = '.$db->quote(($exclusive) ? '1' : '0');

		if ($type_code) {

			$type_code_cond = '';
			if (is_array($type_code)) {
				for($i = 0; $i < count($type_code); $i++) $type_code[$i] = $db->quote($type_code[$i]);
				$type_code_cond = 'IN ('.implode(', ', $type_code).')';
			} else {
				$type_code_cond = '= '.$db->quote($type_code);
			}

			if ($strict_type_code) {
				$sql .= ' AND a.type_code '.$type_code_cond;
			} else {
				require_once SQ_FUDGE_PATH.'/db_extras/db_extras.inc';
				$sql .= ' AND a.type_code IN (~SQ0~)';
				$subs = Array( 'SELECT type_code
								FROM sq_asset_type_inherited
								WHERE inherited_type_code '.$type_code_cond );
				$sql = db_extras_subquery($db, $sql, $subs);
				if (DB::isError($sql))
					trigger_error($sql->getMessage().'<br/>'.$sql->getUserInfo(), E_USER_ERROR);
			}
		}

		if (!is_null($value)) $sql .= '  AND l.value = '.$db->quote($value);

		$sql .= ' ORDER BY l.sort_order';

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
		$sql = 'SELECT  l.linkid, l.'.$other_side.'id, l.value, l.link_type,
						a.type_code, l.sort_order, l.dependant, l.exclusive
				FROM sq_asset_link l, sq_asset a
				WHERE l.linkid = '.$db->quote($linkid).'
				  AND l.'.$other_side.'id = a.assetid';

		if (!$ignore_this) $sql .= ' AND l.'.$side_of_link.'id = '.$db->quote($this->id);

		$result = $db->getRow($sql);
		if (DB::isError($result))
			trigger_error($result->getMessage().'<br/>'.$result->getUserInfo(), E_USER_ERROR);

		return $result;

	}//end getLinkById()


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
		$sql = 'SELECT  l.linkid, l.'.$other_side.'id, l.value, l.link_type,
						a.type_code, l.sort_order, l.dependant, l.exclusive
				FROM sq_asset_link l, sq_asset a
				WHERE l.'.$other_side.'id = a.assetid
				  AND l.'.$side_of_link.'id = '.$db->quote($this->id).'
				  AND (l.link_type & '.$db->quote($link_types).') > 0 ';

		if (!is_null($value))		$sql .= '  AND l.value     = '.$db->quote($value);
		if (!is_null($dependant))	$sql .= '  AND l.dependant = '.$db->quote(($dependant) ? '1' : '0');
		if (!is_null($exclusive))	$sql .= '  AND l.exclusive = '.$db->quote(($exclusive) ? '1' : '0');

		if (!empty($type_code)) {

			$type_code_cond = '';
			if (is_array($type_code)) {
				for($i = 0; $i < count($type_code); $i++) $type_code[$i] = $db->quote($type_code[$i]);
				$type_code_cond = 'IN ('.implode(', ', $type_code).')';
			} else {
				$type_code_cond = '= '.$db->quote($type_code);
			}

			if ($strict_type_code) {
				$sql .= ' AND a.type_code '.$type_code_cond;
			} else {
				require_once SQ_FUDGE_PATH.'/db_extras/db_extras.inc';
				$sql .= ' AND a.type_code IN (~SQ0~)';
				$subs = Array( 'SELECT type_code
								FROM sq_asset_type_inherited
								WHERE inherited_type_code '.$type_code_cond );
				$sql = db_extras_subquery($db, $sql, $subs);
				if (DB::isError($sql))
					trigger_error($sql->getMessage().'<br/>'.$sql->getUserInfo(), E_USER_ERROR);
			}
		}// end if

		$sql .= ' ORDER BY l.sort_order, a.type_code';

		$result = $db->getAll($sql);
		if (DB::isError($result))
			trigger_error($result->getMessage().'<br/>'.$result->getUserInfo(), E_USER_ERROR);

		return $result;

	}//end getLinks()


	/**
	* Returns an array of all the permitted links type, the type asset and the cardinality
	* In the form
	*
	*    Array('[link_type]' => Array('[type_code]' => '[cardinality]));
	* Where:
	*   link_type   = SQ_LINK_TYPE_1|SQ_LINK_TYPE_2|SQ_LINK_TYPE_3|SQ_LINK_NOTICE
	*   cardinality = 1|M
	*
	* @return Array()
	* @access private
	*/
	function _getAllowedLinks()
	{
		return Array(SQ_LINK_TYPE_1 => Array(),
					 SQ_LINK_TYPE_2 => Array(),
					 SQ_LINK_TYPE_3 => Array(),
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
		$extra_where = '';
		if ($link_types) {
			$extra_where .= ' AND (link_type & '.$db->quote($link_types).') > 0';
		}
		if ($type_code) {
			$extra_table .= ', sq_asset a';
			$extra_where .= ' AND l.minorid = a.assetid ';

			$type_code_cond = '';
			if (is_array($type_code)) {
				for($i = 0; $i < count($type_code); $i++) $type_code[$i] = $db->quote($type_code[$i]);
				$type_code_cond = 'IN ('.implode(', ', $type_code).')';
			} else {
				$type_code_cond = '= '.$db->quote($type_code);
			}

			if ($strict_type_code) {
				$extra_where .= ' AND a.type_code '.$type_code_cond;
			} else {
				require_once SQ_FUDGE_PATH.'/db_extras/db_extras.inc';
				$extra_where .= ' AND a.type_code IN (~SQ0~)';
				$subs = Array( 'SELECT type_code
								FROM sq_asset_type_inherited
								WHERE inherited_type_code '.$type_code_cond );
				$extra_where = db_extras_subquery($db, $extra_where, $subs);
				if (DB::isError($sql))
					trigger_error($sql->getMessage().'<br/>'.$sql->getUserInfo(), E_USER_ERROR);
			}// end if
		}// end if
		if ($ignore_linkid) {
			$extra_where .= ' AND l.linkid <> '.$db->quote($ignore_linkid);
		}

		$sql = 'SELECT COUNT(*)
				FROM sq_asset_link l'.$extra_table.'
				WHERE l.'.$side_of_link.'id = '.$db->quote($this->id).$extra_where;
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
	*
	* @return mixed TRUE or error message string
	* @access public
	*/
	function canCreateLink(&$minor, $link_type, $exclusive)
	{
		if (!$this->id) return false;

		// if you dont have write access to this asset you cant create any links
		if (!$GLOBALS['SQ_INSTALL'] && !$this->writeAccess(Array(), true, false)) {
			return 'Cannot create link from '.$this->name.' (# '.$this->id.') to '.$minor->name.' (# '.$minor->id.'), permission denied';
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
		if ($link_type & SQ_LINK_FRONTEND_NAV) {
			$paths = $minor->getWebPaths();
			$bad_paths = $this->_webPathsInUse($paths);
			if (!empty($bad_paths)) {
				return 'Cannot create link from '.$this->name.' (# '.$this->id.') to '.$minor->name.' (# '.$minor->id.'), path(s) "'.implode('", "', $bad_paths).'" already in use by one of the siblings of "'.$this->name.'"';
			}
		}

		// if this link is a significant link, then we need to make sure that this
		// minor asset doesn't already have an exclusive link from anything
		// and that we aren't moving under ourselves
		if ($link_type & SQ_LINK_SIGNIFICANT) {
			//// Check for exclusive ////
			$sql = 'SELECT majorid
					FROM sq_asset_link
					WHERE minorid   = '.$db->quote($minor->id).'
					  AND exclusive = '.$db->quote('1');

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
			$sql = 'SELECT COUNT(DISTINCT ct.linkid)
					FROM sq_asset_link_tree pt,
						sq_asset_link_tree ct INNER JOIN sq_asset_link cl ON ct.linkid = cl.linkid
					WHERE ct.treeid LIKE '.$concat.'
					  AND ct.treeid >= pt.treeid
					  AND pt.linkid IN (~SQ0~)
					  AND cl.minorid = '.$db->quote($this->id);
			$subs = Array('SELECT linkid
							FROM sq_asset_link
							WHERE minorid = '.$db->quote($minor->id));
			$sql = db_extras_subquery($db, $sql, $subs);
			if (DB::isError($sql)) {
				trigger_error($sql->getMessage().'<br/>'.$sql->getUserInfo(), E_USER_WARNING);
				return 'Database Error';
			}
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

		if (!$exclusive && $allowed_links[$link_type][$type]['exclusive']) {
			require_once SQ_INCLUDE_PATH.'/general_occasional.inc';
			return 'Assets of Type "'.$type_code.'" must be "'.link_type_name($link_type).'" exclusivly linked to a "'.$this->type().'"';
		}

		//// Now we need to check that the cardinality is kept valid ////

		// if we are only allowed one of these links
		if ($allowed_links[$link_type][$type]['card'] == 1) {
			$num_curr_links = $this->countLinks('major', $link_type, $type, true, $ignore_linkid);
			//  and we already have our one link, bugger off
			if ($num_curr_links > 0) {
				require_once SQ_INCLUDE_PATH.'/general_occasional.inc';
				return 'Assets of Type "'.$this->type().'" can only have one "'.link_type_name($link_type).'" link to a "'.$type.'"';
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

		if (!($link_type & SQ_LINK_SIGNIFICANT) && $dependant) {
			trigger_error('In order for a link to be dependant it must also be a significant link', E_USER_WARNING);
			return 0;
		}

		if (!($link_type & SQ_LINK_SIGNIFICANT) && $exclusive) {
			trigger_error('In order for a link to be exclusive it must also be a significant link', E_USER_WARNING);
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
		$sql = 'SELECT COUNT(*) as count, MAX(sort_order) as max
				FROM sq_asset_link
				WHERE majorid = '.$db->quote($this->id);
		$row = $db->getRow($sql);
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
		if ($link_type & SQ_LINK_SIGNIFICANT) {
			require_once SQ_INCLUDE_PATH.'/general_occasional.inc';
			//// UPDATE THE TREE TABLE ////

			// Get one of the treeids that we have
			$sql = 'SELECT t.treeid
					FROM sq_asset_link_tree t INNER JOIN sq_asset_link l ON t.linkid = l.linkid
					WHERE l.minorid = '.$db->quote($this->id).'
					LIMIT 1';
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
			$sql = 'SELECT t.treeid, t.num_immediate_kids
					FROM sq_asset_link_tree t INNER JOIN sq_asset_link l ON t.linkid = l.linkid
					WHERE l.minorid = '.$db->quote($minor->id).'
					LIMIT 1';
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
			$sql = 'SELECT ct.treeid
					FROM sq_asset_link_tree ct
					WHERE ct.treeid LIKE '.$db->quote($existing_treeid.'%').'
					  AND ct.treeid > '.$db->quote($existing_treeid).'
					  AND CHARACTER_LENGTH(ct.treeid) = '.(strlen($existing_treeid) + SQ_CONF_ASSET_TREE_SIZE).'
					  AND ct.linkid = 0
					LIMIT 1';
			$free_childid = $db->getOne($sql);
			if (DB::isError($free_childid)) {
				trigger_error($free_childid->getMessage().'<br/>'.$free_childid->getUserInfo(), E_USER_WARNING);
				$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
				return 0;
			}

			// No free children ? generate one from the max value
			if ((string) $free_childid == '') {
				$sql = 'SELECT MAX(ct.treeid)
						FROM sq_asset_link_tree ct
						WHERE ct.treeid LIKE '.$db->quote($existing_treeid.'%').'
						  AND ct.treeid > '.$db->quote($existing_treeid).'
						  AND CHARACTER_LENGTH(ct.treeid) = '.(strlen($existing_treeid) + SQ_CONF_ASSET_TREE_SIZE);
				$free_childid = $db->getOne($sql);
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
				if ($this->id == 1) {
					$sql = 'DELETE FROM sq_asset_link_tree
							WHERE treeid = '.$db->quote($free_childid);
				} else {
					$concat = ($db->phptype == 'mysql')
								? 'CONCAT(t.treeid, '.$db->quote($free_childid).')'
								: 't.treeid || '.$db->quote($free_childid);

					$sql = 'DELETE FROM sq_asset_link_tree
							WHERE treeid IN (~SQ0~)';
					$subs = Array('SELECT '.$concat.'
									FROM sq_asset_link_tree t INNER JOIN sq_asset_link l ON t.linkid = l.linkid
									WHERE l.minorid = '.$db->quote($this->id));
					$sql = db_extras_subquery($db, $sql, $subs);
					if (DB::isError($sql)) {
						trigger_error($sql->getMessage().'<br/>'.$sql->getUserInfo(), E_USER_WARNING);
						$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
						return 0;
					}
				}

				$result = $db->query($sql);
				if (DB::isError($result)) {
					trigger_error($result->getMessage().'<br/>'.$result->getUserInfo(), E_USER_WARNING);
					$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
					return 0;
				}

			}// end if

			$insert = 'INSERT INTO sq_asset_link_tree (treeid, linkid, num_immediate_kids)';

			// if we don't have any existing tree entries then
			// we are the root folder, so do a simple insert
			if ((string) $existing_treeid == '') {
				$sql = $insert.' VALUES ('.$db->quote($free_childid).', '.$db->quote($linkid).', '.$db->quote($minor_tree['num_immediate_kids']).')';

				$result = $db->query($sql);
				if (DB::isError($result)) {
					trigger_error($result->getMessage().'<br/>'.$result->getUserInfo(), E_USER_WARNING);
					$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
					return 0;
				}

			// we have existing tree entries, do a insert..select to create entries for them all
			} else {
				// so much for SQL standards...
				$concat = ($db->phptype == 'mysql')
							? 'CONCAT(t.treeid, '.$db->quote($free_childid).')'
							: 't.treeid || '.$db->quote($free_childid);
				$select = 'SELECT '.$concat.', '.$db->quote($linkid).', '.$db->quote($minor_tree['num_immediate_kids']).'
							FROM sq_asset_link_tree t INNER JOIN sq_asset_link l ON t.linkid = l.linkid
							WHERE l.minorid = '.$db->quote($this->id);

				$result = db_extras_insert_select($db, $insert, $select);
				if (DB::isError($result)) {
					trigger_error($result->getMessage().'<br/>'.$result->getUserInfo(), E_USER_WARNING);
					$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
					return 0;
				}
			}// end if

			// Update the parents to tell them that they are have one kid more
			$sql = 'UPDATE sq_asset_link_tree
					   SET num_immediate_kids = num_immediate_kids + 1
					WHERE treeid IN (~SQ0~)';
			$subs = Array('SELECT SUBSTRING(t.treeid FROM 1 FOR (CHARACTER_LENGTH(t.treeid) - '.SQ_CONF_ASSET_TREE_SIZE.'))
							FROM sq_asset_link_tree t
							WHERE t.linkid = '.$db->quote($linkid));

			$sql = db_extras_subquery($db, $sql, $subs);
			if (DB::isError($sql)) {
				trigger_error($sql->getMessage().'<br/>'.$sql->getUserInfo(), E_USER_WARNING);
				$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
				return 0;
			}

			$result = $db->query($sql);
			if (DB::isError($result)) {
				trigger_error($result->getMessage().'<br/>'.$result->getUserInfo(), E_USER_WARNING);
				$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
				return 0;
			}

			// if this minor has already been linked so do a select into
			if ($minor_tree['treeid'] != '') {

				// so much for SQL standards...
				$concat = ($db->phptype == 'mysql')
							? 'CONCAT(pt.treeid, '.$db->quote($free_childid).', SUBSTRING(ct.treeid FROM '.$db->quote(strlen($minor_tree['treeid']) + 1).'))'
							: 'pt.treeid || '.$db->quote($free_childid).' || SUBSTRING(ct.treeid FROM '.$db->quote(strlen($minor_tree['treeid']) + 1).')';

				// the 'ct.treeid > ' line makes sure that we get only the children of the minor,
				// because we added the tree link to the minor above
				$select = 'SELECT '.$concat.', ct.linkid, ct.num_immediate_kids
							FROM sq_asset_link_tree pt INNER JOIN sq_asset_link pl ON pt.linkid = pl.linkid,
								sq_asset_link_tree ct
							WHERE pl.minorid = '.$db->quote($this->id).'
							  AND ct.treeid LIKE '.$db->quote($minor_tree['treeid'].'%').'
							  AND ct.treeid > '.$db->quote($minor_tree['treeid']);

				$result = db_extras_insert_select($db, $insert, $select);
				if (DB::isError($result)) {
					trigger_error($result->getMessage().'<br/>'.$result->getUserInfo(), E_USER_WARNING);
					$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
					return 0;
				}

				//// PULL ACROSS MINOR ASSET'S PERMISSIONS LOOKUP'S ////

				// so much for SQL standards...
				$concat_start_treeid = ($db->phptype == 'mysql')
					? 'CONCAT(pt.treeid, '.$db->quote($free_childid).', SUBSTRING(ctp.start_treeid FROM '.$db->quote(strlen($minor_tree['treeid']) + 1).'))'
					: 'pt.treeid || '.$db->quote($free_childid).' || SUBSTRING(ctp.start_treeid FROM '.$db->quote(strlen($minor_tree['treeid']) + 1).')';
				$concat_stop_treeid = ($db->phptype == 'mysql')
					? 'CONCAT(pt.treeid, '.$db->quote($free_childid).', SUBSTRING(ctp.stop_treeid FROM '.$db->quote(strlen($minor_tree['treeid']) + 1).'))'
					: 'pt.treeid || '.$db->quote($free_childid).' || SUBSTRING(ctp.stop_treeid FROM '.$db->quote(strlen($minor_tree['treeid']) + 1).')';

				$permission_insert = 'INSERT INTO sq_asset_permission_lookup
										(permissionid, start_treeid, stop_treeid, inc_stop) ';
				$select = 'SELECT ctp.permissionid, '.$concat_start_treeid.', '.$concat_stop_treeid.', ctp.inc_stop
							FROM sq_asset_link_tree pt INNER JOIN sq_asset_link pl ON pt.linkid = pl.linkid,
								sq_asset_permission_lookup ctp
							WHERE pl.minorid = '.$db->quote($this->id).'
							  AND ctp.start_treeid LIKE '.$db->quote($minor_tree['treeid'].'%');
				$result = db_extras_insert_select($db, $permission_insert, $select);
				if (DB::isError($result)) {
					trigger_error($result->getMessage().'<br/>'.$result->getUserInfo(), E_USER_WARNING);
					$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
					return 0;
				}
			}// end if

		}// end if significant link

		// move the sort_order for all the other assets up one
		$sql = 'UPDATE sq_asset_link
				SET sort_order = sort_order + 1
				WHERE majorid     = '.$db->quote($this->id).'
				  AND sort_order >= '.$db->quote($sort_order);

		$result = $db->query($sql);
		if (DB::isError($result)) {
			trigger_error($result->getMessage().'<br/>'.$result->getUserInfo(), E_USER_WARNING);
			$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
			return 0;
		}

		// and finally insert the link
		$sql = 'INSERT INTO sq_asset_link
				(linkid, majorid, minorid, link_type, value, sort_order, dependant, exclusive, last_updated, last_userid)
				VALUES
				('.$db->quote($linkid).', '.$db->quote($this->id).', '.$db->quote($minor->id).', '.$db->quote($link_type).', '
				.$db->quote($value).', '.$db->quote($sort_order).', '.$db->quote(($dependant) ? '1' : '0').', '.$db->quote(($exclusive) ? '1' : '0').', '.$db->quote(ts_iso8601(time())).',
				'.$db->quote($GLOBALS['SQ_SYSTEM']->currentUserId()).')';


		$result = $db->query($sql);
		if (DB::isError($result)) {
			trigger_error($result->getMessage().'<br/>'.$result->getUserInfo(), E_USER_WARNING);
			$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
			return 0;
		}

		// tell, the asset it has updated
		if (!$this->_linksUpdated()) {
			$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
			return 0;
		}

		// if this is a frontend nav link, then we need to inform the asset to update it's lookups
		if ($link_type & SQ_LINK_FRONTEND_NAV) {
			if (!$minor->updateLookups()) {
				$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
				return 0;
			}
		}

		// if this is a significant link, it's time to refresh the permissions lookup
		if ($link_type & SQ_LINK_SIGNIFICANT) {
			//// UPDATE OUR PERMISSIONS LOOKUP'S ////
			if (!$this->_refreshPermissionLookups()) {
				$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
				return 0;
			}
		}

		$GLOBALS['SQ_SYSTEM']->doTransaction('COMMIT');
		$GLOBALS['SQ_SYSTEM']->broadcastEvent('CreateLink', Array('assetid' => $this->id, 'linkid' => $linkid));

		return $linkid;

	}//end createLink()


	/**
	* Returns TRUE if we can delete the passed link, or a string with the error msg
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
	* Removes a link where this asset is the major player
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

		// if this is a significant link
		if ($link['link_type'] & SQ_LINK_SIGNIFICANT) {

			$minor = &$GLOBALS['SQ_SYSTEM']->am->getAsset($link['minorid']);
			if (is_null($minor)) {
				$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
				return false;
			}

			$num_other_links = $minor->countLinks('minor', SQ_LINK_SIGNIFICANT, '', true, $linkid);

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
				// whatever it is inheriting right now so it keeps the while in the trash
				$perms = Array(SQ_PERMISSION_ADMIN, SQ_PERMISSION_WRITE, SQ_PERMISSION_READ);
				foreach ($perms as $perm) {
					$all_permissions = $minor->getPermission($perm, null, false, true, false, true);
					foreach($all_permissions as $userid => $data) {
						if (empty($data)) continue;
						$accesses = array_unique($data);
						foreach($accesses as $access) {
							if (!$minor->setPermission($userid, $perm, $access)) {
								$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
								return false;
							}
						}
					}
				}

				if (!$trash_folder->createLink($minor, SQ_LINK_TYPE_2)) {
					$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
					return false;
				}
			}

			//// UPDATE PERMISSIONS LOOKUP TABLE ////

			// Remove all lookup references to the permissions that occured below this link
			$sql = 'DELETE FROM sq_asset_permission_lookup
					WHERE start_treeid in (~SQ0~)';
			$concat = ($db->phptype == 'mysql') ? 'CONCAT(t.treeid, '.$db->quote('%').')' : 't.treeid || '.$db->quote('%');
			$subs = Array('SELECT pl.start_treeid
							FROM sq_asset_link_tree t, sq_asset_permission_lookup pl
							WHERE t.linkid = '.$db->quote($linkid).'
							  AND pl.start_treeid LIKE '.$concat);
			$sql = db_extras_subquery($db, $sql, $subs);
			if (DB::isError($sql)) {
				$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
				trigger_error($sql->getMessage().'<br/>'.$sql->getUserInfo(), E_USER_WARNING);
				return false;
			}

			$result = $db->query($sql);
			if (DB::isError($result)) {
				$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
				trigger_error($result->getMessage().'<br/>'.$result->getUserInfo(), E_USER_WARNING);
				return false;
			}

			//// UPDATE THE TREE ////

			// Update the parents to tell them that they are going to be one kid less
			$sql = 'UPDATE sq_asset_link_tree
					   SET num_immediate_kids = num_immediate_kids - 1
					WHERE treeid IN (~SQ0~)';
			$subs = Array('SELECT SUBSTRING(t.treeid FROM 1 FOR (CHARACTER_LENGTH(t.treeid) - '.SQ_CONF_ASSET_TREE_SIZE.'))
							FROM sq_asset_link_tree t
							WHERE t.linkid = '.$db->quote($linkid));

			$sql = db_extras_subquery($db, $sql, $subs);
			if (DB::isError($sql)) {
				$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
				trigger_error($sql->getMessage().'<br/>'.$sql->getUserInfo(), E_USER_WARNING);
				return false;
			}

			$result = $db->query($sql);
			if (DB::isError($result)) {
				$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
				trigger_error($result->getMessage().'<br/>'.$result->getUserInfo(), E_USER_WARNING);
				return false;
			}

			// We can delete all the links under these nodes because it will be a clean start
			// when we insert into the gap's we create below
			$sql = 'DELETE FROM sq_asset_link_tree
					WHERE treeid in (~SQ0~)';
			$concat = ($db->phptype == 'mysql') ? 'CONCAT(pt.treeid, '.$db->quote('%').')' : 'pt.treeid || '.$db->quote('%');
			$subs = Array('SELECT ct.treeid
							FROM sq_asset_link_tree pt, sq_asset_link_tree ct
							WHERE pt.linkid = '.$db->quote($linkid).'
							  AND ct.treeid LIKE '.$concat.'
							  AND ct.treeid > pt.treeid');

			$sql = db_extras_subquery($db, $sql, $subs);
			if (DB::isError($sql)) {
				$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
				trigger_error($sql->getMessage().'<br/>'.$sql->getUserInfo(), E_USER_WARNING);
				return false;
			}

			$result = $db->query($sql);
			if (DB::isError($result)) {
				$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
				trigger_error($result->getMessage().'<br/>'.$result->getUserInfo(), E_USER_WARNING);
				return false;
			}

			// We are going to set the treeid nodes that this link is associated
			// with to zero so that we can find it as a gap when we createLink() later on
			$sql = 'UPDATE sq_asset_link_tree
					   SET linkid = 0,
					       num_immediate_kids = 0
					WHERE linkid = '.$db->quote($linkid);

			$result = $db->query($sql);
			if (DB::isError($result)) {
				$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
				trigger_error($result->getMessage().'<br/>'.$result->getUserInfo(), E_USER_WARNING);
				return false;
			}

		}// end if significant link

		// move 'em up, higher
		$sql = 'UPDATE sq_asset_link
				SET sort_order = sort_order - 1
				WHERE majorid    = '.$db->quote($this->id).'
				  AND sort_order > '.$db->quote($link['sort_order']);

		$result = $db->query($sql);
		if (DB::isError($result)) {
			$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
			trigger_error($result->getMessage().'<br/>'.$result->getUserInfo(), E_USER_WARNING);
			return false;
		}

		$sql = 'DELETE FROM sq_asset_link
				WHERE linkid  = '.$db->quote($linkid).'
				  AND majorid = '.$db->quote($this->id);

		$result = $db->query($sql);
		if (DB::isError($result)) {
			$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
			trigger_error($result->getMessage().'<br/>'.$result->getUserInfo(), E_USER_WARNING);
			return false;
		}

		// tell, the asset it has updated
		if (!$this->_linksUpdated()) {
			$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
			return false;
		}

		// if this is a frontend nav link, then we need to inform the asset to update it's lookups
		if ($link['link_type'] & SQ_LINK_FRONTEND_NAV) {
			if (!$minor->updateLookups()) {
				$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
				return 0;
			}
		}

		// if this is a significant link, it's time to refresh the permissions lookup
		if ($link['link_type'] & SQ_LINK_SIGNIFICANT) {
			//// UPDATE OUR PERMISSIONS LOOKUP'S ////
			if (!$this->_refreshPermissionLookups()) {
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

		$new_parent->acquireLock();
		$new_linkid = $new_parent->createLink($minor, $link_type, '', $to_parent_pos);
		$new_parent->releaseLock();
		if (!$new_linkid) {
			$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
			return false;
		}

		// Now let's try and delete the old link
		$this->acquireLock();
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
	* Moves a link from one place in this assets links order to another position
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
	* Called whenever any type of link is changed, allows caching of objects, links and other stuff
	* to occur but not cause integrity issues during script execution
	* returns true it it's successfull
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
	* Updates the details of an existing link
	* If any of the detail vars are NULL they aren't updated
	* NOTE: No transaction stuff is put around the SQL calls in the fn, that sould be done externally
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

		if ($link_type_changed) {

			// We need to make sure that we aren't going to have to be fucking
			// about with tree to get the update working
			// so because the tree only contains significant links if the
			// significant state has changed... barf
			$current_is_sig = (bool) ((int) $link['link_type'] & SQ_LINK_SIGNIFICANT);
			$new_is_sig     = (bool) ($link_type & SQ_LINK_SIGNIFICANT);
			if ($current_is_sig !== $new_is_sig) {
				trigger_error('Unable to update link #'.$linkid.' , you cannot change a link from being a significant link to a non-significant link or vice versa', E_USER_WARNING);
				return false;
			}

			$minor = &$GLOBALS['SQ_SYSTEM']->am->getAsset((int) $link['minorid']);
			if (($err_msg = $this->canLinkToType($minor->type(), $link_type)) !== true) {
				trigger_error('Unable to update link #'.$linkid.' ('.$err_msg.')', E_USER_WARNING);
				return false;
			}

			// check for web path conflicts with the new parents children
			// if the old link is not a frontend nav link but the new one is
			if (!($link['link_type'] & SQ_LINK_FRONTEND_NAV) && ($link_type & SQ_LINK_FRONTEND_NAV)) {
				$paths = $minor->getWebPaths();
				$bad_paths = $this->_webPathsInUse($paths);
				if (!empty($bad_paths)) {
					trigger_error('Cannot update link #'.$linkid.', path(s) "'.implode('", "', $bad_paths).'" already in use by one of the siblings of "'.$this->name.'"', U_USER_WARNING);
					return false;
				}
			}

			$set_clauses[] = 'link_type = '.$db->quote($link_type);

		}// end if link_type_changed

		if ($value_changed) {
			$set_clauses[] = 'value = '.$db->quote($value);
		}

		if ($sort_order_changed) {

			$sql = 'SELECT COUNT(*) as count, MAX(sort_order) as max
					FROM sq_asset_link
					WHERE majorid = '.$db->quote($this->id);
			$row = $db->getRow($sql);
			if (DB::isError($row)) {
				trigger_error($row->getMessage().'<br/>'.$row->getUserInfo(), E_USER_WARNING);
				return false;
			}
			$max = ($row['count'] > 0) ? (int) $row['max'] : 0;
			if ($sort_order > $max || $sort_order < 0) $sort_order = $max;

			$set_clauses[] = 'sort_order = '.$db->quote($sort_order);
		}

		// dickhead check
		if (empty($set_clauses)) return true;

		$sql = 'UPDATE sq_asset_link
				SET '.implode(', ', $set_clauses).',
				    last_userid  = '.$db->quote($GLOBALS['SQ_SYSTEM']->currentUserId()).',
				    last_updated = '.$db->quote(ts_iso8601(time())).'
				WHERE linkid    = '.$db->quote($linkid).'
				  AND majorid   = '.$db->quote($this->id);

		$result = $db->query($sql);
		if (DB::isError($result)) {
			trigger_error($result->getMessage().'<br/>'.$result->getUserInfo(), E_USER_WARNING);
			return false;
		}

		if ($link_type_changed) {
			// if this is a frontend nav link or if the old link type was,
			// then we need to inform the asset to update it's lookups
			if (($link_type & SQ_LINK_FRONTEND_NAV) || ((int) $link['link_type'] & SQ_LINK_FRONTEND_NAV)) {
				if (!$minor->updateLookups()) {
					return false;
				}
			}
		}

		if ($sort_order_changed) {
			// move 'em up, higher
			if ($link['sort_order'] > $sort_order) {
				$sql = 'UPDATE sq_asset_link
						SET sort_order = sort_order + 1
						WHERE majorid = '.$db->quote($this->id).'
						  AND linkid <> '.$db->quote($linkid).'
						  AND sort_order >= '.$db->quote($sort_order).'
						  AND sort_order <= '.$db->quote($link['sort_order']);

			} else {
				$sql = 'UPDATE sq_asset_link
						SET sort_order = sort_order - 1
						WHERE majorid = '.$db->quote($this->id).'
						  AND linkid <> '.$db->quote($linkid).'
						  AND sort_order >= '.$db->quote($link['sort_order']).'
						  AND sort_order <= '.$db->quote($sort_order);
			}

			$result = $db->query($sql);
			if (DB::isError($result)) {
				trigger_error($result->getMessage().'<br/>'.$result->getUserInfo(), E_USER_WARNING);
				return false;
			}

		}// end if sort order changed

		return true;

	}//end _updateLink()


	/**
	* Set the value for the passed variable to the passed value
	*
	* @param string		$name				the name of the attribute
	* @param string		$value				the new value of the attribute
	* @param boolean	$dont_run_updated	if true this->_updated() won't be run
	*										(THIS IS ONLY EVER CALLED FROM Asset::create())
	*
	* @return boolean
	* @access public
	*/
	function setAttrValue($name, $value, $dont_run_updated=false)
	{
		if (empty($this->vars[$name])) {
			trigger_error('Attribute "'.$name.'" not found for Asset of type "'.$this->type().'"', E_USER_WARNING);
			return false;
		}

		$attribute = &$this->getAttribute($name);
		if (!$attribute->setValue($value)) {
			trigger_error('"'.$value.'" is not a valid value for attribute "'.$name.'"', E_USER_WARNING);
			return false;
		}

		if ($this->vars[$name]['value'] == $value) return true;

		$save_value = $value;
		$old_value  = $this->vars[$name]['value'];

		// if this is a seriliaze attribute, unserilize it
		// now so that $save_value is the serialized version
		// and $value becomes the unserialize version for
		// later use by the asset
		if ($this->vars[$name]['type'] == 'serialise') {
			$value = unserialize($value);
		}

		// if we have an id then we need to update the database
		if ($this->id) {

			$db = &$GLOBALS['SQ_SYSTEM']->db;

			// open the transaction
			$GLOBALS['SQ_SYSTEM']->doTransaction('BEGIN');

			$attributeid = $this->vars[$name]['attributeid'];

			// find any previous entry
			$sql = 'SELECT COUNT(*) FROM sq_asset_attribute_value
					WHERE assetid = '.$db->quote($this->id).'
					  AND attributeid = '.$db->quote($attributeid);

			$count = $db->getOne($sql);
			if (DB::isError($count)) {
				$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
				trigger_error($count->getMessage().'<br/>'.$count->getUserInfo(), E_USER_WARNING);
				$attribute->setValue($old_value);
				return false;
			}

			if ($count) {
				// now insert new entry
				$sql = 'UPDATE sq_asset_attribute_value
						SET custom_value = '.$db->quote($save_value).'
						WHERE assetid = '.$db->quote($this->id).'
						  AND attributeid = '.$db->quote($attributeid);

			} else {
				// now insert new entry
				$sql = 'INSERT INTO sq_asset_attribute_value
						(assetid, attributeid, custom_value)
						VALUES
						('.$db->quote($this->id).', '.$db->quote($attributeid).', '.$db->quote($save_value).')';

			}// end if

			$result = $db->query($sql);
			if (DB::isError($result)) {
				$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
				$attribute->setValue($old_value);
				trigger_error($result->getMessage().'<br/>'.$result->getUserInfo(), E_USER_WARNING);
				return false;
			}

		}// end if this->id

		$this->vars[$name]['value'] = $value;

		// tell, the asset it has updated
		if (!$dont_run_updated && !$this->_updated()) {
			$this->vars[$name]['value'] = $old_value;
			$attribute->setValue($old_value);
			if ($this->id) $GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
			return false;
		}

		if (!$this->id) {
			if (!isset($this->_tmp['vars_set'])) $this->_tmp['vars_set'] = Array();
			$this->_tmp['vars_set'][] = $name;
		}

		// If we get this far, then it's all OK
		if ($this->id) $GLOBALS['SQ_SYSTEM']->doTransaction('COMMIT');

		return true;

	}//end setAttrValue()


	/**
	* Easy way to get the value of an attribute
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
	* @param boolean	$and_inherited	Whether to get inherited permissions as well
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
	function getPermission($permission, $access=null, $and_greater=true, $and_inherited=true, $expand_groups=false, $all_info=false)
	{
		$permission = (int) $permission;
		// can't be and_greater with workflow
		if ($permission == SQ_PERMISSION_WORKFLOW) $and_greater = false;

		if (($and_greater || $expand_groups) && $all_info) {
			trigger_error(__CLASS__.'::'.__FUNCTION__.'() - You cannot have either and_greater or expand_groups arguments set to true if you want to have all_info set to true', E_USER_NOTICE);
			$all_info = false;
		}

		// return cached version if we can
		$storage_name =(($and_greater) ? 'effective_' : '').'permission_'.$permission;
		if (!isset($this->_tmp['permission_cache'][$storage_name])) {

			$db = &$GLOBALS['SQ_SYSTEM']->db;

			$concat_1 = ($db->phptype == 'mysql') ? 'CONCAT(pl1.start_treeid, '.$db->quote('%').')' : 'pl1.start_treeid || '.$db->quote('%');
			$concat_2 = ($db->phptype == 'mysql') ? 'CONCAT(t2.treeid, '.$db->quote('%').')'        : 't2.treeid || '.$db->quote('%');

			// NOTE: we can't really join the userid to the asset table to get the type code
			//       because that would cause public access (ie userid 0) to not be found
			$sql = 'SELECT DISTINCT p1.assetid, p1.userid, p1.access
					FROM sq_asset_permission_lookup pl1
							INNER JOIN sq_asset_permission p1 ON pl1.permissionid = p1.permissionid,
						sq_asset_link l2
							INNER JOIN sq_asset_link_tree t2 ON l2.linkid = t2.linkid
					WHERE l2.minorid = '.$db->quote($this->id).'
					  AND t2.treeid       LIKE '.$concat_1.'
					  AND pl1.stop_treeid LIKE '.$concat_2.'
					  AND pl1.start_treeid <= t2.treeid
					  AND (t2.treeid < pl1.stop_treeid OR (pl1.stop_treeid = t2.treeid AND pl1.inc_stop = '.$db->quote('1').'))
					  AND p1.permission '.(($and_greater) ? '>= ' : '= ').$db->quote($permission);

#			pre_echo($sql);
			$result = $db->getAll($sql);
			if (DB::isError($result)) {
				trigger_error($result->getMessage().'<br/>'.$result->getUserInfo(), E_USER_WARNING);
				return Array();
			}

			// cache the result for next time
			$this->_tmp['permission_cache'][$storage_name] = $result;

		}// end if

		$ret_val = Array();
		foreach($this->_tmp['permission_cache'][$storage_name] as $data) {
			if (!$and_inherited && $data['assetid'] != $this->id) continue;

			if      ($access === false && $data['access'] != '0') continue;
			else if ($access === true  && $data['access'] != '1') continue;

			// return all info
			if ($all_info) {
				if(!isset($ret_val[$data['userid']])) $ret_val[$data['userid']] = Array();
				$ret_val[$data['userid']][$data['assetid']] = $data['access'];

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
					}// end if
				} else {
					$ret_val[] = $data['userid'];
				}// end if
			}// end if all info

		}// end foreach

		if ($all_info) {
			return $ret_val;
		} else {
			return array_unique($ret_val);
		}

	}//end getPermission()


	/**
	* Sets a permission for the passed user or user_group
	*
	* @param int		$userid		the id of the user or user_group to add the permission for
	* @param string		$permission	the permission code you are adding
	* @param string		$access		whether this permission grants access or not ('1' or '0')
	*
	* @return boolean
	* @access public
	*/
	function setPermission($userid, $permission, $access)
	{
		$userid = (int) $userid;
		$permission = (int) $permission;
		$access = ($access) ? '1' : '0';
		$current = &$this->getPermission($permission, false, false);
		if (in_array($userid, array_keys($current))) return false;

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
		}

		$db = &$GLOBALS['SQ_SYSTEM']->db;
		$GLOBALS['SQ_SYSTEM']->doTransaction('BEGIN');

		$permissionid = $db->nextId('sq_sequence_asset_permission');
		if (DB::isError($permissionid)) {
			trigger_error($permissionid->getMessage().'<br/>'.$permissionid->getUserInfo(), E_USER_WARNING);
			$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
			return false;
		}

		// add the permission into the database
		$sql = 'INSERT INTO sq_asset_permission
				(permissionid, assetid, userid, permission, access)
				VALUES
				('.$db->quote($permissionid).', '.$db->quote($this->id).', '.$db->quote($userid).', '.$db->quote($permission).', '.$db->quote($access).')';

		$result = $db->query($sql);
		if (DB::isError($result)) {
			trigger_error($result->getMessage().'<br/>'.$result->getUserInfo(), E_USER_WARNING);
			$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
			return false;
		}

		if (!$this->_addPermissionLookup($permissionid, $userid, $permission)) {
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
	* @param string	$permission		the permission code you are adding
	*
	* @return boolean
	* @access public
	*/
	function deletePermission($userid, $permission)
	{
		$userid     = (int) $userid;
		$permission = (int) $permission;

		$db = &$GLOBALS['SQ_SYSTEM']->db;

		$sql = 'SELECT permissionid
				FROM sq_asset_permission
				WHERE userid     = '.$db->quote($userid).'
				  AND permission = '.$db->quote($permission).'
				  AND assetid    = '.$db->quote($this->id);

		$permissionid = $db->getOne($sql);
		if (DB::isError($permissionid)) {
			trigger_error($permissionid->getMessage().'<br/>'.$permissionid->getUserInfo(), E_USER_WARNING);
			return false;
		}

		if (!$permissionid) {
			trigger_error('Cannot delete permission for "'.$this->name.'" (Id #'.$this->id.'), no permission found for User Id #'.$userid, E_USER_WARNING);
			return false;
		}

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

		$sql = 'DELETE FROM sq_asset_permission
				WHERE permissionid = '.$db->quote($permissionid);
		$result = $db->query($sql);
		if (DB::isError($result)) {
			trigger_error($result->getMessage().'<br/>'.$result->getUserInfo(), E_USER_WARNING);
			$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
			return false;
		}

		if (!$this->_deletePermissionLookup($permissionid, $userid, $permission)) {
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
	* Resets the permission lookup table for this asset
	* Done by removing and then adding the permissions to the lookup
	*
	* @return boolean
	* @see _addPermissionsLookup()
	* @see _deletePermissionsLookup()
	* @access protected
	*/
	function _refreshPermissionLookups()
	{
		$db = &$GLOBALS['SQ_SYSTEM']->db;
		$GLOBALS['SQ_SYSTEM']->doTransaction('BEGIN');

		$concat_1 = ($db->phptype == 'mysql') ? 'CONCAT(pl1.start_treeid, '.$db->quote('%').')' : 'pl1.start_treeid || '.$db->quote('%');
		$concat_2 = ($db->phptype == 'mysql') ? 'CONCAT(t2.treeid, '.$db->quote('%').')'        : 't2.treeid || '.$db->quote('%');
		// Find all the permissions that affect us, not just the ones that we have directly set,
		// as these will probably need updating
		$sql = 'SELECT DISTINCT a1.assetid, a1.type_code, p1.permissionid, p1.userid, p1.permission
				FROM sq_asset a1
						INNER JOIN sq_asset_link l1 ON a1.assetid = l1.minorid
						INNER JOIN sq_asset_link_tree t1 ON l1.linkid = t1.linkid
						INNER JOIN sq_asset_permission_lookup pl1 ON t1.treeid = pl1.start_treeid
						INNER JOIN sq_asset_permission p1 ON pl1.permissionid = p1.permissionid,
					sq_asset_link l2
						INNER JOIN sq_asset_link_tree t2 ON l2.linkid = t2.linkid
				WHERE l2.minorid = '.$db->quote($this->id).'
				  AND t2.treeid       LIKE '.$concat_1.'
				  AND pl1.stop_treeid LIKE '.$concat_2.'
				  AND pl1.start_treeid <= t2.treeid
				  AND (t2.treeid < pl1.stop_treeid OR (pl1.stop_treeid = t2.treeid AND pl1.inc_stop = '.$db->quote('1').'))';

		$lookups = $db->getAll($sql);
		if (DB::isError($lookups)) {
			trigger_error($lookups->getMessage().'<br/>'.$lookups->getUserInfo(), E_USER_WARNING);
			$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
			return false;
		}

		// if there are no permissions we are all done
		if (empty($lookups)) {
			$GLOBALS['SQ_SYSTEM']->doTransaction('COMMIT');
			return true;
		}

		foreach($lookups as $data) {
			$asset = &$GLOBALS['SQ_SYSTEM']->am->getAsset($data['assetid'], $data['type_code']);
			if(is_null($asset)) continue;

			if (!$asset->_deletePermissionLookup((int) $data['permissionid'], (int) $data['userid'], (int) $data['permission'], false)) {
				$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
				return false;
			}

			if (!$asset->_addPermissionLookup((int) $data['permissionid'], (int) $data['userid'], (int) $data['permission'])) {
				$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
				return false;
			}
		}// end foreach

		$GLOBALS['SQ_SYSTEM']->doTransaction('COMMIT');
		return true;

	}// end _refreshPermissionLookups()


	/**
	* Updates the permission lookup table for the by adding a permission in
	*
	* @param string	$permissionid	the id of the new permission
	* @param int	$userid			the user that has had permission set
	* @param string	$permission		the permission code you are adding
	*
	* @return boolean
	* @access protected
	*/
	function _addPermissionLookup($permissionid, $userid, $permission)
	{
		//// UPDATE PERMISSIONS LOOKUP TABLE ////
		$db = &$GLOBALS['SQ_SYSTEM']->db;
		$GLOBALS['SQ_SYSTEM']->doTransaction('BEGIN');

		// First we check to see if this permission will be inserting into the
		// middle of another currently set permission
		$concat_1 = ($db->phptype == 'mysql') ? 'CONCAT(pl.start_treeid, '.$db->quote('%').')' : 'pl.start_treeid || '.$db->quote('%');
		$concat_2 = ($db->phptype == 'mysql') ? 'CONCAT(t.treeid, '.$db->quote('%').')'        : 't.treeid || '.$db->quote('%');
		$sql = 'SELECT t.treeid as our_treeid, pl.start_treeid, pl.permissionid, pl.stop_treeid
				FROM sq_asset_permission_lookup pl INNER JOIN sq_asset_permission p ON pl.permissionid = p.permissionid,
					sq_asset_link l INNER JOIN sq_asset_link_tree t ON l.linkid = t.linkid
				WHERE l.minorid = '.$db->quote($this->id).'
				  AND t.treeid       LIKE '.$concat_1.'
				  AND pl.stop_treeid LIKE '.$concat_2.'
				  AND pl.start_treeid <= t.treeid
				  AND (t.treeid < pl.stop_treeid OR (pl.stop_treeid = t.treeid AND pl.inc_stop = '.$db->quote('1').'))
				  AND p.permission = '.$db->quote($permission).'
				  AND p.userid  = '.$db->quote($userid);

		$result = $db->query($sql);
		if (DB::isError($result)) {
			trigger_error($result->getMessage().'<br/>'.$result->getUserInfo(), E_USER_WARNING);
			$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
			return false;
		}

		$existing_perms = Array();
		while (DB_OK === $result->fetchInto($row)) {
			if (!isset($existing_perms[$row['our_treeid']])) $existing_perms[$row['our_treeid']] = Array();
			if (!isset($existing_perms[$row['our_treeid']][$row['start_treeid']])) {
				$existing_perms[$row['our_treeid']][$row['start_treeid']] = Array('permissionid' => $row['permissionid'], 'stop_treeids' => Array());
			}
			$existing_perms[$row['our_treeid']][$row['start_treeid']]['stop_treeids'][] = $db->quote($row['stop_treeid']);
		}
		$result->free();

		$in_str = '';
		foreach(array_keys($existing_perms) as $i) $in_str .= $db->quote($i).',';
		$in_str = substr($in_str, 0, -1);

		// Get the leaf nodes below us, ignoring any that we have already got from existing permissions
		$concat = ($db->phptype == 'mysql') ? 'CONCAT(pt.treeid, '.$db->quote('%').')' : 'pt.treeid || '.$db->quote('%');
		$sql = 'SELECT pt.treeid as our_treeid, ct.treeid as leaf_treeid
				FROM sq_asset_link_tree ct,
				  sq_asset_link pl INNER JOIN sq_asset_link_tree pt ON pl.linkid = pt.linkid
				WHERE pl.minorid = '.$db->quote($this->id).'
				  AND ct.treeid LIKE '.$concat.'
				  AND ct.num_immediate_kids = 0
				  AND ct.linkid != 0
				';
		if ($in_str) $sql .= '  AND pt.treeid NOT IN ('.$in_str.')';

		$leaf_treeids = $db->getAssoc($sql, false, Array(), DB_FETCHMODE_DEFAULT, true);
		if (DB::isError($leaf_treeids)) {
			trigger_error($leaf_treeids->getMessage().'<br/>'.$leaf_treeids->getUserInfo(), E_USER_WARNING);
			$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
			return false;
		}

		// Do any permissions start below us ? Ignore any that we have already got from existing permissions
		$concat = ($db->phptype == 'mysql') ? 'CONCAT(t.treeid, '.$db->quote('%').')' : 't.treeid || '.$db->quote('%');
		$sql = 'SELECT DISTINCT t.treeid as our_treeid, pl.start_treeid
				FROM sq_asset_permission_lookup pl INNER JOIN sq_asset_permission p ON pl.permissionid = p.permissionid,
					sq_asset_link l INNER JOIN sq_asset_link_tree t ON l.linkid = t.linkid
				WHERE l.minorid = '.$db->quote($this->id).'
				  AND pl.start_treeid LIKE '.$concat.'
				  AND p.permission = '.$db->quote($permission).'
				  AND p.userid  = '.$db->quote($userid);
		if ($in_str) $sql .= '  AND t.treeid NOT IN ('.$in_str.')';

		$perms_below = $db->getAssoc($sql, false, Array(), DB_FETCHMODE_DEFAULT, true);
		if (DB::isError($perms_below)) {
			trigger_error($perms_below->getMessage().'<br/>'.$perms_below->getUserInfo(), E_USER_WARNING);
			$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
			return false;
		}

		foreach($existing_perms as $our_treeid => $existing_perm) {
			foreach($existing_perm as $start_treeid => $existing_perm_data) {
				// update the existing entries to start with ourselves
				$sql = 'UPDATE sq_asset_permission_lookup
						SET start_treeid = '.$db->quote($our_treeid).',
							permissionid = '.$db->quote($permissionid).'
						WHERE start_treeid = '.$db->quote($start_treeid).'
						  AND stop_treeid IN ('.implode(',', $existing_perm_data['stop_treeids']).')
						  AND permissionid = '.$db->quote($existing_perm_data['permissionid']);

				$result = $db->query($sql);
				if (DB::isError($result)) {
					trigger_error($result->getMessage().'<br/>'.$result->getUserInfo(), E_USER_WARNING);
					$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
					return false;
				}

				// create a bridge between the old parent and ourselves
				// making sure that we exclude the stop point
				$sql = 'INSERT INTO sq_asset_permission_lookup
						(permissionid, start_treeid, stop_treeid, inc_stop)
						VALUES
						('.$db->quote($existing_perm_data['permissionid']).', '.$db->quote($start_treeid).', '.$db->quote($our_treeid).', '.$db->quote('0').')';

				$result = $db->query($sql);
				if (DB::isError($result)) {
					trigger_error($result->getMessage().'<br/>'.$result->getUserInfo(), E_USER_WARNING);
					$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
					return false;
				}

			}// end foreach
		}// end foreach

		$inserted_perm_belows = Array();
		foreach($leaf_treeids as $our_treeid => $leafs) {
			$start_points = Array();
			foreach($leafs as $leaf) {
				$stop_treeid = $leaf;
				$inc_stop    = '1';
				if (!empty($perms_below[$our_treeid])) {
					foreach($perms_below[$our_treeid] as $perm_below) {
						// if the start of this permission below is the start of this leaf treeid
						// then that means that something is set for this permission
						// so we can't make our stop_treeid the leaf_treeid, we need to make it the
						// start_treeid of the permission below us
						$found = preg_match('/^'.$perm_below.'/', $leaf);
						if ($found === false) trigger_error('Error with preg', E_USER_ERROR);
						if ($found) {
							// if we have already dealt with this permission stop point
							// continue to next leaf
							if (in_array($perm_below, $inserted_perm_belows)) {
								continue 2;
							} else {
								$inserted_perm_belows[] = $perm_below;
								$stop_treeid = $perm_below;
								$inc_stop    = '0';
								break;
							}
						}
					}// end foreach
				}// endif

				$sql = 'INSERT INTO sq_asset_permission_lookup
						(permissionid, start_treeid, stop_treeid, inc_stop)
						VALUES
						('.$db->quote($permissionid).', '.$db->quote($our_treeid).', '.$db->quote($stop_treeid).', '.$db->quote($inc_stop).')';

				$result = $db->query($sql);
				if (DB::isError($result)) {
					trigger_error($result->getMessage().'<br/>'.$result->getUserInfo(), E_USER_WARNING);
					$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
					return false;
				}

			}// end for leafs

		}// end foreach leaf treeids

		$GLOBALS['SQ_SYSTEM']->doTransaction('COMMIT');
		return true;

	}// end _addPermissionLookup()


	/**
	* Updates the permission lookup table for the by removing a permission
	*
	* @param string		$permissionid		the id of the permission to remove
	* @param int		$userid				the id of the user or user_group to add the permission for
	* @param string		$permission			the permission code you are adding
	* @param boolean	$reset_inherited	whether or not to reset the inherited permissions lookups
	*										(only false when called from _refreshPermissionsLookups())
	*
	* @return boolean
	* @access protected
	*/
	function _deletePermissionLookup($permissionid, $userid, $permission, $reset_inherited=true)
	{
		//// UPDATE PERMISSIONS LOOKUP TABLE ////
		$db = &$GLOBALS['SQ_SYSTEM']->db;
		$GLOBALS['SQ_SYSTEM']->doTransaction('BEGIN');

		$sql = 'SELECT DISTINCT t.treeid as our_treeid
				FROM sq_asset_permission_lookup p,
					sq_asset_link l INNER JOIN sq_asset_link_tree t ON l.linkid = t.linkid
				WHERE l.minorid = '.$db->quote($this->id).'
				  AND p.start_treeid = t.treeid
				  AND p.permissionid = '.$db->quote($permissionid);

		$existing_perms = $db->getCol($sql);
		if (DB::isError($existing_perms)) {
			trigger_error($existing_perms->getMessage().'<br/>'.$existing_perms->getUserInfo(), E_USER_WARNING);
			$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
			return false;
		}

		if ($reset_inherited) {
			$sql = 'SELECT t.treeid as our_treeid, pl.start_treeid, pl.permissionid
					FROM sq_asset_permission_lookup pl INNER JOIN sq_asset_permission p ON pl.permissionid = p.permissionid,
						sq_asset_link l INNER JOIN sq_asset_link_tree t ON l.linkid = t.linkid
					WHERE l.minorid = '.$db->quote($this->id).'
					  AND pl.stop_treeid = t.treeid
					  AND p.permission = '.$db->quote($permission).'
					  AND p.userid  = '.$db->quote($userid);

			$inherited_perms = $db->getAssoc($sql, false, Array(), DB_FETCHMODE_ASSOC);
			if (DB::isError($inherited_perms)) {
				trigger_error($inherited_perms->getMessage().'<br/>'.$inherited_perms->getUserInfo(), E_USER_WARNING);
				$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
				return false;
			}
		}


		if (!empty($existing_perms)) {

			foreach($existing_perms as $our_treeid) {

				if ($reset_inherited && !empty($inherited_perms[$our_treeid])) {

					// Now delete our inherited entry entries
					$sql = 'DELETE FROM sq_asset_permission_lookup
							WHERE stop_treeid  = '.$db->quote($our_treeid).'
							  AND permissionid = '.$db->quote($inherited_perms[$our_treeid]['permissionid']);

					$result = $db->query($sql);
					if (DB::isError($result)) {
						trigger_error($result->getMessage().'<br/>'.$result->getUserInfo(), E_USER_WARNING);
						$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
						return false;
					}

					// Reset our entries to start from the inherited treeid
					$sql = 'UPDATE sq_asset_permission_lookup
							SET start_treeid = '.$db->quote($inherited_perms[$our_treeid]['start_treeid']).',
								permissionid = '.$db->quote($inherited_perms[$our_treeid]['permissionid']).'
							WHERE start_treeid = '.$db->quote($our_treeid).'
							  AND permissionid = '.$db->quote($permissionid);

					$result = $db->query($sql);
					if (DB::isError($result)) {
						trigger_error($result->getMessage().'<br/>'.$result->getUserInfo(), E_USER_WARNING);
						$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
						return false;
					}

				} else {

					// Now delete our entries
					$sql = 'DELETE FROM sq_asset_permission_lookup
							WHERE start_treeid  = '.$db->quote($our_treeid).'
							  AND permissionid = '.$db->quote($permissionid);

					$result = $db->query($sql);
					if (DB::isError($result)) {
						trigger_error($result->getMessage().'<br/>'.$result->getUserInfo(), E_USER_WARNING);
						$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
						return false;
					}

				}// endif

			}// end foreach

		}// end if existing perms

		$GLOBALS['SQ_SYSTEM']->doTransaction('COMMIT');
		return true;

	}// _deletePermissionLookup()


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
	*
	* @return boolean
	* @access public
	*/
	function adminAccess($assetids=Array(), $effective=true, $editing=true)
	{
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
		if (!$this->id) return true;
		if ($this->status != SQ_STATUS_LIVE) return true;

		include_once SQ_INCLUDE_PATH.'/workflow_manager.inc';
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
			if ($this->status & SQ_STATUS_PENDING) return false;

			// if we are in a state of total approval, no editing allowed
			if ($this->status & SQ_STATUS_ALL_APPROVED) return false;

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
		if (empty($assetids) && $GLOBALS['SQ_SYSTEM']->user->id) {
			// if we are the root user, we can do anything
			if ($GLOBALS['SQ_SYSTEM']->userRoot()) return true;

			// if we are a system administrator, we can do anything
			// unless an asset overwrites this function
			if ($GLOBALS['SQ_SYSTEM']->userSystemAdmin()) return true;

			// use the current user and their groups
			$assetids = $GLOBALS['SQ_SYSTEM']->user->getParents(Array('user', 'user_group'), false);
			$assetids[] = $GLOBALS['SQ_SYSTEM']->user->id;
		}

		$perms = $this->getPermission($perm, true);
		if (in_array('0', $perms)) return true;
		$common = array_intersect($assetids, $perms);
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
		$sql = 'SELECT l.designid, a.type_code
				FROM sq_asset_lookup l, sq_asset a
				WHERE l.designid = a.assetid
				  AND l.url        = '.$db->quote($url).'
				  AND l.assetid    = '.$db->quote($this->id);
		$result = $db->getRow($sql);
		if (DB::isError($result)) {
			trigger_error($result->getMessage().'<br/>'.$result->getUserInfo(), E_USER_ERROR);
		}

		// we have found the design to use
		if ($result) {
			$design = &$GLOBALS['SQ_SYSTEM']->am->getAsset($result['designid'], $result['type_code']);
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
	* @param string	$screen	the screen that you want displayed when the asset is loaded
	*
	* @access public
	*/
	function getBackendHref($screen='')
	{
		return sq_web_path('edit').'/?SQ_BACKEND_PAGE=main&backend_section=am&am_section=edit_asset&assetid='.$this->id.'&asset_ei_screen='.$screen;

	}//end getBackendHref()


	/**
	* Returns the URL for this asset, closest to the $base_url
	*
	* @param string	$base_url	the url that we are want to match closest to
	*
	* @return string
	* @access public
	*/
	function getURL($base_url=null)
	{
		return $GLOBALS['SQ_SYSTEM']->am->getAssetURL($this->id, $base_url);

	}//end getURL()


	/**
	* Returns the href for this asset closest to the $base_url, but relative to the current url
	*
	* @param string	$base_url	the url that we are want to match closest to
	*
	* @return string
	* @access public
	*/
	function getHref($base_url=null)
	{
		return $GLOBALS['SQ_SYSTEM']->am->getAssetHref($this->id, $base_url);

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
			$sql = 'SELECT path
					FROM sq_asset_path
					WHERE assetid = '.$db->quote($this->id).'
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
		$save_paths = makeValidWebPaths($paths);

		// if there is no difference in the arrays (including in the sort order),
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
			$parents = $this->getLinks(SQ_LINK_FRONTEND_NAV, '', null, 'minor');
			for($i = 0; $i < count($parents); $i++) {
				$parent = &$GLOBALS['SQ_SYSTEM']->am->getAsset($parents[$i]['majorid'], $parents[$i]['type_code']);
				if (is_null($parent)) continue;
				$bad_paths = $parent->_webPathsInUse($inserts);
				if (!empty($bad_paths)) {
					trigger_error('Path(s) "'.implode('", "', $bad_paths).'" already in use by one of the siblings of "'.$parent->name.'"', E_USER_WARNING);
					return false;
				}
			}
		}// end if inserts

		$GLOBALS['SQ_SYSTEM']->doTransaction('BEGIN');

		// Remove all the old paths
		$sql = 'DELETE FROM sq_asset_path
				WHERE assetid = '.$db->quote($this->id);
		$result = $db->query($sql);
		if (DB::isError($result)) {
			trigger_error($result->getMessage().'<br/>'.$result->getUserInfo(), E_USER_ERROR);
			$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
			return false;
		}

		// Now insert the paths
		foreach($save_paths as $sort_order => $path) {

			$sql = 'INSERT INTO sq_asset_path
					(path, assetid, sort_order)
					VALUES
					('.$db->quote($path).', '.$db->quote($this->id).', '.$db->quote($sort_order).')';
			$result = $db->query($sql);
			if (DB::isError($result)) {
				trigger_error($result->getMessage().'<br/>'.$result->getUserInfo(), E_USER_ERROR);
				$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
				return false;
			}

		}// end foreach

		unset($this->_tmp['paths']);
		unset($this->_tmp['lookups']);
		unset($this->_tmp['url']);
		unset($this->_tmp['href']);

		if ($this->_updateLookups()) {
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
	* @param boolean	$return_alternatives	return all paths but replace the bad paths
	*											with a valid alternative
	*
	* @return array(string)
	* @access protected
	*/
	function _webPathsInUse($paths=Array(), $return_alternatives=false)
	{
		if (empty($paths)) return Array();
		$return_paths = Array();

		$db = &$GLOBALS['SQ_SYSTEM']->db;

		// get all our nav siblings
		$siblingids = Array();
		$my_kids = $this->getLinks(SQ_LINK_FRONTEND_NAV, '', null, 'major');
		for($j = 0; $j < count($my_kids); $j++) {
			if ($my_kids[$j]['minorid'] == $this->id) continue;
			$siblingids[] = $db->quote($my_kids[$j]['minorid']);
		}

		$sibling_paths = Array();

		if (count($siblingids)) {
			$sql = 'SELECT path
					FROM sq_asset_path
					WHERE assetid IN ('.implode(',', $siblingids).')';

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
					include_once SQ_FUDGE_PATH.'/general/file_system.inc';
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
	*						(url, root_urlid, designid)
	*
	* @return Array()
	* @access public
	*/
	function getLookups($field='')
	{
		if (!isset($this->_tmp['lookups'])) {
			$db = &$GLOBALS['SQ_SYSTEM']->db;
			$sql = 'SELECT url, root_urlid, designid
					FROM sq_asset_lookup
					WHERE assetid = '.$db->quote($this->id);
			$result = $db->getAll($sql);
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
	* @access public
	*/
	function updateLookups()
	{
		$GLOBALS['SQ_SYSTEM']->doTransaction('BEGIN');
		if ($this->_updateLookups()) {
			$GLOBALS['SQ_SYSTEM']->doTransaction('COMMIT');
			return true;
		} else {
			$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
			return false;
		}

	}// end updateLookups


	/**
	* Fn that does the real processing of updateLookups()
	* This is separated to allow for the DB transaction stuff
	* Will be called by parent assets recursively on their kids so not 'really' private
	* and by the saveWebPaths() so that their transaction calls can wrap around it
	*
	* @return boolean
	* @access private
	* @see Asset::updateLookups(), Asset::saveWebPaths()
	*/
	function _updateLookups()
	{
		$paths = $this->getWebPaths();
		$db = &$GLOBALS['SQ_SYSTEM']->db;

		unset($this->_tmp['lookups']);
		unset($this->_tmp['url']);
		unset($this->_tmp['href']);

		// Remove all the old URLs
		$sql = 'DELETE FROM sq_asset_lookup
				WHERE assetid = '.$db->quote($this->id);
		$result = $db->query($sql);
		if (DB::isError($result)) {
			trigger_error($result->getMessage().'<br/>'.$result->getUserInfo(), E_USER_ERROR);
			return false;
		}

		// if we have paths then do some url inserting
		if (count($paths)) {

			$design_link = $this->getLink(SQ_LINK_TYPE_3, 'design', false);
			$designid = (is_null($design_link)) ? 0 : $design_link['minorid'];

			$parents = $this->getLinks(SQ_LINK_FRONTEND_NAV, '', null, 'minor');
			$done_urls = Array();
			for($i = 0; $i < count($parents); $i++) {
				$parent = &$GLOBALS['SQ_SYSTEM']->am->getAsset($parents[$i]['majorid'], $parents[$i]['type_code']);
				if (is_null($parent)) continue;

				$parent_urls = $parent->getLookups();
				for($j = 0; $j < count($parent_urls); $j++) {
					for($k = 0; $k < count($paths); $k++) {
						$new_url = $parent_urls[$j]['url'].'/'.$paths[$k];
						if (in_array($new_url, $done_urls)) continue;
						$sql = 'INSERT INTO sq_asset_lookup
								(url, assetid, root_urlid, designid)
								VALUES
								('.$db->quote($new_url).',
								'.$db->quote($this->id).',
								'.$db->quote($parent_urls[$j]['root_urlid']).',
								'.$db->quote(($designid) ? $designid : $parent_urls[$j]['designid']).')';
						$result = $db->query($sql);
						if (DB::isError($result)) {
							trigger_error($result->getMessage().'<br/>'.$result->getUserInfo(), E_USER_WARNING);
							return false;
						}
						$done_urls[] = $new_url;
					}// end for paths
				}// end for parent_urls

			}// end for parents

		}// end if count paths

		$kids = $this->getLinks(SQ_LINK_FRONTEND_NAV, '', null, 'major');
		for($i = 0; $i < count($kids); $i++) {
			$kid = &$GLOBALS['SQ_SYSTEM']->am->getAsset($kids[$i]['minorid'], $kids[$i]['type_code']);
			if (is_null($kid)) continue;

			if (!$kid->_updateLookups()) {
				return false;
			}

		}// end for kids

		return true;

	}// end _updateLookups()


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
