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
* @package Resolve_Packages
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
	}

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

	/*
	* Returns the parent design/design_customisation that this customisation is customising
	*
	* @return object Design | null
	* @access public
	*/
	function &getParentDesign() 
	{
		if (empty($this->_tmp['parent_design'])) {
			$parent_link = $this->getLink(SQ_LINK_TYPE_2, 'design', false, 'customisation', 'minor');
			$this->_tmp['parent_design'] = &$GLOBALS['SQ_SYSTEM']->am->getAsset($parent_link['majorid'], $parent_link['type_code']);
		}
		return $this->_tmp['parent_design'];
	}//end getParentDesign()


	/*
	* Returns an array of links for design areas that have been customised by this design
	*
	* @return Array(string)
	* @access public
	*/
	function getCustomisedAreas() 
	{

		if (!isset($this->_tmp['customised_areas'])) {
			$parent = &$this->getParentDesign();
			if (is_null($parent)) return Array();

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

		return $this->_tmp['customised_areas'];

	}// end getCustomisedAreas()

	/*
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




	/*
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
			if (!$this->deleteLink($existing_id_names[$id_name])) return false;
		}// end foreach

		foreach($added_id_names as $id_name) {
			$da = &$am->getAsset($parent_da_links[$id_name]['minorid'], $parent_da_links[$id_name]['type_code'], true);
			if (is_null($da)) continue;
			$this->createLink($da, SQ_LINK_TYPE_3, $id_name);
		}// end foreach

		//// THIRD, let's update the customised, but not specialised, design areas ////

		$links = $this->getCustomisedAreas();
		foreach($links as $link) {
			$da = &$am->getAsset($link['minorid'], $link['type_code']);
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
		if (file_exists($this->data_path.'/files/'.$filename)) {
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
