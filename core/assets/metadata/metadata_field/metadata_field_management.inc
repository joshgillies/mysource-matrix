<?php
/**
* Copyright (c) 2003 - Squiz Pty Ltd
*
* $Id: metadata_field_management.inc,v 1.5 2003/10/07 06:49:17 dwong Exp $
* $Name: not supported by cvs2svn $
*/


require_once SQ_INCLUDE_PATH.'/asset_management.inc';

/**
* Metadata_Field_Management
*
* @see Asset_Management
*
* @author  Greg Sherwood <greg@squiz.net>
* @version $Version$ - 1.0
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

		$this->vars = Array('name'				=> Array(
													'added' => '0.1',
													'type'  => 'text', 'default'   => ''
													),
							'default'			 => Array(
													'added' => '0.1',
													'type'  => 'text', 'default'   => ''
													),
							'scheme'			=> Array(
													'added' => '0.1',
													'type'  => 'text', 'default'   => ''
													),
							'language'			=> Array(
													'added' => '0.1',
													'type'  => 'text', 'default'   => ''
													),
							'value_components'	=> Array(
													'added' => '0.0.1',
													'type'  => 'serialise', 'default' => Array()
													),
							'multi_line'		=> Array(
													'added' => '0.1',
													'type'  => 'boolean', 'default'   => false
													),
							'editable'			=> Array(
													'added' => '0.1',
													'type'  => 'boolean', 'default'   => true
													),
							'required'			=> Array(
													'added' => '0.1',
													'type'  => 'boolean', 'default'   => false
													),
							'weighting'			=> Array(
													'added' => '0.1',
													'type'	=> 'int', 'default' => '-1'
													),
							);
	}//end constructor


	/**
	* Gets a list of all metadata files that we want web accessable
	*
	* @access private
	* @return array(string)
	*/
	function _getFileList()
	{
		return Array(
					 'popups/keywords.html',
					 );

	}//end _getFileList()

}//end class

?>