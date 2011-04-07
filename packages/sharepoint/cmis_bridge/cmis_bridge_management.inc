<?php
/**
* +--------------------------------------------------------------------+
* | This MySource Matrix CMS file is Copyright (c) Squiz Pty Ltd	   |
* | ACN 084 670 600													   |
* +--------------------------------------------------------------------+
* | IMPORTANT: Your use of this Software is subject to the terms of    |
* | the Licence provided in the file licence.txt. If you cannot find   |
* | this file please contact Squiz (www.squiz.net) so we may provide   |
* | you a copy.														   |
* +--------------------------------------------------------------------+
*
* $Id: cmis_bridge_management.inc,v 1.3 2011/04/07 00:56:03 mhaidar Exp $
*
*/


require_once SQ_INCLUDE_PATH.'/asset_management.inc';

/**
* CMIS_Bridge_Management
*
* @see Asset_Management
*
* @author  Mohamed Haidar <mhaidar@squiz.com.au>
* @version $Revision: 1.3 $
* @package MySource_Matrix_Packages
* @subpackage sharepoint
*/
class CMIS_Bridge_Management extends Asset_Management
{


	/**
	* Constructor
	*
	*/
	function CMIS_Bridge_Management(&$pm)
	{
		$this->Asset_Management($pm);
		$this->vars = Array(
						'name'					=> Array(
													'added'			=> '0.1',
													'type'			=> 'text',
													'default'		=> '',
													'description'	=> 'The full name of the bridge',
													'update_fn'		=> '',
													'is_admin'		=> FALSE,
													'is_contextable' => TRUE,
												   ),
						'repository_name'		=> Array(
													'added'			=> '0.1',
													'type'			=> 'text',
													'default'		=> '',
													'description'	=> 'Root document library name to synchronize with'
												   ),
						'wsdl'					=> Array (
													'added'			=> '0.1',
													'type'			=> 'serialise',
													'default'		=> Array(
																			'ObjectService' => '',
																			'NavigationService' => '',
																			'RepositoryService' => ''
																		)
												   ),
						'metadata_fields'		=> Array (
													'added'			=> '0.1',
													'type'			=> 'serialise',
													'default'		=> Array(
												   							'objectId' 			   => Array ( 
												   														'type_code' => 'metadata_field_text',
												   														'field' 	=> ''
												   													),
												   							'lastModificationDate' => Array ( 
												   														'type_code' => 'metadata_field_date',
												   														'field' 	=> ''
												   													),
												   							'createdBy'			   => Array ( 
												   														'type_code' => 'metadata_field_text',
												   														'field' 	=> ''
												   													),
												   							'creationDate'		   => Array ( 
												   														'type_code' => 'metadata_field_date',
												   														'field' 	=> ''
												   													)
												   						)
												   ),
						'ws_security'			=> Array (
													'added'			=> '0.1',
													'type'			=> 'boolean',
													'default'		=> 'false',
													'parameters'    => Array( 'allow_empty' => FALSE,),
													'description'	=> 'Whether to send WS-Security headers with each SOAP request',
												   ),
                        'username'				=> Array(
		                                            'added'     => '0.1',
		                                            'type'      => 'text',
		                                            'default'   => '',
		                                           ),
                        'password'				=> Array(
		                                        	'added'     => '0.1',
		                                            'type'      => 'password',
		                                            'default'   => '',
		                                           ),
					  );

	}//end constructor

	
}//end class

?>
