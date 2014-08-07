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
* $Id: cron_job_future_permission_management.inc,v 1.3 2012/08/30 01:09:11 ewang Exp $
*
*/


require_once SQ_INCLUDE_PATH.'/asset_management.inc';

/**
* Cron_Job_Future_Permission_Management
*
* @see Asset_Management
*
* @author  Anh Ta <ata@squiz.co.uk>
* @version $Revision: 1.3 $
* @package MySource_Matrix_Packages
* @subpackage __core__
*/
class Cron_Job_Future_Permission_Management extends Asset_Management
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
						'type'				=> Array(
												'default'	=> 'one_off',
											   ),
						'add'				=> Array(
												'added'			=> '0.1',
												'type'			=> 'boolean',
												'default'		=> TRUE,
												'parameters'	=> Array(
																	'allow_empty'	=> FALSE,
																   ),
												'description'	=> 'The flag to indicate adding or removing the permission',
											   ),
						'permission'		=> Array(
												'added'			=> '0.1',
												'type'			=> 'int',
												'default'		=> SQ_PERMISSION_READ,
												'description'	=> 'The permission to change the asset to',
											   ),
						'userids'			=> Array(
												'added'			=> '0.1',
												'type'			=> 'serialise',
												'default'		=> Array(),
												'description'	=> 'The userids and grants of permissions',
											   ),
						'cascade_to_new'	=> Array(
												'added'			=> '0.1',
												'type'			=> 'boolean',
												'default'		=> FALSE,
												'parameters'	=> Array(
																	'allow_empty'	=> FALSE,
																   ),
												'description'	=> 'New children should inherit permissions from this asset or not',
											   ),
						'dependants_only'	=> Array(
												'added'			=> '0.1',
												'type'			=> 'boolean',
												'default'		=> TRUE,
												'parameters'	=> Array(
																	'allow_empty'	=> FALSE,
																   ),
												'description'	=> 'If the permission change should cascade to all children',
											   ),
					  );

	}//end constructor


}//end class
?>
