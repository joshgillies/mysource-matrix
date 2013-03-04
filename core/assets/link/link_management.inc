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
* $Id: link_management.inc,v 1.6 2013/03/04 01:35:47 ewang Exp $
*
*/


require_once SQ_INCLUDE_PATH.'/asset_management.inc';

/**
* Link_Management
*
* @see Asset_Management
*
* @author  Anh Ta <ata@squiz.co.uk>
* @version $Revision: 1.6 $
* @package MySource_Matrix_Packages
* @subpackage __core__
*/
class Link_Management extends Asset_Management
{


	/**
	* Constructor
	*
	* @param Package_Manager	$pm	The package manager relevant to this particular asset
	*
	*/
	function __construct(Package_Manager $pm)
	{
		Asset_Management::__construct($pm);

		$this->vars = Array(
						'name'				=> Array(
												'added'			=> '0.1',
												'type'			=> 'text',
												'default'		=> '',
												'is_admin'		=> FALSE,
												'is_contextable'=> TRUE,
											   ),
						'link_url'			=> Array(
												'added'		=> '0.1',
												'type'		=> 'text',
												'default'	=> '',
												'is_admin'	=> FALSE,
											   ),
						'link_asset_id'		=> Array(
												'added'		=> '0.1',
												'type'		=> 'assetid',
												'default'	=> '',
											   ),
						'link_relation'		=> Array(
												'added'		=> '0.1',
												'type'		=> 'text',
												'default'	=> '',
												'is_admin'		=> FALSE,
											   ),
					  );
		
	}//end constructor


}//end class
?>
