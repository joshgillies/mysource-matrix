<?php
/**
* +- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - +
* | This MySource Matrix CMS file is Copyright (c) Squiz Pty Ltd       |
* | ACN 084 670 600													   |
* +- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - +
* | IMPORTANT: Your use of this Software is subject to the terms of    |
* | the Licence provided in the file licence.txt. If you cannot find   |
* | this file please contact Squiz (www.squiz.com.au) so we may provide|
* | you a copy.                                                        |
* +- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - +
*
* $Id: squiz_suite_system_search_management.inc,v 1.2 2012/08/30 00:59:25 ewang Exp $
*
*/


require_once SQ_INCLUDE_PATH.'/asset_management.inc';

/**
* Squiz_Search_System_Search_Management
*
* @see Asset_Management
*
* @author  Benjamin Pearson <bpearson@squiz.com.au>
* @version $Revision: 1.2 $
* @package MySource_Matrix_Packages
* @subpackage squiz_suite
*/

class Squiz_Suite_System_Search_Management extends Asset_Management
{


    /**
    * Constructor
    *
    */
    function __construct(Package_Manager $pm)
    {
        Asset_Management::__construct($pm);

		$this->vars	= Array (
						'name'			=> Array(
											'added'			=> '0.0.1',
											'type'			=> 'text',
											'default'		=> '',
											'description'	=> 'The full name of the page',
											'update_fn'		=> '',
											'is_admin'		=> FALSE,
											'is_contextable'	=> TRUE,
										   ),
						'short_name'	=> Array(
											'added'			=> '0.0.1',
											'type'			=> 'text',
											'default'		=> '',
											'description'	=> 'The short name of the page',
											'update_fn'		=> '',
											'is_admin'		=> FALSE,
											'is_contextable'	=> TRUE,
										   ),
                     );

	}//end constructor


}//end class

?>
