<?php

require_once SQ_CORE_PACKAGE_PATH.'/designs/design/design.inc';

/**
* Design_Customisation
*
* Purpose
*
*
* @author  Blair Robertson <blair@squiz.net>
* @version $Version$ - 1.0
* @package MySource_Matrix_Packages
* @subpackage __core__
*/
class Design_Customisation extends Design
{

	/**
	* Constructor
	*
	* @param int	$assetid	the asset id to be loaded
	*
	*/
	function Design_Customisation($assetid=0)
	{
		$this->Design($assetid);

	}//end constructor


	/*
	* Returns an array of all the permitted links type, the type asset and the cardinality
	*
	* @return array
	* @access private
	* @see Asset::_getAllowLinks()
	*/
	function _getAllowedLinks()
	{
		$links = parent::_getAllowedLinks();
		$links[SQ_LINK_TYPE_3]['file'] = Array('card' => 'M', 'exclusive' => false);
		return $links;

	}//end _getAllowedLinks()


	/**
	* Removes a link where this asset is the major player
	*
	* @param int	$linkid	the link id of the link to remove
	*
	* @return boolean
	* @access public
	*/
	function deleteLink($linkid)
	{
		// first, we should try and find the link
		$delete_link = $GLOBALS['SQ_SYSTEM']->am->getLinkById($linkid, $this->id);
		if (empty($delete_link)) {
			trigger_error('Existing Link #'.$linkid.' not found, unable to delete link', E_USER_WARNING);
			return false;
		}

		$am = &$GLOBALS['SQ_SYSTEM']->am;
		$minor = &$am->getAsset($delete_link['minorid'], $delete_link['minor_type_code']);

		$GLOBALS['SQ_SYSTEM']->doTransaction('BEGIN');

		if (!parent::deleteLink($linkid)) {
			$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
			return false;
		}

		// special processing if we are deleting a TYPE_2 linked file (a file overridding a design file)
		if ($delete_link['link_type'] == SQ_LINK_TYPE_2 && is_a($minor, 'file')) {

			$design =& $this->getParentDesign();

			// go through all the links of the design we are customisaing and find a file
			// linked to it with the same name as the one we just deleted so it can take its place
			$file_link_ids = Array();
			$file_links = $GLOBALS['SQ_SYSTEM']->am->getLinks($design->id, SQ_LINK_TYPE_2, 'file', false);
			foreach ($file_links as $link) $file_link_ids[$link['minorid']] = $link['linkid'];
			$file_link_info = $GLOBALS['SQ_SYSTEM']->am->getAssetInfo(array_keys($file_link_ids));

			foreach ($file_link_info as $info) {
				if ($info['name'] == $minor->name) {
					// found the link to create
					$new_minor = &$am->getAsset($info['assetid']);
					if (!$this->createLink($new_minor, SQ_LINK_TYPE_3, '', null, '1')) {
						trigger_error('Failed creating file link to customisation', E_USER_WARNING);
						$am->forgetAsset($design);
						$am->forgetAsset($new_minor);
						$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
						return false;
					}
					$am->forgetAsset($new_minor);
				}
			}// end foreach $file_links

			$am->forgetAsset($design);
		}

		$GLOBALS['SQ_SYSTEM']->doTransaction('COMMIT');
		return true;

	}//end deleteLink()


	/**
	* Called whenever any type of link is changed, allows caching of objects, links and other stuff
	* to occur but not cause integrity issues during script execution
	* returns true it it's successfull
	*
	* @return boolean
	* @access private
	*/
	function _linksUpdated()
	{
		if (!parent::_linksUpdated()) return false;
		unset($this->_tmp['parent_design']);
		unset($this->_tmp['customised_areas']);
		unset($this->_tmp['uncustomised_areas']);
		return true;

	}//end _linksUpdated()


	/**
	* Returns the parent design/design_customisation that this customisation is customising
	*
	* @return mixed object Design or null
	* @access public
	*/
	function &getParentDesign()
	{
		if (empty($this->_tmp['parent_design'])) {
			$parent_link = $GLOBALS['SQ_SYSTEM']->am->getLink($this->id, SQ_LINK_TYPE_2, 'design', false, 'customisation', 'minor');
			$this->_tmp['parent_design'] = &$GLOBALS['SQ_SYSTEM']->am->getAsset($parent_link['majorid'], $parent_link['major_type_code']);
		}
		return $this->_tmp['parent_design'];

	}//end getParentDesign()


	/**
	* Returns an array of links for design areas that have been customised by this design
	*
	* @return Array(string)
	* @access public
	*/
	function getCustomisedAreas()
	{
		if (!isset($this->_tmp['customised_areas'])) {
			$parent = &$this->getParentDesign();
			if (!is_null($parent)) {
				// OK, get our design area links and our parents
				$our_links     = $this->getDesignAreaLink();
				$parents_links = $parent->getDesignAreaLink();
				$this->_tmp['customised_areas'] = Array();

				// Basically we do a search through our links and see if the design areas (ie the id_names)
				// are pointing to the same assetid or not to ascertain whether we have customised this area or not
				foreach($our_links as $our_link) {
					foreach($parents_links as $parents_link) {
						// if the id_names are the same
						if ($our_link['value'] == $parents_link['value']) {
							// if the id's are different, we have customised this one
							if ($parents_link['minorid'] == $our_link['minorid']) {
								$this->_tmp['uncustomised_areas'][] = $our_link;
							} else {
								$this->_tmp['customised_areas'][]   = $our_link;
							}
							continue;
						}// end if
					}// end foreach
				}// end foreach
			}// end if
		}// end if

		if (!isset($this->_tmp['customised_areas'])) $this->_tmp['customised_areas'] = Array();
		if (!isset($this->_tmp['uncustomised_areas'])) $this->_tmp['uncustomised_areas'] = Array();

		return $this->_tmp['customised_areas'];

	}// end getCustomisedAreas()


	/**
	* Returns an array of links for design areas that have NOT been customised by this design
	*
	* @return Array(string)
	* @access public
	*/
	function getUnCustomisedAreas()
	{
		if (!isset($this->_tmp['uncustomised_areas'])) {
			$this->getCustomisedAreas();
		}// end if

		return $this->_tmp['uncustomised_areas'];

	}// end getUnCustomisedAreas()


	/**
	* Updates this customisation from our parent,
	* ie. Updates our contents array and the contents arrays for any customised but NOT specialised design areas
	*     No need to do the uncustomised design areas because they would be done by our parent
	*     No need to do the specialised design areas because, bugger ya, you specialised them so you're on your own
	*
	* @param object Design	$parent	the parent design for this customisation
	*
	* @return bool
	* @access protected
	*/
	function updateFromParent(&$parent)
	{
		$am = &$GLOBALS['SQ_SYSTEM']->am;

		//// FIRST, let's set our new contents ////
		if (!$this->setAttrValue('contents', $parent->attr('contents'))) return false;

		//// SECOND, let's remove any design area links that we aren't going to be using any more ////

		$tmp = $parent->getDesignAreaLink();
		$parent_da_links = Array();
		foreach ($tmp as $link) $parent_da_links[$link['value']] = $link;

		$tmp = $this->getDesignAreaLink();
		$this_da_links = Array();
		foreach ($tmp as $link) $this_da_links[$link['value']] = $link;

		$remove_id_names = array_diff(array_keys($this_da_links),   array_keys($parent_da_links));
		$added_id_names  = array_diff(array_keys($parent_da_links), array_keys($this_da_links));

		// now remove any unused design area customisations
		foreach($remove_id_names as $id_name) {
			if (!$this->deleteLink($this_da_links[$id_name]['linkid'])) return false;
		}// end foreach

		foreach($added_id_names as $id_name) {
			$da = &$am->getAsset($parent_da_links[$id_name]['minorid'], $parent_da_links[$id_name]['minor_type_code'], true);
			if (is_null($da)) continue;
			$this->createLink($da, SQ_LINK_TYPE_3, $id_name, null, '1');
		}// end foreach

		//// THIRD, let's update the customised, but not specialised, design areas ////

		$links = $this->getCustomisedAreas();
		foreach($links as $link) {
			$da = &$am->getAsset($link['minorid'], $link['minor_type_code']);
			if (is_null($da)) continue;
			// you specialised ? well, buddy, you're on your own
			if ($da->attr('specialised')) continue;

			$parent_da = &$parent->getDesignArea($link['value']);
			if (is_null($parent_da)) continue;

			$da->setAttrValue('contents', $parent_da->attr('contents'));

		}// end foreach

		//// AND FINALLY, regenerate our design file and update any of our customisations ////
		$this->generateDesignFile(false); // not recursive because update customisations takes care of that
		$this->_updateCustomisations();

		return true;

	}//end updateFromParent()


	/**
	* Paints out our design
	*
	* @param object Asset	$ASSET		the asset that we are going to print the body for
	* @param array			$FILE_URLS	an array of filename => URL to use to print files in the design
	*									doesnt need to be passed in unless some URLs are overridden by
	*									a customisation of the design
	*
	* @return void
	* @access public
	*/
	function paint(&$ASSET, $FILE_URLS=Array())
	{
		// generate an array of all the URLs we are going to need for the files in this customisation
		// that are inherited from the design we are customising
		$file_link_ids = Array();
		$parent_link = $GLOBALS['SQ_SYSTEM']->am->getLinks($this->id, SQ_LINK_TYPE_2, 'design', true, 'minor');
		if (!empty($parent_link)) {
			$parent_design = &$GLOBALS['SQ_SYSTEM']->am->getAsset($parent_link[0]['majorid'], $parent_link[0]['major_type_code']);
			if (!is_null($parent_design)) {
				$file_links = $GLOBALS['SQ_SYSTEM']->am->getLinks($parent_design->id, SQ_LINK_TYPE_2, 'file', false);
				foreach ($file_links as $link) $file_link_ids[] = $link['minorid'];
			}
		}

		$file_link_info = $GLOBALS['SQ_SYSTEM']->am->getAssetInfo(array_unique($file_link_ids));
		$file_link_urls = $GLOBALS['SQ_SYSTEM']->am->getAssetURL(array_unique($file_link_ids));

		foreach ($file_link_info as $minorid => $file_info) {
			$file_url = $file_link_urls[$minorid];
			if ($file_url == '') $file_url = "Javascript: alert('This URL for \'".$file_info['name']."\' does not exist');";
			$FILE_URLS[$file_info['name']] = $file_url;
		}

		return parent::paint($ASSET, $FILE_URLS);

	}//end paint()


	/**
	* Returns the file system path to the directory holding uploaded files
	* Because we are a customisation, the path that we return is the one that the
	* original "Design" object has
	*
	* @param string	$filename	the filename to return the path to
	*
	* @return string
	* @access public
	* @see Design::filePath()
	*/
	function filePath($filename)
	{
		$full_path = $this->data_path.'/files/'.$filename;
		// if we have this file then use our path
		if (file_exists($full_path)) {
			return $full_path;

		// otherwise, let's hope the parent has this file
		} else {
			$parent = &$this->getParentDesign();
			if (is_null($parent)) return '';
			return $parent->filePath($filename);

		}

	}//end filePath()


	/**
	* Returns the path that has to be appended to the sq_web_path('data') to
	* get access to the file for this design
	*
	* @param string	$filename	the filename to return the path to
	*
	* @return string
	* @access public
	*/
	function webFileSuffix($filename)
	{
		// if we have this file then use our path
		if (file_exists($this->data_path_public.'/files/'.$filename)) {
			return parent::webFileSuffix($filename);

		// otherwise, let's hope the parent has this file
		} else {
			$parent = &$this->getParentDesign();
			if (is_null($parent)) return '';
			return $parent->webFileSuffix($filename);

		}
	}//end webFilesSuffix()


}//end class
?>
