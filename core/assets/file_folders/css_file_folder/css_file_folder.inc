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


require_once SQ_CORE_PACKAGE_PATH.'/file_folders/file_folder/file_folder.inc';

/**
* Css_File_Folder
*
* @author  Chiranjivi Upreti <cupreti@squiz.com.au>
* @version $Revision: $
* @package MySource_Matrix_Packages
* @subpackage __core__
*/
class Css_File_Folder extends File_Folder
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
	* Merge and minifiy the linked file into a single file
	*
	* @return boolean
	* @access public
	*/
	public function _minifyFiles()
	{

	}//end minifyFiles()


	/**
	* Get the extension of the folder's file type
	*
	* @return boolean
	* @access public
	*/
	public function getFileExtension()
	{
		return 'css';

	}//end getFileExtension()


	/**
	* Get the file asset types that this folder supports
	*
	* @return boolean
	* @access public
	*/
	public function getFileAssetTypes()
	{
		return Array(
				'css_file',
				'design_css',
			);

	}//end getFileAssetTypes()


}//end class

?>
