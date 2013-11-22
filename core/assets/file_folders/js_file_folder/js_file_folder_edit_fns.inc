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


require_once SQ_CORE_PACKAGE_PATH.'/file_folders/file_folder/file_folder_edit_fns.inc';

/**
* JS_File_Folder_Edit_Fns
*
* Purpose
*
*
* @author  Chiranjivi Upreti <cupreti@squiz.com.au>
* @version $Revision: $
* @package MySource_Matrix_Packages
* @subpackage __core__
*/
class Js_File_Folder_Edit_Fns extends File_Folder_Edit_Fns
{

	/**
	* Constructor
	*
	*/
	function __construct()
	{
		parent::__construct();

	}//end constructor


	/**
	* Paint the Source Map file
	*
	* @param Asset				$asset	the asset whose interface we are painting
	* @param Backend_Outputter	$o		the Backend Outputter object
	* @param string				$prefix	prefix for the form element
	*
	* @return boolean
	* @access public
	*/
	public function paintSourceMapFile(Asset $asset, Backend_Outputter $o, $prefix)
	{
		if ($asset->sourceMapEnabled()) {
			$sm_filename = $asset->attr('name').'.map';
			$sm_path = $asset->data_path.'/'.$sm_filename;
			echo htmlspecialchars(file_get_contents($sm_path));
		}

		return TRUE;

	}//end paintSourceMapFile()


	/**
	* Paint the Source Map file
	*
	* @param Asset				$asset	the asset whose interface we are painting
	* @param Backend_Outputter	$o		the Backend Outputter object
	* @param string				$prefix	prefix for the form element
	*
	* @return boolean
	* @access public
	*/
	public function paintSourceJsFiles(Asset $asset, Backend_Outputter $o, $prefix)
	{
		if ($asset->sourceMapEnabled()) {
			$filenames = $asset->attr('source_map_files');
			foreach($filenames as $filename) {
				$file_path = $asset->data_path.'/'.$filename;

				if (is_file($file_path) && pathinfo($filename, PATHINFO_EXTENSION) == 'js') {
					echo '<b>'.$filename.'</b><br />';
					echo nl2br(htmlspecialchars(file_get_contents($file_path)));
					echo '<br /><br />';
				}//end if
			}//end foreach
		}//end if

		return TRUE;

	}//end paintSourceJsFiles()

}//end class

?>
