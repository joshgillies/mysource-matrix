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
	var $id;

	/**
	* String representation of this attribute
	* @var string
	*/
	var $name;

	/**
	* Short String representation of this attribute
	* @var string
	*/
	var $short_name;

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
	var $vars;

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
	var $_ser_atrs = false;

	/**
	* An array of tab code names and their display names
	* @var Array()
	*/
	var $_tabs = Array('permissions' => Array('name' => 'Permissions', 
											  'icon' => 'users'
											  )
					   );


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
	* Function to return's the suffix  a data path for a particular asset.
	*
	* @param object Asset	$asset
	*
	* @returns string
	* @access public
	*/
	function getDataPathSuffix($asset)
	{
		return (($asset->_restricted) ? '' : 'un').'restricted/'.$asset->type().'/'.$asset->id;
	}


	/**
	* Create this asset
	*
	* @param array(string)	$link	information used to create the initial link
	*
	* @return boolean
	* @access public
	*/
	function create($link)
	{

		if (!empty($link)) {
			// make sure the initial link information is passed in
			if (!isset($link['asset']))     trigger_error('Cant create asset without an asset to link to', E_USER_ERROR);
			if (!isset($link['link_type'])) trigger_error('Cant create asset without a link type', E_USER_ERROR);
		}

		$db = &$GLOBALS['SQ_SYSTEM']->db;

		// welcome to sequences, they are cool, see the PEAR::DB docs for more info
		$assetid = $db->nextId('sq_sequence_asset');
		if (DB::isError($assetid)) trigger_error($assetid->getMessage().'<br/>'.$assetid->getUserInfo(), E_USER_ERROR);

		$name   = ucwords(str_replace('_', ' ', $this->type())).' #'.$assetid;
		$now    = time();
		$userid = $GLOBALS['SQ_SYSTEM']->currentUserId();

		$sql = 'INSERT INTO sq_asset
				(assetid, type_code, name, short_name, last_updated, last_userid)
				VALUES
				('.$db->quote($assetid).','.$db->quote($this->type()).', '.$db->quote($name).', '.$db->quote($name).', '.$db->quote(ts_iso8601($now)).', '.$db->quote($userid).')';

		$result = $db->query($sql);
		if (DB::isError($result)) {
			trigger_error($result->getMessage().'<br/>'.$result->getUserInfo(), E_USER_WARNING);
			return false;
		}

		$this->id   = $assetid;
		$this->name = $name;
		$this->short_name   = $name;
		$this->last_updated = $now;
		$this->last_userid  = $userid;

		$this->_loadVars();
		$this->_updated();

		if (!empty($link)) {
			// create the initial link
			if (!isset($link['value'])) $link['value'] = '';
			if (!isset($link['position'])) $link['position'] = -1;
			return $link['asset']->createLink($this, $link['link_type'], $link['value'], $link['position']);
		} else {
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

		$sql = 'SELECT assetid, name, short_name, type_code, last_updated, last_userid
				FROM sq_asset
				WHERE assetid = '.$db->quote($assetid);
		$result = $db->getRow($sql, null, DB_FETCHMODE_ORDERED);
		if (DB::isError($result)) trigger_error($result->getMessage().'<br/>'.$result->getUserInfo(), E_USER_ERROR);

		if (is_null($result)) {
			return;
		}

		list($this->id, $this->name, $this->short_name, $type_code, $this->last_updated, $this->last_userid) = $result;
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

		$this->vars = $GLOBALS['SQ_SYSTEM']->db->getAssoc($sql, false, array(), DB_FETCHMODE_ASSOC); // need all this because of DB API
		if (DB::isError($this->vars)) trigger_error($this->vars->getMessage().'<br/>'.$this->vars->getUserInfo(), E_USER_ERROR);

		if ($this->_ser_atrs && $this->vars) {
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

		$now    = time();
		$userid = $GLOBALS['SQ_SYSTEM']->currentUserId();
		$name       = $this->_getName();
		$short_name = $this->_getName(true);

		$db = &$GLOBALS['SQ_SYSTEM']->db;
		$sql = 'UPDATE sq_asset
				SET name         = '.$db->quote($name).',
					short_name   = '.$db->quote($short_name).',
					last_updated = '.$db->quote(ts_iso8601($now)).',
					last_userid  = '.$db->quote($userid).'
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

		$this->name         = $name;
		$this->short_name   = $short_name;
		$this->last_updated = $now;
		$this->last_userid  = $userid;

		$GLOBALS['SQ_SYSTEM']->broadcastEvent('AssetUpdate', Array( 'assetid' => $this->id,
																	'old_name' => $old_name,
																	'new_name' => $this->name,
																	'old_short_name' => $old_short_name,
																	'new_short_name' => $this->short_name,
																	'old_last_updated' => $this,
																	'new_last_updated' => $this->last_updated,
																	'old_last_userid' => $old_last_userid,
																	'new_last_userid' => $this->last_userid
																   ));
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
	}//end _getName();


	/**
	* Creates a duplicate of this asset. It DOES NOT duplicate the links that this asset has either to it or from it
	* Returns a reference to the new object or if an error occured FALSE
	*
	* @param bool	$dupe_directory		whether or not to duplicate thia assets data directory
	*
	* @return object Asset | NULL
	* @access public
	*/
	function &duplicate($dupe_directory=true)
	{
		$null = null; // needed because we return by reference

		if (!$this->id) return $null;

		$db = &$GLOBALS['SQ_SYSTEM']->db;

		// welcome to sequences, they are cool, see the PEAR::DB docs for more info
		$assetid = $db->nextId('sq_sequence_asset');
		if (DB::isError($assetid)) trigger_error($assetid->getMessage().'<br/>'.$assetid->getUserInfo(), E_USER_ERROR);

		$now    = time();
		$userid = $GLOBALS['SQ_SYSTEM']->currentUserId();

		$sql = 'INSERT INTO sq_asset
				(assetid, type_code, name, short_name, last_updated, last_userid)
				VALUES
				('.$db->quote($assetid).','.$db->quote($this->type()).', '.$db->quote($this->name.' - Duplicate').', '.$db->quote($this->short_name.' - Duplicate').', '.$db->quote(ts_iso8601($now)).', '.$db->quote($userid).')';

		$result = $db->query($sql);
		if (DB::isError($result)) {
			trigger_error($result->getMessage().'<br/>'.$result->getUserInfo(), E_USER_WARNING);
			return $null;
		}

		$dupe = &$GLOBALS['SQ_SYSTEM']->am->getAsset($assetid, $this->type());
		if (is_null($dupe)) {
			return $null;
		}

		// Now we set all the attributes
		foreach($this->vars as $name => $data) {
			if (!$dupe->setAttrValue($name, $data['value'])) {
				return $null;
			}
		}

		// OK if we got this far let's copy the directory (if it exists)
		if ($dupe_directory && is_dir($this->data_path)) {
			require_once SQ_FUDGE_PATH.'/general/file_system.inc';
			if (!copy_directory($this->data_path, $dupe->data_path)) {
				return $null;
			}
		}

		// if we got this far all is well, so return the duplicate
		return $dupe;

	}//end duplicate()


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
		$db->query('BEGIN');

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
					$db->query('ROLLBACK');
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
					$db->query('ROLLBACK');
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
					$db->query('ROLLBACK');
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
				$db->query('ROLLBACK');
				return false;
			}
		}// end if delete var list

		$sql = 'UPDATE sq_asset
				SET type_code    = '.$db->quote($new_type_code).'
				WHERE assetid = '.$db->quote($this->id);

		$result = $db->query($sql);
		if (DB::isError($result)) {
			trigger_error($result->getMessage().'<br/>'.$result->getUserInfo(), E_USER_WARNING);
			$db->query('ROLLBACK');
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
			$db->query('COMMIT');
			return true;

		// bugger some thing went wrong
		} else {
			$db->query('ROLLBACK');
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
	* @return array(int)
	* @access public
	*/
	function getParents($type_code='', $strict_type_code=true)
	{
		return $GLOBALS['SQ_SYSTEM']->am->getParents($this->id, $type_code, $strict_type_code);
	}//end getParents()


	/**
	* Returns the link information for a specific link that this asset is the major asset in
	*
	* @param int	$linkid
	* @param string $side_of_link	Which side of the link this (the current) asset is on ('major' or 'minor')
	*
	* @return Array
	* @access public
	*/
	function getLinkById($linkid, $side_of_link='major')
	{
		if ($side_of_link != 'major' && $side_of_link != 'minor')
			trigger_error('Unknown Side of Link "'.$side_of_link.'"', E_USER_ERROR);

		if (!$this->id) return Array();

		$other_side = ($side_of_link == 'major') ? 'minor' : 'major';

		$db = &$GLOBALS['SQ_SYSTEM']->db;
		$sql = 'SELECT l.linkid, l.'.$other_side.'id, l.value, l.link_type, a.type_code, l.sort_order
				FROM sq_asset_link l, sq_asset a
				WHERE l.linkid = '.$db->quote($linkid).'
				  AND l.'.$side_of_link.'id = '.$db->quote($this->id).'
				  AND l.'.$other_side.'id   = a.assetid';

		$result = $db->getRow($sql);
		if (DB::isError($result))
			trigger_error($result->getMessage().'<br/>'.$result->getUserInfo(), E_USER_ERROR);

		return $result;

	}//end getLinkById()


	/**
	* Returns information for a link that this asset is the major asset in
	*
	* @param int	$link_type		integer that should be a single integer of the SQ_LINK_* constants
	* @param string	$type_code		the type of asset that is linked (eg 'Design_Area', 'Page', 'File', etc);
	* @param bool	$strict			whether we are finding an asset that are just a $type_code or $type_code and any of it's sub-classes
	* @param string $value			the value that is associated with this link
	* @param string $side_of_link	Which side of the link this (the current) asset is on ('major' or 'minor')
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
		$sql = 'SELECT l.linkid, l.'.$other_side.'id, l.value, l.link_type, a.type_code, l.sort_order
				FROM sq_asset_link l, sq_asset a
				WHERE l.'.$side_of_link.'id = '.$db->quote($this->id).'
				  AND l.'.$other_side.'id   = a.assetid
				  AND l.link_type = '.$db->quote($link_type);

		if ($type_code) {
			if ($strict_type_code) {
				$sql .= ' AND a.type_code = '.$db->quote($type_code);
			} else {
				require_once SQ_FUDGE_PATH.'/db_extras/db_extras.inc';
				$sql .= ' AND (a.type_code = '.$db->quote($type_code).' OR a.type_code IN (~SQ0~))';
				$subs = Array( 'SELECT type_code
								FROM sq_asset_type_inherited
								WHERE inherited_type_code = '.$db->quote($type_code));
				$sql = db_extras_subquery($db, $sql, $subs);
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
	* @param int	$link_types			integer that can be the product of bitwise operations on the SQ_LINK_* constants
	* @param string	$type_code			the type of asset that is linked (eg 'Page', 'File', etc);
	* @param bool	$strict_type_code	whether we are finding assets that are just a $type_code or 
	*									$type_code and any of it's sub-classes
	* @param string $side_of_link		Which side of the link this (the current) asset is on ('major' or 'minor')
	* @param string $value				The value for all the links must equal this (if not null)
	*
	* @return Array()
	* @access public
	*/
	function getLinks($link_types, $type_code='', $strict_type_code=true, $side_of_link='major', $value=null)
	{
		if ($side_of_link != 'major' && $side_of_link != 'minor')
			trigger_error('Unknown Side of Link "'.$side_of_link.'"', E_USER_ERROR);

		if (!$this->id) return Array();

		$other_side = ($side_of_link == 'major') ? 'minor' : 'major';

		$db = &$GLOBALS['SQ_SYSTEM']->db;
		$sql = 'SELECT l.linkid, l.'.$other_side.'id, l.value, l.link_type, a.type_code, l.sort_order
				FROM sq_asset_link l, sq_asset a
				WHERE l.'.$other_side.'id   = a.assetid
				  AND l.'.$side_of_link.'id = '.$db->quote($this->id).'
				  AND (l.link_type & '.$db->quote($link_types).') > 0 ';

		if (!is_null($value)) $sql .= '  AND l.value = '.$db->quote($value);

		if ($type_code) {
			if ($strict_type_code) {
				$sql .= ' AND a.type_code = '.$db->quote($type_code);
			} else {
				require_once SQ_FUDGE_PATH.'/db_extras/db_extras.inc';
				$sql .= ' AND (a.type_code = '.$db->quote($type_code).' OR a.type_code IN (~SQ0~))';
				$subs = Array( 'SELECT type_code
								FROM sq_asset_type_inherited
								WHERE inherited_type_code = '.$db->quote($type_code));
				$sql = db_extras_subquery($db, $sql, $subs);
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
	* @param string $side_of_link		Which side of the link this (the current) asset is on ('major' or 'minor')
	* @param int	$link_types			integer that can be the product of bitwise operations on the SQ_LINK_* constants
	* @param string	$type_code			the type of asset that is linked (eg 'Page', 'File', etc);
	* @param bool	$strict_type_code	whether we are finding assets that are just a $type_code or $type_code and any of it's sub-classes
	* @param int	$ignore_linkid		ignore the link represented by this link id when returning the count
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

			if ($strict_type_code) {
				$extra_where .= ' AND a.type_code = '.$db->quote($type_code);
			} else {
				require_once SQ_FUDGE_PATH.'/db_extras/db_extras.inc';
				$extra_where .= ' AND (a.type_code = '.$db->quote($type_code).' OR a.type_code IN (~SQ0~))';
				$subs = Array( 'SELECT type_code
								FROM sq_asset_type_inherited
								WHERE inherited_type_code = '.$db->quote($type_code));
				$extra_where = db_extras_subquery($db, $extra_where, $subs);
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
	* @param object Asset $minor  the minor asset that we are linking to
	* @param string $link_type    the type of link this is
	*
	* @return TRUE|string
	* @access public
	*/
	function canCreateLink(&$minor, $link_type)
	{
		if (!$this->id) return false;

		// if you dont have write access to this asset you cant create any links
		if (!$GLOBALS['SQ_INSTALL'] && !$this->writeAccess()) {
			return 'Cannot create link, permission denied';
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
#			pre_echo($sql);
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
	* @param object Asset $minor  the minor asset that we are linking to
	* @param string $link_type    the type of link this is
	* @param string $value        the value that is to be associated with this link
	* @param string $sort_order   the position in the links list that this link should take, if less than zero places at end of list
	*
	* @return int
	* @access public
	*/
	function createLink(&$minor, $link_type, $value='', $sort_order=-1)
	{
		if (!$this->id) return 0;
		require_once SQ_FUDGE_PATH.'/db_extras/db_extras.inc';

		if (!is_a($minor, 'Asset')) {
			trigger_error('Minor is not an asset, unable to create link', E_USER_WARNING);
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
		if ($sort_order > $max || $sort_order < 0) $sort_order = $max;

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
#			pre_echo($sql);
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

#			pre_echo($sql);
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
					  AND CHARACTER_LENGTH(ct.treeid) = '.(strlen($existing_treeid) + SQ_ASSET_TREE_SIZE).'
					  AND ct.linkid = 0
					LIMIT 1';
#			pre_echo($sql);
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
						  AND CHARACTER_LENGTH(ct.treeid) = '.(strlen($existing_treeid) + SQ_ASSET_TREE_SIZE);
#				pre_echo($sql);
				$free_childid = $db->getOne($sql);
				if (DB::isError($free_childid)) {
					trigger_error($free_childid->getMessage().'<br/>'.$free_childid->getUserInfo(), E_USER_WARNING);
					$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
					return 0;
				}
				// No max ? we must be the first
				if ((string) $free_childid == '') {
					if (($free_childid = asset_tree_base64_convert(0, true)) === false) {
						$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
						return 0;
					}

				// we found a max so increment it
				} else {
					// get only the child part
					$free_childid = substr($free_childid, -1 * SQ_ASSET_TREE_SIZE);
#					pre_echo("FREE CHILDID : ".$free_childid);
					if (($child_num    = asset_tree_base64_convert($free_childid, false)) === false) {
						$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
						return 0;
					}
					if (($free_childid = asset_tree_base64_convert($child_num + 1, true)) === false)  {
						$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
						return 0;
					}
				}// end if

			// else there was a free entry
			} else {

				// get only the child part
				$free_childid = substr($free_childid, -1 * SQ_ASSET_TREE_SIZE);

				// remove the current zeroed entries in the tree table,
				// as we will be inserting over the top of them
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
#				pre_echo($sql);
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
#				pre_echo($sql);
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

				$result = db_extras_insert_select($db, $insert, $select);
				if (DB::isError($result)) {
					trigger_error($result->getMessage().'<br/>'.$result->getUserInfo(), E_USER_WARNING);
					$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
					return 0;
				}

			}// end if

#			pre_echo("OK");

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
				(linkid, majorid, minorid, link_type, value, sort_order, last_updated, last_userid)
				VALUES
				('.$db->quote($linkid).', '.$db->quote($this->id).', '.$db->quote($minor->id).', '.$db->quote($link_type).', '
				.$db->quote($value).', '.$db->quote($sort_order).', '.$db->quote(ts_iso8601(time())).', '.$db->quote($GLOBALS['SQ_SYSTEM']->currentUserId()).')';

#		pre_echo($sql);

		$result = $db->query($sql);
		if (DB::isError($result)) {
			trigger_error($result->getMessage().'<br/>'.$result->getUserInfo(), E_USER_WARNING);
			$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
			return 0;
		}

		$GLOBALS['SQ_SYSTEM']->doTransaction('COMMIT');

		$GLOBALS['SQ_SYSTEM']->broadcastEvent('CreateLink', Array('assetid' => $this->id, 'linkid' => $linkid));
		$this->_linksUpdated();
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
	*
	* @access private
	*/
	function _linksUpdated()
	{
	}//end _linksUpdated()


	/**
	* Moves a link from one place in this assets links order to another position
	*
	* @param int	$linkid		the link id of the link to move about
	* @param int	$sort_order	the position in the links list that this link should take, if less than zero places at end of list
	*
	* @return boolean
	* @access public
	*/
	function moveLinkPos($linkid, $sort_order=-1)
	{
		// open the transaction
		$GLOBALS['SQ_SYSTEM']->doTransaction('BEGIN');
		if ($this->_updateLink($linkid, null, null, null, $sort_order)) {
			$GLOBALS['SQ_SYSTEM']->doTransaction('COMMIT');
			$this->_linksUpdated();
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
	* @param int	$sort_order	the position in the links list that this link should take, if less than zero places at end of list
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

#		pre_echo($sql);

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
				if (!$trash_folder->createLink($minor, SQ_LINK_UNITE)) {
					$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
					return false;
				}
			}

			//// UPDATE THE TREE ////

			// set all the links that are under this 
			$sql = 'UPDATE sq_asset_link_tree
					   SET linkid = 0
					WHERE treeid in (~SQ0~)';
			$concat = ($db->phptype == 'mysql') ? 'CONCAT(pt.treeid, '.$db->quote('%').')' : 'pt.treeid || '.$db->quote('%');
			$subs = Array('SELECT ct.treeid
							FROM sq_asset_link_tree pt, sq_asset_link_tree ct
							WHERE pt.linkid = '.$db->quote($linkid).'
							  AND ct.treeid LIKE '.$concat.'
							  AND ct.treeid >= pt.treeid');

			$sql = db_extras_subquery($db, $sql, $subs);
			if (DB::isError($sql)) {
				$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
				trigger_error($sql->getMessage().'<br/>'.$sql->getUserInfo(), E_USER_WARNING);
				return 0;
			}
#			pre_echo($sql);
			$result = $db->query($sql);
			if (DB::isError($result)) {
				$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
				trigger_error($result->getMessage().'<br/>'.$result->getUserInfo(), E_USER_WARNING);
				return 0;
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

		$GLOBALS['SQ_SYSTEM']->doTransaction('COMMIT');

		$GLOBALS['SQ_SYSTEM']->broadcastEvent('DeleteLink', Array('assetid' => $this->id, 'linkid' => $linkid));
		$this->_linksUpdated();
		return true;

	}//end deleteLink()


	/**
	* Set the value for the passed variable to the passed value
	*
	* @param string	$name	the name of the attribute
	* @param string	$value	the new value of the attribute
	*
	* @return boolean
	* @access public
	*/
	function setAttrValue($name, $value)
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

		// if this is a seriliaze attribute, unserilize it
		// now so that $save_value is the serialized version
		// and $value becomes the unserialize version for
		// later use by the asset
		if ($this->vars[$name]['type'] == 'serialise') {
			$value = unserialize($value);
		}

		$db = &$GLOBALS['SQ_SYSTEM']->db;

		// open the transaction
		$db->query('BEGIN');

		$attributeid = $this->vars[$name]['attributeid'];

		// delete previous entry, if any
		$sql = 'SELECT count(*) FROM sq_asset_attribute_value
				WHERE assetid = '.$db->quote($this->id).'
				  AND attributeid = '.$db->quote($attributeid);

		$count = $db->getOne($sql);
		if (DB::isError($count)) {
			trigger_error($count->getMessage().'<br/>'.$count->getUserInfo(), E_USER_WARNING);
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
			$db->query('ROLLBACK');
			trigger_error($result->getMessage().'<br/>'.$result->getUserInfo(), E_USER_WARNING);
			return false;
		}

		$this->vars[$name]['value'] = $value;

		// tell, the asset it has updated
		if (!$this->_updated()) {
			$db->query('ROLLBACK');
			return false;
		}

		// If we get this far, then it's all OK
		$db->query('COMMIT');

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
	* returns a reference to the actual attribute object, used for manipulting the object
	*
	* @param string		$name			the name of the attribute
	* @param boolean	$mute_errors	stops the outputting of errors in this fn, needed because you can't use the '@' operator when returning by reference
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
	* @param string	$permission	the permission code you are adding (R, W, A)
	*
	* @return boolean
	* @access public
	*/
	function grantPermission($assetid, $permission)
	{

		$assetid = (int) $assetid;
		$current = &$this->getPermission($permission, false);
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

		// add the permission into the database
		$sql = 'INSERT INTO sq_asset_permission
				(assetid, userid, permission)
				VALUES
				('.$db->quote($this->id).','.$db->quote($assetid).', '.$db->quote($permission).')';

		$result = $db->query($sql);
		if (DB::isError($result)) {
			trigger_error($result->getMessage().'<br/>'.$result->getUserInfo(), E_USER_WARNING);
			return false;
		}

		// add the permission to the cached version
		$current[$assetid][] = $this->id;

		// now add to effective permissions
		if (isset($this->_tmp['effective_permissions'])) {
			for(reset($this->_tmp['effective_permissions']);
														null !== ($perm = key($this->_tmp['effective_permissions']));
														next($this->_tmp['effective_permissions'])) {
				if ($perm > $permission) continue;
				if (!isset($this->_tmp['effective_permissions'][$perm])) continue;
				$effective =& $this->_tmp['effective_permissions'][$perm];
				if (isset($effective[$assetid])) $effective[$assetid] = Array();
				$effective[$assetid][] = $this->id;
			}
		}

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
		$current = &$this->getPermission($permission, false);
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

		// add the permission into the database
		$sql = 'DELETE FROM sq_asset_permission
				WHERE assetid = '.$db->quote($this->id).'
				  AND userid = '.$db->quote($assetid).'
				  AND permission = '.$db->quote($permission);

		$result = $db->query($sql);
		if (DB::isError($result)) {
			trigger_error($result->getMessage().'<br/>'.$result->getUserInfo(), E_USER_WARNING);
			return false;
		}

		// remove the permission from the cached version
		for (reset($current[$assetid]); null !== ($k = key($current[$assetid])); next($current[$assetid])) {
			if ($current[$assetid][$k] == $this->id) unset($current[$assetid][$k]);
		}
		if(empty($current[$assetid])) unset($current[$assetid]);

		// now remove from effective permissions
		if (isset($this->_tmp['effective_permissions'])) {
			for(reset($this->_tmp['effective_permissions']);
														null !== ($perm = key($this->_tmp['effective_permissions']));
														next($this->_tmp['effective_permissions'])) {
				if ($perm > $permission) continue;
				if (!isset($this->_tmp['effective_permissions'][$perm])) continue;
				$effective =& $this->_tmp['effective_permissions'][$perm];
				if (isset($effective[$assetid])) {
					for (reset($effective[$assetid]); null !== ($k = key($effective[$assetid])); next($effective[$assetid])) {
						if ($effective[$assetid][$k] == $this->id) {
							unset($effective[$assetid][$k]);
							// we break here because if there are multiple values
							// because we do actually need them for effective permissions
							break;
						}
					}
					if(empty($effective[$assetid])) unset($effective[$assetid]);
				}
			}
		}

		return true;
	}//end revokePermission()


	/**
	* Returns an array of all user or user_group assets that have the passed permission for this asset
	*
	* @param string	$permission	the permission code you are getting
	*
	* @return array(int)
	* @access public
	*/
	function getPermission($permission, $and_greater=true)
	{
		$permission = (int) $permission;

		// return cached version if we can
		if (isset($this->_tmp[(($and_greater) ? 'effective_' : '').'permissions'][$permission])) {
			return $this->_tmp[(($and_greater) ? 'effective_' : '').'permissions'][$permission];
		}

		$db = &$GLOBALS['SQ_SYSTEM']->db;

		$parents = $this->getParents();

		// get the permissions
		$in_string = '';
		foreach ($parents as $parent) $in_string .= $db->quote($parent).', ';
		$in_string .= $db->quote($this->id); // check permission set by this asset
		$in_string .= ", '0'";               // check public permissions

		$sql = 'SELECT userid, assetid FROM sq_asset_permission
				WHERE assetid IN ('.$in_string.')
				AND permission '.(($and_greater) ? '>=' : '=').' '.$db->quote($permission);

		$result = $db->getAssoc($sql, false, Array(), DB_FETCHMODE_DEFAULT, true);
		if (DB::isError($result)) {
			trigger_error($result->getMessage().'<br/>'.$result->getUserInfo(), E_USER_WARNING);
			return Array();
		}

		$this->_tmp[(($and_greater) ? 'effective_' : '').'permissions'][$permission] = $result;
		return $result;
	}//end getPermission()


	/**
	* Returns true if at least one of the passed assetids has the passed permission
	*
	* @param array $assetids An array of user_group and/or user ids to check for permission
	*						 If the array is empty, the current user will be checked
	*
	* @return boolean
	* @access public
	*/
	function checkAccess($perm, $assetids=Array())
	{
		if (empty($assetids) && $GLOBALS['SQ_SYSTEM']->user->id) {
			// if we are the root user, we can do anything
			if ($GLOBALS['SQ_SYSTEM']->userRoot()) return true;

			// if we are a system administrator, we can do anything
			// unless an asset overwrites this function
			if ($GLOBALS['SQ_SYSTEM']->userSystemAdmin()) return true;
			
			// use the current user and their groups
			$assetids = $GLOBALS['SQ_SYSTEM']->user->getParents(Array('user', 'user_group'));
			$assetids[] = $GLOBALS['SQ_SYSTEM']->user->id;
		}
		
		$perms = array_keys($this->getPermission($perm));
		if (in_array('0', $perms)) return true;
		$common = array_intersect($assetids, $perms);
		return (!empty($common));
	}//end checkAccess()


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
		return $this->checkAccess(SQ_PERMISSION_READ, $assetids);
	}//end readAccess()


	/**
	* Returns true if at least one of the passed assetids has write access to this asset
	*
	* @param array $assetids An array of user_group and/or user ids to check for write access
	*						 If the array is empty, the current user will be checked
	*
	* @return boolean
	* @access public
	*/
	function writeAccess($assetids=Array())
	{
		return $this->checkAccess(SQ_PERMISSION_WRITE, $assetids);
	}//end writeAccess()


	/**
	* Returns true if at least one of the passed assetids has admin access to this asset
	*
	* @param array $assetids An array of user_group and/or user ids to check for admin access
	*						 If the array is empty, the current user will be checked
	*
	* @return boolean
	* @access public
	*/
	function adminAccess($assetids=Array())
	{
		return $this->checkAccess(SQ_PERMISSION_ADMIN, $assetids);
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
		$url = preg_replace('/\/+$/', '', $_SERVER['HTTP_HOST'].$_SERVER['PHP_SELF']);
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
	* @param object Backend $backend
	*
	* @access public
	*/
	function paintBackend(&$backend)
	{
		$backend->out->addHiddenField('asset_action', (($this->id) ? 'edit' : 'create'));
		$ei = &$this->getEI();
		$ei->paint($this, $backend->out, empty($this->id));
	}// end paintBackend()

	/**
	* Processes a backend submission from this asset, returns true if all OK
	*
	* @param array(string)	$link	information used to create the initial link
	*
	* @return boolean
	* @access public
	*/
	function processBackend($link)
	{
		$ei = &$this->getEI();
		$processed = $ei->process($this, false);

		foreach($processed as $name) {
			$attr = &$this->getAttribute($name, true);
			if (is_null($attr)) continue;

			if ($attr->processed) {
				$this->setAttrValue($name, $attr->value);
			}

		}// end for

		return true;

	}// end processBackend()

	/**
	* returns a reference to the edit interface for this asset type
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

		$db->query('BEGIN');

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
			$GLOBALS['SQ_SYSTEM']->db->query('ROLLBACK');
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
				$GLOBALS['SQ_SYSTEM']->db->query('ROLLBACK');
				return false;
			}

		}// end foreach

		unset($this->_tmp['paths']);
		unset($this->_tmp['lookups']);
		unset($this->_tmp['url']);
		unset($this->_tmp['href']);

		if ($this->_updateLookups()) {
			$GLOBALS['SQ_SYSTEM']->db->query('COMMIT');
			return true;
		} else {
			$GLOBALS['SQ_SYSTEM']->db->query('ROLLBACK');
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
		$GLOBALS['SQ_SYSTEM']->db->query('BEGIN');
		if ($this->_updateLookups()) {
			$GLOBALS['SQ_SYSTEM']->db->query('COMMIT');
			return true;
		} else {
			$GLOBALS['SQ_SYSTEM']->db->query('ROLLBACK');
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
	* @see Asset::updateLookups(),  Asset::saveWebPaths()
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
								(url, linkid, assetid, root_urlid, designid)
								VALUES
								('.$db->quote($parent_urls[$j]['url'].'/'.$paths[$k]).',
								'.$db->quote($parents[$i]['linkid']).',
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
		require_once SQ_SYSTEM_ROOT.'/'.$GLOBALS['SQ_SYSTEM']->am->getInfo($this->type(), 'dir').'/'.$class_name.'.inc';
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
	* Returns the URL for this asset, closest to the current url
	*
	* @return string
	* @access public
	*/
	function getURL()
	{

		if (!isset($this->_tmp['url'])) {
			$db = &$GLOBALS['SQ_SYSTEM']->db;

			$url_bits = explode('/', preg_replace('/\\/+$/', '', current_url(false)));

			$sql = 'SELECT l.url, u.http, (';
			$str = '';
			// so much for SQL standards...
			$concat = ($db->phptype == 'mysql') ? 'CONCAT(l.url, '.$db->quote('/').')' : 'l.url || '.$db->quote('/');
			foreach($url_bits as $i => $bit) {
				$str .= $bit.'/';
				$sql .= (($i) ? '+' : '').'
				(CASE WHEN SUBSTRING('.$concat.' FROM 0 FOR '.strlen($str).') = '.$db->quote($str).' THEN 1 ELSE 0 END)';
			}

			$sql .= '
			) as weighting
			FROM sq_asset_lookup l INNER JOIN sq_asset_url u ON l.root_urlid = u.urlid
			WHERE l.assetid = '.$db->quote($this->id).'
			ORDER BY weighting DESC
			LIMIT 1';

			$result = $db->getRow($sql);
			if (DB::isError($result)) {
				trigger_error($result->getMessage().'<br/>'.$result->getUserInfo(), E_USER_ERROR);
				return '';
			}
			if (empty($result)) {
				$this->_tmp['url'] = '';
			} else {
				$this->_tmp['url'] = (($result['http']) ? 'http' : 'https').'://'.$result['url'];
			}

		}// end if !isset

		return $this->_tmp['url'];

	}//end getURL()


	/**
	* Returns the relative href for this asset, closest to the current url
	*
	* @return string
	* @access public
	*/
	function getHref()
	{
		if (!isset($this->_tmp['href'])) {
			require_once SQ_FUDGE_PATH.'/general/www.inc';
			if ($url = $this->getURL()) {
				$this->_tmp['href'] = relative_href(current_url(), $url);
			} else {
				$this->_tmp['href'] = '';
			}
		}// end if !isset
		return $this->_tmp['href'];
	}//end getHref()


}//end class
?>
