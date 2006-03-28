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
* $Id: image.inc,v 1.38.2.6 2006/03/28 03:16:01 lwright Exp $
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
* @version $Revision: 1.38.2.6 $
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
		$this->_ser_attrs = true;
		$this->File($assetid);

	}//end constructor


	/**
	* Set the value of an attribute
	*
	* @param boolean	$dont_run_updated	if true this->_updated() won't be run
	*										(THIS IS ONLY EVER CALLED FROM Asset::create())
	*
	* @return boolean
	* @access public
	*/
	function saveAttributes($dont_run_updated=false)
	{
		if (!$this->id) return true;

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
	* @param string		$name				the name of the attribute
	* @param string		$value				the new value of the attribute
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
			$fv = $GLOBALS['SQ_SYSTEM']->getFileVersioning();

			if (isset($varieties['data']) && is_array($varieties['data'])) {
				foreach($varieties['data'] as $variety_shadowid => $variety) {
					$old_variety_path = $old_base_path.'/'.$variety['filename'];
					$new_variety_path = $new_base_path.'/'.$variety['filename'];

					if (!copy($old_variety_path, $new_variety_path)) {
						trigger_localised_error('CORE0036', E_USER_WARNING, $old_variety_path, $new_variety_path);
						return false;
					}

					if (!$fv->add($clone->data_path_suffix.'/varieties', $new_variety_path)) {
						return false;
					}

					if (!$fv->checkOut($clone->data_path_suffix.'/varieties/'.$variety['filename'], $clone->data_path.'/varieties')) {
						trigger_localised_error('CORE0103', E_USER_WARNING);
						return false;
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
		if (!parent::validFile($info)) return false;

		// check that the extension is allowed
		require_once SQ_FUDGE_PATH.'/general/file_system.inc';
		$ext = get_file_type($info['filename']);
		if (!in_array($ext, $this->allowed_extensions)) {
			$edit_fns = $this->getEditFns();
			trigger_localised_error('CORE0107', E_USER_WARNING, $edit_fns->_getExtensionString($this->allowed_extensions, 'or'), strtoupper($ext));
			return false;
		}
		return true;

	}//end validFile()


	/**
	* Get an array of some useful data about the existing uploaded file
	*
	* @return array(string)
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
	* @param string 	$alt	the alt tag content. If empty the alt var is used
	* @param string 	$title	the alt tag content. If empty the and $alt is set, then that is used, otherwise the title var is used
	* @param boolean 	$return	if TRUE the tag will be returned
	*
	* @return string | void
	* @access public
	*/
	function printImageTag($alt='', $title='', $return=false)
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
	* Return the keywords with their replacements
	*
	* The return value is in the form:
	* <pre>
	* Array(
	*     'name' => Array (
	*          'value' => 'page',
	*          'description' => 'the asset name',
	*     ),
	* )
	* </pre>
	*
	* @param boolean $descriptions if TRUE descriptions will be included in the array
	*
	* @access public
	* @return Array(mixed)
	*/
	function getAssetKeywords($descriptions=false)
	{
		$keywords = parent::getAssetKeywords($descriptions);

		$existing = $this->getExistingFile();
		$keywords['image_width']	= Array('value' => $this->attr('width'));
		$keywords['image_height']	= Array('value' => $this->attr('height'));
		$keywords['image_tag']		= Array('value' => $this->printImageTag('', '', true));

		$varieties = $this->attr('varieties');
		if (!empty($varieties)) {
			foreach ($varieties['lookups'] as $varietyid) {
				$variety = &$this->getAsset($varietyid);
				if (is_null($variety)) continue;
				$keywords['image_v_'.$variety->name] = Array('value' => $variety->printImageTag($this->attr('alt'), $this->attr('title'), true));
			}
		}

		if ($descriptions) {
			$keywords['image_width']['description']		= 'Width of the image in pixels';
			$keywords['image_height']['description']	= 'Height of the image in pixels';
			$keywords['image_tag']['description']		= 'HTML IMG tag to display this image';
			$keywords['image_v_'] = Array('description'	=> 'Image variation');
		}

		return $keywords;

	}//end getAssetKeywords()


	/**
	* Prints out the backend interface for the asset
	*
	* @param object Backend_Outputter	&$o
	*
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
	*
	* @param object Image_Variety	$variety			the variety to save
	* @param boolean				$recreate_image		whether to generate the image file or not
	* @param boolean				$save_attrs			whether to run saveAttributes() or not,
	*													usually you only would set this to false if you are running
	*													this multiple times and only want to save once at the end
	*
	* @return false|int	false on error, shadow assetid on success
	* @access public
	*/
	function saveVariety($variety, $recreate_image=true, $save_attrs=true)
	{
		if (!$this->writeAccess('attributes')) {
			trigger_localised_error('CORE0069', E_USER_WARNING, $this->name, $this->id);
			return false;

		}

		// use attr() instead of just ->name because if the variety hasn't been created yet it won't have a name
		$name = $variety->attr('name');
		if (trim($name) == '') {
			trigger_localised_error('CORE0100', E_USER_WARNING);
			return false;

		}

		$varieties = $this->attr('varieties');

		// check if this is already one of our varieties
		if ($variety->id) {
			list($assetid, $varietyid) = explode(':', $variety->id, 2);
			if ($assetid != $this->id) {
				trigger_localised_error('CORE0099', E_USER_WARNING);
				return false;

			}

			if (isset($varieties['data'][$varietyid])) {
				if ($varieties['data'][$varietyid]['name'] != $name) {
					trigger_localised_error('CORE0097', E_USER_WARNING, $varietyid);
					return false;

				}
			} else {
				trigger_localised_error('CORE0098', E_USER_WARNING, $varietyid, $this->name, $this->id);
				return false;

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
					return false;
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
			return false;
		}

		$variety = &$this->getAsset($varietyid);
		if (is_null($variety)) return false;


		$GLOBALS['SQ_SYSTEM']->doTransaction('BEGIN');

		// create the actual scaled image if necessary and add any lookups
		if ($recreate_image) {
			if (!$variety->updateImage()) {
				$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
				return false;

			}

			// Now get the variables again because updateImage() can update stuff
			foreach ($variety->vars as $attr_name => $attr_data) {
				$varieties['data'][$varietyid][$attr_name] = $attr_data['value'];
			}
			if (!$this->setAttrValue('varieties', $varieties)) {
				return false;
			}
		}

		if ($save_attrs && !$this->saveAttributes()) {
			$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
			return false;
		}

		$GLOBALS['SQ_SYSTEM']->doTransaction('COMMIT');
		return $varietyid;

	}//end saveVariety()


	/**
	* Deletes a variety from this image
	*
	* @param string		$varietyid	the shadow assetid or just the shadow part of the variety to delete
	*
	* @return boolean
	* @access public
	*/
	function deleteVariety($varietyid)
	{
		if (!$this->writeAccess('attributes')) {
			trigger_localised_error('CORE0070', E_USER_WARNING, $this->name, $this->id);
			return false;
		}

		if (strpos($varietyid, ':') !== false) {
			list($assetid, $varietyid) = explode(':', $varietyid, 2);
			if ($assetid != $this->id) {
				trigger_localised_error('CORE088', E_USER_WARNING);
				return false;
			}
		}

		$variety = &$this->getAsset($varietyid);
		if (is_null($variety)) return false;

		$GLOBALS['SQ_SYSTEM']->doTransaction('BEGIN');

		// delete the actual scaled image files
		if (!$variety->deleteImage()) {
			$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
			return false;
		}

		$varieties = $this->attr('varieties');

		if (!isset($varieties['data'][$varietyid])) {
			trigger_localised_error('CORE87', $varietyid, E_USER_WARNING);
			$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
			return false;
		}

		unset($varieties['data'][$varietyid]);
		$lookup_name = array_search($varietyid, $varieties['lookups']);
		if ($lookup_name !== false) {
			unset($varieties['lookups'][$lookup_name]);
		}

		if (!$this->setAttrValue('varieties', $varieties) || !$this->saveAttributes()) {
			$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
			return false;
		}

		if (!$this->updateLookups()) {
			$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
			return false;
		}

		$GLOBALS['SQ_SYSTEM']->doTransaction('COMMIT');
		return true;

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
				$variety = &$this->getAsset($varietyid);
				if (is_null($variety)) continue;

				if (!$variety->checkFileState($this->effectiveUnrestricted())) {
					return false;
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
		$db = &$GLOBALS['SQ_SYSTEM']->db;
		$GLOBALS['SQ_SYSTEM']->doTransaction('BEGIN');

		if (!parent::updateLookups()) {
			$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
			return false;
		}

		$use_public_path = $this->usePublicPath();

		$varieties = $this->attr('varieties');
		if (!empty($varieties)) {
			foreach ($varieties['lookups'] as $varietyid) {
				$variety = &$this->getAsset($varietyid);
				if (is_null($variety)) continue;

				if (!$variety->updateLookups($use_public_path))  {
					$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
					return false;
				}

			}//end foreach
		}// end if

		$GLOBALS['SQ_SYSTEM']->doTransaction('COMMIT');
		return true;

	}//end updateLookups()


//--        BRIDGE INTERFACE        --//


	/**
	* Returns a reference to the asset represented by the passed assetid
	*
	* @param int		$shadowid		the shadow part asset id to be loaded (i.e part after the ':')
	* @param string		$type_code		if this exists then this object is used to
	*									load the asset, if not then the DB is queried
	*									to find out the asset type
	* @param boolean	$mute_errors	stops the outputting of errors in this fn, needed because
	*									you can't use the '@' operator when returning by reference
	*
	* @return object Asset or null on error
	* @access public
	* @see Asset_Manager::&getAsset()
	*/
	function &getAsset($shadowid, $type_code='', $mute_errors=false)
	{
		$asset = null;
		if (empty($shadowid)) return $asset;

		// Varieties are preceded with a 'v'
		if ($shadowid{0} == 'v') {

			$varieties = $this->attr('varieties');

			if (!isset($varieties['data'][$shadowid])) {
				trigger_localised_error('CORE0073', E_USER_WARNING, $this->id, $shadowid);
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
	function getLinks($assetid, $link_types, $type_code='', $strict_type_code=true, $side_of_link='major', $sort_by=null)
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
				$descendants = $GLOBALS['SQ_SYSTEM']->am->getTypeDescendants($type_code, true);
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
										'majorid'			=> $this->id,
										'minor_type_code'	=> 'image_variety',
										'value'				=> '',
										'link_type'			=> SQ_LINK_TYPE_2,
										'is_dependant'		=> true,
										'is_exclusive'		=> true,
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
						if (!isset($varieties['data'][$assetid])) {
							trigger_localised_error('CORE0074', E_USER_WARNING, $assetid, $GLOBALS['SQ_SYSTEM']->am->getTypeInfo($this->type(), 'name'), $this->id);
							return Array();

						} else {
							$links[] = Array(
										'linkid'			=> 0,
										'majorid'			=> $this->id,
										'major_type_code'	=> $this->type(),
										'value'				=> '',
										'link_type'			=> SQ_LINK_TYPE_2,
										'is_dependant'		=> true,
										'is_exclusive'		=> true,
										'sort_order'		=> array_search($assetid, array_keys($varieties['data'])),
									   );
						}//endif

					// because the varieties are only ever going to be one level down
					// if we are looking down the tree there will be nothing
					} else {
						$links = Array();

					}// endif

				}// end if

				if (!is_null($sort_by)) {
					uasort($links, create_function('$a,$b', 'return $a["'.$sort_by.'"] < $b["'.$sort_by.'"];'));
				}

			}// end if

			$this->_tmp['getLinks'][$assetid][$side_of_link] = $links;

		}//end if

		return $this->_tmp['getLinks'][$assetid][$side_of_link];

	}//end getLinks()


	/**
	* Get all asset ids that are below the passed assetid in the various trees in which it exists
	*
	* @param int			$assetid			the id of the asset to get its children for
	* @param string|array	$type_code			the type of asset that is linked
	*											(eg 'Page', 'File', etc)
	*											if an array returns link if matches any
	*											of the array values
	* @param bool			$strict_type_code	whether we are finding assets that are just a $type_code
	*											or $type_code and any of it's sub-classes
	* @param string			$sort_by			a field in the asset table you would like the results
	*											sorted by (eg. name, short_name etc)
	*
	* @return Array(int => string)	returns an array of assetids and their type code
	* @access public
	* @see Asset_Manager::getChildren()
	*/
	function getChildren($assetid, $type_code='', $strict_type_code=true, $sort_by=null)
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
				$descendants = $GLOBALS['SQ_SYSTEM']->am->getTypeDescendants($type_code, true);
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
	* @param bool			$strict_type_code	whether we are finding assets that are just a $type_code
	*											or $type_code and any of it's sub-classes
	*
	* @return Array(int)
	* @access public
	* @see Asset_Manager::getParents()
	*/
	function getParents($assetid, $type_code='', $strict_type_code=true)
	{
		if ($assetid == $this->id || strpos($assetid, 'v') === false) {
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
				$descendants = $GLOBALS['SQ_SYSTEM']->am->getTypeDescendants($type_code, true);
				if (!in_array($ret_type_code, $descendants)) {
					return $parents;
				}
			}

		}//end if type_code

		$parents[] = $this->id;

		return $parents;

	}//end getParents()


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


}//end class

?>
