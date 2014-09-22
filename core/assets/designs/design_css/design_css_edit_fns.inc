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
* $Id: design_css_edit_fns.inc,v 1.24 2012/08/30 01:09:06 ewang Exp $
*
*/


require_once SQ_CORE_PACKAGE_PATH.'/designs/design/design_edit_fns.inc';

/**
* Design_Css_Edit_Fns
*
* Purpose
*
*
* @author  Avi Miller <avi.miller@squiz.net>
* @version $Revision: 1.24 $
* @package MySource_Matrix_Packages
* @subpackage __core__
*/
class Design_Css_Edit_Fns extends Design_Edit_Fns
{


	/**
	* Constructor
	*
	*/
	function __construct()
	{
		parent::__construct();
		$this->static_screens['details']['force_unlock'] = FALSE;

	}//end constructor


	/**
	* Parses the string into an array recursively cut up into html and tag pieces
	*
	* @param object	&$asset			the asset to which we belong
	* @param string	$file_contents	the contents to parse
	*
	* @return array
	* @access public
	*/
	public function _parseString(Asset $asset, $file_contents)
	{
		// Let's place an print operation at the top of the file
		// to allow us to print the default header stuff like
		// copyright and content-type information
		$file_contents= '<'.$this->tag_name."_PRINT id_name='__global__' var='css_header' />\n"
						.$file_contents;

		return parent::_parseString($asset, $file_contents);

	}//end _parseString()


	/**
	* Creates a new file asset, populates it, and links it to $asset
	*
	* @param Asset	$asset	the asset to link the file to
	* @param array	$info	info about the uploaded file
	*
	* @return boolean
	* @access public
	*/
	public function _processUploadedFile(Asset $asset, Array $info=Array())
	{
		// if we already have a file with the same name TYPE_3 linked to us,
		// delete this link so we link up this new file
		$existing_ids = Array();
		$existing = $GLOBALS['SQ_SYSTEM']->am->getLinks($asset->id, SQ_LINK_TYPE_3, 'file', FALSE);
		foreach ($existing as $link) {
			$existing_ids[$link['minorid']] = $link['linkid'];
		}

		$existing_info = $GLOBALS['SQ_SYSTEM']->am->getAssetInfo(array_keys($existing_ids));
		foreach ($existing_info as $assetid => $existing_info) {
			// if the name is the same, delete the link
			if ($existing_info['name'] == $info['name']) {
				$linkid = $existing_ids[$assetid];
				$asset->deleteExistingLink($linkid);
				$asset->updateLookups();
			}
		}

		return parent::_processUploadedFile($asset, $info);

	}//end _processUploadedFile()


	/**
	* Paint interface for a customisation
	*
	* @param object	&$asset	the asset whose interface we are painting
	* @param object	&$o		the outputter class
	* @param string	$prefix	prefix for the form element
	*
	* @return boolean
	* @access public
	*/
	public function paintNewCustomisation(Design_CSS $asset, Backend_Outputter $o, $prefix)
	{
		if (!$asset->writeAccess('links')) return FALSE;
		check_box($prefix.'_new_customisation');
		text_box($prefix.'_new_customisation_name', $asset->name.'.css', 40);
		return TRUE;

	}//end paintNewCustomisation()


	/**
	* Process the options for creating a new customisation
	*
	* @param object	&$asset	the asset to which we belong
	* @param object	&$o		the outputter class
	* @param string	$prefix	prefix for the form element
	*
	* @return boolean
	* @access public
	*/
	public function processNewCustomisation(Design_CSS $asset, Backend_Outputter $o, $prefix)
	{
		if (empty($_POST[$prefix.'_new_customisation']) || !($new_name = trim($_POST[$prefix.'_new_customisation_name']))) {
			return FALSE;
		}

		$GLOBALS['SQ_SYSTEM']->changeDatabaseConnection('db2');
		$GLOBALS['SQ_SYSTEM']->doTransaction('BEGIN');

		// First let's duplicate ourselves, ignoring dependants and the directory with the files
		$map = Array();
		$create_link = Array();

		// increase run level to allow setting of components without cloning permissions from parents
		if ($asset->writeAccess('all')) {
             $GLOBALS['SQ_SYSTEM']->setRunLevel($GLOBALS['SQ_SYSTEM']->getRunLevel() & SQ_RUN_LEVEL_FORCED);
             // This clone will be morphed to customisation later, so it will NOT clone the parse file.
             $asset->_tmp['custom'] = TRUE;
			 $clone = $GLOBALS['SQ_SYSTEM']->am->cloneAsset($asset, $create_link, $map, Array('attributes', 'permissions', 'roles'), FALSE);
			 unset ($asset->_tmp['custom']);
			 $GLOBALS['SQ_SYSTEM']->restoreRunLevel();
		}



		if (is_null($clone)) {
			$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
			$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();
			return FALSE;
		}

		$GLOBALS['SQ_SYSTEM']->am->rememberAsset($da);

		// Acquire a lock on the new object
		if (!$GLOBALS['SQ_SYSTEM']->am->acquireLock($clone->id, 'all')) {
			$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
			$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();
			return FALSE;
		}

		// Now let's morph the dupe into a design customisation
		$clone = $clone->morph('design_css_customisation');

		if (!$clone) {
			$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
			$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();
			return FALSE;
		}

		// and link it back to ourselves
		if (!$asset->createLink($clone, SQ_LINK_TYPE_2, 'customisation', NULL, '1')) {
			$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
			$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();
			return FALSE;
		}
		if (!($clone->setAttrValue('id_name', $new_name) && $clone->saveAttributes())) {
			$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
			$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();
			return FALSE;
		}

		$am = $GLOBALS['SQ_SYSTEM']->am;

		// Now create the links to our design areas
		$da_links = $asset->getDesignAreaLink();
		foreach ($da_links as $link) {
			$da = $am->getAsset($link['minorid'], $link['minor_type_code'], TRUE);
			if (is_null($da)) continue;
			if (!$clone->createLink($da, SQ_LINK_TYPE_3, $link['value'], NULL, '1')) {
				$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
				$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();
				return FALSE;
			}
		}// end foreach

		// set an initial web path
		$initial_path = strtolower($new_name);
		require_once SQ_INCLUDE_PATH.'/general_occasional.inc';
		$valid_paths = make_valid_web_paths(Array($initial_path));
		$good_paths = $GLOBALS['SQ_SYSTEM']->am->webPathsInUse($asset, $valid_paths, $clone->id, TRUE);
		if (!$clone->saveWebPaths($good_paths)) {
			$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
			$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();
			return FALSE;
		}

		// link up all the files from this design to the customisation
		$file_link_ids = Array();
		$file_links = $GLOBALS['SQ_SYSTEM']->am->getLinks($asset->id, SQ_LINK_TYPE_2 | SQ_LINK_TYPE_3, 'file', FALSE);
		foreach ($file_links as $link) {
			$minor = $am->getAsset($link['minorid'], $link['minor_type_code']);
			if (!$clone->createLink($minor, SQ_LINK_TYPE_3, '', NULL, 1)) {
				trigger_localised_error('CORE0162', E_USER_WARNING);
				$am->forgetAsset($minor);
				$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
				$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();
				return FALSE;
			}
		}
		$am->forgetAsset($minor);

		// release the lock on the new object
		if (!$GLOBALS['SQ_SYSTEM']->am->releaseLock($clone->id, 'all')) {
			$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
			$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();
			return FALSE;
		}

		$GLOBALS['SQ_SYSTEM']->doTransaction('COMMIT');
		$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();

		return TRUE;

	}//end processNewCustomisation()


	/**
	* Paints this assets current customisations
	*
	* @param Design_CSS			$asset	the asset whose interface we are painting
	* @param Backend_Outputter	$o		the outputter class
	* @param string				$prefix	prefix for the form element
	*
	* @return boolean
	* @access public
	*/
	public function paintCurrentCustomisations(Design_CSS $asset, Backend_Outputter $o, $prefix)
	{
		$customisation_links = $GLOBALS['SQ_SYSTEM']->am->getLinks($asset->id, SQ_LINK_TYPE_2, 'design_css_customisation', TRUE, 'major', 'customisation');
		if ($customisation_links) {
			$am = $GLOBALS['SQ_SYSTEM']->am;
			foreach ($customisation_links as $link) {
				$customisation = $am->getAsset($link['minorid'], $link['minor_type_code']);
				if (is_null($customisation)) continue;
				echo '# ', $customisation->id, '&nbsp;&nbsp;<a href="', $customisation->getBackendHref(), '">', $customisation->name, '</a><br>';

			}

		} else {
			echo translate('No Customisations Created');

		}

	}//end paintCurrentCustomisations()


}//end class
?>
