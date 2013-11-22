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

		// File assetids linked at the last minification
		$linked_files = $asset->getLinkedFilesAtLastMinification();

		// Regeneration will be required based on if the new files have been linked or un-linked
		$regeneration_required = array_diff($file_assetids, $linked_files) || array_diff($linked_files, $file_assetids);

		$file_assetids[] = $asset->id;
		if (!empty($file_assetids)) {
			$files_info = $GLOBALS['SQ_SYSTEM']->am->getAssetInfo($file_assetids);
		}

		// Minified file last update (file regeneration) date
		$last_updated = isset($files_info[$asset->id]['updated']) ? $files_info[$asset->id]['updated'] : '';

		?>
			<table class="sq-backend-table" style="width: 360px;">
				<tr>
					<td class="sq-backend-table-header" style="width: 250px;"><?php echo 'Filename'; ?></td>
					<td class="sq-backend-table-header" style="width: 110px;"><?php echo 'Last Updated'; ?></td>
				</tr>
		<?php
			foreach($files_info as $file_id => $file_info) {
				// We are not printing ourself
				if ($file_id == $asset->id) {
					continue;
				}
				$updated_flag = '';
				if ($last_updated && $file_info['updated'] > $last_updated) {
					$updated_flag = '*';
					$regeneration_required = TRUE;
				}
				echo '<tr>';
				echo '<td>'.$file_info['name'].'&nbsp; (#'.$file_id.')'.$updated_flag.'</td>';
				echo '<td>'.$file_info['updated'].'</td>';
				echo '</tr>';
			}

		?>
			</table>
		<?php
		if ($regeneration_required) {
			echo "<br />File(s) marked with '*' has been updated since the last file regeneration. ";
			echo '<span style="color: red">File regeneration required!</span>';
		}

		return TRUE;

	}//end paintLinkedFiles()


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
