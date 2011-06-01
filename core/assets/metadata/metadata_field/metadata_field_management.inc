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
* $Id: metadata_field_management.inc,v 1.22 2007/12/06 23:06:23 lwright Exp $
*
*/


require_once SQ_INCLUDE_PATH.'/asset_management.inc';

/**
* Metadata_Field_Management
*
* @see Asset_Management
*
* @author  Greg Sherwood <greg@squiz.net>
* @version $Revision: 1.22 $
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
	public function _getFileList()
	{
		return Array(
				'popups/keywords.php',
				'popups/keyword_extraction.php',
			   );

	}//end _getFileList()


}//end class

?>
