<?php
/**
* +--------------------------------------------------------------------+
* | Squiz.net Open Source Licence                                      |
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
* $Id: workspace_management.inc,v 1.12 2005/01/20 13:07:39 brobertson Exp $
*
*/

require_once SQ_INCLUDE_PATH.'/asset_management.inc';

/**
* Workspace_Management
*
* @see Asset_Management
*
* @author  Greg Sherwood <greg@squiz.net>
* @version $Revision: 1.12 $
* @package MySource_Matrix_Packages
* @subpackage __core__
*/
class Workspace_Management extends Asset_Management
{

	/**
	* Constructor
	*
	*/
	function Workspace_Management(&$pm)
	{
		$this->Asset_Management($pm);

		$this->vars = Array('name' => Array(
											'added'     => '0.1',
											'type'      => 'text',
											'default'   => ''
											),
							);

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
					'event_name'                 => 'usernameUpdated',
					'broadcast_type_code'        => 'user',
					'broadcast_strict_type_code' => false,
					'options'	=> Array(
						'side_of_link' => 'major',
						'indirect'     => false,
						'is_exclusive' => null,
						'is_dependant' => 1,
						'value'        => '',
						'link_type'    => SQ_LINK_TYPE_2 | SQ_LINK_TYPE_1,
					),
				),
		);
	}//end getEventList()

}//end class
?>
