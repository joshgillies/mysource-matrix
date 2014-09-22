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
* $Id: design_css_customisation.inc,v 1.19 2012/08/30 01:09:06 ewang Exp $
*
*/


require_once SQ_CORE_PACKAGE_PATH.'/designs/design_css/design_css.inc';

/**
* Design_Css_Customisation
*
* Purpose
*
*
* @author  Avi Miller <avi.miller@squiz.net>
* @version $Revision: 1.19 $
* @package MySource_Matrix_Packages
* @subpackage __core__
*/
class Design_Css_Customisation extends Design_Css
{


	/**
	* Constructor
	*
	* @param int	$assetid	the asset id to be loaded
	*
	*/
	function __construct($assetid=0)
	{
		parent::__construct($assetid);

	}//end constructor


	/**
	* Returns an array of all the permitted links type, the type asset and the cardinality
	*
	* @return array
	* @access private
	* @see Asset::_getAllowLinks()
	*/
	function _getAllowedLinks()
	{
		$links = parent::_getAllowedLinks();
		$links[SQ_LINK_TYPE_3]['file'] = Array('card' => 'M', 'exclusive' => FALSE);
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
	public function deleteLink($linkid)
	{
		// first, we should try and find the link
		$delete_link = $GLOBALS['SQ_SYSTEM']->am->getLinkById($linkid, $this->id);
		if (empty($delete_link)) {
			trigger_localised_error('CORE0155', sprintf(translate('Existing Link #%s not found, unable to delete link'), $linkid), E_USER_WARNING)
			return FALSE;
		}

		$am    = $GLOBALS['SQ_SYSTEM']->am;
		$minor = $am->getAsset($delete_link['minorid'], $delete_link['minor_type_code']);

		$GLOBALS['SQ_SYSTEM']->changeDatabaseConnection('db2');
		$GLOBALS['SQ_SYSTEM']->doTransaction('BEGIN');

		if (!$this->deleteExistingLink($linkid)) {
			$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
			$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();
			return FALSE;
		}

		// special processing if we are deleting a TYPE_2 linked file (a file overridding a design file)
		if ($delete_link['link_type'] == SQ_LINK_TYPE_2 && ($minor instanceof File)) {

			$design = $this->getParentDesign();

			// go through all the links of the design we are customisaing and find a file
			// linked to it with the same name as the one we just deleted so it can take its place
			$file_link_ids = Array();
			$file_links = $GLOBALS['SQ_SYSTEM']->am->getLinks($design->id, SQ_LINK_TYPE_2, 'file', FALSE);
			foreach ($file_links as $link) {
				$file_link_ids[$link['minorid']] = $link['linkid'];
			}
			$file_link_info = $GLOBALS['SQ_SYSTEM']->am->getAssetInfo(array_keys($file_link_ids));

			foreach ($file_link_info as $info) {
				if ($info['name'] == $minor->name) {
					// found the link to create
					$new_minor = $am->getAsset($info['assetid']);
					if (!$this->createLink($new_minor, SQ_LINK_TYPE_3, '', NULL, '1')) {
						trigger_localised_error('CORE0162', translate('Failed creating file link to customisation'), E_USER_WARNING)
						$am->forgetAsset($design);
						$am->forgetAsset($new_minor);
						$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
						$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();
						return FALSE;
					}
					$am->forgetAsset($new_minor);
				}
			}//end foreach $file_links

			$am->forgetAsset($design);
		}

		$GLOBALS['SQ_SYSTEM']->doTransaction('COMMIT');
		$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();
		return TRUE;

	}//end deleteLink()


	/**
	* Does the actual removing of the link for the customisation
	*
	* The default deleteLink() replaces the (Type-2) links of any customised design files with a Type-3
	* to the uncustomised version. This screws things around when creating the customised versions in
	* the first place.
	*
	* @param int	$linkid	the link id of the link to remove
	*
	* @return boolean
	* @access public
	*/
	public function deleteExistingLink($linkid)
	{
		// first, we should try and find the link
		$delete_link = $GLOBALS['SQ_SYSTEM']->am->getLinkById($linkid, $this->id);
		if (empty($delete_link)) {
			trigger_localised_error('CORE0155', sprintf(translate('Existing Link #%s not found, unable to delete link'), $linkid), E_USER_WARNING)
			return FALSE;
		}

		$am    = $GLOBALS['SQ_SYSTEM']->am;
		$minor = $am->getAsset($delete_link['minorid'], $delete_link['minor_type_code']);

		$GLOBALS['SQ_SYSTEM']->changeDatabaseConnection('db2');
		$GLOBALS['SQ_SYSTEM']->doTransaction('BEGIN');

		if (!parent::deleteLink($linkid)) {
			$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
			$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();
			return FALSE;
		}

		$GLOBALS['SQ_SYSTEM']->doTransaction('COMMIT');
		$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();

		return TRUE;

	}//end deleteExistingLink()


	/**
	* Called whenever any type of link is changed, allows caching of objects, links and other stuff
	* to occur but not cause integrity issues during script execution
	* returns TRUE it it's successfull
	*
	* @return void
	* @access private
	*/
	public function linksUpdated()
	{
		parent::linksUpdated();
		unset($this->_tmp['parent_design']);
		unset($this->_tmp['customised_areas']);
		unset($this->_tmp['uncustomised_areas']);

	}//end linksUpdated()


	/**
	* Returns the parent design/design_customisation that this customisation is customising
	*
	* @return mixed object|NULL
	* @access public
	*/
	public function getParentDesign()
	{
		if (empty($this->_tmp['parent_design'])) {
			$parent_link = $GLOBALS['SQ_SYSTEM']->am->getLink($this->id, SQ_LINK_TYPE_2, 'design_css', FALSE, 'customisation', 'minor');
			$this->_tmp['parent_design'] = $GLOBALS['SQ_SYSTEM']->am->getAsset($parent_link['majorid'], $parent_link['major_type_code']);
		}
		return $this->_tmp['parent_design'];

	}//end getParentDesign()


	/**
	* Returns an array of links for design areas that have been customised by this design
	*
	* @return array
	* @access public
	*/
	public function getCustomisedAreas()
	{
		if (!isset($this->_tmp['customised_areas'])) {
			$parent = $this->getParentDesign();
			if (!is_null($parent)) {
				// OK, get our design area links and our parents
				$our_links     = $this->getDesignAreaLink();
				$parents_links = $parent->getDesignAreaLink();
				$this->_tmp['customised_areas'] = Array();

				// Basically we do a search through our links and see if the design areas (ie the id_names)
				// are pointing to the same assetid or not to ascertain whether we have customised this area or not
				foreach ($our_links as $our_link) {
					foreach ($parents_links as $parents_link) {
						// if the id_names are the same
						if ($our_link['value'] == $parents_link['value']) {
							// if the id's are different, we have customised this one
							if ($parents_link['minorid'] == $our_link['minorid']) {
								$this->_tmp['uncustomised_areas'][] = $our_link;
							} else {
								$this->_tmp['customised_areas'][]   = $our_link;
							}
							continue;
						}//end if
					}//end foreach
				}//end foreach
			}//end if
		}//end if

		if (!isset($this->_tmp['customised_areas'])) {
			$this->_tmp['customised_areas'] = Array();
		}
		if (!isset($this->_tmp['uncustomised_areas'])) {
			$this->_tmp['uncustomised_areas'] = Array();
		}

		return $this->_tmp['customised_areas'];

	}//end getCustomisedAreas()


	/**
	* Returns an array of links for design areas that have NOT been customised by this design
	*
	* @return array
	* @access public
	*/
	public function getUnCustomisedAreas()
	{
		if (!isset($this->_tmp['uncustomised_areas'])) {
			$this->getCustomisedAreas();
		}//end if

		return $this->_tmp['uncustomised_areas'];

	}//end getUnCustomisedAreas()


	/**
	* Updates this customisation from our parent,
	* ie. Updates our contents array and the contents arrays for any customised design areas
	*     No need to do the uncustomised design areas because they would be done by our parent
	*
	* @param Design_CSS	$parent	the parent design for this customisation
	*
	* @return boolean
	* @access protected
	*/
	public function updateFromParent(Design_CSS $parent, $update_children = TRUE)
	{
		$am = $GLOBALS['SQ_SYSTEM']->am;

		//// FIRST, let's set our new contents ////
		if (!$this->setAttrValue('contents', $parent->attr('contents')) || !$this->saveAttributes()) {
			return FALSE;
		}

		//// SECOND, let's remove any design area links that we aren't going to be using any more ////

		$tmp = $parent->getDesignAreaLink();
		$parent_da_links = Array();
		foreach ($tmp as $link) {
			$parent_da_links[$link['value']] = $link;
		}

		$tmp = $this->getDesignAreaLink();
		$this_da_links = Array();
		foreach ($tmp as $link) {
			$this_da_links[$link['value']] = $link;
		}

		$remove_id_names = array_diff(array_keys($this_da_links),   array_keys($parent_da_links));
		$added_id_names  = array_diff(array_keys($parent_da_links), array_keys($this_da_links));

		// now remove any unused design area customisations
		foreach ($remove_id_names as $id_name) {
			if (!$this->deleteLink($this_da_links[$id_name]['linkid'])) {
				return FALSE;
			}
		}//end foreach

		foreach ($added_id_names as $id_name) {
			$da = $am->getAsset($parent_da_links[$id_name]['minorid'], $parent_da_links[$id_name]['minor_type_code'], TRUE);
			if (is_null($da)) continue;
			$this->createLink($da, SQ_LINK_TYPE_3, $id_name, NULL, '1');
		}//end foreach

		//// THIRD, let's update the customised design areas ////

		$links = $this->getCustomisedAreas();
		foreach ($links as $link) {
			$da = $am->getAsset($link['minorid'], $link['minor_type_code']);
			if (is_null($da)) continue;

			$parent_da = $parent->getDesignArea($link['value']);
			if (is_null($parent_da)) continue;

			$da->setAttrValue('contents', $parent_da->attr('contents'));
			$da->saveAttributes();

		}//end foreach

		//// AND FINALLY, regenerate our design file and update any of our customisations ////
		$this->generateDesignFile(FALSE); // not recursive because update customisations takes care of that
		if($update_children)
		    $this->updateCustomisations();

		return TRUE;

	}//end updateFromParent()


	/**
	* Paints out our design
	*
	* @param Asset	$ASSET		the asset that we are going to print the body for
	* @param array	$FILE_URLS	an array of filename => URL to use to print files in the design
	*							doesnt need to be passed in unless some URLs are overridden by
	*							a customisation of the design
	* @param boolean $send_header   Whether to send 'css' header when painting
	*
	* @return void
	* @access public
	*/
	public function paint(Asset $ASSET, Array $FILE_URLS=Array(), $send_header=TRUE)
	{
		// generate an array of all the URLs we are going to need for the files in this customisation
		// that are inherited from the design we are customising
		$file_link_ids = Array();
		$file_links = $GLOBALS['SQ_SYSTEM']->am->getLinks($this->id, SQ_LINK_TYPE_2 | SQ_LINK_TYPE_3, 'file', FALSE);
		foreach ($file_links as $link) {
			$file_link_ids[] = $link['minorid'];
		}

		$file_link_info = $GLOBALS['SQ_SYSTEM']->am->getAssetInfo(array_unique($file_link_ids));
		$file_link_urls = $GLOBALS['SQ_SYSTEM']->am->getAssetURL(array_unique($file_link_ids));

		foreach ($file_link_info as $minorid => $file_info) {
			$file_url = $file_link_urls[$minorid];
			if ($file_url == '') {
				$file_url = "Javascript: alert(js_translate('no_url_found', '".addslashes($file_info['name'])."', '".addslashes($this->attr('id_name'))."'));";
			} else if ($this->attr('static_url_versioning')){
				$served_by_apache = strpos($file_url, '/__data/') !== FALSE || (SQ_CONF_STATIC_ROOT_URL && strpos($file_url, SQ_CONF_STATIC_ROOT_URL.'/') !== FALSE);
				if ($served_by_apache) $file_url .= '?v='.$file_info['version'];
			}
			$FILE_URLS[$file_info['name']] = $file_url;
		}

		return parent::paint($ASSET, $FILE_URLS, $send_header);

	}//end paint()


}//end class
?>
