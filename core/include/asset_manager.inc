<?php

/**
* Asset_Manager
*
* Purpose
*
*    Holds information on installed packages, as represented by the DB
*
* @author  Blair Robertson <blair@squiz.net>
* @version $Version$ - 1.0
* @package Resolve
*/
class Asset_Manager extends Resolve_Object
{

	/**
	* Info for all the assets types
	*
	* @var Array()
	*/
	var $_asset_types;

	/**
	* this array holds the references to the different types of objects in the system
	* so that there will only ever (with luck :) be one instance of an Asset
	*
	* @var Array(Asset)
	*/
	var $_assets = Array();

	/**
	* Constructor
	*
	*/
	function Asset_Manager()
	{
		$this->Resolve_Object();
		$this->_loadAssetTypes();
	}

	/**
	* Loads all the assets types into the asset array
	*
	* @access private
	*/
	function _loadAssetTypes()
	{
		$this->_asset_types = Array();

		$db = &$GLOBALS['SQ_SYSTEM']->db;
		$sql = 'SELECT type_code, version, name, instantiable, system_only, parent_type, dir, customisation
				FROM sq_asset_type';
		$result = $db->query($sql);
		if (DB::isError($result)) trigger_error($result->getMessage().'<br/>'.$result->getUserInfo(), E_USER_ERROR);

		$this->_asset_types = Array();
		while (null !== ($row = $result->fetchRow())) {
			$this->_asset_types[$row['type_code']] = $row;
		}//end while
		$result->free();

	}

	/**
	* Refreshes the passed asset types data in the assets array
	* Also updates the sq_asset_type_inherited table so as to have all
	* links pointing correctly
	*
	* @access public
	*/
	function refreshAssetType($type_code)
	{

		$db = &$GLOBALS['SQ_SYSTEM']->db;
		$sql = 'SELECT type_code, version, name, instantiable, system_only, parent_type, dir, customisation
				FROM sq_asset_type
				WHERE type_code = '.$db->quote($type_code);
		$result = $db->getRow($sql);
		if (DB::isError($result)) trigger_error($result->getMessage().'<br/>'.$result->getUserInfo(), E_USER_ERROR);

		if (empty($result)) {
			trigger_error('Asset "'.$type_code.'" is not installed on the system', E_USER_WARNING);
			return;
		} else {
			$this->_asset_types[$result['type_code']] = $result;
		}

		$parents = $this->getParentList($type_code);

		$sql = 'SELECT inherited_type_code
				FROM sq_asset_type_inherited
				WHERE type_code = '.$db->quote($type_code);
		$db_parents = $db->getCol($sql);
		if (DB::isError($result)) trigger_error($result->getMessage().'<br/>'.$result->getUserInfo(), E_USER_ERROR);

		$inserts = array_diff($parents, $db_parents);
		$deletes = array_diff($db_parents, $parents);

		foreach($inserts as $inherited_type_code) {
			$sql = 'INSERT INTO sq_asset_type_inherited (inherited_type_code, type_code)
					VALUES ('.$db->quote($inherited_type_code).', '.$db->quote($type_code).')';
			$result = $db->query($sql);
			if (DB::isError($result)) trigger_error($result->getMessage().'<br/>'.$result->getUserInfo(), E_USER_ERROR);
		}

		foreach($deletes as $inherited_type_code) {
			$sql = 'DELETE FROM sq_asset_type_inherited
					WHERE inherited_type_code = '.$db->quote($inherited_type_code).'
					  AND type_code = '.$db->quote($type_code);
			$result = $db->query($sql);
			if (DB::isError($result)) trigger_error($result->getMessage().'<br/>'.$result->getUserInfo(), E_USER_ERROR);
		}

	}//end refreshAssetType()

	/**
	* Returns whether the passed asset is installed in the system or not
	*
	* @param string $type_code  the code name for the asset that you want to check
	*
	* @return boolean
	* @access public
	*/
	function installed($type_code) {
		return isset($this->_asset_types[$type_code]);
	}//end installed()

	/**
	* Returns an array of type codes that are installed in the system
	*
	* @return array
	* @access public
	*/
	function getTypeList()
	{
		return array_keys($this->_asset_types);
	}//end getInfo()

	/**
	* Returns an array of asset information or a specific piece of information
	*
	* @param string $type_code  the code name for the asset that you want to check
	* @param string $field      if exists returns this information from the array for this specific field
	*
	* @return mixed
	* @access public
	*/
	function getInfo($type_code, $field='')
	{

		if (!isset($this->_asset_types[$type_code])) {
			trigger_error('Asset "'.$type_code.'" is not installed on the system', E_USER_WARNING);
			return ($field) ? null : Array();
		}

		// because we don't get the load all the information about a type, if we don't know about a field
		// get it from the DB
		if ($field && !isset($this->_asset_types[$type_code][$field])) {

			$db = &$GLOBALS['SQ_SYSTEM']->db;
			$sql = 'SELECT '.$field.'
					FROM sq_asset_type
					WHERE type_code = '.$db->quote($type_code);
			$result = $db->getOne($sql);
			if (DB::isError($result)) trigger_error($result->getMessage().'<br/>'.$result->getUserInfo(), E_USER_ERROR);

			$this->_asset_types[$type_code][$field] = ($field == 'editing_options') ? unserialize($result) : $result;

		}// end if

		return ($field) ? $this->_asset_types[$type_code][$field] : $this->_asset_types[$type_code];

	}//end getInfo()


	/**
	* Returns an array representing the class hierarchy for the asset types, either for whole system
	* or for under the the passed asset type
	*
	* @param string base_type_code	the asset type's hierarchy to return
	*
	* @return array
	* @access public
	*/
	function getAssetTypeHierarchy($base_type_code='asset', $ignore_system_only=false)
	{

		if (empty($base_type_code)) $base_type_code = 'asset';

		$type_codes = Array();
		if ($ignore_system_only) {
			$db = &$GLOBALS['SQ_SYSTEM']->db;
			$sql = 'SELECT type_code
					FROM sq_asset_type
					WHERE system_only != 1';
			$type_codes = $db->getCol($sql);
			if (DB::isError($type_codes)) trigger_error($type_codes->getMessage().'<br/>'.$type_codes->getUserInfo(), E_USER_ERROR);
		} else {
			$type_codes = array_keys($this->_asset_types);
		}

		$offspring = Array();
		foreach($type_codes as $type_code) {
			$parent = $this->_asset_types[$type_code]['parent_type'];
			if (!isset($offspring[$parent])) $offspring[$parent] = Array();
			$offspring[$parent][] = $type_code;
		}//end for

		$hier = $this->_recurseGetAssetTypeHierarchy($offspring, $base_type_code);

		return $hier;

	}// end getAssetTypeHierarchy()

	function _recurseGetAssetTypeHierarchy($offspring, $base_type_code)
	{
		if (empty($offspring[$base_type_code])) return Array();
		$arr = Array();
		for($i = 0; $i < count($offspring[$base_type_code]); $i++) {
			$type = $offspring[$base_type_code][$i];
			$arr[$type] = Array('name' => $this->_asset_types[$type]['name'],
								'subs' => $this->_recurseGetAssetTypeHierarchy($offspring, $type)
								);
		}
		return $arr;
	}


	/**
	* Includes the class file for the passed asset
	*
	* @param string $type_code  the code name for the asset that you want to check
	*
	* @access public
	*/
	function includeAsset($type_code)
	{

		if (!isset($this->_asset_types[$type_code])) {
			trigger_error('Asset "'.$type_code.'" is not installed on the system, unable to include it\'s Source File', E_USER_ERROR);
		}

		require_once SQ_SYSTEM_ROOT.'/'.$this->_asset_types[$type_code]['dir'].'/'.$type_code.'.inc';

	}//end includeAsset()

	/**
	* Returns an array of all assets types that are parents for the passed asset
	*
	* @param string		$type_code		the code name for the asset that you want to check
	* @param boolean	$include_asset	when true adds the "Asset" type to the parent list, even though it's an unistantiable object
	*
	* @access public
	*/
	function getParentList($type_code, $include_asset=true)
	{

		if (!isset($this->_asset_types[$type_code])) {
			trigger_error('Asset Type "'.$type_code.'" is not installed on the system', E_USER_ERROR);
			return Array();
		}

		$arr = Array();

		while($this->_asset_types[$type_code]['parent_type'] != 'asset') {
			// this should NEVER happen, if it does DIE
			if (!isset($this->_asset_types[$this->_asset_types[$type_code]['parent_type']])) {
				trigger_error('Asset "'.$this->_asset_types[$type_code]['parent_type'].'" is not installed on the system,<br/>'
							 .'therefore we cannot get the parent type of Asset "'.$type_code.'".<br/>'
							 .'Check that you got the Asset_Manager by reference', E_USER_ERROR);
			}
			$type_code = $this->_asset_types[$type_code]['parent_type'];
			$arr[]     = $type_code;
		}

		if ($include_asset) $arr[] = 'asset';

		return $arr;

	}//end getParentList()

	/**
	* returns a reference to the asset represented by the passed assetid
	*
	* @param int		$assetid		the asset id to be loaded
	* @param string		$type_code		if this exists then this object is used to load the asset, if not then the DB is queried to find out the asset type
	* @param boolean	$mute_errors	stops the outputting of errors in this fn, needed because you can't use the '@' operator when returning by reference
	*
	* @access public
	*/
	function &getAsset($assetid, $type_code='', $mute_errors=false)
	{

		if (!isset($this->_assets[$assetid]) || !is_object($this->_assets[$assetid])) {

			if (empty($type_code)) {
				$type_code = $GLOBALS['SQ_SYSTEM']->db->getOne('SELECT type_code FROM sq_asset WHERE assetid = '.$GLOBALS['SQ_SYSTEM']->db->quote($assetid));
				if (empty($type_code)) {
					if (!$mute_errors) trigger_error('Asset #'.$assetid.' does not exist', E_USER_WARNING);
					$this->_assets[$assetid] = null; // do this because of return by reference
					return $this->_assets[$assetid]; 
				}
			}//end if

			if (!isset($this->_asset_types[$type_code])) {
				if (!$mute_errors) trigger_error('Asset Type "'.$type_code.'" is not installed on this system', E_USER_WARNING);
				$this->_assets[$assetid] = null;
			} else {
				$this->includeAsset($type_code);
				$this->_assets[$assetid] = new $type_code($assetid);
				// make sure of the asset is correct
				if (empty($this->_assets[$assetid]->id)) $this->_assets[$assetid] = null;

			}//end if

		}//end if

		return $this->_assets[$assetid];

	}//end getAsset()

	/**
	* registers the passed object in the store, for others to get
	* used when a new record has just been "create()d"
	*
	* @param object Asset $obj  the asset to be registered
	*
	* @access public
	*/
	function registerAsset(&$obj)
	{
		// wrong class or no id, fuck off
		if (!is_object($obj) || is_a($obj, 'Asset') || !$obj->id) return;

		if (isset($this->_assets[$obj->id])) unset($this->_assets[$obj->id]);
		$this->_assets[$obj->id] = &$obj;

	}//end registerAsset();

	/**
	* unregisters the passed object from the store, 
	*
	* @param object Asset $obj  the asset to be registered
	*
	* @access public
	*/
	function unRegisterAsset(&$obj)
	{
		// wrong class or no id, fuck off
		if (!is_object($obj) || is_a($obj, 'Asset') || !$obj->id) return;
		if (isset($this->_assets[$obj->id])) unset($this->_assets[$obj->id]);

	}//end unRegisterAsset();

	/**
	* returns list of all assetids of a certain type
	*
	* @param string		$type_code		if this exists then this object is used to load the asset, if not then the DB is queried to find out the asset type
	* @param boolean	$strict			whether we are finding assets that are just a $type_code or $type_code and any of it's sub-classes
	* @param boolean	$include_type	if false returns Array(assetid, assetid, ...), if true returns returns Array(assetid => asset_type, assetid => asset_type, ...)
	* @param boolean	$mute_errors	stops the outputting of errors in this fn, needed because you can't use the '@' operator when returning by reference
	*
	* @access public
	*/
	function getTypeAssetids($type_code, $strict=true, $include_type=false, $mute_errors=false)
	{
		$db = &$GLOBALS['SQ_SYSTEM']->db;

		$sql = 'SELECT assetid'.(($include_type) ? ', type_code' : '').'
				FROM sq_asset 
				WHERE type_code = '.$db->quote($type_code);

		if (!$strict) {
			require_once SQ_FUDGE_PATH.'/db_extras/db_extras.inc';
			$sql .= '   OR type_code IN (~SQ0~)';
			$subs = Array( 'SELECT type_code
							FROM sq_asset_type_inherited
							WHERE inherited_type_code = '.$db->quote($type_code));
			$sql = db_extras_subquery($db, $sql, $subs);
		}

		$result = ($include_type) ? $db->getAssoc($sql) : $db->getCol($sql);
		if (DB::isError($result)) {
			if (!$mute_errors) trigger_error($result->getMessage().'<br/>'.$result->getUserInfo(), E_USER_WARNING);
			return Array();
		}

		return $result;

	}//end getTypeAssetids()

	/**
	* prints out the interface for viewing and customising asset types
	*
	* @param object Backend $backend
	*
	* @access public
	*/
	function printBackend(&$backend)
	{

		$o  = &$backend->out;

		if (!isset($_GET['am_section'])) $_GET['am_section'] = '';
		$o->addFormActionGetVar('am_section', $_GET['am_section']);
		$o->addHiddenField('am_form_submitted', '1');

		switch($_GET['am_section']) {
			case 'add_asset' :
				$parent = &$this->getAsset($_GET['parent_assetid']);
				if (is_null($parent)) {
					trigger_error('Parent Asset #'.$_GET['parent_assetid'].' not found', E_USER_WARNING);
					break;
				}

				// make sure that we are allowed to link this type to the parent
				if (($err_msg = $parent->canLinkToType($_GET['type_code'], $_GET['link_type'])) !== true) {
					trigger_error($err_msg, E_USER_WARNING);
					break;
				}

				$o->addFormActionGetVar('parent_assetid', $_GET['parent_assetid']);
				$o->addFormActionGetVar('pos',            $_GET['pos']);
				$o->addFormActionGetVar('type_code',      $_GET['type_code']);
				$o->addFormActionGetVar('link_type',      $_GET['link_type']);

				$this->includeAsset($_GET['type_code']);
				$asset = new $_GET['type_code']();

				// if the form has been submitted then, process it
				// and if that is successfull, then create the link to the parent asset
				if (!empty($_POST['am_form_submitted']) && $asset->processBackend()) {
					$o->addMessage(SQ_BO_MSG_NOTICE, 'New '.$this->_asset_types[$_GET['type_code']]['name'].' "'.$asset->name.'" created');
					$this->registerAsset($asset);
					if ($parent->createLink($asset, $_GET['link_type'], '', $_GET['pos'])) {
						$o->addMessage(SQ_BO_MSG_NOTICE, '"'.$asset->name.'" successfully linked to "'.$parent->name.'"');
						$o->addOnLoad('top.sidenav.reload_asset("'.$_GET['parent_assetid'].'");');
					} else {
						$o->addMessage(SQ_BO_MSG_WARNING, '"'.$asset->name.'" was not able to be linked to "'.$parent->name.'", it can be found in the trash folder');
						list($trashid) = $this->getTypeAssetids('trash_folder');
						// reload the trash folder
						$o->addOnLoad('top.sidenav.reload_asset("'.$trashid.'");');
					}

				} else {
					$asset->printBackend($backend);

				}// end if

				break;

			case 'edit_asset' :

				$o->addFormActionGetVar('assetid', $_GET['assetid']);

				$asset = &$this->getAsset($_GET['assetid']);
				if (is_null($asset)) {
					trigger_error('Asset #'.$_GET['assetid'].' not found', E_USER_WARNING);
					break;
				}

				// if the form has been submitted then, process it
				// and if that is successfull, then create the link to the parent asset
				$old_name = $asset->name;
				if (!empty($_POST['am_form_submitted']) && $asset->processBackend()) {
					$o->addMessage(SQ_BO_MSG_NOTICE, '"'.$asset->name.'" Updated');
					// if the name has changed inform the side nav
					if ($old_name != $asset->name) {
						$o->addOnLoad('top.sidenav.reload_asset("'.$asset->id.'");');
					}

				}// end if

				$asset->printBackend($backend);
				break;

			default   :
				$this->_printAssetList($o);

		}//end switch

	}//end printBackend();


	/**
	* prints out the interface for viewing and customising asset types
	*
	* @param object Backend_Outputter $o
	* @param object DB $db
	* @param string $parent  the parent asset type that to have it's kids printed
	*
	* @access public
	*/
	function _printAssetList(&$o)
	{

		require_once SQ_LIB_PATH.'/html_form/html_form.inc';

		if (!empty($_REQUEST['am_action'])) {

			require_once SQ_INCLUDE_PATH.'/custom_asset_type.inc';

			$o->addFormActionGetVar('am_action', 'edit');

			pre_echo($_POST);
			switch ($_REQUEST['am_action']) {
				case 'create_custom' :
					$custom = new Custom_Asset_Type();
					if ($custom->create($_REQUEST['am_new_type_code'], $_REQUEST['am_type_code'])) {
						$o->addFormActionGetVar('am_type_code', $_REQUEST['am_new_type_code']);
						$custom->printBackend($backend);
						return;
					}
					break;
				case 'edit' :
					$o->addFormActionGetVar('am_type_code', $_REQUEST['am_type_code']);
					$custom = new Custom_Asset_Type($_REQUEST['am_type_code']);
					$custom->printBackend($backend);
					return;
					break;
			}
		}//end if


		$o->addCrummingLink('Asset Types', $o->getCurrentLocation());
		$o->setHeading('Assets Types', 'create');
		$o->addHiddenField('am_action');
		$o->addHiddenField('am_new_type_code');

		$o->openSection('Existing Assets');
		$o->openField('', 'new_line');


		$offspring = Array();
		for (reset($this->_asset_types); null !== ($type_code = key($this->_asset_types)); next($this->_asset_types)) {
			$parent = $this->_asset_types[$type_code]['parent_type'];
			if (!isset($offspring[$parent])) $offspring[$parent] = Array();
			$offspring[$parent][] = $type_code;
		}//end for

	?>
		<script language="JavaScript" type="text/javascript">
		<!--

			var current_asset_name = '';

			function asset_checked(asset_name, customisation) {
				current_asset_name = asset_name;

				var button = get_element('edit_button');
				if (customisation) {
					button.value = 'Edit ' + asset_name;
				} else {
					button.value = '- - - - - - - - - - - - -';
				}
			}

			function create_custom() {

				if (get_element_value('am_type_code')) {

					var type_codes = new Array("<?php echo implode('", "', array_keys($this->_asset_types)); ?>");
					var new_type_code = '';
					var prompt_str = '';
					do {

						if (new_type_code == '') {
							prompt_str = 'Please Enter a unique Asset Type Code for your new customisation';
						} else {
							prompt_str = '"' + new_type_code + '" is already in use\nPlease enter a unique Asset Type Code';
						}

						new_type_code = prompt(prompt_str, new_type_code);
						// they hit cancel
						if (new_type_code == null) return false;

						// make sure it's in a proper format
						new_type_code = new_type_code.toLowerCase();
						new_type_code = new_type_code.replace(/ /g, '_');
						new_type_code = new_type_code.replace(/[^a-z_]/g, '');
						new_type_code = new_type_code.replace(/_+/g, '_');

					} while (array_search(type_codes, new_type_code) != null)

					set_hidden_field('am_new_type_code', new_type_code);
					set_hidden_field('am_action', 'create_custom');
					return true;
				}
				return false;
			}


		//-->
		</script>
		<table cellpadding="0" cellspacing="0" border="0">
			<tr>
				<td class="sq-backend-data"><img src="<?php echo $o->filesPath('/images/blank.gif'); ?>" width="1" height="20" border="0" alt="branch" /></td>
				<td class="sq-backend-data">
					&nbsp;Asset
				</td>
			</tr>
			<tr>
				<td class="sq-backend-data"><img src="<?php echo $o->filesPath('/images/blank.gif'); ?>" width="1" height="1" alt="blank" /></td>
				<td class="sq-backend-data">
			<?php
				$this->_recursePrintAssetList($o, $offspring, 'asset');
			?>
				</td>
			</tr>
			<tr>
				<td class="sq-backend-data"><img src="<?php echo $o->filesPath('/images/blank.gif'); ?>" width="1" height="1" alt="blank" /></td>
				<td class="sq-backend-data">
			<?php
				submit_button('create_button', 'Create Custom Asset', 'return create_custom();');
				submit_button('edit_button', '- - - - - - - - - - - - -', 'if (this.value.substr(0, 4) == \'Edit\') { set_hidden_field(\'am_action\', \'edit\'); return true; } else { return false };');
			?>
				</td>
			</tr>
		</table>
	<?php

	}// end printAssetList()

	function _recursePrintAssetList(&$o, &$offspring, $parent)
	{

	?>
		<table cellpadding="0" cellspacing="0" border="0">
	<?php
		$num_kids = count($offspring[$parent]);
		for ($i = 0; $i < $num_kids; $i++) {

			$type_code = $offspring[$parent][$i];
			$end = ($i == $num_kids - 1);
//			$bg_img = 'images/page_'.(($page[effective_visible]) ? '' : 'in').'visible_'.page::get_status_colour($page[effective_status]).'.gif';

			$bg = ($end) ? '' : 'background="'.$o->filesPath('/images/tree/stalk.gif').'"';
		?>
			<tr>
				<td class="sq-backend-data" <?php echo $bg; ?>><img src="<?php echo $o->filesPath('/images/tree/branch.gif');?>" width="20" height="20" border="0" alt="branch" /></td>
				<td class="sq-backend-data">
					<?php radio_button('am_type_code', $type_code, false, 'asset_checked(\''.addslashes($this->_asset_types[$type_code]['name']).'\', '.(($this->_asset_types[$type_code]['customisation']) ? 'true' : 'false').');'); ?>
					<span class="sq-backend-<?php echo ($this->_asset_types[$type_code]['customisation']) ? 'custom-' : ''?>asset"><?php echo $this->_asset_types[$type_code]['name'].' ('.$type_code.')';?></span>
				</td>
			</tr>
		<?php
			if (!empty($offspring[$type_code])) {
			?>
				<tr>
					<td class="sq-backend-data" <?php echo $bg; ?>><img src="<?php echo $o->filesPath('/images/blank.gif'); ?>" width="1" height="1" alt="blank" /></td>
					<td class="sq-backend-data">
				<?php
					$this->_recursePrintAssetList($o, $offspring, $type_code);
				?>
					</td>
				</tr>
			<?php
			}//end if

		}//end for
	?>
		</table>
	<?php

	}//end _printAssetList()

	/**
	* returns a reference to the asset that has the passed url
	* if no url is passed the current (aka PHP_SELF) one is used
	*
	* @param string		$protocol		the protocol to match
	* @param string		$url			the urls to check for
	* @param boolean	$exact_match	when true only returns an asset if there is an exact match on the url, if false searches back along the URL path for the asset that matches
	* @param boolean	$mute_errors	stops the outputting of errors in this fn, needed because you can't use the '@' operator when returning by reference
	*
	* @return object Asset | null
	* @access public
	*/
	function &getAssetFromURL($protocol=null, $url=null, $exact_match=true, $mute_errors=false)
	{

		if (is_null($url))		$url = $_SERVER['HTTP_HOST'].$_SERVER['PHP_SELF'];
		// remove any trailing slashes
		$url = preg_replace('/\/+$/', '', $url);

		if (!is_null($protocol) && $protocol != 'https') {
			$protocol = 'http';
		}

		$db = &$GLOBALS['SQ_SYSTEM']->db;

		$url_condition = '';
		if ($exact_match) {
			$url_condition = ' = '.$db->quote($url);
		
		} else {
			$sections = explode('/', $url);
			$option  = '';
			$url_condition = ' IN (';
			foreach($sections as $piece) {
				$url_condition .= (($option) ? ',' : '').$db->quote($option.$piece);
				$option .= $piece.'/';
			}
			$url_condition .= ') ';
		}// end if

		$extra_table = '';
		$extra_where = '';
		if (!is_null($protocol)) {
			$extra_table = ' INNER JOIN sq_asset_url u ON l.root_urlid = u.urlid';
			$extra_where = ' AND u.'.$protocol.' = '.$db->quote(1);
		}

		$sql = 'SELECT l.url, l.assetid
				FROM sq_asset_lookup l '.$extra_table.'
				WHERE l.url '.$url_condition.'
				'.$extra_where.'
				ORDER BY CHARACTER_LENGTH(l.url) DESC
				LIMIT 1';
		$row = $db->getRow($sql);
		if (DB::isError($row)) trigger_error($row->getMessage().'<br/>'.$row->getUserInfo(), E_USER_ERROR);

		// URL not found
		if (empty($row)) {
			$fancy_url = (($protocol) ? $protocol.'://' : '').$url;
			if (!$mute_errors) trigger_error('Unable to find an Asset for the URL "'.$fancy_url.'"', E_USER_WARNING);
			return null;

		} else {
			return $this->getAsset($row['assetid'], '', $mute_errors);
		}// end if

	}//end getAssetFromURL()

}//end class
?>
