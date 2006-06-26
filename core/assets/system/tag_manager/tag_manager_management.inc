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
* $Id: tag_manager_management.inc,v 1.2 2006/06/26 04:32:34 rhoward Exp $
*
*/

require_once SQ_INCLUDE_PATH.'/asset_management.inc';

/**
* Tag_Manager_Management
*
* @see Asset_Management
*

* @author	Elden McDonald <emcdonald@squiz.net>
* @version $Revision: 1.2 $
* @package MySource_Matrix_Packages
* @subpackage __core__
*/
class Tag_Manager_Management extends Asset_Management
{


	/**
	* Constructor
	*
	*/
	function Tag_Manager_Management(&$pm)
	{
		$this->Asset_Management($pm);

		$this->vars = Array(
						'thesaurus'				=> Array(
													'added'			=> '0.1',
													'type'			=> 'assetid',
													'description'	=> 'The thesaurus from which tags may be selected',
													'default'		=> '',
												   ),
						'restrict_tag_source'	=> Array(
													'added'			=> '0.2',
													'type'			=> 'boolean',
													'parameters'	=> Array(
																		'allow_empty'	=> FALSE,
																	   ),
													'description'	=> 'Determines whether or not tags are restricted to a single thesaurus',
													'default'		=> FALSE,
												   ),
					  );


	}//end constructor


	/**
	* Upgrade script
	*
	* @param string	$current_version	the version we are upgrading FROM
	*
	* @return boolean
	* @access private
	* @see Asset_Management::_upgrade()
	*/
	function _upgrade($current_version)
	{
		if (!parent::_upgrade($current_version)) return FALSE;

		if (version_compare($current_version, '0.2', '<')) {
			pre_echo('UPGRADING TAG MANAGER - FROM VERSION '.$current_version);
			// adding 'restrict_tag_source' attribute - set it to TRUE/FALSE based
			// on whether a tagging thesaurus is currently selected
			$tag_manager =& $GLOBALS['SQ_SYSTEM']->getTagManager();
			if ($tag_manager->attr('thesaurus')) {
				$tag_manager->setAttrValue('restrict_tag_source', TRUE);
				$tag_manager->saveAttributes();
			}
		}//end if version 0.2

		return TRUE;

	}//end _upgrade()



}//end class

?>
