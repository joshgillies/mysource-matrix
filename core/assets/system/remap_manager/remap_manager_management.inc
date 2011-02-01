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
* $Id: remap_manager_management.inc,v 1.14 2011/02/01 23:49:48 mhaidar Exp $
*
*/


require_once SQ_INCLUDE_PATH.'/asset_management.inc';

/**
* Remap_Manager_Management
*
* @see Asset_Management
*
* @author  Greg Sherwood <greg@squiz.net>
* @version $Revision: 1.14 $
* @package MySource_Matrix_Packages
* @subpackage __core__
*/
class Remap_Manager_Management extends Asset_Management
{


	/**
	* Constructor
	*
	* @param Package_Manager	$pm	The package manager handling this asset
	*
	*/
	function __construct(Package_Manager $pm)
	{
		Asset_Management::__construct($pm);

		$this->vars = Array(
						'404_asset'			=> Array(
											'added'		=> '0.0.1',
											'type'		=> 'assetid',
											'default'	=> '0',
										   ),
						'remap_upon_webpath_change'	=> Array(
											'added'		=> '0.2.1',
											'type'		=> 'boolean',
											'default'	=> TRUE,
										   ),
						'never_delete_remap_default'	=> Array(
											'added'		=> '0.2.1',
											'type'		=> 'boolean',
											'default'	=> FALSE,
										   ),
					  );

	}//end constructor


    /**
    * Upgrade this asset type
    *
    * @param string $current_version    the version of the asset that is currenly installed
    *
    * @return boolean
    * @access public
    */
    public function _upgrade($current_version)
    {
        if (!parent::_upgrade($current_version)) return FALSE;
		
		if (version_compare($current_version, '0.2.2', '<')) {
			
			$rm = $GLOBALS['SQ_SYSTEM']->am->getSystemAsset('remap_manager');

			$sql = "SELECT custom_val FROM sq_ast_attr a, sq_ast_attr_val v where v.assetid = '".$rm->id."' AND a.name='never_delete_remap_default' AND a.attrid=v.attrid";
			$record = MatrixDAL::executeSqlAssoc($sql);

			if (empty($record)) {
				$rm->setAttrValue('never_delete_remap_default', '1');
				$rm->saveAttributes();
			}
		}

		return TRUE;

	}//end _updrade()

}//end class

?>
