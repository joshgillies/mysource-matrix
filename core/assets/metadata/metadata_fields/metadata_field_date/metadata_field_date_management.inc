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
* $Id: metadata_field_date_management.inc,v 1.3.2.4 2004/04/21 10:55:16 brobertson Exp $
* $Name: not supported by cvs2svn $
*/


require_once SQ_INCLUDE_PATH.'/asset_management.inc';

/**
* Metadata_Field_Text_Management
*
* @see Asset_Management
*
* @author  Dmitri Iarandine <diarandine@squiz.net>
* @version $Version$ - 1.0
* @package MySource_Matrix_Packages
* @subpackage __core__
*/
class Metadata_Field_Date_Management extends Asset_Management
{


	/**
	* Constructor
	*
	*/
	function Metadata_Field_Date_Management(&$pm)
	{
		$this->Asset_Management($pm);

		$this->vars = Array(
							'offset'		=> Array(
													'added'   => '0.1',
													'type'    => 'text', 
													'default' => ' ',
													),
							'format'		=> Array(
													'added'   => '0.1',
													'type'    => 'text', 
													'default' => 'd/m/Y H:i:s',
													),
							'setup_params'	=> Array(
													'added'   => '0.0.1',
													'type'    => 'serialise',
													'default' => Array(),
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