<?php
/**
* +--------------------------------------------------------------------+
* | Squiz.net Open Source Licence                                      |
* +--------------------------------------------------------------------+
* | Copyright (c), 2003 Squiz Pty Ltd (ABN 77 084 670 600).            |
* +--------------------------------------------------------------------+
* | This source file may be used subject to, and only in accordance    |
* | with, the Squiz Open Source Licence Agreement found at             |
* | http://www.squiz.net/licence.                                      |
* | Make sure you have read and accept the terms of that licence,      |
* | including its limitations of liability and disclaimers, before     |
* | using this software in any way. Your use of this software is       |
* | deemed to constitute agreement to be bound by that licence. If you |
* | modify, adapt or enhance this software, you agree to assign your   |
* | intellectual property rights in the modification, adaptation and   |
* | enhancement to Squiz Pty Ltd for use and distribution under that   |
* | licence.                                                           |
* +--------------------------------------------------------------------+
*
* $Id: metadata_field_management.inc,v 1.17 2006/01/26 22:34:04 lwright Exp $
*
*/


require_once SQ_INCLUDE_PATH.'/asset_management.inc';

/**
* Metadata_Field_Management
*
* @see Asset_Management
*
* @author  Greg Sherwood <greg@squiz.net>
* @version $Revision: 1.17 $
* @package MySource_Matrix_Packages
* @subpackage __core__
*/
class Metadata_Field_Management extends Asset_Management
{


	/**
	* Constructor
	*
	*/
	function Metadata_Field_Management(&$pm)
	{
		$this->Asset_Management($pm);

		$this->vars = Array(
						'name'				=> Array(
												'added'	=> '0.1',
												'type'	=> 'text', 'default'	=> '',
											   ),
						'friendly_name'		=> Array(
												'added'	=> '0.1',
												'type'	=> 'text', 'default'	=> '',
											   ),
						'description'		=> Array(
												'added'	=> '0.1',
												'type'	=> 'text', 'default'	=> '',
											   ),
						'default'			=> Array(
												'added'	=> '0.1',
												'type'	=> 'text', 'default'	=> '',
											   ),
						'scheme'			=> Array(
												'added'	=> '0.1',
												'type'	=> 'text', 'default'	=> '',
											   ),
						'language'			=> Array(
												'added'	=> '0.1',
												'type'	=> 'text', 'default'	=> '',
											   ),
						'value_components'	=> Array(
												'added'	=> '0.0.1',
												'type'	=> 'serialise', 'default'	=> Array(),
											   ),
						'editable'			=> Array(
												'added'			=> '0.1',
												'type'			=> 'boolean',
												'default'		=> TRUE,
												'parameters'	=> Array('allow_empty'	=> FALSE),
											   ),
						'required'			=> Array(
												'added'			=> '0.1',
												'type'			=> 'boolean',
												'default'		=> FALSE,
												'parameters'	=> Array('allow_empty' => FALSE),
												'description'	=> 'If this field is Required, then either the value or any one of the value components must be set for an asset to be approved',
											   ),
						'frontend'			=> Array(
												'added'			=> '0.1',
												'type'			=> 'selection',
												'default'		=> 'show',
												'parameters'	=> Array(
																	'options'		=> Array(
																						'show'	=> 'Always',
																						'fill'	=> 'Only If Filled',
																						'hide'	=> 'Never',
																					   ),
																	'multiple'		=> FALSE,
																	'allow_empty'	=> FALSE,
																   ),
											   ),
					  );

	}//end constructor


	/**
	* Gets a list of all metadata files that we want web accessable
	*
	* @return array
	* @access private
	*/
	function _getFileList()
	{
		return Array(
				'popups/keywords.html',
				'popups/keyword_extraction.php',
			   );

	}//end _getFileList()


}//end class

?>