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
* $Id: image.inc,v 1.99 2013/10/14 04:33:25 cupreti Exp $
*
*/


require_once SQ_CORE_PACKAGE_PATH.'/files/file/file.inc';
require_once SQ_DATA_PATH.'/private/conf/tools.inc';
require_once SQ_CORE_PACKAGE_PATH.'/interfaces/bridge/bridge.inc';


/**
* Image
*
* Purpose
*
*
* @author  Greg Sherwood <greg@squiz.net>
* @version $Revision: 1.99 $
* @package MySource_Matrix_Packages
* @subpackage __core__
*/
class Image extends File implements Bridge
{

	/**
	* All allowed extensions for an image
	* @var Array(string)
	*/
	var $allowed_extensions = Array('gif', 'jpg', 'jpeg', 'png');


	/**
	* Constructor
	*
	* @param int	$assetid	the asset id to be loaded
	*
	*/
	function Image($assetid=0)
	{
		$this->_ser_attrs = TRUE;
		parent::__construct($assetid);

	}//end constructor


	/**
	* Processes the file upload from the source file so that file versioning cloned properly
	*
	* @param object		&$clone		the clone asset shell
	* @param array		$components	the wanted components to clone eg.
	* <pre>
	* 						Array(
	* 							'attributes',
	* 							'metadata_schemas',
	* 							'metadata',
	* 							'workflow',
	* 							'permissions',
	* 							'data',
	* 							'content_tags',
	* 							'roles',
	* 						);
	* 						or alternately
	* 						Array('all');
	* </pre>
	* @param boolean	$override	whether or not to override the existing permission, metadata schemas, workflow schemas with the new ones.
	*
	* @return boolean
	* @access public
	* @see asset::cloneComponents()
	*/
	function cloneComponents(Asset $clone, Array $components, $override=FALSE)
	{
		// need to go through the varieties that have already been added, and
		// re-upload/create them underneath this asset - to get the file
		// versioning correct

		 $GLOBALS['SQ_CLONE_COMPONENTS'] = TRUE;

		if (in_array('attributes', $components) || in_array('all', $components)) {

			$varieties = $this->attr('varieties');
			$old_base_path = $this->data_path.'/varieties';
			$new_base_path = $clone->data_path.'/varieties';

			create_directory($new_base_path);

			// file versioning magick :D
			$fv = $GLOBALS['SQ_SYSTEM']->getFileVersioning();

			if (isset($varieties['data']) && is_array($varieties['data'])) {
				foreach ($varieties['data'] as $variety_shadowid => $variety) {
					$old_variety_path = $old_base_path.'/'.$variety['filename'];
					$new_variety_path = $new_base_path.'/'.$variety['filename'];

					if (!copy($old_variety_path, $new_variety_path)) {
						$GLOBALS['SQ_CLONE_COMPONENTS'] = FALSE;
						trigger_localised_error('CORE0036', E_USER_WARNING, $old_variety_path, $new_variety_path);
						return FALSE;
					}

					if (!$fv->add($clone->data_path_suffix.'/varieties', $new_variety_path)) {
						$GLOBALS['SQ_CLONE_COMPONENTS'] = FALSE;
						return FALSE;
					}

					if (!$fv->checkOut($clone->data_path_suffix.'/varieties/'.$variety['filename'], $clone->data_path.'/varieties')) {
						$GLOBALS['SQ_CLONE_COMPONENTS'] = FALSE;
						trigger_localised_error('CORE0103', E_USER_WARNING);
						return FALSE;
					}
				}
			}
		}//end if attributes|all

		$GLOBALS['SQ_CLONE_COMPONENTS'] = FALSE;

		return parent::cloneComponents($clone, $components, $override);

	}//end cloneComponents()


	/**
	* Saves a copy of this asset at this point in time
	* This copy gets used when the asset is live and the user does not have write access
	*
	* @return boolean
	* @access public
	*/
	public function saveSystemVersion()
	{
		if (!parent::saveSystemVersion()) return FALSE;

		$varieties = $this->attr('varieties');
		if (!empty($varieties['lookups'])) {
			// make sure our varieties system directory exists
			if (!create_directory($this->data_path.'/.sq_system/varieties')) {
				trigger_localised_error('CORE0050', E_USER_WARNING, $this->name);
				return FALSE;
			}

			// make sure there is nothing in the varieties system directory
			if (!clear_directory($this->data_path.'/.sq_system/varieties')) {
				trigger_localised_error('CORE0046', E_USER_WARNING, $this->name);
				return FALSE;
			}

			// store the current version of each image variety file
			$fv = $GLOBALS['SQ_SYSTEM']->getFileVersioning();
			$versions = Array();

			foreach ($varieties['lookups'] as $varietyid) {
				$variety = $this->getAsset($this->id.':'.$varietyid);
				if (is_null($variety)) continue;

				$file_info = $fv->_checkOutCheck($variety->data_path_suffix.'/'.$variety->attr('filename'));
				$versions[$variety->attr('filename')] = $file_info['version'];
			}//end foreach

			if (!array_to_file($versions, 'versions', $this->data_path.'/.sq_system/varieties/sq_system_version_no')) {
				trigger_localised_error('CORE0051', E_USER_WARNING, $this->name);
				return FALSE;
			}
		}// end if

		return TRUE;

	}//end saveSystemVersion()


	/**
	* Replaces the editing version with the system (live) version
	*
	* @return boolean
	* @access public
	*/
	public function revertToSystemVersion()
	{
		//grab version info first before it is cleared
		if (is_file($this->data_path.'/.sq_system/varieties/sq_system_version_no')) {
			include ($this->data_path.'/.sq_system/varieties/sq_system_version_no');
		}

		if (!parent::revertToSystemVersion()) return FALSE;

		$varieties = $this->attr('varieties');
		if (!empty($varieties['lookups'])) {
			// make sure our varieties directory exists
			if (!create_directory($this->data_path.'/varieties')) {
				trigger_localised_error('CORE0049', E_USER_WARNING, $this->name);
				return FALSE;
			}

			// make sure our varieties directory is clear
			if (!clear_directory($this->data_path.'/varieties')) {
				trigger_localised_error('CORE0045', E_USER_WARNING, $this->name);
				return FALSE;
			}

			foreach ($varieties['lookups'] as $varietyid) {
				$variety = $this->getAsset($this->id.':'.$varietyid);
				if (is_null($variety)) continue;

				$file_version = isset($versions[$variety->attr('filename')]) ? $versions[$variety->attr('filename')] : NULL;
				$fv = $GLOBALS['SQ_SYSTEM']->getFileVersioning();

				// check out the old version of the file into the private data directory
				$rep_path = $variety->data_path_suffix.'/'.$variety->attr('filename');
				if (!$fv->checkOut($rep_path, $variety->data_path, $file_version)) {
					trigger_localised_error('CORE0044', E_USER_WARNING, $variety->attr('filename'));
					return FALSE;
				}

				// create a new versioning entry for the old version
				$file_info = $fv->_checkOutCheck($rep_path, $file_version);
				$fv->_updateFile($file_info['fileid'], $variety->data_path_suffix, $variety->data_path.'/'.$variety->attr('filename'));

				// check out the new version
				if (!$fv->checkOut($rep_path, $variety->data_path)) {
					trigger_localised_error('CORE0044', E_USER_WARNING, $variety->attr('filename'));
					return FALSE;
				}

			}//end foreach
		}

		return TRUE;

	}//end revertToSystemVersion()


	/**
	* Get an array of some useful data about the existing uploaded file
	*
	* @return array
	* @access private
	*/
	function getExistingFile()
	{
		$info = parent::getExistingFile();
		if (!empty($info['path'])) {
			$size = getimagesize($info['path']);
			$info['width']  = $size[0];
			$info['height'] = $size[1];
		}

		return $info;

	}//end getExistingFile()


	/**
	* Prints the tag for an image
	*
	* The output includes the width and height of the image, and the image
	* name for the alt tag
	*
	* @param string		$alt	the alt tag content. If empty the alt var is used
	* @param string		$title	the alt tag content. If empty the and $alt is set, then that is used, otherwise the title var is used
	* @param boolean	$return	if TRUE the tag will be returned
	*
	* @return mixed string|void
	* @access public
	*/
	function printImageTag($alt='', $title='', $return=FALSE)
	{
		if ($title == '') {
			$title = ($alt != '') ? $alt : $this->attr('title');
		}
		if ($alt == '') $alt = $this->attr('alt');

		$tag = '<img src="'.$this->getURL().'" width="'.$this->attr('width').'" height="'.$this->attr('height').'" alt="'.htmlentities($alt).'" title="'.htmlentities($title).'" />';
		if ($return) return $tag;
		echo $tag;

	}//end printImageTag()


	/**
	* Prints out the backend interface for the asset
	*
	* @param object	&$o	A reference to the backend outputter
	*
	* @return boolean
	* @access public
	*/
	function paintBackend(Backend_Outputter $o)
	{
		if (!isset($_REQUEST['asset_ei_screen'])) {
			return parent::paintBackend($o);
		}

		switch ($_REQUEST['asset_ei_screen']) {
			case 'image_info' :
				include_once SQ_FUDGE_PATH.'/var_serialise/var_serialise.inc';
				$output = Array();
				$output['alt']    = $this->attr('alt');
				$output['width']  = $this->attr('width');
				$output['height'] = $this->attr('height');
				$output['caption'] = $this->attr('caption');
				$output['name']   = $this->name;

				echo var_serialise($output);
				exit();
			default :
				return parent::paintBackend($o);

		}//end switch

	}//end paintBackend()


//--        VARIETIES        --//


	/**
	* Creates/Updates a variety for this image
	*
	* If the variety has an ID then it must belong to this asset in order to be saved.
	* Returns FALSE on error, shadow assetid on success
	*
	* @param object		$variety			the variety to save
	* @param boolean	$recreate_image		hether to generate the image file or not
	* @param boolean	$save_attrs			whether to run saveAttributes() or not,
	*										usually you only would set this to FALSE if you are running
	*										this multiple times and only want to save once at the end
	* @param boolean	$file_versioning	the argument which will be passed onto image_variety to tell
	*										it not to use file_versioning
	* @param boolean	$check_write_access	it is FALSE when we dont want to check the write access of the user
	*
	* @return mixed boolean|int
	* @access public
	*/
	function saveVariety($variety, $recreate_image=TRUE, $save_attrs=TRUE, $file_versioning=TRUE, $check_write_access=TRUE)
	{
		if ($check_write_access && !$this->writeAccess('attributes')) {
			trigger_localised_error('CORE0069', E_USER_WARNING, $this->name, $this->id);
			return FALSE;

		}

		// use attr() instead of just ->name because if the variety hasn't been created yet it won't have a name
		$name = $variety->attr('name');
		if (trim($name) == '') {
			trigger_localised_error('CORE0100', E_USER_WARNING);
			return FALSE;

		}

		$varieties = $this->attr('varieties');

		// check if this is already one of our varieties
		if ($variety->id) {
			list($assetid, $varietyid) = explode(':', $variety->id, 2);
			if ($assetid != $this->id) {
				trigger_localised_error('CORE0099', E_USER_WARNING);
				return FALSE;

			}

			if (isset($varieties['data'][$varietyid])) {
				if ($varieties['data'][$varietyid]['name'] != $name) {
					trigger_localised_error('CORE0097', E_USER_WARNING, $varietyid);
					return FALSE;

				}
			} else {
				trigger_localised_error('CORE0098', E_USER_WARNING, $varietyid, $this->name, $this->id);
				return FALSE;

			}

		} else {

			if (isset($varieties['lookups'][$name])) {
				$varietyid = $varieties['lookups'][$name];
			} else {
				$varietyid = 'v'.$this->attr('variety_count');
				if (!isset($varieties['lookups'])) {
					$varieties['lookups'] = Array();
				}
				$varieties['lookups'][$name] = $varietyid;

				if (!$this->setAttrValue('variety_count', $this->attr('variety_count') + 1)) {
					return FALSE;
				}

			}

		}//end if

		if (!isset($varieties['data'])) {
			$varieties['data'] = Array();
		}
		$varieties['data'][$varietyid] = Array();
		foreach ($variety->vars as $attr_name => $attr_data) {
			$varieties['data'][$varietyid][$attr_name] = $attr_data['value'];
		}

		if (!$this->setAttrValue('varieties', $varieties)) {
			return FALSE;
		}

		$variety = $this->getAsset($this->id.':'.$varietyid);
		if (is_null($variety)) return FALSE;

		$GLOBALS['SQ_SYSTEM']->changeDatabaseConnection('db2');
		$GLOBALS['SQ_SYSTEM']->doTransaction('BEGIN');

		// create the actual scaled image if necessary and add any lookups
		if ($recreate_image) {
			if (!$variety->updateImage($file_versioning)) {
				$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
				$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();
				return FALSE;

			}

			// Now get the variables again because updateImage() can update stuff
			foreach ($variety->vars as $attr_name => $attr_data) {
				$varieties['data'][$varietyid][$attr_name] = $attr_data['value'];
			}
			if (!$this->setAttrValue('varieties', $varieties)) {
				return FALSE;
			}
		}

		if ($save_attrs && !$this->saveAttributes()) {
			$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
			$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();
			return FALSE;
		}

		$GLOBALS['SQ_SYSTEM']->doTransaction('COMMIT');
		$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();
		return $varietyid;

	}//end saveVariety()


	/**
	* Deletes a variety from this image
	*
	* @param string	$varietyid	the shadow assetid or just the shadow part of the variety to delete
	*
	* @return boolean
	* @access public
	*/
	function deleteVariety($varietyid)
	{
		if (!$this->writeAccess('attributes')) {
			trigger_localised_error('CORE0070', E_USER_WARNING, $this->name, $this->id);
			return FALSE;
		}

		if (strpos($varietyid, ':') !== FALSE) {
			list($assetid, $varietyid) = explode(':', $varietyid, 2);
			if ($assetid != $this->id) {
				trigger_localised_error('CORE0088', E_USER_WARNING);
				return FALSE;
			}
		}

		$variety = $this->getAsset($this->id.':'.$varietyid);
		if (is_null($variety)) return FALSE;

		$GLOBALS['SQ_SYSTEM']->changeDatabaseConnection('db2');
		$GLOBALS['SQ_SYSTEM']->doTransaction('BEGIN');

		// delete the actual scaled image files
		if (!$variety->deleteImage()) {
			$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
			$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();
			return FALSE;
		}

		$varieties = $this->attr('varieties');

		if (!isset($varieties['data'][$varietyid])) {
			trigger_localised_error('CORE87', $varietyid, E_USER_WARNING);
			$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
			$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();
			return FALSE;
		}

		unset($varieties['data'][$varietyid]);
		$lookup_name = array_search($varietyid, $varieties['lookups']);
		if ($lookup_name !== FALSE) {
			unset($varieties['lookups'][$lookup_name]);
		}

		if (!$this->setAttrValue('varieties', $varieties) || !$this->saveAttributes()) {
			$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
			$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();
			return FALSE;
		}

		if (!$this->updateLookups()) {
			$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
			$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();
			return FALSE;
		}

		$GLOBALS['SQ_SYSTEM']->doTransaction('COMMIT');
		$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();
		return TRUE;

	}//end deleteVariety()


//--        VARIETY FILES        --//


	/**
	* Look's after the placing and removing of out files in the public directory
	*
	* @return boolean
	* @access private
	*/
	protected function _checkFileState($filename='', $update_lookups=TRUE)
	{
		if ($this->status & SQ_SC_STATUS_SAFE_EDITING) return TRUE; //don't touch public file in safe edit

		$varieties = $this->attr('varieties');
		if (!empty($varieties)) {
			foreach ($varieties['lookups'] as $varietyid) {
				$variety = $this->getAsset($this->id.':'.$varietyid);
				if (is_null($variety)) continue;

				if (!$variety->checkFileState($this->effectiveUnrestricted() && $this->attr('allow_unrestricted'))) {
					return FALSE;
				}
			}
		}

		return parent::_checkFileState($filename, $update_lookups);

	}//end _checkFileState()


	/**
	* Called to force an update of this assets lookup information
	*
	* @return boolean
	* @access public
	* @see Asset::updateLookups(), Asset::saveWebPaths()
	*/
	function updateLookups($auto_add_remaps = TRUE)
	{
		$GLOBALS['SQ_SYSTEM']->changeDatabaseConnection('db2');
		$GLOBALS['SQ_SYSTEM']->doTransaction('BEGIN');

		if (!parent::updateLookups($auto_add_remaps)) {
			$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
			$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();
			return FALSE;
		}

		$varieties = $this->attr('varieties');
		if (!empty($varieties)) {
			foreach ($varieties['lookups'] as $varietyid) {
				$variety = $this->getAsset($this->id.':'.$varietyid);
				if (is_null($variety)) continue;

				if (!$variety->updateLookups()) {
					$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
					$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();
					return FALSE;
				}

			}//end foreach
		}// end if

		$GLOBALS['SQ_SYSTEM']->doTransaction('COMMIT');
		$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();
		return TRUE;

	}//end updateLookups()


//--        BRIDGE INTERFACE        --//


	/**
	* Returns a reference to the asset represented by the passed assetid
	* (Asset or NULL on error)
	*
	* @param int		$assetid		the shadow part asset id to be loaded (i.e part after the ':')
	* @param string		$type_code		if this exists then this object is used to
	*									load the asset, if not then the DB is queried
	*									to find out the asset type
	* @param boolean	$mute_errors	stops the outputting of errors in this fn, needed because
	*									you can't use the '@' operator when returning by reference
	*
	* @return object
	* @access public
	* @see Asset_Manager::&getAsset()
	*/
	function getAsset($assetid, $type_code='', $mute_errors=FALSE)
	{
		$asset = NULL;
		$id_parts = explode(':', $assetid);

		if (isset($id_parts[1])) {
			$shadowid = $id_parts[1];
		} else {
			return $asset;
		}

		// Varieties are preceded with a 'v'
		if ($shadowid{0} == 'v') {

			$varieties = $this->attr('varieties');

			if (!isset($varieties['data'][$shadowid])) {
				if(!$mute_errors) {
					trigger_localised_error('CORE0073', E_USER_WARNING, $this->id, $shadowid);
				}
				return $asset;
			}

			$GLOBALS['SQ_SYSTEM']->am->includeAsset('image_variety');
			$asset = new Image_Variety($this->id.':'.$shadowid, $varieties['data'][$shadowid]);

		}//end if

		return $asset;

	}//end getAsset()


	/**
	* Return all links that this asset has to other assets
	*
	* @param int			$assetid			the shadow part asset id to be loaded (i.e part after the ':') of the the $side_of_link asset
	* @param int			$link_types			integer that can be the product of bitwise operations
	*											on the SQ_LINK_* constants
	* @param string|array	$type_code			the type of asset that is linked
	*											(eg 'Page', 'File', etc)
	*											if an array returns link if matches any
	*											of the array values
	* @param boolean		$strict_type_code	whether we are finding assets that are just a $type_code
	*											or $type_code and any of it's sub-classes
	* @param string			$side_of_link		Which side of the link this (the current) asset
	*											is on ('major' or 'minor')
	* @param string			$sort_by			a field in the asset table you would like the results
	*											sorted by (eg. name, short_name etc)
	*
	* @return array
	* @access public
	*/
	function getLinks($assetid, $link_types, $type_code='', $strict_type_code=TRUE, $side_of_link='major', $sort_by=NULL)
	{
		// only TYPE_2 links are ever returned
		if (!($link_types & SQ_LINK_TYPE_2)) {
			return Array();
		}

		// we can check the type codes outside the loop because they are only ever going to be a single type
		if ($type_code) {
			// if we are looking for children, they will be image_variety,
			// if we are looking for parent, they will whatever this class is
			$ret_type_code = ($side_of_link == 'major') ? 'image_variety' : $this->type();
			if ($strict_type_code) {
				if (is_array($type_code)) {
					if (!in_array($ret_type_code, $type_code)) {
						return Array();
					}
				} else {
					if ($ret_type_code != $type_code) return Array();
				}
			} else {
				$descendants = $GLOBALS['SQ_SYSTEM']->am->getTypeDescendants($type_code, TRUE);
				if (!in_array($ret_type_code, $descendants)) {
					return Array();
				}
			}

		}//end if type_code

		if (!isset($this->_tmp['getLinks'][$assetid][$side_of_link])) {

			$links = Array();
			$varieties = $this->attr('varieties');

			if (!empty($varieties)) {

				// if we are looking from this asset and not a shadow asset
				if ($this->id == $assetid) {
					if ($side_of_link == 'minor') {
						// should never happen...but anyway
						trigger_localised_error('CORE0089', E_USER_ERROR);

					} else {
						$i = 0;
						foreach ($varieties['data'] as $varid => $data) {
							$links[] = Array(
										'linkid'			=> 0,
										'minorid'			=> $this->id.':'.$varid,
										'minor_type_code'	=> 'image_variety',
										'majorid'			=> $this->id,
										'value'				=> '',
										'link_type'			=> SQ_LINK_TYPE_2,
										'is_dependant'		=> TRUE,
										'is_exclusive'		=> TRUE,
										'sort_order'		=> $i,
										'locked'			=> 0,
									   );
							$i++;
						}// end foreach

					}// endif


				// looking from shadow asset
				} else {

					// because the varieties are only ever going to be one level down
					// if we are looking up the tree all we can see is this image asset
					if ($side_of_link == 'minor') {
						list(,$shadowid) = explode(':', $assetid);

						if (!isset($varieties['data'][$shadowid])) {
							trigger_localised_error('CORE0074', E_USER_WARNING, $assetid, $GLOBALS['SQ_SYSTEM']->am->getTypeInfo($this->type(), 'name'), $this->id);
							return Array();

						} else {
							$links[] = Array(
										'linkid'			=> 0,
										'majorid'			=> $this->id,
										'major_type_code'	=> $this->type(),
										'value'				=> '',
										'link_type'			=> SQ_LINK_TYPE_2,
										'is_dependant'		=> TRUE,
										'is_exclusive'		=> TRUE,
										'sort_order'		=> array_search($assetid, array_keys($varieties['data'])),
										'locked'			=> 0,
									   );
						}//endif

					// because the varieties are only ever going to be one level down
					// if we are looking down the tree there will be nothing
					} else {
						$links = Array();

					}//endif

				}//end else - looking from shadow asset

				if (!is_null($sort_by)) {
					uasort($links, function($a, $b) use ($sort_by) {
						return $a[$sort_by] < $b[$sort_by];
					});
				}

			}//end if

			$this->_tmp['getLinks'][$assetid][$side_of_link] = $links;

		}//end if

		return $this->_tmp['getLinks'][$assetid][$side_of_link];

	}//end getLinks()


	/**
	* Get all asset ids that are below the passed assetid in the various trees in which it exists
	* Returns an array of assetids and their type code
	*
	* @param int			$assetid			the id of the asset to get its children for
	* @param string|array	$type_code			the type of asset that is linked
	*											(eg 'Page', 'File', etc)
	*											if an array returns link if matches any
	*											of the array values
	* @param boolean		$strict_type_code	whether we are finding assets that are just a $type_code
	*											or $type_code and any of it's sub-classes
	* @param boolean		$dependant			if TRUE, results will be filtered to assets that are dependants.
	*											If FALSE, results will all not be dependants.
	*											If NULL, results will not be filtered
	* @param string			$sort_by			a field in the asset table you would like the results
	*											sorted by (eg. name, short_name etc)
	*
	* @return array
	* @access public
	* @see Asset_Manager::getChildren()
	*/
	function getChildren($assetid, $type_code='', $strict_type_code=TRUE, $dependant=NULL, $sort_by=NULL)
	{
		// if this is a shadow asset, then it has no children
		if (!is_numeric($assetid)) return Array();

		// we can check the type codes outside the loop because they are only ever going to be a single type
		if ($type_code) {

			$ret_type_code = 'image_variety';
			if ($strict_type_code) {
				if (is_array($type_code)) {
					if (!in_array($ret_type_code, $type_code)) {
						return Array();
					}
				} else {
					if ($ret_type_code != $type_code) return Array();
				}
			} else {
				$descendants = $GLOBALS['SQ_SYSTEM']->am->getTypeDescendants($type_code, TRUE);
				if (!in_array($ret_type_code, $descendants)) {
					return Array();
				}
			}

		}//end if type_code

		$children = Array();

		$varieties = $this->attr('varieties');
		if (!empty($varieties)) {
			foreach ($varieties['data'] as $varid => $data) {
				$children[$this->id.':'.$varid] = Array(Array('type_code' => 'image_variety'));
			}// end foreach
		}

		return $children;

	}//end getChildren()


	/**
	* Get all asset ids that are above the passed assetid in the various trees in which it exists
	*
	* Can only find parents of type <i>form_section</i> or <i>form</i>
	*
	* @param int			$assetid			the id of the asset to get its parents for
	* @param string|array	$type_code			the type of asset that is linked (eg 'Form_Section', 'Form_Question', etc)
	*											if an array returns link if matches any of the array values
	* @param boolean		$strict_type_code	whether we are finding assets that are just a $type_code
	*											or $type_code and any of it's sub-classes
	*
	* @return array
	* @access public
	* @see Asset_Manager::getParents()
	*/
	function getParents($assetid, $type_code='', $strict_type_code=TRUE)
	{
		if ($assetid == $this->id || strpos($assetid, ':v') === FALSE) {
			// should never happen...but anyway
			trigger_localised_error('CORE0090', E_USER_ERROR);
		}

		$parents = $GLOBALS['SQ_SYSTEM']->am->getParents($this->id, $type_code, $strict_type_code);

		// Now add this class only if it is a valid type code
		if ($type_code) {

			$ret_type_code = $this->type();
			if ($strict_type_code) {
				if (is_array($type_code)) {
					if (!in_array($ret_type_code, $type_code)) {
						return $parents;
					}
				} else {
					if ($ret_type_code != $type_code) return $parents;
				}
			} else {
				$descendants = $GLOBALS['SQ_SYSTEM']->am->getTypeDescendants($type_code, TRUE);
				if (!in_array($ret_type_code, $descendants)) {
					return $parents;
				}
			}

		}//end if type_code

		$parents[] = $this->id;

		return $parents;

	}//end getParents()


	/**
	* Prints the body of the asset
	*
	* @return void
	* @access public
	*/
	function printBody()
	{
		$this->printImageTag();

	}//end printBody()


	/**
	* Return the keywords for this asset type.
	* This function is called in asset-listing situations like the asset listing, search page etc
	*
	* The return value is in the form:
	* <pre>
	* Array(
	*   'name' => 'description' => 'the asset name',
	* );
	* </pre>
	*
	* @return array
	* @access public
	*/
	function getAvailableKeywords()
	{
		$keywords = parent::getAvailableKeywords();

		$varieties = $this->attr('varieties');
		if (!empty($varieties)) {
			foreach ($varieties['lookups'] as $varietyid) {
				$variety = $this->getAsset($this->id.':'.$varietyid);
				if (is_null($variety)) continue;
				$keywords['image_v_'.$variety->name] = 'Image variety '.$variety->name;
				$keywords['image_v_'.$variety->name.'_url'] = 'Image variety '.$variety->name.' URL';
				$keywords['image_v_'.$variety->name.'_width'] = 'Image variety '.$variety->name.' Width';
				$keywords['image_v_'.$variety->name.'_height'] = 'Image variety '.$variety->name.' Height';
			}
		}
		$keywords['image_width']	= 'Width of the image in pixels';
		$keywords['image_height']	= 'Height of the image in pixels';
		$keywords['image_tag']		= 'HTML IMG tag to display this image';

		$embedded_data = $this->attr('embedded_data');
		if (!empty($embedded_data)) {
			foreach ($embedded_data as $keyword => $value_set) {
				if (empty($value_set)) continue;

				$keywords['image_embedded_'.$keyword] = 'Embedded Data: "'.$value_set['name'].'"';
			}
		}

		return $keywords;

	}//end getAvailableKeywords()


	/**
	* Get Image Width keyword replacement
	*
	* @return string
	* @access public
	*/
	function getImageWidthKeywordReplacement()
	{
		return $this->attr('width');

	}//end getImageWidthKeywordReplacement()


	/**
	* Get Image Height keyword replacement
	*
	* @return string
	* @access public
	*/
	function getImageHeightKeywordReplacement()
	{
		return $this->attr('height');

	}//end getImageHeightKeywordReplacement()


	/**
	* Get Image Tag keyword replacement
	*
	* @return string
	* @access public
	*/
	function getImageTagKeywordReplacement()
	{
		return $this->printImageTag('', '', TRUE);

	}//end getImageTagKeywordReplacement()


	/**
	* Get bodycopy keyword replacement
	*
	* Provides a mechanism for providing keyword replacement functions by
	* looking up the function named 'getKeywordNameKeywordReplacement'
	* so an asset wishing to add support for 'best_ever' custom keyword
	* should implement a function getBestEverKeywordReplacement()
	* which returns a replacement string
	*
	* @param string	$keyword	Keyword to find a replacement for
	*
	* @return string
	* @access public
	*/
	function getKeywordReplacement($keyword)
	{
		$replacement = NULL;

		// Remove any modifiers from keyword...saving the full keyword for
		// later in case we have to pass it to the parent asset type
		$full_keyword = $keyword;
		$keyword = parse_keyword($keyword, $modifiers);
		$contextid = extract_context_modifier($modifiers);

		if ($contextid !== NULL) {
			// If we were able to extract a context ID to change to, and it's
			// different to our current one, then change and then reload a copy
			// of the asset in that context (as we would need to do anyway)

			if ((int)$contextid !== $GLOBALS['SQ_SYSTEM']->getContextId()) {
				$GLOBALS['SQ_SYSTEM']->changeContext($contextid);
				$contexted_asset = $GLOBALS['SQ_SYSTEM']->am->getAsset($this->id);

				// Get the keyword without any modifiers
				$replacement = $contexted_asset->getKeywordReplacement($keyword);

				// Then apply the ones we had remaining, then return it
				apply_keyword_modifiers($replacement, $modifiers, Array('assetid' => $contexted_asset->id));

				unset($contexted_asset);
				$GLOBALS['SQ_SYSTEM']->restoreContext();
				return $replacement;

			}//end if contextid is not the currently active context

		}//end if contextid is not NULL

		if (0 === strpos($keyword, 'image_v_')) {
			$varieties = $this->attr('varieties');
			if (empty($varieties)) return '';
			$variety_name = substr($keyword, 8);
			$url_only = FALSE;
			if (substr($variety_name, -4) == '_url') {
				$variety_name = substr($variety_name, 0, strlen($variety_name) - 4);
				$url_only = TRUE;
			} else	if (substr($variety_name, -6) == '_width') {
				$variety_name = substr($variety_name, 0, strlen($variety_name) - 6);
				$size = $this->_varietyImageSize($variety_name);
				if (isset($size[0])) $replacement = $size[0];
			} else if (substr($variety_name, -7) == '_height') {
				$variety_name = substr($variety_name, 0, strlen($variety_name) - 7);
				$size = $this->_varietyImageSize($variety_name);
				if (isset($size[1])) $replacement = $size[1];
			}
			$variety_id = array_get_index($varieties['lookups'], $variety_name);
			if (!empty($variety_id) && !$replacement) {
				$variety = $GLOBALS['SQ_SYSTEM']->am->getAsset($this->id.':'.$variety_id);
				if ($url_only) {
					$string = $variety->getURL();
				} else {
					$string = $variety->printImageTag($this->attr('alt'), $this->attr('title'), TRUE);
				}
				$replacement = $string;
			}
		} else if (0 === strpos($keyword, 'image_embedded_')) {
			$embedded_key = substr($keyword, strlen('image_embedded_'));
			$replacement = $this->_getEmbeddedKeywordReplacement($embedded_key);
		}

		if ($replacement !== NULL) {
			if (count($modifiers) > 0) {
				apply_keyword_modifiers($replacement, $modifiers, Array('assetid' => $this->id));
			}
		} else {
			// Use the full keyword to send up, so modifiers still get respected
			$replacement = parent::getKeywordReplacement($full_keyword);
		}

		return $replacement;

	}//end getKeywordReplacement()


	/**
	* Get the image size of an image variety
	*
	* @param string	$variety_name	Asset name of the variety
	*
	* @return string
	* @access private
	*/
	function _varietyImageSize($variety_name)
	{
		$varieties = $this->attr('varieties');
		if (isset($varieties['data'][$varieties['lookups'][$variety_name]]['filename'])) {
			$filename = $varieties['data'][$varieties['lookups'][$variety_name]]['filename'];
			$varietyid = $varieties['lookups'][$variety_name];
			$variety = $this->getAsset($this->id.':'.$varietyid);

			if ($this->useSystemVersion()) {
				if (is_file($this->data_path.'/varieties/sq_system_version_no')) {
					include ($this->data_path.'/varieties/sq_system_version_no');
				}
				$file_version = isset($versions[$filename]) ? $versions[$filename] : NULL;
				$fv = $GLOBALS['SQ_SYSTEM']->getFileVersioning();
				$rep_path = $variety->data_path_suffix.'/'.$filename;
				if (!$fv->checkOut($rep_path, $variety->data_path, $file_version)) {
					trigger_localised_error('CORE0044', E_USER_WARNING, $variety->attr('filename'));
					return FALSE;
				}

				// create a new versioning entry for the old version
				$file_info = $fv->_checkOutCheck($rep_path, $file_version);
				$size = getimagesize($file_info['source_file']);
			} else {
				$size = getimagesize($this->data_path.'/varieties/'.$filename);
			}

			return $size;
		}

	}//end _varietyImageSize()


	/**
	* Get replacement for an embedded keyword
	*
	* @param string	$keyword	Keyword to find a replacement for
	*
	* @return string
	* @access public
	*/
	function _getEmbeddedKeywordReplacement($keyword)
	{
		$replacement = '';

		$embedded_data = $this->attr('embedded_data');
		$data_set = array_get_index($embedded_data, $keyword);
		if (is_null($data_set)) return $replacement;

		$replacement = implode(',', $data_set['value']);

		return $replacement;

	}//end _getEmbeddedKeywordReplacement()


	/**
	* Returns an array of assetid's + info in the order that they propogate out from THIS BRIDGE'S url
	*
	* @param string	$assetid	the id of the last asset in the lineage
	* @param string	$protocol	the protocol to match -> NULL means it is ignored
	* @param string	$url		the url to check for -> NULL defaults it to current url
	*
	* @return array
	* @access public
	*/
	function getLineageFromURL($assetid, $protocol, $url)
	{
		return Array();

	}//end getLineageFromURL()


	/**
	 * Returns link lineage information for assets, relative to this bridge
	 *
	 * Also returns basic information about the last link (linkid and link type)
	 * linkid => Array (
	 *			  'linkid'     => linkid,
	 *			  'link_type'  => link_type,
	 *			  'link_value' => link_value,
	 *			  'lineage'    => Array(...),
	 *			 )
	 *
	 * This shouldn't be called directly - use Asset_Manager's function and it'll
	 * work them out. If called directly, lineages and tree IDs will be based from
	 * this bridge
	 *
	 * @param string		$assetid			Full asset ID of the shadow asset
	 * @param int			$result_limit		limits the number of results returned
	 * @param string|array	$fields				Returns these asset field(s) in lineage details (e.g name, type_code)
	 * @param boolean		$only_significant	Only return the significant links, and ignore notice links
	 *
	 * @return array
	 * @access public
	 * @see Asset_Manager::getLinkLineages()
	 */
	function getLinkLineages($assetid, $result_limit=0, $fields='name', $only_significant=FALSE)
	{
		$lineages = Array();

		$varieties_attr = $this->attr('varieties');
		$shadow_parts = explode(':', $assetid, 2);
		if ((string) $shadow_parts[0] === (string) $this->id) {
			// Correct asset.
			if ($shadow_parts[1]{0} === 'v') {
				// Varieties start with "v"
				if (array_key_exists($shadow_parts[1], $varieties_attr['data']) === TRUE) {
					// This shadow asset exists.
					include_once SQ_INCLUDE_PATH.'/general_occasional.inc';

					$varietyNum = (int) substr($shadow_parts[1], 1);
					$treeid     = asset_link_treeid_convert($varietyNum - 1, TRUE);

					$linkid     = $this->id.':'.$assetid;
					$lineage    = Array(
								   'linkid'     => $linkid,
								   'link_type'  => SQ_LINK_TYPE_2,
								   'link_value' => '',
								   'lineage'    => Array(),
								  );

					if (is_array($fields) === TRUE) {
						foreach ($fields as $field) {
							if ($field === 'type_code') {
								$lineage['lineage'][$assetid][$field] = 'image_variety';
							} else {
								$lineage['lineage'][$assetid][$field] = array_get_index($varieties_attr['data'][$shadow_parts[1]], $field, '');
							}
						}
					} else {
						if ($fields === 'type_code') {
							$lineage['lineage'][$assetid] = 'image_variety';
						} else {
							$lineage['lineage'][$assetid] = array_get_index($varieties_attr['data'][$shadow_parts[1]], $fields, '');
						}
					}

					$lineages[$linkid] = $lineage;
				}
			}
		}

		return $lineages;

	}//end getLinkLineages()


	/**
	* Returns an array of all user or user_group assets that have the passed permission for the passed asset
	*
	* @param int		$assetid		the assetid for asset whose permissions to get
	* @param string		$permission		the permission code you are getting
	* @param boolean	$granted		type of Access : NULL = all, TRUE = granted, FALSE = denied
	* @param boolean	$and_greater	get effective permission (eg read access = read
	*									or write or admin)
	* @param boolean	$expand_groups	expand user groups so only user ids get returned
	*									(NOTE: only valid if $all_info is FALSE)
	* @param boolean	$all_info		when FALSE fn just returns an array of userids for
	*									those that have permission
	*									When TRUE fn returns all info about the permission in the form of
	*									Array(userid => granted)
	*									(NOTE: TRUE is only valid if $and_greater and
	*									$expand_groups are FALSE)
	*
	* @return array
	* @access public
	*/
	function getPermission($assetid, $permission, $granted=NULL, $and_greater=TRUE, $expand_groups=FALSE, $all_info=FALSE)
	{
		return Array();

	}//end getPermission()


	/**
	* Sets a permission for the passed user or user_group
	*
	* @param int		$assetid	the assetid for asset whose permissions to set
	* @param int		$userid		the id of the user or user_group to add the permission for
	* @param string		$permission	the permission code you are adding
	* @param boolean	$granted	whether this permission grants access or not
	*
	* @return boolean
	* @access public
	*/
	function setPermission($assetid, $userid, $permission, $granted)
	{
		return FALSE;

	}//end setPermission()


	/**
	* Deletes a permission with the passed user or user_group
	*
	* @param int	$assetid	the assetid for asset whose permissions to delete
	* @param int	$userid		the id of the user or user_group to remove the permission from
	* @param string	$permission	the permission code you are deleting
	*
	* @return boolean
	* @access public
	*/
	function deletePermission($assetid, $userid, $permission)
	{
		return FALSE;

	}//end deletePermission()


	/**
	* Returns an array of information about assets of the passed codes
	*
	* If one value in $field then the return value is Array(assetid => '[field]'), otherwise Array(assetid => Array())
	*
	* @param array		$assetids			an array of assets to limit
	* @param mixed		$type_code			an asset type code, or array of typecodes, to limit results to
	* @param boolean	$strict_type_code	ensure returned assets are of the passed type codes
	* @param string		$field				single field from the asset table to return (along with the assetid)
	*										Default is all fields
	*
	* @return array
	* @access public
	*/
	function getAssetInfo($assetids, $type_code=Array(), $strict_type_code=TRUE, $field='')
	{
		$info = Array();
		foreach ($assetids as $assetid) {
			$asset = $this->getAsset($assetid, '', TRUE);

			// Only add if there is an asset to get info for
			if ($asset !== NULL) {
				if (!empty($field)) {
					if ($field == 'assetid') {
						$info[$assetid] = $assetid;
					} else if ($field == 'type_code') {
						$info[$assetid] = $asset->type();
					} else if (in_array($field, Array('version', 'name', 'short_name', 'status'))) {
						$info[$assetid] = $asset->$field;
					} else {
						$info[$assetid] = $this->$field;
					}
				} else {
					$info[$assetid] = Array(
										'assetid'				=> $assetid,
										'type_code'				=> $asset->type(),
										'version'				=> $asset->version,
										'name'					=> $asset->name,
										'short_name'			=> $asset->short_name,
										'status'				=> $asset->status,
										'languages'				=> $asset->languages,
										'charset'				=> $asset->charset,
										'force_secure'			=> $this->force_secure,
										'created'				=> $asset->created,
										'created_userid'		=> $asset->created_userid,
										'updated'				=> $asset->updated,
										'updated_userid'		=> $asset->updated_userid,
										'published'				=> $asset->published,
										'published_userid'		=> $asset->published_userid,
										'status_changed'		=> $asset->status_changed,
										'status_changed_userid'	=> $asset->status_changed_userid,
									  );
				}

			}

		}//end foreach

		return $info;

	}//end getAssetInfo()


	/**
	* Determine if the passed assetid or assetids refer(s) to a valid asset in the system
	*
	* This function shouldn't be called directly - use the Asset Manager's equivalent
	* function. It will send this function the full asset ID of any shadow assets.
	*
	* Return TRUE/FALSE if passed a single assetid, or if an array return an array of
	* assetids that exist.
	*
	* @param mixed	$assetids	An assetid (or array of asset IDs) to check.
	*
	* @return mixed boolean|array
	* @access public
	* @see Asset_Manager::assetExists()
	*/
	function assetExists($assetids)
	{
		$my_assetids  = Array();
		$return_array = TRUE;
		if (is_array($assetids) === FALSE) {
			$return_array = FALSE;
			$assetids  = Array($assetids);
		}

		$varieties_attr = $this->attr('varieties');
		foreach ($assetids as $assetid) {
			$shadow_parts = explode(':', $assetid, 2);
			if ((string) $shadow_parts[0] === (string) $this->id) {
				// Correct asset.
				if ($shadow_parts[1]{0} === 'v') {
					// Varieties start with "v"
					if (array_key_exists($shadow_parts[1], $varieties_attr['data']) === TRUE) {
						$my_assetids[] = $assetid;
					}
				}
			}
		}

		if ($return_array === TRUE) {
			return $return_array;
		} else {
			if (count($my_assetids) > 0) {
				return TRUE;
			} else {
				return FALSE;
			}
		}//end if

	}//end assetExists()


	/**
	* Get details of a link with the specified characteristics
	*
	* @param int		$assetid			id of the the $side_of_link asset
	* @param int		$link_type			integer that should be a single integer of the SQ_LINK_* constants
	* @param mixed		$type_code			the type of asset that is linked
	*										(eg 'Page', 'File', etc)
	*										if an array returns link if matches any
	*										of the array values
	* @param boolean	$strict_type_code	whether we are finding an asset that
	*										is just a $type_code or potentially an inherited type
	*										or $type_code and any of it's sub-classes
	* @param string		$value				the value that is associated with this link
	* @param string		$side_of_link		Which side of the link this (the current) asset
	*										is on ('major' or 'minor')
	* @param boolean	$exclusive			The exclusive status for the link must be
	*										this (if not NULL)
	*
	* @return array
	* @access public
	*/
	function getLink($assetid, $link_type=NULL, $type_code='', $strict_type_code=TRUE, $value=NULL, $side_of_link='major', $exclusive=NULL)
	{
		return Array();

	}//end getLink()


	/**
	* Get details of the link with specified linkid
	*
	* @param int	$linkid			id of the link were returning
	* @param int	$assetid		id of the the $side_of_link asset
	*								if zero no check is made and both major and minor information is returned
	* @param string	$side_of_link	Which side of the link the asset represented by $assetid is on ('major' or 'minor')
	*
	* @return array
	* @access public
	*/
	function getLinkById($linkid, $assetid=0, $side_of_link='major')
	{
		return Array();

	}//end getLinkById()


	/**
	* Return the number of links in the system that involve the specified asset
	*
	* Can be restricted by either link type afnd/or asset type code
	*
	* @param int		$assetid			id of the the $side_of_link asset
	* @param string		$side_of_link		Which side of the link this (the current) asset
	*										is on ('major' or 'minor')
	* @param int		$link_types			integer that can be the product of bitwise operations
	*										on the SQ_LINK_* constants
	* @param mixed		$type_code			the type of asset that is linked (eg 'Page', 'File', etc)
	*										if an array returns link if matches any
	*										of the array values
	* @param boolean	$strict_type_code	whether we are finding assets that are just a $type_code
	*										or $type_code and any of it's sub-classes
	* @param int		$ignore_linkid		ignore the link represented by this link
	*										id when returning the count
	*
	* @return int
	* @access public
	*/
	function countLinks($assetid, $side_of_link='major', $link_types=0, $type_code='', $strict_type_code=TRUE, $ignore_linkid=0)
	{
		return 0;

	}//end countLinks()


	/**
	* Get details of the link(s) between the two specified assets
	*
	* @param int		$assetid		id of the the $side_of_link asset
	* @param int		$other_assetid	the asset on the other side of the link
	* @param int		$link_types		integer that can be the product of bitwise operations
	*									on the SQ_LINK_* constants
	* @param string		$value			the value that is associated with this link
	* @param string		$side_of_link	Which side of the link the first assetid is on
	*									('major' or 'minor')
	* @param boolean	$force_array	force the results to return an array of links
	*									even if there is only one link
	* @param boolean	$dependant		The dependant status for all the links must be
	*									this (if not NULL)
	* @param boolean	$exclusive		The exclusive status for all the links must be
	*									this (if not NULL)
	*
	* @return array
	* @access public
	*/
	function getLinkByAsset($assetid, $other_assetid, $link_types=NULL, $value=NULL, $side_of_link='major', $force_array=FALSE, $dependant=NULL, $exclusive=NULL)
	{
		$links = Array();

		$varieties_attr = $this->attr('varieties');

		if ($side_of_link === 'major') {
			$shadow_parts 	  = explode(':', $other_assetid, 2);
			$expected_bridge  = $assetid;
		} else {
			$shadow_parts 	  = explode(':', $assetid, 2);
			$expected_bridge  = $other_assetid;
		}

		if (((string) $expected_bridge === (string) $this->id) && ((string) $shadow_parts[0] === (string) $this->id)) {
			// Correct asset.
			if ($shadow_parts[1]{0} === 'v') {
				// Varieties start with "v"
				if (array_key_exists($shadow_parts[1], $varieties_attr['data']) === TRUE) {
					// This shadow asset exists.
					$varietyNum = (int) substr($shadow_parts[1], 1);
					$link       = Array(
								   'linkid'          => $assetid.':'.$other_assetid,
								   'value'           => '',
								   'link_type'       => SQ_LINK_TYPE_2,
								   'sort_order'      => $varietyNum,
								   'is_dependant'    => 1,
								   'is_exclusive'    => 1,
								   'locked'          => 1,
								  );

					if ($side_of_link === 'major') {
						$link['minorid']         = $other_assetid;
						$link['minor_type_code'] = 'image_variety';
					} else {
						$link['majorid']         = $other_assetid;
						$link['major_type_code'] = $this->type();
					}

					$links[] = $link;
				}//end if
			}//end if
		}//end if

		if (($force_array === FALSE) && (count($links) === 1)) {
			$links = $links[0];
		}

		return $links;

	}//end getLinkByAsset()


	/**
	* Get details of the links from the specified asset to all its children
	*
	* @param string	$assetid	the assetid of the asset to find the child links for
	* @param int	$link_type	the type of links to get
	*
	* @return array
	* @access public
	*/
	function getAllChildLinks($assetid, $link_type=0)
	{
		return Array();

	}//end getAllChildLinks()


	/**
	* Update the details of an existing link
	*
	* If any of the detail vars are NULL they are not updated.
	*
	* @param int	$linkid		the link id of the link to update
	* @param int	$link_type	the type of the link (one of the SQ_LINK_* constants)
	* @param string	$value		the value to place on the link
	* @param int	$sort_order	the position in the links list that this link should take,
	*							if less than zero places at end of list
	*
	* @return boolean
	* @access public
	*/
	function updateLink($linkid, $link_type=NULL, $value=NULL, $sort_order=NULL)
	{
		return FALSE;

	}//end updateLink()


	/**
	* Remove a link by id
	*
	* @param int		$linkid	the link id of the link to remove
	* @param boolean	$moving	TRUE if this delete is part of a move operation
	*
	* @return boolean
	* @access public
	*/
	function deleteAssetLink($linkid, $moving=FALSE)
	{
		return FALSE;

	}//end deleteAssetLink()


	/**
	* Create a link between the two supplied assets
	*
	* @param object		&$major		the major asset that we are linking from
	* @param object		&$minor		the minor asset that we are linking to
	* @param string		$link_type	the type of link this is
	* @param string		$value		the value that is to be associated with this link
	* @param string		$sort_order	the position in the links list that this link should take,
	*								if NULL or less than zero places at end of list
	* @param string		$dependant	'0' / '1' - whether the minor asset is dependant on the major
	* @param string		$exclusive	'0' / '1' - whether the major asset is to be the minor's only parent
	* @param boolean	$moving		whether or not this asset is being moved
	*
	* @return int
	* @access public
	*/
	function createAssetLink(&$major, &$minor, $link_type, $value='', $sort_order=NULL, $dependant='0', $exclusive='0', $moving=FALSE)
	{
		return 0;

	}//end createAssetLink()


	/**
	* Get asset info for use by asset map
	*
	* @param string	$assetid	Full Asset id to get information for
	*
	* @return array
	* @access public
	*/
	function getAssetMapAssetInfo($assetid)
	{
		return Array();

	}//end getAssetMapAssetInfo()


	/**
	* Checks a file on the file system to see if it can be used as a valid image file
	*
	* @param array	$info	an array of information about the uploaded file
	*
	* @return boolean
	* @access public
	*/
	public function validFile(Array $info)
	{
		if (!parent::validFile($info)) {
			return FALSE;
		}

		if (getimagesize($info['path']) === FALSE) {
			trigger_error('Uploaded file is not a valid image file', E_USER_WARNING);
			return FALSE;
		}

		return TRUE;

	}//end validFile()


}//end class

?>
