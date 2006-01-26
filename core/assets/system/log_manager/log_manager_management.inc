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
* $Id: log_manager_management.inc,v 1.3 2006/01/26 22:34:06 lwright Exp $
*
*/

require_once SQ_INCLUDE_PATH.'/asset_management.inc';

/**
* Log_Manager_Management
*
* @see Asset_Management
*
* @author	Andrei Railean <arailean@squiz.net>
* @author	Robert Howard <rhoward@squiz.net>
* @version $Revision: 1.3 $
* @package MySource_Matrix_Packages
* @subpackage __core__
*/
class Log_Manager_Management extends Asset_Management
{


	/**
	* Constructor
	*
	*/
	function Log_Manager_Management(&$pm)
	{
		$this->Asset_Management($pm);

		$this->vars = Array(
						'enabled'	=> Array(
										'added'			=> '0.1',
										'type'			=> 'boolean',
										'default'		=> TRUE,
										'parameters'	=> Array(
															'allow_empty'	=> FALSE,
														   ),
										'description'	=> 'A flag which enables or disables logging',
									   ),
						'logs'		=> Array(
										'added'			=> '0.1',
										'type'			=> 'serialise',
										'default'		=> Array(),
										'description'	=> 'Log metadata relating to rotations',
									   ),
					  );


	}//end constructor


}//end class

?>
