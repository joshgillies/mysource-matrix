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
* $Id: metadata_field_management.inc,v 1.28.2.1 2013/10/21 03:41:55 akarelia Exp $
*
*/


require_once SQ_INCLUDE_PATH.'/asset_management.inc';

/**
* Metadata_Field_Management
*
* @see Asset_Management
*
* @author  Greg Sherwood <greg@squiz.net>
* @version $Revision: 1.28.2.1 $
* @package MySource_Matrix_Packages
* @subpackage __core__
*/
class Metadata_Field_Management extends Asset_Management
{


	/**
	* Constructor
	*
	*/
	function __construct(Package_Manager $pm)
	{
		Asset_Management::__construct($pm);

		$this->vars = Array(
						'name'				=> Array(
												'added'				=> '0.1',
												'type'				=> 'text',
												'default'			=> '',
											   ),
						'friendly_name'		=> Array(
												'added'				=> '0.1',
												'type'				=> 'text',
												 'default'			=> '',
												'is_contextable'	=> TRUE,
												'is_admin'			=> '0',
											   ),
						'description'		=> Array(
												'added'				=> '0.1',
												'type'				=> 'text',
												'default'			=> '',
												'is_contextable'	=> TRUE,
											   ),
						'default'			=> Array(
												'added'				=> '0.1',
												'type'				=> 'text',
												'default'			=> '',
												'is_contextable'	=> TRUE,
											   ),
						'scheme'			=> Array(
												'added'				=> '0.1',
												'type'				=> 'text',
												'default'			=> '',
											   ),
						'language'			=> Array(
												'added'				=> '0.1',
												'type'				=> 'text',
												'default'			=> '',
												'is_contextable'	=> TRUE,
											   ),
						'value_components'	=> Array(
												'added'				=> '0.0.1',
												'type'				=> 'serialise',
												'default'			=> Array(),
												'is_contextable'	=> TRUE,
											   ),
						'editable'			=> Array(
												'added'				=> '0.1',
												'type'				=> 'boolean',
												'default'			=> TRUE,
												'parameters'		=> Array('allow_empty'	=> FALSE),
											   ),
						'is_contextable'	=> Array(
												'added'				=> '0.3',
												'type'				=> 'boolean',
												'default'			=> TRUE,
												'parameters'		=> Array('allow_empty'	=> FALSE),
												'description'		=> 'If set to "Yes", this metadata field will be allowed to take different values in different contexts. The default value can also change between contexts.',
											   ),
						'required'			=> Array(
												'added'				=> '0.1',
												'type'				=> 'boolean',
												'default'			=> FALSE,
												'parameters'		=> Array('allow_empty' => FALSE),
												'description'		=> 'If this field is Required, then either the value or any one of the value components must be set for an asset to be approved',
											   ),
						'frontend'			=> Array(
												'added'				=> '0.1',
												'type'				=> 'selection',
												'default'			=> 'show',
												'parameters'		=> Array(
																		'options'		=> Array(
																							'show'	=> 'Always',
																							'fill'	=> 'Only If Filled',
																							'hide'	=> 'Never',
																						   ),
																		'multiple'		=> FALSE,
																		'allow_empty'	=> FALSE,
																	   ),
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
	* Gets a list of all metadata files that we want web accessable
	*
	* @return array
	* @access private
	*/
	public function _getFileList()
	{
		return Array();

	}//end _getFileList()


}//end class

?>
