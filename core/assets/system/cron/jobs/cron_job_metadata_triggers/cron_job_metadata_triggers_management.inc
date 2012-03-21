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
* $Id: cron_job_metadata_triggers_management.inc,v 1.1.22.1 2012/03/21 02:43:51 ewang Exp $
*
*/


require_once SQ_INCLUDE_PATH.'/asset_management.inc';

/**
* Cron_Job_Metadata_Triggers_Management
*
* @see Asset_Management
*
* @author  Geoffroy Noel <gnoel@squiz.co.uk>
* @version $Revision: 1.1.22.1 $
* @package MySource_Matrix_Packages
* @subpackage __core__
*/
class Cron_Job_Metadata_Triggers_Management extends Asset_Management
{

		/**
	* Constructor
	*
	*/
	function __construct(&$pm)
	{
		$this->Asset_Management($pm);
		$this->vars = Array(
						'type'	=> Array(
									'default'	=> 'repeating',
								   ),
						'when'	=> Array(
									'default'	=> 'ET=',
								   ),
						'ignore_date'	=> Array(
												'added'			=> '0.1',
												'type'			=> 'datetime',
												'default'		=> '2008-01-01 00:00:00',
												'parameters'	=> Array(
																	'allow_null'	=> TRUE,
																   ),
											   ),
						'use_cron_last_run' => Array(
												'added'			=> '0.2',
												'type'			=> 'boolean',
												'default'		=> '0',
												'parameters'	=> Array(
																	'allow_empty' => false,
																   ),
											   ),
					  );

	}//end constructor


}//end class
?>