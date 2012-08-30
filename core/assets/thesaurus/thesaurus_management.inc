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
* $Id: thesaurus_management.inc,v 1.9 2012/08/30 01:09:16 ewang Exp $
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
* @version $Revision: 1.9 $
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
												'added'			=> '0.0.1',
												'type'			=> 'text',
												'default'		=> '',
												'is_admin'		=> FALSE,
												'is_contextable'=> TRUE,
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
						'hierarchy_mode'	=> Array(
												'added'			=> '0.2',
												'type'			=> 'boolean',
												'default'		=> '1',
												'parameters'	=> Array ('allow_empty'=>FALSE),
											   ),
						'readonly_mode'		=> Array(
												'added'			=> '0.3',
												'type'			=> 'boolean',
												'default'		=> '0',
												'parameters'	=> Array ('allow_empty'=>FALSE),
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
						'default_rel'		=> Array(
												'added'		=> '0.3',
												'type'		=> 'text',
												'default'	=> NULL,
											   ),
						'reflections'		=> Array(
												'added'			=> '0.0.1',
												'type'			=> 'serialise',
												'default'		=> Array(),
												'description'	=> 'An array with reflective pair index. Format is "relation_id" => "other_relation_id". The property of this array is that if "A"=>"B", "B"=>"A"',
											   ),
						'enforce_reflection'	=> Array(
												'added'			=> '0.3',
												'type'			=> 'boolean',
												'default'		=> FALSE,
												'parameters'	=> Array ('allow_empty'=>FALSE),
											   ),
						'hierarchy_rel'		=> Array(
												'added'		=> '0.4',
												'type'		=> 'text',
												'default'	=> NULL,
											   ),
						'hierarchy_w_stray'	=> Array(
												'added'			=> '0.4',
												'type'			=> 'boolean',
												'default'		=> FALSE,
												'parameters'	=> Array ('allow_empty'=>FALSE),
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
