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
	* Whether or not this asset is restricted
	* Changes to this var's state probably should be made in _updated()
	*
	* @var boolean
	*/
	var $_restricted = true;

	/**
	* The current data path for this asset
	* Changes to this var's state probably should be made in _updated()
	*
	* @var string
	*/
	var $data_path = '';

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
	}


	/**
	* The type that this asset is
	*
	* @return string
	* @access public
	*/
	function type()
	{
		return get_class($this);
	}//end type();


	/**
	* Create this asset
	* The return value will be:
	*    FALSE if the asset wa snot created
	*    the ID of the newly created link if the asset and intital link were created
	*    TRUE if the asset was created but $link was empty
	*
	* @param Array	&$link	information used to create the initial link, 
	*						Array ('asset' => [ref major asset to create link under], 
	*						'link_type' => SQ_LINK_?, 'value' => [link value], 
	*						'sort_order' => [link sort order], 'dependant' = [0|1])
	*
	* @return int|false
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
			if (!isset($link['value']))	     $link['value']      = '';
			if (!isset($link['sort_order'])) $link['sort_order'] = -1;
			if (!isset($link['dependant']))	 $link['dependant']  = 0;
			$link['asset']->aquireLock();
			$linkid = $link['asset']->createLink($this, $link['link_type'], $link['value'], $link['sort_order'], $link['dependant']);
			$link['asset']->releaseLock();
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

		$sql = 'SELECT assetid, name, short_name, status, type_code, last_updated, last_userid
				FROM sq_asset
				WHERE assetid = '.$db->quote($assetid);
		$result = $db->getRow($sql, null, DB_FETCHMODE_ORDERED);
		if (DB::isError($result)) trigger_error($result->getMessage().'<br/>'.$result->getUserInfo(), E_USER_ERROR);

		if (is_null($result)) {
			return;
		}

		list($this->id, $this->name, $this->short_name, $this->status, $type_code, $this->last_updated, $this->last_userid) = $result;
		$this->last_updated = iso8601_ts($this->last_updated);
		unset($result);

		// make sure the asset we are loading is of the same type as our class
		if ($type_code != $this->type()) {
			trigger_error('Asset #'.$assetid.' is not a '.$this->type(), E_USER_WARNING);
			$this->id   = null;
			$this->last_updated = null;
			$this->last_userid  = null;
			return;
		}

		$this->_loadVars();

	}//end load()


	/**
	* Loads the vars for this asset/ asset type into the vars array
	*
	* @access private
	*/
	function _loadVars()
	{
		// firstly let's setup the data path
		$this->data_path = SQ_DATA_PATH.'/'.(($this->_restricted) ? '' : 'un').'restricted/'.$this->type().'/'.$this->id;

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
	* @access private
	*/
	function _updated()
	{

		$last_updated	= time();
		$last_userid	= $GLOBALS['SQ_SYSTEM']->currentUserId();
		$name			= $this->_getName();
		$short_name		= $this->_getName(true);

		// if we have an ID hit the DB
		if ($this->id) { 

			$db = &$GLOBALS['SQ_SYSTEM']->db;
			$sql = 'UPDATE sq_asset
					SET name         = '.$db->quote($name).',
						short_name   = '.$db->quote($short_name).',
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

			$GLOBALS['SQ_SYSTEM']->broadcastEvent('AssetUpdate', Array( 'assetid'          => $this->id,
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
	* Returns a name to use to describe this instance of this asset
	* should be overridden, used to set $this->name and $this->short_name
	*
	* @param bool	$short_name	whether or not we are after the shortname or the full name
	*
	* @access private
	*/
	function _getName($short_name=false)
	{
		return ucwords(str_replace('_', ' ', $this->type())).' #'.$this->id;
	}//end _getName()


	/**
	* Returns an array of statii that the current user can set for this asset
	* Value returned is a key => value array for a drop down
	*
	* @return array
	* @access public
	*/
	function getAvailableStatii() {
		include_once SQ_INCLUDE_PATH.'/workflow_manager.inc';
		$wfm = new Workflow_Manager($this);

		$statii = Array();

		switch ($wfm->state) {
			case 'new' :
			case 'idle' :
				if ($this->writeAccess(Array(), false)) {
					if ($this->status != SQ_STATUS_LIVE) {
						if (empty($wfm->steps) && empty($wfm->inherited_workflows)) {
							// no direct workflows are set and
							// no inherited workflows are set
							// so anyone with admin access can publish
							if ($this->adminAccess(Array(), false)) {
								$statii[SQ_STATUS_LIVE] = 'Approve and Make Live';
							}
						} else {
							// so we have workflows, but none of them are running
							// so lets see what will happen if this user approves
							// to determine if they can live edit
							$status = $wfm->testPublish($GLOBALS['SQ_SYSTEM']->currentUserId());
							if ($status == 'complete') {
								$statii[SQ_STATUS_LIVE] = 'Approve and Make Live';
							} else {
								$statii[SQ_STATUS_LIVE] = 'Make Live';
							}
						}
					} else {
						$statii[SQ_STATUS_UNDER_CONSTRUCTION] = 'Place Under Construction';
						$statii[SQ_STATUS_EDITING] = 'Safe Edit';
					}
				}
				break;
			case 'running' :
				// workflows are currently running,
				// so we are looking for publishers

				// if this asset is currently pending approval, there
				// must be a workflow in progress, so check to see if the
				// user logged in can publish
				$publishers = $wfm->whoCanPublish();
				
				$userid = $GLOBALS['SQ_SYSTEM']->currentUserId();
				if (in_array($userid, $publishers)) {
					// the current user can approve this asset
					// to keep the workflow going
					$status = $wfm->testPublish($GLOBALS['SQ_SYSTEM']->currentUserId());
					if ($status == 'complete') {
						$statii[SQ_STATUS_LIVE] = 'Approve and Make Live';
					} else {
						$statii[SQ_STATUS_LIVE] = 'Approve Changes';
					}
					$statii[SQ_STATUS_UNDER_CONSTRUCTION] = 'Reject Changes';
				}
				break;
		}

		return $statii;
	}// end getAvailableStatii()

	
	/**
	* Returns a description representing the passed or current status
	*
	* @param int	$status	the status to get the description for - leave blank
	*						to get the description for the current status of this asset
	*
	* @return string
	* @access public
	*/
	function getStatusDescription($status=0)
	{
		if (!$status) $status = $this->status;
		$description = '';

		switch ($status) {
			case SQ_STATUS_LIVE :
				$description = 'Live';
				break;
			case SQ_STATUS_EDITING :
				$description = 'Safe Editing';
				break;
			case SQ_STATUS_PENDING_APPROVAL   :
				$description = 'Pending Approval';
				break;
			case SQ_STATUS_EDITING_APPROVAL   :
				$description = 'Safe Editing Pending Approval';
				break;
			case SQ_STATUS_UNDER_CONSTRUCTION :
				$description = 'Under Construction';
				break;
		}

		return $description;
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
		// dont change if we already have this status
		if ($this->status == $new_status) return false;

		$old_status = $this->status;
		$current_userid = $GLOBALS['SQ_SYSTEM']->currentUserid();
		$pending_statii = Array(SQ_STATUS_PENDING_APPROVAL, SQ_STATUS_EDITING_APPROVAL);

		switch ($new_status) {
			case SQ_STATUS_LIVE :
				include_once SQ_INCLUDE_PATH.'/workflow_manager.inc';
				$wfm = new Workflow_Manager($this);

				if ($wfm->state != 'running' && empty($wfm->steps) && empty($wfm->inherited_workflows)) {
					// no direct workflow, no inherited, and no running
					// so anyone with admin access can make this live
					if ($this->adminAccess(Array(), false)) {
						$this->status = SQ_STATUS_LIVE;
						break;
					}
				}

				// if the workflow is running, check to make sure
				// our current status reflects this
				if ($wfm->state == 'running') {
					if (!in_array($this->status, $pending_statii)) $this->status = SQ_STATUS_PENDING_APPROVAL;
				}

				if (in_array($this->status, $pending_statii)) {
					// we are doing a workflow and someone has just
					// approved the asset, so record this
					if (!$wfm->recordPublish($current_userid)) {
						trigger_error('Approval by user '.$current_userid.' not set in processStatusChange', E_USER_WARNING);
						$this->status = $old_status;
						return false;
					}

					// send an internal message
					$ms = &$GLOBALS['SQ_SYSTEM']->getMessagingService();
					$user = &$GLOBALS['SQ_SYSTEM']->am->getAsset($current_userid);
					$body = 'User "'.$user->name.'" has approved '.$this->type().' "'.$this->name.'" to go live.';
					$ms->sendMessage(array_keys($this->getPermission(SQ_PERMISSION_ADMIN)), 0, 'Asset Changes Approved', $body);

					if ($wfm->state == 'complete') {
						// all workflows are done so we
						// can go ahead and make this live
						$this->status = SQ_STATUS_LIVE;

						// send an internal message
						$body = 'The workflow process for '.$this->type().' "'.$this->name.'" has been completed. "'.$this->name.'" is now live.';
						$ms->sendMessage(array_keys($this->getPermission(SQ_PERMISSION_ADMIN)), 0, 'Workflow Completed', $body);
					}

				} else {
					if (!$this->writeAccess(Array(), false)) return false;

					if ($this->status == SQ_STATUS_UNDER_CONSTRUCTION) {
						// we are going to make this asset live
						// by starting the workflow process
						$status = $wfm->testPublish($current_userid);
						if ($status == 'complete') {
							$this->status = SQ_STATUS_LIVE;
						} else {
							if (!$wfm->start()) {
								trigger_error('Workflow failed to start for "'.$this->name.'"', E_USER_WARNING);
								$this->status = $old_status;
								return false;
							}

							$this->status = SQ_STATUS_PENDING_APPROVAL;

							// send an internal message
							$ms = &$GLOBALS['SQ_SYSTEM']->getMessagingService();
							$user = &$GLOBALS['SQ_SYSTEM']->am->getAsset($current_userid);
							$body = 'User "'.$user->name.'" has requested '.$this->type().' "'.$this->name.'" to go live from Under Construction.';
							$ms->sendMessage(array_keys($this->getPermission(SQ_PERMISSION_ADMIN)), 0, 'Workflow Started', $body);
						}

					} else if ($this->status == SQ_STATUS_EDITING) {
						// asset is currently safe editing
						// so we need to start the workflow process
						$status = $wfm->testPublish($current_userid);
						if ($status == 'complete') {
							$this->status = SQ_STATUS_LIVE;
						} else {
							if (!$wfm->start()) {
								trigger_error('Workflow failed to start for "'.$this->name.'"', E_USER_WARNING);
								$this->status = $old_status;
								return false;
							}

							$this->status = SQ_STATUS_EDITING_APPROVAL;

							// send an internal message
							$ms = &$GLOBALS['SQ_SYSTEM']->getMessagingService();
							$user = &$GLOBALS['SQ_SYSTEM']->am->getAsset($current_userid);
							$body = 'User "'.$user->name.'" has requested '.$this->type().' "'.$this->name.'" to go live from Safe Editing.';
							$ms->sendMessage(array_keys($this->getPermission(SQ_PERMISSION_ADMIN)), 0, 'Workflow Started', $body);
						}
					}
				}

				$wfm->save();
				break;

			case SQ_STATUS_EDITING :
				if (!$this->writeAccess(Array(), false)) return false;
				if ($this->status == SQ_STATUS_LIVE) {
					// we are safe editing
					$this->status = SQ_STATUS_EDITING;
				}
				break;

			case SQ_STATUS_PENDING_APPROVAL :
				break;

			case SQ_STATUS_UNDER_CONSTRUCTION :
				include_once SQ_INCLUDE_PATH.'/workflow_manager.inc';
				$wfm = new Workflow_Manager($this);

				$publishers = $wfm->whoCanPublish();

				if ($this->status == SQ_STATUS_LIVE && $this->adminAccess(Array(), false)) {
					// if we are currently live, we can set to
					// under construction without problems
					$this->status = SQ_STATUS_UNDER_CONSTRUCTION;
				} else if (in_array($this->status, $pending_statii) && in_array($current_userid, $publishers)) {
					// we are currently in workflow and someone has decide
					// to reject the changes and cancel the workflow
					
					if ($this->status == SQ_STATUS_EDITING_APPROVAL) $this->status = SQ_STATUS_EDITING;
					else $this->status = SQ_STATUS_UNDER_CONSTRUCTION;

					if (!$wfm->cancel()) {
						trigger_error('Workflow cancel failed', E_USER_WARNING);
						$this->status = $old_status;
					} else {
						// send an internal message
						$ms = &$GLOBALS['SQ_SYSTEM']->getMessagingService();
						$user = &$GLOBALS['SQ_SYSTEM']->am->getAsset($current_userid);
						$body = 'User "'.$user->name.'" has rejected the changes made to '.$this->type().' "'.$this->name.'". The status of "'.$this->name.'" has reverted to '.$this->getStatusDescription();
						$ms->sendMessage(array_keys($this->getPermission(SQ_PERMISSION_ADMIN)), 0, 'Asset Changes Rejected', $body);
					}
				}
				break;

			default :
				trigger_error("Can not process status change, status '$new_status' not recognised", E_USER_WARNING);
				return false;
		}

		if ($old_status != $this->status) {
			$db = &$GLOBALS['SQ_SYSTEM']->db;
			$sql = 'UPDATE sq_asset
					SET status    = '.$db->quote($this->status).'
					WHERE assetid = '.$db->quote($this->id);

			$result = $db->query($sql);
			if (DB::isError($result)) {
				trigger_error($result->getMessage().'<br/>'.$result->getUserInfo(), E_USER_WARNING);
				return false;
			}

			$GLOBALS['SQ_SYSTEM']->broadcastEvent('AssetStatusUpdate', Array( 'assetid'    => $this->id,
																			  'old_status' => $old_status,
																			  'new_status' => $this->status,
																			));

			$this->_updated();
		}

		return true;

	}//end processStatusChange()


	/**
	* Creates a duplicate of this asset.
	* The only child assetst that are duplicated are the ones that are DEPENDANTLY linked to this asset
	* NO OTHER linked assets are duplicated
	* Returns a reference to the new object or if an error occured NULL
	*
	* @param Array	&$link				information used to create the initial link, 
	*									Array ('asset' => [ref major asset to create link under], 
	*									'link_type' => SQ_LINK_?, 'value' => [link value], 
	*									'sort_order' => [link sort order], 'dependant' = [boolean])
	* @param Array	&$dupe_map			a map for that can be used for re-indexing data held in 
	*									assets of the form Array([old asset id] => [new asset id]);
	* @param bool	$dupe_dependants	whether or not to duplicate any assets that are dependantly 
	*									linked to this asset, only works if $link is not empty
	* @param bool	$dupe_directory		whether or not to duplicate this assets data directory
	*
	* @return object Asset | NULL
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

		// Now we set all the attributes
		foreach($this->vars as $name => $data) {
			if (!$dupe->setAttrValue($name, $data['value'])) {
				$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
				return $null;
			}
		}

		if (!empty($link)) {
			// create the initial link
			if (!isset($link['value']))	     $link['value']      = '';
			if (!isset($link['sort_order'])) $link['sort_order'] = -1;
			if (!isset($link['dependant']))	 $link['dependant']  = 0;
			$linkid = $link['asset']->createLink($dupe, $link['link_type'], $link['value'], $link['sort_order'], $link['dependant']);
			if(empty($linkid)) {
				$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
				return $null;
			}


			// OK now that we are linked up let's our dependants
			if ($dupe_dependants) {
				$dependant_links = $this->getLinks(SQ_LINK_SIGNIFICANT, '', true, 'major', null, 1);

				if (!empty($dependant_links)) {
					$create_link = Array('asset'		=> &$dupe, 
										'link_type'		=> null, 
										'value'			=> null,
										'sort_order'	=> -1,
										'dependant'		=> '1');
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

							$linkid = $dupe->createLink($duped_child, $data['link_type'], $data['value'], $data['sort_order'], '1');
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
	* Remaps existing assetids used by this asset to new ones
	*
	* @param Array	&$map	a map of the form Array([old asset id] => [new asset id]);
	*
	* @return bool
	* @access public
	*/
	function remapAssetids($map)
	{
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
				if ($tmp->canLinkToType($link['type_code'], $link['link_type']) !== true) {
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


		// All is OK so override ourselves with the temporary
		if ($all_ok) {
			$this = $tmp;
			$GLOBALS['SQ_SYSTEM']->doTransaction('COMMIT');
			return true;

		// bugger some thing went wrong
		} else {
			$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
			return false;

		}// end if

	}//end cast()


	/**
	* Returns an array of all assets types that this asset extends from
	*
	* @param string $code_name  the code name for the asset that you want to check
	*
	* @access public
	*/
	function getTypeAncestors()
	{
		return $GLOBALS['SQ_SYSTEM']->am->getTypeAncestors($this->type());
	}//end getTypeAncestors()


	/**
	* Get all asset ids that are above this asset in the various trees in which it exists
	*
	* @param string|array	$type_code			the type of asset that is linked (eg 'Page', 'File', etc)
	*											if an array returns link if matches any of the array values
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
	* @param string|array	$type_code			the type of asset that is linked (eg 'Page', 'File', etc) 
	*											if an array returns link if matches any of the array values
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
	* Can the current user forceably aquire the current lock on this asset?
	*
	* @return boolean
	* @access public
	*/
	function canForceablyAquireLock() {
		$current_lock = $this->getLockInfo();
		if (empty($current_lock)) return false;

		$user = &$GLOBALS['SQ_SYSTEM']->am->getAsset($current_lock['userid']);

		$can_aquire_lock = false;

		// lets work out if the current user has a high
		// enough level of access to forceably aquire the lock
		if (!$GLOBALS['SQ_SYSTEM']->userRoot($user)) {
			if ($GLOBALS['SQ_SYSTEM']->userSystemAdmin($user)) {
				// lock is held by a system admin, so need
				// to be root to aquire this lock
				if ($GLOBALS['SQ_SYSTEM']->userRoot()) $can_aquire_lock = true;
			} else {
				// need to be a system admin to aquire this lock
				if ($GLOBALS['SQ_SYSTEM']->userRoot()) $can_aquire_lock = true;
				else if ($GLOBALS['SQ_SYSTEM']->userSystemAdmin()) $can_aquire_lock = true;
			}
		}

		return $can_aquire_lock;
	}//end canForceablyAquireLock()


	/**
	* Aquires a lock on this asset and its dependant assets
	*
	* @param int	$source		the id of the asset that started this locking process
	* @param string	$expires	when the lock expires
	*
	* @return boolean
	* @access public
	*/
	function aquireLock($source=0, $expires='')
	{
		$current_lock = $this->getLockInfo();

		if (!empty($current_lock) && $current_lock['userid'] == $GLOBALS['SQ_SYSTEM']->currentUserid()) {
			// the user is asking to aquire a lock they already had
			// so just update the lock expiry date
			$this->updateLock($expires);
			return true;
		}

		$db = &$GLOBALS['SQ_SYSTEM']->db;

		if($this->canForceablyAquireLock()) {
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
			$current_user = &$GLOBALS['SQ_SYSTEM']->am->getAsset($GLOBALS['SQ_SYSTEM']->currentUserid());
			$user = &$GLOBALS['SQ_SYSTEM']->am->getAsset($current_lock['userid']);

			foreach ($locked_ids as $locked) {
				$locked_asset = &$GLOBALS['SQ_SYSTEM']->am->getAsset($locked);
				$body = 'User "'.$current_user->name.'" has forceably aquired the lock on '.$locked_asset->type().' "'.$locked_asset->name.'", formally held by user "'.$user->name.'".';
				$to_list = array_keys($locked_asset->getPermission(SQ_PERMISSION_ADMIN));
				$to_list[] = $user->id;
				$ms->sendMessage($to_list, 0, 'Asset Lock Forceably Aquired', $body);
			}

			// refresh the lock info
			$current_lock = $this->getLockInfo();
		}
		
		// is this asset already locked
		if (!empty($current_lock)) {
			$user = &$GLOBALS['SQ_SYSTEM']->am->getAsset($current_lock['userid']);
			trigger_error('Can not aquire lock on "'.$this->name.'", lock already held by "'.$user->name.'"', E_USER_WARNING);
			return false;
		}

		// if we have no source, we are the asset aquiring the locks
		if (!$source) $source = $this->id;
		if (empty($expires)) $expires = ts_iso8601((time() + (5 * 60)));

		$GLOBALS['SQ_SYSTEM']->doTransaction('BEGIN');

		// try and aquire locks on our dependants
		$dependant_links = $this->getLinks(SQ_LINK_SIGNIFICANT, '', true, 'major', null, 1);
		if (!empty($dependant_links)) {
			$am = &$GLOBALS['SQ_SYSTEM']->am;
			foreach ($dependant_links as $link) {
				$asset = &$am->getAsset($link['minorid']);
				if (!$asset->aquireLock($source, $expires)) {
					$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
				}
			}
		}

		// now try and get the lock on ourself
		$sql = 'INSERT INTO sq_asset_lock
				(assetid, source_asset, userid, expires)
				VALUES
				('.$db->quote($this->id).', '.$db->quote($source).', 
				 '.$db->quote($GLOBALS['SQ_SYSTEM']->currentUserid()).', 
				 '.$db->quote($expires).')';

		$result = $db->query($sql);
		if (DB::isError($result)) {
			$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
			trigger_error($result->getMessage().'<br/>'.$result->getUserInfo(), E_USER_WARNING);
			return false;
		}

		$GLOBALS['SQ_SYSTEM']->doTransaction('COMMIT');
		return true;
	}//end aquireLock();


	/**
	* Release the locks that this asset holds
	*
	* @return boolean
	* @access public
	*/
	function releaseLock()
	{
		$current_lock = $this->getLockInfo(false);
		if (empty($current_lock)) return true;

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
	}//end aquireLock();


	/**
	* Updates the expiry date of the lock on this asset
	*
	* @param string	$expires	when the lock expires
	*
	* @return boolean
	* @access public
	*/
	function updateLock($expires='')
	{
		$current_lock = $this->getLockInfo();
		if (empty($current_lock)) return true;

		if (empty($expires)) $expires = ts_iso8601((time() + (5 * 60)));

		$GLOBALS['SQ_SYSTEM']->doTransaction('BEGIN');

		$db = &$GLOBALS['SQ_SYSTEM']->db;
		$sql = 'UPDATE sq_asset_lock
				SET expires = '.$db->quote($expires).'
				WHERE source_asset = '.$db->quote($current_lock['source_asset']);

		$result = $db->query($sql);
		if (DB::isError($result)) {
			$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
			trigger_error($result->getMessage().'<br/>'.$result->getUserInfo(), E_USER_WARNING);
			return false;
		}

		$GLOBALS['SQ_SYSTEM']->doTransaction('COMMIT');
		return true;
	}//end updateLock();


	/**
	* Returns an array of information about the current lock on this asset (if any)
	*
	* @param boolean	$check_expires	check the expiry date and cleanup the lock if it has expired
	*
	* @return array
	* @access public
	*/
	function getLockInfo($check_expires=true)
	{
		$db = &$GLOBALS['SQ_SYSTEM']->db;

		// just get the permissions set for this asset
		$sql = 'SELECT * FROM sq_asset_lock
				WHERE assetid = '.$db->quote($this->id);

		$result = $db->getRow($sql);
		if (DB::isError($result)) {
			trigger_error($result->getMessage().'<br/>'.$result->getUserInfo(), E_USER_WARNING);
			return Array();
		}

		if ($check_expires) {
			if (!empty($result)) {
				$result['expires'] = iso8601_ts($result['expires']);
				if ($result['expires'] < time()) {
					// this lock has expired
					if ($this->cleanupLocks()) return Array();
				}
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
		$current_lock = $this->getLockInfo(false);
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
	* Returns the link information for a specific link that this asset is the major asset in
	*
	* @param int	$linkid
	* @param string	$side_of_link	Which side of the link this (the current) asset is on ('major' or 'minor')
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
		$sql = 'SELECT l.linkid, l.'.$other_side.'id, l.value, l.link_type, a.type_code, l.sort_order, l.dependant
				FROM sq_asset_link l, sq_asset a
				WHERE l.linkid = '.$db->quote($linkid).'
				  AND l.'.$other_side.'id   = a.assetid';

		if (!$ignore_this) $sql .= ' AND l.'.$side_of_link.'id = '.$db->quote($this->id);

		$result = $db->getRow($sql);
		if (DB::isError($result))
			trigger_error($result->getMessage().'<br/>'.$result->getUserInfo(), E_USER_ERROR);

		return $result;

	}//end getLinkById()


	/**
	* Returns information for a link that this asset is the major asset in
	*
	* @param int			$link_type		integer that should be a single integer of the SQ_LINK_* constants
	* @param string|array	$type_code		the type of asset that is linked (eg 'Page', 'File', etc) 
	*										if an array returns link if matches any of the array values
	* @param bool			$strict			whether we are finding an asset that are just a $type_code 
	*										or $type_code and any of it's sub-classes
	* @param string			$value			the value that is associated with this link
	* @param string			$side_of_link	Which side of the link this (the current) asset is on ('major' or 'minor')
	*
	* @return Array()
	* @access public
	*/
	function getLink($link_type, $type_code='', $strict_type_code=true, $value=null, $side_of_link='major')
	{
		if ($side_of_link != 'major' && $side_of_link != 'minor')
			trigger_error('Unknown Side of Link "'.$side_of_link.'"', E_USER_ERROR);

		if (!$this->id) return Array();

		$other_side = ($side_of_link == 'major') ? 'minor' : 'major';

		$db = &$GLOBALS['SQ_SYSTEM']->db;
		$sql = 'SELECT l.linkid, l.'.$other_side.'id, l.value, l.link_type, a.type_code, l.sort_order, l.dependant
				FROM sq_asset_link l, sq_asset a
				WHERE l.'.$side_of_link.'id = '.$db->quote($this->id).'
				  AND l.'.$other_side.'id   = a.assetid
				  AND l.link_type = '.$db->quote($link_type);

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
				$sql .= ' AND (a.type_code '.$type_code_cond.' OR a.type_code IN (~SQ0~))';
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
	* Return all links that this asset has to other assets
	*
	* @param int			$link_types			integer that can be the product of bitwise operations 
	*											on the SQ_LINK_* constants
	* @param string|array	$type_code			the type of asset that is linked (eg 'Page', 'File', etc) 
	*											if an array returns link if matches any of the array values
	* @param bool			$strict_type_code	whether we are finding assets that are just a $type_code 
	*											or $type_code and any of it's sub-classes
	* @param string			$side_of_link		Which side of the link this (the current) asset is on ('major' or 'minor')
	* @param string			$value				The value for all the links must equal this (if not null)
	* @param boolean		$dependant			The dependant status for all the links must be this (if not null)
	*
	* @return Array()
	* @access public
	*/
	function getLinks($link_types, $type_code='', $strict_type_code=true, $side_of_link='major', $value=null, $dependant=null)
	{
		if ($side_of_link != 'major' && $side_of_link != 'minor')
			trigger_error('Unknown Side of Link "'.$side_of_link.'"', E_USER_ERROR);

		if (!$this->id) return Array();

		$other_side = ($side_of_link == 'major') ? 'minor' : 'major';

		$db = &$GLOBALS['SQ_SYSTEM']->db;
		$sql = 'SELECT l.linkid, l.'.$other_side.'id, l.value, l.link_type, a.type_code, l.sort_order, l.dependant
				FROM sq_asset_link l, sq_asset a
				WHERE l.'.$other_side.'id   = a.assetid
				  AND l.'.$side_of_link.'id = '.$db->quote($this->id).'
				  AND (l.link_type & '.$db->quote($link_types).') > 0 ';

		if (!is_null($value))		$sql .= '  AND l.value     = '.$db->quote($value);
		if (!is_null($dependant))	$sql .= '  AND l.dependant = '.$db->quote(($dependant) ? '1' : '0');

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
				$sql .= ' AND (a.type_code '.$type_code_cond.' OR a.type_code IN (~SQ0~))';
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
	* Returns the number of links that this asset has as either a major or minor party,
	* can be restricted by either link type and/or asset type code
	*
	* @param string			$side_of_link		Which side of the link this (the current) asset is on ('major' or 'minor')
	* @param int			$link_types			integer that can be the product of bitwise operations 
	*											on the SQ_LINK_* constants
	* @param string|array	$type_code			the type of asset that is linked (eg 'Page', 'File', etc) 
	*											if an array returns link if matches any of the array values
	* @param bool			$strict_type_code	whether we are finding assets that are just a $type_code 
	*											or $type_code and any of it's sub-classes
	* @param int			$ignore_linkid		ignore the link represented by this link id when returning the count
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
				$extra_where .= ' AND (a.type_code '.$type_code_cond.' OR a.type_code IN (~SQ0~))';
				$subs = Array( 'SELECT type_code
								FROM sq_asset_type_inherited
								WHERE inherited_type_code '.$type_code_cond );
				$extra_where = db_extras_subquery($db, $extra_where, $subs);
				if (DB::isError($sql))
					trigger_error($sql->getMessage().'<br/>'.$sql->getUserInfo(), E_USER_ERROR);
			}// end if
		}// end if
		if ($ignore_linkid) {
			$extra_where .= ' AND l.linkid != '.$db->quote($ignore_linkid);
		}

		$sql = 'SELECT COUNT(*)
				FROM sq_asset_link l'.$extra_table.'
				WHERE l.'.$side_of_link.'id   = '.$db->quote($this->id).$extra_where;
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
	* @return TRUE|string
	* @access public
	*/
	function canCreateLink(&$minor, $link_type)
	{
		if (!$this->id) return false;

		// if you dont have write access to this asset you cant create any links
		if (!$GLOBALS['SQ_INSTALL'] && !$this->writeAccess()) {
			return 'Cannot create link from '.$this->name.' (# '.$this->id.') to '.$minor->name.' (# '.$minor->id.'), permission denied';
		}

		require_once SQ_FUDGE_PATH.'/db_extras/db_extras.inc';

		if (!is_a($minor, 'Asset')) {
			return 'Minor is not an asset';
		}

		// check if we are allowed to link to these type of assets
		if (($err_msg = $this->canLinkToType($minor->type(), $link_type)) !== true) {
			return $err_msg;
		}

		$db = &$GLOBALS['SQ_SYSTEM']->db;

		// if this link is a significant link, then we need to make sure that this
		// minor asset doesn't already have an exclusive link from anything
		// and that we aren't moving under ourselves
		if ($link_type & SQ_LINK_SIGNIFICANT) {
			//// Check for exclusive ////
			$sql = 'SELECT majorid
					FROM sq_asset_link
					WHERE minorid   = '.$db->quote($minor->id).'
					  AND link_type = '.$db->quote(SQ_LINK_EXCLUSIVE);

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
	*
	* @return int
	* @access public
	*/
	function createLink(&$minor, $link_type, $value='', $sort_order=null, $dependant='0')
	{

		if (!$this->id) return 0;
		require_once SQ_FUDGE_PATH.'/db_extras/db_extras.inc';

		if (!is_a($minor, 'Asset')) {
			trigger_error('Minor is not an asset, unable to create link', E_USER_WARNING);
			return 0;
		}

		if (!($link_type & SQ_LINK_SIGNIFICANT) && $dependant) {
			trigger_error('In order for a link to be dependant it must also be a significant link', E_USER_WARNING);
			return 0;
		}

		// First, we should check that we don't already have a link of this type
		$current_links = $this->getLinks($link_type, $minor->type());
		$num_curr_links = count($current_links);
		for($i = 0; $i < $num_curr_links; $i++) {
			// it already exists, so we don't need to do anything
			if ($current_links[$i]['minorid'] == $minor->id && $current_links[$i]['value'] == $value) {
				trigger_error('Link Already Exists', E_USER_WARNING);
				return $current_links[$i]['linkid'];
			}
		}//end for


		// check if we are allowed to link to these type of assets
		if (($err_msg = $this->canCreateLink($minor, $link_type)) !== true) {
			trigger_error($err_msg, E_USER_WARNING);
			return 0;
		}

		$GLOBALS['SQ_SYSTEM']->doTransaction('begin');
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

#			pre_echo("Existing Tree Id : ".$existing_treeid);

			// OK, what we are going to get a treeid and then do a "INSERT INTO ... SELECT FROM"
			// into the tree table of all the links that are under the minor asset
			$sql = 'SELECT t.treeid
					FROM sq_asset_link_tree t INNER JOIN sq_asset_link l ON t.linkid = l.linkid
					WHERE l.minorid = '.$db->quote($minor->id).'
					LIMIT 1';
			$minor_treeid = $db->getOne($sql);
			if (DB::isError($minor_treeid)) {
				trigger_error($minor_treeid->getMessage().'<br/>'.$minor_treeid->getUserInfo(), E_USER_WARNING);
				$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
				return 0;
			}
#			pre_echo("Minor    Tree Id : ".$minor_treeid);

			$existing_treeid = (string) $existing_treeid;
			$minor_treeid    = (string) $minor_treeid;

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
#					pre_echo("FREE CHILDID : ".$free_childid);
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

				// because the root folder is not a minor party in any links we have to something special for it
				if ($this->id == 1) {
					$sql = 'DELETE FROM sq_asset_link_tree
							WHERE treeid LIKE '.$db->quote($free_childid);
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
						return 0;
					}
				}
#pre_echo($sql);
				$result = $db->query($sql);
				if (DB::isError($result)) {
					trigger_error($result->getMessage().'<br/>'.$result->getUserInfo(), E_USER_WARNING);
					$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
					return 0;
				}

			}// end if

#			pre_echo("Free Childid : ".$free_childid);

			$insert = 'INSERT INTO sq_asset_link_tree (treeid, linkid)';

			// if we don't have any existing tree entries then
			// we are the root folder, so do a simple insert
			if ((string) $existing_treeid == '') {
				$sql = $insert.' VALUES ('.$db->quote($free_childid).', '.$db->quote($linkid).')';
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
				$select = 'SELECT '.$concat.', '.$db->quote($linkid).'
							FROM sq_asset_link_tree t INNER JOIN sq_asset_link l ON t.linkid = l.linkid
							WHERE l.minorid = '.$db->quote($this->id);

				$result = db_extras_insert_select($db, $insert, $select);
				if (DB::isError($result)) {
					trigger_error($result->getMessage().'<br/>'.$result->getUserInfo(), E_USER_WARNING);
					$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
					return 0;
				}
			}// end if

			// if this minor has already been linked so do a select into
			if ((string) $minor_treeid != '') {

				// so much for SQL standards...
				$concat = ($db->phptype == 'mysql')
							? 'CONCAT(pt.treeid, '.$db->quote($free_childid).', SUBSTRING(ct.treeid FROM '.$db->quote(strlen($minor_treeid) + 1).'))'
							: 'pt.treeid || '.$db->quote($free_childid).' || SUBSTRING(ct.treeid FROM '.$db->quote(strlen($minor_treeid) + 1).')';

				// the 'ct.treeid > ' line makes sure that we get only the children of the minor,
				// because we added the tree link to the minor above
				$select = 'SELECT '.$concat.', ct.linkid
							FROM sq_asset_link_tree pt INNER JOIN sq_asset_link pl ON pt.linkid = pl.linkid, sq_asset_link_tree ct
							WHERE pl.minorid = '.$db->quote($this->id).'
							  AND ct.treeid LIKE '.$db->quote($minor_treeid.'%').'
							  AND ct.treeid > '.$db->quote($minor_treeid);

#pre_echo($select);

				$result = db_extras_insert_select($db, $insert, $select);
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
				(linkid, majorid, minorid, link_type, value, sort_order, dependant, last_updated, last_userid)
				VALUES
				('.$db->quote($linkid).', '.$db->quote($this->id).', '.$db->quote($minor->id).', '.$db->quote($link_type).', '
				.$db->quote($value).', '.$db->quote($sort_order).', '.$db->quote(($dependant) ? '1' : '0').', '.$db->quote(ts_iso8601(time())).', 
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

		$GLOBALS['SQ_SYSTEM']->doTransaction('COMMIT');
		$GLOBALS['SQ_SYSTEM']->broadcastEvent('CreateLink', Array('assetid' => $this->id, 'linkid' => $linkid));

		return $linkid;

	}//end createLink()


	/**
	* Checks whether a link of a certain type can be made to this asset
	* Returns TRUE if link allowed and a string containing the error if it is not
	*
	* @param string $type_code		the type code that we want to check
	* @param string $link_type		the type of link to check
	* @param int	$ignore_linkid	if it gets down to the checking of the cardinality,
	*								ignore the link represented by this link id
	*
	* @return TRUE | string
	* @access public
	*/
	function canLinkToType($type_code, $link_type, $ignore_linkid=0)
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

		//// Now we need to check that the cardinality is kept valid ////

		// if we are only allowed one of these links
		if ($allowed_links[$link_type][$type] == 1) {
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
	* Returns an array of all the permitted links type, the type asset and the cardinality
	* In the form
	*
	*    Array('[link_type]' => Array('[type_code]' => '[cardinality]));
	* Where:
	*   link_type   = SQ_LINK_EXCLUSIVE|SQ_LINK_UNITE|SQ_LINK_USES|SQ_LINK_NOTICE
	*   cardinality = 1|M
	*
	* @return Array()
	* @access private
	*/
	function _getAllowedLinks()
	{

		return Array(SQ_LINK_EXCLUSIVE => Array(),
					 SQ_LINK_UNITE     => Array(),
					 SQ_LINK_USES      => Array(),
					 SQ_LINK_NOTICE    => Array()
					);

	}//end _getAllowedLinks()


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
		if ($this->_updateLink($linkid, null, null, null, $sort_order) && $this->_linksUpdated()) {
			$GLOBALS['SQ_SYSTEM']->doTransaction('COMMIT');
			return true;
		} else {
			$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
			return false;
		}

	}//end moveLinkPos()


	/**
	* Updates the details of an existing link
	* If any of the detail vars are NULL they aren't updated
	* NOTE: No transaction stuff is put around the SQL calls in the fn, that sould be done externally
	*
	* @param int	$linkid		the link id of the link to update
	* @param int	$minorid
	* @param int	$link_type
	* @param string $value
	* @param int	$sort_order	the position in the links list that this link should take, 
	*							if less than zero places at end of list
	*
	* @return boolean
	* @access public
	*/
	function _updateLink($linkid, $minorid=null, $link_type=null, $value=null, $sort_order=null)
	{
		// First, we should try and find the link
		$link = $this->getLinkById($linkid);
		if (empty($link)) {
			trigger_error('Existing Link #'.$linkid.' not found, unable to update link', E_USER_WARNING);
			return false;
		}
#		pre_echo("SORT ORDER : ".$sort_order);
#		pre_echo(array_contents($link));

		$db = &$GLOBALS['SQ_SYSTEM']->db;

		$set_clauses = Array();

		$minorid_changed	= (!is_null($minorid)		&& $link['minorid']		!= $minorid		);
		$link_type_changed	= (!is_null($link_type)		&& $link['link_type']	!= $link_type	);
		$value_changed		= (!is_null($value)			&& $link['value']		!= $value		);
		$sort_order_changed	= (!is_null($sort_order)	&& $link['sort_order']	!= $sort_order	);

		$minorid    = (int) $minorid;
		$link_type  = (int) $link_type;
		$sort_order = (int) $sort_order;

		if ($minorid_changed || $link_type_changed) {
			$minorid   = ($minorid_changed)   ? $minorid   : (int) $link['minorid'];
			$link_type = ($link_type_changed) ? $link_type : (int) $link['link_type'];

			$minor = &$GLOBALS['SQ_SYSTEM']->am->getAsset($minorid);
			if (($err_msg = $this->canLinkToType($minor->type(), $link_type)) !== true) {
				trigger_error('Unable to update link #'.$linkid.' ('.$err_msg.')', E_USER_WARNING);
				return false;
			}
			if ($minorid_changed)   $set_clauses[] = 'minorid   = '.$db->quote($minorid);
			if ($link_type_changed) $set_clauses[] = 'link_type = '.$db->quote($link_type);

		}

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

		if ($sort_order_changed) {
			// move 'em up, higher
			if ($link['sort_order'] > $sort_order) {
				$sql = 'UPDATE sq_asset_link
						SET sort_order = sort_order + 1
						WHERE majorid = '.$db->quote($this->id).'
						  AND linkid != '.$db->quote($linkid).'
						  AND sort_order >= '.$db->quote($sort_order).'
						  AND sort_order <= '.$db->quote($link['sort_order']);

			} else {
				$sql = 'UPDATE sq_asset_link
						SET sort_order = sort_order - 1
						WHERE majorid = '.$db->quote($this->id).'
						  AND linkid != '.$db->quote($linkid).'
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
	* Removes a link where this asset is the major player
	*
	* @param int	$linkid	the link id of the link to remove
	*
	* @return boolean
	* @access public
	*/
	function deleteLink($linkid)
	{

		// First, we should try and find the link
		$link = $this->getLinkById($linkid);
		if (empty($link)) {
			trigger_error('Existing Link #'.$linkid.' not found, unable to delete link', E_USER_WARNING);
			return false;
		}

		// if you dont have write access to this asset you cant delete any links
		if (!$this->writeAccess()) {
			trigger_error('Cannot delete link, permission denied', E_USER_WARNING);
			return false;
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

			// OK, if we are going to be deleting the last significant link
			// then we need to create a link to the trash for this
			if (!$num_other_links) {
				$trash_folder = &$GLOBALS['SQ_SYSTEM']->am->getSystemAsset('trash_folder');
				if (is_null($minor)) {
					$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
					return false;
				}

				// some assets may not be able to have their last significant link
				// deleted, so lets check first before going ahead
				if (!$minor->canDelete()) {
					trigger_error($minor->_getName(false).' can not have its last significant link removed' , E_USER_WARNING);
					$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
					return false;
				}

				// this asset is destined for the trash, so we need to set its permissions to
				// whatever it is inheriting right now so it keeps the while in the trash
				$perms = Array(SQ_PERMISSION_ADMIN, SQ_PERMISSION_WRITE, SQ_PERMISSION_READ);
				foreach ($perms as $perm) {
					$userids = array_keys($minor->getPermission($perm, false));
					foreach($userids as $userid) $minor->grantPermission($userid, $perm);
				}

				if (!$trash_folder->createLink($minor, SQ_LINK_UNITE)) {
					$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
					return false;
				}
			}

			//// UPDATE THE TREE ////

			// Wwe can delete all the links under these nodes because it will be a clean start 
			// when we insert into the gap's we created above
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
#pre_echo($sql);
			$result = $db->query($sql);
			if (DB::isError($result)) {
				$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
				trigger_error($result->getMessage().'<br/>'.$result->getUserInfo(), E_USER_WARNING);
				return false;
			}

			// We are going to set the treeid nodes that this link is associated with to zero so that we can
			// find it as a gap when we createLink() later on
			$sql = 'UPDATE sq_asset_link_tree
					   SET linkid = 0
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

#		error_log($sql);

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
		$GLOBALS['SQ_SYSTEM']->doTransaction('COMMIT');
		$GLOBALS['SQ_SYSTEM']->broadcastEvent('DeleteLink', Array('assetid' => $this->id, 'linkid' => $linkid));

		return true;

	}//end deleteLink()


	/**
	* Set the value for the passed variable to the passed value
	*
	* @param string		$name				the name of the attribute
	* @param string		$value				the new value of the attribute
	* @param boolean	$dont_run_updated	if true this->_updated() won't be run (THIS IS ONLY EVER CALLED FROM Asset::create())
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
			trigger_error("Value of attribute '$name' not set in setAttrValue()", E_USER_WARNING);
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
	* @param string	$name
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
	}//end attr();


	/**
	* Returns a reference to the actual attribute object, used for manipulting the object
	*
	* @param string		$name			the name of the attribute
	* @param boolean	$mute_errors	stops the outputting of errors in this fn, needed
	*									because you can't use the '@' operator when returning by reference
	*
	* @return object Asset_Attribute | NULL
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
	* Grants a permission for the passed user or user_group
	*
	* @param int	$assetid	the id of the user or user_group to add the permission for
	* @param string	$permission	the permission code you are adding
	*
	* @return boolean
	* @access public
	*/
	function grantPermission($assetid, $permission)
	{

		$assetid = (int) $assetid;
		$current = &$this->getPermission($permission, false, false);
		if (in_array($assetid, array_keys($current))) return false;

		// if we dont have an assetid, we are granting public access
		if (!empty($assetid)) {
			// check that the passed assetid is a user or user_group
			$asset = &$GLOBALS['SQ_SYSTEM']->am->getAsset($assetid, '', true);
			if (!$asset->id) {
				trigger_error('Cannot grant permission for an asset that does not exist', E_USER_WARNING);
				return false;
			} else if (!is_a($asset, 'user') && !is_a($asset, 'user_group')) {
				trigger_error('Cannot grant permission for asset type "'.$asset->type().'", not a user or user group', E_USER_WARNING);
				return false;
			}
		}

		$db = &$GLOBALS['SQ_SYSTEM']->db;
		$GLOBALS['SQ_SYSTEM']->doTransaction('BEGIN');

		// add the permission into the database
		$sql = 'INSERT INTO sq_asset_permission
				(assetid, userid, permission)
				VALUES
				('.$db->quote($this->id).','.$db->quote($assetid).', '.$db->quote($permission).')';

		$result = $db->query($sql);
		if (DB::isError($result)) {
			trigger_error($result->getMessage().'<br/>'.$result->getUserInfo(), E_USER_WARNING);
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

	}//end grantPermission()


	/**
	* Revokes a permission from the passed user or user_group
	*
	* @param int	$assetid	the id of the user or user_group to remove the permission from
	* @param string	$permission	the permission code you are adding
	*
	* @return boolean
	* @access public
	*/
	function revokePermission($assetid, $permission)
	{

		// make sure the permission is set before trying to delete
		$current = &$this->getPermission($permission, false, false);
		if (!in_array($assetid, array_keys($current))) return false;
		
		// if we dont have an assetid, we are revoking public access
		if (!empty($assetid)) {
			// check that the passed assetid is a user or user_group
			$asset = &$GLOBALS['SQ_SYSTEM']->am->getAsset($assetid);
			if (!is_a($asset, 'user') && !is_a($asset, 'user_group')) {
				trigger_error('Cannot revoke permission for asset type "'.$asset->type().'", not a user or user group', E_USER_WARNING);
				return false;
			}
		}

		$db = &$GLOBALS['SQ_SYSTEM']->db;

		$GLOBALS['SQ_SYSTEM']->doTransaction('BEGIN');

		// add the permission into the database
		$sql = 'DELETE FROM sq_asset_permission
				WHERE assetid = '.$db->quote($this->id).'
				  AND userid = '.$db->quote($assetid).'
				  AND permission = '.$db->quote($permission).'
				  AND revoked = 0';

		$result = $db->query($sql);
		if (DB::isError($result)) {
			trigger_error($result->getMessage().'<br/>'.$result->getUserInfo(), E_USER_WARNING);
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

	}//end revokePermission()


	/**
	* Revokes an inherited permission
	*
	* @param int	$assetid	the id of the user or user_group to stop inheriting the permission for
	* @param string	$permission	the permission code you are stopping
	*
	* @return boolean
	* @access public
	*/
	function revokeInheritedPermission($assetid, $permission)
	{

		$assetid = (int) $assetid;
		$current = $this->getRevokedPermission($permission);
		if (in_array($assetid, array_keys($current))) return false;

		// if we dont have an assetid, we are revoking public access
		if (!empty($assetid)) {
			// check that the passed assetid is a user or user_group
			$asset = &$GLOBALS['SQ_SYSTEM']->am->getAsset($assetid, '', true);
			if ($permission != SQ_PERMISSION_WORKFLOW) {
				if (!$asset->id) {
					trigger_error('Cannot revoke inherited permission for an asset that does not exist', E_USER_WARNING);
					return false;
				} else if (!is_a($asset, 'user') && !is_a($asset, 'user_group')) {
					trigger_error('Cannot revoke inherited permission for asset type "'.$asset->type().'", not a user or user group', E_USER_WARNING);
					return false;
				}
			}
		}

		$db = &$GLOBALS['SQ_SYSTEM']->db;
		$GLOBALS['SQ_SYSTEM']->doTransaction('BEGIN');

		// add the permission into the database
		$sql = 'INSERT INTO sq_asset_permission
				(assetid, userid, permission, revoked)
				VALUES
				('.$db->quote($this->id).','.$db->quote($assetid).', '.$db->quote($permission).', 1)';

		$result = $db->query($sql);
		if (DB::isError($result)) {
			trigger_error($result->getMessage().'<br/>'.$result->getUserInfo(), E_USER_WARNING);
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

	}//end revokeInheritedPermission()


	/**
	* Reinstates a revoked permission
	*
	* @param int	$assetid	the id of the user or user_group to reinstate the permission for
	* @param string	$permission	the permission code you are reinstating
	*
	* @return boolean
	* @access public
	*/
	function reinstateRevokedPermission($assetid, $permission)
	{

		$assetid = (int) $assetid;
		$current = $this->getRevokedPermission($permission);
		if (!in_array($assetid, array_keys($current))) return false;

		if ($permission != SQ_PERMISSION_WORKFLOW) {
			// if we dont have an assetid, we are reinstating public access
			if (!empty($assetid)) {
				// check that the passed assetid is a user or user_group
				$asset = &$GLOBALS['SQ_SYSTEM']->am->getAsset($assetid, '', true);
				if (!$asset->id) {
					trigger_error('Cannot reinstate revoked permission for an asset that does not exist', E_USER_WARNING);
					return false;
				} else if (!is_a($asset, 'user') && !is_a($asset, 'user_group')) {
					trigger_error('Cannot reinstate revoked permission for asset type "'.$asset->type().'", not a user or user group', E_USER_WARNING);
					return false;
				}
			}
		}

		$db = &$GLOBALS['SQ_SYSTEM']->db;
		$GLOBALS['SQ_SYSTEM']->doTransaction('BEGIN');

		// reomve the permission from the database
		$sql = 'DELETE FROM sq_asset_permission
				WHERE assetid = '.$db->quote($this->id).'
				  AND userid = '.$db->quote($assetid).'
				  AND permission = '.$db->quote($permission).'
				  AND revoked = 1';

		$result = $db->query($sql);
		if (DB::isError($result)) {
			trigger_error($result->getMessage().'<br/>'.$result->getUserInfo(), E_USER_WARNING);
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
	}//end reinstateRevokedPermission()


	/**
	* Returns an array of all user or user_group assets that have the passed permission for this asset
	*
	* @param string		$permission		the permission code you are getting
	* @param boolean	$and_greater	get effective permission (eg read access = read or write or admin)
	* @param boolean	$and_inherited	get permission from parents as well
	* @param boolean	$expand_groups	expand user groups so only user ids get returned
	*
	* @return array(int)
	* @access public
	*/
	function getPermission($permission, $and_greater=true, $and_inherited=true, $expand_groups=false)
	{
		$permission = (int) $permission;

		// return cached version if we can
		$storage_name =(($and_greater) ? 'effective_' : '').'permissions'.(($and_inherited) ? '_inherited' : '');
		if (isset($this->_tmp['permission_cache'][$storage_name][$permission])) {
			$result = $this->_tmp['permission_cache'][$storage_name][$permission];
		} else {

			// work out the max_limit if needed
			// we will need this if checking permission below workflow
			// so we dont interperate workflow as admin access
			$max_limit = 0;
			if ($permission < SQ_PERMISSION_WORKFLOW) $max_limit = SQ_PERMISSION_WORKFLOW;

			$db = &$GLOBALS['SQ_SYSTEM']->db;

			// get the permissions
			if ($and_inherited) {
				// getting all permissions taking note of revoked permissions
				// anywhere up the trees this asset is in
				$concat = ($db->phptype == 'mysql') ? 'CONCAT(pt.treeid, '.$db->quote('%').')' : 'pt.treeid || '.$db->quote('%');
				$concat_userid = ($db->phptype == 'mysql') 
						? 'CONCAT(p.userid, '.$db->quote('~').', p.permission, '.$db->quote('~').', t.treeid)' 
						: 'p.userid || '.$db->quote('~').' || p.permission || '.$db->quote('~').' || t.treeid';
				$concat_userid_sub = ($db->phptype == 'mysql') 
						? 'CONCAT(pp.userid, '.$db->quote('~').', pp.permission, '.$db->quote('~').', MAX(pt.treeid))' 
						: 'pp.userid || '.$db->quote('~').' || pp.permission || '.$db->quote('~').' || MAX(pt.treeid)';

				$sql = 'SELECT DISTINCT p.userid, p.assetid
						FROM sq_asset_link l
						  INNER JOIN sq_asset_link_tree t ON l.linkid = t.linkid 
						  INNER JOIN sq_asset_permission p ON l.minorid = p.assetid
						WHERE '.$concat_userid.' IN (~SQ0~)
						  AND p.permission '.(($and_greater) ? '>=' : '=').' '.$db->quote($permission).'
						GROUP BY p.userid, p.assetid
						HAVING MIN(p.revoked) = 0';

				$subs = Array( 'SELECT DISTINCT '.$concat_userid_sub.'
								FROM sq_asset_link cl
								  INNER JOIN sq_asset_link_tree ct ON cl.linkid = ct.linkid,
								  sq_asset_link pl INNER JOIN sq_asset_link_tree pt ON pl.linkid = pt.linkid 
								  INNER JOIN sq_asset_permission pp ON pl.minorid = pp.assetid
								WHERE cl.minorid = '.$db->quote($this->id).'
								  AND ct.treeid LIKE '.$concat.'
								  AND pt.treeid <= ct.treeid
								  AND pp.permission '.(($and_greater) ? '>=' : '=').' '.$db->quote($permission).' '
								.(($max_limit) ? 'AND pp.permission < '.$db->quote($max_limit) : '').'
								GROUP BY ct.treeid, pp.userid, pp.permission');

				require_once SQ_FUDGE_PATH.'/db_extras/db_extras.inc';
				$sql = db_extras_subquery($db, $sql, $subs);
				if (DB::isError($sql)) {
					trigger_error($sql->getMessage().'<br/>'.$sql->getUserInfo(), E_USER_WARNING);
					return Array();
				}

			} else {
				// just get the permissions set for this asset
				$sql = 'SELECT userid, assetid
						FROM sq_asset_permission
						WHERE assetid = '.$db->quote($this->id).'
						  AND permission '.(($and_greater) ? '>=' : '=').' '.$db->quote($permission).'
						  AND revoked = 0';
			}

			$result = $db->getAssoc($sql, false, Array(), DB_FETCHMODE_DEFAULT, true);
			if (DB::isError($result)) {
				trigger_error($result->getMessage().'<br/>'.$result->getUserInfo(), E_USER_WARNING);
				return Array();
			}

			// cache the result for next time
			$this->_tmp['permission_cache'][$storage_name][$permission] = $result;
		}

		if ($expand_groups) {
			$return_result = Array();
			foreach ($result as $assetid => $locs) {
				$asset = &$GLOBALS['SQ_SYSTEM']->am->getAsset($assetid);
				if ($asset->id && is_a($asset, 'user_group')) {
					$return_result = array_merge($return_result, $asset->getChildren(Array('user'), false));
				} else {
					$return_result[] = $assetid;
				}
			}
			$result = array_unique($return_result);
		}

		return $result;
	}//end getPermission()


	/**
	* Returns an array of all user or user_group assets that have the passed permission for this asset
	*
	* @param string		$permission		the permission code you are getting
	* @param boolean	$and_greater	get effective permission (eg read access = read or write or admin)
	* @param boolean	$and_inherited	get permission from parents as well
	*
	* @return array(int)
	* @access public
	*/
	function getInheritedPermission($permission, $userids)
	{
		if (empty($userids)) return Array();

		$permission = (int) $permission;

		// return cached version if we can
		if (isset($this->_tmp['permission_cache']['full_permissions'][$permission])) {
			return $this->_tmp['permission_cache']['full_permissions'][$permission];
		}

		$db = &$GLOBALS['SQ_SYSTEM']->db;

		// get all the revoke points where inheritance stops
		$concat = ($db->phptype == 'mysql') ? 'CONCAT(pt.treeid, '.$db->quote('%').')' : 'pt.treeid || '.$db->quote('%');
		$userid_string = '';
		foreach ($userids as $u) $userid_string .= $db->quote($u).', ';
		$userid_string = trim($userid_string, ', ');
		$sql = 'SELECT pp.userid AS userid, ct.treeid AS child, MAX(pt.treeid) AS parent
				FROM sq_asset_link cl
				  INNER JOIN sq_asset_link_tree ct ON cl.linkid = ct.linkid,
				  sq_asset_link pl INNER JOIN sq_asset_link_tree pt ON pl.linkid = pt.linkid 
				  INNER JOIN sq_asset_permission pp ON pl.minorid = pp.assetid
				WHERE cl.minorid = '.$db->quote($this->id).'
				  AND ct.treeid LIKE '.$concat.'
				  AND pt.treeid <= ct.treeid
				  AND pp.permission = '.$db->quote($permission).'
				  AND pp.revoked = 1
				  AND pp.userid IN ('.$userid_string.')
				GROUP BY ct.treeid, pp.assetid, pp.userid';

		$result = $db->getAssoc($sql, false, Array(), DB_FETCHMODE_DEFAULT, true);
		if (DB::isError($result)) {
			trigger_error($result->getMessage().'<br/>'.$result->getUserInfo(), E_USER_WARNING);
			return Array();
		}

		// this is going to be the mega query to get all permissions
		$major_query = '';
		$major_query_conds = Array();

		$no_revokes = array_diff($userids, array_keys($result));

		// if any user permissions are not revoked
		// they wont appear in the results of the first query,
		// so we need to get the treeids from this asset
		if (!empty($no_revokes)) {
			$sql = 'SELECT ct.treeid
					FROM sq_asset_link cl INNER JOIN sq_asset_link_tree ct ON cl.linkid = ct.linkid
					WHERE cl.minorid = '.$db->quote($this->id);
			
			$rev_result = $db->getCol($sql);
			if (DB::isError($rev_result)) {
				trigger_error($rev_result->getMessage().'<br/>'.$rev_result->getUserInfo(), E_USER_WARNING);
				return Array();
			}

			foreach ($no_revokes as $u) {
				foreach ($rev_result as $t) {
					$major_query_conds[] = '( ct.treeid = '.$db->quote($t).' 
											  AND ct.treeid LIKE '.$concat.' 
											  AND pt.treeid < ct.treeid 
											  AND pp.userid = '.$db->quote($u).')';
				}
			}
		}

		// now add the conditions for revoked permissions (if any)
		if (!empty($result)) {
			foreach ($result as $u => $u_data) {
				foreach ($u_data as $i => $t_data) {
					$major_query_conds[] = '( ct.treeid = '.$db->quote($t_data[0]).' 
											  AND ct.treeid LIKE '.$concat.' 
											  AND pt.treeid >= '.$db->quote($t_data[1]).' 
											  AND pp.userid = '.$db->quote($u).')';
				}
			}
		}

		// generate the MEGA query
		$major_query = 'SELECT DISTINCT pp.userid, pp.assetid
						FROM sq_asset_link cl
						  INNER JOIN sq_asset_link_tree ct ON cl.linkid = ct.linkid,
						  sq_asset_link pl INNER JOIN sq_asset_link_tree pt ON pl.linkid = pt.linkid
						  INNER JOIN sq_asset_permission pp ON pl.minorid = pp.assetid
						WHERE pp.permission = '.$db->quote($permission).'
						  AND pp.revoked = 0
						  AND 
						(';

		$or_query = '';
		$or_query = implode("\n  OR\n  ", $major_query_conds);

		$major_query = $major_query."\n".$or_query."\n)";

		$major_result = $db->getAssoc($major_query, false, Array(), DB_FETCHMODE_DEFAULT, true);
		if (DB::isError($major_result)) {
			trigger_error($major_result->getMessage().'<br/>'.$major_result->getUserInfo(), E_USER_WARNING);
			return Array();
		}

		$this->_tmp['permission_cache']['full_permissions'][$permission] = $major_result;
		return $major_result;
	}//end getInheritedPermission()


	/**
	* Returns an array of all user or user_group assets that have the passed permission for this asset
	*
	* @param string		$permission		the permission code you are getting
	* @param boolean	$and_greater	get effective permission (eg read access = read or write or admin)
	* @param boolean	$and_inherited	get permission from parents as well
	*
	* @return array(int)
	* @access public
	*/
	function getRevokedPermission($permission)
	{
		$permission = (int) $permission;

		// return cached version if we can
		if (isset($this->_tmp['permission_cache']['revoked_permissions'][$permission])) {
			return $this->_tmp['permission_cache']['revoked_permissions'][$permission];
		}

		$db = &$GLOBALS['SQ_SYSTEM']->db;

		// just get the permissions set for this asset
		$sql = 'SELECT userid, assetid FROM sq_asset_permission
				WHERE assetid = '.$db->quote($this->id).'
				  AND permission = '.$db->quote($permission).'
				  AND revoked = 1';

		$result = $db->getAssoc($sql, false, Array(), DB_FETCHMODE_DEFAULT, true);
		if (DB::isError($result)) {
			trigger_error($result->getMessage().'<br/>'.$result->getUserInfo(), E_USER_WARNING);
			return Array();
		}

		$this->_tmp['permission_cache']['revoked_permissions'][$permission] = $result;
		return $result;
	}//end getRevokedPermission()


	/**
	* Returns true if at least one of the passed assetids has the passed permission
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
		if ($effective) {
			if ($this->status == SQ_STATUS_PENDING_APPROVAL || $this->status == SQ_STATUS_EDITING_APPROVAL) return false;

			if (empty($assetids)) {
				$lock = $this->getLockInfo();
				if (!empty($lock)) {
					// this asset is currently locked
					$user = &$GLOBALS['SQ_SYSTEM']->am->getAsset($lock['userid']);
					if ($user->id == $GLOBALS['SQ_SYSTEM']->currentUserid()) {
						return $this->_checkPermissionAccess($perm);
					}
					return false;
				}
			}
			return false;
		} else {
			return $this->_checkPermissionAccess($perm, $assetids);
		}
	}//end checkAccess()


	function _checkPermissionAccess($perm, $assetid=Array())
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

		$perms = array_keys($this->getPermission($perm));
		if (in_array('0', $perms)) return true;
		$common = array_intersect($assetids, $perms);
		return (!empty($common));
	}//end _checkDbAccess()


	/**
	* Returns true if at least one of the passed assetids has read access to this asset
	*
	* @param array $assetids An array of user_group and/or user ids to check for read access
	*						 If the array is empty, the current user will be checked
	*
	* @return boolean
	* @access public
	*/
	function readAccess($assetids=Array())
	{
		if (!$this->id) return true;
		return $this->_checkPermissionAccess(SQ_PERMISSION_READ, $assetids);
	}//end readAccess()


	/**
	* Returns true if at least one of the passed assetids has write access to this asset
	*
	* @param array	$assetids	an array of user_group and/or user ids to check for write access
	*							If the array is empty, the current user will be checked
	* @param boolean $effective	effective access baed on locking and status
	*
	* @return boolean
	* @access public
	*/
	function writeAccess($assetids=Array(), $effective=true)
	{
		if (!$this->id) return true;
		return $this->checkAccess(SQ_PERMISSION_WRITE, $assetids, $effective);
	}//end writeAccess()


	/**
	* Returns true if at least one of the passed assetids has admin access to this asset
	*
	* @param array	$assetids	an array of user_group and/or user ids to check for write access
	*							If the array is empty, the current user will be checked
	* @param boolean $effective	effective access baed on locking and status
	*
	* @return boolean
	* @access public
	*/
	function adminAccess($assetids=Array(), $effective=true)
	{
		if (!$this->id) return true;
		return $this->checkAccess(SQ_PERMISSION_ADMIN, $assetids, $effective);
	}//end adminAccess()


	/**
	* Prints out the Frontend for this asset
	* it's up to the kids to override
	*
	* @access public
	*/
	function printFrontend()
	{
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
		foreach($paths as $path) {
			$path = preg_replace('/\\s+/', '_', $path);
			// Taken (in part) from info here -> http://www.w3.org/Addressing/URL/5_URI_BNF.html
			$path = preg_replace('/[^a-zA-Z0-9\-$_@.&!*(),]/', '',  $path);
			$path = trim($path);
			// ignore blanks
			if ($path && !in_array($path, $save_paths)) $save_paths[] = $path;
		}// end foreach

		// if there is no difference in the arrays (including in the sort order), then there is nothing to do
		if (!array_diff_assoc($save_paths, $current_paths)) return;

		$GLOBALS['SQ_SYSTEM']->doTransaction('BEGIN');

		//// CHECK INSERTED PATHS ////
		// to make sure that they path name is not in use by any of our siblings
		$inserts = array_diff($save_paths, $current_paths);
		if (!empty($inserts)) {

			// now we need to check that none of our parents have
			// any nav kids that are also using this path name
			$siblingids = Array();
			$parents = $this->getLinks(SQ_LINK_NAV, '', null, 'minor');
			for($i = 0; $i < count($parents); $i++) {
				$parent = &$GLOBALS['SQ_SYSTEM']->am->getAsset($parents[$i]['majorid'], $parents[$i]['type_code']);
				if (is_null($parent)) continue;
				// get all the nav siblings
				$parent_kids = $parent->getLinks(SQ_LINK_NAV, '', null, 'major');
				for($j = 0; $j < count($parent_kids); $j++) {
					$siblingids[] = $db->quote($parent_kids[$j]['minorid']);
				}
			}

			$sibling_paths = Array();

			if (count($siblingids)) {
				$db_quote_inserts = Array();
				foreach($inserts as $v) $db_quote_inserts[] = $db->quote($v);
				$sql = 'SELECT path
						FROM sq_asset_path
						WHERE assetid IN ('.implode(',', $siblingids).')
						  AND path    IN ('.implode(',', $db_quote_inserts).')';

				$sibling_paths = $db->getCol($sql);
				if (DB::isError($sibling_paths)) {
					trigger_error($sibling_paths->getMessage().'<br/>'.$sibling_paths->getUserInfo(), E_USER_ERROR);
					return false;
				}

				if (!empty($sibling_paths)) {
					trigger_error('Path "'.implode('", "', $sibling_paths).'" already in use by one of the siblings of "'.$this->name.'"', E_USER_WARNING);
					return false;
				}// end if

			}// end if sublings

		}// end if inserts

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
	* Returns a list of lookup information associated with this asset
	*
	* @param string	$field	if only one field is required pass it's name through (url, root_urlid, designid)
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
	* Fn that does the real processing of updateLookups(), separated to allow for the DB transaction stuff
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

			$design_link = $this->getLink(SQ_LINK_USES, 'design', false);
			$designid = (is_null($design_link)) ? 0 : $design_link['minorid'];

			$parents = $this->getLinks(SQ_LINK_NAV, '', null, 'minor');
			for($i = 0; $i < count($parents); $i++) {
				$parent = &$GLOBALS['SQ_SYSTEM']->am->getAsset($parents[$i]['majorid'], $parents[$i]['type_code']);
				if (is_null($parent)) continue;

				$parent_urls = $parent->getLookups();
				for($j = 0; $j < count($parent_urls); $j++) {
					for($k = 0; $k < count($paths); $k++) {
						$sql = 'INSERT INTO sq_asset_lookup
								(url, assetid, root_urlid, designid)
								VALUES
								('.$db->quote($parent_urls[$j]['url'].'/'.$paths[$k]).',
								'.$db->quote($this->id).',
								'.$db->quote($parent_urls[$j]['root_urlid']).',
								'.$db->quote(($designid) ? $designid : $parent_urls[$j]['designid']).')';
						$result = $db->query($sql);
						if (DB::isError($result)) {
							trigger_error($result->getMessage().'<br/>'.$result->getUserInfo(), E_USER_ERROR);
							return false;
						}
					}// end for paths
				}// end for parent_urls

			}// end for parents

		}// end if count paths

		$kids = $this->getLinks(SQ_LINK_NAV, '', null, 'major');
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
		if (is_null($base_url)) $base_url = current_url(false);
		if (!isset($this->_tmp['url'])) $this->_tmp['url'] = Array();

		$base_url = strip_url($base_url, true);

		if (!isset($this->_tmp['url'][$base_url])) {
			$db = &$GLOBALS['SQ_SYSTEM']->db;

#		pre_echo("BASE URL : $base_url");
			$url_bits = explode('/', preg_replace('/\\/+$/', '', $base_url));

			$sql = 'SELECT l.url, u.http, (';
			$str = '';
			// so much for SQL standards...
			$concat = ($db->phptype == 'mysql') ? 'CONCAT(l.url, '.$db->quote('/').')' : 'l.url || '.$db->quote('/');
			foreach($url_bits as $i => $bit) {
				$str .= $bit.'/';
				$sql .= (($i) ? '+' : '').'
				(CASE WHEN SUBSTRING('.$concat.' FROM 1 FOR '.strlen($str).') = '.$db->quote($str).' THEN 1 ELSE 0 END)';
			}

			$sql .= '
			) as weighting
			FROM sq_asset_lookup l INNER JOIN sq_asset_url u ON l.root_urlid = u.urlid
			WHERE l.assetid = '.$db->quote($this->id).'
			ORDER BY weighting DESC, l.url
			LIMIT 1';
			$result = $db->getRow($sql);
			if (DB::isError($result)) {
				trigger_error($result->getMessage().'<br/>'.$result->getUserInfo(), E_USER_ERROR);
				return '';
			}

			if (empty($result)) {
				$this->_tmp['url'][$base_url] = '';
			} else {
				$this->_tmp['url'][$base_url] = (($result['http']) ? 'http' : 'https').'://'.$result['url'];
			}

		}// end if !isset

		return $this->_tmp['url'][$base_url];

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
		if (is_null($base_url)) $base_url = current_url(false);
		if (!isset($this->_tmp['href'])) $this->_tmp['href'] = Array();

		$base_url = strip_url($base_url, true);

		if (!isset($this->_tmp['href'][$base_url])) {
			if ($url = $this->getURL($base_url)) {
				require_once SQ_FUDGE_PATH.'/general/www.inc';
				// delieratley don't strip url here because if there is any trailing slashes then 
				// relative_href will work to accomodate them
				$this->_tmp['href'][$base_url] = relative_href(current_url(), $url);
			} else {
				$this->_tmp['href'][$base_url] = '';
			}
		}// end if !isset
		return $this->_tmp['href'][$base_url];
	}//end getHref()


}//end class
?>
