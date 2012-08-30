<?php
/**
* +--------------------------------------------------------------------+
* | This MySource Matrix CMS file is Copyright (c) Squiz Pty Ltd       |
* | ABN 77 084 670 600                                                 |
* +--------------------------------------------------------------------+
* | IMPORTANT: Your use of this Software is subject to the terms of    |
* | the Licence provided in the file licence.txt. If you cannot find   |
* | this file please contact Squiz (www.squiz.com.au) so we may provide|
* | you a copy.                                                        |
* +--------------------------------------------------------------------+
*
* $Id: cron_job_management.inc,v 1.16 2012/08/30 01:09:10 ewang Exp $
*
*/


require_once SQ_INCLUDE_PATH.'/asset_management.inc';

/**
* Cron_Job_Management
*
* @see Asset_Management
*
* @author  Blair Robertson <blair@squiz.net>
* @version $Revision: 1.16 $
* @package MySource_Matrix_Packages
* @subpackage __core__
*/
class Cron_Job_Management extends Asset_Management
{


	/**
	* Constructor
	*
	* @param Package_Manager	$pm	The package manager handling this asset
	*
	*/
	function __construct(Package_Manager $pm)
	{
		Asset_Management::__construct($pm);

		$this->vars = Array(
						'type'			=> Array(
											'added'			=> '0.0.1',
											'type'			=> 'selection',
											'default'		=> 'one_off',
											'parameters'	=> Array(
																'options'		=> Array(
																					'one_off'	=> 'One Off',
																					'repeating'	=> 'Repeating',
																				   ),
																'multiple'		=> FALSE,
																'allow_empty'	=> FALSE,
															   ),
										   ),
						'when'			=> Array(
											'added'			=> '0.1',
											'type'			=> 'text',
											'default'		=> '',
											'description'	=> 'This is an encoded description of when this job is to run',
										   ),
						'running_as'	=> Array(
											'added'			=> '0.2',
											'type'			=> 'assetid',
											'default'		=> '',
											'description'	=> 'The user the cron job will run as when executed',
										   ),
						'read_only'		=> Array(
											'added'			=> '0.3',
											'type'			=> 'boolean',
											'default'		=> FALSE,
											'description'	=> 'If true, only the system can delete this cron job',
										   ),
						'running'		=> Array(
											'added'			=> '0.4',
											'type'			=> 'boolean',
											'default'		=> FALSE,
											'description'	=> 'running flag for potential long job',
										   ),
						'long'			=> Array(
											'added'			=> '0.4',
											'type'			=> 'boolean',
											'default'		=> FALSE,
											'description'	=> 'whether this is a potential long job that will deadlock the cm',
											'parameters'	=> Array(
																'allow_empty'	=> FALSE,
															   ),
										   ),
					  );

	}//end constructor


}//end class
?>
