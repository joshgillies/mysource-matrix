<?php
/**
* +--------------------------------------------------------------------+
* | Squiz.net Commercial Module Licence                                |
* +--------------------------------------------------------------------+
* | Copyright (c) Squiz Pty Ltd (ACN 084 670 600).                     |
* +--------------------------------------------------------------------+
* | This source file is not open source or freely usable and may be    |
* | used subject to, and only in accordance with, the Squiz Commercial |
* | Module Licence.                                                    |
* | Please refer to http://www.squiz.net/licence for more information. |
* +--------------------------------------------------------------------+
*
* $Id: thesaurus_management.inc,v 1.2.2.1 2005/05/02 00:35:50 gsherwood Exp $
* $Name: not supported by cvs2svn $
*/

require_once SQ_INCLUDE_PATH.'/asset_management.inc';

/**
* Thesaurus_Management
*
* @see Asset_Management
*
* @author  Andrei Railean <arailean@squiz.net>
* @author  Dmitry Baranovskiy <dbaranovskiy@squiz.net>
* @version $Revision: 1.2.2.1 $
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