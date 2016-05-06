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
* Js_File_Folder_Management
*
* @see Asset_Management
*
* @author  Chiranjivi Upreti <cupreti@squiz.com.au>
* @version $Revision: $
* @package MySource_Matrix_Packages
* @subpackage __core__
*/
class Js_File_Folder_Management extends Asset_Management
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
																						'closure'	=> 'Closure Compiler',
																						'closure_with_sm'	=> 'Closure Compiler - Create Source Map',
																					),
												),
											'description'	=> 'Compilation type to minify the linked files when merging',
										);

		$this->vars['source_map_files']		= Array(
											'added'			=> '0.0.1',
											'type'			=> 'serialise',
											'default'		=> Array(),
											'description'	=> 'Filenames of the additional Source Map files',
										);

	}//end constructor


	/**
	* Return an array with information about the events that this File Folder is listening to
	*
	* @return boolean
	* @access public
	*/
	function getEventList()
	{
		return Array(
				Array(
					'event_name'					=> 'ContentsUpdated',
					'broadcast_type_code'			=> 'js_file',
					'broadcast_strict_type_code'	=> TRUE,
					'options'						=> Array(
														'side_of_link'	=> 'minor',
														'indirect'		=> FALSE,
														'is_exclusive'	=> NULL,
														'is_dependant'	=> NULL,
														'value'			=> 'linked_folder',
														'link_type'		=> SQ_LINK_NOTICE,
													),
												),
				);

    }//end getEventList()


}//end class

?>
