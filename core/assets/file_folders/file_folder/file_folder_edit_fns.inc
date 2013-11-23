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
*/


require_once SQ_CORE_PACKAGE_PATH.'/files/file/file_edit_fns.inc';

/**
* File_Folder_Edit_Fns
*
* Purpose
*
*
* @author  Chiranjivi Upreti <cupreti@squiz.com.au>
* @version $Revision: $
* @package MySource_Matrix_Packages
* @subpackage __core__
*/
class File_Folder_Edit_Fns extends File_Edit_Fns
{

	/**
	* Constructor
	*
	*/
	function __construct()
	{
		$this->_ser_attrs = TRUE;
		parent::__construct();

	}//end constructor


	/**
	* Paint the merged file last update date
	*
	* @param Asset				$asset	the asset whose interface we are painting
	* @param Backend_Outputter	$o		the Backend Outputter object
	* @param string				$prefix	prefix for the form element
	*
	* @return boolean
	* @access public
	*/
	public function paintFileLastUpdateDate(Asset $asset, Backend_Outputter $o, $prefix)
	{
		echo date('Y-m-d H:i:s', $asset->updated);
		if ($asset->regenerationRequired()) {
			echo '&nbsp;'.'<span style="color: red">Out of date!</span>';
		}

	}//end paintFileLastUpdateDate()


	/**
	* Paint the interface for the File Folder current files
	*
	* @param Asset				$asset	the asset whose interface we are painting
	* @param Backend_Outputter	$o		the Backend Outputter object
	* @param string				$prefix	prefix for the form element
	*
	* @return boolean
	* @access public
	*/
	public function paintLinkedFiles(Asset $asset, Backend_Outputter $o, $prefix)
	{
		// Curretly linked files
		$file_assetids = $asset->getLinkedFiles();
		$file_asset_type = $asset->getFileAssetType();

		if ($asset->writeAccess('links')) {
			multiple_asset_finder($prefix.'_linked_files', $file_assetids, Array($file_asset_type => 'D'));
		} else {
			$infos = $GLOBALS['SQ_SYSTEM']->am->getAssetInfo($file_assetids);
			?><ul style="margin: 1px 15px;"><?php
			foreach ($infos as $assetid => $info) {
				echo '<li>'.get_asset_tag_line($assetid).'</li>';
			}
			?></ul><?php
		}

		return TRUE;

	}//end paintLinkedFiles()


	/**
	* Process the interface for the File Folder current files
	*
	* @param Asset				$asset	the asset whose interface we are painting
	* @param Backend_Outputter	$o		the Backend Outputter object
	* @param string				$prefix	prefix for the form element
	*
	* @return boolean
	* @access public
	*/
	public function processLinkedFiles(Asset $asset, Backend_Outputter $o, $prefix)
	{
		if (!isset($_POST[$prefix.'_linked_files']) || (isset($asset->_tmp['reverting_to_system_version']) && $asset->_tmp['reverting_to_system_version'])) {
			return FALSE;
		}
		$file_asset_type = $asset->getFileAssetType();

		$new_linked_files = $_POST[$prefix.'_linked_files'];
		foreach ($new_linked_files as $new_linked_file) {
			$new_linked_file_ids[] = $new_linked_file['assetid'];
		}

		// Get existing linked files
		$file_links = $GLOBALS['SQ_SYSTEM']->am->getLinks($asset->id, SQ_LINK_NOTICE, $file_asset_type, TRUE, 'major', 'linked_folder');

		$GLOBALS['SQ_SYSTEM']->changeDatabaseConnection('db2');
		$GLOBALS['SQ_SYSTEM']->doTransaction('BEGIN');
		$ok = TRUE;

		// Delete the file assets that have been removed
		foreach ($file_links as $file_link) {
			if (!in_array($file_link['minorid'], $new_linked_file_ids)) {
				if (!$GLOBALS['SQ_SYSTEM']->am->deleteAssetLink($file_link['linkid'])) {
					$ok = FALSE;
				}
			}
		}

		// Create the new links
		foreach ($new_linked_files as $index => $node) {
			// Check if the node already exists. If so, ignore it
			foreach ($file_links as $file_link) {
				if ($file_link['minorid'] == $node['assetid']) {
					continue 2;
				}
			}
			// Otherwise, create the appropriate notice link
			if ($node['assetid'] != 0) {
				$new_file = $GLOBALS['SQ_SYSTEM']->am->getAsset($node['assetid']);
				// Check the asset type
				if ($new_file instanceof $file_asset_type) {
					if (!$asset->createLink($new_file, SQ_LINK_NOTICE, 'linked_folder')) {
						$ok = FALSE;
						trigger_error('Unable to link the File #'.$new_file->id.' to the File Folder');
						break;
					}
				}
			}//end if
		}//end foreach

		if ($ok) {
			$GLOBALS['SQ_SYSTEM']->doTransaction('COMMIT');
		} else {
			$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
		}

		$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();

		return $ok;

	}//end processLinkedFiles()


	/**
	* Paint the interface for the Minification options
	*
	* @param Asset				$asset	the asset whose interface we are painting
	* @param Backend_Outputter	$o		the Backend Outputter object
	* @param string				$prefix	prefix for the form element
	*
	* @return boolean
	* @access public
	*/
	public function paintMinifyOption(Asset $asset, Backend_Outputter $o, $prefix)
	{
		$wa = $asset->writeAccess('attributes');

		$attribute = $asset->getAttribute('compiler_type');
		$options = $attribute->getOptions();

		$current_val = $asset->attr('compiler_type');
		if ($wa) {
			combo_box($prefix.'_compiler_type', $options, FALSE, $current_val);
		} else {
			echo $options[$current_val];
		}

	}//end paintMinifyOption()


	/**
	* Process the interface for the Minification options
	*
	* @param Asset				$asset	the asset whose interface we are painting
	* @param Backend_Outputter	$o		the Backend Outputter object
	* @param string				$prefix	prefix for the form element
	*
	* @return boolean
	* @access public
	*/
	public function processMinifyOption(Asset $asset, Backend_Outputter $o, $prefix)
	{
		$wa = $asset->writeAccess('attributes');
		if (!$wa) {
			return FALSE;
		}

		$success = TRUE;
		if (isset($_POST[$prefix.'_compiler_type'])) {
			$new_val = $_POST[$prefix.'_compiler_type'];
			$current_val = $asset->attr('compiler_type');
			if ($new_val != $current_val) {
				$asset->setAttrValue('compiler_type', $new_val);
				$success = $asset->mergeFiles();
				if ($success) {
					$file_assetids = $asset->getLinkedFiles();
					$success = $asset->setAttrValue('linked_files', $file_assetids);
				}
			}
		}

		return TRUE;

	}//end processMinifyOption()


	/**
	* Paint the interface for the "Renenerate File" option
	*
	* @param Asset				$asset	the asset whose interface we are painting
	* @param Backend_Outputter	$o		the Backend Outputter object
	* @param string				$prefix	prefix for the form element
	*
	* @return boolean
	* @access public
	*/
	public function paintRegenerateFile(Asset $asset, Backend_Outputter $o, $prefix)
	{
		$wa = $asset->writeAccess('attributes');
		check_box($prefix.'_regenerate', '1', FALSE, '', !$wa ? 'disabled' : '');

		return TRUE;

	}//end paintRegenerateFile()

	/**
	* Process the interface for the "Renenerate File" option
	*
	* @param Asset				$asset	the asset whose interface we are painting
	* @param Backend_Outputter	$o		the Backend Outputter object
	* @param string				$prefix	prefix for the form element
	*
	* @return boolean
	* @access public
	*/
	public function processRegenerateFile(Asset $asset, Backend_Outputter $o, $prefix)
	{
		$wa = $asset->writeAccess('attributes');
		if (!$wa) {
			return FALSE;
		}

		if (isset($_POST[$prefix.'_regenerate'])) {
			if ($asset->mergeFiles()) {
				$file_assetids = $asset->getLinkedFiles();
					$asset->setAttrValue('linked_files', $file_assetids);
			}
		}

		return TRUE;

	}//end processRegenerateFile()


}//end class

?>
