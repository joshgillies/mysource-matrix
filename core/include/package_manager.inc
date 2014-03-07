<?php
/**
* +--------------------------------------------------------------------+
* | This MySource Matrix CMS file is Copyright (c) Squiz Pty Ltd       |
* | ABN 77 084 670 600                                                 |
* +--------------------------------------------------------------------+
* | IMPORTANT: Your use of this Software is subject to the terms of    |
* | the Licence provided in the file licence.txt. If you cannot find   |
* | this file please contact Squiz (www.squiz.com.au) so we may provide|
* | you a copy.                                                        |
* +--------------------------------------------------------------------+
*
* $Id: package_manager.inc,v 1.92 2012/08/30 01:09:19 ewang Exp $
*
*/


require_once SQ_INCLUDE_PATH.'/asset_management.inc';
require_once SQ_INCLUDE_PATH.'/asset_edit_interface.inc';

/**
* Package_Manager
*
* Purpose
*    On a per package basis this object let's the system install and upgrade
*    packages and their assets, uses info gleaned from the package.xml and asset.xml files
*
* @author  Blair Robertson <blair@squiz.net>
* @version $Revision: 1.92 $
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
	* @var array
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
			trigger_localised_error('SYS0238', E_USER_ERROR, get_class($this));
		}

		// make sure that this package is in our system rootm, otherwise the base path won't work
		if (substr($this->_full_path, 0, strlen(SQ_SYSTEM_ROOT)) != SQ_SYSTEM_ROOT) {
			trigger_localised_error('SYS0239', E_USER_ERROR, get_class($this));
		}

		// OK, if this is the "core" package then we just need to fiddle with the path
		$this->_base_path = substr($this->_full_path, strlen(SQ_SYSTEM_ROOT) + 1);
		$package_path = $this->_full_path.'/package.xml';

		if (file_exists($package_path)) {
			try {
				$info = simplexml_load_string(file_get_contents($package_path));
			} catch (Exception $e) {
				throw new Exception('Unable to parse package definition file "'.$package_path.'": '.$e->getMessage());
			}

			if ($this->getPackageName() != $info->{'code_name'}) {
				throw new Exception('Unable to parse package definition file "'.$package_path.'": package manager name "'.$this->getPackageName().'" does not match package name in definition file "'.$info->{'code_name'}.'"');
			}
			$this->name			= (string)$info->name;
			$this->version		= (string)$info->version;
			$this->description	= (string)$info->description;

			$GLOBALS['SQ_SYSTEM']->lm->includePackageStrings($this->name);
			$this->_loadPackageAssets();

		} else {
			trigger_localised_error('SYS0199', E_USER_ERROR, $this->getPackageName());
		}

	}//end constructor


	/**
	* Returns the name of the package that this management class is working for
	*
	* @access public
	*/
	function getPackageName()
	{
		return strtolower(substr(get_class($this), 16));

	}//end getPackageName()


	/**
	* Checks to see if the installation/upgrade of this package is able to go ahead
	*
	* Will return 'true' if all assets are okay, otherwise will return an array
	* of assetids that failed requirements.
	*
	* @param array	$update_assets		an array of asset types to check details for
	*									if empty, will check all assets
	* @param array	$deferred_assets	an array of previously deferred assets
	* @return mixed
	* @access public
	*/
	function checkPackageDetails($update_assets=Array(), $deferred_assets=Array())
	{
		$am = $GLOBALS['SQ_SYSTEM']->am;

		for (reset($this->assets); null !== ($type_code = key($this->assets)); next($this->assets)) {
			// Don't bother checking an asset that's not defined in the array.
			if (!empty($update_assets) && !in_array($type_code, $update_assets)) {
				continue;
			}
			if (!isset($install_reqs[$type_code])) {
				$install_reqs[$type_code] = Array();
			}

			// make sure all requirements are met
			$file = SQ_SYSTEM_ROOT.'/'.$this->assets[$type_code]['dir'].'/asset.xml';
			list($result, $req) = $this->checkRequirementsFromFile($file, $deferred_assets, true);

			if (!$result) {
				// It failed the requirement, so we want to let the caller know what asset
				// failed so that it can be revisited at a later date if needbe
				if (!in_array($type_code, $deferred_assets)) {
					pre_echo('Asset "'.$type_code.'" fails the "'.$req['type'].'" requirement for "'.$req['code'].' v'.$req['version'].'"'."\n".'As specified by '.$file.' and will be deferred');
					$deferred_assets[] = $type_code;
				}
				continue;
			}

			// OK, let's check that this asset is of an available type,
			// or that the type is in this package that we are checking
			$parent_type = $this->assets[$type_code]['parent_type'];
			if ($parent_type != 'asset' && !$am->installed($parent_type) && !isset($this->assets[$parent_type])) {
				// It failed the requirement, so we want to let the caller know what asset
				// failed so that it can be revisited at a later date if needbe
				if (!in_array($type_code, $deferred_assets)) {
					pre_echo('Asset "'.$type_code.'" attempts to inherit from the unknown type "'.$parent_type.'"'."\n".'As specified by '.$file);
					$deferred_assets[] = $type_code;
				}
				continue;
			}
		}//end for

		return count($deferred_assets) ? $deferred_assets : true;

	}//end checkPackageDetails()


	/**
	* Installs/Updates the passed package in the system
	*
	* @param array	$update_assets	an array of asset types to update details for
	*								if empty, will do all assets
	*
	* @return mixed					success status, or an array of type codes
	*								that has been deferred for later checking
	* @access public
	*/
	function updatePackageDetails($update_assets=Array())
	{
		$em = $GLOBALS['SQ_SYSTEM']->getEventManager();

		// If there were some type codes that failed the requirements, save
		// them in the deferred array so that they don't get installed later
		// on in the process
		$last_deferred = false;
		$deferred = true;

		// We need to run this function over until no more deferred assets are
		// detected - because of the order of checking the package details, we
		// may not pick everything up first go
		while ($last_deferred !== $deferred) {
			$last_deferred = $deferred;
			$deferred = $this->checkPackageDetails($update_assets, is_array($deferred) ? $deferred : Array());
			if (is_bool($deferred) && !$deferred) {
				return $deferred;
			}
		}

		$GLOBALS['SQ_SYSTEM']->changeDatabaseConnection('db2');
		$GLOBALS['SQ_SYSTEM']->doTransaction('BEGIN');

		// Make sure deferred is an array.
		$deferred = is_array($deferred) ? $deferred : Array();

		$install_order = $this->_getInstallOrder($update_assets, $deferred);
		if (!empty($install_order)) {

			$package_name = $this->getPackageName();

			$db = MatrixDAL::getDb();

			$sql = 'SELECT code_name FROM sq_pkg WHERE code_name = :code_name';
			try {
				$query = MatrixDAL::preparePdoQuery($sql);
				MatrixDAL::bindValueToPdo($query, 'code_name', $package_name);
				$exists = MatrixDAL::executePdoOne($query);
			} catch (Exception $e) {
				throw new Exception('Failed to get package information for package "'.$package_name.'" due to database error: '.$e->getMessage());
			}

			try {
				// DAL query
				$bind_vars = Array(
								'version'      => $this->version,
								'name'         => $this->name,
								'description'  => $this->description,
								'package_name' => $package_name,
							 );

				if (!$exists) {
					$query_name = 'installPackage';
				} else {
					$query_name = 'updatePackage';
				}
				MatrixDAL::executeQuery('core', $query_name, $bind_vars);
			} catch (Exception $e) {
				// In case it needs to be localised, split the two messages
				// instead of doing one with a ?: switch in the middle
				if ($exists) {
					throw new Exception('Failed to update package information for package "'.$package_name.'" due to database error: '.$e->getMessage());
				} else {
					throw new Exception('Failed to add package information for new package "'.$package_name.'" due to database error: '.$e->getMessage());
				}
			}

			for (reset($install_order); null !== ($k = key($install_order)); next($install_order)) {
				$type_code = $install_order[$k];

				// Don't bother checking an asset that's not defined in the update_assets array.
				if (!empty($update_assets) && !in_array($type_code, $update_assets)) {
					continue;
				}

				// If the asset was found in the list of failed dependencies,
				// then don't try installing it just yet, but install
				// all other types as they may be vital to the dependency
				// resolution.
				if (in_array($type_code, $deferred)) continue;

				// now we simply call the management class for this object
				$class = $type_code.'_management';
				$file = SQ_SYSTEM_ROOT.'/'.$this->assets[$type_code]['dir'].'/'.$class.'.inc';
				if (!is_file($file)) {
					trigger_localised_error('CORE0276', E_USER_WARNING, $file);
					$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
					$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();
					return false;
				}

				// include rather that require so that if it doesn't exist we can kill the transaction
				include_once $file;
				// make sure the class exists
				if (!class_exists($class)) {
					trigger_localised_error('SYS0083', E_USER_WARNING, $type_code, $file);
					$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
					$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();
					return false;
				}

				// now just create an instance of it, and run update()
				$management = new $class($this);
				if (!$management->update()) {
					echo 'Unable to install '.$type_code."\n";
					$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
					$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();
					return false;
				}

				// generate the temporary list of event listeners
				$events = $management->getEventList();
				if (!is_array($events)) {
					trigger_localised_error('SYS0142', E_USER_WARNING, $type_code, gettype($events));
					$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
					$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();
					return false;
				}

				if (!empty($events)) {
					// register the event to be installed later
					$em->installStaticEvents($events, $type_code);
				}


				// fire assetInstall event to signal asset installation or update
				// sends the type_code of the installed asset
				$em->broadcastEvent($this, 'assetInstall', Array('type_code' => $type_code));

			}//end for

			// fire PackageInstall event to signal package installation or update
			// sends a list of all the assets in the installed package
			$em->broadcastEvent($this, 'packageInstall', $install_order);

		}//end if asset types to install

		// create the xml summarys of screens info for the asset map
		if (!$this->_createAssetMapScreensXML($deferred)) {
			trigger_localised_error('SYS0242', E_USER_WARNING);
			$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
			$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();
			return false;
		}

		$this->_postInstall();

		$GLOBALS['SQ_SYSTEM']->doTransaction('COMMIT');
		$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();

		return count($deferred) ? $deferred : true;

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

		// reload the asset types here :/
		// otherwise any changes in asset locations will not be picked up by the asset manager
		$GLOBALS['SQ_SYSTEM']->am->_loadAssetTypes();

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
	function _recurseLoadPackageAssets(Array &$assets, $full_path, $base_path)
	{
		$package = $this->getPackageName();
		$d = new DirectoryIterator($full_path);
		foreach ($d as $entry) {
			$entry = (string)$entry;
			if ($entry === '.' || $entry === '..') {
				continue;
			}

			if ($entry === 'asset.xml') {
				// we have an asset process it
				$mgt = new Asset_Management($this);
				$info = $mgt->getAssetInfo($full_path.'/asset.xml');
				if (empty($info)) continue;

				if ($package != $info['package']) {
					trigger_localised_error('SYS0210', E_USER_ERROR, $full_path, $package, $info['package']);
				}

				$info['dir'] = $base_path;

				// check the path of this type in db we may need to update it here incase it was moved
				$GLOBALS['SQ_SYSTEM']->changeDatabaseConnection('db2');
				$db = MatrixDAL::getDb();
				$GLOBALS['SQ_SYSTEM']->doTransaction('BEGIN');

				$sql = 'SELECT dir FROM sq_ast_typ WHERE type_code = :type_code';

				try {
					$query = MatrixDAL::preparePdoQuery($sql);
					MatrixDAL::bindValueToPdo($query, 'type_code', $info['type_code']);
					$result = MatrixDAL::executePdoOne($query);
				} catch (DALException $e) {
					throw new Exception ('Unable to get directory of type code '.$info['type_code'].' due to database error: '.$e->getMessage());
				}

				if (!empty($result) && ($result != $info['dir'])) {
					// update path
					$sql = 'UPDATE
								sq_ast_typ
							SET
								dir = :dir
							WHERE
								type_code = :type_code';

					try {
						$query = MatrixDAL::preparePdoQuery($sql);
						MatrixDAL::bindValueToPdo($query, 'dir',       $info['dir']);
						MatrixDAL::bindValueToPdo($query, 'type_code', $info['type_code']);
						MatrixDAL::execPdoQuery($query);
					} catch (DALException $e) {
						throw new Exception ('Unable to update directory of type code '.$info['type_code'].' due to database error: '.$e->getMessage());
					}
				}

				$GLOBALS['SQ_SYSTEM']->doTransaction('COMMIT');
				$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();

				$assets[$info['type_code']] = $info;

			} else if (is_dir($full_path.'/'.$entry)) {
				// if this is a directory, recursively search it
				$this->_recurseLoadPackageAssets($assets, $full_path.'/'.$entry, $base_path.'/'.$entry);
			}//end if
		}//end foreach
		$d = NULL;

	}//end _recurseLoadPackageAssets()


	/**
	* Generates an array holding the type_codes for the order in which
	* the assets should be installed in order to respect the heirarchy
	* defined in the asset.xml files
	*
	* @param Array $update_assets Assets that are going to be updated.
	* @param Array $deferred Any asset types that are going to be deferred this installation pass
	* @access private
	*/
	function _getInstallOrder($update_assets=Array(), $deferred=Array())
	{
		$asset_types = array_keys($this->assets);
		$install_order = Array();

		while ($last_count = count($asset_types)) {

			for ($i = 0; $i < count($asset_types); $i++) {

				$type_code = $asset_types[$i];

				if (!empty($update_assets) && !in_array($type_code, $update_assets)) {
					unset($asset_types[$i]);
					$asset_types = array_values($asset_types);
					$i--;
					continue;
				}

				if (in_array($type_code, $deferred)) {
					unset($asset_types[$i]);
					$asset_types = array_values($asset_types);
					$i--;
					continue;
				}

				$parent    = $this->assets[$type_code]['parent_type'];

				$remove = false;

				// if this type is inheritted from an existing type, then all OK
				// but not if this type is in this package
				if (($parent == 'asset' || $GLOBALS['SQ_SYSTEM']->am->installed($parent)) && !in_array($parent, $asset_types)) {
					$install_order[] = $type_code;
					$remove = true;

				// if this type is inheriting from a type that will be installed before it, then all OK
				} else if (in_array($parent, $install_order)) {
					$install_order[] = $type_code;
					$remove = true;


				// if this type is not inheriting from a type that is yet to be dealt with, then
				// the type is in heriting from something that doesn't exist
				} else if (!in_array($parent, $asset_types) && !in_array($parent, $deferred)) {
					trigger_localised_error('SYS0080', E_USER_WARNING, $type_code, $parent);
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
				trigger_localised_error('SYS0216', E_USER_WARNING, $this->getPackageName(), implode(', ', $asset_types));
				return Array();
			}


		}//end while

		return $install_order;

	}//end _getInstallOrder()


	/**
	* Generates a segment of an XML document for the Asset_Map.inc to use to send to the
	* flash
	*
	* @param Array $deferred			An array of asset types to defer this
	* 									install
	* @access private
	*/
	function _createAssetMapScreensXML($deferred=Array())
	{
		$root = new SimpleXMLElement('<tmp />');
		$str  = '';

		for (reset($this->assets); null !== ($type_code = key($this->assets)); next($this->assets)) {
			if (in_array($type_code, $deferred)) continue;

			$attrs = Array(
						'type_code'			=> $type_code,
						'name'				=> $this->assets[$type_code]['name'],
						'version'			=> $this->assets[$type_code]['version'],
						'instantiable'		=> $this->assets[$type_code]['instantiable'],
						'allowed_access'	=> $this->assets[$type_code]['allowed_access'],
						'parent_type'		=> $this->assets[$type_code]['parent_type'],
						'flash_menu_path'	=> $this->assets[$type_code]['flash_menu_path'],
					 );

			$type_node = $root->addChild('type');
			foreach ($attrs as $attr_name => $attr_value) {
				if ($attr_value !== '') {
					$type_node->addAttribute($attr_name, $attr_value);
				}
			}

			$GLOBALS['SQ_SYSTEM']->am->includeAsset($type_code);
			$as = new $type_code();
			$ef = $as->getEditFns();

			// Ensure that the Details screen is first
			if (!empty($ef->static_screens['details'])) {
				$screen_node = $type_node->addChild('screen', $ef->static_screens['details']['name']);
				$screen_node->addAttribute('code_name', 'details');
			}

			// Asset-specific custom inteface screens next
			$ei = new Asset_Edit_Interface($type_code);
			$screens = $ei->getScreens();
			for (reset($screens); null !== ($code_name = key($screens)); next($screens)) {
				if ($screens[$code_name]['invisible']) continue;
				$screen_node = $type_node->addChild('screen', $screens[$code_name]['name']);
				$screen_node->addAttribute('code_name', $code_name);
			}

			// Static screens finally (other than Details -we already have that)
			$GLOBALS['SQ_SYSTEM']->am->includeAsset($type_code);
			$as = new $type_code();
			$ef = $as->getEditFns();
			for (reset($ef->static_screens); null !== ($code_name = key($ef->static_screens)); next($ef->static_screens)) {
				if ($code_name == 'details') continue;
				$screen_node = $type_node->addChild('screen', $ef->static_screens[$code_name]['name']);
				$screen_node->addAttribute('code_name', $code_name);
			}

			$str .= $type_node->asXml();

		}// end for

		require_once SQ_FUDGE_PATH.'/general/file_system.inc';
		return string_to_file($str, SQ_DATA_PATH.'/private/asset_map/'.$this->getPackageName().'.xml');

	}//end _createAssetMapScreensXML()


	/**
	* checks the requirements of the passed asset.xml file
	* see checkRequirements() for more info
	*
	* We need to pass the list of deferred assets to this function, because while it can handle
	* direct requirements it can't normally handle assets that have to be deferred because their
	* parents have had to be deferred (eg. if type B has to be deferred because type A needs to
	* be installed, then it couldn't pick up that type C (which depends on B) needs to be
	* deferred too
	*
	* @param string		$file				the code name for the asset that you want to check
	* @param array		$deferred_assets	a list of previously deferred assets
	* @param boolean	$need_feedback
	* @param array		$ignore_assets		if the requirement is an asset, but is in this list,
	*										the requirement is considered to have passed
	*
	* @return mixed
	* @access public
	*/
	function checkRequirementsFromFile($file, $deferred_assets=Array(), $need_feedback=false)
	{
		$db = MatrixDAL::getDb();

		try {
			$info = simplexml_load_string(file_get_contents($file));
		} catch (Exception $e) {
			throw new Exception('Could not parse asset definition file "'.$file.'": '.$e->getMessage());
		}

		// if there are no requirements, return true
		if (!isset($info->requirements->requirement)) {
			return ($need_feedback) ? Array(true, null) : true;
		}

		foreach ($info->requirements->requirement as $req) {
			$required_type    = (string)$req->attributes()->type;
			$required_code    = (string)$req->code;
			$required_version = (string)$req->version;

			$ok = false;
			switch ($required_type) {
				case 'package':
					$sql = 'SELECT version FROM sq_pkg WHERE code_name = :code_name';

					try {
						$query = MatrixDAL::preparePdoQuery($sql);
						MatrixDAL::bindValueToPdo($query, 'code_name', $required_code);
						$installed_version = MatrixDAL::executePdoOne($query);
					} catch (DALException $e) {
						throw new Exception('Unable to get version of package "'.$required_code.'" due to database error: '.$e->getMessage());
					}

					if ($installed_version) {
						$ok = version_compare($installed_version, $required_version, '>=');
					}
				break;

				case 'system_asset':
				case 'asset':
					if (in_array($required_code, $deferred_assets)) {
						// we have to defer an asset that depends on another deferred asset
						$ok = false;
					} else {
						if ($GLOBALS['SQ_SYSTEM']->am->installed($required_code)) {
							$ok = version_compare($GLOBALS['SQ_SYSTEM']->am->getTypeInfo($required_code, 'version'), $required_version, '>=');
						}
						// if the test failed, but this asset is in this package, check it's version from that
						if (!$ok && isset($this->assets[$required_code])) {
							$ok = version_compare($this->assets[$required_code]['version'], $required_version, '>=');
						}
						// If it's a system_asset check AND it has already passed the other checks,
						// make sure the system_asset is actually available.
						if ($required_type === 'system_asset' && $ok) {
							$ok = $GLOBALS['SQ_SYSTEM']->am->isSystemAssetType($required_code);
						}
					}
				break;

				case 'system':
					switch ($required_code) {
						case 'System' :
							$ok = version_compare(SQ_SYSTEM_VERSION, $required_version, '>=');
						break;
						case 'PHP' :
							$ok = version_compare(PHP_VERSION, $required_version, '>=');
						break;
					}
				break;

			}//end switch

			// this requirement failed
			if (!$ok) {

				if ($need_feedback) {
					$ret_arr = Array(
								'type'		=> $required_type,
								'code'		=> $required_code,
								'version'	=> $required_version,
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
	* @return array(string)
	* @access public
	*/
	function getTypeAncestors($type_code)
	{
		if (!isset($this->assets[$type_code])) {
			throw new Exception('Asset type "'.$type_code.'" does not exist in the "'.$this->getPackageName().'" package.');
			trigger_localised_error('SYS0082', E_USER_ERROR, $type_code, $this->getPackageName());
			return Array();
		}

		$arr = Array();

		while (isset($this->assets[$type_code]) && $this->assets[$type_code]['parent_type'] != 'asset') {
			$type_code = $this->assets[$type_code]['parent_type'];
			$arr[]     = $type_code;
		}

		// if we are inheriting from something else then go find it from the installed list
		if (!isset($this->assets[$type_code])) {
			$new_arr = $GLOBALS['SQ_SYSTEM']->am->getTypeAncestors($type_code, TRUE, TRUE);
			foreach ($new_arr as $type_code) {
				$arr[] = $type_code;
			}

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
			throw new Exception('Asset type "'.$type_code.'" does not exist in the "'.$this->getPackageName().'" package.');
			trigger_localised_error('SYS0082', E_USER_ERROR, $type_code, $this->getPackageName());
			return ($field) ? null : Array();
		}

		return ($field) ? $this->assets[$type_code][$field] : $this->assets[$type_code];

	}//end getTypeInfo()


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
						if (!isset($prefs->pref_vars[$var])) {
							unset($preferences[$type_code][$var]);
						} else {
							if ($preferences[$type_code][$var]['name'] != $prefs->pref_vars[$var]['name']) {
								($preferences[$type_code][$var]['name'] = $prefs->pref_vars[$var]['name']);
							}
						}
					}
					foreach ($prefs->pref_vars as $var => $data) {
						if (!isset($preferences[$type_code][$var])) {
							$preferences[$type_code][$var] = $data;
						}
					}
				}
			}
		}//end foreach

	}//end installUserPreferences()


	/**
	* Returns an array of all the asset types in the package that are to be treated as system assets
	* NOTE: Assets will be installed in the order they appear in this list
	*
	* @return array
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

		foreach ($this->getSystemAssetTypes() as $type) {
			if (!$GLOBALS['SQ_SYSTEM']->am->isSystemAssetType($type)) {
				$function = 'create'.str_replace('_','',$type);
				$new_system_asset = $this->$function();
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


	/**
	* Generates the baked Database Abstraction Layer (DAL) queries for the package
	*
	* @return boolean
	* @access public
	*/
	function _installDALQueries()
	{
		$name = $this->getPackageName();
		if ($name == '__core__') $name = 'core';
		$old_path = ini_get('include_path');
		ini_set('include_path', SQ_LIB_PATH);
		require_once SQ_LIB_PATH.'/MatrixDAL/MatrixDALBaker.inc';
		$queries_found = MatrixDALBaker::addPackageQueries($name);
		if ($queries_found) {
			MatrixDALBaker::bakeQueriesFile($name.'_package');
		}
		ini_set('include_path', $old_path);

	}//end _installDALQueries()


}//end class

?>
