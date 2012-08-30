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
* $Id: thesaurus_term_management.inc,v 1.5 2012/08/30 01:09:16 ewang Exp $
*
*/


require_once SQ_INCLUDE_PATH.'/asset_management.inc';

/**
* Thesaurus_Term_Management
*
* @see Asset_Management
*
* @author  Andrei Railean <arailean@squiz.net>
* @version $Revision: 1.5 $
* @package MySource_Matrix_Packages
* @subpackage __core__
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
										'is_admin'		=> FALSE,
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
