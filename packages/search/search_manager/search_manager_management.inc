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
* $Id: search_manager_management.inc,v 1.22 2012/08/30 00:59:15 ewang Exp $
*
*/


require_once SQ_INCLUDE_PATH.'/asset_management.inc';

/**
* Search_Manager_Management
*
* @see Asset_Management
*
* @author  Dominic Wong <dwong@squiz.net>
* @version $Revision: 1.22 $ - 0.1
* @package MySource_Matrix_Packages
* @subpackage search
*/
class Search_Manager_Management extends Asset_Management
{


	/**
	* Constructor
	*/
	function Search_Manager_Management(&$pm)
	{
		$this->Asset_Management($pm);
		$this->vars = Array(
						'indexing'			=> Array(
												'added'			=> '0.1',
												'type'			=> 'boolean',
												'default'		=> TRUE,
												'description'	=> 'Turns indexing on or off',
												'parameters'	=> Array(
																	'allow_empty'	=> FALSE,
																   ),
											   ),
						'min_word_length'	=> Array(
												'added'			=> '0.1',
												'type'			=> 'int',
												'default'		=> '4',
												'parameters'	=> Array(
																	'allow_negative'	=> FALSE,
																   ),
											   ),
						'noise_word_list'	=> Array(
												'added'		=> '0.1',
												'type'		=> 'serialise',
												'default'	=> Array(
																'a',
																'the',
																'it',
																'this',
															   ),
											   ),
						'white_word_list'	=> Array(
												'added'		=> '0.1',
												'type'		=> 'serialise',
												'default'	=> Array(),
											   ),
						'enable_begins_with'	=> Array(
													'added'			=> '0.5',
													'type'			=> 'boolean',
													'default'		=> TRUE,
													'description'	=> 'Disable this option for faster search. Searching for \'room\' will find \'roommate\' but not \'bedroom\'',
													'parameters'	=> Array(
																		'allow_empty'	=> FALSE,
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
					'event_name'					=> 'AssetCreate',
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
