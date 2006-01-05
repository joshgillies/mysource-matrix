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
* $Id: bulkmail_job_management.inc
*
*/

require_once SQ_INCLUDE_PATH.'/asset_management.inc';

/**
* Bulkmail_Job_Management
*
* @see Asset_Management
*
* @author  Nathan de Vries <ndvries@squiz.net>
* @author  Rayn Ong <rong@squiz.net>
*
* @version $Revision: 1.6 $
* @package MySource_Matrix_Packages
* @subpackage bulkmail
*/
class Bulkmail_Job_Management extends Asset_Management
{


	/**
	* Constructor
	*
	*/
	function Bulkmail_Job_Management(&$pm)
	{
		$this->Asset_Management($pm);

		$this->vars = Array(
						'name'						=> Array(
														'added'		=> '0.1',
														'type'		=> 'text',
														'default'	=> '',
													   ),
						'use_post_office_header'	=> Array(
														'added'			=> '0.1',
														'type'			=> 'boolean',
														'default'		=> TRUE,
														'description'	=> 'Whether to use the post office header details.',
													   ),
					  );

	}//end constructor


}//end class
?>