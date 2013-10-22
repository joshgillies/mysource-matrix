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
* $Id: image_management.inc,v 1.22 2013/10/22 04:09:14 akarelia Exp $
*
*/


require_once SQ_INCLUDE_PATH.'/asset_management.inc';

/**
* Image_Management
*
* @see Asset_Management
*
* @author  Greg Sherwood <greg@squiz.net>
* @version $Revision: 1.22 $
* @package MySource_Matrix_Packages
* @subpackage __core__
*/
class Image_Management extends Asset_Management
{


	/**
	* Constructor
	*
	* 'embedded_data' is in format Array('keyword'=>Array('name'=>name of data element, 'value'=>Array(value,...)))
	*
	*/
	function Image_Management(&$pm)
	{
		$this->Asset_Management($pm);

		$this->vars = Array(
						'alt'			=> Array(
											'added'			=> '0.2',
											'type'			=> 'text',
											'default'		=> '',
											'description'	=> 'The Alt attribute for the IMG tag',
											'is_admin'		=> FALSE,
											'is_contextable'	=> TRUE,
										   ),
						'caption'		=> Array(
											'added'			=> '0.2',
											'type'			=> 'text',
											'default'		=> '',
											'description'	=> 'The Caption attribute for the image',
											'is_admin'		=> FALSE,
											'is_contextable'	=> TRUE,
										   ),
						'width'			=> Array(
											'added'			=> '0.2',
											'type'			=> 'int',
											'default'		=> 0,
											'description'	=> 'Width of the image (generated from the image file)',
											'is_admin'		=> FALSE,
										   ),
						'height'		=> Array(
											'added'			=> '0.2',
											'type'			=> 'int',
											'default'		=> 0,
											'description'	=> 'Height of the image (generated from the image file)',
											'is_admin'		=> FALSE,
										   ),
						'size'			=> Array(
											'added'			=> '0.2',
											'type'			=> 'int',
											'default'		=> 0,
											'description'	=> 'Size (in bytes) of the image (generated from the image file)',
											'is_admin'		=> FALSE,
										   ),
						'varieties'		=> Array(
											'added'		=> '0.3',
											'type'		=> 'serialise',
											'default'	=> Array(),
											'is_admin'	=> TRUE,
										   ),
						'variety_count'	=> Array(
											'added'		=> '0.3',
											'type'		=> 'int',
											'default'	=> 1,
											'is_admin'	=> TRUE,
										   ),
						'embedded_data'	=> Array(
											'added'			=> '0.4',
											'type'			=> 'serialise',
											'default'		=> Array(),
											'description'	=> 'Embedded Metadata placed by Photoshop',
											'is_admin'		=> TRUE,
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
		if (!parent::_upgrade($current_version)) return FALSE;

		$asset_name = strtoupper($this->_pm->getTypeInfo($this->getAssetType(), 'name'));

		if (version_compare($current_version, '0.2', '<')) {
			// version 0.1 -> 0.2
			// Added the attributes of which alt, title, width, height, size need to be set
			pre_echo('STARTING '.$asset_name.' UPGRADE - FROM VERSION 0.1');
			$images = $GLOBALS['SQ_SYSTEM']->am->getTypeAssetids($this->getAssetType(), FALSE, TRUE);
			foreach ($images as $assetid => $type_code) {
				$asset = $GLOBALS['SQ_SYSTEM']->am->getAsset($assetid, $type_code);
				if (is_null($asset)) continue;

				if (!$asset->setAttrValue('alt',   $asset->name) || !$asset->setAttrValue('title', $asset->name)) {
					echo 'Asset #', $asset->id, ' (', $asset->name,') Unable to set alt and/or title', "\n";
					continue;
				}

				$info = $asset->getExistingFile();
				if (!empty($info)) {
					if (!$asset->setAttrValue('width',  $info['width']) || !$asset->setAttrValue('height', $info['height']) || !$asset->setAttrValue('size',   $info['size'])) {
						echo 'Asset #', $asset->id, ' (', $asset->name,') Unable to set width, height and/or size', "\n";
						continue;
					}
				}//end if

				if (!$asset->saveAttributes()) {
					echo 'Asset #', $asset->id, ' (', $asset->name,') Unable to save attributes', "\n";
					continue;
				}
				$GLOBALS['SQ_SYSTEM']->am->forgetAsset($asset);
				unset($asset);

			}//end foreach
			pre_echo($asset_name.' UPGRADE COMPLETE - FROM VERSION 0.1');

		}//end if - upgrade to version 0.2

		return TRUE;

	}//end _upgrade()


}//end class
?>
