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
* $Id: paint_layout_bodycopy_management.inc,v 1.13 2012/08/30 01:09:09 ewang Exp $
*
*/


require_once SQ_INCLUDE_PATH.'/asset_management.inc';

/**
* Folder_Management
*
* @see Asset_Management
*
* @author  Marc McIntyre <mmcintyre@squiz.net>
* @version $Revision: 1.13 $
* @package MySource_Matrix_Packages
* @subpackage __core__
*/
class Paint_Layout_Bodycopy_Management extends Asset_Management
{


	/**
	* Constructor
	*
	*/
	function __construct(Package_Manager $pm)
	{
		Asset_Management::__construct($pm);

		$this->vars = Array(
						'conditional_keywords'	=> Array(
													'added'		=> '0.1',
													'type'		=> 'serialise',
													'default'	=> Array(),
												   ),
						'associated_asset_type'	=> Array(
													'added'		=> '0.1',
													'type'		=> 'text',
													'default'	=> '',
												   ),
						'lineage_separator'		=> Array (
													'added'			=> '0.1',
													'type'			=> 'text',
													'default'		=> ' > ',
													'description'	=> 'Seperator to use when painting the lineage of an asset.',
												   ),
					  );

	}//end constructor


	/**
	* Upgrade
	*
	* @param string	$current_version	the version of the asset that is currenly installed
	*
	* @return boolean
	* @access public
	*/
	function _upgrade($current_version)
	{
		parent::_upgrade($current_version);

		if (version_compare($current_version, '0.6', '<')) {
			echo('UPGRADING PAINT LAYOUT BODYCOPY FROM '.$current_version." TO VERSION 0.6\n");

			pre_echo('Updating Paint Layout Bodycopy...');
			// Get all Search Page assets
			$paint_layout_bodycopy = $GLOBALS['SQ_SYSTEM']->am->getTypeAssetids('paint_layout_bodycopy', FALSE, TRUE);
			foreach ($paint_layout_bodycopy as $id => $type) {
				$asset = $GLOBALS['SQ_SYSTEM']->am->getAsset($id, '', TRUE);

				if (!is_null($asset)) {
					$asset->_updated();
				}

				$asset = NULL;
			}//

			pre_echo('PAINT LAYOUT BODYCOPY SUCCESSFULLY UPGRADED TO VERSION 0.6');
		}//end if

		return TRUE;

	}//end _upgrade()

	
}//end class

?>
