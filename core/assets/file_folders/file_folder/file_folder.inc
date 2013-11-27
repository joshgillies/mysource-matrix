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


require_once SQ_CORE_PACKAGE_PATH.'/files/file/file.inc';

/**
* File_Folder
* Base class for all the File Folder assets
*
* File Folders are basically file assets that combines the linked files
* into a single file using some compiler
*
* @author  Chiranjivi Upreti <cupreti@squiz.com.au>
* @version $Revision: $
* @package MySource_Matrix_Packages
* @subpackage __core__
*/
class File_Folder extends File
{

	/**
	* Constructor
	*
	* @param int	$assetid	the asset id to be loaded
	*
	*/
	function __construct($assetid=0)
	{
		$this->_ser_attrs = TRUE;
		parent::__construct($assetid);

	}//end constructor


	/**
	* Create this asset
	*
	* @param array	&$link	information used to create the initial link
	* @param array  $info   info about the uploaded file
	*
	* @return mixed int|boolean
	* @access public
	* @see Asset::create()
	*/
	public function create(Array &$link, $info=Array())
	{
		$this->_tmp['file_create_data'] =& $info;

		if (empty($info)) {
			// Try getting the uploaded file details
			$info = get_file_upload_info($this->getPrefix());
		}

		if ($info === FALSE || empty($info)) {
			// See if they've chosen a pre-uploaded file
			$edit = $this->getEditFns();
			$info = $edit->getChosenFileInfo($this->getPrefix());
		}

		$file_name = '';
		if ($GLOBALS['SQ_SYSTEM']->runLevelEnables(SQ_SECURITY_DATA_VALIDATION)) {

			//Create a new empty file

			$file_title = trim($this->attr('title'));
			$file_title = preg_replace('/\s/', '_', $file_title);
			$file_title = preg_replace('/[^A-Za-z0-9_\-]/', '', $file_title);

			$extension = $this->getFileExtension();
			$file_name = $file_title.'.min.'.$extension;

			$create_location = SQ_SYSTEM_ROOT.'/'.'data/temp';
			// Create an temporary empty file
			if (!fopen($create_location.'/'.$file_name, 'w+')) {
				trigger_localised_error('SYS0025', E_USER_WARNING, $file_name. " from given title ".$file_title);
				return FALSE;
			}

			// Create info array for the file just created

			$info = Array();

			$info['name'] = $file_name;
			$info['tmp_name'] = $create_location;
			$info['created_file'] = TRUE;
		}

		// Make the info array look like a result from getExistingFile()
		if (!isset($info['path'])) {
			$info['path'] = array_get_index($info, 'tmp_name', '');
		}
		if (!isset($info['filename'])) {
			$info['filename'] = array_get_index($info, 'name', '');
		}

		if ($GLOBALS['SQ_SYSTEM']->runLevelEnables(SQ_SECURITY_DATA_VALIDATION)) {
			if (!$this->validFile($info)) return FALSE;
		}

		if ($GLOBALS['SQ_SYSTEM']->runLevelEnables(SQ_SECURITY_DATA_VALIDATION)) {
			// Check that we are not going to have web path conflicts
			require_once SQ_INCLUDE_PATH.'/general_occasional.inc';
			$valid_names = make_valid_web_paths(Array($info['name']));
			$name = array_shift($valid_names);

			// Make grep sure the new web path is not already is use
			$bad_paths = $GLOBALS['SQ_SYSTEM']->am->webPathsInUse($link['asset'], Array($name));
			if (!empty($bad_paths)) {
				trigger_localised_error('CORE0086', E_USER_WARNING, $name);
				return FALSE;
			}

			$this->setAttrValue('name', $name);
		}//end if

		return parent::create($link, $info);

	}//end create()


	/**
	* Return the currently linked files assetids
	*
	* @return array
	* @access public
	*/
	public function getLinkedFiles()
	{
		$file_assetids = Array();

		$file_type = $this->getFileAssetType();
		$links = $GLOBALS['SQ_SYSTEM']->am->getLinks($this->id, SQ_LINK_NOTICE, $file_type, TRUE, 'major', 'linked_folder');
		foreach($links as $link) {
			if (!empty($link['minorid'])) {
				$file_assetids[] = $link['minorid'];
			}
		}

		return $file_assetids;

	}//end getLinkedFiles()


	/**
	* Return the linked files assetids that were linked at the last minification
	*
	* @return array
	* @access public
	*/
	public function getLinkedFilesAtLastMinification()
	{
		return $this->attr('linked_files');

	}//end getLinkedFilesAtLastMinification()


	/**
	* Merge the linked file into a single file using
	*
	* @return boolean
	* @access public
	*/
	public function mergeFiles()
	{
		$linked_file_assetids = $this->getLinkedFiles();
		if (empty($linked_file_assetids)) {
			return TRUE;
		}

		$merged_file_info = $this->getExistingFile();
		if (empty($merged_file_info['path'])) {
			trigger_error('File Folder\'s file path not found', E_USER_WARNING);
			return FALSE;
		}
		$merged_file_path = $merged_file_info['path'];

		$source_files = Array();
		foreach($linked_file_assetids as $linked_file_assetid) {
			$linked_file = $GLOBALS['SQ_SYSTEM']->am->getAsset($linked_file_assetid);
			$file_info = $linked_file->getExistingFile();
			if (empty($file_info['path'])) {
				trigger_error('File path not found for the asset #'.$linked_file_assetid, E_USER_WARNING);
				continue;
			}
			$source_files[] = $file_info['path'];
		}

		if (empty($source_files)) {
			// Nothing to merge
			return TRUE;
		}

		// Minify the files if required
		$minify = $this->attr('compiler_type') != 'none';
		require_once SQ_SYSTEM_ROOT.'/scripts/ees_minify/jsmin.php';
		$output = '';
		foreach ($source_files as $source_file) {
			$file_content = file_get_contents($source_file) ;
			$output .= $minify ? JSMin::minify($file_content) : $file_content;
		}

		// Write into the merged file
		$success = $this->setFileContent($output);

		return TRUE;

	}//end mergeFiles()


	/**
	* Set file's content
	*
	* @param string $content
	*
	* @return boolean
	* @access public
	*/
	public function setFileContent($content)
	{
		$file_edit_fns = $this->getEditFns();
		$prefix = $this->getPrefix();
		$file_info = Array(
						'name' 				=> $this->name,
						'tmp_name'			=> $this->data_path.'/'.$this->name.'.new',
						'non_uploaded_file' => TRUE,
					);
		if (!string_to_file($content, $file_info['tmp_name'])) {
			trigger_error('Error occurred when setting the file content into a temporary file', E_USER_NOTICE);
			return FALSE;
		}

		require_once SQ_INCLUDE_PATH.'/backend_outputter.inc';
		$o = new Backend_Outputter();
		$GLOBALS['SQ_SYSTEM']->setRunLevel(SQ_RUN_LEVEL_FORCED);
			$success = $file_edit_fns->processFileUpload($this, $o, $prefix, $file_info);
		$GLOBALS['SQ_SYSTEM']->restoreRunLevel();

		unlink($file_info['tmp_name']);

		if (!$success) {
			trigger_error('Error occurred when setting the file content', E_USER_NOTICE);
			return FALSE;
		}

		return TRUE;

	}//end setFileContent()


	/**
	* Returns TRUE if the merged file needs to be regenerated
	*
	* @return boolean
	* @access public
	*/
	public function regenerationRequired()
	{
		// Curretly linked files
		$file_assetids = $this->getLinkedFiles();

		// File assetids linked at the last minification
		$linked_files = $this->getLinkedFilesAtLastMinification();

		// Regeneration will be required based on if the new files have been added or removed
		$regeneration_required = array_diff($file_assetids, $linked_files) || array_diff($linked_files, $file_assetids);

		if (!$regeneration_required) {
			$file_assetids[] = $this->id;
			if (!empty($file_assetids)) {
				$files_info = $GLOBALS['SQ_SYSTEM']->am->getAssetInfo($file_assetids);
			}

			// Minified file last update (file regeneration) date
			$last_updated = isset($files_info[$this->id]['updated']) ? $files_info[$this->id]['updated'] : '';

			foreach($files_info as $file_id => $file_info) {
				// We are not printing ourself
				if ($file_id == $this->id) {
					continue;
				}

				if ($last_updated && $file_info['updated'] > $last_updated) {
					$regeneration_required = TRUE;
					break;
				}
			}//end foreach
		}

		return $regeneration_required;

	}//end regenerationRequired()


	/**
	* Get the extension of the folder's file type
	*
	* @return string
	* @access public
	*/
	public function getFileExtension()
	{
		return '';

	}//end minifyFiles()


	/**
	* Get the file asset type that this folder supports
	*
	* @return string
	* @access public
	*/
	public function getFileAssetType()
	{
		return '';

	}//end minifyFiles()


	/**
	* Returns an array of all the permitted links type, the type asset and the cardinality
	*
	* @return array
	* @access private
	* @see Asset::_getAllowLinks()
	*/
	public function _getAllowedLinks()
	{
		$asset_type = $this->getFileAssetType();

		return Array(
				SQ_LINK_TYPE_1	=> Array(
									$asset_type	=> Array('card'	=> 'M', 'exclusive'	=> FALSE),
								   ),
				SQ_LINK_TYPE_2	=> Array(
									$asset_type	=> Array('card'	=> 'M', 'exclusive'	=> FALSE),
								   ),
				SQ_LINK_TYPE_3	=> Array(),
				SQ_LINK_NOTICE	=> Array(
									$asset_type	=> Array('card'	=> 'M', 'exclusive'	=> FALSE),
								   ),
			   );

	}//end _getAllowedLinks()


}//end class

?>
