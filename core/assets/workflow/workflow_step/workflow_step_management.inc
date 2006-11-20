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
* $Id: workflow_step_management.inc,v 1.12 2006/11/20 05:20:52 emcdonald Exp $
*
*/


require_once SQ_INCLUDE_PATH.'/asset_management.inc';

/**
* Workflow_Step_Management
*
* @see Asset_Management
*
* @author  Greg Sherwood <greg@squiz.net>
* @version $Revision: 1.12 $
* @package MySource_Matrix_Packages
* @subpackage __core__
*/
class Workflow_Step_Management extends Asset_Management
{


	/**
	* Constructor
	*
	*/
	function Workflow_Step_Management(&$pm)
	{
		$this->Asset_Management($pm);

		$this->vars = Array(
						'name'									=> Array(
																	'default'	=> '',
																   ),
						'logic'									=> Array(
																	'added'		=> '0.1',
																	'type'		=> 'text',
																	'default'	=> 'all',
																   ),
						'logic_num'								=> Array(
																	'added'		=> '0.1',
																	'type'		=> 'int',
																	'default'	=> '0',
																   ),
						'expiry_time'							=> Array(
																	'added'		=> '0.1',
																	'type'		=> 'duration',
																	'default'	=> '0',
																   ),
						'escalate'								=> Array(
																	'added'			=> '0.3',
																	'type'			=> 'boolean',
																	'default'		=> 'false',
																	'parameters'	=> Array(
																						'allow_empty'	=> FALSE,
																					   ),
																   ),
						'require_comment_from_unlisted_users'	=> Array(
																	'added'			=> '0.2',
																	'type'			=> 'boolean',
																	'default'		=> '0',
																	'parameters'	=> Array(
																						'allow_empty'	=> FALSE,
																					   ),
																   ),
					  );

	}//end constructor


}//end class

?>