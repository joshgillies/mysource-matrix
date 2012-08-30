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
* $Id: cron_job_update_trim_record_management.inc,v 1.2 2012/08/30 00:59:28 ewang Exp $
*
*/


require_once SQ_INCLUDE_PATH.'/asset_management.inc';

/**
* Cron_Job_Packaging_Trim_Record_Management
*
* @see Asset_Management
*
* @author  Huan Nguyen <hnguyen@squiz.net>
* @version $Revision: 1.2 $
* @package MySource_Matrix_Packages
* @subpackage __core__
*/
class Cron_Job_Update_Trim_Record_Management extends Asset_Management
{


	/**
	* Constructor
	*
	*/
	function __construct(&$pm)
	{
		$this->Asset_Management($pm);

		$this->vars	= Array(
						'trim_saved_search_id'	=> Array (
													'added'			=> '0.1',
												   	'type'			=> 'text',
												   	'default'		=> '',
												   	'description'	=> 'The id of the data source we are using to connect'
												   ),
						'type'					=> Array(
													'default'   => 'repeating',
		                                 		   ),
						'when'					=> Array(
		                                    		'default'   => 'DL=00:00',
												   ),
					  );

	}//end constructor


}//end class
?>
