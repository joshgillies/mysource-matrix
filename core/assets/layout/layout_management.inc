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
* $Id: layout_management.inc,v 1.12 2013/09/24 01:16:55 ewang Exp $
*
*/


require_once SQ_INCLUDE_PATH.'/asset_management.inc';

/**
* Folder_Management
*
* @see Asset_Management
*
* @author  Marc McIntyre <mmcintyre@squiz.net>
* @version $Revision: 1.12 $
* @package MySource_Matrix_Packages
* @subpackage __core__
*/
class Layout_Management extends Asset_Management
{


	/**
	* Constructor
	*
	*/
	function __construct(Package_Manager $pm)
	{
		Asset_Management::__construct($pm);

		$this->vars = Array(
						'name'		=> Array(
										'added'		=> '0.0.1',
										'type'		=> 'text',
										'default'	=> '',
									   ),
						'options'	=> Array(
										'added'		=> '0.2',
										'type'		=> 'serialise',
										'default'	=> Array(
														'show_cascade_metadata_field'	=> FALSE,
													   ),
									   ),

					  );

	}//end constructor


	/**
	* Return an array with information about the events that the account manager is listening for
	*
	* @return boolean
	* @access public
	*/
	public function getEventList()
	{
		return Array(
				Array(
					'event_name'					=> 'requestKeywords',
					'broadcast_type_code'			=> 'content_type',
					'broadcast_strict_type_code'	=> FALSE,
					'options'						=> Array(
														'side_of_link'	=> 'major',
														'indirect'		=> TRUE,
														'is_exclusive'	=> NULL,
														'is_dependant'	=> 1,
														'value'			=> '',
														'link_type'		=> SQ_LINK_TYPE_2 | SQ_LINK_TYPE_1,
													   ),
				),
			   );

	}//end getEventList()


	/**
	* return an array which contains the list of external files needed
	* by this asset
	*
	* @return array
	* @access private
	*/
	public function _getFileList()
	{
	  return Array();

	}//end _getFileList()


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

        $asset_name = strtoupper($this->_pm->getTypeInfo($this->getAssetType(), 'name'));
        $am = $GLOBALS['SQ_SYSTEM']->am;

        if (version_compare($current_version, '0.2', '<')) {
            // version 0.1 -> 0.2
            pre_echo('STARTING '.$asset_name.' UPGRADE - TO VERSION 0.2');

			$default_val = Array('show_cascade_metadata_field'   => FALSE);

            $pages = $GLOBALS['SQ_SYSTEM']->am->getTypeAssetids($this->getAssetType(), TRUE, TRUE);

            foreach ($pages as $assetid => $type_code) {
                $asset = $GLOBALS['SQ_SYSTEM']->am->getAsset($assetid, $type_code['type_code']);
                if (is_null($asset)) continue;
                echo 'Upgrading Asset #'.$assetid."...\n";

				// Add "options" attribute to existing Layout assets
				
                if (!$asset->setAttrValue('options', $default_val)) {
                    echo 'Asset #', $asset->id, ' (', $asset->name,') Could not add "options" attribute to existing Layout', "\n";
                    continue;
                }

				if (!$asset->saveAttributes()) {
                    echo 'Asset #', $asset->id, ' (', $asset->name,') Unable to save attributes', "\n";
                    continue;
                }
                $GLOBALS['SQ_SYSTEM']->am->forgetAsset($asset);
                unset($asset);
            }//end foreach
            pre_echo($asset_name.' UPGRADE COMPLETE - TO VERSION 0.2');

        }//end if - upgrade to version 0.2

		return TRUE;

	}//end _upgrade()


}//end class
?>
