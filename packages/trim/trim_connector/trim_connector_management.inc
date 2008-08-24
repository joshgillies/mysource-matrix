<?php
/**
* +- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - +
* | This MySource Matrix Module file is Copyright (c) Squiz Pty Ltd    |
* | ACN 084 670 600                                                    |
* +- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - +
* | IMPORTANT: This Module is not available under an open source       |
* | license and consequently distribution of this and any other files  |
* | that comprise this Module is prohibited. You may only use this     |
* | Module if you have the written consent of Squiz.                   |
* +- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - +
*
* $Id: trim_connector_management.inc,v 1.1.1.1 2008/07/04 04:56:22 hnguyen Exp $
*
*/


require_once SQ_INCLUDE_PATH.'/asset_management.inc';

/**
* Trim_Connector_Management
*
* @see Asset_Management
*
* @author  Huan Nguyen <hnguyen@squiz.net>
* @version $Revision: 1.1.1.1 $
* @package MySource_Matrix_Packages
* @subpackage trim
*/

class Trim_Connector_Management extends Asset_Management
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
						'wsdl'		=> Array (
										'added'			=> '0.1',
										'type'			=> 'text',
										'default'		=> '',
										'description'	=> 'The URL to the WSDL of the TRIM system',
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
										'description'	=> 'The username used to connect to TRIM',
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
										'description'	=> 'The password used to connect to TRIM',
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
