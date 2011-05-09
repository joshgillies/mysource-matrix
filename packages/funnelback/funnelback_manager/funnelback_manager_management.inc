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
* $Id: funnelback_manager_management.inc,v 1.3 2011/05/09 03:38:08 akarelia Exp $
*
*/

require_once SQ_INCLUDE_PATH.'/asset_management.inc';

/**
* Funnelback_Manager_Management
*
* @see Asset_Management
*
* @author  Benjamin Pearson <bpearson@squiz.net>
* @version $Revision: 1.3 $ - 0.1
* @package MySource_Matrix_Packages
* @subpackage funnelback
*/
class Funnelback_Manager_Management extends Asset_Management
{


	/**
	* Constructor
	*/
	function Funnelback_Manager_Management(&$pm)
	{
		$this->Asset_Management($pm);
		$this->vars = Array(
						'indexing'			=> Array(
												'added'			=> '0.1',
												'type'			=> 'boolean',
												'default'		=> FALSE,
												'description'	=> 'Should funnelback be indexing?',
												'parameters'	=> Array(
																	'allow_empty'	=> FALSE,
																   ),
											   ),
						'collections'		=> Array(
												'added'			=> '0.1',
												'type'			=> 'serialise',
												'default'		=> Array(),
												'description'	=> 'List of Funnelback Collections',
												'parameters'	=> Array(
																	'allow_empty'	=> TRUE,
																   ),
											   ),
						'feature_pages'		=> Array(
												'added'			=> '0.1',
												'type'			=> 'serialise',
												'default'		=> Array(),
												'description'	=> 'List of Funnelback Feature Pages',
												'parameters'	=> Array(
																	'allow_empty'	=> TRUE,
																   ),
											   ),
						'politeness'		=> Array(
												'added'			=> '0.1',
												'type'			=> 'int',
												'default'		=> 0,
												'description'	=> 'When indexing, set this delay between URL documents (in millisecs)',
												'parameters'	=> Array(
																	'allow_empty'	=> TRUE,
																   ),
											   ),
						'orgname'			=> Array(
												'added'			=> '0.1',
												'type'			=> 'text',
												'default'		=> '',
												'description'	=> 'When indexing, use this as the current Organisation name',
												'parameters'	=> Array(
																	'allow_empty'	=> TRUE,
																   ),
											   ),
						'ibd'				=> Array(
												'added'			=> '0.1',
												'type'			=> 'boolean',
												'default'		=> FALSE,
												'description'	=> 'When indexing, index all documents even if they are binary',
												'parameters'	=> Array(
																	'allow_empty'	=> FALSE,
																   ),
											   ),
						'ixcom'				=> Array(
												'added'			=> '0.1',
												'type'			=> 'boolean',
												'default'		=> FALSE,
												'description'	=> 'When indexing, index words in HTML or XML comments',
												'parameters'	=> Array(
																	'allow_empty'	=> FALSE,
																   ),
											   ),
						'spelleroff'		=> Array(
												'added'			=> '0.1',
												'type'			=> 'boolean',
												'default'		=> FALSE,
												'description'	=> 'When indexing, do not run spelling index',
												'parameters'	=> Array(
																	'allow_empty'	=> FALSE,
																   ),
											   ),
						'spelling_threshold'=> Array(
												'added'			=> '0.1',
												'type'			=> 'int',
												'default'		=> 2,
												'description'	=> 'When indexing, number of occurances before spelling is used',
												'parameters'	=> Array(
																	'allow_empty'	=> TRUE,
																   ),
											   ),
						'keep_log_period'	=> Array(
												'added'			=> '0.1',
												'type'			=> 'int',
												'default'		=> 6,
												'description'	=> 'How long to keep archived logs',
												'parameters'	=> Array(
																	'allow_empty'	=> TRUE,
																   ),
											   ),
						'user'				=> Array(
												'added'		=> '0.1',
												'type'		=> 'assetid',
												'default'	=> '',
											   ),
						'password'			=> Array(
												'added'		=> '0.1',
												'type'		=> 'password',
												'default'	=> '',
											   ),
						'metamap'			=> Array(
												'added'			=> '0.1',
												'type'			=> 'serialise',
												'default'		=> Array(),
												'description'	=> 'List of Funnelback Metadata Mapping',
												'parameters'	=> Array(
																	'allow_empty'	=> TRUE,
																   ),
											   ),
						'global_exclude'	=> Array(
												'added'			=> '0.1',
												'type'			=> 'serialise',
												'default'		=> Array(),
												'description'	=> 'List of global exclusions from funnelback indexing',
												'parameters'	=> Array(
																	'allow_empty'	=> TRUE,
																   ),
											   ),
					  );

	}//end constructor


	/**
	* returns an array with information about the events that the form is listening for
	*
	* @return boolean
	* @access public
	*/
	function getEventList()
	{
		return Array(
				Array(
					'event_name'					=> 'assetDeleted',
					'broadcast_type_code'			=> 'global',
					'broadcast_strict_type_code'	=> TRUE,
					'options'						=> Array(
														'global'	=> TRUE,
													   ),
				),
				Array(
					'event_name'					=> 'AssetUpdate',
					'broadcast_type_code'			=> 'global',
					'broadcast_strict_type_code'	=> TRUE,
					'options'						=> Array(
														'global'	=> TRUE,
													   ),
				),
				Array(
					'event_name'					=> 'ContentsUpdated',
					'broadcast_type_code'			=> 'global',
					'broadcast_strict_type_code'	=> TRUE,
					'options'						=> Array(
														'global'	=> TRUE,
													   ),
				),
				Array(
					'event_name'					=> 'AssetStatusUpdate',
					'broadcast_type_code'			=> 'global',
					'broadcast_strict_type_code'	=> TRUE,
					'options'						=> Array(
														'global'	=> TRUE,
													   ),
				),
				Array(
					'event_name'					=> 'attributeChange',
					'broadcast_type_code'			=> 'global',
					'broadcast_strict_type_code'	=> TRUE,
					'options'						=> Array(
														'global'	=> TRUE,
													   ),
				),
				Array(
					'event_name'					=> 'PermissionChange',
					'broadcast_type_code'			=> 'global',
					'broadcast_strict_type_code'	=> TRUE,
					'options'						=> Array(
														'global'	=> TRUE,
													   ),
				),
				Array(
					'event_name'					=> 'LookupsUpdated',
					'broadcast_type_code'			=> 'global',
					'broadcast_strict_type_code'	=> TRUE,
					'options'						=> Array(
														'global'	=> TRUE,
													   ),
				),
				Array(
					'event_name'					=> 'MetadataUpdate',
					'broadcast_type_code'			=> 'global',
					'broadcast_strict_type_code'	=> TRUE,
					'options'						=> Array(
														'global'	=> TRUE,
													   ),
				),
				Array(
					'event_name'					=> 'MetadataDeleted',
					'broadcast_type_code'			=> 'global',
					'broadcast_strict_type_code'	=> TRUE,
					'options'						=> Array(
														'global'	=> TRUE,
													   ),
				),
			   );

	}//end getEventList()


}//end class
?>
