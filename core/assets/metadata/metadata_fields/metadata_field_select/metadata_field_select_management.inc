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
* $Id: metadata_field_select_management.inc,v 1.16 2008/06/20 06:21:12 bpearson Exp $
*
*/


require_once SQ_INCLUDE_PATH.'/asset_management.inc';

/**
* Metadata_Field_Text_Management
*
* @see Asset_Management
*
* @author  Dmitri Iarandine <diarandine@squiz.net>
* @version $Revision: 1.16 $
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
