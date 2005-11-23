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
* $Id: metadata_field_thesaurus_management.inc,v 1.1 2005/11/23 00:53:35 arailean Exp $
*
*/


require_once SQ_INCLUDE_PATH.'/asset_management.inc';

/**
* Metadata_Field_Thesaurus_Management
*
* @see Asset_Management
*
* @author  Andrei Railean <arailean@squiz.net>
* @version $Revision: 1.1 $
* @package MySource_Matrix_Packages
* @subpackage __core__
*/
class Metadata_Field_Thesaurus_Management extends Asset_Management
{


	/**
	* Constructor
	*
	*/
	function Metadata_Field_Thesaurus_Management(&$pm)
	{
		$this->Asset_Management($pm);

		$this->vars = Array(
						'thesaurus_assetid'	=> Array(
												'added'			=> '0.1',
												'type'			=> 'assetid',
												'default'		=> '',
												'description'	=> 'Thesaurus asset which will supply the data to this metadata field.',
											   ),
					  );

	}//end constructor


}//end class

?>