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
* $Id: file_management.inc,v 1.20 2007/12/11 03:29:57 rong Exp $
*
*/


require_once SQ_INCLUDE_PATH.'/asset_management.inc';

/**
* File_Management
*
* @see Asset_Management
*
* @author  Greg Sherwood <greg@squiz.net>
* @version $Revision: 1.20 $
* @package MySource_Matrix_Packages
* @subpackage __core__
*/
class File_Management extends Asset_Management
{


	/**
	* Constructor
	*
	*/
	function __construct(Package_Manager $pm)
	{
		Asset_Management::__construct($pm);

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
				$asset = $GLOBALS['SQ_SYSTEM']->am->getAsset($assetid);
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
