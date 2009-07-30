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
* $Id: js_api_management.inc,v 1.2 2009/07/30 22:56:51 bpearson Exp $
*
*/

require_once SQ_INCLUDE_PATH.'/asset_management.inc';

/**
* Javascript API Management
*
* Purpose
*
*
* @author  Nic Hubbard <nic@zedsaid.com>
* @version $Revision: 1.2 $
* @package MySource_Matrix_Packages
* @subpackage web_services
*/
class JS_Api_Management extends Asset_Management
{


    /**
    * Constructor
    *
    */
    function __construct(Package_Manager $pm)
    {
        Asset_Management::__construct($pm);
		
		// Create a random key
		$key = rand(1000000000, 9999999999);

		$this->vars	= Array (
						'name'						=> Array(
														'added'			=> '0.0.1',
														'type'			=> 'text',
														'default'		=> '',
														'description'	=> 'The full name of the page',
														'update_fn'		=> '',
														'is_admin'		=> FALSE,
										   	   			),
						'short_name'				=> Array(
														'added'			=> '0.0.1',
														'type'			=> 'text',
														'default'		=> '',
														'description'	=> 'The short name of the page',
														'update_fn'		=> '',
														'is_admin'		=> FALSE,
										       			),
						'root_node'					=> Array(
														'added'			=> '0.0.1',
														'type'			=> 'assetid',
														'default'		=> '',
														'description'	=> 'The root node to restrict to',
														'update_fn'		=> '',
														'is_admin'		=> FALSE,
											   			),
						'api_key'					=> Array(
														'added'			=> '0.0.1',
														'type'			=> 'text',
														'default'		=> $key,
														'description'	=> 'The API Key',
														'update_fn'		=> '',
														'is_admin'		=> FALSE,
											   			),
						'get_general'				=> Array(
														'added'			=> '0.0.1',
														'type'			=> 'boolean',
														'default'		=> FALSE,
														'parameters'	=> Array(
																	'allow_empty'	=> FALSE,
																   ),
											   			),
						'get_attributes'			=> Array(
														'added'			=> '0.0.1',
														'type'			=> 'boolean',
														'default'		=> FALSE,
														'parameters'	=> Array(
																	'allow_empty'	=> FALSE,
																   ),
											   			),
						'set_attribute'				=> Array(
														'added'			=> '0.0.1',
														'type'			=> 'boolean',
														'default'		=> FALSE,
														'parameters'	=> Array(
																	'allow_empty'	=> FALSE,
																   ),
											   			),
						'get_metadata'				=> Array(
														'added'			=> '0.0.1',
														'type'			=> 'boolean',
														'default'		=> FALSE,
														'parameters'	=> Array(
																	'allow_empty'	=> FALSE,
																   ),
											   			),
						'set_metadata'				=> Array(
														'added'			=> '0.0.1',
														'type'			=> 'boolean',
														'default'		=> FALSE,
														'parameters'	=> Array(
																	'allow_empty'	=> FALSE,
																   ),
											   			),
						'trash_asset'				=> Array(
														'added'			=> '0.0.1',
														'type'			=> 'boolean',
														'default'		=> FALSE,
														'parameters'	=> Array(
																	'allow_empty'	=> FALSE,
																   ),
											   			),
						'get_parents'				=> Array(
														'added'			=> '0.0.2',
														'type'			=> 'boolean',
														'default'		=> FALSE,
														'parameters'	=> Array(
																	'allow_empty'	=> FALSE,
																   ),
											   			),
						'get_children'				=> Array(
														'added'			=> '0.0.2',
														'type'			=> 'boolean',
														'default'		=> FALSE,
														'parameters'	=> Array(
																	'allow_empty'	=> FALSE,
																   ),
											   			),
						'get_permissions'			=> Array(
														'added'			=> '0.0.2',
														'type'			=> 'boolean',
														'default'		=> FALSE,
														'parameters'	=> Array(
																	'allow_empty'	=> FALSE,
																   ),
											   			),
						'create_asset'				=> Array(
														'added'			=> '0.0.2',
														'type'			=> 'boolean',
														'default'		=> FALSE,
														'parameters'	=> Array(
																	'allow_empty'	=> FALSE,
																   ),
											   			),
						'get_asset_types'			=> Array(
														'added'			=> '0.0.2',
														'type'			=> 'boolean',
														'default'		=> FALSE,
														'parameters'	=> Array(
																	'allow_empty'	=> FALSE,
																   ),
											   			),
						'acquire_lock'				=> Array(
														'added'			=> '0.0.2',
														'type'			=> 'boolean',
														'default'		=> FALSE,
														'parameters'	=> Array(
																	'allow_empty'	=> FALSE,
																   ),
											   			),
						'release_lock'				=> Array(
														'added'			=> '0.0.2',
														'type'			=> 'boolean',
														'default'		=> FALSE,
														'parameters'	=> Array(
																	'allow_empty'	=> FALSE,
																   ),
											   			),
						'create_link'				=> Array(
														'added'			=> '0.0.2',
														'type'			=> 'boolean',
														'default'		=> FALSE,
														'parameters'	=> Array(
																	'allow_empty'	=> FALSE,
																   ),
											   			),
						'remove_link'				=> Array(
														'added'			=> '0.0.2',
														'type'			=> 'boolean',
														'default'		=> FALSE,
														'parameters'	=> Array(
																	'allow_empty'	=> FALSE,
																   ),
											   			),
						'move_link'					=> Array(
														'added'			=> '0.0.2',
														'type'			=> 'boolean',
														'default'		=> FALSE,
														'parameters'	=> Array(
																	'allow_empty'	=> FALSE,
																   ),
											   			),
						'update_link'				=> Array(
														'added'			=> '0.0.2',
														'type'			=> 'boolean',
														'default'		=> FALSE,
														'parameters'	=> Array(
																	'allow_empty'	=> FALSE,
																   ),
											   			),
						'get_link_id'				=> Array(
														'added'			=> '0.0.2',
														'type'			=> 'boolean',
														'default'		=> FALSE,
														'parameters'	=> Array(
																	'allow_empty'	=> FALSE,
																   ),
											   			),
						'get_asset_tree'			=> Array(
														'added'			=> '0.0.2',
														'type'			=> 'boolean',
														'default'		=> FALSE,
														'parameters'	=> Array(
																	'allow_empty'	=> FALSE,
																   ),
											   			),
											
					  );

	}//end constructor


}//end class
?>
