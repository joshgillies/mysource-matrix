<?php
/**
* +--------------------------------------------------------------------+
* | This MySource Matrix CMS file is Copyright (c) Squiz Pty Ltd       |
* | ACN 084 670 600                                                    |
* +--------------------------------------------------------------------+
* | IMPORTANT: Your use of this Software is subject to the terms of    |
* | the Licence provided in the file licence.txt. If you cannot find   |
* | this file please contact Squiz (www.squiz.net) so we may provide   |
* | you a copy.                                                        |
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
* @version $Revision: 1.11 $
* @package MySource_Matrix_Packages
* @subpackage bulkmail
*/
class Bulkmail_Job_Management extends Asset_Management
{


	/**
	* Constructor
	*
	*/
	function __construct(&$pm)
	{
		parent::__construct($pm);

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
						'content_text_only'			=> Array(
														'added'			=> '0.2',
														'type'			=> 'boolean',
														'default'		=> FALSE,
														'description'	=> 'When TRUE, the bulkmail will be text only.',
														'parameters'	=> Array(
																			'allow_empty'	=> FALSE,
																		   ),
													   ),
						'content_body_filter'		=> Array(
														'added'			=> '0.2',
														'type'			=> 'boolean',
														'default'		=> FALSE,
														'parameters'	=> Array(
																			'allow_empty'	=> FALSE,
																		   ),
													   ),
						'content_body_filter_match'	=> Array(
														'added'			=> '0.2',
														'type'			=> 'boolean',
														'default'		=> TRUE,
														'parameters'	=> Array(
																			'allow_empty'	=> FALSE,
																		   ),
													   ),
						'content_body_filter_text'	=> Array(
														'added'			=> '0.2',
														'type'			=> 'text',
														'default'		=> '',
													   ),
					  );

	}//end constructor


}//end class
?>