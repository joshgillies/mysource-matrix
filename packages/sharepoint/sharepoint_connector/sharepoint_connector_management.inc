<?php
/**
* +- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - +
* | This MySource Matrix CMS file is Copyright (c) Squiz Pty Ltd       |
* | ACN 084 670 600                                                    |
* +- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - +
* | IMPORTANT: Your use of this Software is subject to the terms of    |
* | the Licence provided in the file licence.txt. If you cannot find   |
* | this file please contact Squiz (www.squiz.net) so we may provide   |
* | you a copy.                                                        |
* +- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - +
*
* $Id: sharepoint_connector_management.inc,v 1.2 2010/05/05 07:15:02 ewang Exp $
*
*/


require_once SQ_INCLUDE_PATH.'/asset_management.inc';

/**
* Sharepoint_Connector_Management
*
* @see Asset_Management
*
* @author  Huan Nguyen <hnguyen@squiz.net>
* @version $Revision: 1.2 $
* @package MySource_Matrix_Packages
* @subpackage sharepoint
*/

class Sharepoint_Connector_Management extends Asset_Management
{


    /**
    * Constructor
    *
    */
    function __construct(Package_Manager $pm)
    {
        Asset_Management::__construct($pm);

		$this->vars	= Array (
                        'name'      => Array(
                                        'added'     => '0.1',
                                        'type'      => 'text',
                                        'default'   => '',
                                       ),
						'url'		=> Array (
										'added'			=> '0.1',
										'type'			=> 'text',
										'default'		=> '',
										'description'	=> 'The URL to a site of a Sharepoint system',
										'is_admin'		=> '1',
										'parameters'	=> Array (
															'multiple'		=> FALSE,
															'allow_empty'	=> TRUE,
															   ),
										   ),
						'username'	=> Array (
										'added'			=> '0.1',
										'type'			=> 'text',
										'default'		=> '',
										'description'	=> 'The username used to connect to Sharepoint',
										'is_admin'		=> '1',
										'parameters'	=> Array (
															'multiple'		=> FALSE,
															'allow_empty'	=> TRUE,
															   ),
										   ),
						'password'	=> Array (
										'added'			=> '0.1',
										'type'			=> 'password',
										'default'		=> '',
										'description'	=> 'The password used to connect to Sharepoint',
										'is_admin'		=> '1',
										'parameters'	=> Array (
															'multiple'		=> FALSE,
															'allow_empty'	=> TRUE,
															   ),
										   ),
					  );

	}//end constructor


}//end class
?>
