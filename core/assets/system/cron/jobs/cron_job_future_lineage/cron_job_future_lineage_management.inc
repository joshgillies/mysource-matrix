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
* $Id: cron_job_future_lineage_management.inc,v 1.3 2003/11/18 15:37:28 brobertson Exp $
* $Name: not supported by cvs2svn $
*/


require_once SQ_INCLUDE_PATH.'/asset_management.inc';

/**
* Cron_Job_Future_Lineage_Management
*
* @see Asset_Management
*
* @author  Blair Robertson <blair@squiz.net>
* @version $Version$ - 1.0
* @package MySource_Matrix_Packages
* @subpackage __core__
*/
class Cron_Job_Future_Lineage_Management extends Asset_Management
{

	/**
	* Constructor
	*
	*/
	function Cron_Job_Future_Lineage_Management(&$pm)
	{
		$this->Asset_Management($pm);

		$this->vars =	Array(
							'type' => Array(
								'default'		=> 'one_off',
							),
							'delete_linkid' => Array(
								'added'			=> '0.1',
								'type'			=> 'int',
								'default'		=> 0,
								'description'	=> 'If we need to delete an existing link in this process, this is it\'s linkid',
							),
							'link_info' => Array(
								'added'			=> '0.1',
								'type'			=> 'serialise',
								'default'		=> Array(),
								'description'	=> 'The link information similar in form to what is passed to Asset::create(), minus the "asset" element',
							),
						);

	}//end constructor

}//end class
?>
