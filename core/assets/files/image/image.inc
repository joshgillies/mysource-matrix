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
* $Id: image.inc,v 1.59 2006/06/27 07:40:45 lwright Exp $
*
*/


require_once SQ_CORE_PACKAGE_PATH.'/files/file/file.inc';

/**
* Image
*
* Purpose
*
*
* @author  Greg Sherwood <greg@squiz.net>
* @version $Revision: 1.59 $
* @package MySource_Matrix_Packages
* @subpackage __core__
*/
register_implementation('image', 'bridge');
class Image extends File
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
		$this->File($assetid);

	}//end constructor


	/**
	* Set the value of an attribute
	*
	* @param boolean	$dont_run_updated	if TRUE this->_updated() won't be run
	*										(THIS IS ONLY EVER CALLED FROM Asset::create())
	*
	* @return boolean
	* @access public
	*/
	function saveAttributes($dont_run_updated=FALSE)
	{
		if (!$this->id) return TRUE;

		// if there are no values for the alt attribute use the filename
		if (isset($this->_tmp['vars_set']['alt']) && trim($this->attr('alt')) == '') {
			$this->setAttrValue('alt', $this->attr('name'));
		}//end if

		return parent::saveAttributes($dont_run_updated);

	}//end saveAttributes()


	/**
	* Set the value for the passed variable to the passed value
	* If the user changes the name of this file, we also need
	* to update the file system
	*
	* @param string	$name	the name of the attribute
	* @param string	$value	the new value of the attribute
	*
	* @return boolean
	* @access public
	*/
	function setAttrValue($name, $value)
	{
		switch ($name) {
			case 'alt':
				$value = htmlspecialchars($value);
			break;
		}

		return parent::setAttrValue($name, $value);

	}//end setAttrValue()


	/**
	* Processes the file upload from the source file so that file versioning cloned properly
	*
	* @param object	&$clone		the clone asset shell
	* @param array	$components	the wanted components to clone eg.
	*							Array(
	*							'attributes',
	*							'metadata_schemas',
	*							'metadata',
	*							'workflow',
	*							'permissions',
	*							'data'
	*							);
	*							or alternately Array('all');
	*
	* @return boolean
	* @access public
	* @see asset::cloneComponents()
	*/
	function cloneComponents(&$clone, $components)
	{
		// need to go through the varieties that have already been added, and
		// re-upload/create them underneath this asset - to get the file
		// versioning correct

		if (in_array('attributes', $components) || in_array('all', $components)) {

			$varieties = $this->attr('varieties');
			$old_base_path = $this->data_path.'/varieties';
			$new_base_path = $clone->data_path.'/varieties';

			create_directory($new_base_path);

			// file versioning magick :D
			$fv =& $GLOBALS['SQ_SYSTEM']->getFileVersioning();

			if (isset($varieties['data']) && is_array($varieties['data'])) {
				foreach ($varieties['data'] as $variety_shadowid => $variety) {
					$old_variety_path = $old_base_path.'/'.$variety['filename'];
					$new_variety_path = $new_base_path.'/'.$variety['filename'];

					if (!copy($old_variety_path, $new_variety_path)) {
						trigger_localised_error('CORE0036', E_USER_WARNING, $old_variety_path, $new_variety_path);
						return FALSE;
					}

					if (!$fv->add($clone->data_path_suffix.'/varieties', $new_variety_path)) {
						return FALSE;
					}

					if (!$fv->checkOut($clone->data_path_suffix.'/varieties/'.$variety['filename'], $clone->data_path.'/varieties')) {
						trigger_localised_error('CORE0103', E_USER_WARNING);
						return FALSE;
					}
				}
			}
		}//end if attributes|all

		return parent::cloneComponents($clone, $components);

	}//end cloneComponents()


	/**
	* Checks a file on the file system to see if it can be used as an image
	*
	* @param array	$info	an array of information about the uploaded file
	*
	* @return boolean
	* @access public
	*/
	function validFile($info)
	{
		if (!parent::validFile($info)) return FALSE;

		// check that the extension is allowed
		require_once SQ_FUDGE_PATH.'/general/file_system.inc';
		$ext = get_file_type($info['filename']);
		if (!in_array($ext, $this->allowed_extensions)) {
			$edit_fns = $this->getEditFns();
			trigger_localised_error('CORE0107', E_USER_WARNING, $edit_fns->_getExtensionString($this->allowed_extensions, 'or'), strtoupper($ext));
			return FALSE;
		}
		return TRUE;

	}//end validFile()


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

		$tag = '<img src="'.$this->getURL().'" width="'.$this->attr('width').'" height="'.$this->attr('height').'" alt="'.$alt.'" title="'.$title.'" />';
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
	function paintBackend(&$o)
	{
		if (!isset($_REQUEST['asset_ei_screen'])) {
			return parent::paintBackend($o);
		}

		switch ($_REQUEST['asset_ei_screen']) {
			case 'image_info' :
				include_once SQ_FUDGE_PATH.'/var_serialise/var_serialise.inc';
				$ouput = Array();
				$output['alt']    = $this->attr('alt');
				$output['width']  = $this->attr('width');
				$output['height'] = $this->attr('height');
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

		$variety =& $this->getAsset($this->id.':'.$varietyid);
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
				trigger_localised_error('CORE088', E_USER_WARNING);
				return FALSE;
			}
		}

		$variety =& $this->getAsset($this->id.':'.$varietyid);
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
	function _checkFileState()
	{
		$varieties = $this->attr('varieties');
		if (!empty($varieties)) {
			foreach ($varieties['lookups'] as $varietyid) {
				$variety =& $this->getAsset($this->id.':'.$varietyid);
				if (is_null($variety)) continue;

				if (!$variety->checkFileState($this->effectiveUnrestricted())) {
					return FALSE;
				}
			}
		}

		return parent::_checkFileState();

	}//end _checkFileState()


	/**
	* Called to force an update of this assets lookup information
	*
	* @return boolean
	* @access public
	* @see Asset::updateLookups(), Asset::saveWebPaths()
	*/
	function updateLookups()
	{
		$GLOBALS['SQ_SYSTEM']->changeDatabaseConnection('db2');
		$GLOBALS['SQ_SYSTEM']->doTransaction('BEGIN');

		if (!parent::updateLookups()) {
			$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
			$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();
			return FALSE;
		}

		$varieties = $this->attr('varieties');
		if (!empty($varieties)) {
			foreach ($varieties['lookups'] as $varietyid) {
				$variety =& $this->getAsset($this->id.':'.$varietyid);
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
	function &getAsset($assetid, $type_code='', $mute_errors=FALSE)
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
				trigger_localised_error('CORE0073', E_USER_WARNING, $this->id, $shadowid);
				return $asset;
			}

			$GLOBALS['SQ_SYSTEM']->am->includeAsset('image_variety');
			$asset =& new Image_Variety($this->id.':'.$shadowid, $varieties['data'][$shadowid]);

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
									   );
						}//endif

					// because the varieties are only ever going to be one level down
					// if we are looking down the tree there will be nothing
					} else {
						$links = Array();

					}//endif

				}//end else - looking from shadow asset

				if (!is_null($sort_by)) {
					uasort($links, create_function('$a,$b', 'return $a["'.$sort_by.'"] < $b["'.$sort_by.'"];'));
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
				$children[$this->id.':'.$varid] = 'image_variety';
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
				$variety =& $this->getAsset($this->id.':'.$varietyid);
				if (is_null($variety)) continue;
				$keywords['image_v_'.$variety->name] = 'Image variety '.$variety->name;
				$keywords['image_v_'.$variety->name.'_url'] = 'Image variety '.$variety->name.' URL';
			}
		}
		$keywords['image_width']	= 'Width of the image in pixels';
		$keywords['image_height']	= 'Height of the image in pixels';
		$keywords['image_tag']		= 'HTML IMG tag to display this image';

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
		if (0 === strpos($keyword, 'image_v_')) {
			$varieties = $this->attr('varieties');
			if (empty($varieties)) return '';
			$variety_name = substr($keyword, 8);
			$url_only = FALSE;
			if (substr($variety_name, -4) == '_url') {
				$variety_name = substr($variety_name, 0, strlen($variety_name) - 4);
				$url_only = TRUE;
			}
			$variety_id = array_get_index($varieties['lookups'], $variety_name);
			if (!empty($variety_id)) {
				$variety =& $GLOBALS['SQ_SYSTEM']->am->getAsset($this->id.':'.$variety_id);
				if ($url_only) {
					$string = $variety->getURL();
				} else {
					$string = $variety->printImageTag($this->attr('alt'), $this->attr('title'), TRUE);
				}
				return $string;
			}
		}
		return parent::getKeywordReplacement($keyword);

	}//end getKeywordReplacement()


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
			$asset =& $this->getAsset($assetid);

			if (!empty($field)) {
				if ($field == 'assetid') {
					$info[$assetid] = $assetid;
				} else if ($field == 'type_code') {
					$info[$assetid] = $asset->type();
				} else if (in_array($field , Array('version', 'name', 'short_name', 'status'))) {
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

		}//end foreach

		return $info;

	}//end getAssetInfo()


	/**
	* Determine if the passed assetid or assetids refer(s) to a valid asset in the system
	*
	* Note that this will always return FALSE if you pass it a shadow asset
	*
	* @param mixed	$assetids	the ID of the asset we are going to the check for existence
	*							or an array of assetids we want to check
	*
	* @return mixed boolean|array
	* @access public
	*/
	function assetExists($assetids)
	{
		return FALSE;

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
		return Array();

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


}//end class

?>
