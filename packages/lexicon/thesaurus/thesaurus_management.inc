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
* $Id: thesaurus_management.inc,v 1.3 2005/01/20 13:21:23 brobertson Exp $
*
*/

require_once SQ_INCLUDE_PATH.'/asset_management.inc';

/**
* Thesaurus_Management
*
* @see Asset_Management
*
* @author  Andrei Railean <arailean@squiz.net>
* @author  Dmitry Baranovskiy <dbaranovskiy@squiz.net>
* @version $Revision: 1.3 $
* @package MySource_Matrix_Packages
* @subpackage lexicon
*/
class Thesaurus_Management extends Asset_Management
{

	/**
	* Constructor
	*
	*/
	function Thesaurus_Management(&$pm)
	{
		$this->Asset_Management($pm);
		$this->vars = Array(
			'name' => Array(
				'added'       => '0.0.1',
				'type'        => 'text',
				'default'   => '',
				'description' => 'The full name of the page',
			),

			'import_timestamp'=> Array(
				'added'       => '0.0.1',
				'type'        => 'text',
				'default'     => '',
				'description' => 'Date and Time of last Thesaurus import',
			),

			'active' => Array(
				'added'       => '0.0.1',
				'type'        => 'boolean',
				'default'   => false,
			),

			'relations' => Array(
				'added'      => '0.0.1',
				'type'		 => 'serialise',
				'default'	 => Array(),
				'description'=> 'Cached listing of all the relations in this thesaurus in the form [name => id]',
			),

			'relation_settings' => Array(
				'added'      => '0.0.1',
				'type'		 => 'serialise',
				'default'	 => Array(),
				'description'=> 'Information required for the built-in relations to function.',
			),

			'builtin_relations' => Array(
				'added'      => '0.0.1',
				'type'		 => 'serialise',
				'default'	 => Array(),
				'description'=> 'Mapping from built-in relation type to the relation id as stored in the database. Also serves as an indication of whether or not a certain relation is enabled.',
			),
		);

	}//end constructor

}//end class

?>