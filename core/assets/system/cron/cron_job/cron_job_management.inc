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
* $Id: cron_job_management.inc,v 1.5 2003/11/26 00:50:59 gsherwood Exp $
* $Name: not supported by cvs2svn $
*/


require_once SQ_INCLUDE_PATH.'/asset_management.inc';

/**
* Cron_Job_Management
*
* @see Asset_Management
*
* @author  Blair Robertson <blair@squiz.net>
* @version $Version$ - 1.0
* @package MySource_Matrix_Packages
* @subpackage __core__
*/
class Cron_Job_Management extends Asset_Management
{

	/**
	* Constructor
	*
	*/
	function Cron_Job_Management(&$pm)
	{
		$this->Asset_Management($pm);

		$this->vars =	Array(
							'type' => Array(
								'added'			=> '0.0.1',
								'type'			=> 'selection',
								'default'		=> 'one_off',
								'parameters'	=> Array(
									'options' => Array(
										'one_off'	=> 'One Off', 
										'repeating'	=> 'Repeating'
									),
								),
							),
							'when' => Array(
								'added'			=> '0.1',
								'type'			=> 'text',
								'default'		=> '',
								'description'	=> 'This is an encoded description of when this job is to run',
							),
						);

	}//end constructor

}//end class
?>
