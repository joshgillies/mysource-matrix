<?php
/**
* +--------------------------------------------------------------------+
* | MySource 3 - MySource Matrix                                       |
* +--------------------------------------------------------------------+
* | Copyright (c), 2003 Squiz Pty Ltd (ABN 77 084 670 600).            |
* +--------------------------------------------------------------------+
* | This source file may be used subject to, and only in accordance    |
* | with, the Squiz Open Source Licence Agreement found at             |
* | http://www.squiz.net/licence.                                      |
* | Make sure you have read and accept the terms of that licence,      |
* | including its limitations of liability and disclaimers, before     |
* | using this software in any way. Your use of this software is       |
* | deemed to constitute agreement to be bound by that licence. If you |
* | modify, adapt or enhance this software, you agree to assign your   |
* | intellectual property rights in the modification, adaptation and   |
* | enhancement to Squiz Pty Ltd for use and distribution under that   |
* | licence.                                                           |
* +--------------------------------------------------------------------+
*
* $Id: workspace_management.inc,v 1.7 2003/11/21 22:34:40 mmcintyre Exp $
* $Name: not supported by cvs2svn $
*/


require_once SQ_CORE_PACKAGE_PATH.'/folder/folder_management.inc';

/**
* Workspace_Management
*
* @see Asset_Management
*
* @author  Greg Sherwood <greg@squiz.net>
* @version $Version$ - 1.0
* @package MySource_Matrix_Packages
* @subpackage __core__
*/
class Workspace_Management extends Folder_Management
{

	/**
	* Constructor
	*
	*/
	function Workspace_Management(&$pm)
	{
		$this->Folder_Management($pm);

	}//end constructor


	/**
	* returns an array with information about the events that the workspace is listening for
	*
	* @access public
	* @return boolean
	*/
	function getEventList()
	{
		return Array(
				Array(
					'event_name'				 => 'usernameUpdated',
					'broadcast_type_code'		 => 'user',
					'broadcast_strict_type_code'	=> false,	// user, root_user, backend_user, system_user
					'options'	=> Array(
						'side_of_link'					=> 'major',	// side of link where the broadcaster exists
						'indirect'						=> false,
						'exclusive'						=> null,
						'dependant'						=> 1,
						'value'							=> '',
						'link_type'						=> SQ_LINK_TYPE_2 | SQ_LINK_TYPE_1,
					),
				),
		);
	}//end getEventList()

}//end class
?>
