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
* $Id: thesaurus_management.inc,v 1.4 2006/12/05 23:40:15 bcaldwell Exp $
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
* @version $Revision: 1.4 $
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