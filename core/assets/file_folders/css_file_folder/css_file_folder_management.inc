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
class Css_File_Folder_Management extends Asset_Management
{


	/**
	* Constructor
	*
	*/
	function __construct(Package_Manager $pm)
	{
		Asset_Management::__construct($pm);

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
					'broadcast_type_code'			=> 'css_file',
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
				Array(
					'event_name'					=> 'ContentsUpdated',
					'broadcast_type_code'			=> 'design_css',
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
