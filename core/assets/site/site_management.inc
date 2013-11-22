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
* $Id: site_management.inc,v 1.17 2012/08/30 01:09:09 ewang Exp $
*
*/


require_once SQ_INCLUDE_PATH.'/asset_management.inc';

/**
* Site_Management
*
* @see Asset_Management
*
* @author  Blair Robertson <blair@squiz.net>
* @version $Revision: 1.17 $
* @package MySource_Matrix_Packages
* @subpackage __core__
*/
class Site_Management extends Asset_Management
{


	/**
	* Constructor
	*
	*/
	function __construct(Package_Manager $pm)
	{
		Asset_Management::__construct($pm);
		$this->vars = Array(
						'not_found_page_cache_globally'	=> Array(
												'added'			=> '0.0.1',
												'type'			=> 'boolean',
												'default'		=> FALSE,
												'parameters'	=> Array(
																	'allow_empty'	=> FALSE,
																   ),
												'description'	=> 'Global cache 404 page',
										 ),
						'available_conditions'		=>  Array(					'added'		=> '0.1',
												'type'		=> 'serialise',
												'default'	=> Array(),
										),
				);

	}//end constructor


}//end class

?>
