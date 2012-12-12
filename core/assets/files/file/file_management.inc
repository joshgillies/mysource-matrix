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
* $Id: file_management.inc,v 1.24 2012/12/12 01:16:49 ewang Exp $
*
*/


require_once SQ_INCLUDE_PATH.'/asset_management.inc';

/**
* File_Management
*
* @see Asset_Management
*
* @author  Greg Sherwood <greg@squiz.net>
* @version $Revision: 1.24 $
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
													'is_contextable'	=> TRUE,
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
		
		if (version_compare($current_version, '0.5', '<')) {
			pre_echo('UPGRADING FILE ASSETS - FROM VERSION '.$current_version);
			$where = 'type_code IN (
									SELECT type_code
									FROM sq_ast_typ_inhd
									WHERE inhd_type_code = :type_code
									)';
			$where .= ' AND (status = \''.SQ_STATUS_EDITING.'\' OR status = \''.SQ_STATUS_EDITING_APPROVAL.'\' OR status = \''.SQ_STATUS_EDITING_APPROVED.'\')';
			$sql = 'SELECT assetid FROM '.SQ_TABLE_RUNNING_PREFIX.'ast WHERE '.$where;
			$query = MatrixDAL::preparePdoQuery($sql);
			MatrixDAL::bindValueToPdo($query, 'type_code', $this->getAssetType());
			$children = MatrixDAL::executePdoAssoc($query, 0);
			if(!empty($children)) {
			    foreach ($children as $assetid) {
				    $asset = $GLOBALS['SQ_SYSTEM']->am->getAsset($assetid);
				    $url = $asset->getURL();
				    if(empty($asset)) continue;
				    if(empty($url)) continue;
				    pre_echo('FILE #'.$asset->id.' UPDATED');
				    $asset->updateLookups();
				    $GLOBALS['SQ_SYSTEM']->am->forgetAsset($asset, TRUE);
			    }
			}
			pre_echo('FILE ASSETS UPGRADE COMPLETE - VERSION 0.5');
		}//end if version 0.5

		return TRUE;

	}//end _upgrade()


}//end class
?>
