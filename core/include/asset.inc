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
* $Id: asset.inc,v 1.325 2004/09/14 00:55:41 mmcintyre Exp $
* $Name: not supported by cvs2svn $
*/


/**
* Asset
*
* Purpose
*
*
* @author  Blair Robertson <blair@squiz.net>
* @version $Version$ - 1.0
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
	* @var Array(keyword => Array('name' => short description, 'default' => default replacement, 'type' => '[inline|block]'))
	*/
	var $_available_keywords = Array();

	/**
	* The weightings for this asset's components
	* @var Array($component => Array('default' => [true|false], 'weighting => $weighting))
	*/
	var $_weightings = null;

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
		assert_valid_db_result($assetid);

		$name   = ucwords(str_replace('_', ' ', $this->type())).' #'.$assetid;
		$now    = time();
		$userid = $GLOBALS['SQ_SYSTEM']->currentUserId();
		$initial_version = '0.0.0';

		$values = Array('assetid'          => $assetid,
						'version'          => $initial_version,
						'type_code'        => $this->type(),
						'name'             => $name,
						'short_name'       => $name,
						'status'           => SQ_STATUS_UNDER_CONSTRUCTION,
						'created'          => ts_iso8601($now),
						'created_userid'   => $userid,
						'updated'          => ts_iso8601($now),
						'updated_userid'   => $userid,
						'published'        => null,
						'published_userid' => null,
						);
		if (!$GLOBALS['SQ_SYSTEM']->rollbackInsert('asset', $values)) {
			$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
			return false;
		}

		$this->id               = $assetid;
		$this->version          = $initial_version;
		$this->name             = $name;
		$this->short_name       = $name;
		$this->status           = SQ_STATUS_UNDER_CONSTRUCTION;
		$this->created          = $now;
		$this->created_userid   = $userid;
		$this->updated          = $now;
		$this->updated_userid   = $userid;
		$this->published        = null;	// because we've never been published before
		$this->published_userid = null;

		// log what happened
		$ms = &$GLOBALS['SQ_SYSTEM']->getMessagingService();
		$ms->openLog();

		// do this so that all calls to getAsset() get a reference to us, and don't load a new instance from the DB
		$GLOBALS['SQ_SYSTEM']->am->rememberAsset($this);

		// OK, what we need to do here is set write access for this asset to the current user
		// so that they can actually do anything
		// NOW normally this would require admin access but this is a very special case
		$this->_tmp[__CLASS__.'_in_create_cascading'] = true;
		if (!$GLOBALS['SQ_SYSTEM']->am->acquireLock($this->id, 'permissions')) {
			$ms->abortLog();
			$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
			$GLOBALS['SQ_SYSTEM']->am->forgetAsset($this);
			$this->id = 0;
			return false;
		}

		// only set the permission if there is a userid to set it for - otherwise we are going
		// to be setting public write access
		if ($GLOBALS['SQ_SYSTEM']->currentUserId()) {
			if (!$GLOBALS['SQ_SYSTEM']->am->setPermission($this->id, $GLOBALS['SQ_SYSTEM']->currentUserId(), SQ_PERMISSION_WRITE, true)) {
				$ms->abortLog();
				$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
				$GLOBALS['SQ_SYSTEM']->am->forgetAsset($this);
				$this->id = 0;
				return false;
			}
		}

		unset($this->_tmp[__CLASS__.'_in_create_cascading']);

		$message_body = 'New '.$this->type().' "'.$this->name.'" created';
		$message = $ms->newMessage(Array(), 'Asset Created', $message_body, 'asset');
		$message->parameters['assetid'] = $this->id;
		$message->send();

		// set the attribute values that have been stored temporarily
		// Now we should probably get all the locks to this asset so that we can do what is required
		if (!$GLOBALS['SQ_SYSTEM']->am->acquireLock($this->id, 'attributes') || !$this->saveAttributes(true)) {
			$ms->abortLog();
			$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
			$GLOBALS['SQ_SYSTEM']->am->forgetAsset($this);
			$this->id = 0;
			return false;
		}
		$GLOBALS['SQ_SYSTEM']->am->releaseLock($this->id, 'attributes');

		$this->_loadVars();

		if (!$this->_updated()) {
			$ms->abortLog();
			$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
			$GLOBALS['SQ_SYSTEM']->am->forgetAsset($this);
			$this->id = 0;
			return false;
		}

		if (!empty($link)) {
			// create the initial link
			if (!isset($link['value']))      $link['value']      = '';
			if (!isset($link['sort_order'])) $link['sort_order'] = -1;
			if (!isset($link['dependant']))  $link['dependant']  = 0;
			if (!isset($link['exclusive']))  $link['exclusive']  = 0;

			$lock_info = @$GLOBALS['SQ_SYSTEM']->am->getLockInfo($link['asset']->id, 'links');
			$parent_was_locked = !empty($lock_info);
			if (!$GLOBALS['SQ_SYSTEM']->am->acquireLock($link['asset']->id, 'links')) {
				$ms->abortLog();
				$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
				$GLOBALS['SQ_SYSTEM']->am->forgetAsset($this);
				$this->id = 0;
				return false;
			}

			// if we are creating a dependant link, we need to make sure this new
			// asset is locked in the same chain as the parent (ie, has the same source_assetid)
			$source_assetid = 0;
			if ($link['dependant']) {
				$lock = $GLOBALS['SQ_SYSTEM']->am->getLockInfo($link['asset']->id, 'links');
				$source_assetid = $lock['source_assetid'];
			}
			if (!$GLOBALS['SQ_SYSTEM']->am->acquireLock($this->id, 'links', $source_assetid)) {
				$ms->abortLog();
				$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
				$GLOBALS['SQ_SYSTEM']->am->forgetAsset($this);
				$this->id = 0;
				return false;
			}

			$linkid = $link['asset']->createLink($this, $link['link_type'], $link['value'], $link['sort_order'], $link['dependant'], $link['exclusive']);

			// get the link so we can find out if it was dependant (cant rely on the info passed in
			// because other assets may have changed what happens in createLink)
			$new_link = $GLOBALS['SQ_SYSTEM']->am->getLinkById($linkid, $this->id, 'minor');

			// cascade permissions, and schemas on this asset that the parent has if the parent is not the root folder
			$this->_tmp[__CLASS__.'_in_create_cascading'] = true;

			if (!$link['asset']->cloneComponents($this, Array('permissions', 'metadata_schemas', 'workflow'))) {
				$ms->abortLog();
				$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
				$GLOBALS['SQ_SYSTEM']->am->forgetAsset($this);
				$this->id = 0;
				return false;
			}
			unset($this->_tmp[__CLASS__.'_in_create_cascading']);

			if (!$GLOBALS['SQ_SYSTEM']->am->releaseLock($this->id, 'permissions')) {
				$ms->abortLog();
				$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
				$GLOBALS['SQ_SYSTEM']->am->forgetAsset($this);
				$this->id = 0;
				return false;
			}


			// release locks depending on if the parent was locked before we were created
			if (!$parent_was_locked) {
				// parent was not locked, so unlock both the parent and us
				if (!$GLOBALS['SQ_SYSTEM']->am->releaseLock($link['asset']->id, 'links')) {
					$ms->abortLog();
					$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
					$GLOBALS['SQ_SYSTEM']->am->forgetAsset($this);
					$this->id = 0;
					return false;
				}
			} else if ($new_link['dependant']) {
				// the parent was locked before we created this asset and the new asset
				// is dependantly linked, so make sure we have all the locks of our parent
				$parent_lock_info = $GLOBALS['SQ_SYSTEM']->am->getLockInfo($link['asset']->id, 'all');
				foreach (array_values($parent_lock_info) as $lock_details) {
					if (empty($lock_details)) continue;
					if (!$GLOBALS['SQ_SYSTEM']->am->acquireLock($this->id, $lock_details['lock_type'], $lock_details['source_assetid'])) {
							$ms->abortLog();
							$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
							$GLOBALS['SQ_SYSTEM']->am->forgetAsset($this);
							$this->id = 0;
							return false;
					}
				}
			}

			if (!$new_link['dependant']) {
				if (!$GLOBALS['SQ_SYSTEM']->am->releaseLock($this->id, 'links')) {
					$ms->abortLog();
					$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
					$GLOBALS['SQ_SYSTEM']->am->forgetAsset($this);
					$this->id = 0;
					return false;
				}
			}

			if (!$linkid) {
				$ms->abortLog();
				$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
				$GLOBALS['SQ_SYSTEM']->am->forgetAsset($this);
				$this->id = 0;
				return false;
			}
			$GLOBALS['SQ_SYSTEM']->doTransaction('COMMIT');
			$ms->closeLog();
			return (int) $linkid;
		} else {
			$GLOBALS['SQ_SYSTEM']->doTransaction('COMMIT');
			$ms->closeLog();

			// index the asset with what we have so far
			$this->informChanges();

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
		$sql = 'SELECT assetid, type_code, version, name, short_name, status, languages, charset, force_secure,
					created, created_userid, updated, updated_userid, published, published_userid
				FROM '.SQ_TABLE_RUNNING_PREFIX.'asset ';
		$where = 'assetid = '.$db->quote($assetid);
		$where = $GLOBALS['SQ_SYSTEM']->constructRollbackWhereClause($where);
		
		$result = $db->getRow($sql.$where, null, DB_FETCHMODE_ORDERED);
		assert_valid_db_result($result);
		
		assert_not_null($result);

		list($this->id,
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
			 $this->published_userid) = $result;
		$this->created   = iso8601_ts($this->created);
		$this->updated   = iso8601_ts($this->updated);
		if (!is_null($this->published)) {
		  $this->published = iso8601_ts($this->published);
		}
		unset($result);

		// make sure the asset we are loading is of the same type as our class
		if ($type_code != $this->type()) {
			trigger_error('Asset #'.$assetid.' is not a '.$this->type(), E_USER_WARNING);
			$this->id               = 0;
			$this->created          = null;
			$this->created_userid   = null;
			$this->updated          = null;
			$this->updated_userid   = null;
			$this->published        = null;
			$this->published_userid = null;

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
		
		$this->vars = $GLOBALS['SQ_SYSTEM']->db->getAssoc($sql, false, Array(), DB_FETCHMODE_ASSOC);
		assert_valid_db_result($this->vars);

		if ($this->_ser_attrs && $this->vars) {
			for(reset($this->vars); NULL !== ($name = key($this->vars)); next($this->vars)) {
				if ($this->vars[$name]['type'] != 'serialise') continue;
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
			trigger_error('Cannot delete asset, asset "'.$this->name.'" [#'.$this->id.'] is not in the trash', E_USER_WARNING);
			return false;
		}

		$db = &$GLOBALS['SQ_SYSTEM']->db;
		$GLOBALS['SQ_SYSTEM']->doTransaction('BEGIN');

		// say to other assets: 'Delete me bitch cause I'm outa here'
		$links = $GLOBALS['SQ_SYSTEM']->am->getLinks($this->id, SQ_SC_LINK_ALL, '', true, 'minor');
		foreach ($links as $link) {
			$major = &$GLOBALS['SQ_SYSTEM']->am->getAsset($link['majorid'], $link['major_type_code']);
			if (!is_null($major)) {
				$major->deleteLink($link['linkid']);
			} else {
				trigger_error('Could not load asset with id '.$link['majorid'], E_USER_WARNING);
				$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
				return false;
			}
		}

		if ($release_lock) {
			// we are about to go to the big asset manager in the sky,
			// so we wont be needing worldly things such as locks
			if (!$GLOBALS['SQ_SYSTEM']->am->releaseLock($this->id, 'all')) {
				trigger_error('Delete failed, the lock on "'.$this->name.'" could not be released', E_USER_WARNING);
				$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
				return false;
			}
		}

		// remove web paths
		if (!$this->saveWebPaths(Array())) {
			trigger_error('Delete failed, unable to clear the web paths of "'.$this->name.'"', E_USER_WARNING);
			$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
			return false;
		}

		// update the lookups to clear them; figures that if there are no web paths there cannot
		// be any lookups
		if (!$this->updateLookups()) {
			trigger_error('Delete failed, unable to clear the lookups of "'.$this->name.'"', E_USER_WARNING);
			$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
			return false;
		}

		// delete from the asset table
		$where_cond = 'assetid = '.$db->quote($this->id);
		if (!$GLOBALS['SQ_SYSTEM']->rollbackDelete('asset', $where_cond)) {
			$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
			return false;
		}

		// delete the attributes
		if (!$GLOBALS['SQ_SYSTEM']->rollbackDelete('asset_attribute_value', $where_cond)) {
			$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
			return false;
		}
		if (!$GLOBALS['SQ_SYSTEM']->rollbackDelete('asset_attribute_unique_value', $where_cond)) {
			$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
			return false;
		}

		// delete any permissions we have assigned to us or are assigned to
		$where_cond = 'assetid = '.$db->quote($this->id). 'OR userid = '.$db->quote($this->id);
		if (!$GLOBALS['SQ_SYSTEM']->rollbackDelete('asset_permission', $where_cond)) {
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
		
		$em = &$GLOBALS['SQ_SYSTEM']->getEventManager();
		$em->broadcastEvent($this, 'assetDeleted', Array());
		$GLOBALS['SQ_SYSTEM']->am->forgetAsset($this, true);

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
		if (!isset($this->_tmp['use_system_version'])) {
			$usv = (($this->status & SQ_SC_STATUS_SAFE_EDITING) && !$this->writeAccess(''));
			if ($usv && $GLOBALS['SQ_SYSTEM']->currentUserId()) {
				$wfm = &$GLOBALS['SQ_SYSTEM']->getWorkflowManager();
				$publishers = $wfm->whoCanPublish($this->id);
				if (!empty($publishers) && in_array($GLOBALS['SQ_SYSTEM']->currentUserId(), $publishers)) $usv = false;
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
		$GLOBALS['SQ_REVERT_TO_SYSTEM_VERSION'] = true;
		require_once SQ_FUDGE_PATH.'/general/file_system.inc';

		// load our object data in as it was before
		if (is_file($this->data_path.'/.sq_system/.object_data')) {
			$old_version = unserialize(file_to_string($this->data_path.'/.sq_system/.object_data'));
		} else {
			trigger_error('Failed reverting asset "'.$this->name.'" to system version, could not locate object data file', E_USER_WARNING);
			$GLOBALS['SQ_REVERT_TO_SYSTEM_VERSION'] = false;
			return false;
		}

		// copy over the old files we stored in the system directory
		$files_to_copy = list_files($this->data_path.'/.sq_system');
		foreach ($files_to_copy as $filename) {
			// skip hidden files
			if (strpos($filename, '.') === 0) continue;

			if (!copy_file($this->data_path.'/.sq_system/'.$filename, $this->data_path.'/'.$filename)) {
				trigger_error('Failed reverting asset "'.$this->name.'" to system version, could not copy file "'.$filename.'" to restricted data directory', E_USER_WARNING);
				$GLOBALS['SQ_REVERT_TO_SYSTEM_VERSION'] = false;
				return false;
			}
		}

		// update all the vars
		foreach ($old_version->vars as $var_name => $var_data) {
			if ($var_data['type'] == 'serialise') $var_data['value'] = serialize($var_data['value']);
			if (!$this->setAttrValue($var_name, $var_data['value'], true)) {
				trigger_error('Failed reverting asset "'.$this->name.'" to system version, could not save value of attribute "'.$var_name.'"', E_USER_WARNING);
				$GLOBALS['SQ_REVERT_TO_SYSTEM_VERSION'] = false;
				return false;
			}
		}
		if (!$this->saveAttributes()) {
			trigger_error('Failed reverting asset "'.$this->name.'" to system version, could not save previous version of attributes', E_USER_WARNING);
			$GLOBALS['SQ_REVERT_TO_SYSTEM_VERSION'] = false;
			return false;
		}

		if (!$this->clearSystemVersion()) {
			trigger_error('Failed reverting asset "'.$this->name.'" to system version, could not clear system version', E_USER_WARNING);
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
	* Call this function when the asset has been changed in some way so we can indicated in the
	* asset table when the asset was last changed and who by. This function will also increment the micro
	* version number for the asset.
	*
	* @param boolean	$update_parents	should we go through an update all our dependant parents as well ?
	*									NOTE: this is passed to incrementVersion to let it know that the parents
	*									should have their versions updated as well.
	*
	* @return boolean
	* @access private
	*/
	function _updated($update_parents=true)
	{
		$updated = time();
		$updated_userid  = $GLOBALS['SQ_SYSTEM']->currentUserId();
		$name         = $this->_getName();
		$short_name   = $this->_getName(true);

		// if we have an ID hit the DB
		if ($this->id) {

			$db = &$GLOBALS['SQ_SYSTEM']->db;

			$GLOBALS['SQ_SYSTEM']->doTransaction('BEGIN');

			$where = 'assetid = '.$db->quote($this->id);
			$values = Array('name'           => $db->quote($name),
							'short_name'     => $db->quote($short_name),
							'languages'      => $db->quote($this->languages),
							'charset'        => $db->quote($this->charset),
							'force_secure'   => $db->quote($this->force_secure),
							'updated'        => $db->quote(ts_iso8601($updated)),
							'updated_userid' => $db->quote($updated_userid)
							);
			if (!$GLOBALS['SQ_SYSTEM']->rollbackUpdate('asset', $values, $where)) {
				$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
				return false;
			}

			$em = &$GLOBALS['SQ_SYSTEM']->getEventManager();
			$em->broadcastEvent($this, 'AssetUpdate',
												  Array( 'old_name'           => $this->name,
														 'new_name'           => $name,
														 'old_short_name'     => $this->short_name,
														 'new_short_name'     => $short_name,
														 'old_updated'        => $this->updated,
														 'new_updated'        => $updated,
														 'old_updated_userid' => $this->updated_userid,
														 'new_updated_userid' => $updated_userid
														));

			// increment the micro version number
			if (!$this->incrementVersion('micro', $update_parents)) {
				trigger_error('Failed updating "'.$this->name.'", version not incremented', E_USER_WARNING);
				$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
				return false;
			}

			$GLOBALS['SQ_SYSTEM']->doTransaction('COMMIT');

		}// end if $this->id

		$this->name				= $name;
		$this->short_name		= $short_name;
		$this->updated			= $updated;
		$this->updated_userid	= $updated_userid;

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
	* @return array()
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
	*
	* @return boolean
	* @access public
	*/
	function incrementVersion($number='micro', $update_parents=true)
	{
		if (!$this->id) return false;

		switch($number) {
			case 'major' : $part_num = 0; break;
			case 'minor' : $part_num = 1; break;
			case 'micro' : $part_num = 2; break;
			default :
				trigger_error('Failed incrementing version of "'.$this->name.'" [#'.$this->id.'], number part "'.$number.'" not known.', E_USER_WARNING);
				return false;
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

		$GLOBALS['SQ_SYSTEM']->doTransaction('BEGIN');
		$db = &$GLOBALS['SQ_SYSTEM']->db;

		$where = 'assetid = '.$db->quote($this->id);
		$values = Array('version' => $db->quote($new_version));
		if (!$GLOBALS['SQ_SYSTEM']->rollbackUpdate('asset', $values, $where)) {
			$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
			return false;
		}

		if ($update_parents) {
			// increment the micro version of all dependant parents because a 'part' of them
			// has been updated - even though they havnt been updated directly
			$dependant_parents = $GLOBALS['SQ_SYSTEM']->am->getDependantParents($this->id);
			foreach ($dependant_parents as $parentid) {
				$dep_parent = &$GLOBALS['SQ_SYSTEM']->am->getAsset($parentid);
				if (!is_null($dep_parent)) {
					if (!$dep_parent->_updated(false)) {
						$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
						$GLOBALS['SQ_SYSTEM']->am->forgetAsset($dep_parent);
						trigger_error('Failed incrementing version of "'.$this->name.'" [#'.$this->id.'], version updated of dependant parent "'.$dep_parent->name.'" [#'.$dep_parent->id.'] failed.', E_USER_WARNING);
						return false;
					}
				}
				$GLOBALS['SQ_SYSTEM']->am->forgetAsset($dep_parent);
			}
		}

		$em = &$GLOBALS['SQ_SYSTEM']->getEventManager();
		$em->broadcastEvent($this, 'AssetVersionUpdate',
											  Array( 'old_version'      => $this->version,
													 'new_version'      => $new_version
													));

		$GLOBALS['SQ_SYSTEM']->doTransaction('COMMIT');

		$this->version = $new_version;
		return true;

	}//end incrementVersion()


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

		$em = &$GLOBALS['SQ_SYSTEM']->getEventManager();
		$em->broadcastEvent($this, 'AssetLanguageUpdate',
											   Array( 'old_languages' => $old_languages,
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

		$em = &$GLOBALS['SQ_SYSTEM']->getEventManager();
		$em->broadcastEvent($this, 'AssetCharsetUpdate',
											   Array( 'old_charset' => $old_charset,
													  'new_charset' => $this->charset,
													));
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
			trigger_error('Unable to set the Force Secure flag, Permission Denied', E_USER_WARNING);
			return false;
		}

		$force_secure = ($force_secure) ? '1' : '0';
		if($force_secure == $this->force_secure) return false;
		$old_force_secure = $this->force_secure;
		$this->force_secure = $force_secure;
		if (!$this->_updated()) {
			trigger_error('Failed setting Force Secure flag for "'.$this->name.'" (#'.$this->id.')', E_USER_WARNING);
			$this->force_secure = $old_force_secure;
			return false;
		}

		$em = &$GLOBALS['SQ_SYSTEM']->getEventManager();
		$em->broadcastEvent($this, 'AssetForceSecureUpdate',
											   Array( 'old_force_secure' => $old_force_secure,
													  'old_force_secure' => $this->force_secure,
													)
											);
		return true;

	}//end setForceSecure()


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
		$dependant_links = $GLOBALS['SQ_SYSTEM']->am->getLinks($this->id, SQ_SC_LINK_SIGNIFICANT, '', true, 'major', null, 1);
		if (!empty($dependant_links)) {
			$am = &$GLOBALS['SQ_SYSTEM']->am;
			foreach ($dependant_links as $link) {
				$asset = &$am->getAsset($link['minorid'], $link['minor_type_code']);
				if (!$asset->remapAssetids($map)) {
					$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
					return false;
				}
			}
		}

		$GLOBALS['SQ_SYSTEM']->doTransaction('COMMIT');
		return true;

	}//end remapAssetids()


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
			trigger_error('Unable to cast "'.$this->name.'" to "'.$new_type_code.'", "'.$new_type_code.'" is not in the "'.$this->type().'" asset type hierarchy.', E_USER_WARNING);
			$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
			return false;
		}

		$am->includeAsset($new_type_code);
		$tmp = new $new_type_code();

		//// FIRST we need to check that all our links are allowed for the new type ////

		// if there any children links
		if ($num_links = $GLOBALS['SQ_SYSTEM']->am->countLinks($this->id, 'major')) {

			$links = $GLOBALS['SQ_SYSTEM']->am->getLinks($this->id, SQ_SC_LINK_ALL);
			foreach($links as $link) {
				if ($GLOBALS['SQ_SYSTEM']->am->canLinkToType($tmp, $link['minor_type_code'], $link['link_type'], 0, $link['exclusive']) !== true) {
					require_once SQ_INCLUDE_PATH.'/general_occasional.inc';
					trigger_error('Unable to cast "'.$this->name.'" to "'.$new_type_code.'",<br>"'.$link['minor_type_code'].'" assets are not allowed to be "'.link_type_name($link['link_type']).'" linked to "'.$new_type_code.'" assets.', E_USER_WARNING);
					$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
					return false;
				}
			}// end foreach

		}// end if child links

		// if there any parent links
		$num_links = $GLOBALS['SQ_SYSTEM']->am->countLinks($this->id, 'minor');
		if ($num_links) {
			
			$links = $GLOBALS['SQ_SYSTEM']->am->getLinks($this->id, SQ_SC_LINK_ALL, '', true, 'minor');
			foreach($links as $link) {
				$parent = &$am->getAsset($link['majorid'], $link['major_type_code']);
				if (is_null($parent)) continue;
				if (($err_msg = $GLOBALS['SQ_SYSTEM']->am->canLinkToType($parent, $new_type_code, $link['link_type'], $link['linkid'], $link['exclusive'])) !== true) {
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


			$em = &$GLOBALS['SQ_SYSTEM']->getEventManager();
			$em->broadcastEvent($this, 'AssetTypeUpdate',
												  Array( 'old_type' => $old_type_code,
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
	* Add valid keywords for this asset to an array of keywords when asked
	*
	* @param &object Asset	$asset	the asset that triggered the event
	* @param Array			$vars	the vars that get submitted by the broadcaster
	*								we add keywords to the $vars['keywords'] array
	*
	* @return boolean
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
	* @param &object Asset	$clone		the clone shell
	* @param array			$components	the wanted components to clone eg.
	*                       Array(
	*                           'attributes',
	*                           'metadata_shemas',
	*                           'metadata',
	*                           'workflow',
	*                           'permissions',
	*                           'data'
	*                       );
	*                       or alternately
	*                       Array('all');
	*
	* @return boolean
	* @access public
	*/
	function cloneComponents(&$clone, $components)
	{
		if (!is_array($components) || empty($components)) return false;
		if (is_null($clone)) return false;

		$GLOBALS['SQ_SYSTEM']->doTransaction('BEGIN');


		  ///////////////////
		 /// PERMISSIONS ///
		///////////////////
		// we need to clone permissions first (if we are cloning permissions) so that the user has
		// access to save attributes, schemas etc later on

		if (in_array('permissions', $components) || in_array('all', $components)) {
			$old_sq_install = (isset($GLOBALS['SQ_INSTALL'])) ? $GLOBALS['SQ_INSTALL'] : null;
			$GLOBALS['SQ_INSTALL'] = true;
			$this->_tmp[__CLASS__.'_in_create_cascading'] = true;
			foreach (Array(SQ_PERMISSION_READ, SQ_PERMISSION_WRITE, SQ_PERMISSION_ADMIN) as $perm) {
				$set_perms = $GLOBALS['SQ_SYSTEM']->am->getPermission($this->id, $perm, null, false, false, true);
				foreach ($set_perms as $userid => $access) {
					if (!$GLOBALS['SQ_SYSTEM']->am->setPermission($clone->id, $userid, $perm, $access)) {
						$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
						unset($this->_tmp[__CLASS__.'_in_create_cascading']);
						unset($GLOBALS['SQ_INSTALL']);
						return false;
					}
				}
			}//end foreach
			unset($this->_tmp[__CLASS__.'_in_create_cascading']);
			if (is_null($old_sq_install)) unset($GLOBALS['SQ_INSTALL']);
			else $GLOBALS['SQ_INSTALL'] = $old_sq_install;
		}//end if permissions


		  //////////////////
		 /// ATTRIBUTES ///
		//////////////////

		if (in_array('attributes', $components) || in_array('all', $components)) {
			// Now we set all the attributes
			foreach ($this->vars as $name => $data) {
				if (!$clone->setAttrValue($name, $data['value'])) {
					$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
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
			// apply the schemas and metadata from the cloner to the clonee
			$schemas = $mm->getSchemas($this->id);
			foreach ($schemas as $schemaid => $access) {
				if (!$mm->setSchema($clone->id, $schemaid, $access)) {
					$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
					return false;
				}
				if (in_array('metadata', $components) || in_array('all', $components)) {
					$metadata = $mm->getSchemaMetadata($this->id, $schemaid);
					if (!empty($metadata)) {
						if (!$mm->setSchemaMetadata($clone->id, $schemaid, $metadata)) {
							$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
							return false;
						}
					}
				}
			}//end foreach
		}//end if metadata


		  ////////////////
		 /// WORKFLOW ///
		////////////////

		if (in_array('workflow', $components) || in_array('all', $components)) {
			$wfm = &$GLOBALS['SQ_SYSTEM']->getWorkflowManager();
			$schemas = $wfm->getSchemas($this->id);
			foreach ($schemas as $schemaid => $access) {
				if (!$wfm->setSchema($clone->id, $schemaid, $access)) {
					$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
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
					return false;
				}
			}
		}//end if data


		$GLOBALS['SQ_SYSTEM']->doTransaction('COMMIT');
		return true;

	}//end cloneComponents()


//--        STATUS        --//


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

		$status = &$this->getStatus();
		if (!$status->processStatusChange($new_status)) return false;

		$GLOBALS['SQ_SYSTEM']->doTransaction('BEGIN');

		if ($new_status != $this->status) {

			$db = &$GLOBALS['SQ_SYSTEM']->db;

			$where = 'assetid = '.$db->quote($this->id);
			$values = Array('status' => $db->quote($new_status));
			if (!$GLOBALS['SQ_SYSTEM']->rollbackUpdate('asset', $values, $where)) {
				$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
				return false;
			}

			$em = &$GLOBALS['SQ_SYSTEM']->getEventManager();
			
			$em->broadcastEvent($this, 'AssetStatusUpdate',
												  Array('old_status' => $this->status,
														'new_status' => $new_status,
														));

			require_once SQ_INCLUDE_PATH.'/general_occasional.inc';
			$ms = &$GLOBALS['SQ_SYSTEM']->getMessagingService();
			$message_body = 'The status of "'.$this->name.'" was successfully changed from "'.get_status_description($this->status).'" to "'.get_status_description($new_status).'"';
			$message = $ms->newMessage(Array(), 'Asset Status Changed', $message_body, 'asset.status');
			$message->parameters['assetid'] = $this->id;
			$message->send();

			$this->status = $new_status;
			unset($this->_tmp['status_object']);
			$this->_updated();

			// if we have just made this asset live, we update its minor version
			if ($new_status == SQ_STATUS_LIVE) {
				if (!$this->incrementVersion('minor')) {
					trigger_error('processStatusChange failed for asset "'.$this->name.'", version not updated', E_USER_WARNING);
					$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
					return false;
				}
			}
		}

		$GLOBALS['SQ_SYSTEM']->doTransaction('COMMIT');

		return true;

	}//end processStatusChange()


	/**
	* Modifies the specified date to the specified timestamp
	*
	* The user field for the specified date is set to the userid specified. If
	* none is specified, then the current user is used
	*
	* @param int		$time		the time to set the date to. If NULL then use current time
	* @param int		$userid 	the userid that modified the specified date
	* @param string 	$date_type	the date to update, which must be one of 'published' or 'created'
	*
	* @access public
	* @return boolean indicating success
	*/
	function setDate($time=null, $userid=null, $date_type)
	{
		if (!in_array($date_type, Array('published' ,'created'))) {
			trigger_error('Only the published and created date can be modified', E_USER_WARNING);
			return false;
		}
		
		$db = &$GLOBALS['SQ_SYSTEM']->db;

		if (is_null($userid)) $userid = $GLOBALS['SQ_SYSTEM']->currentUserId();
		if (is_null($time)) {
			if ($date_type == 'created') {
				$time = time();
			} else {
				$userid = null;
			}
		}

		$time_iso = ts_iso8601($time);

		// begin a transaction
		$GLOBALS['SQ_SYSTEM']->doTransaction('BEGIN');

		$where = 'assetid = '.$db->quote($this->id);
		
		$values = Array(
			$date_type           => $db->quote($time_iso),
			$date_type.'_userid' => $db->quote($userid)
		);
	
		if (!$GLOBALS['SQ_SYSTEM']->rollbackUpdate('asset', $values, $where)) {
			$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
			return false;
		}

		$GLOBALS['SQ_SYSTEM']->doTransaction('COMMIT');

		eval('$this->'.$date_type.' = $time;');
		eval('$this->'.$date_type.'_userid = $userid;');

		return true;

	}//end setDate()


//--        LOCKING        --//


	/**
	* Returns a list of lock types available for this asset type
	*
	* @return Array()
	* @access public
	*/
	function lockTypes()
	{
		$lock_types =  Array(
					'settings'		=>  1,
					'attributes'	=>  2,
					'permissions'	=>  4,
					'links'			=>  8,
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
	* @param string		$lock_type		any valid lock type for this asset's type,
	*									plus 'all' which will attempt to acquire all locks that this asset has
	* @return boolean
	* @access public
	*/
	function canForceablyAcquireLock($lock_type)
	{
		$current_locks = $GLOBALS['SQ_SYSTEM']->am->getLockInfo($this->id, $lock_type, true, false);
		// lock type not known ? bugger off
		if (empty($current_locks)) return false;

		foreach($current_locks as $lock) {
			if (empty($lock)) continue;
			$user = null;
			if ($lock['userid']) {
				$user = &$GLOBALS['SQ_SYSTEM']->am->getAsset($lock['userid']);
			}
			// lets work out if the current user has a high
			// enough level of access to forceably acquire the lock
			if (!is_null($user)) {
				// locked by root, no-one can force acquire...
				if ($GLOBALS['SQ_SYSTEM']->userRoot($user)) return false;

				// locked by a sysadmin ? only root can force acquire...
				if ($GLOBALS['SQ_SYSTEM']->userSystemAdmin($user) && !$GLOBALS['SQ_SYSTEM']->userRoot()) return false;
			}

			// locked by someone else ? need to be a system admin to acquire this lock
			if (!$GLOBALS['SQ_SYSTEM']->userRoot() && !$GLOBALS['SQ_SYSTEM']->userSystemAdmin()) return false;

		}//end foreach
		return true;

	}//end canForceablyAcquireLock()


	/**
	* Returns any locks that are needed for editing, based on a set of custom keywords
	*
	* @param array $keywords the keywords to govern what locks are required
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
	* @return Array()
	* @access private
	*/
	function _getAllowedLinks()
	{
		return Array(SQ_LINK_TYPE_1 => Array(),
		   	     SQ_LINK_TYPE_2 => Array(),
			     SQ_LINK_TYPE_3 => Array('design' => Array('card' => 'M', 'exclusive' => false)),
			     SQ_LINK_NOTICE => Array('thumbnail' => Array('card' => 1, 'exclusive' => false))
			);

	}//end _getAllowedLinks()


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
	* @see Asset_Manager::createLink()
	*/
	function createLink(&$minor, $link_type, $value='', $sort_order=null, $dependant='0', $exclusive='0')
	{
		if (!$this->id) return 0;
		return $GLOBALS['SQ_SYSTEM']->am->createAssetLink($this, $minor, $link_type, $value, $sort_order, $dependant, $exclusive);

	}//end createLink()


	/**
	* Prepares for linking by checking that the link values are valid for this link
	*
	* This function will be called if this asset is the major or minor party in the link, so the side_of_link flag
	* should be checked to work out what side of the link this asset is on. The return value should indicate
	* if any of the link fields were changed.
	*
	* @param object Asset	$asset			the major or minor asset that we are linking to
	* @param string			$side_of_link	the side of the link we are on (major or minor)
	* @param string			$link_type		the type of link we are creating
	* @param string			$value			the value that is to be associated with the link
	* @param string			$sort_order		the position in the links list that this link should take,
	* @param string			$dependant		'0' / '1' on whether the this asset is dependant on the
	*										asset that will be linked by the new link
	* @param string			$exclusive		'0' / '1' on whether the this asset is linked exclusivly
	*										to the asset that will be linked by the new link
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
	* @param object Asset	$minor		the minor asset that we are linking to
	* @param string			$link_type	the type of link this is
	* @param int			$exclusive	the exclusive status of the link (gets passed to canLinkToType)
	*
	* @return mixed TRUE or error message string
	* @access public
	*/
	function canCreateLink(&$minor, $link_type, $exclusive)
	{
		if (!$this->id) return 'Asset Not Created';
		return $GLOBALS['SQ_SYSTEM']->am->canCreateLink($this, $minor, $link_type, $exclusive);

	}//end canCreateLink()


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
	* Determine if we can delete the passed link
	*
	* @param int	$linkid	the link id of the link to remove
	*
	* @return mixed TRUE or error msg string
	* @access public
	*/
	function canDeleteLink($linkid)
	{
		if (($err_msg = $this->isDeletableLink($linkid)) !== true) return $err_msg;
		if (!$this->id) return 'Asset Not Created';
		return $GLOBALS['SQ_SYSTEM']->am->canDeleteLink($this, $linkid);

	}//end canDeleteLink()


	/**
	* Remove a link where this asset is the major player
	*
	* @param int	$linkid	the link id of the link to remove
	*
	* @return boolean
	* @access public
	* @see Asset_Manager::deleteLink()
	*/
	function deleteLink($linkid)
	{
		$link = $GLOBALS['SQ_SYSTEM']->am->getLinkById($linkid, $this->id);
		if (empty($link)) {
			trigger_error('Unable cannot delete link, Link #'.$linkid.' does not exist', E_USER_NOTICE);
			return false;
		} else {
			return $GLOBALS['SQ_SYSTEM']->am->deleteAssetLink($linkid);
		}

	}//end deleteLink()


	/**
	* Allow any required processesing to occur when any link is updated for the asset
	*
	* Called whenever any type of link is changed. Allows caching of objects, links and other stuff
	* to occur but not cause integrity issues during script execution.
	*
	* ONLY TO BE CALLED FROM THE ASSET MANAGER
	*
	* @return boolean
	* @access protected
	*/
	function linksUpdated()
	{
		return $this->_updated();

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
			trigger_error('You do not have permission to save the attributes of "'.$this->name.'" (#'.$this->id.')', E_USER_WARNING);
			return false;
		}

		$save_vars = Array();
		$attr_ids  = Array();

		$db = &$GLOBALS['SQ_SYSTEM']->db;

		// open a queue for all messages we are going to be logging
		$ms = &$GLOBALS['SQ_SYSTEM']->getMessagingService();
		$ms->openLog();

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
				assert_valid_db_result($owning_attributeid);

			}// end if

			$attr_value = $this->vars[$var_name]['value'];
			if ($this->vars[$var_name]['type'] == 'serialise') $attr_value = serialize($attr_value);
			$save_vars[$attr_id]['value'] = $attr_value;
			$save_vars[$attr_id]['name']  = $var_name;
			$save_vars[$attr_id]['owning_attrid']  = $owning_attributeid;
			$attr_ids[] = $attr_id;

			// log a message for the asset - this wont get logged until we close the queue
			$message_body = 'The value of attribute "'.$attribute->name.'" for asset "'.$this->name.'" has been changed';
			if (is_scalar($var_data['old_value']) && is_scalar($attr_value)) {
				$message_body .= ' from "'.$var_data['old_value'].'" to "'.$attr_value.'"';
			}
			$message = $ms->newMessage(Array(), 'Asset Attribute Updated', $message_body, 'asset.attributes.fulllog');
			$message->parameters['assetid'] = $this->id;
			$ms->logMessage($message);

			// log a smaller message for users to view - this wont get logged until we close the queue
			require_once SQ_FUDGE_PATH.'/general/general.inc';
			$message_body = 'The value of attribute "'.$attribute->name.'" for asset "'.$this->name.'" has been changed';
			if (is_scalar($var_data['old_value']) && is_scalar($attr_value)) {
				$message_body .= ' from "'.ellipsisize($var_data['old_value'], 30).'" to "'.ellipsisize($attr_value, 30).'"';
			}
			$sml_message = $ms->newMessage(Array(), 'Asset Attribute Updated', $message_body, 'asset.attributes');
			$sml_message->parameters['assetid'] = $this->id;
			$ms->logMessage($sml_message);
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
		assert_valid_db_result($existing);
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
						$ms->abortLog();
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
				$ms->abortLog();
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
							$ms->abortLog();
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
				$ms->abortLog();
				return false;
			}

		}//end new custom attribute values


		// tell, the asset it has updated
		if (!$dont_run_updated && !$this->_updated()) {
			$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
			$ms->abortLog();
			return false;
		}

		unset($this->_tmp['vars_set']);

		// If we get this far, then it's all OK
		$GLOBALS['SQ_SYSTEM']->doTransaction('COMMIT');
		$ms->closeLog();
		$this->informChanges();

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

		// if the value being set is the same as the current value - dont do anything
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
			assert_valid_db_result($result);

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
	* Return the keywords with their replacements
	*
	* The return value is in the form:
	* <pre>
	* Array(
	*     'name' => Array (
	*          'value' => 'page',
	*          'description' => 'the asset name',
	*     ),
	* )
	* </pre>
	*
	* @param boolean $descriptions if TRUE descriptions will be included in the array
	*
	* @access public
	* @return Array(mixed)
	*/
	function getAssetKeywords($descriptions=false)
	{
		$keywords = Array();
		$default_keywords = Array(	'asset_name',
						'asset_short_name',
						'asset_version',
						'asset_created',
						'asset_updated',
						'asset_created_short',
						'asset_updated_short',
						'asset_created_readable',
						'asset_updated_readable',
						'asset_created_by_name',
						'asset_updated_by_name',
						'asset_published',
						'asset_published_short',
						'asset_published_readable',
						'asset_published_by_name',
						'asset_url',
						'asset_href',
						'asset_thumbnail'
						);

		$mm = &$GLOBALS['SQ_SYSTEM']->getMetadataManager();
		$mm_keywords = $mm->generateKeywordReplacements($this, $default_keywords, false);
		foreach ($default_keywords as $keyword) $keywords[$keyword]['value'] = $mm_keywords[$keyword];

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
			$keywords['asset_assetid']['descriptions']				= "The Asset's ID";
			$keywords['asset_name']['description']                  = 'Full name of the asset';
			$keywords['asset_short_name']['description']            = 'Short name of the asset';
			$keywords['asset_version']['description']               = 'Version of the asset being displayed';
			$keywords['asset_created']['description']               = 'The date and time the asset was created (yyyy-mm-dd hh:mm:ss)';
			$keywords['asset_updated']['description']               = 'The date and time the asset was last updated (yyyy-mm-dd hh:mm:ss)';
			$keywords['asset_published']['description']             = 'The date and time the asset was last published (yyyy-mm-dd hh:mm:ss)';
			$keywords['asset_created_short']['description']         = 'The date when the asset was created (yyyy-mm-dd)';
			$keywords['asset_updated_short']['description']         = 'The date when the asset was last updated (yyyy-mm-dd)';
			$keywords['asset_published_short']['description']       = 'The date when the asset was last published (yyyy-mm-dd)';
			$keywords['asset_created_readable']['description']      = 'The date when the asset was created (dd m yyyy h:mm[am pm])';
			$keywords['asset_updated_readable']['description']      = 'The date when the asset was last updated (dd m yyyy h:mm[am pm])';
			$keywords['asset_published_readable']['description']    = 'The date when the asset was last published (dd m yyyy h:mm[am pm])';
			$keywords['asset_created_by_name']['description']       = 'The name of the user who created this asset';
			$keywords['asset_updated_by_name']['description']       = 'The name of the user who last updated this asset';
			$keywords['asset_published_by_name']['description']     = 'The name of the user who last published this asset';
			$keywords['asset_url']['description']                   = 'The absolute url to the asset';
			$keywords['asset_href']['description']                  = 'The relative href to the asset';
			$keywords['asset_thumbnail']['description']             = 'The thumbnail attached to the asset';
		}
		
		return $keywords;

	}//end getAssetKeywords()


	/**
	* Gets replacements for keywords that are specific to this asset
	*
	* @param Array $keywords the keywords to get replacements for
	*
	* @return Array()
	* @access public
	*/
	function getCustomKeywordReplacements($keywords=Array())
	{
		return Array();

	}//end getCustomKeywordReplacements()

	
	/**
	* Processes anything that was previously printed for a custom keyword
	*
	* An example of this might be a keyword replacement for a bodycopy. When submitted, we need to process
	* The bodycopy here
	*
	* @param Array $keywords the keywords that were originally printed
	*
	* @return boolean
	* @access public
	*/
	function processCustomKeywords($keywords=Array())
	{
		return true;

	}//end processCustomKeywords()



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
		if (!empty($publishers) && in_array($GLOBALS['SQ_SYSTEM']->currentUserId(), $publishers)) return true;
		
		// if asset is not live we really need to check for write access
		if ($this->status < SQ_STATUS_LIVE) {
			// if we are in rollback view mode, bypass extra write access checks or we
			// we always be returning false here (writeAccess returns false)
			if (SQ_ROLLBACK_VIEW) return $this->_checkPermissionAccess(SQ_PERMISSION_WRITE, $assetids);
			return $this->writeAccess('', $assetids);
		}

		return $this->_checkPermissionAccess(SQ_PERMISSION_READ, $assetids);

	}//end readAccess()


	/**
	* Returns true if at least one of the passed assetids has write access to this asset
	*
	* @param string	$lock_type	the type of write access required - if blank returns a plain check against
	*							permissions set for this asset, otherwise returns effective access based on locking
	*							and status
	* @param Array	$assetids	An array of user_group and/or user ids to check for write access
	*							If the array is empty, the current user will be checked
	*
	* @return boolean
	* @access public
	*/
	function writeAccess($lock_type, $assetids=Array())
	{
		if (SQ_ROLLBACK_VIEW) return false;
		if (!$this->id || !empty($GLOBALS['SQ_INSTALL'])) return true;
		if ($this->status == SQ_STATUS_LIVE) return $this->liveEditAccess($lock_type);
		return $this->checkAccess(SQ_PERMISSION_WRITE, $lock_type, $assetids);

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
	function adminAccess($lock_type, $assetids=Array())
	{
		if (SQ_ROLLBACK_VIEW) return false;
		if (!$this->id || !empty($GLOBALS['SQ_INSTALL']) || !empty($this->_tmp[__CLASS__.'_in_create_cascading'])) return true;
		$access = $this->checkAccess(SQ_PERMISSION_ADMIN, $lock_type, $assetids);
		if ($access && $this->status == SQ_STATUS_LIVE) return $this->liveEditAccess($lock_type);
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
	* @return boolean
	* @access public
	* @see canliveEdit()
	*/
	function liveEditAccess($lock_type)
	{
		if (empty($lock_type) || $this->canLiveEdit($lock_type)) return $this->checkAccess(SQ_PERMISSION_WRITE, $lock_type);
		return false;

	}//end liveEditAccess()


	/**
	* Returns true if the current user could live edit this asset if they had the lock
	*
	* Live edit means the asset is currently SQ_STATUS_LIVE but this
	* user can edit the asset without going through a safe edit
	* NOTE: will return false if asset is not currently live
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
				if (!in_array('attributes', array_keys($locks))) return true;
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
		return ($this->status >= SQ_STATUS_LIVE && $this->_checkPermissionAccess(SQ_PERMISSION_READ, Array('0')));

	}//end effectiveUnrestricted()


	/**
	* Returns true if at least one of the passed assetids has the passed permission
	* Can perform effective access check based on current status, lock, workflow, and metadata
	* NOTE: Only for write and above access
	*
	* @param int	$perm		the permission to check access for
	* @param string	$lock_type	the type of write access required - if blank returns a plain check against
	*							permissions set for this asset, otherwise returns effective access based on locking
	*							and status
	* @param array	$assetids	An array of user_group and/or user ids to check for permission
	*							If the array is empty, the current user will be checked
	*
	* @return boolean
	* @access public
	*/
	function checkAccess($perm, $lock_type, $assetids=Array())
	{
		if ($perm == SQ_PERMISSION_READ) {
			trigger_error(__CLASS__.'::'.__FUNCTION__.' is not to be used for read access', E_USER_ERROR);
		}

		if ($lock_type == '') {
			return $this->_checkPermissionAccess($perm, $assetids);

		} else {
			// effective access is only valid for the current user
			if (!empty($assetids)) return false;

			$locks = $GLOBALS['SQ_SYSTEM']->am->getLockInfo($this->id, $lock_type, true);
			// if there is no lock or
			if (empty($locks)) return false;

			foreach ($locks as $lock) {
				// if any one of these locks is not owned by the current user they can't edit
				if (empty($lock) || $lock['userid'] != $GLOBALS['SQ_SYSTEM']->currentUserId()) return false;
			}

			// if we are in a state of total approval no editing allowed
			if ($this->status & SQ_SC_STATUS_ALL_APPROVED) return false;

			// if the asset is archived no editing allowed
			if ($this->status & SQ_STATUS_ARCHIVED) return false;

			// otherwise they can edit if they have permission access
			return $this->_checkPermissionAccess($perm);

		}// end if

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
			$assetids = $GLOBALS['SQ_SYSTEM']->am->getParents($GLOBALS['SQ_SYSTEM']->currentUserId(), Array('user', 'user_group'), false);
			$assetids[] = $GLOBALS['SQ_SYSTEM']->currentUserId();
		}

		if (!$GLOBALS['SQ_PURGING_TRASH']) {
			// if we are in workflow no editing allowed
			// except by those users who can currently approve the asset
			if ($this->status & SQ_SC_STATUS_PENDING) {
				if ($perm < SQ_PERMISSION_ADMIN) {
					$wfm = &$GLOBALS['SQ_SYSTEM']->getWorkflowManager();
					$publishers = $wfm->whoCanPublish($this->id);
					if (in_array($GLOBALS['SQ_SYSTEM']->currentUserId(), $publishers)) return true;

					if (empty($publishers)) {
						// nobody can publish - great
						// better let admins take a punt
						return $this->_checkPermissionAccess(SQ_PERMISSION_ADMIN, $assetids);
					}
				}
				if ($perm != SQ_PERMISSION_READ) return false;
			}
		}

		$grants  = $GLOBALS['SQ_SYSTEM']->am->getPermission($this->id, $perm, true);
		$revokes = $GLOBALS['SQ_SYSTEM']->am->getPermission($this->id, $perm, false, false);

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

	}//end _checkPermissionAccess()


	/**
	* Called when the any permission is updated for this asset
	*
	* @return boolean	whether everything is OK
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
		if (!$this->readAccess()) {
			$GLOBALS['SQ_SYSTEM']->paintLogin('Login', 'You do not have permission to access <i>'.$this->name.'</i>');
			return;
		}

		if($this->charset) header("Content-type: text/html; charset=$this->charset");
		if($this->languages) header("Content-language: $this->languages");

		$db = &$GLOBALS['SQ_SYSTEM']->db;
		$url = strip_url(current_url(false, true));

		// Have they set a design name that they want to view ??
		if (!empty($_GET['SQ_DESIGN_NAME'])) {
			$design_name = 'user_design::'.$_GET['SQ_DESIGN_NAME'];
		} elseif (!empty($_SESSION['SQ_DESIGN_NAME'])) {
			$design_name = 'user_design::'.$_SESSION['SQ_DESIGN_NAME'];
		} else {
			$design_name = 'system_design::frontend';
		}

		$result = $GLOBALS['SQ_SYSTEM']->am->getDesignFromURL($url, $design_name);
		
		// if no design was found, try and get the default frontend
		// design if we were not already trying to get it
		if (empty($result) && $design_name != 'system_design::frontend') {
			$result = $GLOBALS['SQ_SYSTEM']->am->getDesignFromURL($url, 'system_design::frontend');
		}
		
		// if no design was found, try using the asset's URL from the DB
		if (empty($result)) {
			$url = strip_url($this->getUrl(), true);
			$result = $GLOBALS['SQ_SYSTEM']->am->getDesignFromURL($url, $design_name);
			if (empty($result) && $design_name != 'system_design::frontend') {
				$result = $GLOBALS['SQ_SYSTEM']->am->getDesignFromURL($url, 'system_design::frontend');
			}
		}

		if ($result) {
			// we have found the design to use
			$design = &$GLOBALS['SQ_SYSTEM']->am->getAsset($result['designid'], $result['type_code']);
			$design->paint($this);
		} else {
			// we can't find a design, oh well let's just print out our body
			$this->printBody();
		}

	}//end printFrontend()


	/**
	* Called by the design to print extra material in the head of the HTML document for this asset
	* it's up to the kids to override
	*
	* @access public
	*/
	function printHead()
	{
		echo '<!-- HEAD PRINTED BY ASSET.INC -->';

	}//end printHead()
	
	
	/**
	* Called by the design to print the body of this asset
	* it's up to the kids to override
	*
	* @access public
	*/
	function printBody()
	{

	}//end printBody()


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

	}//end paintBackend()


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
		return str_replace(' ', '_', $this->type().'_'.$this->id);

	}//end getPrefix()


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

			default :
				return $ei->process($this, $o, false);

		}// end switch

	}//end processBackend()


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

	}//end getEI()


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

	}//end getEditFns()


//--        URL's & HREF's        --//


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
		return sq_web_path('data').'/assets/'.$this->type().'/'.$this->id;

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
			assert_valid_db_result($result);
			$this->_tmp['paths'] = $result;

		}// end if

		return $this->_tmp['paths'];

	}//end getWebPaths()


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
			$parents = $GLOBALS['SQ_SYSTEM']->am->getLinks($this->id, SQ_SC_LINK_WEB_PATHS, '', null, 'minor');
			for($i = 0; $i < count($parents); $i++) {
				$parent = &$GLOBALS['SQ_SYSTEM']->am->getAsset($parents[$i]['majorid'], $parents[$i]['major_type_code']);
				if (is_null($parent)) continue;
				$bad_paths = $GLOBALS['SQ_SYSTEM']->am->webPathsInUse($parent, $inserts, $this->id);
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

	}//end saveWebPaths()


//--        LOOKUPS        --//


	/**
	* Returns a list of lookup information associated with this asset
	*
	* @param string	$field	if only one field is required pass it's name through
	*						(url, http, https, root_urlid)
	*
	* @return Array()
	* @access public
	*/
	function getLookups($field='')
	{
		if (!isset($this->_tmp['lookups'])) {
			$db = &$GLOBALS['SQ_SYSTEM']->db;

			$sql = 'SELECT url, http, https, root_urlid
					FROM '.SQ_TABLE_RUNNING_PREFIX.'asset_lookup ';
			$where = 'assetid = '.$db->quote($this->id);
			$where = $GLOBALS['SQ_SYSTEM']->constructRollbackWhereClause($where);

			$result = $db->getAll($sql.$where);
			assert_valid_db_result($result);
			$this->_tmp['lookups'] = $result;

		}// end if

		if (!$field) {
			return $this->_tmp['lookups'];
		} else {
			$ret_val = Array();
			foreach($this->_tmp['lookups'] as $data) $ret_val[] = $data[$field];
			return $ret_val;
		}

	}//end getLookups()


	/**
	* Returns a list of design lookup information associated with this asset
	*
	* @return Array()
	* @access public
	*/
	function getDesignLookups()
	{
		if (!isset($this->_tmp['design_lookups'])) {
			$this->_tmp['design_lookups'] = Array();

			$db = &$GLOBALS['SQ_SYSTEM']->db;

			$sql = 'SELECT l.url, ld.name, ld.designid
					FROM '.SQ_TABLE_RUNNING_PREFIX.'asset_lookup l
							INNER JOIN '.SQ_TABLE_RUNNING_PREFIX.'asset_lookup_design ld ON l.url = ld.url
					';
			$where = 'l.assetid = '.$db->quote($this->id);
			$where = $GLOBALS['SQ_SYSTEM']->constructRollbackWhereClause($where, 'l');

			$result = $db->query($sql.$where);
			assert_valid_db_result($result);
			
			while(null !== ($row = $result->fetchRow())) {
				if (!isset($this->_tmp['design_lookups'][$row['url']])) $this->_tmp['design_lookups'][$row['url']] = Array();
				$this->_tmp['design_lookups'][$row['url']][$row['name']] = $row['designid'];
			}
			$result->free();

		}// end if

		return $this->_tmp['design_lookups'];

	}//end getDesignLookups()


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
		unset($this->_tmp['url']);
		unset($this->_tmp['href']);

		require_once SQ_FUDGE_PATH.'/db_extras/db_extras.inc';
		$db = &$GLOBALS['SQ_SYSTEM']->db;
		$GLOBALS['SQ_SYSTEM']->doTransaction('BEGIN');


		// Remove all the old URLs
		$sql = 'SELECT url, http, https
				FROM '.SQ_TABLE_RUNNING_PREFIX.'asset_lookup l
				'.$GLOBALS['SQ_SYSTEM']->constructRollbackWhereClause('assetid = '.$db->quote($this->id), 'l');

		$old_urls = $db->getAll($sql);
		assert_valid_db_result($old_urls);

		$current_urls = Array();
		foreach ($old_urls as $url_data) $current_urls[] = $db->quote($url_data['url']);

		if (!empty($current_urls)) {
			$where = 'url IN ('.implode(',', $current_urls).')';

			if (!$GLOBALS['SQ_SYSTEM']->rollbackDelete('asset_lookup_design', $where)) {
				$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
				return false;
			}

			$where = 'assetid = '.$db->quote($this->id);
			if (!$GLOBALS['SQ_SYSTEM']->rollbackDelete('asset_lookup', $where)) {
				$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
				return false;
			}
		}


		// if we have paths then do some url inserting
		if (!empty($paths)) {

			$num_paths = count($paths);

			$our_design_links = $GLOBALS['SQ_SYSTEM']->am->getLinks($this->id, SQ_LINK_TYPE_3, 'design', false);
			$our_designs = Array();
			foreach($our_design_links as $link) {
				if (preg_match('/^(system_design|user_design)::.*$/', $link['value'])) {
					$our_designs[$link['value']] = $link['minorid'];
				}// end if
			}// end foreach

			$parents = $GLOBALS['SQ_SYSTEM']->am->getLinks($this->id, SQ_SC_LINK_WEB_PATHS, '', null, 'minor');

			$done_urls    = Array();
			$done_parents = Array();

			for($i = 0; $i < count($parents); $i++) {
				if (in_array($parents[$i]['majorid'], $done_parents)) continue;
				$parent = &$GLOBALS['SQ_SYSTEM']->am->getAsset($parents[$i]['majorid'], $parents[$i]['major_type_code']);
				if (is_null($parent)) continue;
				$done_parents[] = $parent->id;

				$parent_designs = $parent->getDesignLookups();

				$parent_urls    = $parent->getLookups();
				for($j = 0, $num_parent_urls = count($parent_urls); $j < $num_parent_urls; $j++) {
					for($k = 0; $k < $num_paths; $k++) {
						$new_url = $parent_urls[$j]['url'].'/'.$paths[$k];
						if (isset($done_urls[$new_url])) continue;

						$http  = ($this->force_secure) ? '0' : $parent_urls[$j]['http'];
						$https = $parent_urls[$j]['https'];
						// if we have no protocol then we can just ignore this URL
						if (!$http && !$https) continue;

						$values = Array('url'        => $new_url,
										'assetid'    => $this->id,
										'http'       => $http,
										'https'      => $https,
										'root_urlid' => $parent_urls[$j]['root_urlid'],
										);
						
						$done_urls[$new_url] = $values;

						if (!$GLOBALS['SQ_SYSTEM']->rollbackInsert('asset_lookup', $values)) {
							$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
							return false;
						}

						// get any of our parent's designs that we haven't overridden
						$designs = $our_designs;
						if (!empty($parent_designs[$parent_urls[$j]['url']])) {
							foreach($parent_designs[$parent_urls[$j]['url']] as $name => $designid) {
								if (isset($designs[$name])) continue;
								$designs[$name] = $designid;
							}
						}
						foreach($designs as $name => $designid) {
							$values = Array('url'      => $new_url,
											'name'     => $name,
											'designid' => $designid,
											);
							if (!$GLOBALS['SQ_SYSTEM']->rollbackInsert('asset_lookup_design', $values)) {
								$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
								return false;
							}
						}// endforeach

					}// end for paths

				}// end for parent_urls

			}// end for parents

			foreach ($old_urls as $url_data) {
				$old_url = $url_data['url'];
				if (!isset($done_urls[$old_url])) {
					$rm = &$GLOBALS['SQ_SYSTEM']->am->getSystemAsset('remap_manager');
					$new_url = array_shift(array_keys($done_urls));

					if ($new_url != '') {	// don't add a remap if we don't have a URL to map to

						if ($url_data['http'])  $rm->addRemapURL('http://'.$old_url,  'http://'.$new_url);
						if ($url_data['https']) $rm->addRemapURL('https://'.$old_url, 'https://'.$new_url);

					}

				}
			}

		}// end if count paths

		$GLOBALS['SQ_SYSTEM']->doTransaction('COMMIT');
		return true;

	}//end updateLookups()


//--        INDEXING        --//


	/**
	* Retrieves a list of key data from this asset (used by the indexing service).
	* Subclasses override this to give the content they want to make searchable.
	*
	* @return array				an array of all the keydata for this asset. Keydata is of the form
	* <pre>
	* Array (
	*	Array (
	*		'value'     => ...,
	*		'type'      => ..., (text, date, number, etc.)
	*		'component' => ..., (whatever you want, just as long as this asset
	*							 knows how to deal with it if it needs to)
	*		), ...
	* )
	* </pre>
	* @access public
	*/
	function getKeyData()
	{
		$sm = &$GLOBALS['SQ_SYSTEM']->am->getSystemAsset('search_manager');
		if (!$sm) return Array();

		require_once SQ_FUDGE_PATH.'/general/text.inc';

		// index metadata
		$mm = &$GLOBALS['SQ_SYSTEM']->getMetadataManager();
		$schemaids = $mm->getSchemas($this->id, true);
		$keydata   = $sm->getMetadataKeyData($this->id, $schemaids);

		// index attributes
		$weightings = $this->getWeightings();
		if (!isset($weightings['__name__']['weighting'])) $weightings['__name__']['weighting'] = 1;

		// supply name
		$value = strtolower($this->name);
		foreach (get_word_counts($value) as $word => $count) {
			$this->buildKeyData($keydata, $word, 'text', 'attr:'.$this->type().':__name__', $count * $weightings['__name__']['weighting']);
		}

		foreach ($this->vars as $var_name => $var_data) {
			if (!isset($weightings[$var_name])) continue;

			// skip non scalar values because we cant index them properly
			if (!is_scalar($var_data['value'])) continue;

			$attribute = &$this->getAttribute($var_name);
			$values = $attribute->getKeyData();
			if (empty($values)) continue;

			$comp_name = 'attr:'.$this->type().':'.$var_name;
			foreach ($values as $value => $count) {
				$this->buildKeyData($keydata, $value, $var_data['type'], $comp_name, $count * $weightings[$var_name]['weighting']);
			}
		}

		return $keydata;

	}//end getKeyData()


	/**
	* Informs the indexing service that this asset's contents have changed, and
	* that it should reflect this by retrieving new key data.
	*
	* @return void
	* @access protected
	*/
	function informChanges()
	{
		$em =& $GLOBALS['SQ_SYSTEM']->getEventManager();
		$em->broadcastEvent($this, 'requestReindex');

	}//end informChanges()


	/**
	* Save component weightings to the database
	*
	* @param array	$newWeightings	the component wieghtings to save
	*
	* @return void
	* @access public
	*/
	function saveComponentWeightings($new_weightings)
	{
		$sm = &$GLOBALS['SQ_SYSTEM']->am->getSystemAsset('search_manager');
		if (!$sm) return false;
		$dirty = false;
		$weightings =& $this->getWeightings();

		foreach ($new_weightings as $component => $weighting_data) {
			if ($weighting_data['default']) {
				$weighting = null;
				if (!$weightings[$component]['default']) $dirty = true;
			} else {
				$weighting = $weighting_data['weighting'];
				$weightings[$component]['default'] = $weighting_data['default'];
				if (!$dirty) $weightings[$component]['weighting'] = $weighting;
			}
			$sm->setComponentWeighting($this->id, $component, $weighting);
		}

		if ($dirty) {
			// need to reload weightings for defaults - next call to getWeightings will do this
			unset($this->_weightings);
			$this->_weightings = null;
		}

	}//end saveComponentWeightings()


	/**
	* Adds a piece of keydata to the array to be passed to the indexing service.
	*
	* @param array		&$keydata			a reference to the keydata array being built
	* @param string		$value				the key data value
	* @param string		$type				the key data type, defaults to 'text' (e.g. text, date, number, etc.)
	* @param string		$component			the component to which this data pertains to
	* @param int		$score				weighting of this word for the search
	* @param int		$force_processing	if the initial checking routines should be skipped
	*
	* @return void
	* @access protected
	*/
	function buildKeyData(&$keydata, $value, $type='text', $component='', $score=1, $force_processing=0)
	{
		// checking if we should skip initial test of given values, because
		// they are almost always null if we are indexing metadata fields
		if ($force_processing != 1 && (!is_array($keydata) || $score <= 0)) return false;

		array_push($keydata, Array (
									'value'     => $value,
									'type'      => $type,
									'component' => $component,
									'score'     => $score,
									)
					);

		return true;

	}//end buildKeyData()


	/**
	* Retrieves weightings for this asset and its components - loads on demand.
	*
	* @return Array
	* @access public
	*/
	function &getWeightings()
	{
		$sm =& $GLOBALS['SQ_SYSTEM']->am->getSystemAsset('search_manager');
		if (!$sm) return false;

		return $sm->getAssetWeightings($this->id);

	}//end getWeightings()


}//end class

?>
