<?php

/**
* Asset
*
* Purpose
*
*
* @author  Blair Robertson <blair@squiz.net>
* @version $Version$ - 1.0
*/
class Asset extends Resolve_Object
{

	/**
	* The unique ID for this asset
	*
	* @var Array()
	*/
	var $id;

	/**
	* The Vars that belong to this asset
	*
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
		}
	}

	/*
	* Create this asset
	*
	* @return boolean
	* @access public
	*/
	function create()
	{

		$db = &$GLOBALS['SQ_RESOLVE']->getDb();

		# welcome to sequences, they are cool, see the PEAR::DB docs for more info
		$assetid = $db->nextid('sq_sequence_asset');

		$sql = 'INSERT INTO sq_asset
				(assetid, type_code, last_updated, last_userid)
				VALUES
				('.$db->quote($assetid).','.$db->quote(get_class($this)).', now(),'.$db->quote($this->currentUserId()).')';

		pre_echo($sql);

		$result = $db->query($sql);
		if (DB::isError($result)) {
			trigger_error($result->getMessage().'<br>'.$result->getUserInfo(), E_USER_WARNING);
			return false;
		}

		$this->id   = $assetid;

		$this->_loadVars($db);

		return true;

	}#end create()


	/*
	* Load the asset represented by the passed asset id
	*
	* @param int    $assetid      the asset id to be loaded
	*
	* @access public
	*/
	function load($assetid)
	{

		$db = &$GLOBALS['SQ_RESOLVE']->getDb();

		# OK, the first thing to do is check we actually exist :)

		$sql = 'SELECT assetid, type_code, last_updated, last_userid
				FROM sq_asset
				WHERE assetid = '.$db->quote($assetid);
		$result = $db->getRow($sql, NULL, DB_FETCHMODE_ORDERED);
		if (DB::isError($result)) trigger_error($result->getMessage().'<br>'.$result->getUserInfo(), E_USER_ERROR);

		if (is_null($result)) {
			trigger_error('Asset #'.$assetid.' does not exist', E_USER_WARNING);
			return;
		}

		list($this->id, $type_code, $this->last_updated, $this->last_userid) = $result;
		unset($result);

		# make sure the asset we are loading is of the same type as our class
		if ($type_code != get_class($this)) {
			trigger_error('Asset #'.$assetid.' is not a '.get_class($this), E_USER_WARNING);
			$this->id   = null;
			$this->last_updated = null;
			$this->last_userid  = null;
			return;
		}

		$this->_loadVars($db);

	}#end load()


	/*
	* Loads the vars for this asset into the vars array
	*
	* @param object DB $db    a reference to the db object
	*
	* @access public
	*/
	function _loadVars(&$db) {

		if (!$this->id) return;

		# Right, now we need to get any values that this asset has customised
		$this->vars = Array();

		$sql = 'SELECT a.attributeid, a.name, v.value
				FROM sq_asset_attribute a, sq_asset_attribute_value v
				WHERE a.attributeid = v.attributeid
				  AND v.assetid = '.$db->quote($this->id);

		$result = $db->query($sql);
		if (DB::isError($result)) trigger_error($result->getMessage().'<br>'.$result->getUserInfo(), E_USER_ERROR);


		$vars_str = '';
		while (NULL !== ($row = $result->fetchRow())) {
			$vars_str .= (($vars_str) ? ',' : '').$db->quote($row['name']);
			$this->vars[$row['name']] = Array('attributeid' => $row['attributeid'], 'value' => $row['value']);
		}#end while
		$result->free();

		$parents = $this->getParentList();
		$parents[] = get_class($this);
		# OK, because of the way things work we now need to get any vars that this asset has
		# or has inherited but has not customised - ie the default values
		# NOTE: we specifically don't get defaults for vars we have customised
		$parents_str = '';
		foreach($parents as $i) $parents_str .= (($parents_str) ? ',' : '').$db->quote($i);
		$sql = 'SELECT a.attributeid, a.name, d.value
				FROM sq_asset_attribute a, sq_asset_attribute_default d, sq_asset_type t
				WHERE a.attributeid = d.attributeid
				  AND d.type_code = t.type_code '
				.(($parents_str) ? ' AND t.type_code IN ('.$parents_str.') ' : '')
				.(($vars_str)    ? ' AND a.name  NOT IN ('.$vars_str.') '    : '')
				.'ORDER BY a.attributeid, t.level';

		$result = $db->query($sql);
		if (DB::isError($result)) trigger_error($result->getMessage().'<br>'.$result->getUserInfo(), E_USER_ERROR);

		while (NULL !== ($row = $result->fetchRow())) {
			$this->vars[$row['name']] = Array('attributeid' => $row['attributeid'], 'value' => $row['value']);
		}#end while
		$result->free();

	}#end _loadVars()

	/*
	* Returns an array of all assets types that this asset extends from
	*
	* @param string $code_name  the code name for the asset that you want to check
	*
	* @access public
	*/
	function getParentList() {

		return $GLOBALS['SQ_RESOLVE']->am->getParentList(get_class($this));

	}#end getParentList()


	/*
	* Return all links to this asset 
	*
	* @param int    $link_types  integer that can be the product of bitwise operations on the SQ_LINK_* constants
	* @param string $asset_type  the type of asset that is linked (eg 'Page', 'File', etc);
	*
	* @return Array()
	* @access public
	*/
	function getLinks($link_types, $asset_type='') {

		$elem_types = bit_elements($link_types);
		// OK, let's check what we already have cached, and only do a DB call for what we need
		$search_link_types = 0;
		foreach($elem_types as $link_type) {
			# if we haven't got all the links yet or we have got the specific type of links
			if (empty($this->_tmp['links'][$link_type.'___GOT_ALL___'])
				|| ($asset_type && !isset($this->_tmp['links'][$link_type][$asset_type]))) {
				$search_link_types |= $link_type;
			}
		}

		if ($search_link_types) {

			$db = &$GLOBALS['SQ_RESOLVE']->getDb();
			$sql = 'SELECT l.minorid, l.value, l.last_updated, l.last_userid, l.link_type, a.type_code
					FROM sq_asset_link l, sq_asset a
					WHERE l.minorid   = a.assetid 
					  AND l.majorid   = '.$db->quote($this->id).'
					  AND (link_type & '.$db->quote($search_link_types).') > 0 '
					.(empty($asset_type) ? 'ORDER BY a.type_code' : ' AND a.type_code = '.$db->quote($asset_type));

//			pre_echo($sql);

			$result = $db->query($sql);
			if (DB::isError($result)) 
				trigger_error($result->getMessage().'<br>'.$result->getUserInfo(), E_USER_ERROR);

			$this->_tmp['links'][$link_type] = Array();
			while (NULL !== ($row = $result->fetchRow())) {
				$link_type = $row['link_type'];
				$type_code = $row['type_code'];
				if (!isset($this->_tmp['links'][$link_type][$type_code])) $this->_tmp['links'][$link_type][$type_code] = Array();

				unset($row['link_type']);
				unset($row['type_code']);
				$this->_tmp['links'][$link_type][$type_code][] = $row;

			}#end while
			$result->free();

			// a bit of integrity
			if (empty($asset_type)) {

				foreach($elem_types as $link_type) $this->_tmp['links'][$link_type.'___GOT_ALL___'] = true;
			} else {
				foreach($elem_types as $link_type) {
					if (!isset($this->_tmp['links'][$link_type][$asset_type])) {
						// cause we are just need to set this to something, 
						// no need to waste space with an array
						$this->_tmp['links'][$link_type][$asset_type] = false;
					}
				}
			}

		}#end if

		$ret_arr = Array();
		foreach($elem_types as $link_type) {
			if (empty($asset_type) && !empty($this->_tmp['links'][$link_type])) {
				$ret_arr = array_merge($ret_arr, $this->_tmp['links'][$link_type]);
			} elseif (!empty($this->_tmp['links'][$link_type][$asset_type])) {
				$ret_arr = array_merge($ret_arr, $this->_tmp['links'][$link_type][$asset_type]);
			}
		}

		return $ret_arr;

	}#end getLinks()


	/*
	* Returns the number of links from this asset
	*
	* @param int    $link_types  integer that can be the product of bitwise operations on the SQ_LINK_* constants
	* @param string $asset_type  the type of asset that is linked (eg 'Page', 'File', etc);
	*
	* @return int
	* @access public
	*/
	function countLinks($link_types, $asset_type='') {

			$db = &$GLOBALS['SQ_RESOLVE']->getDb();
			if (empty($asset_type)) {
				$sql = 'SELECT count(*)
						FROM sq_asset_link l
						WHERE l.majorid   = '.$db->quote($this->id).'
						  AND (link_type & '.$db->quote($link_types).') > 0';
			} else {
				$sql = 'SELECT count(*)
						FROM sq_asset_link l, sq_asset a
						WHERE l.minorid   = a.assetid 
						  AND l.majorid   = '.$db->quote($this->id).'
						  AND (link_type & '.$db->quote($link_types).') > 0 
						  AND a.type_code = '.$db->quote($asset_type);
			}

//			pre_echo($sql);

			$result = $db->getOne($sql);
			if (DB::isError($result)) 
				trigger_error($result->getMessage().'<br>'.$result->getUserInfo(), E_USER_ERROR);
			
			return $result;

	}#end countLinks()
	
	/*
	* Called when the links are changed for this asset so that the tmp cache can be cleared
	*
	* @param string $link_type    the type of link this is
	* @param string $asset_type   the type of asset that is linked (eg 'Page', 'File', etc);
	*
	* @access private
	*/
	function _linkChanged($link_type, $asset_type='') {

		if (empty($asset_type)) {
			unset($this->_tmp['links'][$link_type]);
		} else {
			# only unset this specific type so calls for any other type 
			# can used the cached version if it exists
			unset($this->_tmp['links'][$link_type][$asset_type]);
		}
		
		unset($this->_tmp['links'][$link_type.'___GOT_ALL___']);

	}#end _linkChanged()

	/*
	* Creates a link between this asset and the passed asset, of a certain type
	*
	* @param object Asset $minor  the minor asset that we are linking to
	* @param string $link_type    the type of link this is
	* @param string $value        the value that is to be associated with this link
	*
	* @return boolean
	* @access public
	*/
	function createLink(&$minor, $link_type, $value='') {

		if (!is_a($minor, 'Asset')) {
			trigger_error('Minor is not an asset, unable to create link', E_USER_WARNING);
			return false;
		}


		# get the minor assets parents and add it's type to the front of the indexed array
		$types = $minor->getParentList();
		array_unshift($types, get_class($minor));

		$allowed_links = $this->_getAllowedLinks();

		# we will be ascending up the parent tree from the current asset type.
		# that way the major asset can have specific cardinality for different assets types
		# EG, assets => 'M', user => '1' -> this means that many assets can be linked to this this asset
		#     and only one user, but because 'user' is an 'asset' we need to check for any 'user' references
		#     bfore we check for any 'asset' references
		$type = '';
		for($i = 0; $i < count($types); $i++) {
			$type = $types[$i];
			if (!empty($allowed_links[$link_type][$type])) break;
		}

		if (empty($allowed_links[$link_type][$type])) {
			trigger_error('Assets of Type "'.$type.'" cannot be "'.$link_type.'" linked to a "'.get_class($this).'"', E_USER_WARNING);
			return false;
		}

		$current_links = $this->getLinks($link_type, $type);
		$num_curr_links = count($current_links);

		# First, we should check that we don't already have a link of this type
		for($i = 0; $i < $num_curr_links; $i++) {
			# it already exists, so we don't need to do anything
			if ($current_links[$i]['minorid'] == $minor->id && $current_links[$i]['value'] == $value) {
				return true;
			}
		}#end for

		### Now we need to check that the cardinality is kept valid

		# if we are only allowed one of these links and we already have our one link, bugger off
		if ($allowed_links[$link_type][$type] == 1 && $num_curr_links > 0) {
			trigger_error('Assets of Type "'.get_class($this).'" can only have one "'.$link_type.'" link to a "'.$type.'"', E_USER_WARNING);
			return false;
		}#end if

		$db = &$GLOBALS['SQ_RESOLVE']->getDb();


		# if this link is either an exclusive or a unite link, then we need to make sure that this
		# minor asset doesn't already have an exclusive link from anything
		if ($link_type & (SQ_LINK_EXCLUSIVE | SQ_LINK_UNITE)) {
			$sql = 'SELECT majorid
					FROM sq_asset_link
					WHERE minorid   = '.$db->quote($minor->id).'
					  AND link_type = '.$db->quote(SQ_LINK_EXCLUSIVE);
//			pre_echo($sql);


			$majorid = $db->getOne($sql);
			if (DB::isError($majorid)) {
				trigger_error($majorid->getMessage().'<br>'.$majorid->getUserInfo(), E_USER_WARNING);
				return false;
			}

			if ($majorid) {
				$major = &$GLOBALS['SQ_RESOLVE']->am->getAsset($majorid);
				trigger_error('Asset "'.$minor->name().'" (Asset #'.$minor->id.') cannot be linked to "'.$this->name().'" (Asset #'.$this->id.') it is already exclusively linked to Asset #'.$majorid.' ("'.$major->name().'")', E_USER_WARNING);
				return false;
			}
			
		}#end if

		$sql = 'INSERT INTO sq_asset_link
				(majorid, minorid, link_type, value, last_updated, last_userid)
				VALUES
				('.$db->quote($this->id).', '.$db->quote($minor->id).', '.$db->quote($link_type).', '.$db->quote($value).', NOW(), '.$db->quote($this->currentUserId()).')';

		pre_echo($sql);

		$result = $db->query($sql);
		if (DB::isError($result)) {
			trigger_error($result->getMessage().'<br>'.$result->getUserInfo(), E_USER_WARNING);
			return false;
		}

		$this->_linkChanged($link_type, get_class($minor));

		return true;

	}#end createLink()


	/*
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
	function _getAllowedLinks() {

		return Array(SQ_LINK_EXCLUSIVE => Array(),
					 SQ_LINK_UNITE     => Array(),
					 SQ_LINK_NOTICE    => Array()
					);

	}#end _getAllowedLinks()


	/*
	* Set the value for the passed variable to the passed value
	*
	* @access public
	*/
	function setAttribute($name, $value) {

		if (empty($this->vars[$name])) {
			trigger_error('Attribute "'.$name.'" not found for Asset of type "'.get_class($this).'"', E_USER_WARNING);
			return false;
		}

		if ($this->vars[$name]['value'] == $value) {
			return true;
		}

		trigger_error('Validate value of attribute in setAttribute()', E_USER_WARNING);


		$db = &$GLOBALS['SQ_RESOLVE']->getDb();
		$attributeid = $this->vars[$name]['attributeid'];

		# delete previous entry, if any
		$sql = 'DELETE FROM sq_asset_attribute_value
				WHERE assetid = '.$db->quote($this->id).'
				  AND attributeid = '.$db->quote($attributeid);

//		pre_echo($sql);
		$result = $db->query($sql);
		if (DB::isError($result)) {
			trigger_error($result->getMessage().'<br>'.$result->getUserInfo(), E_USER_WARNING);
			return false;
		}

		# now insert new entry
		$sql = 'INSERT INTO sq_asset_attribute_value
				(assetid, attributeid, value)
				VALUES
				('.$db->quote($this->id).', '.$db->quote($attributeid).', '.$db->quote($value).')';

		pre_echo($sql);

		$result = $db->query($sql);
		if (DB::isError($result)) {
			trigger_error($result->getMessage().'<br>'.$result->getUserInfo(), E_USER_WARNING);
			return false;
		}

		$this->vars[$name]['value'] = $value;

		return true;

	}#end setAttribute()

	/*
	* Returns a name to use to describe this instance of this asset
	* should be overridden
	*
	* @access public
	*/
	function name() {
		return ucwords(str_replace('_', ' ', get_class($this))).' #'.$this->id;
	}#end name();

	/*
	* easy way to get the value of an attribute
	*
	* @access public
	*/
	function attr($name) {

		if (empty($this->vars[$name])) {
			trigger_error('Attribute "'.$name.'" not found for Asset"'.get_class($this).'"', E_USER_WARNING);
			return null;
		}

		return $this->vars[$name]['value'];

	}#end attr();

}#end class
?>
