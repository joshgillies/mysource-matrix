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
* $Id: file_bridge_management.inc,v 1.4 2012/08/30 00:58:36 ewang Exp $
*
*/


require_once SQ_INCLUDE_PATH.'/asset_management.inc';

/**
* File_Bridge_Management
*
* @see Asset_Management
*
* @author  Benjamin Pearson <bpearson@squiz.net>
* @version $Revision: 1.4 $
* @package MySource_Matrix_Packages
* @subpackage file
*/
class File_Bridge_Management extends Asset_Management
{


	/**
	* Constructor
	*
	*/
	function __construct(Package_Manager $pm)
	{
		Asset_Management::__construct($pm);

		$this->vars = Array(
						'name'	=> Array(
							  		'added'			=> '0.1',
							  		'type'			=> 'text',
							  		'default'		=> '',
							  		'description'	=> 'The name of the bridge',
							  		'update_fn'		=> '',
							  		'is_admin'		=> '0',
							  	   ),
						'path'	=> Array(
									'added'			=> '0.1',
									'type'			=> 'text',
									'default'		=> '',
									'description'	=> 'The path the bridge is responsible for',
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

		if (version_compare($current_version, '0.2', '<')) {
			echo('UPGRADING SEARCH PAGE FROM '.$current_version."\n");
			pre_echo('Updating File Bridge...');

			// all we care is removing the XML for "webpaths" screen
			// so just do it
			$file_for_webpath = SQ_DATA_PATH.'/private/asset_types/file_bridge/localised_screens/screen_paths.en';
			if (is_file($file_for_webpath)) {
				unlink($file_for_webpath);
			}

			$GLOBALS['SQ_SYSTEM']->changeDatabaseConnection('db2');
			$GLOBALS['SQ_SYSTEM']->doTransaction('BEGIN');

			// Get all File Bridge assets
			$file_bridges = $GLOBALS['SQ_SYSTEM']->am->getTypeAssetids('file_bridge', FALSE, TRUE);

			$count = 0;
			foreach ($file_bridges as $id => $file_bridge) {
				$bridge = $GLOBALS['SQ_SYSTEM']->am->getAsset($id);
				$GLOBALS['SQ_SYSTEM']->setRunLevel(SQ_RUN_LEVEL_FORCED);
					if (!$bridge->saveWebPaths(Array(), FALSE)) {
						$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
						$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();

						pre_echo('UPGRADE FAILED - Could not save webpaths');
						$GLOBALS['SQ_SYSTEM']->restoreRunLevel();
						return FALSE;
					}
				$GLOBALS['SQ_SYSTEM']->am->forgetAsset($bridge, TRUE);
				unset($bridge);
				$count++;

			}//end foreach bridge

			pre_echo('Updated - '.$count.' File Bridge(s)...');

			$GLOBALS['SQ_SYSTEM']->doTransaction('COMMIT');
			$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();


			pre_echo('FILE BRIDGES SUCCESSFULLY UPGRADED TO VERSION 0.2');
		}//end if
		
		
		if (version_compare($current_version, '0.3', '<')) {
			$asset_name = strtoupper($this->_pm->getTypeInfo($this->getAssetType(), 'name'));
			// version 0.2 -> 0.3
			pre_echo('STARTING '.$asset_name.' UPGRADE - TO VERSION 0.3');
			$file_bridges = $GLOBALS['SQ_SYSTEM']->am->getTypeAssetids($this->getAssetType(), FALSE, TRUE);
			$found = FALSE;
			foreach ($file_bridges as $assetid => $type_code) {
				$asset =& $GLOBALS['SQ_SYSTEM']->am->getAsset($assetid);
				if (is_null($asset)) continue;

				// We need to output our content files, otherwise old paint layouts will print blank
				$path = $asset->attr('path');
				if(!empty($path)) {
				    echo('Detected File Bridge asset #'.$asset->id.' with Path attribute "'.$asset->attr('path').'"'."\n");
				    $found = TRUE;
				}

				$GLOBALS['SQ_SYSTEM']->am->forgetAsset($asset);
				unset($asset);

			}//end foreach
			
			if($found) {
			    echo ("\n");
			    echo ('******************************************************************************************************************'."\n");
			    echo ("Security Updates\n");
			    echo('Paths of above assets are now restored to "data/public" for security purposes'."\n");
			    echo('If old paths are required and considered safe, please configure '.SQ_DATA_PATH.'/private/conf/file_bridge.inc'."\n");
			    echo('******************************************************************************************************************'."\n");
			}	    
			pre_echo($asset_name.' UPGRADE COMPLETE - FROM VERSION '.$current_version.' TO 0.3');

		}//end version 0.3 upgrade

		return TRUE;

	}//end _upgrade()
	
}//end class

?>
