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
* $Id: file_management.inc,v 1.17 2006/01/18 00:59:32 lwright Exp $
*
*/


require_once SQ_INCLUDE_PATH.'/asset_management.inc';

/**
* File_Management
*
* @see Asset_Management
*
* @author  Greg Sherwood <greg@squiz.net>
* @version $Revision: 1.17 $
* @package MySource_Matrix_Packages
* @subpackage __core__
*/
class File_Management extends Asset_Management
{


	/**
	* Constructor
	*
	*/
	function File_Management(&$pm)
	{
		$this->Asset_Management($pm);

		$this->vars = Array(
						'name'					=> Array(
													'added'			=> '0.1',
													'type'			=> 'text',
													'default'		=> '',
													'description'	=> 'The name of the file (eg. myfile.zip)',
													'update_fn'		=> '',
													'is_admin'		=> '0',
												   ),
						'allow_unrestricted'	=> Array(
													'added'			=> '0.2',
													'type'			=> 'boolean',
													'default'		=> TRUE,
													'description'	=> 'Allows the use of the __data URL if true',
													'parameters'	=> Array('allow_empty' => FALSE),
													'update_fn'		=> '',
												   ),
						'title'					=> Array(
													'added'			=> '0.3',
													'type'			=> 'text',
													'default'		=> '',
													'is_admin'		=> '0',
													'description'	=> 'Friendly name for the file',
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

		if (version_compare($current_version, '0.4', '<')) {
			pre_echo('UPGRADING FILE ASSETS - FROM VERSION '.$current_version);
			$children = $GLOBALS['SQ_SYSTEM']->am->getTypeAssetids($this->getAssetType(), FALSE);
			foreach ($children as $assetid) {
				$asset =& $GLOBALS['SQ_SYSTEM']->am->getAsset($assetid);
				$attr = $asset->attr('title');
				if (empty($attr)) {
					$asset->setAttrValue('title', $asset->attr('name'));
					$asset->saveAttributes();
				}
				$GLOBALS['SQ_SYSTEM']->am->forgetAsset($asset, TRUE);
			}
		}//end if version 0.4

		return TRUE;

	}//end _upgrade()


}//end class
?>
