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
* $Id: metadata_field_select_management.inc,v 1.19 2013/10/18 04:30:22 akarelia Exp $
*
*/


require_once SQ_INCLUDE_PATH.'/asset_management.inc';

/**
* Metadata_Field_Text_Management
*
* @see Asset_Management
*
* @author  Dmitri Iarandine <diarandine@squiz.net>
* @version $Revision: 1.19 $
* @package MySource_Matrix_Packages
* @subpackage __core__
*/
class Metadata_Field_Select_Management extends Asset_Management
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
						'select_options'	=> Array(
												'added'		=> '0.1',
												'type'		=> 'serialise',
												'default'	=> Array(),
												'is_contextable'	=> TRUE,
											   ),
						'multiple'			=> Array(
												'added'			=> '0.2',
												'type'			=> 'boolean',
												'default'		=> FALSE,
												'parameters'	=> Array('allow_empty' => FALSE,),
											   ),
						'edit_params'		=> Array(
												'added'		=> '0.2',
												'type'		=> 'serialise',
												'default'	=> Array(),
											   ),
						'visible_part'		=> Array(
												'added'			=> '0.3',
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
						'allow_empty'		=> Array(
												'added'			=> '0.3',
												'type'			=> 'boolean',
												'default'		=> TRUE,
												'parameters'	=> Array('allow_empty' => FALSE,),
											   ),
						'field_name_text'	=> Array(
												'added'				=> '0.3',
												'type'				=> 'text',
												'default'			=> 'name',
												'is_contextable'	=> TRUE,
											   ),
						'value_name_text'	=> Array(
												'added'				=> '0.3',
												'type'				=> 'text',
												'default'			=> 'content',
												'is_contextable'	=> TRUE,
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
		return Array('js/metadata_field_select.js');

	}//end _getFileList()


}//end class

?>
