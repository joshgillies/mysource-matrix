<?php
/**
* +- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - +
* | This MySource Matrix CMS file is Copyright (c) Squiz Pty Ltd       |
* | ABN 77 084 670 600                                                 |
* +- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - +
* | IMPORTANT: Your use of this Software is subject to the terms of    |
* | the Licence provided in the file licence.txt. If you cannot find   |
* | this file please contact Squiz (www.squiz.com.au) so we may provide|
* | you a copy.                                                        |
* +- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - +
*
* $Id: trim_connector_management.inc,v 1.5 2012/08/30 00:59:28 ewang Exp $
*
*/


require_once SQ_INCLUDE_PATH.'/asset_management.inc';

/**
* Trim_Connector_Management
*
* @see Asset_Management
*
* @author  Huan Nguyen <hnguyen@squiz.net>
* @version $Revision: 1.5 $
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
										'is_contextable'	=> TRUE,
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
						'use_proxy'		=> Array(
													'added'         => '0.1',
													'type'          => 'selection',
													'default'       => 'No',
													'description'	=> 'Whether to use proxy',
													'is_admin'      => FALSE,
													'parameters'    => Array(
																		'allow_empty'   => FALSE,
																		'multiple'      => FALSE,
																		'options'		=> Array (
																							'Yes'	=> 'Yes',
																							'No'	=> 'No',
										   													'Use Matrix Primary Proxy'	=> 'Use Matrix Primary Proxy',
																						   ),
																	   ),
												   ),
						'proxy_host'	=> Array (
										'added'			=> '0.1',
										'type'			=> 'text',
										'default'		=> '',
										'description'	=> 'Proxy Host',
										'is_admin'		=> '1',
										'parameters'	=> Array (
															'multiple'		=> FALSE,
															'allow_empty'	=> TRUE,
															   ),
										   ),
						'proxy_port'	=> Array (
										'added'			=> '0.1',
										'type'			=> 'int',
										'default'		=> '',
										'description'	=> 'Proxy Port',
										'is_admin'		=> '1',
										'parameters'	=> Array (
															'multiple'		=> FALSE,
															'allow_empty'	=> TRUE,
															   ),
										   ),
						'proxy_login'	=> Array (
										'added'			=> '0.1',
										'type'			=> 'text',
										'default'		=> '',
										'description'	=> 'Proxy Login',
										'is_admin'		=> '1',
										'parameters'	=> Array (
															'multiple'		=> FALSE,
															'allow_empty'	=> TRUE,
															   ),
										   ),
						'proxy_password'	=> Array (
										'added'			=> '0.1',
										'type'			=> 'password',
										'default'		=> '',
										'description'	=> 'Proxy Password',
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
