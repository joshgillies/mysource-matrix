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
* $Id: cron_job_future_lineage_management.inc,v 1.10 2007/07/20 00:22:18 mbrydon Exp $
*
*/


require_once SQ_INCLUDE_PATH.'/asset_management.inc';

/**
* Cron_Job_Future_Lineage_Management
*
* @see Asset_Management
*
* @author  Blair Robertson <blair@squiz.net>
* @version $Revision: 1.10 $
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

		$this->vars = Array(
						'type'				=> Array(
												'default'	=> 'one_off',
											   ),
						'delete_linkid'		=> Array(
												'added'			=> '0.1',
												'type'			=> 'int',
												'default'		=> 0,
												'description'	=> 'If we need to delete an existing link in this process, this is it\'s linkid',
											   ),
						'delete_link_all'	=> Array(
												'added'			=> '0.3',
												'type'			=> 'boolean',
												'default'		=> FALSE,
												'description'	=> 'If we need to delete all existing links in this process',
											   ),
						'link_info'			=> Array(
												'added'			=> '0.1',
												'type'			=> 'serialise',
												'default'		=> Array(),
												'description'	=> 'The link information similar in form to what is passed to Asset::create(), minus the "asset" element',
											   ),
						'asset_type'		=> Array(
												'added'			=> '0.4',
												'type'			=> 'text',
												'default'		=> '',
												'description'	=> '',
											   ),
						'attribute'			=> Array(
												'added'			=> '0.4',
												'type'			=> 'text',
												'default'		=> '',
												'description'	=> '',
											   ),
					  );

	}//end constructor


}//end class
?>
