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
	* @var Array()
	*/
	var $id;

	/**
	* String representation of this attribute
	* @var string
	*/
	var $name;

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
	* Constructor
	*
	* @param int    $assetid      the asset id to be loaded
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
	*
	* @return boolean
	* @access public
	*/
	function create()
	{

		$db = &$GLOBALS['SQ_SYSTEM']->db;

		// welcome to sequences, they are cool, see the PEAR::DB docs for more info
		$assetid = $db->nextId('sq_sequence_asset');
		if (DB::isError($assetid)) trigger_error($assetid->getMessage().'<br/>'.$assetid->getUserInfo(), E_USER_ERROR);

		$name   = ucwords(str_replace('_', ' ', $this->type())).' #'.$assetid;
		$now    = time();
		$userid = $GLOBALS['SQ_SYSTEM']->currentUserId();

		$sql = 'INSERT INTO sq_asset
				(assetid, type_code, name, last_updated, last_userid)
				VALUES
				('.$db->quote($assetid).','.$db->quote($this->type()).', '.$db->quote($name).', '.$db->quote(ts_iso8601($now)).', '.$db->quote($userid).')';

		pre_echo($sql);

		$result = $db->query($sql);
		if (DB::isError($result)) {
			trigger_error($result->getMessage().'<br/>'.$result->getUserInfo(), E_USER_WARNING);
			return false;
		}

		$this->id   = $assetid;
		$this->name = $name;
		$this->last_updated = $now;
		$this->last_userid  = $userid;

		$this->_loadVars();

		return true;

	}//end create()


	/**
	* Load the asset represented by the passed asset id
	*
	* @param int    $assetid      the asset id to be loaded
	*
	* @access public
	*/
	function load($assetid)
	{

		$db = &$GLOBALS['SQ_SYSTEM']->db;

		// OK, the first thing to do is check we actually exist :)

		$sql = 'SELECT assetid, name, type_code, last_updated, last_userid
				FROM sq_asset
				WHERE assetid = '.$db->quote($assetid);
		$result = $db->getRow($sql, null, DB_FETCHMODE_ORDERED);
		if (DB::isError($result)) trigger_error($result->getMessage().'<br/>'.$result->getUserInfo(), E_USER_ERROR);

		if (is_null($result)) {
			return;
		}

		list($this->id, $this->name, $type_code, $this->last_updated, $this->last_userid) = $result;
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

#		pre_echo($sql);

		$this->vars = $GLOBALS['SQ_SYSTEM']->db->getAssoc($sql, false, array(), DB_FETCHMODE_ASSOC); // need all this because of DB API
		if (DB::isError($this->vars)) trigger_error($this->vars->getMessage().'<br/>'.$this->vars->getUserInfo(), E_USER_ERROR);

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
		$name   = $this->_getName();

		$db = &$GLOBALS['SQ_SYSTEM']->db;
		$sql = 'UPDATE sq_asset
				SET name         = '.$db->quote($name).',
					last_updated = '.$db->quote(ts_iso8601($now)).',
					last_userid  = '.$db->quote($userid).'
				WHERE assetid = '.$db->quote($this->id);
		
		pre_echo($sql);

		$result = $db->query($sql);
		if (DB::isError($result)) {
			trigger_error($result->getMessage().'<br/>'.$result->getUserInfo(), E_USER_WARNING);
			return false;
		}
		
		$this->name         = $name;
		$this->last_updated = $now;
		$this->last_userid  = $userid;

		return true;

	}//end _updated();

	/**
	* Returns a name to use to describe this instance of this asset
	* should be overridden, used to set $this->name
	*
	* @access private
	*/
	function _getName() 
	{
		return ucwords(str_replace('_', ' ', $this->type())).' #'.$this->id;
	}//end _getName();

	/**
	* Returns an array of all assets types that this asset extends from
	*
	* @param string $code_name  the code name for the asset that you want to check
	*
	* @access public
	*/
	function getParentList() 
	{

		return $GLOBALS['SQ_SYSTEM']->am->getParentList($this->type());

	}//end getParentList()


	/**
	* Returns the link information for a specific link that this asset is the major asset in
	* 
	* @param int	linkid
	*
	* @return Array
	* @access public
	*/
	function getLink($linkid)
	{

		$db = &$GLOBALS['SQ_SYSTEM']->db;
		$sql = 'SELECT linkid, majorid, minorid, link_type, value, sort_order, last_updated, last_userid
				FROM sq_asset_link
				WHERE majorid = '.$db->quote($this->id).'
				  AND linkid = '.$db->quote($linkid);

		error_log("\n".$sql);

		$result = $db->getRow($sql);
		if (DB::isError($result)) 
			trigger_error($result->getMessage().'<br/>'.$result->getUserInfo(), E_USER_ERROR);

		return $result;
			
	}//end getLink()


	/**
	* Return all links that this asset has to other assets
	*
	* @param int	$link_types		integer that can be the product of bitwise operations on the SQ_LINK_* constants
	* @param string	$asset_type		the type of asset that is linked (eg 'Page', 'File', etc);
	* @param string	$ret_type		flag to the structure of the return array - only relevant when $asset_type is blank
	*								'T' = sorted/structured by type 
	*									eg Array('folder' => array(link1, link2, ..), 'user' => Array(link3, link4, ..))
	*								'S' = sorted by sort_order, single array 
	*									eg Array(link1, link2, ..)
	* @param string $side_of_link	Which side of the link this asset is on ('major' or 'minor')
	*
	* @return Array()
	* @access public
	*/
	function getLinks($link_types, $asset_type='', $ret_type='T', $side_of_link='major') 
	{
		if ($side_of_link != 'major' && $side_of_link != 'minor') 
			trigger_error('Unknown Side of Link "'.$side_of_link.'"', E_USER_ERROR);
	
		if (!isset($this->_tmp['links'])) $this->_tmp['links'] = Array();
		if (!isset($this->_tmp['links'][$side_of_link])) $this->_tmp['links'][$side_of_link] = Array();

		$elem_types = bit_elements($link_types);
		// OK, let's check what we already have cached, and only do a DB call for what we need
		$search_link_types = 0;
		foreach($elem_types as $link_type) {
			// if we haven't got all the links yet or we have got the specific type of links
			if (empty($this->_tmp['links'][$side_of_link][$link_type.'___GOT_ALL___'])
				|| ($asset_type && !isset($this->_tmp['links'][$side_of_link][$link_type][$asset_type]))) {
				$search_link_types |= $link_type;
			}
		}

		if ($search_link_types) {

			$other_side = ($side_of_link == 'major') ? 'minor' : 'major';

			$db = &$GLOBALS['SQ_SYSTEM']->db;
			$sql = 'SELECT l.linkid, l.'.$other_side.'id, l.value, l.link_type, a.type_code, l.sort_order
					FROM sq_asset_link l, sq_asset a
					WHERE l.'.$other_side.'id   = a.assetid 
					  AND l.'.$side_of_link.'id   = '.$db->quote($this->id).'
					  AND (l.link_type & '.$db->quote($search_link_types).') > 0 '
					.(empty($asset_type) ? 'ORDER BY l.sort_order, a.type_code' : ' AND a.type_code = '.$db->quote($asset_type));

			$result = $db->query($sql);
			if (DB::isError($result)) 
				trigger_error($result->getMessage().'<br/>'.$result->getUserInfo(), E_USER_ERROR);

			$this->_tmp['links'][$side_of_link][$link_type] = Array();
			while (null !== ($row = $result->fetchRow())) {
				if (!isset($this->_tmp['links'][$side_of_link][$row['link_type']][$row['type_code']])) $this->_tmp['links'][$side_of_link][$row['link_type']][$row['type_code']] = Array();
				$this->_tmp['links'][$side_of_link][$row['link_type']][$row['type_code']][] = $row;
			}//end while
			$result->free();

			// a bit of integrity
			if (empty($asset_type)) {
				foreach($elem_types as $link_type) $this->_tmp['links'][$side_of_link][$link_type.'___GOT_ALL___'] = true;

			} else {
				foreach($elem_types as $link_type) {
					if (!isset($this->_tmp['links'][$side_of_link][$link_type][$asset_type])) {
						// cause we are just need to set this to something, 
						// no need to waste space with an array
						$this->_tmp['links'][$side_of_link][$link_type][$asset_type] = false;
					}
				}
			}

		}//end if

		$ret_arr = Array();

		// if we want all asset types, if get's a bit messy
		if (empty($asset_type)) {
			if ($ret_type == 'T') {
				foreach($elem_types as $link_type) {
					if (!empty($this->_tmp['links'][$side_of_link][$link_type])) {
						$ret_arr = array_merge_recursive($ret_arr, $this->_tmp['links'][$side_of_link][$link_type]);
					}
				}// end foreach
			} else {

				foreach($elem_types as $link_type) {
					if (!empty($this->_tmp['links'][$side_of_link][$link_type])) {
						for(reset($this->_tmp['links'][$side_of_link][$link_type]); null !== ($type = key($this->_tmp['links'][$side_of_link][$link_type])); next($this->_tmp['links'][$side_of_link][$link_type])) {
							$ret_arr = array_merge($ret_arr, $this->_tmp['links'][$side_of_link][$link_type][$type]);
						}
					}
				}// end foreach
				usort($ret_arr, Array(__CLASS__, '_getLinksCmp'));

			}// end if

		// else we only want a specific type
		} else {
			foreach($elem_types as $link_type) {
				$ret_arr = array_merge($ret_arr, $this->_tmp['links'][$side_of_link][$link_type][$asset_type]);
			}

		}// end if

		return $ret_arr;

	}//end getLinks()

	/* This is the static comparing function: */
    function _getLinksCmp($a, $b)
    {
        if ($a['sort_order'] == $b['sort_order']) return 0;
        return ($a['sort_order'] < $b['sort_order']) ? -1 : +1;
    }



	/**
	* Returns the number of links that this asset has as either a major or minor party
	*
	* @param int		$link_types		integer that can be the product of bitwise operations on the SQ_LINK_* constants
	* @param string		$asset_type		the type of asset that is linked (eg 'Page', 'File', etc);
	* @param boolean	$major			type of party that the asset is in the links major = true, minor = false
	*
	* @return int
	* @access public
	*/
	function countLinks($link_types, $asset_type='', $major=true) 
	{
		$party = ($major) ? 'major' : 'minor';


		$db = &$GLOBALS['SQ_SYSTEM']->db;
		if (empty($asset_type)) {
			$sql = 'SELECT COUNT(*)
					FROM sq_asset_link l
					WHERE l.'.$party.'id   = '.$db->quote($this->id).'
					  AND (link_type & '.$db->quote($link_types).') > 0';
		} else {
			$sql = 'SELECT COUNT(*)
					FROM sq_asset_link l, sq_asset a
					WHERE l.minorid   = a.assetid 
					  AND l.'.$party.'id   = '.$db->quote($this->id).'
					  AND (link_type & '.$db->quote($link_types).') > 0 
					  AND a.type_code = '.$db->quote($asset_type);
		}

		$result = $db->getOne($sql);
		if (DB::isError($result)) 
			trigger_error($result->getMessage().'<br/>'.$result->getUserInfo(), E_USER_ERROR);
		
		return $result;

	}//end countLinks()
	
	/**
	* Called when the links are changed for this asset so that the tmp cache can be cleared
	*
	* @param string	$link_type		the type of link this is
	* @param string	$asset_type		the type of asset that is linked (eg 'Page', 'File', etc);
	* @param string $side_of_link	Which side of the link this asset is on ('major' or 'minor')
	*
	* @access private
	*/
	function _linkChanged($link_type, $asset_type='', $side_of_link='major') 
	{

		if (empty($this->_tmp['links'][$side_of_link])) return;

		if (empty($asset_type)) {
			unset($this->_tmp['links'][$side_of_link][$link_type]);
		} else {
			// only unset this specific type so calls for any other type 
			// can used the cached version if it exists
			unset($this->_tmp['links'][$side_of_link][$link_type][$asset_type]);
		}
		
		unset($this->_tmp['links'][$side_of_link][$link_type.'___GOT_ALL___']);

	}//end _linkChanged()

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

		if (!is_a($minor, 'Asset')) {
			trigger_error('Minor is not an asset, unable to create link', E_USER_WARNING);
			return false;
		}

		// First, we should check that we don't already have a link of this type
		$current_links = $this->getLinks($link_type, $minor->type());
		$num_curr_links = count($current_links);
		for($i = 0; $i < $num_curr_links; $i++) {
			// it already exists, so we don't need to do anything
			if ($current_links[$i]['minorid'] == $minor->id && $current_links[$i]['value'] == $value) {
				trigger_error('Link Already Exists', E_USER_WARNING);
				return true;
			}
		}//end for

		// check if we are allowed to link to these type of assets
		if (($err_msg = $this->canLinkToType($minor->type(), $link_type)) !== true) {
			trigger_error($err_msg, E_USER_WARNING);
			return false;
		}

		$db = &$GLOBALS['SQ_SYSTEM']->db;

		// if this link is a nav link, then we need to make sure that this
		// minor asset doesn't already have an exclusive link from anything
		if ($link_type & SQ_LINK_NAV) {
			$sql = 'SELECT majorid
					FROM sq_asset_link
					WHERE minorid   = '.$db->quote($minor->id).'
					  AND link_type = '.$db->quote(SQ_LINK_EXCLUSIVE);
//			pre_echo($sql);


			$majorid = $db->getOne($sql);
			if (DB::isError($majorid)) {
				trigger_error($majorid->getMessage().'<br/>'.$majorid->getUserInfo(), E_USER_WARNING);
				return false;
			}

			if ($majorid) {
				$major = &$GLOBALS['SQ_SYSTEM']->am->getAsset($majorid);
				trigger_error('Asset "'.$minor->name.'" (Asset #'.$minor->id.') cannot be linked to "'.$this->name.'" (Asset #'.$this->id.') it is exclusively linked to Asset #'.$majorid.' ("'.$major->name.'")', E_USER_WARNING);
				return false;
			}
			
		}//end if


		// make sure the sort order is in a valid range
		$sql = 'SELECT COUNT(*) as count, MAX(sort_order) as max
				FROM sq_asset_link
				WHERE majorid = '.$db->quote($this->id);
		$row = $db->getRow($sql);
		if (DB::isError($row)) {
			trigger_error($row->getMessage().'<br/>'.$row->getUserInfo(), E_USER_WARNING);
			return false;
		}

		$max = ($row['count'] > 0) ? (int) $row['max'] + 1 : 0;
		if ($sort_order > $max || $sort_order < 0) $sort_order = $max;


		// sequences are cool, see the PEAR::DB docs for more info
		$linkid = $db->nextId('sq_sequence_asset_link');
		if (DB::isError($linkid)) {
			trigger_error($linkid->getMessage().'<br/>'.$linkid->getUserInfo(), E_USER_WARNING);
			return false;
		}

		$sql = 'INSERT INTO sq_asset_link
				(linkid, majorid, minorid, link_type, value, sort_order, last_updated, last_userid)
				VALUES
				('.$db->quote($linkid).', '.$db->quote($this->id).', '.$db->quote($minor->id).', '.$db->quote($link_type).', '
				.$db->quote($value).', '.$db->quote($sort_order).', '.$db->quote(ts_iso8601(time())).', '.$db->quote($GLOBALS['SQ_SYSTEM']->currentUserId()).')';

		error_log($sql);

		$result = $db->query($sql);
		if (DB::isError($result)) {
			trigger_error($result->getMessage().'<br/>'.$result->getUserInfo(), E_USER_WARNING);
			return false;
		}

		// move the sort_order for all the other assets up one
		$sql = 'UPDATE sq_asset_link
				SET sort_order = sort_order + 1
				WHERE majorid     = '.$db->quote($this->id).'
				  AND sort_order >= '.$db->quote($sort_order).'
				  AND linkid != '.$db->quote($linkid);
		error_log($sql);

		$result = $db->query($sql);
		if (DB::isError($result)) {
			trigger_error($result->getMessage().'<br/>'.$result->getUserInfo(), E_USER_WARNING);
			return false;
		}

		$this->_linkChanged($link_type, $minor->type());

		return $linkid;

	}//end createLink()


	/**
	* Checks whether a link of a certain type can be made to this asset
	* Returns TRUE if link allowed and a string containing the error if it is not
	*
	* @param string $type_code	the type code that we want to check
	* @param string $link_type  the type of link to check
	*
	* @return mixed
	* @access public
	*/
	function canLinkToType($type_code, $link_type) 
	{


		// get the minor assets parents and add it's type to the front of the indexed array
		$types = $GLOBALS['SQ_SYSTEM']->am->getParentList($type_code);
		array_unshift($types, $type_code);

		$allowed_links = $this->_getAllowedLinks();

		// we will be ascending up the parent tree from the current asset type.
		// that way the major asset can have specific cardinality for different assets types
		// EG, assets => 'M', user => '1' -> this means that many assets can be linked to this this asset
		//     and only one user, but because 'user' is an 'asset' we need to check for any 'user' references
		//     before we check for any 'asset' references
		$type = '';
		for($i = 0; $i < count($types); $i++) {
			$type = $types[$i];
			if (!empty($allowed_links[$link_type][$type])) break;
		}

		if (empty($allowed_links[$link_type][$type])) {
			include_once SQ_INCLUDE_PATH.'/general_occasional.inc';
			return 'Assets of Type "'.$type_code.'" cannot be "'.link_type_name($link_type).'" linked to a "'.$this->type().'"';
		}


		//// Now we need to check that the cardinality is kept valid ////
		$current_links = $this->getLinks($link_type, $type);
		$num_curr_links = count($current_links);

		// if we are only allowed one of these links and we already have our one link, bugger off
		if ($allowed_links[$link_type][$type] == 1 && $num_curr_links > 0) {
			return 'Assets of Type "'.$this->type().'" can only have one "'.$link_type.'" link to a "'.$type.'"';
		}//end if

		// if we get this far all is OK
		return true;

	}// canLinkToType()


	/**
	* Returns an array of all the permitted links type, the type asset and the cardinality
	* In the form
	*
	*    Array('[link_type]' => Array('[asset_type]' => '[cardinality]));
	* Where:
	*   link_type   = SQ_LINK_EXCLUSIVE|SQ_LINK_UNITE|SQ_LINK_NOTICE
	*   cardinality = 1|M
	*
	* @return Array()
	* @access private
	*/
	function _getAllowedLinks() 
	{

		return Array(SQ_LINK_EXCLUSIVE => Array(),
					 SQ_LINK_UNITE     => Array(),
					 SQ_LINK_NOTICE    => Array()
					);

	}//end _getAllowedLinks()


	/**
	* Moves a link from one place in this assets links order to another position
	*
	* @param int	$linkid		the link id of the link to move about
	* @param string $sort_order	the position in the links list that this link should take, if less than zero places at end of list
	*
	* @return boolean
	* @access public
	*/
	function moveLinkPos($linkid, $sort_order=-1) 
	{

		// First, we should try and find the link
		$link = $this->getLink($linkid);
		if (empty($link)) {
			trigger_error('Existing Link #'.$linkid.' not found, unable to move link', E_USER_WARNING);
			return false;
		}

		// dickhead check
		if ($link['sort_order'] == $sort_order) {
			return true;
		}

		$db = &$GLOBALS['SQ_SYSTEM']->db;

		// open the transaction
		$db->query('BEGIN');

		$sql = 'SELECT COUNT(*) as count, MAX(sort_order) as max
				FROM sq_asset_link
				WHERE majorid = '.$db->quote($this->id);
		$row = $db->getRow($sql);
		if (DB::isError($row)) {
			$db->query('ROLLBACK');
			trigger_error($row->getMessage().'<br/>'.$row->getUserInfo(), E_USER_WARNING);
			return false;
		}
		$max = ($row['count'] > 0) ? (int) $row['max'] : 0;
		if ($sort_order > $max || $sort_order < 0) $sort_order = $max;

		$sql = 'UPDATE sq_asset_link
				SET sort_order   = '.$db->quote($sort_order).',
				    last_userid  = '.$db->quote($GLOBALS['SQ_SYSTEM']->currentUserId()).',
				    last_updated = '.$db->quote(ts_iso8601(time())).'
				WHERE linkid    = '.$db->quote($linkid).'
				  AND majorid   = '.$db->quote($this->id);

		error_log($sql);

		$result = $db->query($sql);
		if (DB::isError($result)) {
			$db->query('ROLLBACK');
			trigger_error($result->getMessage().'<br/>'.$result->getUserInfo(), E_USER_WARNING);
			return false;
		}

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

		error_log($sql);

		$result = $db->query($sql);
		if (DB::isError($result)) {
			$db->query('ROLLBACK');
			trigger_error($result->getMessage().'<br/>'.$result->getUserInfo(), E_USER_WARNING);
			return false;
		}

		$db->query('COMMIT');

		$this->_linkChanged($link['link_type']);

		return true;

	}//end moveLinkPos()


	/**
	* Removes a link where this asset is the major player
	*
	* @param int	$linkid		the link id of the link to remove
	*
	* @return boolean
	* @access public
	*/
	function deleteLink($linkid) 
	{

		// First, we should try and find the link
		$link = $this->getLink($linkid);
		if (empty($link)) {
			trigger_error('Existing Link #'.$linkid.' not found, unable to delete link', E_USER_WARNING);
			return false;
		}

		$db = &$GLOBALS['SQ_SYSTEM']->db;

		// open the transaction
		$db->query('BEGIN');

		$sql = 'DELETE FROM sq_asset_link
				WHERE linkid    = '.$db->quote($linkid).'
				  AND majorid   = '.$db->quote($this->id);

		error_log($sql);

		$result = $db->query($sql);
		if (DB::isError($result)) {
			$db->query('ROLLBACK');
			trigger_error($result->getMessage().'<br/>'.$result->getUserInfo(), E_USER_WARNING);
			return false;
		}

		// move 'em up, higher
		$sql = 'UPDATE sq_asset_link
				SET sort_order = sort_order - 1
				WHERE majorid    = '.$db->quote($this->id).'
				  AND sort_order > '.$db->quote($link['sort_order']);

		error_log($sql);

		$result = $db->query($sql);
		if (DB::isError($result)) {
			$db->query('ROLLBACK');
			trigger_error($result->getMessage().'<br/>'.$result->getUserInfo(), E_USER_WARNING);
			return false;
		}

		$db->query('COMMIT');

		$this->_linkChanged($link['link_type']);

		return true;

	}//end deleteLink()


	/**
	* Set the value for the passed variable to the passed value
	*
	* @access public
	*/
	function setAttrValue($name, $value) 
	{

		if (empty($this->vars[$name])) {
			trigger_error('Attribute "'.$name.'" not found for Asset of type "'.$this->type().'"', E_USER_WARNING);
			return false;
		}

		if ($this->vars[$name]['value'] == $value) {
			return true;
		}

		trigger_error('Validate value of attribute in setAttrValue()', E_USER_WARNING);


		$db = &$GLOBALS['SQ_SYSTEM']->db;

		// open the transaction
		$db->query('BEGIN');

		$attributeid = $this->vars[$name]['attributeid'];

		// delete previous entry, if any
		$sql = 'SELECT count(*) FROM sq_asset_attribute_value
				WHERE assetid = '.$db->quote($this->id).'
				  AND attributeid = '.$db->quote($attributeid);

//		pre_echo($sql);
		$count = $db->getOne($sql);
		if (DB::isError($count)) {
			trigger_error($count->getMessage().'<br/>'.$count->getUserInfo(), E_USER_WARNING);
			return false;
		}

		if ($count) {
			// now insert new entry
			$sql = 'UPDATE sq_asset_attribute_value
					SET custom_value = '.$db->quote($value).'
					WHERE assetid = '.$db->quote($this->id).'
					  AND attributeid = '.$db->quote($attributeid);

		} else {
			// now insert new entry
			$sql = 'INSERT INTO sq_asset_attribute_value
					(assetid, attributeid, custom_value)
					VALUES
					('.$db->quote($this->id).', '.$db->quote($attributeid).', '.$db->quote($value).')';

		}// end if

		pre_echo($sql);

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
		if (empty($this->vars[$name])) {
			trigger_error('Attribute "'.$name.'" not found for Asset"'.$this->type().'"', E_USER_WARNING);
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
	* @return object Asset_Attribute
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

				include_once SQ_ATTRIBUTES_PATH.'/'.$this->vars[$name]['type'].'/'.$this->vars[$name]['type'].'.inc';
				$attr_class = 'Asset_Attribute_'.$this->vars[$name]['type'];
				$this->_tmp['attributes'][$name] = new $attr_class($this->vars[$name]['attributeid'], $this->vars[$name]['value']);
				// make sure of the asset is correct
				if (empty($this->_tmp['attributes'][$name]->id)) $this->_tmp['attributes'][$name] = null;

			}//end if

		}//end if

		return $this->_tmp['attributes'][$name];

	}//end getAttribute()


	/**
	* prints out the backend interface for the asset
	* it's up to the kids to override
	*
	* @param object Backend $backend
	*
	* @access public
	*/
	function printBackend(&$backend) 
	{
		$backend->out->addHiddenField('asset_action', (($this->id) ? 'edit' : 'create'));
		$ei = &$this->getEI();
		$ei->paint($this, $backend->out, empty($this->id));
	}// end printBackend()

	/**
	* processes a backend submission from this asset, returns true if all OK
	*
	* @return boolean
	* @access public
	*/
	function processBackend() 
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
	* @return boolean
	* @access public
	*/
	function &getEI() 
	{
		if (empty($this->_tmp['ei']) || get_class($this->_tmp['ei']) != 'asset_edit_interface') {
			include_once SQ_INCLUDE_PATH.'/asset_edit_interface.inc';
			$this->_tmp['ei'] = new Asset_Edit_Interface($this->type());
		}
		return $this->_tmp['ei'];
	}// end getEI()

	/**
	* returns a list of paths that this asset has
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
					WHERE assetid = '.$db->quote($this->id);

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
		$db->query('BEGIN');

		$current_paths = $this->getWebPaths();
		$save_paths = Array();

		// make sure these paths are compliant
		for($i = 0; $i < count($paths); $i++) {
			$path = preg_replace('/\\s+/', '_', $paths[$i]);
			// Taken (in part) from info here -> http://www.w3.org/Addressing/URL/5_URI_BNF.html
			$path = preg_replace('/[^a-zA-Z0-9\-$_@.&!*(),]/', '',  $path);
			$path = trim($path);
			// ignore blanks
			if ($path) $save_paths[] = $path;
		}


		//// INSERT ANY NEW PATHS ////
		$inserts = array_values(array_diff($save_paths, $current_paths));

		if (count($inserts)) {
			
			// now we need to check that none of our parents have 
			// any nav kids that are also using this path name
			$siblingids = Array();
			$parents = $this->getLinks(SQ_LINK_NAV, '', 'S', 'minor');
			for($i = 0; $i < count($parents); $i++) {
				$parent = &$GLOBALS['SQ_SYSTEM']->am->getAsset($parents[$i]['majorid'], $parents[$i]['type_code']);
				if (is_null($parent)) continue;
				// get all the nav siblings
				$parent_kids = $parent->getLinks(SQ_LINK_NAV, '', 'S', 'major');
				for($j = 0; $j < count($parent_kids); $j++) {
					$siblingids[] = $db->quote($parent_kids[$j]['minorid']);
				}
			}

			$sibling_paths = Array();

			if (count($siblingids)) {
				$db_quote_inserts = Array();
				foreach($inserts as $k => $v) $db_quote_inserts[$k] = $db->quote($v);
				$sql = 'SELECT path
						FROM sq_asset_path
						WHERE assetid IN ('.implode(',', $siblingids).')
						  AND path    IN ('.implode(',', $db_quote_inserts).')';

				$sibling_paths = $db->getCol($sql);
				if (DB::isError($sibling_paths)) {
					trigger_error($sibling_paths->getMessage().'<br/>'.$sibling_paths->getUserInfo(), E_USER_ERROR);
					return false;
				} 

			}// end if

			foreach($inserts as $path) {

				if (in_array($path, $sibling_paths)) {
					trigger_error('Path "'.$path.'" already in use for one of the siblings of "'.$this->name.'"', E_USER_WARNING);
					return false;
				}
				$sql = 'INSERT INTO sq_asset_path
						(path, assetid)
						VALUES
						('.$db->quote($path).', '.$db->quote($this->id).')';
				$result = $db->query($sql);
				if (DB::isError($result)) {
					trigger_error($result->getMessage().'<br/>'.$result->getUserInfo(), E_USER_ERROR);
					return false;
				}

			}// end foreach

		}// end if

		//// NOW DELETE ANY OLD PATHS ////
		$deletes = array_diff($current_paths, $save_paths);

		foreach($deletes as $path) {
			// Remove all the old URLs
			$sql = 'DELETE FROM sq_asset_path
					WHERE path    = '.$db->quote($path).'
					  AND assetid = '.$db->quote($this->id);
			$result = $db->query($sql);
			if (DB::isError($result)) {
				trigger_error($result->getMessage().'<br/>'.$result->getUserInfo(), E_USER_ERROR);
				return false;
			}
		}// end if

		unset($this->_tmp['paths']);
		unset($this->_tmp['urls']);

		if ($this->_updateWebURLs()) {
			$GLOBALS['SQ_SYSTEM']->db->query('COMMIT');
			return true;
		} else {
			$GLOBALS['SQ_SYSTEM']->db->query('ROLLBACK');
			return false;
		}

	}// end saveWebPath()

	/**
	* returns a list of urls that point to this asset
	*
	* @param boolean	$inc_urlid	return the root urlid with the url
	*
	* @return Array()
	* @access public
	*/
	function getWebURLs($inc_urlid=false) 
	{
		if (!isset($this->_tmp['urls'])) {
			$db = &$GLOBALS['SQ_SYSTEM']->db;
			$sql = 'SELECT url, root_urlid
					FROM sq_asset_url_lookup
					WHERE assetid = '.$db->quote($this->id);
			$result = $db->getAll($sql);
			if (DB::isError($result)) {
				trigger_error($result->getMessage().'<br/>'.$result->getUserInfo(), E_USER_ERROR);
				$this->_tmp['urls'] = Array();
			} else {
				$this->_tmp['urls'] = $result;
			}

		}// end if

		if ($inc_urlid) {
			return $this->_tmp['urls'];
		} else {
			$ret_val = Array();
			for($i = 0; $i < count($this->_tmp['urls']); $i++) $ret_val[] = $this->_tmp['urls'][$i]['url'];
			return $ret_val;
		}

	}// end getWebURLs()

	/**
	* Called to force an update of this assets URLs
	*
	* @return boolean
	* @access public
	*/
	function updateWebURLs() 
	{
		$GLOBALS['SQ_SYSTEM']->db->query('BEGIN');
		if ($this->_updateWebURLs()) {
			$GLOBALS['SQ_SYSTEM']->db->query('COMMIT');
			return true;
		} else {
			$GLOBALS['SQ_SYSTEM']->db->query('ROLLBACK');
			return false;
		}
	}// end updateWebURLs
			
	/**
	* Fn that does the real processing of updateWebURLs(), separated to allow for the DB transaction stuff
	* Will be called by parent assets recursively on their kids so not 'really' private
	* and by the addWebPaths() and deleteWebPaths() so that their transaction calls can wrap around it
	*
	* @return boolean
	* @access private
	*/
	function _updateWebURLs() 
	{

		$paths = $this->getWebPaths();
		$db = &$GLOBALS['SQ_SYSTEM']->db;

		unset($this->_tmp['urls']);

		// Remove all the old URLs
		$sql = 'DELETE FROM sq_asset_url_lookup
				WHERE assetid = '.$db->quote($this->id);
		$result = $db->query($sql);
		if (DB::isError($result)) {
			trigger_error($result->getMessage().'<br/>'.$result->getUserInfo(), E_USER_ERROR);
			return false;
		}

		// if we have paths then do some url inserting
		if (count($paths)) {

			$parents = $this->getLinks(SQ_LINK_NAV, '', 'S', 'minor');
			for($i = 0; $i < count($parents); $i++) {
				$parent = &$GLOBALS['SQ_SYSTEM']->am->getAsset($parents[$i]['majorid'], $parents[$i]['type_code']);
				if (is_null($parent)) continue;

				$parent_urls = $parent->getWebURLs(true);

				for($j = 0; $j < count($parent_urls); $j++) {
					for($k = 0; $k < count($paths); $k++) {
						$sql = 'INSERT INTO sq_asset_url_lookup
								(url, assetid, root_urlid)
								VALUES
								('.$db->quote($parent_urls[$j]['url'].'/'.$paths[$k]).', '.$db->quote($this->id).', '.$db->quote($parent_urls[$j]['urlid']).')';
						$result = $db->query($sql);
						if (DB::isError($result)) {
							trigger_error($result->getMessage().'<br/>'.$result->getUserInfo(), E_USER_ERROR);
							return false;
						}
					}// end for paths
				}// end for parent_urls
				
			}// end for parents

		}// end if count paths

		$kids = $this->getLinks(SQ_LINK_NAV, '', 'S', 'major');
		for($i = 0; $i < count($kids); $i++) {
			$kid = &$GLOBALS['SQ_SYSTEM']->am->getAsset($kids[$i]['minorid'], $kids[$i]['type_code']);
			if (is_null($kid)) continue;

			if (!$kid->_updateWebURLs()) {
				return false;
			}
			
		}// end for kids


		return true;

	}// end _updateWebURLs()


}//end class
?>
