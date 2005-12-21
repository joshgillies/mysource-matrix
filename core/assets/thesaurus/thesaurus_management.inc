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
* $Id: thesaurus_management.inc,v 1.2 2005/12/21 04:17:21 arailean Exp $
*
*/

require_once SQ_INCLUDE_PATH.'/asset_management.inc';

/**
* Thesaurus_Management
*
* @see Asset_Management
*
* @author  Andrei Railean <arailean@squiz.net>
* @author  Elden McDonald <emcdonald@squiz.net>
* @version $Revision: 1.2 $
* @package MySource_Matrix_Packages
* @subpackage __core__
*/
class Thesaurus_Management extends Asset_Management
{


	/**
	* Constructor
	*
	* @param object	&$pm	package manager
	*
	* @return void
	* @access public
	*/
	function Thesaurus_Management(&$pm)
	{
		$this->Asset_Management($pm);
		$this->vars = Array(
						'name'				=> Array(
												'added'		=> '0.0.1',
												'type'		=> 'text',
												'default'	=> '',
											   ),
						'relations'			=> Array(
												'added'			=> '0.0.1',
												'type'			=> 'serialise',
												'default'		=> Array(),
												'description'	=> 'Cached listing of all the relations in this thesaurus in the form [name => id]',
											   ),
						'contents_changed'	=> Array(
												'added'		=> '0.2',
												'type'		=> 'boolean',
												'default'	=> '0',
											   ),
						'synonym_rel'		=> Array(
												'added'		=> '0.3',
												'type'		=> 'text',
												'default'	=> 'Synonym',
											   ),
						'abbreviation_rel'	=> Array(
												'added'		=> '0.3',
												'type'		=> 'text',
												'default'	=> 'Abbreviation',
											   ),
					  );
	}//end constructor


	/**
	* Gets a list of all files that the bodycopy wants web accessable
	*
	* @return array
	* @access private
	*/
	function _getFileList()
	{
		return Array(
				'files/example.xml',
			   );

	}//end _getFileList()


}//end class

?>