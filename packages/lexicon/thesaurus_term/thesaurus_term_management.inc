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
* $Id: thesaurus_term_management.inc,v 1.1 2005/11/04 06:09:00 arailean Exp $
*
*/


require_once SQ_INCLUDE_PATH.'/asset_management.inc';

/**
* Thesaurus_Term_Management
*
* @see Asset_Management
*
* @author  Andrei Railean <arailean@squiz.net>
* @version $Revision: 1.1 $
* @package MySource_Matrix_Packages
* @subpackage lexicon
*/
class Thesaurus_Term_Management extends Asset_Management
{


	/**
	* Constructor
	*
	*/
	function Thesaurus_Term_Management(&$pm)
	{
		$this->Asset_Management($pm);

		$this->vars = Array(
						'name'		=> Array(
										'added'			=> '0.1',
										'type'			=> 'text',
										'default'		=> '',
										'description'	=> 'Term name (the term itself)',
									   ),
						'relation'	=> Array(
										'added'			=> '0.1',
										'type'			=> 'text',
										'default'		=> '',
										'description'	=> 'Relation this term has to its parent',
									   ),
					  );

	}//end constructor


}//end class
?>
