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
* $Id: workflow_step_condition_management.inc,v 1.10 2004/09/15 04:30:03 gsherwood Exp $
* $Name: not supported by cvs2svn $
*/


require_once SQ_INCLUDE_PATH.'/asset_management.inc';

/**
* Workflow_Step_Condition_Management
*
* @see Asset_Management
*
* @author  Greg Sherwood <greg@squiz.net>
* @version $Version$ - 1.0
* @package MySource_Matrix_Packages
* @subpackage __core__
*/
class Workflow_Step_Condition_Management extends Asset_Management
{


	/**
	* Constructor
	*
	*/
	function Workflow_Step_Condition_Management(&$pm)
	{
		$this->Asset_Management($pm);

		$this->vars = Array(
							'userid'	=>  Array (
												'added'   => '0.1',
												'type'    => 'assetid',
												'default' => 0,
												),
							'logic'		=>  Array (
												'added'   => '0.1',
												'type'    => 'text',
												'default' => '>=',
												),
							'logic_num'	=> Array (
												'added'   => '0.1',
												'type'    => 'int',
												'default' => 1,
												),
							'notify'	=> Array (
												'added'   => '0.2',
												'type'    => 'boolean',
												'default' => false,
												'parameters' => Array(
													'allow_empty' => false,
													)
												),
							);

	}//end constructor


}//end class

?>