<?php

require_once SQ_LIB_PATH.'/xml_array/xml_array.inc';
require_once SQ_INCLUDE_PATH.'/asset_management.inc';

/**
* Package_Manager
*
* Purpose
*    On a per package basis this object let's the system install and upgrade
*    packages and their assets, uses info gleened from the package.xml and asset.xml files
*
* @author  Blair Robertson <blair@squiz.net>
* @version $Version$ - 1.0
*/
class Package_Manager extends Resolve_Object
{

	/**
	* The name of the package that we are managing
	*
	* @var string
	*/
	var $package;

	/**
	* The info that is relevant to this package
	*
	* @var string
	*/
	var $info;

	/**
	* The name of the package that we are managing
	*
	* @var string
	*/
	var $assets;

	/**
	* the path to this package's directory from the system root
	*
	* @var string
	*/
	var $_base_path;

	/**
	* the full system path to this package's directory
	*
	* @var string
	*/
	var $_full_path;

	/**
	* Constructor
	*
	*/
	function Package_Manager($package) 
	{
		$this->Resolve_Object();
	
		// OK, if this is the "core" package then we just need to fiddle with the path
		$this->_base_path = ($package == '__core__') ? 'core/assets' : 'packages/'.$package;
		$this->_full_path = SQ_SYSTEM_ROOT.'/'.$this->_base_path;

		if (file_exists($this->_full_path.'/package.xml')) {
			$xml = new XML_Array();
			$info = $xml->getArrayFromFile($this->_full_path.'/package.xml');
			if (PEAR::isError($info)) {
				trigger_error($info->error_message_prefix.$info->message, E_USER_WARNING);
				return;
			}
			$xml->close();
			if ($package != $info['code_name']['value']) {
				trigger_error('The code name for the package in the file "'.$this->_full_path.'/package.xml" does not match the package that we are trying to open ('.$package.').', E_USER_WARNING);
				return;
			}
			$this->info = Array(
								'name'        => $info['name']['value'],
								'version'     => $info['version']['value'],
								'description' => $info['description']['value']
								);
			$this->package = $package;
			$this->_loadPackageAssets();

		} else {
			trigger_error('Package "'.$package.'" does not have a package.xml file', E_USER_WARNING);
			return;
		}

	}//end constructor

	/*
	* Checks to see if the installation/upgrade of this package is able to go ahead.
	*
	* @return boolean
	* @access public
	*/
	function checkPackageDetails() {

		if (!$this->package) {
			trigger_error('Package is not available for install or upgrade on this system', E_USER_WARNING);
			return false;
		}

		$am = &$GLOBALS['SQ_RESOLVE']->am;

		for(reset($this->assets); null !== ($type_code = key($this->assets)); next($this->assets)) {

			if (!isset($install_reqs[$type_code])) $install_reqs[$type_code] = Array();

			// make sure all requirements are met
			$file = SQ_SYSTEM_ROOT.'/'.$this->assets[$type_code]['dir'].'/asset.xml';
			list($result, $req) = $this->checkRequirementsFromFile($file, true);

			if (!$result) {
				trigger_error('Asset "'.$type_code.'" fails the <b>'.$req['type'].'</b> requirement for <b>'.$req['code'].' v'.$req['version'].'</b><br>As specified by '.$file, E_USER_WARNING);
				return false;
			}//endif

			// OK, let's check that this asset is of an available type, 
			// or that the type is in this package that we are checking
			$parent_type = $this->assets[$type_code]['parent_type'];
			if ($parent_type != 'asset' && !$am->installed($parent_type) && !isset($this->assets[$parent_type])) {
				trigger_error('Asset "'.$type_code.'" attempts to inherit from the unknown type "'.$parent_type.'"<br>As specified by '.$file, E_USER_WARNING);
				return false;
			}//endif

		}//end for

		return true;

	}//end checkPackageDetails()

	/*
	* Installs/Updates the passed package in the system
	*
	* @access public
	*/
	function updatePackageDetails() {

		if (!$this->package) {
			trigger_error('Package is not available for install or upgrade on this system', E_USER_WARNING);
			return;
		}

		if (!$this->checkPackageDetails()) {
			return;
		}

		$install_order = $this->_getInstallOrder();
		if (empty($install_order)) {
			trigger_error('No assets available in the install order, aborting update of <b>'.$this->package.'</b> package.', E_USER_WARNING);
			return;
		}

		pre_echo('Install Order:');
		pre_echo($install_order);

		$db = &$GLOBALS['SQ_RESOLVE']->db;
		// open the transaction
		$db->query('BEGIN');

		$exists = $db->getOne('SELECT code_name FROM sq_package WHERE code_name = '.$db->quote($this->package));
		if (DB::isError($exists)) trigger_error($exists->getMessage().'<br>'.$exists->getUserInfo(), E_USER_ERROR);

		if ($exists) {
			$sql = 'UPDATE sq_package
					SET version     = '.$db->quote($this->info['version']).',
						name        = '.$db->quote($this->info['name']).',
						description = '.$db->quote($this->info['description']).'
					WHERE code_name = '.$db->quote($this->package);
		} else {
			$sql = 'INSERT INTO sq_package
					(code_name, version, name, description)
					VALUES
					('.$db->quote($this->package).', '.$db->quote($this->info['version']).',
					'.$db->quote($this->info['name']).','.$db->quote($this->info['description']).')';
		}//end if

#		pre_echo($sql);
		$result = $db->query($sql);
		if (DB::isError($result)) {
			// Abort Transaction
			$db->query('ROLLBACK');
			trigger_error($result->getMessage().'<br>'.$result->getUserInfo(), E_USER_WARNING);
			return;
		}

		foreach($install_order as $type_code) {

#			pre_echo('Updating : '.$type_code);
			// now we simply call the management class for this object
			$file = SQ_SYSTEM_ROOT.'/'.$this->assets[$type_code]['dir'].'/'.$type_code.'_management.inc';
			@include_once($file);
			$class = $type_code.'_management';
			// make sure the class exists
			if (class_exists($class)) {
				// now just create an instance of it, and run update()
				$management = new $class($this);
				if (!$management->update()) {
					// Abort Transaction
					$db->query('ROLLBACK');
					return false;
				}

			} else {
				trigger_error('Asset "'.$type_code.'" does not have a management class defined for it in '.$file, E_USER_WARNING);
				// Abort Transaction
				$db->query('ROLLBACK');
				return false;
			}

		}//end for

		// Commit the transaction if we got this far
		$db->query('COMMIT');
		return true;

	}//end updatePackageDetails()
	
	/*
	* Recursivly searches through the passed package's directory for asset.xml files
	* and enter's them into the $this->assets array with the details
	*
	* @access private
	*/
	function _loadPackageAssets()
	{
		$this->_recurseLoadPackageAssets($this->assets, $this->_full_path, $this->_base_path);
	}

	function _recurseLoadPackageAssets(&$assets, $full_path, $base_path)
	{
		$d = dir($full_path);
		while (false !== ($entry = $d->read())) {
			if ($entry == '.' || $entry == '..') continue;

			// if this is a directory, recursivly search it
			if (is_dir($full_path.'/'.$entry)) {
				$this->_recurseLoadPackageAssets($assets, $full_path.'/'.$entry, $base_path.'/'.$entry);

			// if we have an asset process it
			} elseif ($entry == 'asset.xml') {

				$info = Asset_Management::getAssetInfo($full_path.'/asset.xml');
				if (empty($info)) {
					continue;
				}

				if ($this->package != $info['package']) {
					trigger_error('The Asset Information file "'.$full_path.'/asset.xml" is in the '.$this->package.' Package directory, but states that it is belongs to '.$info['package'], E_USER_ERROR);
				}

				$info['dir'] = $base_path;

				$assets[$info['type_code']] = $info;

			}//end if
		}//end while
		$d->close();

	}//end _recurseLoadPackageAssets()


	/*
	* Generates an array holding the type_codes for the order in which
	* the assets should be installed in order to respect the heirarchy
	* defined in the asset.xml files
	*
	* @access private
	*/
	function _getInstallOrder() {

		$asset_types = array_keys($this->assets);
		$install_order = Array();

		while(count($asset_types)) {

#			pre_echo('---- Start While : '.$x.'----');

			$last_count = count($asset_types);

			for($i = 0; $i < count($asset_types); $i++) {

				$type_code = $asset_types[$i];
				$parent    = $this->assets[$type_code]['parent_type'];

#				pre_echo('---- Start For : '.$i.', '.$type_code.', '.$parent.' ----');
#				pre_echo($asset_types);
#				pre_echo($install_order);

				$remove = false;

				// if this type is inheritted from an existing type, then all OK
				// but not if this type is in this package
				if (($parent == 'asset' || $GLOBALS['SQ_RESOLVE']->am->installed($parent)) && !in_array($parent, $asset_types)) {
#					pre_echo('Asset Installed');
					$install_order[] = $type_code;
					$remove = true;

				// if this type is inheriting from a type that will be installed before it, then all OK
				} elseif (in_array($parent, $install_order)) {
#					pre_echo('Asset to be installed before');
					$install_order[] = $type_code;
					$remove = true;

				
				// if this type is not inheriting from a type that is yet to be dealt with, then 
				// the type is in heriting from something that doesn't exist
				} elseif (!in_array($parent, $asset_types)) {
					trigger_error('Asset "'.$type_code.'" attempts to inherit from the unknown type "'.$parent.'"', E_USER_WARNING);
					$remove = true;
				}
				// else we do nothing with the asset_type 

				if ($remove) {
					unset($asset_types[$i]);
					$asset_types = array_values($asset_types);
					$i--;
				}

			}//end for

			if ($last_count == count($asset_types)) {
				trigger_error('There seems to be the possibility of recursive inheriting for this <b>'.$this->package.'</b> package.<br>The install order for these assets could not be established: <b>'.implode(', ', $asset_types).'</b>', E_USER_WARNING);
				return Array();
			}


		}//end while

		return $install_order;

	}//end _getInstallOrder()

	/*
	* checks the requirements of the passed asset.xml file
	* see checkRequirements() for more info 
	*
	* @param string  $file              the code name for the asset that you want to check
	* @param boolean $need_feedback  
	* @param array   $ignore_assets     if the requirement is an asset, but is in this list, the requirement is considered to have passed  
	*
	* @return mixed
	* @access public
	*/
	function checkRequirementsFromFile($file, $need_feedback=false) {

		$xml_array = new XML_Array(Asset_Management::getAssetArrayPaths());
		$info = $xml_array->getArrayFromFile($file);
		if (PEAR::isError($info)) {
			trigger_error($info->error_message_prefix.$info->message, E_USER_WARNING);
			return ($need_feedback) ? Array(false, null) : false;
		}

		// if there are no requirements, return true
		if (!isset($info['requirements']['requirement']) || !is_array($info['requirements']['requirement'])) {
			return ($need_feedback) ? Array(true, null) : true;
		}

		for($i = 0; $i < count($info['requirements']['requirement']); $i++) {
			$req = &$info['requirements']['requirement'][$i];

			$ok = false;
			switch($req['@type']) {
				case 'package' :
					$installed_version = $GLOBALS['SQ_RESOLVE']->db->getOne('SELECT version FROM sq_package WHERE code_name = '.$GLOBALS['SQ_RESOLVE']->db->quote($req['code']['value']));
					if (DB::isError($installed_version)) trigger_error($installed_version->getMessage().'<br>'.$installed_version->getUserInfo(), E_USER_ERROR);

					if ($installed_version) { 
						$ok = version_compare($installed_version, $req['version']['value'], '>=');
					}
					break;

				case 'asset' :
					if ($GLOBALS['SQ_RESOLVE']->am->installed($req['code']['value'])) { 
						$ok = version_compare($GLOBALS['SQ_RESOLVE']->am->getInfo($req['code']['value'], 'version'), $req['version']['value'], '>=');
					}
					// if the test failed, but this asset is in this package, check it's version from that
					if (!$ok && isset($this->assets[$req['code']['value']])) {
						$ok = version_compare($this->assets[$req['code']['value']]['version'], $req['version']['value'], '>=');
					}
					break;

				case 'system' :
					switch($req['code']['value']) {
						case 'Resolve' :
							$ok = version_compare(SQ_RESOLVE_VERSION, $req['version']['value'], '>=');
							break;
						case 'PHP' :
							$ok = version_compare(PHP_VERSION, $req['version']['value'], '>=');
							break;
					}
					break;

			}//end switch

			// this requirement failed
			if (!$ok) {

				if ($need_feedback) {
					$ret_arr = Array(
									'type'    => $req['@type'],
									'code'    => $req['code']['value'],
									'version' => $req['version']['value']
									);
					return Array(false, $ret_arr);

				} else {
					return false;

				}//end if

			}//end if


		}//end for

		// if we got this far everything is OK
		return ($need_feedback) ? Array(true, null) : true;

	}//end checkRequirementsFromFile()

	/*
	* Returns an array of all assets types that are parents for the passed asset
	* that is in the this package
	*
	* @param string $type_code  the code name for the asset that you want to check
	*
	* @access public
	*/
	function getParentList($type_code) {

		if (!isset($this->assets[$type_code])) {
			trigger_error('Asset "'.$type_code.'" does not exist in the <b>'.$this->package.'</b> package', E_USER_ERROR);
			return Array();

		}

		$arr = Array();

		while(isset($this->assets[$type_code]) && $this->assets[$type_code]['parent_type'] != 'asset') { 
			$type_code = $this->assets[$type_code]['parent_type'];
			$arr[]     = $type_code;
		}

		// if we are inheriting from something else then go find it from the installed list
		if ($this->assets[$type_code]['parent_type'] != 'asset') {
			$new_arr = $GLOBALS['SQ_RESOLVE']->am->getParentList($this->assets[$type_code]['parent_type']);
			foreach($new_arr as $type_code) $arr[] = $type_code;
		} else {
			$arr[] = 'asset';
		}

#		pre_echo($arr);

		return $arr;

	}//end getParentList()


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

		if (!isset($this->assets[$type_code])) {
			trigger_error('Asset "'.$type_code.'" does not exist in the <b>'.$this->package.'</b> package', E_USER_WARNING);
			return ($field) ? null : Array();
		}

		return ($field) ? $this->assets[$type_code][$field] : $this->assets[$type_code];

	}//end getInfo()

}//end class
?>
