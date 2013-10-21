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
* $Id: metadata_field_wysiwyg_management.inc,v 1.2.10.1 2013/10/21 03:41:55 akarelia Exp $
*
*/


require_once SQ_INCLUDE_PATH.'/asset_management.inc';

/**
* Metadata_Field_WYSIWYG_Management
*
* @see Asset_Management
*
* @author  Basil Shkara <bshkara@squiz.net>
* @version $Revision: 1.2.10.1 $
* @package MySource_Matrix_Packages
* @subpackage __core__
*/
class Metadata_Field_WYSIWYG_Management extends Asset_Management
{


	/**
	* Constructor
	*
	*/
	function __construct(Package_Manager $pm)
	{
		Asset_Management::__construct($pm);

		$this->vars = Array(
						'wysiwyg'			=> Array(
												'added'		=> '0.1',
												'type'		=> 'wysiwyg',
												'default'	=> '',
											   ),
						'default_html'		=> Array(
												'added'		=> '0.1',
												'type'		=> 'wysiwyg',
												'default'	=> '',
											   ),
						'field_name_text'	=> Array(
												'added'				=> '0.1',
												'type'				=> 'text',
												'default'			=> 'name',
												'is_contextable'	=> TRUE,
											   ),
						'value_name_text'	=> Array(
												'added'				=> '0.1',
												'type'				=> 'text',
												'default'			=> 'content',
												'is_contextable'	=> TRUE,
											   ),
					  );

	}//end constructor


}//end class

?>
