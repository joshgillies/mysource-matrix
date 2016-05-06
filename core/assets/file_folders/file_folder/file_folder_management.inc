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


require_once SQ_INCLUDE_PATH.'/asset_management.inc';

/**
* File_Folder_Management
*
* @see Asset_Management
*
* @author  Chiranjivi Upreti <cupreti@squiz.com.au>
* @version $Revision: $
* @package MySource_Matrix_Packages
* @subpackage __core__
*/
class File_Folder_Management extends Asset_Management
{


	/**
	* Constructor
	*
	*/
	function __construct(Package_Manager $pm)
	{
		Asset_Management::__construct($pm);

		$this->vars['compiler_type']	= Array(
											'added'			=> '0.0.1',
											'type'			=> 'selection',
											'default'		=> 'basic',
											'parameters'    => Array(
																'multiple'      => FALSE,
																'allow_empty'   => FALSE,
																'options'       => Array(
																						'none'	=> 'Merge Without Minification',
																						'basic'	=> 'Basic Minification',
																					),
												),
											'description'	=> 'Compilation type to minify the linked files when merging',
										);

		$this->vars['linked_files']	= Array(
											'added'			=> '0.0.1',
											'type'			=> 'serialise',
											'default'		=> Array(),
											'description'	=> 'Stores the assetids of linked files',
										);

		$this->vars['files_order']	= Array(
											'added'			=> '0.0.1',
											'type'			=> 'serialise',
											'default'		=> Array(),
											'description'	=> 'Stores the order in which the linked files are combined',
										);

		$this->vars['auto_regen']	= Array(
											'added'			=> '0.0.1',
											'type'			=> 'boolean',
											'default'		=> FALSE,
											'parameters'    => Array(
																'allow_empty' => FALSE
																),
											'description'	=> 'Whether to auto regenerate the merged file on any of the linked file update',
										);

	}//end constructor

}//end class

?>
