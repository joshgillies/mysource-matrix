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
* $Id: metadata_field_date_management.inc,v 1.14 2013/10/18 04:30:22 akarelia Exp $
*
*/


require_once SQ_INCLUDE_PATH.'/asset_management.inc';

/**
* Metadata_Field_Text_Management
*
* @see Asset_Management
*
* @author  Dmitri Iarandine <diarandine@squiz.net>
* @version $Revision: 1.14 $
* @package MySource_Matrix_Packages
* @subpackage __core__
*/
class Metadata_Field_Date_Management extends Asset_Management
{


	/**
	* Constructor
	*
	*/
	function __construct(Package_Manager $pm)
	{
		Asset_Management::__construct($pm);

		$this->vars = Array(
						'offset'			=> Array(
												'added'				=> '0.1',
												'type'				=> 'text',
												'default'			=> ' ',
											   ),
						'format'			=> Array(
												'added'				=> '0.1',
												'type'				=> 'text',
												'default'			=> 'Y-m-d H:i:s',
											   ),
						'setup_params'		=> Array(
												'added'				=> '0.0.1',
												'type'				=> 'serialise',
												'default'			=> Array(),
											   ),
						'show_keywords'		=> Array(
												'added'				=> '0.1',
												'type'				=> 'boolean',
												'default'			=> TRUE,
												'parameters'		=> Array('allow_empty' => FALSE),
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


	/**
	* Gets a list of all metadata files that we want web accessable
	*
	* @return array
	* @access private
	*/
	public function _getFileList()
	{
		return Array(
				'popups/keywords.html',
				'js/metadata_field_date.js',
			   );

	}//end _getFileList()


}//end class

?>
