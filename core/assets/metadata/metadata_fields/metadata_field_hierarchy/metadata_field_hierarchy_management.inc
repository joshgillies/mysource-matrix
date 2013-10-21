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
* $Id: metadata_field_hierarchy_management.inc,v 1.6.10.1 2013/10/21 03:41:55 akarelia Exp $
*
*/


require_once SQ_INCLUDE_PATH.'/asset_management.inc';

/**
* Metadata_Field_Hierarchy_Management
*
* @see Asset_Management
*
* @author  Benjamin Pearson <bpearson@squiz.net>
* @version $Revision: 1.6.10.1 $
* @package MySource_Matrix_Packages
* @subpackage __core__
*/
class Metadata_Field_Hierarchy_Management extends Asset_Management
{


	/**
	* Constructor
	*
	*/
	function __construct(Package_Manager $pm)
	{
		Asset_Management::__construct($pm);

		// additional attrs., that were not defined in parent
		$this->vars = Array(
						'hierarchy_options'	=> Array(
												'added'		=> '0.1',
												'type'		=> 'serialise',
												'default'	=> Array(),
												'is_contextable'	=> TRUE,
											   ),
						'multiple'			=> Array(
												'added'			=> '0.1',
												'type'			=> 'boolean',
												'default'		=> FALSE,
												'parameters'	=> Array('allow_empty' => FALSE,),
											   ),
						'edit_params'		=> Array(
												'added'		=> '0.1',
												'type'		=> 'serialise',
												'default'	=> Array(),
											   ),
						'allow_empty'		=> Array(
												'added'			=> '0.1',
												'type'			=> 'boolean',
												'default'		=> TRUE,
												'parameters'	=> Array('allow_empty' => FALSE,),
											   ),
						'hier_sort'			=> Array(
												'added'			=> '0.1',
												'type'			=> 'selection',
												'default'		=> '0',
												'parameters'	=> Array(
																	'options'		=> Array(
																						'0'	=> 'Not Sorted',
																						'1'	=> 'Sort by Key',
																						'2'	=> 'Sort by Value',
																					   ),
																	'multiple'		=> FALSE,
																	'allow_empty'	=> FALSE,
																   ),
											   ),
						'hier_level'		=> Array(
												'added'		=> '0.1',
												'type'		=> 'int',
												'default'	=> '1',
											   ),
						'hier_depth'		=> Array(
												'added'		=> '0.1',
												'type'		=> 'int',
												'default'	=> '1',
											   ),
						'hier_structure'	=> Array(
												'added'			=> '0.1',
												'type'			=> 'selection',
												'default'		=> '0',
												'parameters'	=> Array(
																	'options'		=> Array(
																						'0'	=> 'No',
																						'1'	=> 'Yes',
																					   ),
																	'multiple'		=> FALSE,
																	'allow_empty'	=> FALSE,
																   ),
											   ),
						'display_type'	=> Array(
												'added'			=> '0.1',
												'type'			=> 'selection',
												'default'		=> '0',
												'parameters'	=> Array(
																	'options'		=> Array(
																						'0'	=> 'Flat View',
																						'1'	=> 'Drill-Down View',
																					   ),
																	'multiple'		=> FALSE,
																	'allow_empty'	=> FALSE,
																   ),
											   ),
						'level_titles'	=> Array(
												'added'		=> '0.1',
												'type'		=> 'serialise',
												'default'	=> Array(),
												'is_contextable'	=> TRUE,
											   ),
						'visible_part'		=> Array(
												'added'			=> '0.1',
												'type'			=> 'selection',
												'default'		=> 'key',
												'parameters'	=> Array(
																	'options'		=> Array(
																						'key'	=> 'Key',
																						'value'	=> 'Value',
																					   ),
																	'multiple'		=> FALSE,
																	'allow_empty'	=> FALSE,
																   ),
												'description'	=> 'This will determine which part of the options will be printed on the frontend (Key by default).',
											   ),
						'field_name_text'	=> Array(
												'added'			=> '0.1',
												'type'			=> 'text',
												'default'		=> 'name',
												'is_contextable'=> TRUE,
											   ),
						'value_name_text'	=> Array(
												'added'			=> '0.1',
												'type'			=> 'text',
												'default'		=> 'content',
												'is_contextable'=> TRUE,
											   ),
						  );

	}//end constructor


	/**
	* Returns all files this asset wants installed/updated into the data directory
	*
	* @return array
	* @access private
	*/
	public function _getFileList()
	{
		return Array('js/metadata_field_hierarchy.js');

	}//end _getFileList()


}//end class

?>
