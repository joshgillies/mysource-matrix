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
* $Id: tag_manager_management.inc,v 1.5 2012/08/30 01:09:12 ewang Exp $
*
*/

require_once SQ_INCLUDE_PATH.'/asset_management.inc';

/**
* Tag_Manager_Management
*
* @see Asset_Management
*

* @author	Elden McDonald <emcdonald@squiz.net>
* @version $Revision: 1.5 $
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
			$tag_manager = $GLOBALS['SQ_SYSTEM']->getTagManager();
			if ($tag_manager->attr('thesaurus')) {
				$tag_manager->setAttrValue('restrict_tag_source', TRUE);
				$tag_manager->saveAttributes();
			}
		}//end if version 0.2

		return TRUE;

	}//end _upgrade()



}//end class

?>
