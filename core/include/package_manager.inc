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
* $Id: package_manager.inc,v 1.56 2004/11/04 01:20:21 mnyeholt Exp $
* $Name: not supported by cvs2svn $
*/


require_once SQ_LIB_PATH.'/xml_array/xml_array.inc';
require_once SQ_INCLUDE_PATH.'/asset_management.inc';
require_once SQ_INCLUDE_PATH.'/asset_edit_interface.inc';

/**
* Package_Manager
*
* Purpose
*    On a per package basis this object let's the system install and upgrade
*    packages and their assets, uses info gleened from the package.xml and asset.xml files
*
* @author  Blair Robertson <blair@squiz.net>
* @version $Version$ - 1.0
* @package MySource_Matrix
*/
class Package_Manager extends MySource_Object
{

	/**
	* The Name of the package that we are managing
	*
	* @var string
	*/
	var $name = '';

	/**
	* The Version of the package that we are managing
	*
	* @var string
	*/
	var $version = '';

	/**
	* The Description of the package that we are managing
	*
	* @var string
	*/
	var $description = '';

	/**
	* An Array of information on each asset in this package gleened from the asset.xml files
	*
	* @var Array()
	*/
	var $assets = Array();

	/**
	* the path to this package's directory from the system root
	*
	* @var string
	*/
	var $_base_path = '';

	/**
	* the full system path to this package's directory
	*
	* @var string
	*/
	var $_full_path = '';


	/**
	* Constructor
	*
	*/
	function Package_Manager()
	{
		$this->MySource_Object();

		if (!isset($this->_full_path) || !is_dir($this->_full_path)) {
			trigger_error('Unable to construct "'.get_class($this).'" because there is no _full_path set', E_USER_ERROR);
		}

		// make sure that this package is in our system rootm, otherwise the base path won't work
		if (substr($this->_full_path, 0, strlen(SQ_SYSTEM_ROOT)) != SQ_SYSTEM_ROOT) {
			trigger_error('Unable to construct "'.get_class($this).'" because there is the _full_path is not under the SYSTEM_ROOT', E_USER_ERROR);
		}

		// OK, if this is the "core" package then we just need to fiddle with the path
		$this->_base_path = substr($this->_full_path, strlen(SQ_SYSTEM_ROOT) + 1);

		if (file_exists($this->_full_path.'/package.xml')) {
			$xml = new XML_Array();
			$info = $xml->getArrayFromFile($this->_full_path.'/package.xml');
			if (PEAR::isError($info)) {
				trigger_error($info->error_message_prefix.$info->message, E_USER_ERROR);
			}
			$xml->close();
			if ($this->getPackageName() != $info['code_name']['value']) {
				trigger_error('The code name for the package in the file "'.$this->_full_path.'/package.xml" does not match the package that we are trying to open ('.$this->getPackageName().').', E_USER_ERROR);
			}
			$this->name			= $info['name']['value'];
			$this->version		= $info['version']['value'];
			$this->description	= $info['description']['value'];

			$this->_loadPackageAssets();

		} else {
			trigger_error('Package "'.$this->getPackageName().'" does not have a package.xml file', E_USER_ERROR);
		}

	}//end constructor


	/**
	* Returns the name of the package that this management class is working for
	*
	* @access public
	*/
	function getPackageName()
	{
		return substr(get_class($this), 16);

	}//end getPackageName()


	/**
	* Checks to see if the installation/upgrade of this package is able to go ahead.
	* Will return 'true' if all assets are okay, otherwise will return an array
	* of assetids that failed requirements. 
	*
	* @param Array $update_assets		An array of asset types to check 
	*									details for. If empty, will check all assets
	* @return mixed						
	* @access public
	*/
	function checkPackageDetails($update_assets=Array())
	{
		$am = &$GLOBALS['SQ_SYSTEM']->am;

		$deferred_assets = Array();
		
		for (reset($this->assets); null !== ($type_code = key($this->assets)); next($this->assets)) {
			// Don't bother checking an asset that's not defined in the array.
			if (count($update_assets)) {
				if (!in_array($type_code, $update_assets)) {
					continue;
				}
			}
			if (!isset($install_reqs[$type_code])) $install_reqs[$type_code] = Array();

			// make sure all requirements are met
			$file = SQ_SYSTEM_ROOT.'/'.$this->assets[$type_code]['dir'].'/asset.xml';
			list($result, $req) = $this->checkRequirementsFromFile($file, true);

			if (!$result) {
				trigger_error('Asset "'.$type_code.'" fails the "'.$req['type'].'" requirement for "'.$req['code'].' v'.$req['version'].'"'."\n".'As specified by '.$file.' and will be deferred', E_USER_WARNING);
				// It failed the requirement, so we want to let the caller know what asset 
				// failed so that it can be revisited at a later date if needbe
				$deferred_assets[] = $type_code;
				continue;
			}

			// OK, let's check that this asset is of an available type,
			// or that the type is in this package that we are checking
			$parent_type = $this->assets[$type_code]['parent_type'];
			if ($parent_type != 'asset' && !$am->installed($parent_type) && !isset($this->assets[$parent_type])) {
				trigger_error('Asset "'.$type_code.'" attempts to inherit from the unknown type "'.$parent_type.'"'."\n".'As specified by '.$file, E_USER_WARNING);
				return false;
			}

		}//end for

		
		return count($deferred_assets) ? $deferred_assets : true;

	}//end checkPackageDetails()


	/**
	* Installs/Updates the passed package in the system
	*
	* @param Array $update_assets		An array of asset types to update 
	*									details for. If empty, will do all assets
	* @return mixed						success status, or an array of type codes
	*									that has been deferred for later checking
	* @access public
	*/
	function updatePackageDetails($update_assets=Array())
	{
		// If there were some type codes that failed the requirements, save
		// them in the deferred array so that they don't get installed later
		// on in the process.
		$deferred = $this->checkPackageDetails($update_assets);
		if (is_bool($deferred) && !$deferred) {
			return $deferred;
		}
		
		// Make sure deferred is an array.
		$deferred = is_array($deferred) ? $deferred : Array();
		
		$install_order = $this->_getInstallOrder();
		if (!empty($install_order)) {

			pre_echo('Installing '.get_class($this).':');
			
			$package_name = $this->getPackageName();
	
			$db = &$GLOBALS['SQ_SYSTEM']->db;
			// open the transaction
			$GLOBALS['SQ_SYSTEM']->doTransaction('BEGIN');
	
			$exists = $db->getOne('SELECT code_name FROM '.SQ_TABLE_PREFIX.'package WHERE code_name = '.$db->quote($package_name));
			if (DB::isError($exists)) trigger_error($exists->getMessage()."\n".$exists->getUserInfo(), E_USER_ERROR);
	
			if ($exists) {
				$sql = 'UPDATE '.SQ_TABLE_PREFIX.'package
						SET version     = '.$db->quote($this->version).',
							name        = '.$db->quote($this->name).',
							description = '.$db->quote($this->description).'
						WHERE code_name = '.$db->quote($package_name);
			} else {
				$sql = 'INSERT INTO '.SQ_TABLE_PREFIX.'package
						(code_name, version, name, description)
						VALUES
						('.$db->quote($package_name).', '.$db->quote($this->version).',
						'.$db->quote($this->name).','.$db->quote($this->description).')';
			}
	
			$result = $db->query($sql);
			if (DB::isError($result)) {
				$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
				trigger_error($result->getMessage()."\n".$result->getUserInfo(), E_USER_WARNING);
				return false;
			}
	
			for (reset($install_order); null !== ($k = key($install_order)); next($install_order)) {
				$type_code =& $install_order[$k];
	
				// Don't bother checking an asset that's not defined in the update_assets array.
				if (count($update_assets)) {
					if (!in_array($type_code, $update_assets)) {
						continue;
					}
				}
				
				// If the asset was found in the list of failed dependencies, 
				// then don't try installing it just yet, but install
				// all other types as they may be vital to the dependency 
				// resolution.
				if (in_array($type_code, $deferred)) {
					continue;
				}
				
				echo "$k => $type_code \n";
				
				// now we simply call the management class for this object
				$file = SQ_SYSTEM_ROOT.'/'.$this->assets[$type_code]['dir'].'/'.$type_code.'_management.inc';
				// include rather that require so that if it doesn't exist we can kill the transaction
				require_once $file;
				$class = $type_code.'_management';
				// make sure the class exists
				if (class_exists($class)) {
					// now just create an instance of it, and run update()
					$management = new $class($this);
					if (!$management->update()) {
						$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
						return false;
					}
					
				} else {
					trigger_error('Asset "'.$type_code.'" does not have a management class defined for it in '.$file, E_USER_WARNING);
					$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
					return false;
				}
	
			}//end for
			
			// Commit the transaction if we got this far
			$GLOBALS['SQ_SYSTEM']->doTransaction('COMMIT');

		}//end if asset types to install

		// create the xml summarys of screens info for the asset map
		if (!$this->_createAssetMapScreensXML($update_assets)) {
			trigger_error('Unable to create the Asset Map Screens XML', E_USER_WARNING);
			$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
			return false;
		}

		$this->_postInstall();
		return true;

	}//end updatePackageDetails()


	/**
	* Recursivly searches through the passed package's directory for asset.xml files
	* and enter's them into the $this->assets array with the details
	*
	* @access private
	*/
	function _loadPackageAssets()
	{
		$this->assets = Array();
		$this->_recurseLoadPackageAssets($this->assets, $this->_full_path, $this->_base_path);

	}//end _loadPackageAssets()


	/**
	* Recusively load information for the assets in this package
	*
	* @param Array		&$assets	the assets to load the information for
	* @param string		$full_path	the absolute path to the directory where the assets exist
	* @param string		$base_path	the base path where the assets exists eg. for cms base_path=package/cms
	*
	* @return void
	* @access private
	*/
	function _recurseLoadPackageAssets(&$assets, $full_path, $base_path)
	{
		$package = $this->getPackageName();
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

				if ($package != $info['package']) {
					trigger_error('The Asset Information file "'.$full_path.'/asset.xml" is in the "'.$package.'" Package directory, but states that it is belongs to "'.$info['package'].'"', E_USER_ERROR);
				}

				$info['dir'] = $base_path;

				$assets[$info['type_code']] = $info;

			}//end if
		}//end while
		$d->close();

	}//end _recurseLoadPackageAssets()


	/**
	* Generates an array holding the type_codes for the order in which
	* the assets should be installed in order to respect the heirarchy
	* defined in the asset.xml files
	*
	* @access private
	*/
	function _getInstallOrder()
	{
		$asset_types = array_keys($this->assets);
		$install_order = Array();

		while(count($asset_types)) {

			$last_count = count($asset_types);

			for($i = 0; $i < count($asset_types); $i++) {

				$type_code = $asset_types[$i];
				
				$parent    = $this->assets[$type_code]['parent_type'];

				$remove = false;

				// if this type is inheritted from an existing type, then all OK
				// but not if this type is in this package
				if (($parent == 'asset' || $GLOBALS['SQ_SYSTEM']->am->installed($parent)) && !in_array($parent, $asset_types)) {
					$install_order[] = $type_code;
					$remove = true;

				// if this type is inheriting from a type that will be installed before it, then all OK
				} elseif (in_array($parent, $install_order)) {
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
				trigger_error('There seems to be the possibility of recursive inheriting for the "'.$this->getPackageName().'" package.'."\n".'The install order for these assets could not be established: '.implode(', ', $asset_types), E_USER_WARNING);
				return Array();
			}


		}//end while

		return $install_order;

	}//end _getInstallOrder()


	/**
	* Generates a segment of an XML document for the Asset_Map.inc to use to send to the
	* flash
	*
	* @param Array $update_assets		An array of asset types to update 
	*									details for. If empty, will do all assets
	* @access private
	*/
	function _createAssetMapScreensXML($update_assets=Array())
	{
		$output = new XML_Tree();
		$root = &$output->addRoot('tmp', '');
		$str  = '';

		for(reset($this->assets); null !== ($type_code = key($this->assets)); next($this->assets)) {
			// Don't bother checking an asset that's not defined in the update_assets array.
			if (count($update_assets)) {
				if (!in_array($type_code, $update_assets)) {
					continue;
				}
			}
			$attrs = Array(	'type_code'			=> $type_code,
							'name'				=> $this->assets[$type_code]['name'],
							'version'			=> $this->assets[$type_code]['version'],
							'instantiable'		=> $this->assets[$type_code]['instantiable'],
							'allowed_access'	=> $this->assets[$type_code]['allowed_access'],
							'parent_type'		=> $this->assets[$type_code]['parent_type'],
							'flash_menu_path'	=> $this->assets[$type_code]['flash_menu_path'],
						);

			$type_node = &$root->addChild('type', '', $attrs);

			$GLOBALS['SQ_SYSTEM']->am->includeAsset($type_code);
			$as =& new $type_code();
			$ef = &$as->getEditFns();

			if (!empty($ef->static_screens['details'])) {
				$type_node->addChild('screen', $ef->static_screens['details']['name'], Array('code_name' => 'details'));
			}

			$ei = new Asset_Edit_Interface($type_code);
			$screens = $ei->getScreens();
			for(reset($screens); null !== ($code_name = key($screens)); next($screens)) {
				$type_node->addChild('screen', $screens[$code_name]['name'], Array('code_name' => $code_name));
			}

			$GLOBALS['SQ_SYSTEM']->am->includeAsset($type_code);
			$as =& new $type_code();
			$ef = &$as->getEditFns();
			for(reset($ef->static_screens); null !== ($code_name = key($ef->static_screens)); next($ef->static_screens)) {
				if ($code_name == 'details') continue;
				$type_node->addChild('screen', $ef->static_screens[$code_name]['name'], Array('code_name' => $code_name));
			}

			$str .= $type_node->get();

		}// end for
		require_once SQ_FUDGE_PATH.'/general/file_system.inc';
		return string_to_file($str, SQ_DATA_PATH.'/private/asset_map/'.$this->getPackageName().'.xml');

	}//end _createAssetMapScreensXML()


	/**
	* checks the requirements of the passed asset.xml file
	* see checkRequirements() for more info
	*
	* @param string		$file				the code name for the asset that you want to check
	* @param boolean	$need_feedback
	* @param array		$ignore_assets		if the requirement is an asset, but is in this list,
	*										the requirement is considered to have passed
	*
	* @return mixed
	* @access public
	*/
	function checkRequirementsFromFile($file, $need_feedback=false)
	{
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
					$installed_version = $GLOBALS['SQ_SYSTEM']->db->getOne('SELECT version FROM '.SQ_TABLE_PREFIX.'package WHERE code_name = '.$GLOBALS['SQ_SYSTEM']->db->quote($req['code']['value']));
					if (DB::isError($installed_version)) trigger_error($installed_version->getMessage()."\n".$installed_version->getUserInfo(), E_USER_ERROR);

					if ($installed_version) {
						$ok = version_compare($installed_version, $req['version']['value'], '>=');
					}
					break;

				case 'asset' :
					if ($GLOBALS['SQ_SYSTEM']->am->installed($req['code']['value'])) {
						$ok = version_compare($GLOBALS['SQ_SYSTEM']->am->getTypeInfo($req['code']['value'], 'version'), $req['version']['value'], '>=');
					}
					// if the test failed, but this asset is in this package, check it's version from that
					if (!$ok && isset($this->assets[$req['code']['value']])) {
						$ok = version_compare($this->assets[$req['code']['value']]['version'], $req['version']['value'], '>=');
					}
					break;

				case 'system' :
					switch($req['code']['value']) {
						case 'System' :
							$ok = version_compare(SQ_SYSTEM_VERSION, $req['version']['value'], '>=');
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


	/**
	* Returns an array of all assets types that are parents for the passed asset
	* that is in the this package, in the order Array ('parent', 'grand-parent', 'great-grand-parent', ..., 'asset')
	*
	* @param string $type_code  the code name for the asset that you want to check
	*
	* @return Array(string)
	* @access public
	*/
	function getTypeAncestors($type_code)
	{
		if (!isset($this->assets[$type_code])) {
			trigger_error('Asset "'.$type_code.'" does not exist in the "'.$this->getPackageName().'" package', E_USER_ERROR);
			return Array();
		}

		$arr = Array();

		while(isset($this->assets[$type_code]) && $this->assets[$type_code]['parent_type'] != 'asset') {
			$type_code = $this->assets[$type_code]['parent_type'];
			$arr[]     = $type_code;
		}

		// if we are inheriting from something else then go find it from the installed list
		if (!isset($this->assets[$type_code])) {
			$new_arr = $GLOBALS['SQ_SYSTEM']->am->getTypeAncestors($type_code);
			foreach($new_arr as $type_code) $arr[] = $type_code;

		} else {
			$arr[] = 'asset';

		}

		return $arr;

	}//end getTypeAncestors()


	/**
	* Returns an array of asset information or a specific piece of information
	*
	* @param string $type_code  the code name for the asset that you want to check
	* @param string $field      if exists returns this information from the array for this specific field
	*
	* @return mixed
	* @access public
	*/
	function getTypeInfo($type_code, $field='')
	{
		if (!isset($this->assets[$type_code])) {
			trigger_error('Asset "'.$type_code.'" does not exist in the "'.$this->getPackageName().'" package', E_USER_WARNING);
			return ($field) ? null : Array();
		}

		return ($field) ? $this->assets[$type_code][$field] : $this->assets[$type_code];

	}//end getTypeInfo()


	/**
	* Iterate through the asset management class and install any events
	*
	* @return boolean
	* @access public
	*/
	function installEventListeners()
	{
		$em = &$GLOBALS['SQ_SYSTEM']->getEventManager();

		foreach (array_keys($this->assets) as $type_code) {
			$file = SQ_SYSTEM_ROOT.'/'.$this->assets[$type_code]['dir'].'/'.$type_code.'_management.inc';
			require_once $file;
			$class = $type_code.'_management';

			// make sure the class exists
			if (class_exists($class)) {
				$management = new $class($this);
				$events = $management->getEventList();
				if (!is_array($events)) {
					trigger_error('Expected Array in function getEventList() (Asset "'.$type_code.'"), got '.gettype($events), E_USER_WARNING);
					return false;
				}
				if (empty($events)) continue;
				$em->installStaticEvents($events, $type_code);
			}
		}//end foreach

	}//end installEventListeners()


	/**
	* Iterate through the asset preference classes to add preference vars to an array
	*
	* @return array
	* @access public
	*/
	function installUserPreferences(&$preferences)
	{
		foreach (array_keys($this->assets) as $type_code) {
			$file = SQ_SYSTEM_ROOT.'/'.$this->assets[$type_code]['dir'].'/'.$type_code.'_prefs.inc';
			if (!is_file($file)) continue;
			require_once $file;
			$class = $type_code.'_prefs';

			// make sure the class exists
			if (class_exists($class)) {
				$prefs = new $class();
				if (!isset($preferences[$type_code])) {
					$preferences[$type_code] = $prefs->pref_vars;
				} else {
					foreach ($preferences[$type_code] as $var => $data) {
						if (!isset($prefs->pref_vars[$var])) unset($preferences[$type_code][$var]);
					}
					foreach ($prefs->pref_vars as $var => $data) {
						if (!isset($preferences[$type_code][$var])) $preferences[$type_code][$var] = $data;
					}
				}
			}
		}//end foreach

	}//end installUserPreferences()


	/**
	* Returns an array of all the asset types in the package that are to be treated as system assets
	* NOTE: Assets will be installed in the order they appear in this list
	*
	* @return Array()
	* @access public
	* @static
	*/
	function getSystemAssetTypes()
	{
		return Array();

	}//end getSystemAssetTypes()


	/**
	* Installs System Assets for this package
	*
	* @return int the number of system assets created (-1 reserved for error)
	* @access protected
	*/
	function installSystemAssets()
	{
		// check for any system assets that have not yet been installed and install them if required
		$updated = false;
		$assets_created = 0;

		$system_assets = $GLOBALS['SQ_SYSTEM_ASSETS'];
		foreach ($this->getSystemAssetTypes() as $type) {
			if (!isset($system_assets[$type])) {
				$function = 'create'.str_replace('_','',$type);
				$new_system_asset = &$this->$function();
				if (!is_null($new_system_asset)) {
					if ($new_system_asset->id) $updated = true;
					$assets_created++;
				} else {
					// could not create the asset?!?
					return -1;
				}
			}
		}

		// regen the system assets file if needed
		if ($updated) {
			require_once SQ_INCLUDE_PATH.'/system_asset_config.inc';
			$sys_asset_cfg = new System_Asset_Config();
			$sys_asset_cfg->save(Array(), false);
		}

		return $assets_created;

	}//end installSystemAssets()


	/**
	* Post install script for package; runs at end of update packages
	*
	* @abstract
	* @access protected
	*/
	function _postInstall()
	{

	}//end _postInstall()

}//end class

?>