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

	/*
	* Loads all the assets types into the asset array
	*
	* @access private
	*/
	function _loadAssetTypes()
	{

		$db = &$GLOBALS['SQ_RESOLVE']->getDb();
		$sql = 'SELECT type_code, version, name, instantiable, parent_type, dir, customisation
				FROM sq_asset_type';
		$result = $db->query($sql);
		if (DB::isError($result)) trigger_error($result->getMessage().'<br>'.$result->getUserInfo(), E_USER_ERROR);

		$this->_asset_types = Array();
		while (NULL !== ($row = $result->fetchRow())) {
			$this->_asset_types[$row['type_code']] = $row;
		}#end while
		$result->free();

	}

	/*
	* Refreshes the passed asset types data in the assets array
	* Also updates the sq_asset_type_inherited table so as to have all 
	* links pointing correctly
	*
	* @access public
	*/
	function refreshAssetType($type_code)
	{

		$db = &$GLOBALS['SQ_RESOLVE']->getDb();
		$sql = 'SELECT type_code, version, name, instantiable, parent_type, dir, customisation
				FROM sq_asset_type
				WHERE type_code = '.$db->quote($type_code);
		$result = $db->getRow($sql);
		if (DB::isError($result)) trigger_error($result->getMessage().'<br>'.$result->getUserInfo(), E_USER_ERROR);

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
		if (DB::isError($result)) trigger_error($result->getMessage().'<br>'.$result->getUserInfo(), E_USER_ERROR);

		$inserts = array_diff($parents, $db_parents);
		$deletes = array_diff($db_parents, $parents);

		foreach($inserts as $inherited_type_code) {
			$sql = 'INSERT INTO sq_asset_type_inherited (inherited_type_code, type_code)
					VALUES ('.$db->quote($inherited_type_code).', '.$db->quote($type_code).')';
			$result = $db->query($sql);
			if (DB::isError($result)) trigger_error($result->getMessage().'<br>'.$result->getUserInfo(), E_USER_ERROR);
		}

		foreach($deletes as $inherited_type_code) {
			$sql = 'DELETE FROM sq_asset_type_inherited 
					WHERE inherited_type_code = '.$db->quote($inherited_type_code).'
					  AND type_code = '.$db->quote($type_code);
			$result = $db->query($sql);
			if (DB::isError($result)) trigger_error($result->getMessage().'<br>'.$result->getUserInfo(), E_USER_ERROR);
		}
			
	}#end refreshAssetType()

	/*
	* Returns whether the passed asset is installed in the system or not
	*
	* @param string $type_code  the code name for the asset that you want to check
	*
	* @return boolean
	* @access public
	*/
	function installed($type_code) {
		return isset($this->_asset_types[$type_code]);
	}#end installed()

	/*
	* Returns an array of asset information or a specific piece of information
	*
	* @param string $type_code  the code name for the asset that you want to check
	* @param string $field      if exists returns this information from the array for this specific field
	*
	* @return mixed
	* @access public
	*/
	function getInfo($type_code, $field='') {

		if (!isset($this->_asset_types[$type_code])) {
			trigger_error('Asset "'.$type_code.'" is not installed on the system', E_USER_WARNING);
			return ($field) ? null : Array();
		}

		return ($field) ? $this->_asset_types[$type_code][$field] : $this->_asset_types[$type_code];

	}#end getInfo()

	/*
	* Includes the class file for the passed asset
	*
	* @param string $type_code  the code name for the asset that you want to check
	*
	* @access public
	*/
	function includeAsset($type_code) {

		if (!isset($this->_asset_types[$type_code])) {
			trigger_error('Asset "'.$type_code.'" is not installed on the system', E_USER_WARNING);
		}

		include_once(SQ_SYSTEM_ROOT.'/'.$this->_asset_types[$type_code]['dir'].'/'.$type_code.'.inc');

	}#end includeAsset()

	/*
	* Returns an array of all assets types that are parents for the passed asset
	*
	* @param string $type_code  the code name for the asset that you want to check
	*
	* @access public
	*/
	function getParentList($type_code) {

		if (!isset($this->_asset_types[$type_code])) {
			trigger_error('Asset "'.$type_code.'" is not installed on the system', E_USER_WARNING);
			return Array();
		}

		$arr = Array();

		while($this->_asset_types[$type_code]['parent_type'] != 'asset') { 
			# this should NEVER happen, if it does DIE
			if (!isset($this->_asset_types[$this->_asset_types[$type_code]['parent_type']])) {
				trigger_error('Asset "'.$this->_asset_types[$type_code]['parent_type'].'" is not installed on the system,<br>'
							 .'therefore we cannot get the parent type of Asset "'.$type_code.'".<br>'
							 .'Check that you got the Asset_Manager by reference', E_USER_ERROR);
			}
			$type_code = $this->_asset_types[$type_code]['parent_type'];
			$arr[]     = $type_code;
		}

		$arr[] = 'asset';

		return $arr;

	}#end getParentList()

	/*
	* returns a reference to the asset represented by the passed assetid
	*
	* @param int     $assetid      the asset id to be loaded
	* @param string  $type_code    if this exists then this object is used to load the asset, if not then the DB is queried to find out the asset type
	* @param boolean $mute_errors  stops the outputting of errors in this fn, needed because you can't use the '@' operator when returning by reference
	*
	* @access public
	*/
	function &getAsset($assetid, $type_code='', $mute_errors=false) {

		if (!isset($this->_assets[$assetid]) || !is_object($this->_assets[$assetid])) {

			if (empty($type_code)) {
				$db = &$GLOBALS['SQ_RESOLVE']->getDb();
				$type_code = $db->getOne('SELECT type_code FROM sq_asset WHERE assetid = '.$db->quote($assetid));
			}#end if

			if (empty($type_code)) {
				if (!$mute_errors) trigger_error('Asset #'.$assetid.' does not exist', E_USER_WARNING);
				$this->_assets[$assetid] = null;
			} else {

				if (!isset($this->_asset_types[$type_code])) {
					if (!$mute_errors) trigger_error('Asset Type "'.$type_code.'" is not installed on this system', E_USER_WARNING);
					$this->_assets[$assetid] = null;
				} else {
					$this->includeAsset($type_code);
					$this->_assets[$assetid] = new $type_code($assetid);
					# make sure of the asset is correct
					if (empty($this->_assets[$assetid]->id)) $this->_assets[$assetid] = null;
				}

			}#end if

		}#end if

		return $this->_assets[$assetid];

	}#end getAsset()

	/*
	* registers the passed ojbect in the store, for others to get
	* used when a new record has just been "create()d"
	*
	* @param object Asset $assetid  the asset id to be registered
	*
	* @access public
	*/
	function registerAsset(&$obj) {

		# wrong class or no id, fuck off
		if (!is_object($obj) || is_a($obj, 'Asset') || !$obj->id) return;

		$this->_assets[$obj->id] = &$obj;

	}#end registerAsset();


	/*
	* prints out the interface for viewing and customising asset types
	*
	* @param object Backend $backend
	*
	* @access public
	*/
	function printBackend(&$backend) {

		$o  = &$backend->out;

		$o->addCrummingLink('Asset Types', $o->getCurrentLocation());

		if (!empty($_REQUEST['am_action'])) {

			include_once SQ_INCLUDE_PATH.'/custom_asset_type.inc';

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
		}#end if

		$o->setHeading('Assets Types', 'create');
		$o->addHiddenField('am_action');
		$o->addHiddenField('am_new_type_code');
	
		$o->openSection('Existing Assets');
		$o->openField('', 'new_line');

		$this->_printAssetList($o);



	}#end printBackend();


	/*
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

		$offspring = Array();
		for (reset($this->_asset_types); NULL !== ($type_code = key($this->_asset_types)); next($this->_asset_types)) {
			$parent = $this->_asset_types[$type_code]['parent_type'];
			if (!isset($offspring[$parent])) $offspring[$parent] = Array();
			$offspring[$parent][] = $type_code;
		}#end for

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
					
					var type_codes = new Array("<?=implode('", "', array_keys($this->_asset_types))?>");
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
				<td class="sq-backend-data"><img src="<?=$o->filesPath('/images/blank.gif');?>" width="1" height="20" border="0" alt="branch" /></td>
				<td class="sq-backend-data">
					&nbsp;Asset
				</td>
			</tr>
			<tr>
				<td class="sq-backend-data"><img src="<?=$o->filesPath('/images/blank.gif');?>" width="1" height="1" alt="blank" /></td>
				<td class="sq-backend-data">
			<?
				$this->_recursePrintAssetList($o, $offspring, 'asset');
			?> 
				</td>
			</tr>
			<tr>
				<td class="sq-backend-data"><img src="<?=$o->filesPath('/images/blank.gif');?>" width="1" height="1" alt="blank" /></td>
				<td class="sq-backend-data">
			<?
				$o->submit_button('create_button', 'Create Custom Asset', 'return create_custom();');
				$o->submit_button('edit_button', '- - - - - - - - - - - - -', 'if (this.value.substr(0, 4) == \'Edit\') { set_hidden_field(\'am_action\', \'edit\'); return true; } else { return false };');
			?> 
				</td>
			</tr>
		</table>
	<?


	}

	function _recursePrintAssetList(&$o, &$offspring, $parent)
	{

	?> 
		<table cellpadding="0" cellspacing="0" border="0">
	<?
		$num_kids = count($offspring[$parent]);
		for ($i = 0; $i < $num_kids; $i++) {

			$type_code = $offspring[$parent][$i];
			$end = ($i == $num_kids - 1);
//			$bg_img = 'images/page_'.(($page[effective_visible]) ? '' : 'in').'visible_'.page::get_status_colour($page[effective_status]).'.gif';

			$bg = ($end) ? '' : 'background="'.$o->filesPath('/images/tree/stalk.gif').'"';
		?> 
			<tr>
				<td class="sq-backend-data" <?=$bg?>><img src="<?=$o->filesPath('/images/tree/branch.gif');?>" width="20" height="20" border="0" alt="branch" /></td>
				<td class="sq-backend-data">
				<? 
				$o->radio_button('am_type_code', $type_code, false, 'asset_checked(\''.addslashes($this->_asset_types[$type_code]['name']).'\', '.(($this->_asset_types[$type_code]['customisation']) ? 'true' : 'false').');');
				?>
					<span class="sq-backend-<?=($this->_asset_types[$type_code]['customisation']) ? 'custom-' : ''?>asset"><?=$this->_asset_types[$type_code]['name'].' ('.$type_code.')';?></span>
				</td>
			</tr>
		<?
			if (!empty($offspring[$type_code])) {
			?> 
				<tr>
					<td class="sq-backend-data" <?=$bg?>><img src="<?=$o->filesPath('/images/blank.gif');?>" width="1" height="1" alt="blank" /></td>
					<td class="sq-backend-data">
				<?
					$this->_recursePrintAssetList($o, $offspring, $type_code);
				?> 
					</td>
				</tr>
			<?
			}#end if

		}#end for
	?> 
		</table>
	<?

	}#end _printAssetList()


}#end class
?>
